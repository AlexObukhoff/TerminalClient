/* @file Прото-ФР на базе платформы АТОЛ5 для кастомизации по каналам взаимодействия. */

#pragma once

// Modules
#include "Hardware/Common/PollingDeviceBase.h"
#include "Hardware/Common/ExternalPortDeviceBase.h"
#include "Hardware/Common/SerialDeviceUtils.h"
#include "Hardware/FR/ProtoFR.h"

// Project
#include "../Wrapper/AtolDriverWrapper.h"

//--------------------------------------------------------------------------------
namespace CAtol5OnlineFR
{
	typedef QList<bool> TConnectionParameters;
}

//--------------------------------------------------------------------------------
typedef PollingDeviceBase<ExternalPortDeviceBase<ProtoFR>> TPollingExternalFR;

template <class T>
class ProtoAtol5FR: public TPollingExternalFR
{
public:
	/// Проверка наличия функционала, предполагающего связь с устройством.
	bool checkConnectionParameters(AtolDriverWrapper * /*aDriver*/, CAtol5OnlineFR::TConnectionParameters & /*aParametersChanged*/)
	{
		return false;
	}

protected:
	/// Логгировать параметры соединений с устройством.
	void logConnectionParameters() {}
};

//--------------------------------------------------------------------------------
