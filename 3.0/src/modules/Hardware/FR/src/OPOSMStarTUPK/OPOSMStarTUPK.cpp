/* @file Принтер MStar TUP-K на OPOS-драйвере. */

// windows
#include <objbase.h>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QUuid>
#include <QtCore/QRegExp>
#include <QtCore/QSettings>
#include <Common/QtHeadersEnd.h>

// Project
#include "OPOSMStarTUPK.h"
#include "OPOSMStarTUPKData.h"

using namespace PrinterStatusCode;
using namespace SDK::Driver;

//--------------------------------------------------------------------------------
#define NATIVE_BIND(aMethod, aType, ...) COPOS::getFunction<aType>(std::bind(&TNativeDriver::aMethod, mNativeDriver, __VA_ARGS__), OPOS_DEBUG_LOG(aMethod))

#define    INT_CALL_NATIVE(aMethod, ...) processIntMethod(NATIVE_BIND(aMethod, int, __VA_ARGS__), #aMethod##"("##""#__VA_ARGS__##")")
#define   VOID_CALL_NATIVE(aMethod, ...) mThreadProxy.invokeMethod<  void >(NATIVE_BIND(aMethod,   void , __VA_ARGS__))
#define   BOOL_CALL_NATIVE(aMethod, ...) mThreadProxy.invokeMethod<  bool >(NATIVE_BIND(aMethod,   bool , __VA_ARGS__))
#define STRING_CALL_NATIVE(aMethod, ...) mThreadProxy.invokeMethod<QString>(NATIVE_BIND(aMethod, QString, __VA_ARGS__))

//--------------------------------------------------------------------------------
OPOSMStarTUPK::OPOSMStarTUPK() : mNativeDriver(nullptr)
{
	// данные устройства
	mDeviceName = COPOSMStarTUPK::ModelName;
	mNextReceiptProcessing = false;
	mMemoryError = false;
	mFWVersion = 0;
	mProfileNames = getProfileNames();
	mNotLogResult << "PrinterState";
	mCanProcessZBuffer = true;
	//setConfigParameter(CHardware::CanOnline, true);    //TODO: раскомментить после поддержки онлайновой реализации

	// ошибки, при которых возможно выполнение определенных команд
	mUnnecessaryErrors[EFiscalPrinterCommand::ZReport].insert(DeviceStatusCode::Error::MemoryStorage);
	mUnnecessaryErrors.insert(EFiscalPrinterCommand::Encashment, TStatusCodes() << DeviceStatusCode::Error::MemoryStorage);
	mUnnecessaryErrors.insert(EFiscalPrinterCommand::XReport,    TStatusCodes() << DeviceStatusCode::Error::MemoryStorage);

	// налоги
	mTaxData.add( 0, 0);
	mTaxData.add(18, 1);
	mTaxData.add(10, 2);

	// теги
	mTagEngine = Tags::PEngine(new COPOSMStarTUPK::TagEngine());
}

//--------------------------------------------------------------------------------
bool OPOSMStarTUPK::release()
{
	bool result = TPollingOPOSFR::release();

	if (mNativeDriver)
	{
		mNativeDriver = nullptr;
	}

	return result;
}

//--------------------------------------------------------------------------------
bool OPOSMStarTUPK::checkConnectionAbility()
{
	return mNativeDriver && TPollingOPOSFR::checkConnectionAbility();
}

//--------------------------------------------------------------------------------
void OPOSMStarTUPK::initializeResources()
{
	TPollingOPOSFR::initializeResources();

	if (mCOMInitialized && !mNativeDriver && FAILED(mDriver->queryInterface(QUuid(__uuidof(TNativeDriver)), (void**)&mNativeDriver)))
	{
		toOPOS_LOG(LogLevel::Error, "Failed to query interface of the printer.");
		mNativeDriver = nullptr;
	}
}

