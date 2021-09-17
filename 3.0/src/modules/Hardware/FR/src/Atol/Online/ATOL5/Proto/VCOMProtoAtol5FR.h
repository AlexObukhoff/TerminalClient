/* @file Прото-ФР на базе платформы АТОЛ5 для VCOM-порта. */

#pragma once

// Modules
#include "Hardware/Common/SerialDeviceBase.h"

// Project
#include "ProtoAtol5FR.h"

//--------------------------------------------------------------------------------
template <>
class ProtoAtol5FR<SDK::Driver::CInteractionTypes::ItExternalVCOM>: public TPollingExternalFR
{
	SET_INTERACTION_TYPE(ExternalVCOM)

public:
	/// Проверка наличия функционала, предполагающего связь с устройством.
	bool checkConnectionParameters(AtolDriverWrapper * aDriver, CAtol5OnlineFR::TConnectionParameters & aParametersChanged);

protected:
	/// Логгировать параметры соединений с устройством.
	void logConnectionParameters();
};

//--------------------------------------------------------------------------------
