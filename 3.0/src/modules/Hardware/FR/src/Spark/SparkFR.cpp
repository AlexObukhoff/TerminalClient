/* @file ФР СПАРК. */

// STL
#include <cmath>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Modules
#include "PaymentProcessor/PrintConstants.h"
#include "Hardware/Common/PollingExpector.h"

// Project
#include "SparkFR.h"
#include "AdaptiveFiscalLogic.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
SparkFR::SparkFR()
{
	// теги
	mTagEngine = Tags::PEngine(new CSparkFR::TagEngine());

	// кодек
	mCodec = CodecByName[CHardware::Codepages::SPARK];

	// данные устройства
	mDeviceName = CSparkFR::Models::Default;
	mDocumentState = CSparkFR::DocumentStates::Closed;
	mLineFeed = false;
	mSupportedModels = getModelList();
	mSessionOpeningDT = CSparkFR::ClosedSession;
	mZReports = 0;
	mCheckStatus = true;
	mCanProcessZBuffer = true;
	//setConfigParameter(CHardware::CanOnline, true);    //TODO: раскомментить после поддержки онлайновой реализации

	setConfigParameter(CHardware::Printer::RetractorEnable, true);

	using namespace SDK::Driver::IOPort::COM;

	// данные порта
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);     // default
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR4800);     // default after resetting to zero
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR57600);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR38400);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);

	mPortParameters[EParameters::Parity].append(EParity::No);
	mPortParameters[EParameters::Parity].append(EParity::Even);
}

//--------------------------------------------------------------------------------
QDateTime SparkFR::getDateTime()
{
	TKKMInfoData data;

	if (getKKMData(data))
	{
		return parseDateTime(data);
	}

	return QDateTime();
}

//--------------------------------------------------------------------------------
template<class T>
T SparkFR::fromBCD(const QByteArray & aData)
{
	T result = 0;
	int size = aData.size();

	for (int i = 0; i < qMin(int(sizeof(T)), qCeil(size / 2)); ++i)
	{
		int delta = size - 2 * (i + 1);
		result += (T(aData[delta + 0] - ASCII::Zero) << (4 * (2 * i + 1))) +
		          (T(aData[delta + 1] - ASCII::Zero) << (4 * (2 * i + 0)));
	}

	return qToBigEndian(result);
}

//--------------------------------------------------------------------------------
template <>
char SparkFR::fromBCD(const QByteArray & aData)
{
	char result = aData[0] - ASCII::Zero;

	if (aData.size() > 1)
	{
		result <<= 4;
		result += aData[1] - ASCII::Zero;
	}

	return result;
}

//--------------------------------------------------------------------------------
char SparkFR::fromBCD(char aData)
{
	return fromBCD<char>(QByteArray(1, aData));
}

