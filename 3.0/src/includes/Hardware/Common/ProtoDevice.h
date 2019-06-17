/* @file Прото-устройство. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/WarningLevel.h>
#include <SDK/Drivers/Components.h>

// Project
#include "Hardware/Common/MetaDevice.h"

#define SET_DEVICE_TYPE(aType) public: static QString getDeviceType() { return SDK::Driver::CComponents::aType; }

//--------------------------------------------------------------------------------
class ProtoDevice : public QObject
{
	Q_OBJECT

signals:
	/// Изменение состояния.
	void status(SDK::Driver::EWarningLevel::Enum aLevel, const QString & aMessage, int aExtendedStatus);

	/// Окончание инициализации.
	void initialized();

	/// Прошивка обновлена.
	void updated(bool aSuccess);

	/// Необходимо обновить конфигурацию.
	void configurationChanged();

public slots:
	/// Инициализация.
	virtual void initialize() {};

	/// Идентификация.	
	virtual bool checkExistence() { return true; };

	/// Обработчик сигнала полла.
	virtual void onPoll() {};

protected slots:
	/// Запуск/останов поллинга.
	virtual void setPollingActive(bool /*aActive*/) {};

	/// Обновить прошивку.
	virtual void updateFirmware(const QByteArray & /*aBuffer*/) {};

	/// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
	virtual bool performRelease() { return true; };
};

//--------------------------------------------------------------------------------
