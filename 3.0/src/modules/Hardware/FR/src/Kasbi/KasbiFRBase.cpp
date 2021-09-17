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
	mIsOnline = true;
	mFFDFR = EFFD::F105;
	mNextReceiptProcessing = false;
	setConfigParameter(CHardwareSDK::CanOnline, true);
	setConfigParameter(CHardwareSDK::FR::CanWithoutPrinting, true);

	mFFEngine.addData(CKasbiFR::FiscalFields::Data().data());

	// налоги
	mTaxData.add(18, 1);
	mTaxData.add(10, 2);
	mTaxData.add( 0, 6);

	// теги
	mTagEngine = Tags::PEngine(new CKasbiFR::TagEngine());

	// ошибки
	mErrorData = PErrorData(new CKasbiFR::Errors::Data());
	mUnprocessedErrorData.add(CKasbiFR::Commands::GetFiscalTLVData, CKasbiFR::Errors::NoRequiedDataInFS);
}

//--------------------------------------------------------------------------------
void KasbiFRBase::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	TSerialFRBase::setDeviceConfiguration(aConfiguration);

	QString printerModel = getConfigParameter(CHardware::FR::PrinterModel, CKasbiPrinters::Default).toString();

	if (aConfiguration.contains(CHardware::FR::PrinterModel) && (printerModel != CKasbiPrinters::Default) && !isNotPrinting())
	{
		mPPTaskList.append([&] () { mNotPrintingError = !setNotPrintDocument(false); });
	}
}

