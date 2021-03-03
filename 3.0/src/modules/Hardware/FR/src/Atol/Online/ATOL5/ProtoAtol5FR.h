/* @file Прото-ФР на базе платформы АТОЛ5 для кастомизации по каналам взаимодействия. */

#pragma once

// Modules
#include "Hardware/Common/PollingDeviceBase.h"
#include "Hardware/FR/ProtoFR.h"

// Project
#include "Wrapper/AtolDriverWrapper.h"

namespace CAtol5OnlineFR
{
	typedef QList<bool> TConnectionParameters;
}

//--------------------------------------------------------------------------------
template <class T>
class ProtoAtol5FR: public PollingDeviceBase<ProtoFR>
{
public:
	/// Проверка наличия функционала, предполагающего связь с устройством.
	bool checkConnectionParameters(AtolDriverWrapper * /*aDriver*/, CAtol5OnlineFR::TConnectionParameters & /*aParametersChanged*/)
	{
		return false;
	}
};

//--------------------------------------------------------------------------------
template <>
class ProtoAtol5FR<SDK::Driver::CInteractionTypes::ItExternalVCOM>: public PollingDeviceBase<ProtoFR>
{
	SET_INTERACTION_TYPE(ExternalVCOM)

public:
	/// Проверка наличия функционала, предполагающего связь с устройством.
	bool checkConnectionParameters(AtolDriverWrapper * aDriver, CAtol5OnlineFR::TConnectionParameters & aParametersChanged)
	{
		if (!checkConnectionParameter(CHardwareSDK::SystemName))
		{
			return false;
		}

		QVariantMap requiredResourceParameters = getConfigParameter(CHardwareSDK::RequiredResourceParameters).toMap();
		QString portName = requiredResourceParameters[CHardwareSDK::SystemName].toString();

		aParametersChanged = CAtol5OnlineFR::TConnectionParameters()
			<< aDriver->checkSetting<int>(LIBFPTR_SETTING_PORT, LIBFPTR_PORT_COM)
			<< aDriver->checkSetting<QString>(LIBFPTR_SETTING_COM_FILE, portName);

		return true;
	}
};

//--------------------------------------------------------------------------------
template <>
class ProtoAtol5FR<SDK::Driver::CInteractionTypes::ItExternalCOM>: public PollingDeviceBase<ProtoFR>
{
	SET_INTERACTION_TYPE(ExternalCOM)

public:
	/// Проверка наличия функционала, предполагающего связь с устройством.
	bool checkConnectionParameters(AtolDriverWrapper * aDriver, CAtol5OnlineFR::TConnectionParameters & aParametersChanged)
	{
		if (!checkConnectionParameter(CHardwareSDK::SystemName) || !checkConnectionParameter(CHardwareSDK::Port::COM::BaudRate))
		{
			return false;
		}

		QVariantMap requiredResourceParameters = getConfigParameter(CHardwareSDK::RequiredResourceParameters).toMap();
		QString portName = requiredResourceParameters[CHardwareSDK::SystemName].toString();
		QString baudRate = requiredResourceParameters[CHardwareSDK::Port::COM::BaudRate].toString();

		aParametersChanged = CAtol5OnlineFR::TConnectionParameters()
			<< aDriver->checkSetting<int>(LIBFPTR_SETTING_PORT, LIBFPTR_PORT_COM)
			<< aDriver->checkSetting<QString>(LIBFPTR_SETTING_COM_FILE, portName)
			<< aDriver->checkSetting<QString>(LIBFPTR_SETTING_BAUDRATE, baudRate);

		return true;
	}
};

//--------------------------------------------------------------------------------
