/* @file Cервис для работы с HID-устройствами */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
class IHIDService : public QObject
{
	Q_OBJECT

public:
	/// Включить возможность получения данных.
	virtual bool setEnable(bool aEnable, const QString & aDevice = QString()) = 0;

	/// Конвертирует данные сканера в строку
	virtual QString valueToString(const QVariant & aData) = 0;

signals:
	/// Сигнал о получении сырых данных с HID-устройства.
	void data(const QVariantMap & aData);

	/// Сигнал об ошибке чтения штрихкода/карты/т.п.
	void error();

	/// Сингал о том, что карта вставлена
	void inserted(const QVariantMap & aData);

	/// Сигнал - карта изъята
	void ejected();

protected:
	virtual ~IHIDService() {}
};

//------------------------------------------------------------------------------
}} // SDK::PaymentProcessor
