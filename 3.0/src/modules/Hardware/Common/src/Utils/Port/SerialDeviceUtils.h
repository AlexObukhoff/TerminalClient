/* @file Набор функционала для работы с системными ресурсами для [V]COM-портов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <QtCore/QVector>
#include <QtCore/QUuid>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/IIOPort.h>

// Project
#include "Hardware/IOPorts/IOPortGUIDs.h"
#include "SystemDeviceUtils.h"

//--------------------------------------------------------------------------------
/// Данные портов.
typedef QMap<QString, QString> TIOPortDeviceData;

typedef QVector<QUuid> TUuids;

namespace CSerialDeviceUtils
{
	/// Id для идентификации COM-портов.
	namespace Tags
	{
		/// Виртуальные COM-порты (через USB).
		inline QStringList Virtual() { return QStringList()
			<< "USB"         /// Драйвер cp210x и совместимые.
			<< "FTDI"        /// Чип FTDI.
			<< "Virtual";    /// Что-то виртуальное.
		}

		/// Эмуляторы (программные) COM-порты.
		inline QStringList Emulator() { return QStringList()
			<< "COM0COM"
			<< "Emulator";
		}
	}

	/// GUIDы для автопоиска портов. Класс нужен для использования в static-фунцкии.
	class Uuids: public TUuids
	{
	public:
		Uuids()
		{
			append(GUID_DEVINTERFACE_COMPORT);
			append(GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR);
			append(GUIDs::USB1);
			append(GUIDs::USB2);
			append(GUIDs::USB3);
		}
	};

	/// Cвойство для формирования пути для открытия порта.
	const DWORD PathProperty = SPDRP_FRIENDLYNAME;
}

//--------------------------------------------------------------------------------
namespace SerialDeviceUtils
{
	/// Получение системных данных о портах (порт -> виртуальность).
	typedef QMap<QString, SDK::Driver::EPortTypes::Enum> TData;
	TData getSystemData(bool aForce = false);

	/// Получить данные о ресурсах.
	TWinDeviceProperties getDeviceProperties(const TUuids & aUuids, DWORD aPropertyName, bool aQuick = false, TIOPortDeviceData * aData = nullptr);
}

//--------------------------------------------------------------------------------
