/* @file Прото-ФР семейства АТОЛ. */

#include "ProtoAtolFR.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
template class ProtoAtolFR<TSerialFRBase>;
template class ProtoAtolFR<TExternalFRBase>;

//--------------------------------------------------------------------------------
template <class T>
ProtoAtolFR<T>::ProtoAtolFR():
	mMode(CAtolFR::InnerModes::NoMode),
	mSubmode(CAtolFR::InnerSubmodes::NoSubmode),
	mLocked(false),
	mNonNullableAmount(0)
{
}

//--------------------------------------------------------------------------------
template <class T>
bool ProtoAtolFR<T>::isFiscalReady(bool aOnline, EFiscalPrinterCommand::Enum aCommand)
{
	if (!T::isFiscalReady(aOnline, aCommand))
	{
		return false;
	}
	else if (aCommand == EFiscalPrinterCommand::Encashment)
	{
		int sessionInterval = mLastOpenSession.secsTo(QDateTime::currentDateTime()) / 60;

		return sessionInterval < 24 * 60;
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
void ProtoAtolFR<T>::setInitialData()
{
	T::setInitialData();

	mFRBuild = 0;
}

//--------------------------------------------------------------------------------
template <class T>
bool ProtoAtolFR<T>::isConnected()
{
	CAtolFR::TModelKey modelKey;

	if (!processModelKey(modelKey))
	{
		return false;
	}

	CAtolFR::CModelData modelData;

	if (!modelData.data().keys().contains(modelKey))
	{
		toLog(LogLevel::Error, "AtolFR: Unknown model");
	}

	mModelData = modelData[modelKey];
	mDeviceName = mModelData.name;
	mVerified = mModelData.verified;
	mCanProcessZBuffer = mModelData.ZBufferSize;
	mModelCompatibility = mSupportedModels.contains(mDeviceName);
	setConfigParameter(CHardware::Printer::NeedCutting, mModelData.cutter);

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool ProtoAtolFR<T>::processModelKey(CAtolFR::TModelKey & /*aModeKey*/)
{
	return false;
}

//--------------------------------------------------------------------------------
template <class T>
bool ProtoAtolFR<T>::processReceipt(const QStringList & aReceipt, bool aProcessing)
{
	if (!isPrintingNeed(aReceipt))
	{
		return true;
	}

	bool result = T::processReceipt(aReceipt, aProcessing);

	if (aProcessing)
	{
		SleepHelper::msleep(CAtolFR::Timeouts::EndNotFiscalPrint);
	}

	return result;
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoAtolFR<T>::performEncashment(const QStringList & aReceipt, double aAmount)
{
	openFRSession();

	return T::performEncashment(aReceipt, aAmount);
}

//--------------------------------------------------------------------------------
