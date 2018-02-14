/* @file ФР АТОЛ и Пэй Киоск. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Project
#include "KasbiFRBase.h"
#include "KasbiModelData.h"

using namespace SDK::Driver;
using namespace ProtocolUtils;

//--------------------------------------------------------------------------------
KasbiFRBase::KasbiFRBase()
{
	using namespace SDK::Driver::IOPort::COM;

	// данные порта
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);   // default
	mPortParameters[EParameters::Parity].append(EParity::No);

	// данные устройства
	mDeviceName = CKasbiFR::Models::Default;
	mLineFeed = false;
	mIsOnline = true;
	mNextReceiptProcessing = false;
	setConfigParameter(CHardwareSDK::CanOnline, true);

	// налоги
	mTaxData.add(18, 1, "НДС 10%");
	mTaxData.add(10, 2, "НДС 18%");
	mTaxData.add( 0, 6, "БЕЗ НАЛОГА");

	// теги
	mTagEngine = Tags::PEngine(new CKasbiFR::TagEngine());
}

//--------------------------------------------------------------------------------
QDateTime KasbiFRBase::getDateTime()
{
	QByteArray data;

	if (processCommand(CKasbiFR::Commands::GetFRDateTime, &data))
	{
		CFR::TTLVList TLVs = parseSTLV(data);

		if (TLVs.contains(CKasbiFR::FiscalFields::FRDateTime) && (TLVs[CKasbiFR::FiscalFields::FRDateTime].size() == 5))
		{
			data = TLVs[CKasbiFR::FiscalFields::FRDateTime];

			QDate date(int(2000) + data[0], data[1], data[2]);
			QTime time(data[3], data[4]);

			if (date.isValid() && time.isValid())
			{
				return QDateTime(date, time);
			}
		}
	}

	return QDateTime();
}

//--------------------------------------------------------------------------------
bool KasbiFRBase::isConnected()
{
	QByteArray answer;

	if (!processCommand(CKasbiFR::Commands::GetModelInfo, &answer))
	{
		return false;
	}

	CKasbiFR::Models::SData modelData = CKasbiFR::Models::Data[answer];

	if (!CKasbiFR::Models::Data.data().contains(answer))
	{
		toLog(LogLevel::Error, "KasbiFR: Unknown model");
	}

	mDeviceName = modelData.name;
	mVerified = modelData.verified;
	mModelCompatibility = true;

	return true;
}

//--------------------------------------------------------------------------------
bool KasbiFRBase::updateParameters()
{
	if (isDocumentOpened())
	{
		processCommand(CKasbiFR::Commands::CancelDocument);
	}

	QByteArray data;

	if (!processCommand(CKasbiFR::Commands::GetRegistrationData, &data) || (data.size() <= 34))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to get taxation system data");
		return false;
	}

	if (!checkOperationModes(data[32]) || !checkTaxSystems(data[33]) || !checkAgentFlags(data[34]))
	{
		return false;
	}

	processDeviceData(data);

	if (!checkPrintingParameters())
	{
		return false;
	}

	mOFDDataError = true;

	if (processCommand(CKasbiFR::Commands::GetOFDData, &data))
	{
		CFR::TTLVList TLVs = parseSTLV(data);

		if (TLVs.contains(CKasbiFR::FiscalFields::OFDAddress) && TLVs.contains(CKasbiFR::FiscalFields::OFDPort))
		{
			mOFDDataError = !checkOFDData(TLVs[CKasbiFR::FiscalFields::OFDAddress], revert(TLVs[CKasbiFR::FiscalFields::OFDPort]));
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool KasbiFRBase::checkPrintingParameters()
{
	QByteArray data;

	if (!processCommand(CKasbiFR::Commands::GetPrintingParameters, &data))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to get FR printing parameters");
		return false;
	}

	CFR::TTLVList TLVs = parseSTLV(data);

	QByteArray fontSizeData     = TLVs.value(CKasbiFR::FiscalFields::FontSize);
	QByteArray optionalFPData   = TLVs.value(CKasbiFR::FiscalFields::OptionalFiscalParameter);
	QByteArray sessionRRData    = TLVs.value(CKasbiFR::FiscalFields::SessionReportRetraction);
	QByteArray printerModelData = TLVs.value(CKasbiFR::FiscalFields::PrinterModel);

	if (sessionRRData.isEmpty())
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to check retraction of session reports due to no valid data");
		return false;
	}

	if (!printerModelData.isEmpty() && CKasbiFR::PrinterModels.data().contains(printerModelData[0]))
	{
		setDeviceParameter(CDeviceData::FR::Printer, CKasbiFR::PrinterModels[printerModelData[0]]);
	}

	char fontSize = fontSizeData.isEmpty() ? CKasbiFR::FontSize : fontSizeData[0];
	bool wrongRetraction = sessionRRData[0] != CKasbiFR::NoSessionReportRetraction;

	if ((fontSize != CKasbiFR::FontSize) || wrongRetraction)
	{
		QByteArray commandData = QByteArray() +
			getTLVData(CKasbiFR::FiscalFields::PrinterModel, printerModelData) +
			getTLVData(CKasbiFR::FiscalFields::FontSize, CKasbiFR::FontSize) +
			getTLVData(CKasbiFR::FiscalFields::SessionReportRetraction, CKasbiFR::NoSessionReportRetraction) +
			getTLVData(CKasbiFR::FiscalFields::OptionalFiscalParameter, optionalFPData);

		if (!processCommand(CKasbiFR::Commands::SetPrintingParameters, commandData))
		{
			toLog(LogLevel::Error, mDeviceName + ": Failed to set FR printing parameters");

			if (wrongRetraction)
			{
				return false;
			}
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
void KasbiFRBase::processDeviceData(const QByteArray & aRegistrationData)
{
	mRNM = CFR::RNMToString(aRegistrationData.left(20));
	mINN = CFR::INNToString(aRegistrationData.mid(20, 12));

	setDeviceParameter(CDeviceData::FR::ModeFlags, CKasbiFR::Modes().getValues(aRegistrationData[32]));
	QByteArray data;

	if (processCommand(CKasbiFR::Commands::GetSerial, &data) && (data.size() >= 12))
	{
		mSerial = CFR::serialToString(data);
	}

	if (processCommand(CKasbiFR::Commands::GetVersion, &data))
	{
		setDeviceParameter(CDeviceData::Firmware, data);
	}

	if (processCommand(CKasbiFR::Commands::GetFSSerial, &data))
	{
		mFSSerialNumber = CFR::FSSerialToString(data);
	}

	if (processCommand(CKasbiFR::Commands::GetFSVersion, &data))
	{
		setDeviceParameter(CDeviceData::FS::Version, clean(data));
	}

	if (processCommand(CKasbiFR::Commands::GetFSData, &data) && (data.size() >= 5))
	{
		QDate date(int(2000) + data[0], data[1], data[2]);
		setDeviceParameter(CDeviceData::FS::ValidityData, date.toString(CFR::DateLogFormat));
		setDeviceParameter(CDeviceData::FR::ReregistrationNumber, int(data[4]));
		setDeviceParameter(CDeviceData::FR::FreeReregistrations, int(data[3]));
	}

	CKasbiFR::SFSData FSData;

	if (getFSData(FSData))
	{
		setDeviceParameter(CDeviceData::FR::FiscalDocuments, FSData.lastFDNumber);
	}

	checkDateTime();
}

//--------------------------------------------------------------------------------
TResult KasbiFRBase::execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer)
{
	mProtocol.setPort(mIOPort);
	mProtocol.setLog(mLog);

	QByteArray commandData = aCommand + aCommandData;
	QByteArray answer;

	TResult result = mProtocol.processCommand(commandData, answer, CKasbiFR::Commands::Data[aCommand[0]]);

	if ((result != CommandResult::Transport) && aAnswer)
	{
		*aAnswer = answer;
	}

	if (!result)
	{
		return result;
	}

	if (answer.size() < CKasbiFR::MinUnpackedAnswerSize)
	{
		toLog(LogLevel::Error, mDeviceName + ": Answer data is less than " + QString::number(CKasbiFR::MinUnpackedAnswerSize));
		return CommandResult::Answer;
	}

	if (!answer[0])
	{
		if (aAnswer)
		{
			*aAnswer = answer.mid(1);
		}

		return CommandResult::OK;
	}
	else if (answer.size() < CKasbiFR::MinUnpackedErrorSize)
	{
		toLog(LogLevel::Error, mDeviceName + ": Answer data in packet is less than " + QString::number(CKasbiFR::MinUnpackedErrorSize));
		return CommandResult::Answer;
	}

	char error = answer[1];
	toLog(LogLevel::Error, mDeviceName + ": Error: " + CKasbiFR::Errors::Data[error].description);

	if (error == CKasbiFR::Errors::Protocol)
	{
		return CommandResult::Protocol;
	}

	if (!mProcessingErrors.isEmpty() && (mProcessingErrors.last() == error))
	{
		return CommandResult::Device;
	}

	if (!processAnswer(aCommand[0], error))
	{
		return CommandResult::Device;
	}

	mProcessingErrors.pop_back();

	return processCommand(aCommand, aCommandData, aAnswer);
}

//--------------------------------------------------------------------------------
bool KasbiFRBase::isDocumentOpened()
{
	CKasbiFR::SFSData data;

	return getFSData(data) && data.documentOpened;
}

//--------------------------------------------------------------------------------
bool KasbiFRBase::processAnswer(char aCommand, char aError)
{
	switch (aError)
	{
		case CKasbiFR::Errors::NeedAgentData:
		case CKasbiFR::Errors::WrongTotalSum:
		case CKasbiFR::Errors::WrongFSState:
		{
			mProcessingErrors.append(aError);

			if (aCommand == CKasbiFR::Commands::CancelDocument)
			{
				return false;
			}

			if (isDocumentOpened())
			{
				return processCommand(CKasbiFR::Commands::CancelDocument);
			}
		}
		case CKasbiFR::Errors::NeedZReport:
		{
			mProcessingErrors.append(aError);

			return execZReport(true) && openFRSession();
		}
		case CKasbiFR::Errors::UnknownCommand:
		{
			mOldFirmware = mOldFirmware || (aCommand == CKasbiFR::Commands::GetVersion);
		}
		case CKasbiFR::Errors::WrongVATForAgent:
		{
			mOldFirmware = true;
		}
	}

	return false;
}

//--------------------------------------------------------------------------------
bool KasbiFRBase::getStatus(TStatusCodes & aStatusCodes)
{
	QByteArray data;

	if (!processCommand(CKasbiFR::Commands::GetStatus, &data))
	{
		return false;
	}

	if (data.size() < 19)
	{
		aStatusCodes.insert(DeviceStatusCode::Error::Unknown);
	}
	else if (data[18])
	{
		aStatusCodes.insert(CKasbiFR::Statuses[data[18]]);
	}

	if (!processCommand(CKasbiFR::Commands::GetOFDNotSentCount, &data))
	{
		return false;
	}

	auto getInt = [&] (int aIndex, int aShift) -> int { int result = uchar(data[aIndex]); return result << (8 * aShift); };

	int OFDNotSentCount = (data.size() < 2) ? -1 : (getInt(0, 0) | getInt(1, 1));
	checkOFDNotSentCount(OFDNotSentCount, aStatusCodes);

	return true;
}

//--------------------------------------------------------------------------------
bool KasbiFRBase::printLine(const QByteArray & aString)
{
	QByteArray commandData(2, ASCII::NUL);

	for (auto it = CKasbiFR::Tags.data().begin(); it != CKasbiFR::Tags.data().end(); ++it)
	{
		if (mLineTags.contains(it.key()))
		{
			commandData[0] = commandData[0] | it.value();
		}
	}

	if (mLineTags.contains(Tags::Type::Center))
	{
		commandData[1] = CKasbiFR::CenterTag;
	}

	return processCommand(CKasbiFR::Commands::PrintLine, commandData + aString);
}

//--------------------------------------------------------------------------------
bool KasbiFRBase::cut()
{
	return processCommand(CKasbiFR::Commands::Cut);
}

//--------------------------------------------------------------------------------
bool KasbiFRBase::getFSData(CKasbiFR::SFSData & aData)
{
	QByteArray data;

	if (!processCommand(CKasbiFR::Commands::GetFSStatus, &data) || (data.size() < 30))
	{
		return false;
	}

	auto getInt = [&data] (int aIndex) -> uint { int result = uchar(data[26 + aIndex]); return result << (8 * aIndex); };
	uint lastFDNumber = getInt(0) | getInt(1) | getInt(2) | getInt(3);

	aData = CKasbiFR::SFSData(data[1], data[3], data[4], data.mid(10, 16).toULongLong(), lastFDNumber);

	return true;
}

//--------------------------------------------------------------------------------
ESessionState::Enum KasbiFRBase::getSessionState()
{
	CKasbiFR::SFSData data;

	if (!getFSData(data))
	{
		return ESessionState::Error;
	}

	return data.sessionOpened ? ESessionState::Opened : ESessionState::Closed;
}

//--------------------------------------------------------------------------------
bool KasbiFRBase::openSession()
{
	if (getSessionState() == ESessionState::Opened)
	{
		toLog(LogLevel::Warning, mDeviceName + ": Session is opened already");
		return true;
	}

	return processCommand(CKasbiFR::Commands::StartOpeningSession, QByteArray(1, CKasbiFR::Print::No)) &&
	       processCommand(CKasbiFR::Commands::OpenSession);
}

//--------------------------------------------------------------------------------
bool KasbiFRBase::execZReport(bool aAuto)
{
	bool needCloseSession = true; //TODO: isSessionExpired();
	bool cannotAutoZReport = mOperatorPresence && !getConfigParameter(CHardware::FR::ForcePerformZReport).toBool();

	if (aAuto && cannotAutoZReport)
	{
		toLog(LogLevel::Error, mDeviceName + (mOperatorPresence ?
			": Failed to process auto-Z-report due to presence of the operator." :
			": has no Z-buffer, so it is impossible to perform auto-Z-report."));
		mNeedCloseSession = mNeedCloseSession || needCloseSession;

		return false;
	}

	toLog(LogLevel::Normal, mDeviceName + QString(": Begin processing %1Z-report").arg(aAuto ? "auto-" : ""));

	if (getSessionState() == ESessionState::Closed)
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Session is closed, exit!"));
		return false;
	}

	mNeedCloseSession = false;
	char printing = aAuto ? CKasbiFR::Print::No : CKasbiFR::Print::Yes;
	bool result = processCommand(CKasbiFR::Commands::StartZReport, QByteArray(1, printing)) && processCommand(CKasbiFR::Commands::EndZReport);
	mNeedCloseSession = false; //TODO: isSessionExpired();

	if (!mNeedCloseSession)
	{
		mProcessingErrors.removeAll(CKasbiFR::Errors::NeedZReport);
	}

	return result;
}

//--------------------------------------------------------------------------------
bool KasbiFRBase::performZReport(bool /*aPrintDeferredReports*/)
{
	toLog(LogLevel::Normal, mDeviceName + ": Processing Z-report");

	return execZReport(false);
}

