/* @file Принтеры семейства Star. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtEndian>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Project
#include "StarPrinters.h"
#include "StarPrinterData.h"
#include "ModelData.h"

using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
// Модели данной реализации.
namespace CSTAR { inline QStringList getCommonModels()
{
	using namespace Models;
	
	return QStringList()
		<< TUP542
		<< TUP942
		<< TSP613
		<< TSP643
		<< TSP651
		<< TSP654
		<< TSP743
		<< TSP743II
		<< TSP847
		<< TSP847II
		<< TSP828L
		<< TSP1043
		<< FVP10
		<< Unknown;
}}

//--------------------------------------------------------------------------------
StarPrinter::StarPrinter()
{
	// данные порта
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR38400);   // preferable for work
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);    // default
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR4800);

	mPortParameters[EParameters::Parity].append(EParity::No);    // default
	mPortParameters[EParameters::Parity].append(EParity::Odd);
	mPortParameters[EParameters::Parity].append(EParity::Even);

	// теги
	mTagEngine = Tags::PEngine(new CSTAR::TagEngine());

	// данные устройства
	mDeviceName = CSTAR::Models::Unknown;
	mModels = CSTAR::getCommonModels();
	setConfigParameter(CHardware::Printer::FeedingAmount, 4);
	setConfigParameter(CHardware::Printer::Commands::Cutting, "\x1B\x64\x30");
	setConfigParameter(CHardware::Printer::Commands::Pushing, CSTAR::Commands::Reset);
	setConfigParameter(CHardware::Printer::Commands::Retraction, "\x1B\x16\x30\x30");
	mIOMessageLogging = ELoggingType::Write;
	mNeedPaperTakeOut = false;

	for (int i = 0; i < CSTAR::MemorySwitches::Amount; ++i)
	{
		mMemorySwitches.append(CSTAR::SMemorySwitch());
	}

	mFullPolling = false;
}

//--------------------------------------------------------------------------------
QStringList StarPrinter::getModelList()
{
	return CSTAR::getCommonModels();
}

//--------------------------------------------------------------------------------
void StarPrinter::setLog(ILog * aLog)
{
	TSerialPrinterBase::setLog(aLog);

	mMemorySwitchUtils.setLog(aLog);
}

//--------------------------------------------------------------------------------
bool StarPrinter::readIdentificationAnswer(QByteArray & aAnswer)
{
	aAnswer.clear();

	QTime clockTimer;
	clockTimer.restart();

	do
	{
		QByteArray data;

		if (!mIOPort->read(data, 10))
		{
			return false;
		}

		aAnswer.append(data);
	}
	while ((clockTimer.elapsed() < CSTAR::Timeouts::Default) && !aAnswer.endsWith(ASCII::NUL));

	toLog(LogLevel::Normal, QString("%1: << {%2}").arg(mDeviceName).arg(aAnswer.toHex().data()));

	return true;
}

//--------------------------------------------------------------------------------
bool StarPrinter::readMSWAnswer(QByteArray & aAnswer)
{
	aAnswer.clear();

	int attempt = 0;
	bool uniteStatus = false;

	do
	{
		if (attempt)
		{
			QByteArray data;
			QByteArray answer;

			do
			{
				answer.append(data);
			}
			while (mIOPort->read(data, 10) && !data.isEmpty());

			if (!answer.isEmpty())
			{
				SleepHelper::msleep(CSTAR::Timeouts::MSWGettingASB);

				TStatusCodes statusCodes;
				getStatus(statusCodes);
			}
		}

		QTime clockTimer;
		clockTimer.restart();

		do
		{
			QByteArray data;

			if (!mIOPort->read(data, 10))
			{
				return false;
			}

			aAnswer.append(data);
		}
		while ((clockTimer.elapsed() < CSTAR::Timeouts::Default) && (aAnswer.size() < CSTAR::MemorySwitches::AnswerSize));

		toLog(LogLevel::Normal, QString("%1: << {%2}").arg(mDeviceName).arg(aAnswer.toHex().data()));

		uniteStatus = (aAnswer.size() > CSTAR::MemorySwitches::AnswerSize) || !aAnswer.endsWith(ASCII::NUL);
	}
	while (uniteStatus && (++attempt <= CSTAR::MemorySwitches::ReadingAttempts));

	if (uniteStatus && (attempt == CSTAR::MemorySwitches::ReadingAttempts))
	{
		toLog(LogLevel::Error, QString("%1: Failed to get memory switch due to merging another messages from printer").arg(mDeviceName));
		return false;
	}

	if (aAnswer.size() < CSTAR::MemorySwitches::MinAnswerSize)
	{
		toLog(LogLevel::Error, QString("%1: Invalid answer length = %2, need = %3 minimum").arg(mDeviceName).arg(aAnswer.size()).arg(CSTAR::MemorySwitches::MinAnswerSize));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool StarPrinter::reset()
{
	if (!mIOPort->write(CSTAR::Commands::Reset))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to reset printer");
		return false;
	}

	SleepHelper::msleep(CSTAR::Timeouts::Reset);

	if (!mIOPort->write(CSTAR::Commands::SetASB))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to set ASB");
	}

	return true;
}

//--------------------------------------------------------------------------------
bool StarPrinter::isConnected()
{
	if (!initializeRegisters())
	{
		return false;
	}

	namespace WarningLevel = SDK::Driver::EWarningLevel;

	#define STAR_FILTER_MODELS(aCondition) for (auto it = models.begin(); it != models.end();) {it = (aCondition) ? models.erase(it) : (it + 1);}

	CSTAR::Models::TData models = CSTAR::Models::Data.data();
	mMemorySwitches.clear();

	for (int i = 0; i < CSTAR::MemorySwitches::Amount; ++i)
	{
		mMemorySwitches.append(CSTAR::SMemorySwitch());
	}

	simplePoll();

	if (mStatusCollection.contains(DeviceStatusCode::Error::NotAvailable))
	{
		return false;
	}

	mDeviceName = CSTAR::Models::Unknown;

	QVariantMap configuration;
	configuration.insert(CHardware::Port::DeviceModelName, mDeviceName);
	mIOPort->setDeviceConfiguration(configuration);

	bool needPaperTakeOut = mStatusCollection.contains(PrinterStatusCode::Error::NeedPaperTakeOut);

	if (!isPaperInPresenter())
	{
		reset();
	}

	if (isPaperInPresenter() || needPaperTakeOut)
	{
		STAR_FILTER_MODELS(!it->ejector);
		mDeviceName = CSTAR::Models::UnknownEjector;

		if (!needPaperTakeOut && StarPrinter::retract())
		{
			waitEjectorState(false);
		}
	}

	QByteArray answer;
	QString regExpData = QString("([A-Z0-9]+)[V|$]e?r?(%1*\\.?%1+)?").arg("[0-9]");
	QRegExp regExp(regExpData);

	bool loading = !isAutoDetecting();

	if (!mIOPort->write(CSTAR::Commands::GetModelData) || !readIdentificationAnswer(answer) || (regExp.indexIn(answer) == -1))
	{
		QString modelName = getConfigParameter(CHardwareSDK::ModelName).toString();
		bool result = loading && models.contains(modelName);

		if (result)
		{
			mVerified = CSTAR::Models::Data[modelName].verified;
			mDeviceName = modelName;
		}

		return result;
	}

	QStringList data(regExp.capturedTexts());

	QString id = data[1].simplified();
	STAR_FILTER_MODELS(it->deviceId != id);

	if (loading && (getConfigParameter(CHardwareSDK::ModelName).toString() != CSTAR::Models::Unknown))
	{
		STAR_FILTER_MODELS(!it->cutter && (models.size() > 1));
	}

	mMemorySwitchUtils.setModels(models.keys().toSet());

	getMemorySwitches();
	CSTAR::TMemorySwitches memorySwitches(mMemorySwitches);
	mMemorySwitchUtils.setConfiguration(mMemorySwitches);

	configuration.insert(CHardware::AutomaticStatus, CHardwareSDK::Values::Use);
	configuration.insert(CHardware::Printer::VerticalMountMode, CHardwareSDK::Values::NotUse);

	CSTAR::TMemorySwitchTypes memorySwitchTypes = CSTAR::TMemorySwitchTypes()
		//<< ESTARMemorySwitchTypes::ASB
		<< ESTARMemorySwitchTypes::VerticalMountMode;

	if (!mMemorySwitchUtils.update(memorySwitchTypes, memorySwitches, configuration) || !updateMemorySwitches(memorySwitches))
	{
		mInitializationError = true;
	}

	QString modelName = getConfigParameter(CHardwareSDK::ModelName).toString();

	if (loading && models.contains(modelName))
	{
		STAR_FILTER_MODELS(it->ejector != models[modelName].ejector);
	}
	else if (!models.isEmpty() && (std::find_if(models.begin(), models.end(), [&models] (const CSTAR::SModelData aModelData) -> bool
		{ return aModelData.ejector != models.begin()->ejector; }) != models.end()))
	{
		CSTAR::MemorySwitches::CMainSettings mainSettings;
		QVariantMap mainInitConfiguration;

		CSTAR::TMemorySwitchTypes mainMemorySwitchTypes;
		CSTAR::TModels modelNames = models.keys().toSet();

		for (auto it = mainSettings.data().begin(); it != mainSettings.data().end(); ++it)
		{
			if (!(it->models & modelNames).isEmpty())
			{
				mainMemorySwitchTypes << it.key();
			}
		}

		for (auto it = mainSettings.data().begin(); it != mainSettings.data().end(); ++it)
		{
			if ((!mainMemorySwitchTypes.contains(it.key()) && it->models.isEmpty()) || !(it->models & modelNames).isEmpty())
			{
				mainInitConfiguration.unite(it->configuration);
			}
		}

		if (mMemorySwitchUtils.update(mainSettings.data().keys(), memorySwitches, mainInitConfiguration) &&
			updateMemorySwitches(memorySwitches))
		{
			processReceipt(QStringList() << QString(QByteArray("test ").repeated(20)).split(ASCII::Space));

			bool ejectorBusy = waitEjectorState(true);
			STAR_FILTER_MODELS(ejectorBusy != it->ejector);

			retract();
		}
		else
		{
			STAR_FILTER_MODELS(it->ejector);
		}
	}

	mMemorySwitchUtils.setModels(models.keys().toSet());
	mMemorySwitchUtils.setConfiguration(mMemorySwitches);

	mDeviceName = models.isEmpty() ? CSTAR::Models::Unknown : models.begin().key();
	mVerified = CSTAR::Models::Data[mDeviceName].verified;
	mModelCompatibility = mModels.contains(mDeviceName);

	if (data.size() > 2)
	{
		setDeviceParameter(CDeviceData::Firmware, data[2]);

		double firmware = data[2].toDouble();
		double minFirmware = CSTAR::Models::Data[mDeviceName].minFirmware;
		mOldFirmware = minFirmware && firmware && (firmware < minFirmware);
	}

	return true;
}

//--------------------------------------------------------------------------------
bool StarPrinter::setMemorySwitch(int aSwitch, ushort aValue)
{
	if (!mIOPort->write(CSTAR::Commands::setMemorySwitch(char(aSwitch), aValue)))
	{
		return false;
	}

	SleepHelper::msleep(CSTAR::Timeouts::MSWSetting);

	return true;
}

//--------------------------------------------------------------------------------
void StarPrinter::getMemorySwitches()
{
	TStatusCodes statusCodes;

	if (!getStatus(statusCodes) || statusCodes.contains(PrinterStatusCode::Error::NeedPaperTakeOut) ||
		(statusCodes.contains(PrinterStatusCode::OK::PaperInPresenter) && !retract()))
	{
		return;
	}

	for (int i = 0; i < CSTAR::MemorySwitches::Amount; ++i)
	{
		ushort value;
		mMemorySwitches[i].valid = getMemorySwitch(i, value);

		if (mMemorySwitches[i].valid)
		{
			mMemorySwitches[i].value = value;
		}
		else
		{
			toLog(LogLevel::Error, QString("%1: Failed to get memory switch %2").arg(mDeviceName).arg(uchar(i), 1, 16));
		}
	}
}

//--------------------------------------------------------------------------------
bool StarPrinter::getMemorySwitch(int aSwitch, ushort & aValue)
{
	QByteArray request = CSTAR::Commands::getMemorySwitch(char(aSwitch));
	QByteArray answer;

	if (!mIOPort->write(request) || !readMSWAnswer(answer))
	{
		return false;
	}

	if (answer.size() < CSTAR::MemorySwitches::MinAnswerSize)
	{
		toLog(LogLevel::Error, QString("%1: Invalid answer length = %2, need = %3 minimum")
			.arg(mDeviceName).arg(answer.size()).arg(CSTAR::MemorySwitches::MinAnswerSize));
		return false;
	}

	char requestNumber = request[2];
	char answerNumber  = answer[2];

	if (requestNumber != answerNumber)
	{
		toLog(LogLevel::Error, QString("%1: Invalid switch number = %2, need = %3")
			.arg(mDeviceName).arg(ProtocolUtils::toHexLog(answerNumber)).arg(ProtocolUtils::toHexLog(requestNumber)));
		return false;
	}

	bool ok;
	QByteArray value = answer.mid(4, 4);
	ushort result = value.toUShort(&ok, 16);

	if (!ok)
	{
		toLog(LogLevel::Error, QString("%1: Invalid switch value = %2").arg(mDeviceName).arg(value.toHex().data()));
		return false;
	}

	aValue = result;

	return true;
}

//--------------------------------------------------------------------------------
bool StarPrinter::updateMemorySwitches(const CSTAR::TMemorySwitches & aMemorySwitches)
{
	if (aMemorySwitches != mMemorySwitches)
	{
		for (int i = 0; i < CSTAR::MemorySwitches::MaxNumber; ++i)
		{
			if ((aMemorySwitches[i].value != mMemorySwitches[i].value) && !setMemorySwitch(i, aMemorySwitches[i].value))
			{
				return false;
			}

			mMemorySwitches[i].value = aMemorySwitches[i].value;
		}

		if (!mIOPort->write(CSTAR::Commands::WriteMemorySwitches))
		{
			return false;
		}

		SleepHelper::msleep(CSTAR::Timeouts::MSWWriting);

		if (!initializeRegisters())
		{
			return false;
		}

		getMemorySwitches();
	}

	for (int i = 0; i < CSTAR::MemorySwitches::Amount; ++i)
	{
		mMemorySwitches[i].valid = aMemorySwitches[i].value == mMemorySwitches[i].value;

		if (!mMemorySwitches[i].valid)
		{
			toLog(LogLevel::Error, QString("%1: Failed to set memory switch %2 = %3, need %4")
				.arg(mDeviceName).arg(uchar(i), 1, 16)
				.arg(ProtocolUtils::toHexLog(mMemorySwitches[i].value))
				.arg(ProtocolUtils::toHexLog(aMemorySwitches[i].value)));
		}
	}

	return !std::count_if(mMemorySwitches.begin(), mMemorySwitches.end(), [&] (const CSTAR::SMemorySwitch & aMemorySwitch) -> bool { return !aMemorySwitch.valid; });
}

//--------------------------------------------------------------------------------
bool StarPrinter::initializeRegisters()
{
	return mIOPort->write(QByteArray(CSTAR::Commands::Initilize) + CSTAR::Commands::SetASB);
}

//--------------------------------------------------------------------------------
bool StarPrinter::updateParameters()
{
	mFullPolling = false;
	simplePoll();

	setConfigParameter(CHardware::Codepage, CodecByName.key(mCodec));

	CSTAR::SModelData data = CSTAR::Models::Data[mDeviceName];
	int lineSpacing = getConfigParameter(CHardware::Printer::Settings::LineSpacing).toInt();
	int feeding = (lineSpacing == 3) ? data.feeding3 : data.feeding4;
	setConfigParameter(CHardware::Printer::FeedingAmount, feeding);

	/*
	// логика отключена, т.к. вместо ASB (единственный параметр) устанавливаем регистр
	QVariantMap configuration;
	configuration.insert(CHardware::AutomaticStatus, CHardwareSDK::Values::Use);

	CSTAR::TMemorySwitchTypes memorySwitchTypes;
	memorySwitchTypes.append(ESTARMemorySwitchTypes::ASB);

	getMemorySwitches();
	CSTAR::TMemorySwitches memorySwitches(mMemorySwitches);

	if (!mMemorySwitchUtils.update(memorySwitchTypes, memorySwitches, configuration) || !updateMemorySwitches(memorySwitches))
	{
		return false;
	}
	*/
	mMemorySwitchUtils.setConfiguration(getDeviceConfiguration());
	CSTAR::TMemorySwitches updatedMemorySwitches(mMemorySwitches);

	mMemorySwitchUtils.setModels(CSTAR::TModels() << mDeviceName);

	if (!mMemorySwitchUtils.update(updatedMemorySwitches) || !updateMemorySwitches(updatedMemorySwitches))
	{
		return false;
	}

	return initializeRegisters();
}

