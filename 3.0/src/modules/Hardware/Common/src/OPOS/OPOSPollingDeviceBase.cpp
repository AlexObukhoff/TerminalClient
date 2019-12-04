/* @file Базовый класс устройств на OPOS-функционале. */

// windows
#include <objbase.h>

// Qt
#include <Common/QtHeadersBegin.h>

// OPOS
#pragma warning(disable: 4100) // warning C4100: 'identifier' : unreferenced formal parameter
#include <OPOS/QtWrappers/FiscalPrinter.h>
#include <OPOS/QtWrappers/Scanner.h>
#include <Common/QtHeadersEnd.h>

// Modules
#include "SysUtils/ISysUtils.h"
#include "Hardware/FR/ProtoFR.h"
#include "Hardware/HID/ProtoOPOSScanner.h"

// Project
#include "OPOSPollingDeviceBase.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
template <class T, class T2>
OPOSPollingDeviceBase<T, T2>::OPOSPollingDeviceBase():
	mCOMInitialized(false),
	mLastStatus(OPOS::OPOS_SUCCESS),
	mClaimTimeout(1000),
	mTriedToConnect(false),
	mOpened(false),
	mThreadProxy(&mThread)
{
	mThreadProxy.moveToThread(&mThread);

	mNotLogResult << "ResultCode" << "ResultCodeExtended" << "CheckHealth";
}

//--------------------------------------------------------------------------------
template <class T, class T2>
void OPOSPollingDeviceBase<T, T2>::initialize()
{
	START_IN_WORKING_THREAD(initialize)

	initializeResources();
	mProfileName = getConfigParameter(CHardware::OPOSName).toString();

	PollingDeviceBase<T>::initialize();

	if (!checkConnectionAbility())
	{
		processStatusCodes(TStatusCodes() << DeviceStatusCode::Error::ThirdPartyDriverFail);
	}
}

//--------------------------------------------------------------------------------
template <class T, class T2>
bool OPOSPollingDeviceBase<T, T2>::checkConnectionAbility()
{
	return mCOMInitialized;
}

//--------------------------------------------------------------------------------
template <class T, class T2>
void OPOSPollingDeviceBase<T, T2>::initializeResources()
{
	if (!mCOMInitialized && FAILED(CoInitializeEx(0, COINIT_APARTMENTTHREADED)))
	{
		toOPOS_LOG(LogLevel::Error, "Failed to init COM, " + ISysUtils::getLastErrorMessage());

		mDriver.clear();
	}
	else
	{
		if (mDriver.isNull())
		{
			mDriver = POPOSDriver(new T2());
		}

		mCOMInitialized = true;
	}
}

//--------------------------------------------------------------------------------
template <class T, class T2>
bool OPOSPollingDeviceBase<T, T2>::close()
{
	if (!mOpened)
	{
		toLog(LogLevel::Debug, mDeviceName + ": wasn`t opened");
		return true;
	}

	mOpened = false;

	int error = INT_CALL_OPOS(Close).error;

	return (error == OPOS::OPOS_SUCCESS) || (error == OPOS::OPOS_E_CLOSED);
}

//--------------------------------------------------------------------------------
template <class T, class T2>
bool OPOSPollingDeviceBase<T, T2>::checkExistence()
{
	mTriedToConnect = false;

	if (!isWorkingThread())
	{
		if (mThread.isRunning())
		{
			QMetaObject::invokeMethod(this, "checkExistence", Qt::QueuedConnection);
		}
		else
		{
			connect(&mThread, SIGNAL(started()), this, SLOT(checkExistence()), Qt::UniqueConnection);
			mThread.start();
		}

		QMutexLocker locker(&mStartMutex);

		if (!mTriedToConnect)
		{
			mStartCondition.wait(&mStartMutex);
		}

		return mConnected;
	}

	initializeResources();

	bool result = checkConnectionAbility() && PollingDeviceBase<T>::checkExistence();
	mTriedToConnect = true;
	mStartCondition.wakeAll();

	return result;
}