//--------------------------------------------------------------------------------
bool KasbiFRBase::sale(const SAmountData & aAmountData)
{
	int section = (aAmountData.section == -1) ? mTaxData[aAmountData.VAT].group : aAmountData.section;

	QByteArray commandData = QByteArray() +
		getTLVData(FiscalFields::UnitName, aAmountData.name) +
		getTLVData(FiscalFields::PayOffSubjectUnitPrice, qRound64(aAmountData.sum * 100.0)) +
		getTLVData(FiscalFields::PayOffSubjectQuantity, 1.0) +
		getTLVData(FiscalFields::VATRate, char(section)) +
		getTLVData(FiscalFields::PayOffSubjectMethodType, CKasbiFR::FullPrepaymentSettlement);

	return processCommand(CKasbiFR::Commands::Sale, getTLVData(FiscalFields::PayOffSubject, commandData));
}

//--------------------------------------------------------------------------------
bool KasbiFRBase::performFiscal(const QStringList & aReceipt, const SPaymentData & aPaymentData, TFiscalPaymentData & aFPData, TComplexFiscalPaymentData & aPSData)
{
	if ((getSessionState() == ESessionState::Closed) && !openFRSession())
	{
		return false;
	}

	if (isDocumentOpened() && (!processCommand(CKasbiFR::Commands::CancelDocument) || !receiptProcessing()))
	{
		return false;
	}

	if (!processReceipt(aReceipt, false) || !processCommand(CKasbiFR::Commands::OpenDocument))
	{
		receiptProcessing();

		return false;
	}

	bool result = true;

	foreach (auto amountData, aPaymentData.amountDataList)
	{
		result = result && sale(amountData);
	}

	QString userContact = getConfigParameter(CHardware::FiscalFields::UserContact).toString();
	uint totalSum = uint(getTotalAmount(aPaymentData) * 100);

	QByteArray commandData = QByteArray() +
		getTLVData(FiscalFields::TaxSystem, char(aPaymentData.taxSystem)) +
		getTLVData(FiscalFields::CashFiscalTotal, totalSum) +
		getTLVData(FiscalFields::CardFiscalTotal, 0) +
		getTLVData(FiscalFields::PrePaymentFiscalTotal, 0) +
		getTLVData(FiscalFields::PostPaymentFiscalTotal, 0) +
		getTLVData(FiscalFields::CounterOfferFiscalTotal, 0) +
		getTLVData(FiscalFields::UserContact, userContact);

	result = result && processCommand(CKasbiFR::Commands::Total, commandData);

	char command = aPaymentData.back ? CKasbiFR::SettlementTypes::IncomeReturning : CKasbiFR::SettlementTypes::Income;
	commandData = getDigitTLVData(totalSum).left(5);
	commandData = QByteArray(1, command) + commandData + QByteArray(5 - commandData.size(), ASCII::NUL);
	bool closingResult = processCommand(CKasbiFR::Commands::CloseDocument, commandData);

	if (result && (closingResult || !isDocumentOpened()))
	{
		return true;
	}

	processCommand(CKasbiFR::Commands::CancelDocument);
	receiptProcessing();

	return false;
}

//--------------------------------------------------------------------------------
bool KasbiFRBase::processXReport()
{
	return processCommand(CKasbiFR::Commands::StartXReport) && processCommand(CKasbiFR::Commands::EndXReport);
}

//--------------------------------------------------------------------------------
