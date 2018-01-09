/* @file Базовый класс устройств на порту с поллингом. */

// Modules
#include "Hardware/Common/ProtoDevices.h"
#include "Hardware/Dispensers/ProtoDispenser.h"
#include "Hardware/CashAcceptors/ProtoCashAcceptor.h"
#include "Hardware/Watchdogs/ProtoWatchdog.h"
#include "Hardware/FR/ProtoFR.h"
#include "Hardware/HID/ProtoHID.h"
#include "Hardware/CardReaders/ProtoMifareReader.h"

// Project
#include "PortPollingDeviceBase.h"

//-------------------------------------------------------------------------------

template class PortPollingDeviceBase<ProtoPrinter>;
template class PortPollingDeviceBase<ProtoDispenser>;
template class PortPollingDeviceBase<ProtoCashAcceptor>;
template class PortPollingDeviceBase<ProtoWatchdog>;
template class PortPollingDeviceBase<ProtoFR>;
template class PortPollingDeviceBase<ProtoHID>;
template class PortPollingDeviceBase<ProtoMifareReader>;

//--------------------------------------------------------------------------------
template <class T>
void PortPollingDeviceBase<T>::initialize()
{
	START_IN_WORKING_THREAD(initialize)

	PortDeviceBase<PollingDeviceBase<T>>::initialize();

	startPolling(true);
}

//--------------------------------------------------------------------------------
template <class T>
void PortPollingDeviceBase<T>::setPollingActive(bool aActive)
{
	QVariantMap configuration = mIOPort->getDeviceConfiguration();

	if (!configuration.value(CHardware::Port::Suspended).toBool())
	{
		PortDeviceBase<PollingDeviceBase<T>>::setPollingActive(aActive);
	}
}

//--------------------------------------------------------------------------------