//--------------------------------------------------------------------------------
bool StarPrinter::printReceipt(const Tags::TLexemeReceipt & aLexemeReceipt)
{
	if (!initializeRegisters())
	{
		return false;
	}

	mStartPrinting = QDateTime::currentDateTime();

	bool result = SerialPrinterBase::printReceipt(aLexemeReceipt);

	waitForPrintingEnd();

	return result;
}

//--------------------------------------------------------------------------------
bool StarPrinter::waitForPrintingEnd()
{
	mIOPort->write(CSTAR::Commands::WaitForPrintingEnd);

	QTime clockTimer;
	clockTimer.restart();

	QByteArray answer;
	bool result = false;

	do
	{
		QByteArray data;

		if (!mIOPort->read(data, 10))
		{
			return false;
		}

		answer.append(data);
		result = answer.size() >= 8;
	}
	while ((clockTimer.elapsed() < CSTAR::Timeouts::ReceiptProcessing) && !result);

	toLog(LogLevel::Normal, QString("%1: << {%2}").arg(mDeviceName).arg(answer.toHex().data()));

	return result;
}

//---------------------------------------------------------------------------
void StarPrinter::cleanStatusCodes(TStatusCodes & aStatusCodes)
{
	if (aStatusCodes.contains(PrinterStatusCode::OK::PaperInPresenter))
	{
		TStatusCollection lastStatusCollection = mStatusCollectionHistory.lastValue();

		if (isPaperInPresenter() && mNeedPaperTakeOut)
		{
			aStatusCodes.remove(PrinterStatusCode::OK::PaperInPresenter);
			aStatusCodes.insert(PrinterStatusCode::Error::NeedPaperTakeOut);
		}
	}

	if (!CSTAR::Models::Data[mDeviceName].cutter)
	{
		aStatusCodes.remove(PrinterStatusCode::Error::Cutter);
	}

	if (!CSTAR::Models::Data[mDeviceName].headThermistor)
	{
		aStatusCodes.remove(PrinterStatusCode::Error::Temperature);
	}

	if (!CSTAR::Models::Data[mDeviceName].innerPaperEndSensor)
	{
		aStatusCodes.remove(PrinterStatusCode::Warning::PaperNearEnd);
	}

	if (!CSTAR::Models::Data[mDeviceName].voltageSensor)
	{
		aStatusCodes.remove(DeviceStatusCode::Error::PowerSupply);
	}

	if (!CSTAR::Models::Data[mDeviceName].ejector && (mDeviceName != CSTAR::Models::Unknown))
	{
		aStatusCodes.remove(DeviceStatusCode::Error::PowerSupply);
	}

	SerialPrinterBase::cleanStatusCodes(aStatusCodes);
}

