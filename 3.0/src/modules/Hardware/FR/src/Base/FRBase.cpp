/* @file Базовый фискальный регистратор. */

// STL
#include <numeric>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// OPOS
#include <Common/QtHeadersBegin.h>
#pragma warning(disable: 4100) // warning C4100: 'identifier' : unreferenced formal parameter
#include <OPOS/QtWrappers/FiscalPrinter.h>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/OPOSPollingDeviceBase.h"
#include "Hardware/Printers/SerialPrinterBase.h"
#include "Hardware/FR/ProtoFR.h"

// Project
#include "FRBase.h"
#include "Hardware/FR/OFDServerData.h"

using namespace SDK::Driver;
using namespace FRStatusCode;
using namespace ProtocolUtils;

//---------------------------------------------------------------------------
template class FRBase<PrinterBase<OPOSPollingDeviceBase<ProtoFR, OPOS::OPOSFiscalPrinter>>>;
template class FRBase<SerialPrinterBase<PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoFR>>>>>;
template class FRBase<PortPrinterBase<PrinterBase<TCPDeviceBase<PortPollingDeviceBase<ProtoFR>>>>>;

//--------------------------------------------------------------------------------
template <class T>
FRBase<T>::FRBase()
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
	mNextReceiptProcessing = true;
	mRegion                = ERegion::RF;
	mIsOnline              = false;

	mOFDFiscalParameters
		<< FiscalFields::Cashier
		<< FiscalFields::CashierINN
		<< FiscalFields::UserContact;

	mPayTypeData.add(EPayTypes::Cash,   1);
	mPayTypeData.add(EPayTypes::EMoney, 2);
}

//--------------------------------------------------------------------------------
template <class T>
void FRBase<T>::setInitialData()
{
	mEKLZ                  = true;
	mEKLZError             = false;
	mFMError               = false;
	mFSError               = false;
	mZBufferFull           = false;
	mZBufferOverflow       = false;
	mPrinterCollapse       = false;
	mFiscalized            = true;
	mZBufferError          = false;
	mFiscalCollapse        = false;
	mWhiteSpaceZBuffer     = -1;
	mNeedCloseSession      = false;
	mLastOpenSession       = QDateTime::currentDateTime();
	mOFDDataError          = false;
	mOFDNotSentCount       = 0;
	mOFDDTMark             = QDateTime::currentDateTime();
	mFFDFR                 = EFFD::F10Beta;
	mFFDFS                 = EFFD::F10Beta;
	mCanProcessZBuffer     = false;

	mTaxations.clear();
	mAgentFlags.clear();
	mOperationModes.clear();
	mINN.clear();
	mRNM.clear();
	mSerial.clear();
	mFSSerialNumber.clear();
}

//--------------------------------------------------------------------------------
template <class T>
void FRBase<T>::initialize()
{
	setInitialData();

	T::initialize();
}

//--------------------------------------------------------------------------------
template <class T>
void FRBase<T>::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	T::setDeviceConfiguration(aConfiguration);

	if (aConfiguration.contains(CHardware::FR::FiscalMode))
	{
		bool isFiscal = aConfiguration[CHardware::FR::FiscalMode].toBool();
		mStatusCodesSpecification.dynamicCast<FRStatusCode::CSpecifications>()->setFiscal(isFiscal);
	}
}