//--------------------------------------------------------------------------------
bool SparkFR::checkFlag(const QByteArray aFlagBuffer, int aNumber)
{
	auto dataIt = std::find_if(mSystemFlags.begin(), mSystemFlags.end(), [&](const CSparkFR::SystemFlags::SData & aData) -> bool { return aData.number == aNumber; });

	if (dataIt == mSystemFlags.end())
	{
		toLog(LogLevel::Error, QString("%1: Failed to find system flag %2 in device one").arg(mDeviceName).arg(aNumber));
		return false;
	}

	int dataSize = aFlagBuffer.size();
	int size = aNumber * 2;

	if (dataSize < size)
	{
		toLog(LogLevel::Error, QString("%1: Failed to check system flag %2 (%3) due to size = %4, need %5")
			.arg(mDeviceName).arg(aNumber).arg(dataIt->name).arg(dataSize).arg(size));
		return false;
	}

	bool OK;
	QByteArray flagData = aFlagBuffer.mid((aNumber - 1) * 2, 2);
	char flag = char(flagData.toInt(&OK, 16));

	if (!OK)
	{
		toLog(LogLevel::Error, QString("%1: Failed to check system flag %2 (%3) due to wrong flag data = %4 = 0x%5")
			.arg(mDeviceName).arg(aNumber).arg(dataIt->name).arg(flagData.data()).arg(flagData.toHex().data()));
		return false;
	}

	char newFlag = ProtocolUtils::mask(flag, dataIt->mask);

	if (flag != newFlag)
	{
		QByteArray commandData;
		commandData += QString("%1").arg(aNumber, 2, 10, QChar(ASCII::Zero)).right(2) +
		               QString("%1").arg(newFlag, 2, 16, QChar(ASCII::Zero)).right(2);
		QByteArray answer;

		if (!processCommand(CSparkFR::Commands::SetFlag, commandData, &answer))
		{
			toLog(LogLevel::Error, QString("%1: Failed to set system flag %2 (%3)").arg(mDeviceName).arg(aNumber).arg(dataIt->name));
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool SparkFR::getSystemFlags(QByteArray & aData, TTaxData * aTaxData)
{
	QByteArray answer;

	if (!processCommand(CSparkFR::Commands::TaxesAndFlags, &answer))
	{
		return false;
	}

	QList<QByteArray> data = answer.split(CSparkFR::Separator);

	if (data.size() < 5)
	{
		toLog(LogLevel::Error, QString("%1: Failed to check system flags due to size of data list = %2, need %3").arg(mDeviceName).arg(data.size()).arg(5));
		return false;
	}

	if (aTaxData)
	{
		*aTaxData = data.mid(0, 4);
	}

	aData = data[4];

	return true;
}

//--------------------------------------------------------------------------------
bool SparkFR::updateParameters()
{
	QByteArray flagData;
	TTaxData taxData;

	if (!getSystemFlags(flagData, &taxData))
	{
		return false;
	}

	TTaxes taxes;

	for (int i = 0; i < CSparkFR::TaxRateCount; ++i)
	{
		bool OK;
		TVAT VAT = TVAT(taxData[i].toInt(&OK));
		taxes.append(OK ? VAT : -1);
	}

	mTaxes = getActualVATs().toList();
	qSort(mTaxes);

	TVATs absentVATs = getActualVATs() - taxes.toSet();

	if (!absentVATs.isEmpty())
	{
		QByteArray commandData;

		for (int i = 0; i < CSparkFR::TaxRateCount; ++i)
		{
			commandData += (i < mTaxes.size()) ? QString("%1").arg(mTaxes[i], 2, 10, QChar(ASCII::Zero)).toLatin1() : CSparkFR::NoTaxes;
		}

		if (!processCommand(CSparkFR::Commands::SetTaxes, commandData) || !processCommand(CSparkFR::Commands::AcceptTaxes))
		{
			toLog(LogLevel::Error, mDeviceName + ": Failed to set taxes");
			return false;
		}
	}

	foreach(auto data, mSystemFlags)
	{
		if (!checkFlag(flagData, data.number))
		{
			return false;
		}
	}

	if (isAdaptiveFCCreation())
	{
		foreach (int parameter, CSparkFR::TextProperties::Numbers)
		{
			QByteArray answer;
			QByteArray commandData = QByteArray(1, char(parameter) + ASCII::Zero);

			if (!processCommand(CSparkFR::Commands::GetTextPropertyName, commandData, &answer))
			{
				return false;
			}

			QByteArray prefix = QByteArray(3, CSparkFR::TextProperties::Prefix) + QByteArray(17, ASCII::Space);
			commandData = QString("%1").arg(parameter, 2, 10, QChar(ASCII::Zero)).right(2).toLatin1() + prefix;

			if ((answer.mid(3) != prefix) && !processCommand(CSparkFR::Commands::SetTextPropertyName, commandData))
			{
				return false;
			}
		}
	}

	processDeviceData();

	return true;
}

//--------------------------------------------------------------------------------
bool SparkFR::isAdaptiveFCCreation()
{
	return getConfigParameter(CHardware::FR::FiscalChequeCreation).toString() == CHardware::FR::Values::Adaptive;
}

//--------------------------------------------------------------------------------
bool SparkFR::isConnected()
{
	QByteArray answer;
	TResult result = processCommand(CSparkFR::Commands::GetFWVersion, &answer);

	if (CommandResult::ProtocolErrors.contains(result))
	{
		return false;
	}

	QString answerData = mCodec->toUnicode(answer);
	QRegExp regExp(CSparkFR::Models::RegExpData);
	CSparkFR::Models::SData data;

	if (regExp.indexIn(answerData) != -1)
	{
		QStringList capturedData = regExp.capturedTexts();
		data = CSparkFR::Models::Data[capturedData[1].toInt()];
	}

	mDeviceName  = data.name;
	mVerified    = data.verified;
	mLineSize    = data.lineSize;
	mSystemFlags = data.systemFlags;

	return true;
}

//--------------------------------------------------------------------------------
QStringList SparkFR::getModelList()
{
	QStringList result;

	foreach (auto data, CSparkFR::Models::CData().data().values())
	{
		result << data.name;
	}

	return result;
}

//--------------------------------------------------------------------------------
TResult SparkFR::processCommand(const QByteArray & aCommand, QByteArray * aAnswer, int aTimeout)
{
	QByteArray commandData;

	return processCommand(aCommand, commandData, aAnswer, aTimeout);
}

//--------------------------------------------------------------------------------
TResult SparkFR::processCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer, int aTimeout)
{
	MutexLocker locker(&mExternalMutex);

	mProtocol.setPort(mIOPort);
	mProtocol.setLog(mLog);

	CSparkFR::Commands::SData data = CSparkFR::Commands::Data[aCommand];

	if (data.password)
	{
		QByteArray commandData = QByteArray::number(CSparkFR::Password).rightJustified(6, ASCII::Zero, true);
		TResult result = processCommand(CSparkFR::Commands::EnterPassword, commandData);

		if (!result)
		{
			toLog(LogLevel::Error, mDeviceName + ": Failed to enter control password, unable to perform current action therefore");
			return result;
		}
	}

	QByteArray answerData;
	QByteArray & answer = aAnswer ? *aAnswer : answerData;
	QByteArray commandData = aCommand + aCommandData;

	TResult result = mProtocol.processCommand(commandData, answer, aTimeout);

	if (answer == QByteArray(1, ASCII::ENQ))
	{
		if (mCheckStatus)
		{
			simplePoll();
		}

		toLog(LogLevel::Error, QString("%1: Error: %2").arg(mDeviceName).arg(CSparkFR::Errors::Descriptions[mLastError]));

		if (!mProcessingErrors.isEmpty() && (mProcessingErrors.last() == mLastError))
		{
			return CommandResult::Device;
		}

		if (!processAnswer(mLastError))
		{
			return CommandResult::Device;
		}

		mProcessingErrors.pop_back();

		return processCommand(aCommand, aCommandData, aAnswer);
	}
	else if (result == CommandResult::NoAnswer)
	{
		if (mCheckStatus)
		{
			QByteArray ENQTAnswer;
			TResult ENQTResult = mProtocol.processCommand(CSparkFR::Commands::ENQT, ENQTAnswer, CSparkFR::Timeouts::Control);

			if (!ENQTResult)
			{
				return ENQTResult;
			}
		}

		return CommandResult::NoAnswer;

		//TODO: действия если нет ответа
	}
	else if (!result)
	{
		return result;
	}
	else if (!data.sending)
	{
		if (answer == QByteArray(1, ASCII::ACK))
		{
			toLog(LogLevel::Error, mDeviceName + ": ACK received for receiving command");
			return CommandResult::Answer;
		}
		else if (!data.answer.isEmpty())
		{
			if (!answer.startsWith(data.answer))
			{
				toLog(LogLevel::Error, mDeviceName + ": Wrong command code in answer");
				return CommandResult::Answer;
			}

			answer = answer.mid(data.answer.size());
		}
	}

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
bool SparkFR::processAnswer(char aError)
{
	switch (aError)
	{
		case CSparkFR::Errors::NeedZReport:
		case CSparkFR::Errors::TimeOff:
		{
			mProcessingErrors.push_back(aError);

			if (!mStatusCollection.contains(FRStatusCode::Error::ZBufferOverflow))
			{
				if (mOperatorPresence)
				{
					toLog(LogLevel::Error, mDeviceName + ": Failed to process auto-Z-report due to presence of the operator.");
					mNeedCloseSession = mNeedCloseSession || isSessionExpired();

					return false;
				}

				return execZReport();
			}
		}
		//--------------------------------------------------------------------------------
		case CSparkFR::Errors::NeedPayIOOnly:
		case CSparkFR::Errors::NeedSaleOnly:
		{
			mProcessingErrors.push_back(aError);

			return cancelDocument(true);
		}
		//--------------------------------------------------------------------------------
		case CSparkFR::Errors::KKMClosed:
		{
			mProcessingErrors.push_back(aError);

			QByteArray commandData = QByteArray::number(CSparkFR::CashierNumber).rightJustified(2, ASCII::Zero, true) +
				QByteArray::number(CSparkFR::CashierPassword).rightJustified(5, ASCII::Zero, true);

			return processCommand(CSparkFR::Commands::OpenKKM, commandData);
		}
		//--------------------------------------------------------------------------------
		case CSparkFR::Errors::KKMOpened:
		{
			mProcessingErrors.push_back(aError);

			return processCommand(CSparkFR::Commands::CloseKKM);
		}
		//--------------------------------------------------------------------------------
		case CSparkFR::Errors::CashierNotSet:
		{
			mProcessingErrors.push_back(aError);

			QByteArray commandData = QByteArray::number(CSparkFR::CashierPassword).rightJustified(5, ASCII::Zero, true);

			return processCommand(CSparkFR::Commands::SetCashier, commandData);
		}
		//--------------------------------------------------------------------------------
		case CSparkFR::Errors::WrongTextModeCommand:
		{
			mProcessingErrors.push_back(aError);

			return cut();
		}
	}

	return false;
}

//--------------------------------------------------------------------------------
bool SparkFR::printLine(const QByteArray & aString)
{
	char tagModifier = ASCII::NUL;
	if (mLineTags.contains(Tags::Type::UnderLine))    tagModifier += CSparkFR::Tag::UnderLine;
	if (mLineTags.contains(Tags::Type::DoubleHeight)) tagModifier += CSparkFR::Tag::DoubleHeight;

	QByteArray commandData = char(tagModifier + ASCII::Zero) + aString;

	return processCommand(CSparkFR::Commands::PrintLine, commandData);
}

//--------------------------------------------------------------------------------
void SparkFR::execTags(Tags::SLexeme & aTagLexeme, QVariant & aLine)
{
	QByteArray data = mCodec->fromUnicode(aTagLexeme.data);

	if (aTagLexeme.tags.contains(Tags::Type::DoubleWidth))
	{
		Tags::TTypes types;
		types.insert(Tags::Type::DoubleWidth);

		for (int i = 0; i < data.size(); i = i + 2)
		{
			data.insert(i, mTagEngine->getTag(types, Tags::Direction::Open));
		}
	}

	aLine = aLine.toByteArray() + data;
}

//--------------------------------------------------------------------------------
bool SparkFR::getStatus(TStatusCodes & aStatusCodes)
{
	mProtocol.setPort(mIOPort);
	mProtocol.setLog(mLog);

	QByteArray answer;

	if (!mProtocol.processCommand(CSparkFR::Commands::ENQT, answer, CSparkFR::Timeouts::Control))
	{
		return false;
	}

	if (answer.size() > 21)
	{
		mSessionOpeningDT = QDateTime(QDate(fromBCD(answer[19]), fromBCD(answer[18]), fromBCD(answer[17])).addYears(2000),
			QTime(fromBCD(answer[20]), fromBCD(answer[21])));
	}
	else
	{
		aStatusCodes.insert(DeviceStatusCode::Error::Unknown);
	}

	     if (answer.size() > 7) mDocumentState = answer[7];
	else if (answer.size() > 6) mLastError = fromBCD<char>(answer.mid(5, 2));
	else if (answer.size() > 2) CSparkFR::Status3.getSpecification(answer[2], aStatusCodes);
	else if (answer.size() > 1) CSparkFR::Status2.getSpecification(answer[1], aStatusCodes);
	else if (answer.size() > 0) CSparkFR::Status1.getSpecification(answer[0], aStatusCodes);

	mCheckStatus = false;

	if (mStatusCollection.contains(FRStatusCode::Error::ZBuffer))
	{
		getZBufferState();
	}

	if (!processCommand(CSparkFR::Commands::GetSensorState, &answer) || (answer.size() < 2))
	{
		aStatusCodes.insert(DeviceStatusCode::Error::Unknown);
	}
	else if (fromBCD<char>(answer.left(2)) & CSparkFR::PaperInPresenter)
	{
		aStatusCodes.insert(PrinterStatusCode::OK::PaperInPresenter);
	}

	mCheckStatus = true;

	return true;
}

//--------------------------------------------------------------------------------
void SparkFR::getZBufferState()
{
	// только для 110. отнаследовать!
	QByteArray ZBufferSpaceData;
	QByteArray ZBufferCountData;

	if (!processCommand(CSparkFR::Commands::ZBufferSpace, &ZBufferSpaceData) || (ZBufferSpaceData.size() < 9) ||
		!processCommand(CSparkFR::Commands::ZBufferCount, &ZBufferCountData) || (ZBufferCountData.size() < 2))
	{
		mZBufferError = true;
	}
	else
	{
		mZBufferOverflow = mZBufferOverflow || bool(fromBCD(ZBufferSpaceData[8]));
		mZReports = fromBCD<uchar>(ZBufferCountData.left(2));
		/*
		TODO: для мониторинга
		int usedSpace  = fromBCD<ushort>(ZBufferSpaceData.left(4));
		int totalSpace = fromBCD<ushort>(ZBufferSpaceData.mid(4, 4));
		int totalCount = totalSpace * mZReports / usedSpace;
		*/
	}
}

//--------------------------------------------------------------------------------
bool SparkFR::payIO(double aAmount, bool aIn)
{
	QByteArray commandData = QByteArray::number(CSparkFR::CashPaymentType) +
		QByteArray::number(int(aAmount * 100)).rightJustified(10, ASCII::Zero);
	QString commandLog = aIn ? "pay in" : "pay out";

	if (!processCommand(aIn ? CSparkFR::Commands::Payin : CSparkFR::Commands::Payout, commandData))
	{
		toLog(LogLevel::Error, QString("%1: Failed to %2 %3").arg(mDeviceName).arg(commandLog).arg(aAmount, 0, 'f', 2));
		return false;
	}

	if (!processCommand(CSparkFR::Commands::ClosePayIO))
	{
		toLog(LogLevel::Error, QString("%1: Failed to close %2").arg(mDeviceName).arg(commandLog));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool SparkFR::setFiscalParameters(const QStringList & aReceipt)
{
	QByteArray commandData;
	commandData += QString("%1").arg(0, 2, 10, QChar(ASCII::Zero)).right(2);

	if (!processCommand(CSparkFR::Commands::SetTextProperty, commandData))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to clear fiscal parameters");
		return false;
	}

	for (int i = 0; i < aReceipt.size(); ++i)
	{
		if (!aReceipt[i].isEmpty())
		{
			commandData.clear();
			commandData += QString("%1").arg(CSparkFR::TextProperties::Numbers[i], 2, 10, QChar(ASCII::Zero)).right(2).toLatin1() + mCodec->fromUnicode(aReceipt[i]);

			if (!processCommand(CSparkFR::Commands::SetTextProperty, commandData))
			{
				toLog(LogLevel::Error, QString("%1: Failed to set fiscal parameter %2 =\n%3").arg(mDeviceName).arg(CSparkFR::TextProperties::Numbers[i]).arg(aReceipt[i]));
				return false;
			}
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool SparkFR::performFiscal(const QStringList & aReceipt, const SPaymentData & aPaymentData, TFiscalPaymentData & aFPData, TComplexFiscalPaymentData & aPSData)
{
	TVATs absentVATs = getVATs(aPaymentData) - mTaxes.toSet();

	if (!absentVATs.isEmpty())
	{
		toLog(LogLevel::Error, QString("%1: Failed to process fiscal document due to there are some unaccounted taxe(s): %2").arg(mDeviceName).arg(getVATLog(absentVATs)));
		return false;
	}

	TSum totalAmount = getTotalAmount(aPaymentData);

	if (!payIO(totalAmount, true))
	{
		return false;
	}

	AdaptiveFiscalLogic logic(getDeviceConfiguration());

	QStringList noTagReceipt;
	makeReceipt(aReceipt, noTagReceipt);

	bool result = true;

	if (isAdaptiveFCCreation() && logic.adjustReceipt(noTagReceipt))
	{
		if (!setFiscalParameters(logic.getTextProperties()) && !processReceipt(aReceipt))
		{
			result = false;
		}
	}
	else if (!processReceipt(aReceipt))
	{
		result = false;
	}

	if (result)
	{
		foreach(auto amountData, aPaymentData.amountDataList)
		{
			result = result && sale(amountData);
		}

		result = result && processCommand(CSparkFR::Commands::CloseFiscal, QByteArray::number(CSparkFR::CashPaymentType));

		if (!result)
		{
			cancelDocument(true);
		}
	}

	if (!result)
	{
		payIO(totalAmount, false);
	}

	return result;
}

#define CHECK_SPARK_TAX(aNumber) if ((mTaxes.size() >= aNumber) && (aAmountData.VAT == mTaxes[aNumber - 1])) command = CSparkFR::Commands::Sale##aNumber;

//--------------------------------------------------------------------------------
bool SparkFR::sale(const SAmountData & aAmountData)
{
	QByteArray command = CSparkFR::Commands::Sale0;

	int index = mTaxes.indexOf(aAmountData.VAT);

	if (index == 0) command = CSparkFR::Commands::Sale1;
	if (index == 1) command = CSparkFR::Commands::Sale2;
	if (index == 2) command = CSparkFR::Commands::Sale3;
	if (index == 3) command = CSparkFR::Commands::Sale4;

	QByteArray commandData =
		QByteArray::number(qRound64(aAmountData.sum * 100.0)).rightJustified(8, ASCII::Zero) +
		QByteArray::number(1 * 1000).rightJustified(8, ASCII::Zero) +
		QByteArray::number(1).rightJustified(2, ASCII::Zero) +
		mCodec->fromUnicode(aAmountData.name.leftJustified(CSparkFR::LineSize, ASCII::Space, true));

	if (!processCommand(command, commandData))
	{
		toLog(LogLevel::Error, QString("%1: Failed to sale for %2 (%3, VAT = %4)").arg(mDeviceName).arg(aAmountData.sum, 0, 'f', 2).arg(aAmountData.name).arg(aAmountData.VAT));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
double SparkFR::getAmountInCash()
{
	QByteArray answer;

	if (!processCommand(CSparkFR::Commands::GetCashAcceptorTotal, &answer) || (answer.size() < 41))
	{
		return -1;
	}

	return fromBCD<int>(answer.mid(29, 12)) / 100.0;
}

//--------------------------------------------------------------------------------
bool SparkFR::processPayout(double aAmount)
{
	waitEjectorReady();
	int oldDocumentNumber = getLastDocumentNumber();

	bool result = payIO(aAmount, false);
	waitNextPrinting();

	if (!result && (oldDocumentNumber != -1))
	{
		int newDocumentNumber = getLastDocumentNumber();

		if ((newDocumentNumber != -1) && (newDocumentNumber > oldDocumentNumber))
		{
			return true;
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
bool SparkFR::cut()
{
	return processCommand(CSparkFR::Commands::Cut);
}

//--------------------------------------------------------------------------------
bool SparkFR::retract()
{
	auto dataIt = std::find_if(mSystemFlags.begin(), mSystemFlags.end(), [&](const CSparkFR::SystemFlags::SData & aData) -> bool { return aData.number == CSparkFR::SystemOptions2; });

	if (dataIt == mSystemFlags.end())
	{
		toLog(LogLevel::Error, QString("%1: Failed to find system flag %2 in device one").arg(mDeviceName).arg(CSparkFR::SystemOptions2));
		return false;
	}

	QByteArray flagData;

	if (!getSystemFlags(flagData))
	{
		return false;
	}

	// отключаем альтернативный способ опроса датчиков презентера
	dataIt->mask[6] = '0';

	if (!checkFlag(flagData, CSparkFR::SystemOptions2))
	{
		return false;
	}

	if (!processCommand(CSparkFR::Commands::Retract))
	{
		return false;
	}

	// отключаем альтернативный способ опроса датчиков презентера
	dataIt->mask[6] = '1';
	checkFlag(flagData, CSparkFR::SystemOptions2);

	return true;
}

//--------------------------------------------------------------------------------
bool SparkFR::performZReport(bool aPrintDeferredReports)
{
	bool printZBufferOK = !aPrintDeferredReports;

	if (aPrintDeferredReports && mZReports)
	{
		toLog(LogLevel::Normal, mDeviceName + ": Printing deferred Z-reports");
		int timeout = mZReports * CSparkFR::Timeouts::Report;

		printZBufferOK = processCommand(CSparkFR::Commands::PrintZBuffer, CSparkFR::PushZReport, nullptr, timeout);
	}

	bool printZReport = execZReport() && processCommand(CSparkFR::Commands::PrintZBuffer, CSparkFR::PushZReport, nullptr, CSparkFR::Timeouts::Report);

	return (printZBufferOK && aPrintDeferredReports) || printZReport;
}

//--------------------------------------------------------------------------------
bool SparkFR::cancelDocument(bool aDocumentIsOpened)
{
	if (!aDocumentIsOpened)
	{
		return cut();
	}

	if (!processCommand(CSparkFR::Commands::CancelFiscal))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to cancel fiscal document");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
QDateTime SparkFR::parseDateTime(TKKMInfoData & aData)
{
	QTime time = QTime::fromString(aData[9],  CSparkFR::TimeFormat);
	QDate date = QDate::fromString(aData[10].insert(4, "20"), CFR::DateFormat);

	return QDateTime(date, time);
}

//--------------------------------------------------------------------------------
void SparkFR::processDeviceData()
{
	QByteArray answer;

	if (processCommand(CSparkFR::Commands::GetFWVersion, &answer))
	{
		QString answerData = mCodec->toUnicode(answer);
		QRegExp regExp(CSparkFR::Models::RegExpData);

		if (regExp.indexIn(answerData) != -1)
		{
			QStringList capturedData = regExp.capturedTexts();
			setDeviceParameter(CDeviceData::Firmware, QString("%1").arg(capturedData[2].toDouble(), 0, 'f', 2));
		}
	}

	TKKMInfoData data;

	if (getKKMData(data))
	{
		setDeviceParameter(CDeviceData::FR::TotalPaySum,        data[1].toInt());
		setDeviceParameter(CDeviceData::FR::FiscalDocuments,    data[2].toInt());
		setDeviceParameter(CDeviceData::FR::SessionCount,       data[3].toInt());
		setDeviceParameter(CDeviceData::FR::NonFiscalDocuments, data[4].toInt());
		setDeviceParameter(CDeviceData::FR::OwnerId,            data[6].toInt());

		mRNM = CFR::RNMToString(data[7]);
		mSerial = CFR::serialToString(data[8]);
	}

	if (processCommand(CSparkFR::Commands::EKLZInfo, &answer) && (answer.size() >= 42))
	{
		qulonglong EKLZSerial = fromBCD<qulonglong>(answer.mid(32, 10));
		setDeviceParameter(CDeviceData::EKLZ::Serial, EKLZSerial);
	}

	if (processCommand(CSparkFR::Commands::GetEKLZError, &answer) && !answer.isEmpty())
	{
		char reregistrationData = answer[0];
		mFiscalized = reregistrationData != CSparkFR::NoReregistrationNumber;
		setDeviceParameter(CDeviceData::FR::Activated, mFiscalized, CDeviceData::FR::EKLZ);

		if (mFiscalized)
		{
			int reregistrationNumber = fromBCD(reregistrationData);

			if (reregistrationNumber != -1)
			{
				setDeviceParameter(CDeviceData::FR::ReregistrationNumber, reregistrationNumber);
			}
		}
	}

	// пока как инфо, чтобы посмотреть состояние Z-буфера на загрузке. Потом пойдет в мониторинг.
	getZBufferState();
	checkDateTime();
}

//--------------------------------------------------------------------------------
bool SparkFR::processXReport()
{
	waitEjectorReady();
	int oldDocumentNumber = getLastDocumentNumber();

	QByteArray commandData = QByteArray(1, CSparkFR::SessionReport) + CSparkFR::XReport;
	TResult result = processCommand(CSparkFR::Commands::Reports, commandData, nullptr, CSparkFR::Timeouts::Report);
	waitNextPrinting();

	if (!result && (oldDocumentNumber != -1))
	{
		int newDocumentNumber = getLastDocumentNumber();

		if ((newDocumentNumber != -1) && (newDocumentNumber > oldDocumentNumber))
		{
			return true;
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
bool SparkFR::getKKMData(TKKMInfoData & aData)
{
	QByteArray answer;

	if (!processCommand(CSparkFR::Commands::KKMInfo, &answer))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to get KKM info");
		return false;
	}

	aData = answer.split(CSparkFR::Separator);

	if (aData.size() < 11)
	{
		toLog(LogLevel::Error, QString("%1: Too small sections in KKM info answer = %2, need 11 min").arg(mDeviceName).arg(aData.size()));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool SparkFR::isSessionExpired()
{
	TKKMInfoData data;

	return isSessionOpened() && getKKMData(data) && mSessionOpeningDT.daysTo(parseDateTime(data));
}

//--------------------------------------------------------------------------------
int SparkFR::getLastDocumentNumber()
{
	TKKMInfoData data;

	return getKKMData(data) ? data[5].toInt() : -1;
}

//--------------------------------------------------------------------------------
bool SparkFR::isSessionOpened()
{
	return mSessionOpeningDT != CSparkFR::ClosedSession;
}

//--------------------------------------------------------------------------------
bool SparkFR::execZReport()
{
	toLog(LogLevel::Normal, QString("%1: Begin processing %2-report").arg(mDeviceName).arg(CSparkFR::ZReport));

	if (!isSessionOpened())
	{
		toLog(LogLevel::Error, mDeviceName + ": Session is closed, exit!");
		return false;
	}

	mNeedCloseSession = false;
	QByteArray commandData = QByteArray(1, CSparkFR::SessionReport) + CSparkFR::ZReport;
	bool result = processCommand(CSparkFR::Commands::Reports, commandData, nullptr, CSparkFR::Timeouts::ZReport);

	mNeedCloseSession = isSessionExpired();
	mZReports += int(result);

	return result;
}

//--------------------------------------------------------------------------------
bool SparkFR::waitEjectorReady()
{
	TStatusCodes statusCodes;
	bool pollled = false;
	auto poll = [&] () -> bool { pollled = true; statusCodes.clear(); return getStatus(statusCodes); };
	auto condition = [&] () -> bool { return pollled && !statusCodes.contains(DeviceStatusCode::Error::NotAvailable) && !statusCodes.contains(PrinterStatusCode::OK::PaperInPresenter); };

	return PollingExpector().wait<bool>(poll, condition, CSparkFR::WaitingInterval, CSparkFR::Timeouts::Ejector, true);// || retract();
}

//--------------------------------------------------------------------------------
bool SparkFR::waitNextPrinting()
{
	TStatusCodes statusCodes;
	bool pollled = false;
	auto poll = [&] () -> bool { pollled = true; statusCodes.clear(); return getStatus(statusCodes); };
	auto condition = [&] () -> bool { return pollled && !statusCodes.contains(DeviceStatusCode::Error::NotAvailable) && !statusCodes.contains(DeviceStatusCode::Error::Unknown); };

	return PollingExpector().wait<bool>(poll, condition, CSparkFR::WaitingInterval, CSparkFR::Timeouts::Printing);
}

//--------------------------------------------------------------------------------