//--------------------------------------------------------------------------------
int StarPrinter::shiftData(const QByteArray aAnswer, int aByteNumber, int aSource, int aShift, int aDigits) const
{
	return int(aAnswer[aByteNumber] >> aShift) & (QByteArray::number(1).repeated(aDigits).toInt(0, 2) << (aSource - aShift));
}

//--------------------------------------------------------------------------------
bool StarPrinter::readASBAnswer(QByteArray & aAnswer, int & aLength)
{
	QTime clockTimer;
	clockTimer.restart();

	aLength = 0;
	QByteArray data;
	QByteArray result;

	do
	{
		data.clear();

		if (!mIOPort->read(data, 10))
		{
			return false;
		}

		result.append(data);

		if (!result.isEmpty())
		{
			aLength = shiftData(result, 0, 1, 1, 3) | shiftData(result, 0, 5, 2, 1);
		}
	}
	while ((clockTimer.elapsed() < CSTAR::Timeouts::Status) && (!aLength || (result.size() % aLength) || !data.isEmpty()));

	aAnswer = result;
	QString log = QString("%1: << {%2}").arg(mDeviceName).arg(result.toHex().data());

	if ((result.size() > aLength) && aLength)
	{
		int index = aAnswer.lastIndexOf(aAnswer.left(2));

		if ((index + aLength) > aAnswer.size())
		{
			index -= aLength;
		}

		aAnswer = aAnswer.mid(index, aLength);
		log += QString(" -> {%1}").arg(aAnswer.toHex().data());
	}

	toLog(LogLevel::Normal, log);

	return aAnswer.isEmpty() || (aAnswer.size() >= 2);
}

