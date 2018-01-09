/* @file Прокси-класс для работы со сканером в скриптах. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/Event.h>

// Modules
#include <Common/ILogable.h>

namespace SDK {
namespace PaymentProcessor {

class ICore;
class IHIDService;

namespace Scripting {

namespace HID {
	const char STRING[] = "hid_string";
	const char EXTERNAL_DATA[] = "hid_external_data";
	const char RAW[] = "hid_raw";
	const char RAW_BASE64[] = "hid_raw_base64";
	const char SOURCE[] = "hid_source";
	const char SOURCE_CAMERA[] = "camera";
	const char SOURCE_SCANNER[] = "scanner";
	const char SOURCE_CARD[] = "card";
	const char SIGNAL[] = "signal";
	const char SIGNAL_INSERT[] = "insert";
	const char SIGNAL_EJECT[] = "eject";

	const char CAMERA_FACE_DETECTED[] = "hid_camera_face_detected";
	const char CAMERA_FACE_DETECTED_IMAGE[] = "hid_camera_face_image_base64";
}

//------------------------------------------------------------------------------
class HIDService : public QObject, public ILogable
{
	Q_OBJECT

public:
	HIDService(ICore * aCore);

public slots:
	// Обновить параметры сервиса
	void updateParameters(const QVariantMap & aParameters);

	void executeExternalHandler(const QVariantMap & aExpression);

	// Если имя не указываем, то вкл/выкл делаем для всех устройств
	void enable(const QString & aName = QString());

	void disable(const QString & aName = QString());

signals:
	/// Сигнал получения данных со сканера
	void HIDData(const QVariantMap & aData);

	/// Сигнал об ошибке чтения штрихкода/карты/т.п.
	void error();

	void externalHandler(const QVariantMap & aExpression);

private slots:
	/// Обработчик сигнала от сервиса ядра
	void onData(const QVariantMap & aData);

	/// Карта вставлена
	void onInserted(const QVariantMap & aData);

	/// Карта изъята
	void onEjected();

private:
	/// Возвращает скрипт on_external_data для активного платежа, если он есть
	QString getExternalData();

private:
	ICore * mCore;
	IHIDService * mService;
	QVariantMap mParameters;
};

//------------------------------------------------------------------------------
}}} // Scripting::PaymentProcessor::SDK
