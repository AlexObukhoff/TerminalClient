/* @file Базовый фискальный регистратор. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSettings>
#include <Common/QtHeadersEnd.h>

// STL
#include <numeric>

// OPOS
#include <Common/QtHeadersBegin.h>
#pragma warning(disable: 4100) // warning C4100: 'identifier' : unreferenced formal parameter
#include <OPOS/QtWrappers/FiscalPrinter.h>
#include <Common/QtHeadersEnd.h>

// Modules
#include "PaymentProcessor/PrintConstants.h"
#include "Hardware/Common/OPOSPollingDeviceBase.h"
#include "Hardware/Common/WorkingThreadProxy.h"
#include "Hardware/Printers/PortPrintersBase.h"
#include "Hardware/FR/ProtoFR.h"

// Project
#include "FRBase.h"
#include "Hardware/FR/OFDServerData.h"
#include "Hardware/FR/FSSerialData.h"

using namespace SDK::Driver;
using namespace FRStatusCode;
using namespace ProtocolUtils;

//---------------------------------------------------------------------------
template class FRBase<PrinterBase<OPOSPollingDeviceBase<ProtoFR, OPOS::OPOSFiscalPrinter>>>;
template class FRBase<SerialPrinterBase<PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoFR>>>>>;
template class FRBase<PortPrinterBase<PrinterBase<TCPDeviceBase<PortPollingDeviceBase<ProtoFR>>>>>;

//--------------------------------------------------------------------------------
template <class T>
FRBase<T>::FRBase(): mFFEngine(mLog)
{
	// описания для кодов статусов принтеров
	mStatusCodesSpecification = DeviceStatusCode::PSpecifications(new FRStatusCode::CSpecifications());

	// ошибки, при которых возможно выполнение определенных команд
	mUnnecessaryErrors.insert(EFiscalPrinterCommand::Print, getFiscalStatusCodes(EWarningLevel::Error));
	mUnnecessaryErrors.insert(EFiscalPrinterCommand::ZReport, TStatusCodes()
		<< Error::ZBufferOverflow
		<< Error::NeedCloseSession
		<< DeviceStatusCode::Error::Initialization);
	mUnnecessaryErrors.insert(EFiscalPrinterCommand::Encashment, TStatusCodes()
		<< Error::ZBufferOverflow
		<< Error::NeedCloseSession);
	mUnnecessaryErrors.insert(EFiscalPrinterCommand::XReport, TStatusCodes()
		<< Error::ZBufferOverflow
		<< Error::NeedCloseSession
		<< Error::EKLZ
		<< Error::ZBuffer);

	// данные устройства
	setInitialData();
	mRegion = ERegion::RF;
	mIsOnline = false;
	mNextReceiptProcessing = true;
	mCanProcessZBuffer = false;
	mFFEngine.setCodec(mCodec);

	mRecoverableErrors.insert(FRStatusCode::Error::Taxes);

	setConfigParameter(CHardwareSDK::FR::CanWithoutPrinting, false);
	setConfigParameter(CHardwareSDK::FR::WithoutPrinting, CHardwareSDK::Values::Auto);
	setConfigParameter(CHardware::CanSoftReboot, false);

	mOFDFiscalParameters
		<< CFR::FiscalFields::Cashier
		<< CFR::FiscalFields::CashierINN
		<< CFR::FiscalFields::UserContact
		<< CFR::FiscalFields::TaxSystem;

	mPayTypeData.add(EPayTypes::Cash,   1);
	mPayTypeData.add(EPayTypes::EMoney, 2);
}

//--------------------------------------------------------------------------------
template <class T>
void FRBase<T>::setInitialData()
{
	T::setInitialData();

	mEKLZError         = false;
	mFMError           = false;
	mFSError           = false;
	mFSOfflineEnd      = false;
	mZBufferFull       = false;
	mZBufferOverflow   = false;
	mPrinterCollapse   = false;
	mFiscalized        = true;
	mZBufferError      = false;
	mFiscalCollapse    = false;
	mOFDDataError      = false;
	mNeedCloseSession  = false;
	mNotPrintingError  = false;
	mCashierINNError   = false;
	mTaxError          = false;
	mWrongTaxOnPayment = false;
	mWrongFiscalizationSettings = false;
	mNeedTimeSynchronization    = false;

	mEKLZ              = true;
	mWhiteSpaceZBuffer = -1;
	mLastOpenSession   = QDateTime::currentDateTime();
	mOFDNotSentCount   = 0;
	mOFDDTMark         = QDateTime::currentDateTime();
	mFFDFR             = EFFD::Unknown;
	mFFDFS             = EFFD::Unknown;

	mTaxSystems.clear();
	mAgentFlags.clear();
	mOperationModes.clear();
	mINN.clear();
	mRNM.clear();
	mSerial.clear();
	mFSSerialNumber.clear();

	mFFEngine.setDeviceName(mDeviceName);

	removeDeviceParameter(CHardwareSDK::FR::AgentFlags);
}

//--------------------------------------------------------------------------------
template <class T>
void FRBase<T>::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	mFFEngine.setDeviceName(mDeviceName);

	for (auto it = aConfiguration.begin(); it != aConfiguration.end(); ++it)
	{
		if (mFFData.getKey(it.key()))
		{
			mFFEngine.setConfigParameter(it.key(), it.value());
		}
	}

	T::setDeviceConfiguration(aConfiguration);

	if (aConfiguration.contains(CHardware::FR::FiscalMode))
	{
		bool isFiscal = aConfiguration[CHardware::FR::FiscalMode].toBool();
		mStatusCodesSpecification.dynamicCast<FRStatusCode::CSpecifications>()->setFiscal(isFiscal);
	}

	if (aConfiguration.contains(CHardwareSDK::FR::WithoutPrinting))
	{
		QString withoutPrinting = aConfiguration.value(CHardwareSDK::FR::WithoutPrinting).toString();

		if (withoutPrinting != CHardwareSDK::Values::Auto)
		{
			emit configurationChanged();

			if (mConnected && (mInitialized == ERequestStatus::Success))
			{
				checkNotPrinting();
			}
		}
	}

	if (aConfiguration.contains(CFiscalSDK::CashierINN))
	{
		QString cashierINN = getConfigParameter(CFiscalSDK::CashierINN).toString().simplified();
		setConfigParameter(CFiscalSDK::CashierINN, cashierINN);
		mFFEngine.setConfigParameter(CFiscalSDK::CashierINN, cashierINN);

		mCashierINNError = !mFFEngine.checkINN(cashierINN, CFR::INN::Person::Natural);
	}

	if (aConfiguration.contains(CHardwareSDK::FR::DealerTaxSystem)) mFFEngine.setConfigParameter(CHardwareSDK::FR::DealerTaxSystem, getConfigParameter(CHardwareSDK::FR::DealerTaxSystem));
	if (aConfiguration.contains(CHardwareSDK::FR::DealerAgentFlag)) mFFEngine.setConfigParameter(CHardwareSDK::FR::DealerAgentFlag, getConfigParameter(CHardwareSDK::FR::DealerAgentFlag));
	if (aConfiguration.contains(CHardwareSDK::OperatorPresence))    mFFEngine.setConfigParameter(CHardwareSDK::OperatorPresence, mOperatorPresence);

	mFFEngine.checkDealerTaxSystem(mInitialized, !isAutoDetecting());
	mFFEngine.checkDealerAgentFlag(mInitialized, !isAutoDetecting());
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::isConnected()
{
	bool result = T::isConnected();

	mFFEngine.setDeviceName(mDeviceName);

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
void FRBase<T>::finaliseOnlineInitialization()
{
	checkTaxes();

	TTaxSystemData taxSystemData;

	foreach(char taxSystem, mTaxSystems)
	{
		taxSystemData.insert(ETaxSystems::Enum(taxSystem), CFR::TaxSystems[taxSystem]);
	}

	TAgentFlagsData agentFlagsData;

	for (auto it = CFR::AgentFlags.data().begin(); it != CFR::AgentFlags.data().end(); ++it)
	{
		agentFlagsData.insert(EAgentFlags::Enum(it.key()), it.value());
	}

	setConfigParameter(CHardwareSDK::FR::AgentFlagsData, QVariant().fromValue(agentFlagsData));

	if (!mAgentFlags.isEmpty())
	{
		agentFlagsData.clear();

		foreach(char agentFlag, mAgentFlags)
		{
			agentFlagsData.insert(EAgentFlags::Enum(agentFlag), CFR::AgentFlags[agentFlag]);
		}

		setDeviceParameter( CDeviceData::FR::AgentFlags, QStringList(agentFlagsData.values()).join(", "));
		setConfigParameter(CHardwareSDK::FR::AgentFlags, QVariant().fromValue(agentFlagsData.keys()));
	}

	TOperationModeData operationModeData;

	foreach(char operationMode, mOperationModes)
	{
		int field = CFR::OperationModeData[operationMode];
		operationModeData.insert(EOperationModes::Enum(operationMode), mFFData[field].translationPF);
	}

	QStringList operationModeDescriptions = operationModeData.values();

	if (mFFEngine.getConfigParameter(CFiscalSDK::LotteryMode,       0).toInt()) operationModeDescriptions += mFFData[CFR::FiscalFields::LotteryMode].translationPF;
	if (mFFEngine.getConfigParameter(CFiscalSDK::GamblingMode,      0).toInt()) operationModeDescriptions += mFFData[CFR::FiscalFields::GamblingMode].translationPF;
	if (mFFEngine.getConfigParameter(CFiscalSDK::ExcisableUnitMode, 0).toInt()) operationModeDescriptions += mFFData[CFR::FiscalFields::ExcisableUnitMode].translationPF;
	if (mFFEngine.getConfigParameter(CFiscalSDK::InAutomateMode,    0).toInt()) operationModeDescriptions += mFFData[CFR::FiscalFields::InAutomateMode].translationPF;

	if (FS::Data.contains(mFSSerialNumber))
	{
		FS::SData FSData = FS::Data[mFSSerialNumber];

		setDeviceParameter(CDeviceData::FS::Expiration, QString("%1 months").arg(FSData.expiration));
		setDeviceParameter(CDeviceData::FS::FFD, FSData.FFD);
		setDeviceParameter(CDeviceData::FS::Provider, FSData.provider);

		if (!FSData.revision.isEmpty())
		{
			setDeviceParameter(CDeviceData::FS::Revision, FSData.revision);
		}
	}

	setDeviceParameter(CDeviceData::FS::SerialNumber, mFSSerialNumber);
	setDeviceParameter(CDeviceData::FR::TaxSystems, QStringList(taxSystemData.values()).join(", "));
	setDeviceParameter(CDeviceData::FR::OperationModes, operationModeDescriptions.join(", "));
	setDeviceParameter(CDeviceData::FR::FFDFR, CFR::FFD[mFFDFR].description);
	setDeviceParameter(CDeviceData::FR::FFDFS, CFR::FFD[mFFDFS].description);

	setConfigParameter(CHardwareSDK::FR::FSSerialNumber, mFSSerialNumber);
	setConfigParameter(CHardwareSDK::FR::TaxSystems, QVariant().fromValue(taxSystemData));

	if (!containsConfigParameter(CHardwareSDK::FR::FiscalFieldData))
	{
		QList<CFR::FiscalFields::SData> fiscalFieldValues = mFFData.data().values();
		TFiscalFieldData fiscalFieldData;

		for (int i = 0; i < fiscalFieldValues.size(); ++i)
		{
			CFR::FiscalFields::SData & data = fiscalFieldValues[i];
			fiscalFieldData.insert(data.textKey, SFiscalFieldData(data.translationPF, data.isMoney));
		}

		setConfigParameter(CHardwareSDK::FR::FiscalFieldData, QVariant::fromValue(fiscalFieldData));

		mFFEngine.setConfigParameter(CFiscalSDK::SerialFSNumber, getDeviceParameter(CDeviceData::FS::SerialNumber));
		mFFEngine.setConfigParameter(CFiscalSDK::SerialFRNumber, getDeviceParameter(CDeviceData::SerialNumber));
		mFFEngine.setConfigParameter(CFiscalSDK::INN,            getDeviceParameter(CDeviceData::FR::INN));
		mFFEngine.setConfigParameter(CFiscalSDK::RNM,            getDeviceParameter(CDeviceData::FR::RNM));
	}

	checkNotPrinting();
}

//--------------------------------------------------------------------------------
template <class T>
void FRBase<T>::finaliseInitialization()
{
	setDeviceParameter(CDeviceData::FR::OnlineMode, mIsOnline);

	if (mConnected)
	{
		setConfigParameter(CHardwareSDK::SerialNumber, mSerial);
		setDeviceParameter(CDeviceData::SerialNumber, mSerial);
		setDeviceParameter(CDeviceData::FR::INN, mINN);
		setDeviceParameter(CDeviceData::FR::RNM, mRNM);
		setDeviceParameter(CDeviceData::FR::CanProcessZBuffer, mCanProcessZBuffer ? "true" : "false");

		if (mIsOnline)
		{
			finaliseOnlineInitialization();
		}

		QStringList taxes;

		foreach (auto taxData, mTaxData.data().values())
		{
			taxes << QString::number(taxData.deviceVAT);
		}

		auto pred = [] (const QString & arg1, const QString & arg2) -> bool { return arg1.toLower() > arg2.toLower(); };
		qSort(taxes.begin(), taxes.end(), pred);
		setDeviceParameter(CDeviceData::FR::Taxes, taxes.join(", "));
	}

	T::finaliseInitialization();

	QVariantMap configData = mFFEngine.getConfigParameter(CHardware::ConfigData).toMap();
	configData.insert(CHardwareSDK::FR::DealerVAT, getConfigParameter(CHardwareSDK::FR::DealerVAT));
	setConfigParameter(CHardware::ConfigData, configData);

	if (mConnected && mIsOnline)
	{
		initializeZReportByTimer();
	}
}

//--------------------------------------------------------------------------------
template <class T>
void FRBase<T>::initializeZReportByTimer()
{
	QString configOpeningTime = getConfigParameter(CHardware::FR::SessionOpeningTime).toString();
	QTime OT = QTime::fromString(configOpeningTime, CFR::TimeLogFormat);

	QTime ZT = getConfigParameter(CHardwareSDK::FR::ZReportTime).toTime();
	QTime CT = QTime::currentTime();
	QString logZReport;

	if (ZT.isValid())
	{
		if (ZT == CT)
		{
			logZReport = "ZT == CT -> Z-report time == current time";
		}
		else if (OT.isValid())
		{
			QString log = "session opening time < Z-report time < current time";

			if ((CT > ZT) && (OT >= CT))
			{
				logZReport = QString("(CT > ZT) && (OT >= CT) -> %1, session opening time < 0").arg(log);
			}
			else if ((CT > ZT) && (OT < ZT))
			{
				logZReport = QString("(CT > ZT) && (OT < ZT) -> %1").arg(log);
			}
			else if ((CT <= OT) && (OT < ZT))
			{
				logZReport = QString("(CT <= OT) && (OT < ZT) -> %1, session opening time and Z-report time < 0").arg(log);
			}
		}
		else if (!containsConfigParameter(CHardware::FR::SessionOpeningTime))
		{
			toLog(LogLevel::Normal, mDeviceName + ": No session opening time");
		}
		else if (configOpeningTime.isEmpty())
		{
			toLog(LogLevel::Warning, mDeviceName + ": Session opening time is empty");
		}
		else
		{
			toLog(LogLevel::Warning, mDeviceName + ": Wrong session opening time = " + configOpeningTime);
		}
	}
	else if (containsConfigParameter(CHardwareSDK::FR::ZReportTime))
	{
		toLog(LogLevel::Warning, mDeviceName + ": Wrong Z-report time");
	}

	if (!logZReport.isEmpty())
	{
		toLog(LogLevel::Normal, mDeviceName + ": Performing Z-report due to " + logZReport);
		execZReport(true);
	}

	checkZReportByTimer();
}

//---------------------------------------------------------------------------
template <class T>
void FRBase<T>::onExecZReport()
{
	toLog(LogLevel::Normal, mDeviceName + ": Z-report timer has fired");

	execZReport(true);

	checkZReportByTimer();
}

//---------------------------------------------------------------------------
template <class T>
void FRBase<T>::checkZReportByTimer()
{
	QTime ZReportTime = getConfigParameter(CHardwareSDK::FR::ZReportTime).toTime();

	if (!ZReportTime.isValid())
	{
		toLog(LogLevel::Error, mDeviceName + ": Wrong Z-report time");
		return;
	}

	QTime current = QTime::currentTime();
	int delta = current.msecsTo(ZReportTime);

	if (delta < 0)
	{
		delta += CFR::MSecsInDay;
	}

	toLog(LogLevel::Normal, mDeviceName + ": Timer for Z-report is scheduled for " + ZReportTime.toString(CFR::TimeLogFormat));

	QTimer::singleShot(delta, this, SLOT(onExecZReport()));
}

//---------------------------------------------------------------------------
template <class T>
bool FRBase<T>::checkTaxSystems(char aData)
{
	return mFFEngine.checkTaxSystems(aData, mTaxSystems);
}

//---------------------------------------------------------------------------
template <class T>
bool FRBase<T>::checkAgentFlags(char aData)
{
	bool result = mFFEngine.checkAgentFlags(aData, mAgentFlags);

	if ((mAgentFlags.size() >= 1) || (mFFDFR >= EFFD::F105))
	{
		mOFDFiscalParameters.insert(CFR::FiscalFields::AgentFlagsReg);

		//mOFDFiscalParametersOnSale.insert(FiscalFields::ProviderINN);
		//mOFDFiscalParametersOnSale.insert(FiscalFields::AgentFlag);
	}
	else
	{
		mOFDFiscalParameters.remove(CFR::FiscalFields::AgentFlagsReg);

		mOFDFiscalParametersOnSale.remove(CFR::FiscalFields::ProviderINN);
		mOFDFiscalParametersOnSale.remove(CFR::FiscalFields::AgentFlag);
	}

	return result;
}

//---------------------------------------------------------------------------
template <class T>
bool FRBase<T>::checkOperationModes(char aData)
{
	bool result = mFFEngine.checkOperationModes(aData, mOperationModes);

	if (!mOperatorPresence && !mFiscalServerPresence && mOperationModes.contains(char(EOperationModes::Internet)))
	{
		mWrongFiscalizationSettings = true;
		toLog(LogLevel::Error, mDeviceName + ": There is \"FR for internet\" flag and no operator");
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
void FRBase<T>::checkDateTime()
{
	QDateTime deviceDT = getDateTime();

	if (!deviceDT.isValid())
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to get date and time from FR");
		return;
	}

	QString FSDifferenceDT;
	int secs = QDateTime::currentDateTime().secsTo(deviceDT);

	if (secs)
	{
		bool fallsBehind = secs < 0;
		FSDifferenceDT += fallsBehind ? "- " : "+ ";
		secs = std::abs(secs);
		int days = secs/CFR::SecsInDay;

		if (days)
		{
			FSDifferenceDT += QString("%1 day(s), ").arg(days);
			mNeedTimeSynchronization = fallsBehind;
		}

		QTime time = QTime(0, 0).addSecs(secs % CFR::SecsInDay);
		FSDifferenceDT += time.toString(CFR::TimeLogFormat);
	}
	else
	{
		FSDifferenceDT += "none";
	}

	setDeviceParameter(CDeviceData::FS::DifferenceDT, FSDifferenceDT);
}

//---------------------------------------------------------------------------
template <class T>
bool FRBase<T>::setNotPrintDocument(bool /*aEnabled*/, bool /*aZReport*/)
{
	return !getConfigParameter(CHardwareSDK::FR::CanWithoutPrinting).toBool() ||
	       (getConfigParameter(CHardwareSDK::FR::WithoutPrinting).toString() == CHardwareSDK::Values::Auto);
}

