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
#include <SDK/Drivers/IOPort/VCOMData.h>

// Project
#include "Hardware/IOPorts/IOPortGUIDs.h"
#include "SystemDeviceUtils.h"

//--------------------------------------------------------------------------------
/// Данные портов.
typedef QMap<QString, QString> TIOPortDeviceData;

typedef QVector<QUuid> TUuids;

namespace CSerialDeviceUtils
{
	/// Id для идентификации COM/VCOM/USB-портов.
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

		/// Натуральные COM-порты, корректно воспроизводят протокол RS-232.
		inline QStringList COM() { return QStringList()
			<< "ACPI"
			<< "PCI";
		}

		/// Устройства, не имеющие значения при [авто-]поиске/работе терминальных устройств.
		inline QStringList Unimportant() { return QStringList()
			<< "mouse"
			<< "touch"
			<< "hdaudio";
		}

		/// Physical Device Object (PDO).
		const char USBPDO[] = "USBPDO";
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

	inline QStringList VCOMAdapterTags() { return QStringList()
		<< SDK::Driver::VCOM::AdapterTags::CP210
		<< SDK::Driver::VCOM::AdapterTags::FTDI
		<< SDK::Driver::VCOM::AdapterTags::STMicroelectronics;
	}

	inline QStringList AllVCOMTags() { return QStringList()
		<< SDK::Driver::VCOM::ManufacturerTags::FR::Atol
		<< SDK::Driver::VCOM::ManufacturerTags::FR::MStar
		<< SDK::Driver::VCOM::ManufacturerTags::FR::Virtual

		<< SDK::Driver::VCOM::ManufacturerTags::Modem::Huawei
		<< SDK::Driver::VCOM::ManufacturerTags::Modem::Siemens
		<< SDK::Driver::VCOM::ManufacturerTags::Modem::SimTech

		<< SDK::Driver::VCOM::ManufacturerTags::Scanner::AreaImager

		<< VCOMAdapterTags();
	}
}

//--------------------------------------------------------------------------------
namespace SerialDeviceUtils
{
	/// Получает системные данные о портах (порт -> виртуальность).
	typedef QMap<QString, SDK::Driver::EPortTypes::Enum> TData;
	TData getSystemData(bool aForce = false);

	/// Получает данные о ресурсах.
	TWinDeviceProperties getDeviceProperties(const TUuids & aUuids, DWORD aPropertyName, bool aQuick = false, TIOPortDeviceData * aData = nullptr);
}

//--------------------------------------------------------------------------------
