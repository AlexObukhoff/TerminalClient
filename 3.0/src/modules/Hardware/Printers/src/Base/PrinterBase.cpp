/* @file Базовый принтер. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtAlgorithms>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/SerialDeviceBase.h"
#include "Hardware/Common/TCPDeviceBase.h"
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/ProtoDevices.h"
#include "Hardware/Printers/PrinterStatusesDescriptions.h"

// Project
#include "PrinterBase.h"

using namespace SDK::Driver;
using namespace PrinterStatusCode;

//---------------------------------------------------------------------------
template class PrinterBase<PollingDeviceBase<ProtoPrinter>>;
template class PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoPrinter>>>;

//---------------------------------------------------------------------------
template <class T>
PrinterBase<T>::PrinterBase()
{
	// теги по умолчанию
	mTagEngine = Tags::PEngine(new Tags::Engine());

	// описания для кодов статусов
	mStatusCodesSpecification = DeviceStatusCode::PSpecifications(new CSpecifications());
	mMaxBadAnswers = 2;

	// восстановимые ошибки
	QMap<int, SStatusCodeSpecification> & statusCodesData = mStatusCodesSpecification->data();

	for(auto it = statusCodesData.begin(); it != statusCodesData.end(); ++it)
	{
		if (it->warningLevel == EWarningLevel::Warning)
		{
			mRecoverableErrors.insert(it.key());
		}
	}

	// настройки устройства
	mPollingInterval = 5 * 1000;
	mLineSize = 0;
	mLineFeed = true;
	mPaperInPresenter = QDateTime::currentDateTime();
	mActualStringCount = 0;
	mPrintingMode = EPrintingModes::None;
	mClearingDispenserTurnOff = false;

	// настройки для плагинов
	setConfigParameter(CHardware::Printer::NeedSeparating, true);
	setConfigParameter(CHardware::Printer::PresenterEnable, false);
	setConfigParameter(CHardware::Printer::RetractorEnable, false);
	setConfigParameter(CHardware::Printer::FeedingAmount, 0);
	setConfigParameter(CHardware::Printer::NeedCutting, true);

	mExcessStatusCollection[EWarningLevel::OK].insert(PrinterStatusCode::OK::PaperInPresenter);
	mExcessStatusCollection[EWarningLevel::OK].insert(PrinterStatusCode::OK::MotorMotion);
}

//--------------------------------------------------------------------------------
template <class T>
void PrinterBase<T>::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	T::setDeviceConfiguration(aConfiguration);

	if (aConfiguration.contains(CHardwareSDK::Printer::PrintingMode))
	{
		mPrintingMode = EPrintingModes::Enum(aConfiguration[CHardwareSDK::Printer::PrintingMode].toInt());

		if (mPrintingMode == EPrintingModes::Glue)
		{
			mClearingDispenserTurnOff = true;
		}
	}
}

//--------------------------------------------------------------------------------
template<class T>
void PrinterBase<T>::finaliseInitialization()
{
	if (containsConfigParameter(CHardwareSDK::Printer::LineSize))
	{
		setDeviceParameter(CHardwareSDK::Printer::LineSize, getConfigParameter(CHardwareSDK::Printer::LineSize));
	}

	T::finaliseInitialization();
}

//--------------------------------------------------------------------------------
template <class T>
bool PrinterBase<T>::isConnected()
{
	TStatusCodes statusCodes;

	return getStatus(statusCodes) && !statusCodes.contains(DeviceStatusCode::Error::NotAvailable);
}

//---------------------------------------------------------------------------
template <class T>
bool PrinterBase<T>::processNonReentrant(TBoolMethod aCommand)
{
	MutexLocker externalLocker(&mExternalMutex);

	stopPolling(true);

	{
		MutexLocker resourceLocker(&mResourceMutex);

		if (!checkConnectionAbility() || !PrinterBase<T>::isConnected())
		{
			if (mOperatorPresence)
			{
				processStatusCodes(TStatusCodes() << DeviceStatusCode::Error::NotAvailable);
			}

			startPolling(true);

			return false;
		}
	}

	if (mStatusCollection.contains(OK::PaperInPresenter))
	{
		clearDispenser(CHardware::Printer::Settings::PreviousReceipt);
	}

	bool result = aCommand();

	mForceStatusBufferEnabled = mForceStatusBufferEnabled || canForceStatusBufferEnable();
	simplePoll();
	mForceStatusBufferEnabled = false;

	if (getConfigParameter(CHardware::Printer::PresenterEnable).toBool() || getConfigParameter(CHardware::Printer::RetractorEnable).toBool())
	{
		mPaperInPresenter = QDateTime::currentDateTime();
	}

	startPolling(true);

	return result;
}

//---------------------------------------------------------------------------
template <class T>
bool PrinterBase<T>::print(const QStringList & aReceipt)
{
	if (!isPrintingNeed(aReceipt))
	{
		return true;
	}

	QStringList receipt = simplifyReceipt(aReceipt);

	if (!processNonReentrant(std::bind(&PrinterBase<T>::processReceipt, this, std::ref(receipt), true)))
	{
		toLog(LogLevel::Error, "Printing completed unsuccessfully.");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
void PrinterBase<T>::makeLexemeReceipt(const QStringList & aReceipt, Tags::TLexemeReceipt & aLexemeReceipt)
{
	QStringList receipt(aReceipt);

	if (getConfigParameter(CHardware::Printer::NeedSeparating).toBool())
	{
		separate(receipt);
	}

	foreach(auto line, receipt)
	{
		Tags::TLexemesBuffer tagLexemes;
		mTagEngine->splitForLexemes(line, tagLexemes);

		if (!tagLexemes.isEmpty())
		{
			Tags::TLexemesCollection lexemesCollection;
			adjustToLineSize(tagLexemes, lexemesCollection);

			aLexemeReceipt << lexemesCollection;
		}
	}
}

//--------------------------------------------------------------------------------
template <class T>
QStringList PrinterBase<T>::simplifyReceipt(const QStringList & aReceipt)
{
	QStringList result(aReceipt);
	QRegExp regExpEmptyLine("^[ \\n\\r\\t]+$");
	QRegExp regExpFreeSpace("[ \\n\\r\\t]+$");

	for (int i = 0; i < result.size(); ++i)
	{
		QStringList lines = result[i].split(Tags::BR).replaceInStrings(regExpEmptyLine, "");
		lines.removeAll("");

		if (!lines.size())
		{
			lines << "";
		}

		result.removeAt(i--);

		for (int j = 0; j < lines.size(); ++j)
		{
			int index = lines[j].indexOf(regExpFreeSpace);
			result.insert(++i, lines[j].left(index));
		}
	}

	for (int i = 0; i < result.size(); ++i)
	{
		result[i] = result[i].replace(ASCII::TAB, ASCII::Space);

		for (auto it = CPrinters::AutoCorrection.data().begin(); it != CPrinters::AutoCorrection.data().end(); ++it)
		{
			result[i] = result[i].replace(it.key(), it.value());
		}
	}

	for (int i = 0; i < result.size(); ++i)
	{
		if (result[i].simplified().isEmpty())
		{
			result.removeAt(i--);
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
bool PrinterBase<T>::isPrintingNeed(const QStringList & aReceipt)
{
	if (aReceipt.isEmpty())
	{
		toLog(LogLevel::Normal, mDeviceName + ": receipt is empty");
		return false;
	}

	QStringList receipt = simplifyReceipt(aReceipt);

	if (receipt.isEmpty())
	{
		toLog(LogLevel::Normal, mDeviceName + ": simplified receipt is empty");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool PrinterBase<T>::processReceipt(const QStringList & aReceipt, bool aProcessing)
{
	if (!isPrintingNeed(aReceipt))
	{
		return true;
	}

	toLog(LogLevel::Normal, "Printing receipt:\n" + aReceipt.join("\n"));

	Tags::TLexemeReceipt lexemeReceipt;
	QStringList receipt = simplifyReceipt(aReceipt);
	makeLexemeReceipt(receipt, lexemeReceipt);

	bool printing = printReceipt(lexemeReceipt);

	if (mPrintingMode == EPrintingModes::Glue)
	{
		return printing;
	}

	bool processing = (printing && !aProcessing) || receiptProcessing();

	if (printing && aProcessing && (mPrintingMode == EPrintingModes::Continuous))
	{
		if (getConfigParameter(CHardware::Printer::PresenterEnable).toBool())
		{
			push();
		}
		else if (getConfigParameter(CHardware::Printer::RetractorEnable).toBool())
		{
			retract();
		}
	}

	return printing && processing;
}

//--------------------------------------------------------------------------------
template <class T>
bool PrinterBase<T>::printReceipt(const Tags::TLexemeReceipt & aLexemeReceipt)
{
	bool result = true;

	foreach(auto lexemeCollection, aLexemeReceipt)
	{
		foreach(auto lexemes, lexemeCollection)
		{
			QVariant line;

			foreach(auto lexeme, lexemes)
			{
				Tags::TTypes specialTags = CPrinter::SpecialTags & lexeme.tags;

				if (!specialTags.isEmpty())
				{
					if (!line.isNull())
					{
						if (!printLine(line))
						{
							result = false;
						}

						line.clear();
					}

					if (!execSpecialTag(lexeme))
					{
						result = false;
					}
				}
				else
				{
					execTags(lexeme, line);
				}
			}

			mLineTags.clear();

			foreach(auto lexeme, lexemes)
			{
				mLineTags += lexeme.tags;
			}

			bool containsDH = mLineTags.contains(Tags::Type::DoubleHeight) && mTagEngine->contains(Tags::Type::DoubleHeight);
			mActualStringCount += containsDH ? 2 : 1;

			if (!printLine(line))
			{
				if (!isDeviceReady(true))
				{
					toLog(LogLevel::Error, mDeviceName + ": Printer is not ready for printing to continue");
					return false;
				}

				result = printLine(line);
			}
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
bool PrinterBase<T>::receiptProcessing()
{
	bool needCutting =  getConfigParameter(CHardware::Printer::NeedCutting).toBool();
	bool needPresenting =
		!getConfigParameter(CHardware::Printer::Commands::Presentation).toByteArray().isEmpty() &&
		(getConfigParameter(CHardware::Printer::Settings::Loop) == CHardwareSDK::Values::Use) &&
		 getConfigParameter(CHardware::Printer::Settings::PresentationLength).toInt();

	bool feeding = feed();
	bool cutting = !needCutting || cut();
	bool presenting = !needPresenting || present();

	return feeding && cutting && presenting;
}

//--------------------------------------------------------------------------------
template <class T>
bool PrinterBase<T>::feed()
{
	int amount = getConfigParameter(CHardware::Printer::FeedingAmount).toInt();
	bool result = true;

	for (int i = 0; i < amount; ++i)
	{
		if (!printLine(" "))
		{
			result = false;
		}
	}

	if (!result)
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to feed paper");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
void PrinterBase<T>::adjustToLineSize(Tags::TLexemesBuffer & aTagLexemes, Tags::TLexemesCollection & aLexemesCollection)
{
	if (!mLineSize)
	{
		aLexemesCollection.append(aTagLexemes);

		return;
	}

	int linesize = 0;
	int index = 0;
	int rest = 0;

	for (int i = 0; i < aTagLexemes.size(); ++i)
	{
		if (aTagLexemes[i].tags.contains(Tags::Type::HR))
		{
			bool OK;
			int size = aTagLexemes[i].data.toInt(&OK);

			if (!OK)
			{
				size = mLineSize ? mLineSize : CPrinters::DefaultHRSize;
			}

			aTagLexemes[i].data = QString("-").repeated(size);
		}

		if (aTagLexemes[i].tags.contains(Tags::Type::Image) ||
			aTagLexemes[i].tags.contains(Tags::Type::BarCode))
		{
			aLexemesCollection.append(aTagLexemes);

			continue;
		}

		if (aLexemesCollection.size() == index)
		{
			aLexemesCollection.append(Tags::TLexemesBuffer());
		}

		int factor = 1 + int(aTagLexemes[i].tags.contains(Tags::Type::DoubleWidth) && mTagEngine->data().keys().contains(Tags::Type::DoubleWidth));
		linesize += aTagLexemes[i].data.size() * factor;
		aLexemesCollection[index].append(aTagLexemes[i]);

		do
		{
			rest = qCeil(double(linesize - mLineSize) / factor);

			if (rest > 0)
			{
				if (aLexemesCollection.size() == ++index)
				{
					aLexemesCollection.append(Tags::TLexemesBuffer());
				}

				aLexemesCollection[index].append(aLexemesCollection[index - 1].last());
				aLexemesCollection[index][0].data = aLexemesCollection[index][0].data.right(rest);
				aLexemesCollection[index - 1].last().data.chop(rest);

				linesize = rest * factor;
			}
		}
		while((rest > 0));
	}
}

//--------------------------------------------------------------------------------
template <class T>
void PrinterBase<T>::execTags(Tags::SLexeme & aTagLexeme, QVariant & aLine)
{
	QString data = aTagLexeme.data;

	foreach(const Tags::TTypes types, mTagEngine->groupsTypesByPrefix(aTagLexeme.tags))
	{
		QByteArray openTag  = mTagEngine->getTag(types, Tags::Direction::Open);
		QByteArray closeTag = mTagEngine->getTag(types, Tags::Direction::Close);
		data = openTag + data + closeTag;
	}

	aLine = aLine.toString() + data;
}

//--------------------------------------------------------------------------------
template <class T>
bool PrinterBase<T>::execSpecialTag(const Tags::SLexeme & aTagLexeme)
{
	if (aTagLexeme.tags.contains(Tags::Type::Image))
	{
		QImage image;

		if (image.loadFromData(QByteArray::fromBase64(aTagLexeme.data.toLatin1())) && !image.isNull())
		{
			for (int i = 0; i < image.width(); ++i)
			{
				for (int j = 0; j < image.height(); ++j)
				{
					if (!qAlpha(image.pixel(i, j)))
					{
						image.setPixel(i, j, CPrinters::White);
					}
				}
			}

			image = image.convertToFormat(QImage::Format_Mono);
			printImage(image, aTagLexeme.tags);
		}
	}
	else if (aTagLexeme.tags.contains(Tags::Type::BarCode))
	{
		return printBarcode(aTagLexeme.data);
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
void PrinterBase<T>::separate(QStringList & aReceipt) const
{
	QStringList receipt;

	foreach (QString line, aReceipt)
	{
		QString resultLine = line.replace(ASCII::CR, "");

		while (resultLine.indexOf(ASCII::TAB) != -1)
		{
			int tPoint = resultLine.indexOf(ASCII::TAB);
			int numberOfSpaces = qCeil(tPoint / (double)CPrinters::SpacesInTAB) * CPrinters::SpacesInTAB - tPoint;

			if (tPoint % CPrinters::SpacesInTAB == 0)
			{
				numberOfSpaces = CPrinters::SpacesInTAB;
			}

			resultLine = resultLine.replace(tPoint, 1, QString(ASCII::Space).repeated(numberOfSpaces));
		}

		if (resultLine.indexOf(ASCII::LF) != -1)
		{
			receipt.append(resultLine.split(ASCII::LF));
		}
		else
		{
			receipt.append(resultLine);
		}
	}

	aReceipt = receipt;
}

//--------------------------------------------------------------------------------
template <class T>
bool PrinterBase<T>::canCheckReady(bool aOnline)
{
	bool notConnected = !mConnected && (!mOperatorPresence || !aOnline);

	return (mInitialized != ERequestStatus::InProcess) && !notConnected;
}

//---------------------------------------------------------------------------
template <class T>
bool PrinterBase<T>::isDeviceReady(bool aOnline)
{
	return canCheckReady(aOnline) && isPossible(aOnline);
}

//---------------------------------------------------------------------------
template <class T>
bool PrinterBase<T>::isPossible(bool aOnline, QVariant aCommand)
{
	if (mConnected && (mInitialized == ERequestStatus::Fail))
	{
		MutexLocker locker(&mResourceMutex);

		if (mUnnecessaryErrors[aCommand.toInt()].isEmpty())
		{
			return false;
		}
	}

	if (aOnline)
	{
		if (!checkConnectionAbility())
		{
			return false;
		}

		TStatusCodes statusCodes;
		mOperatorPresence ? onPoll() : doPoll(statusCodes);
	}

	MutexLocker locker(&mResourceMutex);

	TStatusCodes errorCodes = mStatusCollection.value(EWarningLevel::Error);

	if (aCommand.type() == QVariant::Int)
	{
		errorCodes -= mUnnecessaryErrors[aCommand.toInt()];
	}

	return errorCodes.isEmpty();
}

//---------------------------------------------------------------------------
template <class T>
void PrinterBase<T>::cleanStatusCodes(TStatusCodes & aStatusCodes)
{
	if (aStatusCodes.contains(Error::PrinterFRNotAvailable))
	{
		TStatusCodes availableErrors = mStatusCodesSpecification.dynamicCast<PrinterStatusCode::CSpecifications>()->getAvailableErrors();
		aStatusCodes -= availableErrors;
	}

	if (containsConfigParameter(CHardware::Printer::Settings::PaperJamSensor) && (getConfigParameter(CHardware::Printer::Settings::PaperJamSensor) == CHardwareSDK::Values::NotUse))
	{
		aStatusCodes.remove(Error::PaperJam);
	}

	if (containsConfigParameter(CHardware::Printer::Settings::RemotePaperSensor) && (getConfigParameter(CHardware::Printer::Settings::RemotePaperSensor) == CHardwareSDK::Values::NotUse))
	{
		aStatusCodes.remove(Warning::PaperNearEnd);
	}

	if (aStatusCodes.isEmpty())
	{
		aStatusCodes.insert(DeviceStatusCode::OK::OK);
	}

	if (getConfigParameter(CHardwareSDK::Printer::OFDNotSentError, false).toBool())
	{
		bool error = getConfigParameter(CHardwareSDK::Printer::BlockTerminalOnError, true).toBool();
		aStatusCodes.insert(error ? Error::OFDNotSent : Warning::OFDNotSent);
	}

	T::cleanStatusCodes(aStatusCodes);

	if (aStatusCodes.contains(Error::PaperEnd))
	{
		aStatusCodes.remove(Warning::PaperNearEnd);
		aStatusCodes.remove(Warning::PaperEndVirtual);
	}

	if (aStatusCodes.contains(Error::Temperature))
	{
		aStatusCodes.remove(Error::PrintingHead);
	}
}

//--------------------------------------------------------------------------------
template <class T>
bool PrinterBase<T>::clearDispenser(const QString & aCondition)
{
	if (mClearingDispenserTurnOff)
	{
		mClearingDispenserTurnOff = mPrintingMode == EPrintingModes::Glue;

		return true;
	}

	bool presenterEnable = getConfigParameter(CHardware::Printer::PresenterEnable).toBool();
	bool retractorEnable = getConfigParameter(CHardware::Printer::RetractorEnable).toBool();

	if (!containsConfigParameter(aCondition) && (presenterEnable || retractorEnable))
	{
		toLog(LogLevel::Warning, mDeviceName + ": Unknown condition for clearing dispenser: " + aCondition);
		return true;
	}

	if (getConfigParameter(aCondition) == CHardware::Printer::Values::Retract)
	{
		if (retractorEnable && !retract())
		{
			toLog(LogLevel::Error, mDeviceName + ": Failed to retract the document");
			return false;
		}
	}
	else if (getConfigParameter(aCondition) == CHardware::Printer::Values::Push)
	{
		if (presenterEnable && !push())
		{
			toLog(LogLevel::Error, mDeviceName + ": Failed to push the document");
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
void PrinterBase<T>::postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection)
{
	if (aNewStatusCollection.contains(OK::PaperInPresenter))
	{
		QDateTime current = QDateTime::currentDateTime();
		int timeout = getConfigParameter(CHardware::Printer::Settings::LeftReceiptTimeout).toInt();

		if ((mPaperInPresenter.secsTo(current) > timeout) && !clearDispenser(CHardware::Printer::Settings::NotTakenReceipt))
		{
			mPaperInPresenter = current.addMSecs((CPrinters::ClearingPresenterRepeatTimeout - timeout) * 1000 - mPollingInterval);
		}
	}
	else
	{
		mPaperInPresenter = QDateTime::currentDateTime();
	}

	T::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

//--------------------------------------------------------------------------------
