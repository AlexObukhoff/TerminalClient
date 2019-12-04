/* @file Базовый класс устройств с поллингом. */

#include "Hardware/Common/ProtoDevices.h"
#include "Hardware/Dispensers/ProtoDispenser.h"
#include "Hardware/CashAcceptors/ProtoCashAcceptor.h"
#include "Hardware/FR/ProtoFR.h"
#include "Hardware/HID/ProtoHID.h"
#include "Hardware/CardReaders/ProtoMifareReader.h"
#include "Hardware/Watchdogs/ProtoWatchdog.h"

#include "PollingDeviceBase.h"

//--------------------------------------------------------------------------------
template class PollingDeviceBase<ProtoPrinter>;
template class PollingDeviceBase<ProtoDispenser>;
template class PollingDeviceBase<ProtoCashAcceptor>;
template class PollingDeviceBase<ProtoFR>;
template class PollingDeviceBase<ProtoHID>;
template class PollingDeviceBase<ProtoMifareReader>;
template class PollingDeviceBase<ProtoDeviceBase>;
template class PollingDeviceBase<ProtoWatchdog>;

//--------------------------------------------------------------------------------
template <class T>
PollingDeviceBase<T>::PollingDeviceBase() : mPollingInterval(0), mPollingActive(false), mForceNotWaitFirst(false)
{
	mPolling.moveToThread(&mThread);
	connect(&mPolling, SIGNAL(timeout()), SLOT(onPoll()));
}

//--------------------------------------------------------------------------------
template <class T>
void PollingDeviceBase<T>::releasePolling()
{
	if (!isAutoDetecting() && (!mStatusCollection.isEmpty() || (mInitialized == ERequestStatus::InProcess)))
	{
		waitCondition([&] () -> bool { return mPollingActive; }, CPollingDeviceBase::StopWaiting);
	}

	stopPolling();
}

//--------------------------------------------------------------------------------
template <class T>
bool PollingDeviceBase<T>::release()
{
	releasePolling();

	return DeviceBase<T>::release();
}

//--------------------------------------------------------------------------------
template <class T>
void PollingDeviceBase<T>::finaliseInitialization()
{
	if (!mConnected)
	{
		processStatusCodes(TStatusCodes() << DeviceStatusCode::Error::NotAvailable);
	}

	bool notWaitFirst = mForceNotWaitFirst || !mConnected;

	startPolling(notWaitFirst);
}

//--------------------------------------------------------------------------------
template <class T>
void PollingDeviceBase<T>::setPollingActive(bool aActive)
{
	if (!mOperatorPresence)
	{
		toLog(LogLevel::Normal, aActive ? "Start polling." : "Stop polling.");

		aActive ? mPolling.start(mPollingInterval) : mPolling.stop();

		mPollingActive = aActive;
	}
}

//--------------------------------------------------------------------------------
template <class T>
void PollingDeviceBase<T>::startPolling(bool aNotWaitFirst)
{
	if (mPollingActive)
	{
		return;
	}

	Qt::ConnectionType connectionType = !isWorkingThread() ? Qt::BlockingQueuedConnection : Qt::DirectConnection;

	if (mPollingInterval)
	{
		QMetaObject::invokeMethod(this, "setPollingActive", connectionType, Q_ARG(bool, true));
	}

	if (!aNotWaitFirst)
	{
		QMetaObject::invokeMethod(this, "onPoll", connectionType);
	}
}

//--------------------------------------------------------------------------------
template <class T>
void PollingDeviceBase<T>::stopPolling(bool aWait)
{
	if (mPollingActive)
	{
		Qt::ConnectionType connectionType = !isWorkingThread() ? Qt::BlockingQueuedConnection : Qt::DirectConnection;
		QMetaObject::invokeMethod(this, "setPollingActive", connectionType, Q_ARG(bool, false));

		if (aWait)
		{
			waitCondition([&] () -> bool { return !mPollingActive; }, CPollingDeviceBase::StopWaiting);
		}
	}
}

//--------------------------------------------------------------------------------
template <class T>
void PollingDeviceBase<T>::setPollingInterval(int aPollingInterval)
{
	Q_ASSERT(aPollingInterval != 0);

	int delta = mPollingInterval - aPollingInterval;
	mPollingInterval = aPollingInterval;

	if (mPollingActive && delta)
	{
		stopPolling();
		startPolling(false);
	}
}

//--------------------------------------------------------------------------------
template <class T>
void PollingDeviceBase<T>::postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection)
{
	if (mConnected)
	{
		for (int i = 0; i < mPPTaskList.size(); ++i)
		{
				mPPTaskList[i]();

				mPPTaskList.removeAt(i--);
		}
	}

	DeviceBase::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

//--------------------------------------------------------------------------------
template <class T>
bool PollingDeviceBase<T>::waitCondition(TBoolMethod aCondition, const SWaitingData & aWaitingData)
{
	if (isWorkingThread())
	{
		return aCondition();
	}

	PollingExpector expector;

	return expector.wait(aCondition, aWaitingData);
}

//--------------------------------------------------------------------------------
template <class T>
void PollingDeviceBase<T>::reInitialize()
{
	stopPolling(false);

	DeviceBase<T>::reInitialize();
}

//---------------------------------------------------------------------------
template <class T>
bool PollingDeviceBase<T>::isInitializationError(TStatusCodes & aStatusCodes)
{
	return mPollingActive && DeviceBase<T>::isInitializationError(aStatusCodes);
}

//--------------------------------------------------------------------------------