//---------------------------------------------------------------------------
template <class T>
bool FRBase<T>::checkNotPrinting(bool aEnabled, bool aZReport)
{
	mNotPrintingError = false;
	bool canWithoutPrinting = getConfigParameter(CHardwareSDK::FR::CanWithoutPrinting).toBool();
	bool canZReportWithoutPrinting = getConfigParameter(CHardware::FR::CanZReportWithoutPrinting, false).toBool();

	if (!canWithoutPrinting && aEnabled && !(canZReportWithoutPrinting && aZReport))
	{
		return true;
	}

	bool notPrintDocument = aEnabled || (getConfigParameter(CHardwareSDK::FR::WithoutPrinting).toString() == CHardwareSDK::Values::Use);

	if (!WorkingThreadProxy(&mThread).invokeMethod<bool>(std::bind(&FRBase<T>::setNotPrintDocument, this, notPrintDocument, aZReport)) && canWithoutPrinting)
	{
		mNotPrintingError = true;

		toLog(LogLevel::Error, mDeviceName + QString(": Failed to check %1 %2 printing")
			.arg(notPrintDocument ? "disabling" : "enabling").arg(aZReport ? "Z-report" : "fiscal document"));
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
template <class T>
bool FRBase<T>::isFiscal() const
{
	return getConfigParameter(CHardware::FR::FiscalMode).toBool();
}

//---------------------------------------------------------------------------
template <class T>
bool FRBase<T>::isOnline() const
{
	return mIsOnline;
}

//---------------------------------------------------------------------------
template <class T>
bool FRBase<T>::checkTaxes()
{
	if (mRegion == ERegion::RF)
	{
		CFR::adjustRFVAT(mTaxData.data());

		if (mIsOnline && mTaxSystems.isEmpty())
		{
			if (mInitialized == ERequestStatus::Success)
			{
				mTaxError = true;
			}

			return true;
		}
	}

	mTaxError = false;
	TVAT dealerVAT = TVAT(getConfigParameter(CHardwareSDK::FR::DealerVAT).toDouble());
	bool dealerVATCritical = (dealerVAT == 18) || (dealerVAT == 20);

	for (auto it = mTaxData.data().begin(); it != mTaxData.data().end(); ++it)
	{
		TVAT tax = it.key();
		it->description = CFR::VATTr[tax];
		it->deviceVAT = -1;

		if (checkTax(tax, it.value()))
		{
			it->deviceVAT = tax;
		}
		else if ((mRegion != ERegion::RF) || (tax != 20) || dealerVATCritical)
		{
			mTaxError = true;
		}
	}

	return !mTaxError || !isFiscal();
}

//---------------------------------------------------------------------------
template <class T>
void FRBase<T>::checkFSFlags(char aFlags, TStatusCodes & aStatusCodes)
{
	for (auto it = CFR::FSFlagData.data().begin(); it != CFR::FSFlagData.data().end(); ++it)
	{
		if (aFlags & it.key())
		{
			aStatusCodes.insert(it.value());
		}
	}
}

//---------------------------------------------------------------------------
template <class T>
void FRBase<T>::checkOFDNotSentCount(int aOFDNotSentCount, TStatusCodes & aStatusCodes)
{
	if (mOperationModes.contains(EOperationModes::Autonomous))
	{
		return;
	}

	if (aOFDNotSentCount < 0)
	{
		aStatusCodes.insert(DeviceStatusCode::Warning::Unknown);
	}
	else if (!mOperationModes.contains(EOperationModes::Autonomous))
	{
		QDateTime currentDT =  QDateTime::currentDateTime();

		if (!aOFDNotSentCount || (aOFDNotSentCount < mOFDNotSentCount))
		{
			mOFDDTMark = currentDT;
		}
		else if (mOFDDTMark.secsTo(currentDT) >= CFR::OFDConnectionTimeout)
		{
			aStatusCodes.insert(FRStatusCode::Warning::OFDNoConnection);
		}

		mOFDNotSentCount = aOFDNotSentCount;
	}
}

//--------------------------------------------------------------------------------
template<class T>
bool FRBase<T>::checkOFDData(const QByteArray & aAddressData, const QByteArray & aPortData)
{
	QString addressData = aAddressData.toLower().simplified();
	CHardware::SURL URL(addressData);
	CHardware::SIP IP(addressData);

	int portValue = aPortData.simplified().toHex().toUShort(0, 16);
	CHardware::SPort port(QString::number(portValue));

	removeDeviceParameter(CDeviceData::FR::OFDServer);

	if ((!URL.valid && !IP.valid) || !port.valid)
	{
		setDeviceParameter(CDeviceData::Address, addressData, CDeviceData::FR::OFDServer);
		setDeviceParameter(CDeviceData::Port, portValue, CDeviceData::FR::OFDServer);
		QStringList log;

		if (URL.value.isEmpty()) log << "URL is empty"; else if (URL.valid ||  !IP.valid) log << "URL " + URL.value;
		if  (IP.value.isEmpty()) log <<  "IP is empty"; else if  (IP.valid || !URL.valid) log <<  "IP " +  IP.value;

		log << (log.isEmpty() ? "Port " : "port ") + port.value;
		toLog(LogLevel::Warning, mDeviceName + ": OFD parameter(s) are not valid. " + log.join(", "));

		return false;
	}

	QMap<QString, CHardware::SOFDData> OFDData = CHardware::OFDData().data();

	auto it = std::find_if(OFDData.begin(), OFDData.end(), [&] (const CHardware::SOFDData & aOFDData) -> bool
	{
		bool testURL = URL.valid && aOFDData.testAddress.URL.valid && (URL.value == aOFDData.testAddress.URL.value);
		bool testIP  =  IP.valid && aOFDData.testAddress.IP.valid  && ( IP.value == aOFDData.testAddress.IP.value);

		return (testURL && (aOFDData.address.URL.value != aOFDData.testAddress.URL.value)) ||
		       (testIP  && (aOFDData.address.IP.value  != aOFDData.testAddress.IP.value))  ||
		       ((testURL || testIP) && (port.value == aOFDData.testAddress.port.value));
	});

	if (it != OFDData.end())
	{
		setDeviceParameter(CDeviceData::FR::OFDServer, it.key() + " (test)");

		if (port.value != it->testAddress.port.value)
		{
			toLog(LogLevel::Warning, mDeviceName + ": The port is not accorded of the URL or IP");
			return false;
		}

		toLog(LogLevel::Warning, mDeviceName + ": It is used test OFD parameters instead of working ones");

		return true;
	}

	it = std::find_if(OFDData.begin(), OFDData.end(), [&] (const CHardware::SOFDData & aOFDData) -> bool
	{
		bool testURL = URL.valid && aOFDData.address.URL.valid && (URL.value == aOFDData.address.URL.value);
		bool testIP  =  IP.valid && aOFDData.address.IP.valid  && ( IP.value == aOFDData.address.IP.value);

		return testURL || testIP;
	});

	if (it == OFDData.end())
	{
		setDeviceParameter(CDeviceData::Address, addressData, CDeviceData::FR::OFDServer);
		setDeviceParameter(CDeviceData::Port, portValue, CDeviceData::FR::OFDServer);
	}
	else
	{
		setDeviceParameter(CDeviceData::FR::OFDServer, it.key());

		if (port.value != it->address.port.value)
		{
			toLog(LogLevel::Warning, mDeviceName + ": The port is not accorded of the URL or IP");
			return false;
		}
	}

	return true;
}

//---------------------------------------------------------------------------
template <class T>
void FRBase<T>::cleanStatusCodes(TStatusCodes & aStatusCodes)
{
	T::cleanStatusCodes(aStatusCodes);

	// фильтр виртуального конца ФН, если есть ошибка окончания ФН
	if (aStatusCodes.contains(Error::FSEnd))
	{
		aStatusCodes.remove(Warning::FSVirtualEnd);
	}

	// фильтр ошибки ФН, если есть конкретная ошибка ФН или необходимо подключение к ОФД
	if (aStatusCodes.contains(Error::FSEnd) || aStatusCodes.contains(Error::NeedOFDConnection))
	{
		aStatusCodes.remove(Error::FS);
	}

	// фильтр предупреждения ЭКЛЗ скоро кончится, если есть ошибка ЭКЛЗ
	if (aStatusCodes.contains(Error::EKLZ))
	{
		aStatusCodes.remove(Warning::EKLZNearEnd);
	}

	// фильтр предупреждения Фискальная память скоро кончится, если есть ошибка фискальной памяти
	if (aStatusCodes.contains(Error::FM))
	{
		aStatusCodes.remove(Warning::FiscalMemoryNearEnd);
	}

	// фильтр предупреждения Буфер Z-отчетов заполнен и ошибки необходимости Z-отчета, если он уже переполнен
	if (aStatusCodes.contains(Error::ZBufferOverflow))
	{
		aStatusCodes.remove(Warning::ZBufferFull);
		aStatusCodes.remove(Error::NeedCloseSession);
	}

	// фильтр предупреждения Буфер Z-отчетов заполнен и ошибки его переполнения, если есть сбой последнего
	if (aStatusCodes.contains(Error::ZBuffer))
	{
		aStatusCodes.remove(Warning::ZBufferFull);
		aStatusCodes.remove(Error::ZBufferOverflow);
	}

	// фильтр ошибки инициализации, если есть ошибка необходимости Z-отчета
	if (aStatusCodes.contains(Error::NeedCloseSession))
	{
		aStatusCodes.remove(DeviceStatusCode::Error::Initialization);
	}
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::isDeviceReady(bool aOnline)
{
	return canCheckReady(aOnline) && isPossible(aOnline, EFiscalPrinterCommand::Print);
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::isFiscalReady(bool aOnline, EFiscalPrinterCommand::Enum aCommand)
{
	if ((aCommand != EFiscalPrinterCommand::Print) && !isFiscal())
	{
		return false;
	}

	return canCheckReady(aOnline) && isPossible(aOnline, aCommand);
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::canProcessZBuffer()
{
	return mConnected && (mInitialized != ERequestStatus::InProcess) && mCanProcessZBuffer;
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::printFiscal(const QStringList & aReceipt, const SPaymentData & aPaymentData, quint32 * aFDNumber)
{
	CFR::ConfigCleaner configCleaner(&mFFEngine);

	if (!isFiscal())
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to process command due to nonfiscal mode");
		return false;
	}

	if (aFDNumber)
	{
		*aFDNumber = 0;
	}

	return processNonReentrant(std::bind(&FRBase::processFiscal, this, std::ref(aReceipt), std::ref(aPaymentData), aFDNumber));
}

//--------------------------------------------------------------------------------
template <class T>
void FRBase<T>::checkUnitNames(SPaymentData & aPaymentData)
{
	int maxUnitNameSize = CFR::FFD[mFFDFS].maxUnitNameSize;

	for (int i = 0; i < aPaymentData.unitDataList.size(); ++i)
	{
		QString & name = aPaymentData.unitDataList[i].name;

		for (auto it = CPrinters::AutoCorrection.data().begin(); it != CPrinters::AutoCorrection.data().end(); ++it)
		{
			name = name.replace(it.key(), it.value());
		}

		name = name.simplified();

		if (mIsOnline)
		{
			name = name.left(maxUnitNameSize);
		}
	}
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::processFiscal(const QStringList & aReceipt, const SPaymentData & aPaymentData, uint * aFDNumber)
{
	SPaymentData paymentData(aPaymentData);
	TUnitDataList & unitDataList = paymentData.unitDataList;

	if ((mRegion == ERegion::RF) && CFR::isRFVAT20())
	{
		for (int i = 0; i < unitDataList.size(); ++i)
		{
			if (unitDataList[i].VAT == 18)
			{
				unitDataList[i].VAT = 20;
			}
		}
	}

	bool taxesOK = checkTaxesOnPayment(paymentData);
	mWrongTaxOnPayment = mWrongTaxOnPayment || !taxesOK;

	if (!checkAmountsOnPayment(paymentData) ||
	    !checkSumInCash(paymentData)        ||
	    !checkPayTypeOnPayment(paymentData) ||
		!checkNotPrinting()                 ||
	    !mFFEngine.checkTaxSystemOnPayment(paymentData) ||
	    !mFFEngine.checkAgentFlagOnPayment(paymentData) ||
		!taxesOK)
	{
		return false;
	}

	checkUnitNames(paymentData);
	addFiscalFieldsOnPayment(paymentData);

	ESessionState::Enum sessionState = getSessionState();

	if (sessionState == ESessionState::Error)
	{
		toLog(LogLevel::Normal, mDeviceName + ": Failed to get session state");
	}
	else if (sessionState == ESessionState::Expired)
	{
		if (!execZReport(true))
		{
			return false;
		}

		sessionState = ESessionState::Closed;
	}

	if ((sessionState == ESessionState::Closed) && !openFRSession())
	{
		return false;
	}

	QStringList receipt = simplifyReceipt(aReceipt);

	return performFiscal(receipt, paymentData, aFDNumber);
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::processFiscalFields(quint32 aFDNumber, TFiscalPaymentData & aFPData, TComplexFiscalPaymentData & aPSData)
{
	if (!getFiscalFields(aFDNumber, aFPData, aPSData))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to process fiscal fields for FD " + QString::number(aFDNumber));
		return false;
	}

	mFFEngine.filterAfterPayment(aFPData, aPSData);
	mFFEngine.checkFPData(aFPData, CFR::FiscalFields::FDNumber, aFDNumber);

	if (!aFPData.contains(CFiscalSDK::SessionNumber))
	{
		int sessionNumber = getSessionNumber();

		if (sessionNumber)
		{
			mFFEngine.checkFPData(aFPData, CFR::FiscalFields::SessionNumber, sessionNumber);
		}
	}

	toLog(LogLevel::Normal, mDeviceName + ": Fiscal payment data:\n" + mFFEngine.getFPDataLog(aFPData));
	QString log;

	foreach(const TFiscalPaymentData FPData, aPSData)
	{
		log += mFFEngine.getFPDataLog(FPData);
	}

	toLog(LogLevel::Normal, mDeviceName + ": Payoff subject data:\n" + log);

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::checkFiscalFields(quint32 aFDNumber, TFiscalPaymentData & aFPData, TComplexFiscalPaymentData & aPSData)
{
	aFPData.clear();
	aPSData.clear();

	return WorkingThreadProxy(&mThread).invokeMethod<bool>(std::bind(&FRBase<T>::processFiscalFields, this, aFDNumber, std::ref(aFPData), std::ref(aPSData)));
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::setOFDParameters()
{
	using namespace CFR::FiscalFields;

	foreach (auto parameter, mOFDFiscalParameters)
	{
		ERequired::Enum required = mFFData[parameter].required;
		bool critical = (required == ERequired::Yes) || (mOperatorPresence && (ERequired::PM));

		if (!setTLV(parameter) && critical)
		{
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::setOFDParametersOnSale(const SUnitData & aUnitData)
{
	if (mOFDFiscalParametersOnSale.contains(CFR::FiscalFields::ProviderINN))
	{
		QString providerINN = aUnitData.providerINN.simplified().leftJustified(CFR::INN::Person::Natural);
		mFFEngine.setConfigParameter(CFiscalSDK::ProviderINN, providerINN);
	}

	foreach (auto parameter, mOFDFiscalParametersOnSale)
	{
		if (!setTLV(parameter, true))
		{
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
QString FRBase<T>::getVATLog(const TVATs & aVATs) const
{
	QStringList result;

	foreach(auto VAT, aVATs)
	{
		result << QString::number(VAT);
	}

	return result.join(", ");
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::printZReport(bool aPrintDeferredReports)
{
	if (!isFiscal())
	{
		toLog(LogLevel::Error, getName() + ": Failed to process command due to nonfiscal mode");
		return false;
	}

	return processNonReentrant(std::bind(&FRBase::performZReport, this, aPrintDeferredReports));
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::printXReport(const QStringList & aReceipt)
{
	mNextDocument = true;

	return processNonReentrant(std::bind(&FRBase::performXReport, this, std::ref(aReceipt)));
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::processEncashment(const QStringList & aReceipt, double aAmount)
{
	double amountInCash = WorkingThreadProxy(&mThread).invokeMethod<double>(std::bind(&FRBase<T>::getAmountInCash, this));

	if (amountInCash < 0)
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to get valid amount in cash");
	}
	else
	{
		LogLevel::Enum level = amountInCash ? LogLevel::Normal : LogLevel::Error;
		toLog(level, mDeviceName + QString(": Amount in cash = %1").arg(amountInCash, 0, 'f', 2));
	}

	if (aAmount != DBL_MAX)
	{
		LogLevel::Enum level = (aAmount > 0) ? LogLevel::Normal : LogLevel::Error;
		toLog(level, mDeviceName + QString(": Amount to payout = %1").arg(aAmount, 0, 'f', 2));
	}

	QString log;

	if ((amountInCash < 0) && (aAmount == DBL_MAX))
	{
		log = " for unknown amount";
	}
	else if ((amountInCash > 0) && (aAmount != DBL_MAX) && (aAmount > amountInCash))
	{
		log = " for amount > amount in cash";
	}
	else if (aAmount > 0)
	{
		if (aAmount == DBL_MAX)
		{
			aAmount = amountInCash;
		}

		mNextDocument = true;

		return processNonReentrant(std::bind(&FRBase::performEncashment, this, std::ref(aReceipt), aAmount));
	}

	toLog(LogLevel::Error, mDeviceName + ": Failed to process payout" + log);

	if (mOperatorPresence)
	{
		return false;
	}

	if (!isPrintingNeed(aReceipt))
	{
		return true;
	}

	auto processingReceipt = [&] () -> bool { return processReceipt(aReceipt, true); };

	return processNonReentrant(processingReceipt);
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::printEncashment(const QStringList & aReceipt, double aAmount)
{
	return processEncashment(aReceipt, aAmount);
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::printEncashment(const QStringList & aReceipt)
{
	return processEncashment(aReceipt);
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::complexFiscalDocument(TBoolMethod aMethod, const QString & aLog)
{
	bool result = aMethod();

	if (!result)
	{
		if (!mNextReceiptProcessing)
		{
			onPoll();

			TStatusCodes nonFiscalErrors = mStatusCollection.value(EWarningLevel::Error) - getFiscalStatusCodes(EWarningLevel::Error);

			if (nonFiscalErrors.isEmpty())
			{
				receiptProcessing();
			}
		}

		toLog(LogLevel::Error, mDeviceName + ": Failed to process " + aLog);
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::performXReport(const QStringList & aReceipt)
{
	bool result = processReceipt(aReceipt, mNextReceiptProcessing);

	if (result && checkNotPrinting())
	{
		complexFiscalDocument(std::bind(&FRBase::processXReport, this), "X-report");
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::performEncashment(const QStringList & aReceipt, double aAmount)
{
	bool result = processReceipt(aReceipt, mNextReceiptProcessing);

	if (result && checkNotPrinting())
	{
		complexFiscalDocument(std::bind(&FRBase::processPayout, this, aAmount), "payout");
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
ESessionState::Enum FRBase<T>::getSessionState()
{
	return ESessionState::Error;
}

//--------------------------------------------------------------------------------
template <class T>
ESessionState::Enum FRBase<T>::checkSessionState()
{
	if (!mFiscalized || !mConnected || (mInitialized == ERequestStatus::InProcess))
	{
		return ESessionState::Error;
	}

	ESessionState::Enum sessionState;

	if (mNeedCloseSession)
	{
		sessionState = ESessionState::Expired;
	}
	else
	{
		processNonReentrant([&] () -> bool { sessionState = getSessionState(); return true; });
	}

	if (sessionState == ESessionState::Error)   toLog(LogLevel::Error,  mDeviceName + ": Failed to get session state");
	if (sessionState == ESessionState::Expired) toLog(LogLevel::Normal, mDeviceName + ": Session is expired");
	if (sessionState == ESessionState::Closed)  toLog(LogLevel::Normal, mDeviceName + ": Session is closed");
	if (sessionState == ESessionState::Opened)  toLog(LogLevel::Normal, mDeviceName + ": Session is opened");

	return sessionState;
}

//--------------------------------------------------------------------------------
template <class T>
EDocumentState::Enum FRBase<T>::getDocumentState()
{
	return EDocumentState::Error;
}

//--------------------------------------------------------------------------------
template <class T>
EDocumentState::Enum FRBase<T>::checkDocumentState()
{
	if (!mFiscalized || !mConnected || (mInitialized == ERequestStatus::InProcess))
	{
		return EDocumentState::Error;
	}

	EDocumentState::Enum documentState;
	processNonReentrant([&] () -> bool { documentState = getDocumentState(); return true; });

	if (documentState == EDocumentState::Error)  toLog(LogLevel::Error,  mDeviceName + ": Failed to get document state");
	if (documentState == EDocumentState::Closed) toLog(LogLevel::Normal, mDeviceName + ": Document is closed");
	if (documentState == EDocumentState::Opened) toLog(LogLevel::Normal, mDeviceName + ": Document is opened");

	return documentState;
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::openFRSession()
{
	if (!mIsOnline)
	{
		return true;
	}

	ESessionState::Enum sessionState = getSessionState();

	if (sessionState == ESessionState::Opened)
	{
		return true;
	}

	QTime currentTime = QTime::currentTime();

	if (!openSession())
	{
		return false;
	}

	if (sessionState == ESessionState::Closed)
	{
		setConfigParameter(CHardware::FR::SessionOpeningTime, currentTime.toString(CFR::TimeLogFormat));

		emit configurationChanged();
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::processStatus(TStatusCodes & aStatusCodes)
{
	if (!T::processStatus(aStatusCodes))
	{
		return false;
	}

	if (mZBufferFull)       aStatusCodes.insert(FRStatusCode::Warning::ZBufferFull);
	if (mZBufferOverflow)   aStatusCodes.insert(FRStatusCode::Error::ZBufferOverflow);
	if (mPrinterCollapse)   aStatusCodes.insert(PrinterStatusCode::Error::PrinterFRCollapse);
	if (!mFiscalized)       aStatusCodes.insert(FRStatusCode::Warning::NotFiscalized);
	if (mZBufferError)      aStatusCodes.insert(FRStatusCode::Error::ZBuffer);
	if (mFiscalCollapse)    aStatusCodes.insert(FRStatusCode::Error::FiscalCollapse);
	if (mNeedCloseSession)  aStatusCodes.insert(FRStatusCode::Error::NeedCloseSession);
	if (mNotPrintingError)  aStatusCodes.insert(DeviceStatusCode::Error::Initialization);
	if (mCashierINNError)   aStatusCodes.insert(FRStatusCode::Error::CashierINN);
	if (mFSOfflineEnd)      aStatusCodes.insert(FRStatusCode::Error::NeedOFDConnection);
	if (mTaxError)          aStatusCodes.insert(FRStatusCode::Error::Taxes);
	if (mWrongTaxOnPayment) aStatusCodes.insert(FRStatusCode::Warning::WrongTaxOnPayment);
	if (mNeedTimeSynchronization) aStatusCodes.insert(FRStatusCode::Warning::NeedTimeSynchronization);

	if (mIsOnline)
	{
		bool compliance = (mFFDFR == mFFDFS) || ((mFFDFS == EFFD::F10) && ((mFFDFR == EFFD::F105) || (mFFDFR == EFFD::F10Beta)));

		if ((mFFDFR != EFFD::Unknown) && (mFFDFS != EFFD::Unknown) && !compliance)
		{
			aStatusCodes.insert(FRStatusCode::Warning::FFDMismatch);
		}

		if ((mFFDFR != EFFD::Unknown) && (mFFDFR < CFR::ActualFFD))
		{
			aStatusCodes.insert(FRStatusCode::Warning::FFDFR);
		}

		if ((mFFDFS != EFFD::Unknown) && (mFFDFS < CFR::ActualFFD))
		{
			aStatusCodes.insert(FRStatusCode::Warning::FFDFS);
		}

		if (mFSError)
		{
			aStatusCodes.insert(FRStatusCode::Error::FS);
		}

		if (mOFDDataError && !mOperationModes.contains(EOperationModes::Autonomous))
		{
			aStatusCodes.insert(FRStatusCode::Warning::OFDData);
		}

		if (!mTaxSystems.isEmpty() && !mFFEngine.checkDealerTaxSystem(mInitialized, false))
		{
			if (mTaxSystems.size() > 1)
			{
				aStatusCodes.insert(FRStatusCode::Error::WrongDealerTaxSystem);
			}
			else if (containsConfigParameter(CHardwareSDK::FR::DealerTaxSystem))
			{
				aStatusCodes.insert(FRStatusCode::Warning::WrongDealerTaxSystem);
			}
		}

		if (!mAgentFlags.isEmpty() && !mFFEngine.checkDealerAgentFlag(mInitialized, false))
		{
			if (mAgentFlags.size() > 1)
			{
				aStatusCodes.insert(FRStatusCode::Error::WrongDealerAgentFlag);
			}
			else if (containsConfigParameter(CHardwareSDK::FR::DealerAgentFlag))
			{
				aStatusCodes.insert(FRStatusCode::Warning::WrongDealerAgentFlag);
			}
		}

		QString automaticNumber = getDeviceParameter(CDeviceData::FR::AutomaticNumber).toString();

		if (!mFiscalServerPresence && (mWrongFiscalizationSettings || (mOperatorPresence && !automaticNumber.isEmpty())))
		{
			aStatusCodes.insert(FRStatusCode::Warning::WrongFiscalizationSettings);
		}

		QString validityFSData = getDeviceParameter(CDeviceData::FS::ValidityData).toString();
		QDate validityFSDate = QDate::fromString(validityFSData, CFR::DateLogFormat);
		QDate currentDate = QDate::currentDate();

		if (validityFSDate.isValid() && (currentDate >= validityFSDate))
		{
			aStatusCodes.insert(FRStatusCode::Warning::FSVirtualEnd);
		}
	}
	else
	{
		if (!mEKLZ)     aStatusCodes.insert(FRStatusCode::Error::EKLZ);
		if (mEKLZError) aStatusCodes.insert(FRStatusCode::Error::EKLZ);
		if (mFMError)   aStatusCodes.insert(FRStatusCode::Error::FM);
	}

	//TODO: сделать обобщенную логику для mWhiteSpaceZBuffer и mLastOpenSession;

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
void FRBase<T>::postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection)
{
	if (mConnected && (mRegion == ERegion::RF))
	{
		checkTaxes2019();
	}

	T::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

//--------------------------------------------------------------------------------
template <class T>
void FRBase<T>::checkTaxes2019()
{
	QSettings settings;
	int year = QDateTime::currentDateTime().date().year();
	bool taxes2019Applied = false;

	auto check = [&] (const QString & aChildKey, const QString & aKey) -> bool { QString key = aChildKey + "/" + aKey;
		return settings.value(key, "").toString() == getDeviceParameter(aKey).toString().simplified(); };
	auto set = [&] (const QString & aChildKey, const QString & aKey) { QString key = aChildKey + "/" + aKey;
		settings.setValue(key, getDeviceParameter(aKey).toString().simplified()); };

	foreach (auto childKey, settings.childGroups())
	{
		if ((childKey == mDeviceName) && check(childKey, CDeviceData::SerialNumber))
		{
			QString key = childKey + "/" + CDeviceData::FR::Taxes2019Applied;
			taxes2019Applied = settings.value(key).toString() == CDeviceData::Values::Yes;
		}
	}

	setDeviceParameter(CDeviceData::FR::Taxes2019Applied, taxes2019Applied);

	bool canZReportWithoutPrinting = getConfigParameter(CHardwareSDK::FR::CanWithoutPrinting, false).toBool() || getConfigParameter(CHardware::FR::CanZReportWithoutPrinting, false).toBool();
	bool cannotReopenSession = !mOperatorPresence && !mCanProcessZBuffer && !canZReportWithoutPrinting;
	/*
	не переоткрываем смену, если:
	1. ТК.
	2. Нет буфера Z-отчетов.
	3. ФР не умеет формировать ФД без ПФ вообще и Z-отчет в частности.
	4. Смена незакрыта или ошибка получения статуса смены.
	*/

	if (!taxes2019Applied && (year == 2019) && !(cannotReopenSession && (checkSessionState() != ESessionState::Closed)))
	{
		execZReport(!mOperatorPresence);

		if (openFRSession())
		{
			setDeviceParameter(CDeviceData::FR::Taxes2019Applied, CDeviceData::Values::Yes);

			set(mDeviceName, CDeviceData::SerialNumber);
			set(mDeviceName, CDeviceData::FR::Taxes2019Applied);

			if (check(mDeviceName, CDeviceData::SerialNumber))
			{
				toLog(LogLevel::Normal, mDeviceName + ": Closing session with VAT 18%, trying to re-initialize");

				reInitialize();
			}
		}
	}
}

//--------------------------------------------------------------------------------
template <class T>
TSum FRBase<T>::getTotalAmount(const SPaymentData & aPaymentData) const
{
	return std::accumulate(aPaymentData.unitDataList.begin(), aPaymentData.unitDataList.end(), TSum(0), [] (TSum aSum, const SUnitData & aData) -> TSum { return aSum + aData.sum; });
}

//--------------------------------------------------------------------------------
template <class T>
TVATs FRBase<T>::getVATs(const SPaymentData & aPaymentData) const
{
	TVATs result;

	foreach(auto unitData, aPaymentData.unitDataList)
	{
		result << unitData.VAT;
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
TVATs FRBase<T>::getActualVATs() const
{
	if (mRegion == ERegion::RF)
	{
		return CFR::isRFVAT20() ? CFR::RFActualVATs20 : CFR::RFActualVATs;
	}

	return CFR::KZActualVATs;
}

//--------------------------------------------------------------------------------
template <class T>
TVATs FRBase<T>::getDeviceVATs()
{
	TVATs result;

	foreach (auto taxData, mTaxData.data())
	{
		result << taxData.deviceVAT;
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::checkAmountsOnPayment(const SPaymentData & aPaymentData)
{
	if (aPaymentData.unitDataList.isEmpty())
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to process command due to no amount data");
		return false;
	}

	QString payOffType = CFR::PayOffTypes[char(aPaymentData.payOffType)];

	foreach(const SUnitData & unitData, aPaymentData.unitDataList)
	{
		if (unitData.sum <= 0)
		{
			toLog(LogLevel::Error, mDeviceName + QString(": Failed to process %1 for sum = %2").arg(payOffType).arg(unitData.sum, 0, 'f', 2));
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::checkSumInCash(const SPaymentData & aPaymentData)
{
	if (!aPaymentData.back())
	{
		return true;
	}

	double amountInCash = WorkingThreadProxy(&mThread).invokeMethod<double>(std::bind(&FRBase<T>::getAmountInCash, this));
	TSum totalAmount = getTotalAmount(aPaymentData);
	QString payOffType = CFR::PayOffTypes[char(aPaymentData.payOffType)];

	if (amountInCash < 0)
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to get valid amount in cash for process " + payOffType);
	}
	else if (totalAmount > amountInCash)
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to process %1 for %2 > %3 in cash").arg(payOffType).arg(totalAmount, 0, 'f', 2).arg(amountInCash, 0, 'f', 2));

		emitStatusCode(FRStatusCode::Error::NoMoney, EFRStatus::NoMoneyForSellingBack);

		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::checkTaxesOnPayment(const SPaymentData & aPaymentData)
{
	TVATs errorVATs = getVATs(aPaymentData) - getDeviceVATs();

	if (!errorVATs.isEmpty())
	{
		toLog(LogLevel::Error, mDeviceName + ": The device taxes does not contain VAT(s): " + getVATLog(errorVATs));
		return false;
	}

	foreach(auto unitData, aPaymentData.unitDataList)
	{
		if (!mTaxData.data().contains(unitData.VAT))
		{
			toLog(LogLevel::Error, mDeviceName + QString(": The taxes specification does not contain VAT = %1").arg(unitData.VAT));
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
void FRBase<T>::addFiscalFieldsOnPayment(const SPaymentData & aPaymentData)
{
	auto addPhone = [&] (const QString & aField, const QVariant & aData) { QString phone = mFFEngine.filterPhone(aData.toString());
		if (!phone.isEmpty()) mFFEngine.setConfigParameter(aField, phone); };

	auto addMail = [&] (const QString & aField, const QVariant & aData) { QString mail = aData.toString();
		if (mail.contains(QRegExp("^[^@]+@[^@]+$"))) mFFEngine.setConfigParameter(aField, mail); };

	mFFEngine.setConfigParameter(CFiscalSDK::PayOffType, aPaymentData.payOffType);

	QVariantMap receiptParameters = getConfigParameter(CHardwareSDK::Printer::ReceiptParameters).toMap();

	addPhone(CFiscalSDK::ProviderPhone, getConfigParameter(CHardwareSDK::Printer::TemplateParameters).toMap()[CPrintConstants::OpPhone]);
	addPhone(CFiscalSDK::AgentPhone,            receiptParameters[CPrintConstants::DealerPhone]);
	addPhone(CFiscalSDK::TransferOperatorPhone, receiptParameters[CPrintConstants::BankPhone]);

	mFFEngine.setConfigParameter(CFiscalSDK::TransferOperatorAddress, receiptParameters[CPrintConstants::BankAddress].toString().simplified());
	mFFEngine.setConfigParameter(CFiscalSDK::TransferOperatorINN,     receiptParameters[CPrintConstants::BankInn].toString().simplified());
	mFFEngine.setConfigParameter(CFiscalSDK::TransferOperatorName,    receiptParameters[CPrintConstants::BankName].toString().simplified());

	addPhone(CFiscalSDK::UserContact, aPaymentData.fiscalParameters.value(CHardwareSDK::FR::UserPhone));
	 addMail(CFiscalSDK::UserContact, aPaymentData.fiscalParameters.value(CHardwareSDK::FR::UserMail));

	QList<int> fiscalFields = mFFData.data().keys();
	QVariantMap FFConfig;

	for (auto it = mFFData.data().begin(); it != mFFData.data().end(); ++it)
	{
		if (containsDeviceParameter(it->textKey)) FFConfig.insert(it->textKey, getDeviceParameter(it->textKey));
		if (containsConfigParameter(it->textKey)) FFConfig.insert(it->textKey, getConfigParameter(it->textKey));
	}

	mFFEngine.setConfiguration(FFConfig);
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::checkPayTypeOnPayment(const SPaymentData & aPaymentData)
{
	if (aPaymentData.payType == EPayTypes::None)
	{
		toLog(LogLevel::Error, mDeviceName + ": pay type is not defined");
		return false;
	}
	else if (!mPayTypeData.data().contains(aPaymentData.payType))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": The pay types specification does not contain type = %1 (%2)").arg(aPaymentData.payType).arg(CFR::PayTypeDescription[aPaymentData.payType]));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
int FRBase<T>::getErrorStatusCode(FRError::EType::Enum aErrorType)
{
	switch (aErrorType)
	{
		case FRError::EType::FR:      return      FRStatusCode::Error::FR;
		case FRError::EType::Printer: return PrinterStatusCode::Error::PrinterFR;
		case FRError::EType::FM:      return      FRStatusCode::Error::FM;
		case FRError::EType::EKLZ:    return      FRStatusCode::Error::EKLZ;
		case FRError::EType::FS:      return      FRStatusCode::Error::FS;
		case FRError::EType::SD:      return  DeviceStatusCode::Error::MemoryStorage;
		case FRError::EType::Retry:   return  DeviceStatusCode::OK::OK;
	}

	return DeviceStatusCode::Error::Unknown;
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::isFS36() const
{
	if (FS::Data.contains(mFSSerialNumber))
	{
		return FS::Data[mFSSerialNumber].expiration == 36;
	}

	QString FSValidityDateText = getDeviceParameter(CDeviceData::FS::ValidityData).toString();
	QDate FSValidityDate = QDate::fromString(FSValidityDateText, CFR::DateLogFormat);

	if (!FSValidityDate.isValid() || FSValidityDate.isNull())
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to check FS validity date");
		return false;
	}

	int days = QDate::currentDate().daysTo(FSValidityDate) + 3;
	toLog(LogLevel::Normal, mDeviceName + QString(": %1 days to validity date").arg(days));

	return days > CFR::SimpleFSValidityDays;
}

//--------------------------------------------------------------------------------
template <class T>
void FRBase<T>::setLog(ILog * aLog)
{
	T::setLog(aLog);

	mFFEngine.setLog(aLog);
}

//--------------------------------------------------------------------------------