//--------------------------------------------------------------------------------
QDateTime KasbiFRBase::getDateTime()
{
	QByteArray data;

	if (processCommand(CKasbiFR::Commands::GetFRDateTime, &data))
	{
		CFR::TTLVList TLVs = mFFEngine.parseSTLV(data);

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
	if (getDocumentState() == EDocumentState::Opened)
	{
		processCommand(CKasbiFR::Commands::CancelDocument);
	}

	processDeviceData();

	CFR::TTLVList requiredPrintingData;
	requiredPrintingData.insert(CKasbiFR::FiscalFields::FontSize, QByteArray(1, CKasbiFR::FontSize));
	requiredPrintingData.insert(CKasbiFR::FiscalFields::SessionReportRetraction, QByteArray(1, CKasbiFR::SessionReportNoRetraction));

	if (!checkPrintingParameters(requiredPrintingData))
	{
		return false;
	}

	if (!isFiscal())
	{
		return true;
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

	mRNM = CFR::RNMToString(data.left(20));
	mINN = CFR::INNToString(data.mid(20, 12));

	return true;
}

//--------------------------------------------------------------------------------
bool KasbiFRBase::checkPrintingParameters(const CFR::TTLVList & aRequiredTLVs)
{
	QByteArray data;

	if (!processCommand(CKasbiFR::Commands::GetPrintingParameters, &data))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to get FR printing parameters");
		return false;
	}

	CFR::TTLVList TLVs = mFFEngine.parseSTLV(data);

	for (auto it = TLVs.begin(); it != TLVs.end(); ++it)
	{
		CFR::STLV TLV(it.key(), it.value());

		if (!mFFEngine.checkTLVData(TLV))
		{
			return false;
		}

		it.value() = TLV.data;
	}

	QSet<int> noFields = aRequiredTLVs.keys().toSet() - TLVs.keys().toSet();
	QString errorLog = mFFData.getLogFromList(noFields.toList());

	if (!errorLog.isEmpty())
	{
		toLog(LogLevel::Error, mDeviceName + ": No requied field(s): " + errorLog);
		return false;
	}

	if (TLVs.contains(CKasbiFR::FiscalFields::PrinterModel))
	{
		char printerModelKey = TLVs[CKasbiFR::FiscalFields::PrinterModel][0];
		QString printerModel = CKasbiPrinters::Models[printerModelKey];
		QString configModel = getConfigParameter(CHardware::FR::PrinterModel).toString();

		if ((configModel.isEmpty() || (configModel == CKasbiPrinters::Default)) && (printerModel != configModel))
		{
			setConfigParameter(CHardware::FR::PrinterModel, printerModel);

			emit configurationChanged();
		}

		if (!printerModel.isEmpty())
		{
			setDeviceParameter(CHardware::FR::PrinterModel, printerModel);
		}
	}

	if (TLVs.contains(CKasbiFR::FiscalFields::PrinterBaudRate))
	{
		uint baudrate = qToBigEndian(TLVs[CKasbiFR::FiscalFields::PrinterBaudRate].toHex().toUInt(0, 16));
		setDeviceParameter(COMPortSDK::BaudRate, baudrate, CHardware::FR::PrinterModel);
	}

	QVariantMap oldFFData;
	mFFEngine.parseTLVDataList(TLVs, oldFFData);

	for (auto it = aRequiredTLVs.begin(); it != aRequiredTLVs.end(); ++it)
	{
		TLVs.insert(it.key(), it.value());
	}

	QVariantMap FFData;
	mFFEngine.parseTLVDataList(TLVs, FFData);

	if (oldFFData != FFData)
	{
		QByteArray commandData;

		for (auto it = FFData.begin(); it != FFData.end(); ++it)
		{
			commandData += mFFEngine.getTLVData(it.key(), it.value());
		}

		if (!processCommand(CKasbiFR::Commands::SetPrintingParameters, commandData))
		{
			toLog(LogLevel::Error, mDeviceName + ": Failed to set FR printing parameters");
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
void KasbiFRBase::processDeviceData()
{
	QByteArray data;

	if (processCommand(CKasbiFR::Commands::GetSerial, &data) && (data.size() >= 12))
	{
		mSerial = CFR::serialToString(data);
	}

	if (processCommand(CKasbiFR::Commands::GetVersion, &data))
	{
		setDeviceParameter(CDeviceData::Firmware, data);

		mOldFirmware = DeviceUtils::isComplexFirmwareOld(data, CKasbiFR::LastFirmware);
	}

	if (processCommand(CKasbiFR::Commands::GetFSSerial, &data))
	{
		mFSSerialNumber = CFR::FSSerialToString(data);
	}

	if (processCommand(CKasbiFR::Commands::GetFSVersion, &data))
	{
		QByteArray FSVersion = clean(data);
		setDeviceParameter(CDeviceData::FS::Version, FSVersion);

		if (FSVersion.contains(CKasbiFR::FS10Id)) mFFDFS = EFFD::F10;
		if (FSVersion.contains(CKasbiFR::FS11Id)) mFFDFS = EFFD::F11;
	}

	if (mFFDFS == EFFD::Unknown)
	{
		mFFDFS = EFFD::F10;
	}

	if (processCommand(CKasbiFR::Commands::GetFSData, &data) && (data.size() >= 5))
	{
		QDate date(int(2000) + data[0], data[1], data[2]);
		setDeviceParameter(CDeviceData::FS::ValidityData, CFR::FSValidityDateOff(date));
		setDeviceParameter(CDeviceData::FR::ReregistrationNumber, int(data[4]));
		setDeviceParameter(CDeviceData::FR::FreeReregistrations, int(data[3]));
	}

	CKasbiFR::SFSData FSData;

	if (getFSData(FSData))
	{
		setDeviceParameter(CDeviceData::FR::FiscalDocuments, FSData.lastFDNumber);
	}

	if (processCommand(CKasbiFR::Commands::GetOFDData, &data))
	{
		CFR::TTLVList TLVs = mFFEngine.parseSTLV(data);

		if (TLVs.contains(CKasbiFR::FiscalFields::OFDAddress) && TLVs.contains(CKasbiFR::FiscalFields::OFDPort))
		{
			mOFDDataError = !checkOFDData(TLVs[CKasbiFR::FiscalFields::OFDAddress], revert(TLVs[CKasbiFR::FiscalFields::OFDPort]));
		}
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

	mLastError = answer[1];
	mLastCommand = aCommand;
	toLog(LogLevel::Error, mDeviceName + ": Error: " + mErrorData->value(mLastError).description);

	if (mLastError == CKasbiFR::Errors::Protocol)
	{
		return CommandResult::Protocol;
	}

	if (!mProcessingErrors.isEmpty() && (mProcessingErrors.last() == mLastError))
	{
		return CommandResult::Device;
	}

	char error = mLastError;

	if (isErrorUnprocessed(aCommand, error) || !processAnswer(aCommand[0], error))
	{
		mLastError = error;
		mLastCommand = aCommand;

		return CommandResult::Device;
	}

	result = processCommand(aCommand, aCommandData, aAnswer);

	if (result)
	{
		mProcessingErrors.pop_back();
	}

	return result;
}

//--------------------------------------------------------------------------------
EDocumentState::Enum KasbiFRBase::getDocumentState()
{
	CKasbiFR::SFSData data;

	if (!getFSData(data))
	{
		return EDocumentState::Error;
	}

	return data.documentOpened ? EDocumentState::Opened : EDocumentState::Closed;
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

			if (getDocumentState() == EDocumentState::Opened)
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

			break;
		}
		case CKasbiFR::Errors::WrongVATForAgent:
		{
			mOldFirmware = true;

			break;
		}
	}

	return false;
}

//--------------------------------------------------------------------------------
bool KasbiFRBase::getStatus(TStatusCodes & aStatusCodes)
{
	QByteArray data = performStatus(aStatusCodes, CKasbiFR::Commands::GetStatus, 18);

	if (data == CFR::Result::Fail)
	{
		return false;
	}
	else if (data != CFR::Result::Error)
	{
		aStatusCodes.insert(CKasbiFR::Statuses[data[18]]);
	}

	data = performStatus(aStatusCodes, CKasbiFR::Commands::GetOFDNotSentCount, 1);

	if (data == CFR::Result::Fail)
	{
		return false;
	}
	else if (data != CFR::Result::Error)
	{
		auto getInt = [&] (int aIndex, int aShift) -> int { int result = uchar(data[aIndex]); return result << (8 * aShift); };

		int OFDNotSentCount = (data.size() < 2) ? -1 : (getInt(0, 0) | getInt(1, 1));
		checkOFDNotSentCount(OFDNotSentCount, aStatusCodes);
	}

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
	return processCommand(CKasbiFR::Commands::StartOpeningSession, QByteArray(1, CKasbiFR::Print::No)) &&
	       processCommand(CKasbiFR::Commands::OpenSession);
}

//--------------------------------------------------------------------------------
bool KasbiFRBase::execZReport(bool aAuto)
{
	toLog(LogLevel::Normal, mDeviceName + QString(": Begin processing %1Z-report").arg(aAuto ? "auto-" : ""));
	ESessionState::Enum sessionState = getSessionState();

	     if (sessionState == ESessionState::Error)  return false;
	else if (sessionState == ESessionState::Closed) return true;

	bool needCloseSession = true; //TODO: sessionState
	bool cannotAutoZReport = mOperatorPresence && !getConfigParameter(CHardware::FR::ForcePerformZReport).toBool();

	if (aAuto && cannotAutoZReport)
	{
		toLog(LogLevel::Error, mDeviceName + (mOperatorPresence ?
			": Failed to process auto-Z-report due to presence of the operator." :
			": has no Z-buffer, so it is impossible to perform auto-Z-report."));
		mNeedCloseSession = mNeedCloseSession || needCloseSession;

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
bool KasbiFRBase::sale(const SUnitData & aUnitData)
{
	int section = (aUnitData.section == -1) ? mTaxData[aUnitData.VAT].group : aUnitData.section;

	QByteArray commandData = QByteArray() +
		mFFEngine.getTLVData(CFR::FiscalFields::UnitName, aUnitData.name) +
		mFFEngine.getTLVData(CFR::FiscalFields::PayOffSubjectUnitPrice, qRound64(aUnitData.sum * 100.0)) +
		mFFEngine.getTLVData(CFR::FiscalFields::PayOffSubjectQuantity, 1.0) +
		mFFEngine.getTLVData(CFR::FiscalFields::VATRate, char(section)) +
		mFFEngine.getTLVData(CFR::FiscalFields::PayOffSubjectMethodType, aUnitData.payOffSubjectMethodType);

	if (mOFDFiscalFieldsOnSale.contains(CFR::FiscalFields::ProviderName))
	{
		commandData += mFFEngine.getTLVData(CFR::FiscalFields::ProviderName, aUnitData.providerName);
	}

	if (mOFDFiscalFieldsOnSale.contains(CFR::FiscalFields::ProviderINN))
	{
		commandData += mFFEngine.getTLVData(CFR::FiscalFields::ProviderINN, aUnitData.providerINN);
	}

	return processCommand(CKasbiFR::Commands::Sale, mFFEngine.getTLVData(CFR::FiscalFields::PayOffSubject, commandData));
}

//--------------------------------------------------------------------------------
bool KasbiFRBase::performFiscal(const QStringList & aReceipt, const SPaymentData & aPaymentData, quint32 * aFDNumber)
{
	if (((getDocumentState() == EDocumentState::Opened) && !processCommand(CKasbiFR::Commands::CancelDocument)) || !receiptProcessing())
	{
		return false;
	}

	if (!processReceipt(aReceipt, false) || !processCommand(CKasbiFR::Commands::OpenDocument))
	{
		receiptProcessing();

		return false;
	}

	bool result = true;

	foreach (auto unitData, aPaymentData.unitDataList)
	{
		result = result && sale(unitData);
	}

	uint totalSum = uint(getTotalAmount(aPaymentData) * 100);
	auto totalPayTypeSum = [&] (EPayTypes::Enum aPayType) -> uint { return totalSum * int(aPaymentData.payType == aPayType); };
	QByteArray commandData =
		mFFEngine.getTLVData(CFR::FiscalFields::TaxSystem, char(aPaymentData.taxSystem)) +
		mFFEngine.getTLVData(CFR::FiscalFields::CashFiscalTotal,         totalPayTypeSum(EPayTypes::Cash)) +
		mFFEngine.getTLVData(CFR::FiscalFields::CardFiscalTotal,         totalPayTypeSum(EPayTypes::EMoney)) +
		mFFEngine.getTLVData(CFR::FiscalFields::PrePaymentFiscalTotal,   totalPayTypeSum(EPayTypes::PrePayment)) +
		mFFEngine.getTLVData(CFR::FiscalFields::PostPaymentFiscalTotal,  totalPayTypeSum(EPayTypes::PostPayment)) +
		mFFEngine.getTLVData(CFR::FiscalFields::CounterOfferFiscalTotal, totalPayTypeSum(EPayTypes::CounterOffer));

	QString cashier     = mFFEngine.getConfigParameter(CFiscalSDK::Cashier).toString();
	QString cashierINN  = mFFEngine.getConfigParameter(CFiscalSDK::CashierINN).toString();
	QString userContact = mFFEngine.getConfigParameter(CFiscalSDK::UserContact).toString();

	if (!cashier.isEmpty())     commandData += mFFEngine.getTLVData(CFR::FiscalFields::Cashier);
	if (!cashierINN.isEmpty())  commandData += mFFEngine.getTLVData(CFR::FiscalFields::CashierINN);
	if (!userContact.isEmpty()) commandData += mFFEngine.getTLVData(CFR::FiscalFields::UserContact);

	result = result && processCommand(CKasbiFR::Commands::Total, commandData);

	if (aPaymentData.agentFlag != EAgentFlags::None)
	{
		commandData = mFFEngine.getTLVData(CFR::FiscalFields::AgentFlagsReg);

		bool isBankAgent    = CFR::isBankAgent(aPaymentData.agentFlag);
		bool isPaymentAgent = CFR::isPaymentAgent(aPaymentData.agentFlag);

		if (isBankAgent)
		{
			commandData +=
				mFFEngine.getTLVData(CFR::FiscalFields::TransferOperatorAddress) +
				mFFEngine.getTLVData(CFR::FiscalFields::TransferOperatorINN) +
				mFFEngine.getTLVData(CFR::FiscalFields::TransferOperatorName) +
				mFFEngine.getTLVData(CFR::FiscalFields::AgentOperation) +
				mFFEngine.getTLVData(CFR::FiscalFields::TransferOperatorPhone);
		}
		else if (isPaymentAgent)
		{
			commandData += mFFEngine.getTLVData(CFR::FiscalFields::ProcessingPhone);
		}

		if (isBankAgent || isPaymentAgent)
		{
			commandData +=
				mFFEngine.getTLVData(CFR::FiscalFields::AgentPhone) +
				mFFEngine.getTLVData(CFR::FiscalFields::ProviderPhone);
		}

		result = result && processCommand(CKasbiFR::Commands::SendAgentData, commandData);
	}

	char payOffType = char(mFFEngine.getConfigParameter(CFiscalSDK::PayOffType).toInt());
	commandData = mFFEngine.getDigitTLVData(totalSum).left(5);
	commandData = QByteArray(1, payOffType) + commandData + QByteArray(5 - commandData.size(), ASCII::NUL);
	bool closingResult = processCommand(CKasbiFR::Commands::CloseDocument, commandData);

	if (!result || (!closingResult && (getDocumentState() == EDocumentState::Opened)))
	{
		processCommand(CKasbiFR::Commands::CancelDocument);
		receiptProcessing();

		return false;
	}

	CKasbiFR::SFSData FSData;

	if (aFDNumber && getFSData(FSData))
	{
		*aFDNumber = FSData.lastFDNumber;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool KasbiFRBase::getFiscalFields(quint32 aFDNumber, TFiscalPaymentData & aFPData, TComplexFiscalPaymentData & aPSData)
{
	if (!processCommand(CKasbiFR::Commands::StartFiscalTLVData, getHexReverted(aFDNumber, 4)))
	{
		return false;
	}

	TGetFiscalTLVData getFiscalTLVData = [&] (QByteArray & aData) -> TResult { return processCommand(CKasbiFR::Commands::GetFiscalTLVData, &aData); };

	return processFiscalTLVData(getFiscalTLVData, &aFPData, &aPSData);
}

//--------------------------------------------------------------------------------
bool KasbiFRBase::processXReport()
{
	return processCommand(CKasbiFR::Commands::StartXReport) && processCommand(CKasbiFR::Commands::EndXReport);
}

//--------------------------------------------------------------------------------
bool KasbiFRBase::setNotPrintDocument(bool aEnabled, bool /*aZReport*/)
{
	if (!aEnabled)
	{
		return true;
	}

	// TODO: ждем функционала по запросу реальной актуальной модели принтера в ПО ФР
	/*
	QString printerModel = getConfigParameter(CHardware::FR::PrinterModel).toString();

	if (!CKasbiPrinters::Models.data().values().contains(printerModel))
	{
		toLog(LogLevel::Error, mDeviceName + ": Unknown printer model " + printerModel);
		return false;
	}

	char printerModelKey = CKasbiPrinters::Models.data().key(printerModel);
	*/

	char printerModelKey = CKasbiPrinters::Virtual;

	if (!printerModelKey)
	{
		toLog(LogLevel::Error, mDeviceName + ": Cannot set auto printer model due to it is not allowed");
		return false;
	}

	CFR::TTLVList requiredPrinterModelData;
	requiredPrinterModelData.insert(CKasbiFR::FiscalFields::PrinterModel, QByteArray(1, printerModelKey));

	return checkPrintingParameters(requiredPrinterModelData);
}

//--------------------------------------------------------------------------------
