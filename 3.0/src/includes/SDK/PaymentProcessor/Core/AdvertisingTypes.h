/* @file Типы системных событий. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
// Сделан в виде класса, чтобы получить метаданные.
class AdvertisingType : public QObject
{
	Q_OBJECT
	Q_ENUMS(Enum)

public:
	enum Enum
	{
		MainScreenBanner,      /// Баннер на первом экране.
		PaymentReceipt,        /// Текстовая реклама на чеке платежа.
	};
};

//---------------------------------------------------------------------------
}} // SDK::PaymentProcessor