//--------------------------------------------------------------------------------
template <class T>
void FRBase<T>::finaliseInitialization()
{
	setDeviceParameter(CDeviceData::FR::OnlineMode, mIsOnline);

	if (mConnected)
	{
		setDeviceParameter(CDeviceData::SerialNumber, mSerial);
		setDeviceParameter(CDeviceData::FR::INN, mINN);
		setDeviceParameter(CDeviceData::FR::RNM, mRNM);

		if (mIsOnline)
		{
			TTaxationData taxationData;

			foreach(char taxation, mTaxations)
			{
				taxationData.insert(ETaxations::Enum(taxation), CFR::Taxations[taxation]);
			}

			TAgentFlagsData agentFlagsData;

			foreach(char agentFlag, mAgentFlags)
			{
				agentFlagsData.insert(EAgentFlags::Enum(agentFlag), CFR::AgentFlags[agentFlag]);
			}

			TOperationModeData operationModedata;

			foreach(char operationMode, mOperationModes)
			{
				operationModedata.insert(EOperationModes::Enum(operationMode), CFR::OperationModes::Data[operationMode].description);
			}

			setDeviceParameter(CDeviceData::FS::SerialNumber, mFSSerialNumber);
			setDeviceParameter(CDeviceData::FR::Taxations, QStringList(taxationData.values()).join(", "));
			setDeviceParameter(CDeviceData::FR::AgentFlags, QStringList(agentFlagsData.values()).join(", "));
			setDeviceParameter(CDeviceData::FR::OperationModes, QStringList(operationModedata.values()).join(", "));
			setDeviceParameter(CDeviceData::FR::FFDFR, CFR::FFD[mFFDFR].description);
			setDeviceParameter(CDeviceData::FR::FFDFS, CFR::FFD[mFFDFS].description);

			setConfigParameter(CHardwareSDK::FR::FSSerialNumber, mFSSerialNumber);
			setConfigParameter(CHardwareSDK::FR::Taxations, QVariant().fromValue(taxationData));
			setConfigParameter(CHardwareSDK::FR::AgentFlags, QVariant().fromValue(agentFlagsData));
		}

		setConfigParameter(CHardwareSDK::SerialNumber, mSerial);
	}

	T::finaliseInitialization();

	checkZReportOnTimer();
}

//---------------------------------------------------------------------------
template <class T>
void FRBase<T>::onExecZReport()
{
	toLog(LogLevel::Normal, mDeviceName + ": Z-report timer has fired");

	execZReport(true);

	checkZReportOnTimer();
}

//---------------------------------------------------------------------------
template <class T>
void FRBase<T>::checkZReportOnTimer()
{
	QTime ZReportTime = getConfigParameter(CHardwareSDK::FR::ZReportTime).toTime();

	if (ZReportTime.isValid())
	{
		QTime current = QTime::currentTime();
		int delta = current.msecsTo(ZReportTime);

		if (delta < 0)
		{
			delta += CFR::MSecsInDay;
		}

		toLog(LogLevel::Normal, mDeviceName + ": Starting Z-report timer to " + ZReportTime.toString(CFR::TimeLogFormat));

		QTimer::singleShot(delta, this, SLOT(onExecZReport()));
	}
}

