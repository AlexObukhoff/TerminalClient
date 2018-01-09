/* @file Базовый класс протокола. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtEndian>
#include <QtCore/QMap>
#include <QtCore/QSet>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/IIOPort.h>
#include <SDK/Drivers/IOPort/COMParameters.h>

// Common
#include <Common/SleepHelper.h>

// Devices
#include "Hardware/Common/ASCII.h"
#include "Hardware/Common/HardwareConstants.h"
#include "Hardware/Protocols/Common/ProtocolNames.h"
#include "Hardware/Protocols/Common/IDeviceProtocol.h"

//--------------------------------------------------------------------------------
/// Базовый класс протокола девайса. Инкапсулирует порт, лог и проч.
template <class T>
class DeviceProtocolBase : public T
{
public:

	DeviceProtocolBase();
	virtual ~DeviceProtocolBase();

	/// Получение параметров порта протокола девайса.
	virtual TDevicePortParameters & getPortParameters();

	/// Получение названия протокола.
	virtual const QString & getName() const;

	/// Получение списка типов (названий), ассоциированных с данным протоколом.
	virtual TProtocolAliases getAliases() const;

protected:
	/// Настройки порта, поддерживаемые данным протоколом.
	TDevicePortParameters mPortParameters;

	/// Тип протокола.
	QString mProtocolName;

	/// Лог.
	ILog * mLog;
};

//--------------------------------------------------------------------------------