//--------------------------------------------------------------------------------
template <class T, class T2>
bool OPOSPollingDeviceBase<T, T2>::isConnected()
{
	if (BOOL_CALL_OPOS(Claimed))
	{
		toOPOS_LOG(LogLevel::Normal, "OPOS device is claimed already.");
		return true;
	}

	int result = INT_CALL_OPOS(Open, mProfileName).error;
	mOpened = (result == OPOS::OPOS_SUCCESS) || (result == OPOS::OPOS_OR_ALREADYOPEN);

	if (!mOpened)
	{
		toOPOS_LOG(LogLevel::Error, "OPOS driver cannot be opened.");
		return false;
	}

	result = 0;
	int index = 0;

	do
	{
		if (index)
		{
			SleepHelper::msleep(500);
		}

		result = INT_CALL_OPOS(ClaimDevice, mClaimTimeout).error;
	}
	while (((result == OPOS::OPOS_E_ILLEGAL) || (result == OPOS::OPOS_E_TIMEOUT)) && (++index < COPOS::ClaimAttempts));

	if (result != OPOS::OPOS_SUCCESS)
	{
		INT_CALL_OPOS(ReleaseDevice);
		close();
		toOPOS_LOG(LogLevel::Error, "OPOS driver can not be claimed, it has been closed.");

		return false;
	}

	setConfigParameter(CHardware::OPOSName, mProfileName);
	setConfigParameter(CHardwareSDK::ModelName, STRING_CALL_OPOS(DeviceName));

	return true;
}

//--------------------------------------------------------------------------------
template <class T, class T2>
bool OPOSPollingDeviceBase<T, T2>::updateParameters()
{
	if (!isConnected())
	{
		return false;
	}

	setDeviceParameter(CDeviceData::OPOS::Description,   STRING_OPOS_PROPERTY(DeviceDescription));
	setDeviceParameter(CDeviceData::OPOS::ControlObject, STRING_OPOS_PROPERTY(ControlObjectDescription));
	setDeviceParameter(CDeviceData::OPOS::ServiceObject, STRING_OPOS_PROPERTY(ServiceObjectDescription));
	setDeviceParameter(CDeviceData::Version, INT_OPOS_PROPERTY(ControlObjectVersion), CDeviceData::OPOS::ControlObject);
	setDeviceParameter(CDeviceData::Version, INT_OPOS_PROPERTY(ServiceObjectVersion), CDeviceData::OPOS::ServiceObject);

	return true;
}

//--------------------------------------------------------------------------------
template <class T, class T2>
bool OPOSPollingDeviceBase<T, T2>::performRelease()
{
	releasePolling();

	bool result = true;

	if (mOpened && BOOL_CALL_OPOS(Claimed))
	{
		int error = INT_CALL_OPOS(ReleaseDevice).error;

		if ((error != OPOS::OPOS_SUCCESS) && (error != OPOS::OPOS_E_CLOSED))
		{
			result = false;
		}
	}

	if (!close())
	{
		result = false;
	}

	CoUninitialize();
	mCOMInitialized = false;

	return result;
}