//---------------------------------------------------------------------------
template <class T>
bool FRBase<T>::checkTaxationData(char aData)
{
	QStringList errorLog;

	for (int i = 0; i < (sizeof(aData) * 8); ++i)
	{
		char taxation = 1 << i;

		if (aData & taxation)
		{
			if (!CFR::Taxations.data().contains(taxation))
			{
				errorLog << toHexLog(taxation);
			}

			mTaxations << taxation;
		}
	}

	if (!errorLog.isEmpty())
	{
		toLog(LogLevel::Error, mDeviceName + ": Unknown taxation system(s): " + errorLog.join(", "));
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
template <class T>
bool FRBase<T>::checkAgentFlags(char aData)
{
	QStringList errorLog;

	for (int i = 0; i < (sizeof(aData) * 8); ++i)
	{
		char agentFlag = 1 << i;

		if (aData & agentFlag)
		{
			if (!CFR::AgentFlags.data().contains(agentFlag))
			{
				errorLog << toHexLog(agentFlag);
			}

			mAgentFlags << agentFlag;
		}
	}

	if (!errorLog.isEmpty())
	{
		toLog(LogLevel::Error, mDeviceName + ": Unknown agent flag(s): " + errorLog.join(", "));
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
template <class T>
bool FRBase<T>::checkOperationModes(char aData)
{
	QStringList errorLog;

	for (int i = 0; i < (sizeof(aData) * 8); ++i)
	{
		char operationMode = 1 << i;

		if (aData & operationMode)
		{
			if (!CFR::OperationModes::Data.data().contains(operationMode))
			{
				errorLog << toHexLog(operationMode);
			}

			mOperationModes << operationMode;
		}
	}

	if (!errorLog.isEmpty())
	{
		toLog(LogLevel::Error, mDeviceName + ": Unknown operation mode(s): " + errorLog.join(", "));
		return false;
	}

	return true;
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
		FSDifferenceDT += (secs < 0) ? "- " : "+ ";
		secs = std::abs(secs);
		int days = secs/CFR::SecsInDay;

		if (days)
		{
			FSDifferenceDT += QString("%1 day(s), ").arg(days);
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
	for (auto it = mTaxData.data().begin(); it != mTaxData.data().end(); ++it)
	{
		if (!checkTax(it.key(), it.value()))
		{
			return false;
		}
	}

	return true;
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
		bool testURL = URL.valid && (URL.value == aOFDData.testAddress.URL.value);
		bool testIP  =  IP.valid && ( IP.value == aOFDData.testAddress.IP.value);

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
		bool testURL = URL.valid && (URL.value == aOFDData.address.URL.value);
		bool testIP  =  IP.valid && ( IP.value == aOFDData.address.IP.value);

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
bool FRBase<T>::checkFiscalField(int aField, bool & aResult)
{
	if (!mFiscalFieldData.data().contains(aField))
	{
		aResult = false;
		toLog(LogLevel::Error, QString("%1: Failed to set %2 fiscal field due to it is unknown").arg(mDeviceName).arg(aField));

		return false;
	}

	using namespace CFR::FiscalFields;

	SData data = mFiscalFieldData[aField];

	if (containsConfigParameter(data.description))
	{
		return true;
	}

	if (data.required == ERequired::No)
	{
		aResult = true;
		toLog(LogLevel::Debug, QString("%1: don`t set field %2 (%3) due to the field is not required").arg(mDeviceName).arg(aField).arg(data.description));

		return false;
	}
	else if ((data.required == CFR::FiscalFields::ERequired::PM) && !mOperatorPresence)
	{
		aResult = true;
		toLog(LogLevel::Debug, QString("%1: don`t set field %2 (%3) due to the operator is not present").arg(mDeviceName).arg(aField).arg(data.description));

		return false;
	}

	aResult = false;
	toLog(LogLevel::Error, QString("%1: Failed to set field %2 (%3) due due to it is absent").arg(mDeviceName).arg(aField).arg(data.description));

	return false;
}

//---------------------------------------------------------------------------
template <class T>
void FRBase<T>::cleanStatusCodes(TStatusCodes & aStatusCodes)
{
	T::cleanStatusCodes(aStatusCodes);

	// фильтр ошибки ЭКЛЗ скоро кончится, если есть ошибка ЭКЛЗ
	if (aStatusCodes.contains(Error::EKLZ))
	{
		aStatusCodes.remove(Warning::EKLZNearEnd);
	}

	// фильтр ошибки Фискальная память скоро кончится, если есть ошибка фискальной памяти
	if (aStatusCodes.contains(Error::FiscalMemory))
	{
		aStatusCodes.remove(Warning::FiscalMemoryNearEnd);
	}

	// фильтр ошибок Буфер Z-отчетов заполнен и необходимости Z-отчета, если он уже переполнен
	if (aStatusCodes.contains(Error::ZBufferOverflow))
	{
		aStatusCodes.remove(Warning::ZBufferFull);
		aStatusCodes.remove(Error::NeedCloseSession);
	}

	// фильтр статусов буфера Z-отчетов, если есть сбой последнего
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
bool FRBase<T>::printFiscal(const QStringList & aReceipt, const SPaymentData & aPaymentData, TFiscalPaymentData & aFPData, TComplexFiscalPaymentData & aPSData)
{
	using namespace CFR::FiscalFields;

	QScopedPointer<ConfigCleaner> configCleaner(new ConfigCleaner(&mConfiguration));

	if (!isFiscal())
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to process command due to nonfiscal mode");
		return false;
	}

	if (aPaymentData.amountDataList.isEmpty())
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to process command due to no amount data");
		return false;
	}

	foreach (const SAmountData & amountData, aPaymentData.amountDataList)
	{
		if (amountData.sum <= 0)
		{
			toLog(LogLevel::Error, QString("%1: Failed to sale%2 for sum = %3").arg(mDeviceName).arg(aPaymentData.back ? " back" : "").arg(amountData.sum, 0, 'f', 2));
			return false;
		}
	}

	if (aPaymentData.back)
	{
		double amountInCash = WorkingThreadProxy().invokeMethod<double>(std::bind(&FRBase<T>::getAmountInCash, this));
		TSum totalAmount = getTotalAmount(aPaymentData);

		if (amountInCash < 0)
		{
			toLog(LogLevel::Error, mDeviceName + ": Failed to get valid amount in cash for selling back");
		}
		else if (totalAmount > amountInCash)
		{
			toLog(LogLevel::Error, QString("%1: Failed to sale back for %2 > %3 in cash").arg(mDeviceName).arg(totalAmount, 0, 'f', 2).arg(amountInCash, 0, 'f', 2));

			emitStatusCode(FRStatusCode::Error::NoMoney, EFRStatus::NoMoneyForSellingBack);

			return false;
		}
	}

	TVATs errorVATs = getVATs(aPaymentData) - getActualVATs();

	if (!errorVATs.isEmpty())
	{
		toLog(LogLevel::Error, mDeviceName + ": The actual taxes does not contain VAT(s): " + getVATLog(errorVATs));
		return false;
	}

	foreach (auto amountData, aPaymentData.amountDataList)
	{
		if (!mTaxData.data().contains(amountData.VAT))
		{
			toLog(LogLevel::Error, mDeviceName + QString(": The taxes specification does not contain VAT = %1").arg(amountData.VAT));
			return false;
		}
	}

	SPaymentData paymentData(aPaymentData);

	if (paymentData.payType == EPayTypes::None)
	{
		toLog(LogLevel::Error, mDeviceName + ": pay type is not defined");
		return false;
	}
	else if (!mPayTypeData.data().contains(paymentData.payType))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": The pay types specification does not contain type = %1 (%2)").arg(paymentData.payType).arg(CFR::PayTypeDescription[paymentData.payType]));
		return false;
	}

	auto joinData = [](const QList<char> & aData) -> char { return std::accumulate(aData.begin(), aData.end(), ASCII::NUL,
		[](char aResult, char aTaxation) -> char { return aResult | aTaxation; }); };

	char taxation = char(paymentData.taxation);

	char taxationData = char(getConfigParameter(CHardwareSDK::FR::DealerTaxation).toInt());
	ETaxations::Enum dealerTaxation = ETaxations::Enum(taxationData);

	if (dealerTaxation != ETaxations::None)
	{
		if (mTaxations.contains(taxationData))
		{
			paymentData.taxation = dealerTaxation;
			setConfigParameter(CHardware::FiscalFields::TaxSystem, taxationData);
		}
		else if (mTaxations.size() == 1)
		{
			paymentData.taxation = ETaxations::Enum(mTaxations[0]);
		}
	}
	else if (!mTaxations.isEmpty())
	{
		char joinedTaxations = joinData(mTaxations);

		if (paymentData.taxation == ETaxations::None)
		{
			if (mTaxations.size() == 1)
			{
				paymentData.taxation = ETaxations::Enum(mTaxations[0]);
			}
			else
			{
				toLog(LogLevel::Error, QString("%1: Failed to determine the required taxation system from the several ones (%2)")
					.arg(mDeviceName).arg(toHexLog(joinedTaxations)));
				return false;
			}
		}
		else if (!mTaxations.contains(taxation))
		{
			toLog(LogLevel::Error, QString("%1: The actual taxation system(s) %2 don`t contain system %3 (%4)")
				.arg(mDeviceName).arg(toHexLog(joinedTaxations)).arg(toHexLog(taxation)).arg(CFR::Taxations[taxation]));
			return false;
		}
	}

	char agentFlags = char(paymentData.agentFlag);

	char agentFlagData = char(getConfigParameter(CHardwareSDK::FR::DealerAgentFlag).toInt());
	EAgentFlags::Enum dealerAgentFlag = EAgentFlags::Enum(agentFlagData);

	if (dealerAgentFlag != EAgentFlags::None)
	{
		if (mAgentFlags.contains(agentFlagData))
		{
			paymentData.agentFlag = dealerAgentFlag;
			setConfigParameter(CHardware::FiscalFields::AgentFlag, agentFlagData);
		}
		else if (mAgentFlags.size() == 1)
		{
			paymentData.agentFlag = EAgentFlags::Enum(mAgentFlags[0]);
		}
	}
	else if (!mAgentFlags.isEmpty())
	{
		char joinedAgentFlags = joinData(mAgentFlags);

		if (paymentData.agentFlag == EAgentFlags::None)
		{
			if (mAgentFlags.size() == 1)
			{
				paymentData.agentFlag = EAgentFlags::Enum(mAgentFlags[0]);
			}
			else
			{
				toLog(LogLevel::Error, QString("%1: Failed to determine the required agent flag from the several ones (%2)")
					.arg(mDeviceName).arg(toHexLog(joinedAgentFlags)));
				return false;
			}
		}
		else if (!mAgentFlags.contains(agentFlags))
		{
			toLog(LogLevel::Error, QString("%1: The actual agent flag(s) %2 don`t contain flag %3 (%4)")
				.arg(mDeviceName).arg(toHexLog(joinedAgentFlags)).arg(toHexLog(agentFlags)).arg(CFR::AgentFlags[agentFlags]));
			return false;
		}
	}

	QString userPhone = getConfigParameter(CHardwareSDK::FR::UserPhone).toString().remove(QRegExp("[^0-9]+"));
	QString userMail  = getConfigParameter(CHardwareSDK::FR::UserMail).toString();

	if (!userPhone.isEmpty())
	{
		setConfigParameter(CHardware::FiscalFields::UserContact, "+" + userPhone);
	}

	if (userMail.contains(QRegExp("^[^@]+@[^@]+$")))
	{
		setConfigParameter(CHardware::FiscalFields::UserContact, userMail);
	}

	QStringList receipt(aReceipt);
	cleanReceipt(receipt);

	for (int i = 0; i < paymentData.amountDataList.size(); ++i)
	{
		QString & name = paymentData.amountDataList[i].name;
		name = name.simplified();

		if (mIsOnline)
		{
			int maxUnitNameSize = CFR::FFD[mFFDFS].maxUnitNameSize;
			name = name.left(maxUnitNameSize);
		}
	}

	aFPData.clear();
	aPSData.clear();

	if (!processNonReentrant(std::bind(&FRBase::performFiscal, this, std::ref(receipt), std::ref(paymentData), std::ref(aFPData), std::ref(aPSData))))
	{
		return false;
	}

	foreach (int field, FiscalTotals)
	{
		if (!aFPData.value(field).toInt())
		{
			aFPData.remove(field);
		}
	}

	EPayOffTypes::Enum payOffType = aPaymentData.back ? EPayOffTypes::DebitBack : EPayOffTypes::Debit;

	aFPData.insert(FiscalFields::FDName, CFR::CashFDName);
	aFPData.insert(FiscalFields::PayOffType, CFR::PayOffTypes[payOffType]);
	aFPData.insert(FiscalFields::TaxSystem, CFR::Taxations[char(paymentData.taxation)]);
	aFPData.insert(FiscalFields::SerialFSNumber, mFSSerialNumber);
	aFPData.insert(FiscalFields::INN, mINN);
	aFPData.insert(FiscalFields::RNM, mRNM);
	aFPData.insert(FiscalFields::SerialFRNumber, mSerial);
	aFPData.insert(FiscalFields::FTSURL, getConfigParameter(CHardware::FiscalFields::FTSURL));
	aFPData.insert(FiscalFields::OFDURL, getConfigParameter(CHardware::FiscalFields::OFDURL));
	aFPData.insert(FiscalFields::OFDName, getConfigParameter(CHardware::FiscalFields::OFDName));
	aFPData.insert(FiscalFields::LegalOwner, getConfigParameter(CHardware::FiscalFields::LegalOwner));
	aFPData.insert(FiscalFields::PayOffAddress, getConfigParameter(CHardware::FiscalFields::PayOffAddress));
	aFPData.insert(FiscalFields::PayOffPlace, getConfigParameter(CHardware::FiscalFields::PayOffPlace));

	if (paymentData.agentFlag != EAgentFlags::None)
	{
		aFPData.insert(FiscalFields::AgentFlag, CFR::AgentFlags[char(paymentData.taxation)]);
	}

	if (containsDeviceParameter(CDeviceData::FR::AutomaticNumber))
	{
		aFPData.insert(FiscalFields::AutomaticNumber, getDeviceParameter(CDeviceData::FR::AutomaticNumber));
	}

	int sessionNumber = getSessionNumber();

	if (sessionNumber)
	{
		aFPData.insert(FiscalFields::SessionNumber, sessionNumber);
	}

	QByteArray FDSignData = aFPData.value(FiscalFields::FDSign).toByteArray();
	qulonglong FDSign = FDSignData.right(4).toHex().toULongLong(0, 16);
	QString FDSignTextData = QString("%1").arg(FDSign, CFR::FDSignSize, 10, QChar(ASCII::Zero));
	aFPData.insert(FiscalFields::FDSign, FDSignTextData);

	QDateTime dateTime = aFPData.value(FiscalFields::FDDateTime).toDateTime();

	if (!dateTime.isValid())
	{
		dateTime = getDateTime();
	}

	aFPData.insert(FiscalFields::FDDateTime, dateTime.toString(CFR::DateTimeShortLogFormat));

	for (int i = 0; i < mOperationModes.size(); ++i)
	{
		CFR::OperationModes::SData modeData = CFR::OperationModes::Data[mOperationModes[i]];

		if (modeData.field)
		{
			aFPData.insert(modeData.field, modeData.description);
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::setOFDParameters()
{
	using namespace CFR::FiscalFields;

	foreach (auto parameter, mOFDFiscalParameters)
	{
		ERequired::Enum required = mFiscalFieldData[parameter].required;
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
	double amountInCash = WorkingThreadProxy().invokeMethod<double>(std::bind(&FRBase<T>::getAmountInCash, this));

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

	if (result)
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

	if (result)
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

		if (mFSError)      aStatusCodes.insert(FRStatusCode::Error::FS);
		if (mOFDDataError) aStatusCodes.insert(FRStatusCode::Warning::OFDData);

		if (containsConfigParameter(CHardwareSDK::FR::DealerTaxation))
		{
			char dealerTaxation = char(getConfigParameter(CHardwareSDK::FR::DealerTaxation).toInt());

			if (!mTaxations.isEmpty() && !mTaxations.contains(dealerTaxation))
			{
				int taxationStatus = (mTaxations.size() == 1) ? FRStatusCode::Warning::WrongTaxation : FRStatusCode::Error::WrongTaxation;
				aStatusCodes.insert(taxationStatus);
			}
		}

		if (containsConfigParameter(CHardwareSDK::FR::DealerAgentFlag))
		{
			char dealerAgentFlag = char(getConfigParameter(CHardwareSDK::FR::DealerAgentFlag).toInt());

			if (!mAgentFlags.isEmpty() && !mAgentFlags.contains(dealerAgentFlag))
			{
				int agentFlagStatus = (mAgentFlags.size() == 1) ? FRStatusCode::Warning::WrongAgentFlag : FRStatusCode::Error::WrongAgentFlag;
				aStatusCodes.insert(agentFlagStatus);
			}
		}
	}
	else
	{
		if (!mEKLZ)        aStatusCodes.insert(FRStatusCode::Error::EKLZ);
		if (mEKLZError)    aStatusCodes.insert(FRStatusCode::Error::EKLZ);
		if (mFMError)      aStatusCodes.insert(FRStatusCode::Error::FiscalMemory);
	}

	//TODO: сделать обобщенную логику для mWhiteSpaceZBuffer и mLastOpenSession;

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
TSum FRBase<T>::getTotalAmount(const SPaymentData & aPaymentData) const
{
	return std::accumulate(aPaymentData.amountDataList.begin(), aPaymentData.amountDataList.end(), TSum(0), [] (TSum aSum, const SAmountData & aData) -> TSum { return aSum + aData.sum; });
}

//--------------------------------------------------------------------------------
template <class T>
TVATs FRBase<T>::getVATs(const SPaymentData & aPaymentData) const
{
	TVATs result;

	foreach(auto amountData, aPaymentData.amountDataList)
	{
		result << amountData.VAT;
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
TVATs FRBase<T>::getActualVATs() const
{
	if (mRegion == ERegion::RF)
	{
		return CFR::RFActualVATs;
	}

	return CFR::KZActualVATs;
}

//--------------------------------------------------------------------------------
template <class T>
bool FRBase<T>::parseTLV(const QByteArray & aData, CFR::STLV & aTLV)
{
	auto getInt = [&aData] (int aIndex, int aShift) -> int { int result = uchar(aData[aIndex]); return result << (8 * aShift); };
	int fullDataSize = aData.size();

	if (fullDataSize < CFR::MinTLVSize)
	{
		toLog(LogLevel::Error, QString("%1: Failed to parse TLV data due to data size = %2, need %3 min").arg(mDeviceName).arg(fullDataSize).arg(CFR::MinTLVSize));
		return false;
	}

	int size = getInt(2, 0) | getInt(3, 1);
	int dataSize = fullDataSize - 4;

	if (dataSize < size)
	{
		toLog(LogLevel::Error, QString("%1: Failed to parse TLV data due to data size = %2, need %3 min").arg(mDeviceName).arg(dataSize).arg(size));
		return false;
	}

	int field = getInt(0, 0) | getInt(1, 1);
	QByteArray data  = aData.mid(4, size);

	using namespace CFR::FiscalFields;

	SData FFData = mFiscalFieldData[field];
	Types::SData typeData = Types::Data[FFData.type];

	int FFMinSize = typeData.minSize;
	int FFDataSize = data.size();

	if (FFDataSize < FFMinSize)
	{
		toLog(LogLevel::Error, QString("%1: Failed to parse %2 field %3 (%4) due to size = %5, need %6 min")
			.arg(mDeviceName).arg(typeData.description).arg(field).arg(FFData.description).arg(FFDataSize).arg(FFMinSize));
		return false;
	}

	if (typeData.fixSize)
	{
		data = data.left(FFMinSize);
	}

	aTLV.field = field;
	aTLV.data  = data;

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
CFR::TTLVList FRBase<T>::parseSTLV(const QByteArray & aData)
{
	CFR::TTLVList result;
	CFR::STLV TLV;
	int index = 0;

	while ((index < aData.size()) && parseTLV(aData.mid(index), TLV))
	{
		result.insert(TLV.field, TLV.data);
		index += TLV.data.size() + 4;
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
QByteArray FRBase<T>::getTLVData(int aField, const QVariant & aValue, QString * aLog = nullptr)
{
	using namespace CFR::FiscalFields;

	SData FFData = mFiscalFieldData[aField];
	QString log = QString("field %1 (%2) = ").arg(aField).arg(FFData.description);

	QByteArray result = getHexReverted(aField, 2);

	switch (FFData.type)
	{
		case ETypes::VLN:
		{
			qulonglong data = aValue.toULongLong();
			result += getDigitTLVData(data);
			log += QString::number(data);

			break;
		}
		case ETypes::FVLN:
		{
			double data = aValue.toDouble();
			int dotNumber = QString::number(data).indexOf(ASCII::Dot) + 1;
			result.append(uchar(dotNumber));

			//TODO: проверить на Касби - qPow(data, data)

			result.append(data ? getDigitTLVData(qulonglong(qPow(data, data))) : ASCII::NUL);
			log += QString::number(data, 'f', 2);

			break;
		}
		case ETypes::String:
		{
			QString data = aValue.toString();
			result += mCodec->fromUnicode(data);
			log += data;

			break;
		}
		case ETypes::Byte:
		{
			char data = char(aValue.toUInt());
			result += data;
			log += toHexLog(data);

			break;
		}
		//TODO: доделать лог для STLV - для Касби
		case ETypes::STLV:
		case ETypes::ByteArray:
		{
			QByteArray data = aValue.toByteArray();
			result += data;
			log += "0x" + data.toHex().toUpper();

			break;
		}
		default:
		{
			result.clear();    // чтобы не прошел запрос установки TLB-параметра из-за неверных данных, с которыми неясно что делать
			log += "0x" + aValue.toByteArray();
			toLog(LogLevel::Error, QString("%1: Unknown data type to set = %2").arg(mDeviceName).arg(int(FFData.type)));

			break;
		}
	}

	result.insert(2, getHexReverted(result.size() - 2, 2));

	if (aLog)
	{
		*aLog = log;
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
QByteArray FRBase<T>::getDigitTLVData(qulonglong aValue)
{
	int size = sizeof(aValue);
	QByteArray result;

	for (int i = 0; i < size; ++i)
	{
		char data = char(aValue >> (8 * (size - i - 1)));

		if (!result.isEmpty() || data)
		{
			result.append(data);
		}
	}

	if (result.isEmpty())
	{
		result.append(ASCII::NUL);
	}

	return revert(result);
}

//--------------------------------------------------------------------------------
template <class T>
void FRBase<T>::parseTLVData(int aField, const QByteArray & aData, TFiscalPaymentData & aFPData)
{
	using namespace CFR::FiscalFields;

	SData FFData = mFiscalFieldData[aField];
	QVariant result;

	if (FFData.type == CFR::FiscalFields::ETypes::STLV)
	{
		return;
	}

	switch (FFData.type)
	{
		case ETypes::String:
		{
			if (mCodec)
			{
				result = mCodec->toUnicode(aData);
			}

			break;
		}
		case ETypes::FVLN:
		{
			qulonglong digitData = revert(aData.mid(1)).toHex().toULongLong(0, 16);
			QString textData = QString::number(digitData);
			textData = textData.insert(textData.size() - int(uchar(aData[0])), QChar(ASCII::Dot));
			result = textData.toDouble();

			break;
		}
		case ETypes::VLN:
		{
			result = revert(aData).toHex().toUInt(0, 16);

			break;
		}
		case ETypes::UnixTime:
		{
			QDateTime dateTime;
			dateTime.setTimeSpec(Qt::UTC);
			uint seconds = ProtocolUtils::revert(aData).toHex().toUInt(0, 16);
			dateTime.setTime_t(seconds);

			result = dateTime;

			break;
		}
		case ETypes::ByteArray:
		{
			result = aData;    // формат априори неизвестен

			break;
		}
		default:
		{
			toLog(LogLevel::Error, QString("%1: Unknown data type to set = %2").arg(mDeviceName).arg(int(FFData.type)));

			break;
		}
	}

	if (result.isValid())
	{
		aFPData.insert(aField, result);
	}
}

//--------------------------------------------------------------------------------
template <class T>
void FRBase<T>::parseSTLVData(const CFR::STLV & aTLV, TComplexFiscalPaymentData & aPSData)
{
	if (mFiscalFieldData[aTLV.field].type == CFR::FiscalFields::ETypes::STLV)
	{
		CFR::TTLVList complexFPData = parseSTLV(aTLV.data);
		TFiscalPaymentData FPData;

		for (auto it = complexFPData.begin(); it != complexFPData.end(); ++it)
		{
			parseTLVData(it.key(), it.value(), FPData);
		}

		if (!FPData.isEmpty())
		{
			aPSData << FPData;
		}
	}
}

//--------------------------------------------------------------------------------