//--------------------------------------------------------------------------------
SOPOSResult OPOSMStarTUPK::processIntMethod(TIntMethod aMethod, const QString & aFunctionData)
{
	if (!mErrors.isEmpty() && (mErrors.last().function == aFunctionData))
	{
		return OPOS::OPOS_E_FAILURE;
	}

	SOPOSResult result = TPollingOPOSFR::processIntMethod(aMethod, aFunctionData);

	if (OPOS_SUCCESS(result))
	{
		foreach (const COPOSMStarTUPK::SErrorData & errorData, mErrors)
		{
			if (errorData.function == aFunctionData)
			{
				mErrors.removeAll(errorData);
			}
		}
	}
	else if (!functionUse(mNotLogResult, aFunctionData))
	{
		if (result.extended)
		{
			int error = QString("%1").arg(result.error, 4, 16, QChar(ASCII::Zero)).mid(1, 1).toInt(0, 16);

			if (error == COPOSMStarTUPK::ExtendedErrors::FR::MemoryFlag)
			{
				mMemoryError = true;
			}
		}

		if (!functionUse(COPOSMStarTUPK::NotProcessResult, aFunctionData))
		{
			COPOSMStarTUPK::SErrorData errorData = COPOSMStarTUPK::SErrorData(aFunctionData, result.error);

			if (!result.extended)
			{
				mErrors.append(errorData);
			}
			else
			{
				toOPOS_LOG(LogLevel::Normal, QString("Printer state is %1.").arg(INT_CALL_OPOS(PrinterState).error));

				if (!mErrors.contains(errorData))
				{
					auto repeatMethod = [&] () -> SOPOSResult {mErrors.removeLast(); toOPOS_LOG(LogLevel::Normal, QString("repeat previous method %1.").arg(aFunctionData));
						return processIntMethod(aMethod, aFunctionData); };

					mErrors.append(errorData);

					switch (result.error)
					{
						case OPOS::OPOS_EFPTR_DAY_END_REQUIRED:
						{
							toOPOS_LOG(LogLevel::Normal, "Going to perform Z-report to buffer.");

							if (execZReport(true))
							{
								result = repeatMethod();
							}

							break;
						}
						//--------------------------------------------------------------------------------
						case COPOSMStarTUPK::ErrorCode::UserNotRegistered:
						{
							toOPOS_LOG(LogLevel::Normal, "Going to user registering.");

							if (OPOS_SUCCESS(INT_CALL_OPOS(SetPOSID, "", "")))
							{
								result = repeatMethod();
							}

							break;
						}
						//--------------------------------------------------------------------------------
						case COPOSMStarTUPK::ErrorCode::TransactionInProgress:
						{
							using namespace COPOSMStarTUPK::DirectIO;

							toOPOS_LOG(LogLevel::Normal, "Going to reset printer.");

							if (OPOS_SUCCESS(INT_CALL_OPOS(ResetPrinter)) && (getDocumentState() == EDocumentState::Closed))
							{
								result = OPOS::OPOS_SUCCESS;
							}

							break;
						}
						//--------------------------------------------------------------------------------
						case OPOS::OPOS_EFPTR_WRONG_STATE:
						{
							toOPOS_LOG(LogLevel::Normal, "Going to checking document state and aborting the document, if it is need.");

							if ((getDocumentState() == EDocumentState::Opened) && abortDocument())
							{
								result = repeatMethod();
							}

							break;
						}
					}
				}
			}
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
bool OPOSMStarTUPK::isConnected()
{
	if (!TPollingOPOSFR::isConnected())
	{
		return false;
	}

	QString deviceName = getConfigParameter(CHardwareSDK::ModelName).toString();
	mVerified = deviceName.contains(COPOSMStarTUPK::DeviceNameTag, Qt::CaseInsensitive) || deviceName.contains(COPOSMStarTUPK::ModelName, Qt::CaseInsensitive);

	return true;
}

//--------------------------------------------------------------------------------
bool OPOSMStarTUPK::updateParameters()
{
	if (!TOPOSFR::updateParameters())
	{
		return false;
	}

	setEnable(COPOSMStarTUPK::Parameters::TaxesPrint, true);    // Ставим печать инфы о налогах в чеках.

	// Устанавливаем синхронный режим работы.
	VOID_CALL_OPOS(SetAsyncMode, false);
	VOID_CALL_OPOS(SetDeviceEnabled, true);

	QString deviceDescription = STRING_CALL_OPOS(DeviceDescription);
	QString regExpData = QString("%1\\.%1\\.%1").arg("[0-9]+");
	QRegExp regExp = QRegExp(regExpData);

	if (regExp.indexIn(deviceDescription) != -1)
	{
		QStringList firmware = regExp.capturedTexts();
		mFWVersion = firmware[0].remove(".").toInt();
	}

	return !isFiscal() || checkTaxes();
}

//--------------------------------------------------------------------------------
bool OPOSMStarTUPK::checkTax(TVAT aVAT, CFR::Taxes::SData & aData)
{
	QString canAutoCloseSession = getConfigParameter(CHardware::FR::CanAutoCloseSession).toString();

	if (BOOL_CALL_OPOS(DayOpened))
	{
		if (canAutoCloseSession == CHardwareSDK::Values::Auto)
		{
			toLog(LogLevel::Normal, "Going to autoclosing session before checking the tax");

			if (!execZReport(true))
			{
				return false;
			}

			setConfigParameter(CHardware::FR::CanAutoCloseSession, CHardwareSDK::Values::NotUse);

			emit configurationChanged();
		}
		else if (canAutoCloseSession == CHardwareSDK::Values::NotUse)
		{
			toLog(LogLevel::Debug, "Don`t need to check the tax due to it was done earlier");
			return true;
		}
	}

	TVAT VAT;

	if (!OPOS_SUCCESS(INT_CALL_OPOS(GetVatEntry, aData.group, 0, std::ref(VAT))))
	{
		return false;
	}

	aData.deviceVAT = VAT;

	if (VAT == aVAT)
	{
		return true;
	}

	return OPOS_SUCCESS(INT_CALL_OPOS(SetVatValue, aData.group, QString::number(aVAT))) && OPOS_SUCCESS(INT_CALL_OPOS(SetVatTable));
}

//--------------------------------------------------------------------------------
bool OPOSMStarTUPK::printLine(const QVariant & aLine)
{
	return OPOS_SUCCESS(INT_CALL_OPOS(PrintNormal, OPOS::FPTR_S_RECEIPT, aLine.toString()));
}

//--------------------------------------------------------------------------------
bool OPOSMStarTUPK::receiptProcessing()
{
	using namespace COPOSMStarTUPK::DirectIO;

	QString data = COPOSMStarTUPK::PrinterCommands::Feed;
	bool feedResult = OPOS_SUCCESS(INT_CALL_OPOS(DirectIO, Printer::Command, Printer::Default, std::ref(data)));

	data = COPOSMStarTUPK::PrinterCommands::Cut;
	bool cutResult = OPOS_SUCCESS(INT_CALL_OPOS(DirectIO, Printer::Command, Printer::Default, data));

	return feedResult && cutResult;
}

//--------------------------------------------------------------------------------
bool OPOSMStarTUPK::processReceipt(const QStringList & aReceipt, bool aProcessing)
{
	if (!isPrintingNeed(aReceipt))
	{
		return true;
	}

	bool serviceOperation = getConfigParameter(CHardwareSDK::Printer::ServiceOperation).toBool();
	bool needCutting = aProcessing && ((mPrintingMode == EPrintingModes::None) || serviceOperation);
	setEnable(COPOSMStarTUPK::Parameters::AutoCutter, needCutting);

	if (!OPOS_SUCCESS(INT_CALL_OPOS(BeginNonFiscal)))
	{
		return false;
	}

	QStringList receipt = simplifyReceipt(aReceipt);

	if (!TOPOSFR::processReceipt(receipt, false) || !OPOS_SUCCESS(INT_CALL_OPOS(EndNonFiscal)))
	{
		fixError(EFiscalPrinterCommand::Print, std::bind(&OPOSMStarTUPK::abortDocument, this));

		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
void OPOSMStarTUPK::postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection)
{
	if (!mLastHandledOperations.isEmpty() &&
		aNewStatusCollection.isEmpty(EWarningLevel::Error) &&
		aOldStatusCollection.isEmpty(EWarningLevel::Error))
	{
		toOPOS_LOG(LogLevel::Normal, QString("Restore fixed operations (%1).").arg(mLastHandledOperations.size()));

		for(int i = 0; i < mLastHandledOperations.size(); ++i)
		{
			mLastHandledOperations[i]();
		}

		mLastHandledOperations.clear();
	}

	TOPOSFR::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

//--------------------------------------------------------------------------------
bool OPOSMStarTUPK::getStatus(TStatusCodes & aStatusCodes)
{
	if (!checkConnectionAbility())
	{
		aStatusCodes.insert(DeviceStatusCode::Error::ThirdPartyDriverFail);
		return true;
	}

	SOPOSResult result;

	if (!checkHealth(aStatusCodes, result))
	{
		return false;
	}

	if (result.extended)
	{
		switch (result.error)
		{
			case COPOSMStarTUPK::ExtendedErrors::Printer::Offline      :
			case COPOSMStarTUPK::ExtendedErrors::Printer::Timeout      :
			case COPOSMStarTUPK::ExtendedErrors::Printer::Unknown      : aStatusCodes.insert(Error::PrinterFR); break;	
			case COPOSMStarTUPK::ExtendedErrors::Printer::Mechanical   : aStatusCodes.insert(DeviceStatusCode::Error::MechanismPosition); break;
			case COPOSMStarTUPK::ExtendedErrors::Printer::Cutter       : aStatusCodes.insert(Error::Cutter); break;
			case COPOSMStarTUPK::ExtendedErrors::Printer::HeadOverheat : aStatusCodes.insert(Error::Temperature); break;
			case COPOSMStarTUPK::ExtendedErrors::Printer::PapperJam    : aStatusCodes.insert(Error::PaperJam); break;

			case COPOSMStarTUPK::ExtendedErrors::FR::NoEKLZ :
			case COPOSMStarTUPK::ExtendedErrors::FR::EKLZ   : aStatusCodes.insert(FRStatusCode::Error::EKLZ); break;
		}
	}

	int ZReportSlots;

	if (!getZBufferSlots(ZReportSlots, false))
	{
		aStatusCodes.insert(FRStatusCode::Error::ZBuffer);
	}
	else if (!ZReportSlots)
	{
		aStatusCodes.insert(FRStatusCode::Error::ZBufferOverflow);
	}
	else if (ZReportSlots == 1)
	{
		aStatusCodes.insert(FRStatusCode::Warning::ZBufferFull);
	}

	if (BOOL_CALL_OPOS(CoverOpen))
	{
		aStatusCodes.insert(DeviceStatusCode::Error::CoverIsOpened);
	}

	if (BOOL_CALL_OPOS(RecNearEnd))
	{
		aStatusCodes.insert(Warning::PaperNearEnd);
	}

	if (BOOL_CALL_OPOS(RecEmpty))
	{
		aStatusCodes.insert(Error::PaperEnd);
	}

	if (mFWVersion < COPOSMStarTUPK::MinRecommendedFirmware)
	{
		aStatusCodes.insert(DeviceStatusCode::Warning::Firmware);
	}

	if (mMemoryError)
	{
		aStatusCodes.insert(DeviceStatusCode::Error::MemoryStorage);
	}

	return true;
}

//--------------------------------------------------------------------------------
EDocumentState::Enum OPOSMStarTUPK::getDocumentState()
{
	int state = INT_CALL_OPOS(PrinterState).error;

	bool result = (state == OPOS::FPTR_PS_NONFISCAL) ||
	              (state == OPOS::FPTR_PS_FISCAL_RECEIPT) ||
	              (state == OPOS::FPTR_PS_FISCAL_DOCUMENT) ||
	              (state == OPOS::FPTR_PS_FISCAL_RECEIPT_ENDING) ||
	              (state == OPOS::FPTR_PS_FISCAL_RECEIPT_TOTAL);

	return result ? EDocumentState::Opened : EDocumentState::Closed;
}

//--------------------------------------------------------------------------------
bool OPOSMStarTUPK::abortDocument()
{
	TStatusCollection lastStatusCollection = mStatusCollectionHistory.lastValue();
	bool noPaper = lastStatusCollection.contains(PrinterStatusCode::Error::PaperEnd);

	int state = INT_CALL_OPOS(PrinterState).error;

	switch (state)
	{
		case OPOS::FPTR_PS_NONFISCAL:
		{
			toOPOS_LOG(LogLevel::Warning, "Aborting non-fiscal receipt.");
			QStringList text = QStringList() << QString::fromWCharArray(COPOSMStarTUPK::NonFiscalCancellation);
			processReceipt(text);

			setEnable(COPOSMStarTUPK::Parameters::AutoCutter, false);

			return OPOS_SUCCESS(INT_CALL_OPOS(EndNonFiscal));
		}
		case OPOS::FPTR_PS_FISCAL_RECEIPT_ENDING:
		case OPOS::FPTR_PS_FISCAL_RECEIPT:
		case OPOS::FPTR_PS_FISCAL_DOCUMENT:
		case OPOS::FPTR_PS_FISCAL_RECEIPT_TOTAL:
		{
			toOPOS_LOG(LogLevel::Warning, "Aborting fiscal receipt.");

			setEnable(COPOSMStarTUPK::Parameters::AutoCutter, !noPaper);

			return OPOS_SUCCESS(INT_CALL_OPOS(PrintRecVoid, "")) && OPOS_SUCCESS(INT_CALL_OPOS(EndFiscalReceipt, false));
		}
	}

	toOPOS_LOG(LogLevel::Warning, QString("PrinterState returns state %1, so can not abort the receipt.").arg(state));

	return false;
}

//--------------------------------------------------------------------------------
bool OPOSMStarTUPK::makeFiscal(const SPaymentData & aPaymentData)
{
	setEnable(COPOSMStarTUPK::Parameters::AutoCutter, true);

	VOID_CALL_OPOS(SetFiscalReceiptType, OPOS::FPTR_RT_SALES);

	bool result = OPOS_SUCCESS(INT_CALL_OPOS(BeginFiscalReceipt, true));

	if (result)
	{
		setEnable(COPOSMStarTUPK::Parameters::AutoCutter, true);

		foreach (auto unitData, aPaymentData.unitDataList)
		{
			int nameSize = unitData.name.size();
			wchar_t * name = new wchar_t[nameSize + 1];
			memset(name, ASCII::NUL, sizeof(wchar_t) * (nameSize + 1));
			unitData.name.toWCharArray(name);

			result = result && OPOS_SUCCESS(INT_CALL_NATIVE(PrintRecItem, name, sum2CY(unitData.sum), 1 * 10000, mTaxData[unitData.VAT].group, sum2CY(unitData.sum), L""));

			delete [] name;
		}

		if (result)
		{
			VOID_CALL_OPOS(SetCheckTotal, false);

			CY total(sum2CY(getTotalAmount(aPaymentData)));

			result = OPOS_SUCCESS(INT_CALL_NATIVE(PrintRecTotal, total, total, L"0")) &&
			         OPOS_SUCCESS(INT_CALL_OPOS(EndFiscalReceipt, false));
		}
		else
		{
			abortDocument();
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
bool OPOSMStarTUPK::performFiscal(const QStringList & aReceipt, const SPaymentData & aPaymentData, quint32 * /*aFDNumber*/)
{
	if (!processReceipt(aReceipt, false))
	{
		return false;
	}

	TBoolMethod fiscal  = std::bind(&OPOSMStarTUPK::makeFiscal, this, aPaymentData);
	bool result = complexFiscalDocument(fiscal, "fiscal document");

	if (!result)
	{
		fixError(EFiscalPrinterCommand::Print, std::bind(&OPOSMStarTUPK::abortDocument, this));
	}

	return result;
}

//--------------------------------------------------------------------------------
ESessionState::Enum OPOSMStarTUPK::getSessionState()
{
	return BOOL_CALL_OPOS(DayOpened) ? ESessionState::Opened : ESessionState::Closed;
}

//--------------------------------------------------------------------------------
bool OPOSMStarTUPK::performZReport(bool aPrintDeferredReports)
{
	bool printZReport = execZReport(false);
	bool printDeferred = aPrintDeferredReports && printDeferredZReports();

	return printZReport || printDeferred;
}

//TODO: попробовать сделать это в базовом классе фискальников. После того как там будет сделана фильтрация однотипных однородных ошибок.
//--------------------------------------------------------------------------------
bool OPOSMStarTUPK::fixError(EFiscalPrinterCommand::Enum aCommand, TBoolMethod aFunction)
{
	simplePoll();

	if (isFiscalReady(false, aCommand))
	{
		return false;
	}

	if (aFunction)
	{
		mLastHandledOperations.append(aFunction);
	}

	if (!mErrors.isEmpty())
	{
		mErrors.removeLast();
	}

	return true;
}

//--------------------------------------------------------------------------------
bool OPOSMStarTUPK::execZReport(bool aAuto)
{
	if (aAuto)
	{
		mNeedCloseSession = true;

		if (mOperatorPresence)
		{
			toOPOS_LOG(LogLevel::Error, "Failed to process auto-Z-report due to presence of the operator.");
			return false;
		}
	}

	toOPOS_LOG(LogLevel::Normal, "Performing Z-report.");

	if (!BOOL_CALL_OPOS(DayOpened))
	{
		toOPOS_LOG(LogLevel::Normal, "Failed to process Z-report due to the session is closed.");
		return false;
	}

	if (!aAuto)
	{
		setEnable(COPOSMStarTUPK::Parameters::AutoCutter, true);
	}

	setEnable(COPOSMStarTUPK::Parameters::ZBuffer, aAuto);
	bool success = OPOS_SUCCESS(INT_CALL_OPOS(PrintZReport));
	mNeedCloseSession = mNeedCloseSession && !success;

	if (!success)
	{
		TBoolMethod command = std::bind(&OPOSMStarTUPK::execZReport, this, aAuto);
		TBoolMethod processing = std::bind(&OPOSMStarTUPK::receiptProcessing, this);

		if (fixError(EFiscalPrinterCommand::Print, processing) && fixError(EFiscalPrinterCommand::ZReport, command))
		{
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool OPOSMStarTUPK::printDeferredZReports()
{
	toOPOS_LOG(LogLevel::Normal, "Printing deferred Z-reports.");

	setEnable(COPOSMStarTUPK::Parameters::AutoCutter, false);

	TBoolMethod command = std::bind(&OPOSMStarTUPK::printDeferredZReports, this);
	TBoolMethod processing = std::bind(&OPOSMStarTUPK::receiptProcessing, this);

	// MSVS bug #644190
	using namespace COPOSMStarTUPK::DirectIO;

	bool printDeferred = false;
	int ZReportSlots;

	if (!getZBufferSlots(ZReportSlots, true))
	{
		int i = 0;
		bool result;

		do
		{
			toOPOS_LOG(LogLevel::Normal, QString("Print %1 Z-report from buffer.").arg(++i));

			QString data;
			result = OPOS_SUCCESS(INT_CALL_OPOS(DirectIO, PrintZReport::Command, PrintZReport::Default, std::ref(data)));
		}
		while(result);

		if (!result)
		{
			fixError(EFiscalPrinterCommand::Print, processing);
			fixError(EFiscalPrinterCommand::ZReport, command);
		}

		printDeferred = i > 1;
	}
	else
	{
		toOPOS_LOG(LogLevel::Normal, QString("There are %1 filled slots in Z-report buffer.").arg(ZReportSlots));

		for (int i = 0; i < ZReportSlots; ++i)
		{
			QString data;

			if (!OPOS_SUCCESS(INT_CALL_OPOS(DirectIO, PrintZReport::Command, PrintZReport::Default, std::ref(data))))
			{
				toOPOS_LOG(LogLevel::Error, QString("Failed to print %1 z-report from buffer.").arg(i + 1));

				if (fixError(EFiscalPrinterCommand::Print, processing) && fixError(EFiscalPrinterCommand::ZReport, command))
				{
					break;
				}
			}
			else
			{
				printDeferred = true;
			}
		}
	}

	return printDeferred;
}

//--------------------------------------------------------------------------------
bool OPOSMStarTUPK::performXReport(const QStringList & aReceipt)
{
	TBoolMethod printing = std::bind(&OPOSMStarTUPK::processReceipt, this, aReceipt, mNextReceiptProcessing);
	TBoolMethod XReport  = std::bind(&OPOSMStarTUPK::processXReport, this);
	TBoolMethod command  = std::bind(&OPOSMStarTUPK::complexFiscalDocument, this, XReport, "X-report");

	bool result = printing();

	if (!result || !command())
	{
		if (result)
		{
			fixError(EFiscalPrinterCommand::Print, std::bind(&OPOSMStarTUPK::receiptProcessing, this));
		}

		fixError(EFiscalPrinterCommand::Print, printing);
		fixError(EFiscalPrinterCommand::XReport, command);
	}

	return result;
}

//--------------------------------------------------------------------------------
bool OPOSMStarTUPK::performEncashment(const QStringList & aReceipt, double aAmount)
{
	TBoolMethod printing = std::bind(&OPOSMStarTUPK::processReceipt, this, std::ref(aReceipt), mNextReceiptProcessing);
	TBoolMethod payout   = std::bind(&OPOSMStarTUPK::processPayout, this, aAmount);
	TBoolMethod command  = std::bind(&OPOSMStarTUPK::complexFiscalDocument, this, payout, "payout");

	bool result = printing();

	if (!result || !command())
	{
		if (result)
		{
			fixError(EFiscalPrinterCommand::Print, std::bind(&OPOSMStarTUPK::abortDocument, this));
		}

		fixError(EFiscalPrinterCommand::Print, printing);
		fixError(EFiscalPrinterCommand::Encashment, command);
	}

	return result;
}

//--------------------------------------------------------------------------------
bool OPOSMStarTUPK::processXReport()
{
	setEnable(COPOSMStarTUPK::Parameters::AutoCutter, true);

	return OPOS_SUCCESS(INT_CALL_OPOS(PrintXReport));
}

//--------------------------------------------------------------------------------
double OPOSMStarTUPK::getAmountInCash()
{
	using namespace COPOSMStarTUPK::DirectIO;

	QString data;
	if (!OPOS_SUCCESS(INT_CALL_OPOS(DirectIO, GetData::Command, GetData::TotalEncash, std::ref(data))))
	{
		return -1;
	}

	bool OK;
	double result = data.toDouble(&OK);

	return OK ? result : -1;
}

//--------------------------------------------------------------------------------
bool OPOSMStarTUPK::processPayout(double aAmount)
{
	using namespace COPOSMStarTUPK::DirectIO;

	setEnable(COPOSMStarTUPK::Parameters::AutoCutter, true);

	VOID_CALL_OPOS(SetFiscalReceiptType, OPOS::FPTR_RT_CASH_OUT);

	if (OPOS_SUCCESS(INT_CALL_NATIVE(BeginFiscalReceipt, true)))
	{
		CY total(sum2CY(aAmount));

		if (OPOS_SUCCESS(INT_CALL_NATIVE(PrintRecCash, total)))
		{
			VOID_CALL_NATIVE(PutCheckTotal, false);

			return OPOS_SUCCESS(INT_CALL_NATIVE(PrintRecTotal, total, total, L"0")) &&
				    OPOS_SUCCESS(INT_CALL_NATIVE(EndFiscalReceipt, false));
		}
	}

	return false;
}

//--------------------------------------------------------------------------------
void OPOSMStarTUPK::setEnable(COPOSMStarTUPK::Parameters::Enum aParameter, bool aEnable)
{
	QSettings settings("SOFTWARE\\OLEforRetail\\ServiceOPOS\\FiscalPrinter\\MstarTK\\HID", QSettings::NativeFormat);
	COPOSMStarTUPK::Parameters::Data data = COPOSMStarTUPK::Parameters::Specification[aParameter];

	settings.setValue(data.name, int(data.value * int(aEnable)));
}

//--------------------------------------------------------------------------------
bool OPOSMStarTUPK::getZBufferSlots(int & aSlots, bool aFilled)
{
	using namespace COPOSMStarTUPK::DirectIO;

	QString data;

	if (OPOS_SUCCESS(INT_CALL_OPOS(DirectIO, GetData::Command, aFilled ? GetData::FilledZBufferSlots : GetData::FreeZBufferSlots, std::ref(data))))
	{
		aSlots = data.toInt();

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------
QString OPOSMStarTUPK::getErrorDescription()
{
	return STRING_CALL_OPOS(ErrorString);
}

//--------------------------------------------------------------------------------
inline CY sum2CY(double arg)
{
	CY result = {qRound64(arg * 10000.0)};

	return result;
}

//--------------------------------------------------------------------------------
QStringList OPOSMStarTUPK::getProfileNames()
{
	return QStringList() << "mstartk";
}

//--------------------------------------------------------------------------------