//--------------------------------------------------------------------------------
template <class T, class T2>
bool OPOSPollingDeviceBase<T, T2>::release()
{
	if (!mCOMInitialized || !mThread.isRunning())
	{
		return true;
	}

	bool result;

	if (!isWorkingThread())
	{
		QMetaObject::invokeMethod(this, "performRelease", Qt::BlockingQueuedConnection, QReturnArgument<bool>("bool", result));
	}
	else
	{
		result = performRelease();
	}

	if (!DeviceBase<T>::release())
	{
		result = false;
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T, class T2>
bool OPOSPollingDeviceBase<T, T2>::functionUse(const COPOS::TFunctionNames & aFunctions, const QString & aFunctionData)
{
	QString functionName = aFunctionData.left(aFunctionData.indexOf("("));

	return std::find_if(aFunctions.begin(), aFunctions.end(), [&](const QString & function) -> bool
		{ return function.startsWith(functionName); }) != aFunctions.end();
}

//--------------------------------------------------------------------------------
template <class T, class T2>
SOPOSResult OPOSPollingDeviceBase<T, T2>::processIntMethod(TIntMethod aMethod, const QString & aFunctionData)
{
	SOPOSResult result;
	result.error = mThreadProxy.invokeMethod<int>(aMethod);

	if (!OPOS_SUCCESS(result))
	{
		bool isFunctionUse = functionUse(mNotLogResult, aFunctionData);
		bool isStatus = functionUse(COPOS::TFunctionNames() << COPOS::Status, aFunctionData);
		auto isStatusChanged = [&] (const SOPOSResult & aResult) -> bool { bool result = isStatus && (mLastStatus != aResult); if (result) mLastStatus = aResult; return result; };

		if (result.error == OPOS::OPOS_E_EXTENDED)
		{
			result.error = INT_CALL_OPOS(ResultCodeExtended).error;
			result.extended = true;
		}

		if (!isFunctionUse || isStatusChanged(result))
		{
			QString log = QString("%1: %2 returns %3code %4").arg(mDeviceName).arg(aFunctionData).arg(result.extended ? "extended " : "").arg(result.error);
			QString description = getErrorDescription();

			if (!description.isEmpty())
			{
				log += ", description: " + description;
			}

			toOPOS_LOG(LogLevel::Error, log);
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T, class T2>
QString OPOSPollingDeviceBase<T, T2>::getErrorDescription()
{
	return "";
}

//--------------------------------------------------------------------------------
template <class T, class T2>
bool OPOSPollingDeviceBase<T, T2>::checkHealth(TStatusCodes & aStatusCodes, SOPOSResult & aResult)
{
	if (!checkConnectionAbility())
	{
		aStatusCodes.insert(DeviceStatusCode::Error::ThirdPartyDriverFail);
		return true;
	}

	if (!mConnected)
	{
		if (!isConnected())
		{
			return false;
		}

		updateParameters();
	}

	aResult = INT_CALL_OPOS(CheckHealth, OPOS::OPOS_CH_INTERNAL);

	if (aResult.error == OPOS::OPOS_SUCCESS)
	{
		return true;
	}

	aStatusCodes.insert(DeviceStatusCode::Error::Unknown);
	int error = aResult.error;

	return aResult.extended ||
		!(// Устройство не подключено
		(error == OPOS::OPOS_E_NOHARDWARE) ||
		(error == OPOS::OPOS_E_OFFLINE)    ||
		// Устройство не проинициализировано должным образом
		(error == OPOS::OPOS_E_CLOSED)     ||
		(error == OPOS::OPOS_E_NOTCLAIMED) ||
		(error == OPOS::OPOS_E_NOSERVICE)  ||
		(error == OPOS::OPOS_E_DISABLED)   ||
		// Устройство занято либо другой программой, либо своими делами
		(error == OPOS::OPOS_E_CLAIMED)    ||
		(error == OPOS::OPOS_E_TIMEOUT)    ||
		(error == OPOS::OPOS_E_BUSY)       ||
		// Возможно, имеет место несоответсвие протокола взаимодействия
		(error == OPOS::OPOS_E_ILLEGAL)    ||
		(error == OPOS::OPOS_E_DEPRECATED) ||
		(error == OPOS::OPOS_E_NOEXIST)    ||
		(error == OPOS::OPOS_E_EXISTS));
}

//--------------------------------------------------------------------------------
template <class T, class T2>
bool OPOSPollingDeviceBase<T, T2>::getStatus(TStatusCodes & aStatusCodes)
{
	SOPOSResult result;

	if (checkHealth(aStatusCodes, result) && !aStatusCodes.contains(DeviceStatusCode::Error::NotAvailable))
	{
		return true;
	}

	if (mOpened)
	{
		INT_CALL_OPOS(ReleaseDevice);
		close();
	}

	return false;
}

//--------------------------------------------------------------------------------
template <class T, class T2>
SDK::Driver::IDevice::IDetectingIterator * OPOSPollingDeviceBase<T, T2>::getDetectingIterator()
{
	mDetectingPosition = -1;

	return mProfileNames.size() > 0 ? this : nullptr;
}

//--------------------------------------------------------------------------------
template <class T, class T2>
bool OPOSPollingDeviceBase<T, T2>::find()
{
	if ((mDetectingPosition < 0) || (mDetectingPosition >= mProfileNames.size()))
	{
		return false;
	}

	mProfileName = mProfileNames[mDetectingPosition];
	toOPOS_LOG(LogLevel::Normal, "Set OPOS profile name " + mProfileName);

	return checkExistence();
}

//--------------------------------------------------------------------------------
template <class T, class T2>
bool OPOSPollingDeviceBase<T, T2>::moveNext()
{
	mDetectingPosition++;

	return (mDetectingPosition >= 0) && (mDetectingPosition < mProfileNames.size());
}

//--------------------------------------------------------------------------------
