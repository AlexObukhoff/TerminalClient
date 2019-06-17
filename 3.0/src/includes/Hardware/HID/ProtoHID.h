/* @file Прото-HID-устройство. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QVariant>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/IHID.h>

// Modules
#include "Hardware/Common/ProtoDevice.h"

//--------------------------------------------------------------------------------
class ProtoHID : public ProtoDevice, public MetaDevice<SDK::Driver::IHID>
{
	Q_OBJECT

	SET_DEVICE_TYPE(Scanner)

signals:
	/// Событие о новых введённых данных.
	void data(const QVariantMap & aData);
};

//--------------------------------------------------------------------------------
