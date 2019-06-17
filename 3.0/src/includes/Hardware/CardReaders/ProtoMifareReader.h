/* @file Прото-Mifare-кардридер. */

#pragma once

// SDK
#include <SDK/Drivers/IMifareReader.h>

// Modules
#include "Hardware/Common/ProtoDevice.h"

//--------------------------------------------------------------------------------
class ProtoMifareReader : public ProtoDevice, public MetaDevice<SDK::Driver::IMifareReader>
{
	Q_OBJECT

	SET_DEVICE_TYPE(CardReader)

signals :
	/// Карта вставлена.
	void inserted(SDK::Driver::ECardType::Enum, const QVariantMap &);

	/// Карта извлечена.
	void ejected();
};

//--------------------------------------------------------------------------------
