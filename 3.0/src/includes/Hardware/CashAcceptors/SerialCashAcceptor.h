/* @file  упюроприемник на COM-порту. */

#pragma once

#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/SerialDeviceBase.h"
#include "Hardware/CashAcceptors/PortCashAcceptor.h"

//--------------------------------------------------------------------------------
class SerialCashAcceptor : public PortCashAcceptor<SerialDeviceBase<PortPollingDeviceBase<ProtoCashAcceptor>>>
{
	SET_VCOM_DATA(None, ConnectionTypes::COMOnly, None)
};

//-------------------------------------------------------------------------------- 
