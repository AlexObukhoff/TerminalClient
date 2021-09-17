/* @file Данные виртуальных COM-портов. */

#pragma once

namespace SDK {
namespace Driver {
namespace VCOM {

const char None[] = "";

/// Идентификаторы производителей устройств.
namespace ManufacturerTags
{
	// Фискальные регистраторы.
	namespace FR
	{
		const char Atol[]    = "Atol";       // АТОЛы и Казначей
		const char MStar[]   = "MSTAR";      // MSTAR-TK, MSTAR-TUP-K (OPOS) или MSTAR-TK2
		const char Virtual[] = "com0com";    // только NeoService
	}

	// Модемы. Но его на порту может и не быть, если unmanaged connection.
	namespace Modem
	{
		const char Huawei[]  = "Huawei";
		const char Siemens[] = "Siemens";
		const char SimTech[] = "SimTech HS-USB";
	}

	// Сканеры.
	namespace Scanner
	{
		const char AreaImager[] = "Area Imager";    // сканер, но только если не найдётся на USB
	}
}

/// Идентификаторы виртуальных портов, на которых находятся устройства.
namespace AdapterTags
{
	// Silicon Labs CP210x.
	const char CP210[] = "CP210";

	// чип FTDI.
	const char FTDI[] = "FTDI";

	// Встроенный порт STMicroelectronics.
	const char STMicroelectronics[] = "STMicroelectronics";
}

// Тип порта.
namespace Types
{
	const char Manufacturer[] = "manufacturer";    // Порт фирмы-производителя устройства. Другие устройства искать на этом порту нет смысла.
	const char Adapter[]      = "adapter";         // Порт-адаптер на чипе типа FTDI или CP210x. Другие устройства также могут жить на таком порту.
};

/// Типы соединений по COM/VCOM порту.
namespace ConnectionTypes
{
	const char Dual[] = "dual";
	const char COMOnly[] = "com_only";
	const char VCOMOnly[] = "vcom_only";
}

}}} // SDK::Driver::VCOM

//---------------------------------------------------------------------------