//--------------------------------------------------------------------------------
bool StarPrinter::getStatus(TStatusCodes & aStatusCodes)
{
	QByteArray answer;
	int length = 0;

	if (mPollingActive && mFullPolling && !mStatusCollection.contains(PrinterStatusCode::Error::PaperEnd))
	{
		if (!readASBAnswer(answer, length))
		{
			return false;
		}

		if (answer.isEmpty())
		{
			if ((mStartPrinting.msecsTo(QDateTime::currentDateTime()) < CSTAR::Timeouts::Printing) || isPaperInPresenter())
			{
				aStatusCodes = getStatusCodes(mStatusCollection);

				return true;
			}

			if (!mIOPort->write(CSTAR::Commands::ETBMark) || !readASBAnswer(answer, length) || answer.isEmpty())
			{
				if (!isPaperInPresenter())
				{
					return false;
				}

				aStatusCodes = getStatusCodes(mStatusCollection);

				return true;
			}
		}
	}
	else
	{
		if (!mIOPort->write(CSTAR::Commands::ASBStatus))
		{
			return false;
		}

		mFullPolling = readASBAnswer(answer, length) && !answer.isEmpty();

		if (!mFullPolling)
		{
			return false;
		}
	}

	if (!mOperatorPresence && !CSTAR::Models::Data[mDeviceName].cutter)
	{
		aStatusCodes.insert(DeviceStatusCode::Error::Firmware);
	}

	if (length < answer.size())
	{
		aStatusCodes.insert(DeviceStatusCode::Error::Unknown);
	}
	else if (length > answer.size())
	{
		aStatusCodes.insert(DeviceStatusCode::Warning::UnknownDataExchange);
	}

	int version = shiftData(answer, 1, 1, 1, 3) | shiftData(answer, 1, 5, 1, 1);

	if (version < CSTAR::MinVersionNumber)
	{
		aStatusCodes.insert(DeviceStatusCode::Warning::Firmware);
	}

	for (int i = 2; i < answer.size(); ++i)
	{
		for (auto it = CSTAR::ASBStatus[i - 1].begin(); it != CSTAR::ASBStatus[i - 1].end(); ++it)
		{
			if (answer[i] & (1 << it.key()))
			{
				aStatusCodes.insert(it.value());
			}
		}
	}

	if ((CSTAR::Models::Data[mDeviceName].ejector || (mDeviceName == CSTAR::Models::Unknown)) && (answer.size() >= 9) && (answer[8] & CSTAR::PresenterStatusMask))
	{
		aStatusCodes.insert(PrinterStatusCode::OK::PaperInPresenter);
	}
	else
	{
		mNeedPaperTakeOut = false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool StarPrinter::waitEjectorState(bool aBusy)
{
	if (mNeedPaperTakeOut)
	{
		return false;
	}

	//TODO: добавить условие аварийного выхода при ошибке
	TStatusCodes statusCodes;
	auto condition = [&] () -> bool { return !statusCodes.isEmpty() && !statusCodes.contains(DeviceStatusCode::Error::NotAvailable) &&
		(aBusy == statusCodes.contains(PrinterStatusCode::OK::PaperInPresenter)); };
	bool result = PollingExpector().wait<void>(std::bind(&StarPrinter::doPoll, this, std::ref(statusCodes)), condition, CSTAR::EjectorWaiting);

	if (!aBusy && !result)
	{
		mNeedPaperTakeOut = true;
	}

	if (mInitialized != ERequestStatus::InProcess)
	{
		processStatusCodes(statusCodes);
	}

	return result;
}

//---------------------------------------------------------------------------
bool StarPrinter::isPaperInPresenter()
{
	return mStatusCollection.contains(PrinterStatusCode::OK::PaperInPresenter) ||
	       mStatusCollection.contains(PrinterStatusCode::Error::NeedPaperTakeOut);
}

//--------------------------------------------------------------------------------
bool StarPrinter::printImage(const QImage & aImage, const Tags::TTypes & /*aTags*/)
{
	int width = aImage.width();
	int height = aImage.height();
	int lines = qCeil(height / double(CSTAR::ImageHeight));
	int widthInBytes = qCeil(width / 8.0);

	for (int i = 0; i < lines; ++i)
	{
		QList<QByteArray> lineData;

		for (int j = 0; j < CSTAR::ImageHeight; ++j)
		{
			int index = i * CSTAR::ImageHeight + j;

			if (index < height)
			{
				lineData << QByteArray::fromRawData((const char *)aImage.scanLine(index), widthInBytes);
			}
			else
			{
				lineData << QByteArray(widthInBytes, ASCII::NUL);
			}
		}

		QByteArray request(CSTAR::Commands::PrintImage);
		request.append(ProtocolUtils::getBufferFromString(QString("%1").arg(qToBigEndian(ushort(width)), 4, 16, QChar(ASCII::Zero))));

		for (int j = 0; j < width; ++j)
		{
			QByteArray data(CSTAR::LineHeight, ASCII::NUL);
			char mask = 1 << (7 - (j % 8));

			for (int k = 0; k < CSTAR::ImageHeight; ++k)
			{
				if (lineData[k][j / 8] & mask)
				{
					char dataMask = 1 << (7 - (k % 8));
					data[k / 8] = data[k / 8] | dataMask;
				}
			}

			request.append(data);
		}

		if (i != (lines - 1))
		{
			request.append(CSTAR::Commands::FeedImageLine);
		}

		if (!mIOPort->write(request))
		{
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
