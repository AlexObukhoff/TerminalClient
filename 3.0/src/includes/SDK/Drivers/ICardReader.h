/* @file Интерфейс драйвера кардридера. */
#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMetaType>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/IDevice.h>
#include <SDK/Drivers/CardReader/CardReaderStatus.h>

//------------------------------------------------------------------------------
namespace SDK {
namespace Driver {

/// Типы карт.
namespace ECardType
{
	enum Enum
	{
		RF,        /// Радиочастотные.

		MS,        /// С магнитной полосой.
		IC,        /// С чипом.
		MSIC,      /// С магнитной полосой и чипом.
		MSICRF     /// Радиочастотные c магнитной полосой и чипом.
	};
}

class ICardReader: public IDevice
{
public:
	/// Карта вставлена.
	static const char * InsertedSignal; // = SIGNAL(inserted(ECardType::Enum, const QVariantMap &));

	/// Карта извлечена.
	static const char * EjectedSignal; // = SIGNAL(ejected());

	/// Проверка доступности устройства и карты.
	virtual bool isDeviceReady() const = 0;

	/// Выбросить карту (для моторизированных ридеров) или отключить электрически (для немоторизованных).
	virtual void eject() = 0;

protected:
	virtual ~ICardReader() {}
};

}} // namespace SDK::Driver

Q_DECLARE_METATYPE(SDK::Driver::ECardType::Enum);

//--------------------------------------------------------------------------------
