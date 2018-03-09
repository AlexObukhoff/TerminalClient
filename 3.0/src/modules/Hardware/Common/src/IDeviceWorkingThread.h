/* @file Интерфейс рабочиего потока устройства. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/WarningLevel.h>

//--------------------------------------------------------------------------------
class IDeviceWorkingThread : public QThread
{
	Q_OBJECT

signals:
	/// Изменение состояния.
	void status(SDK::Driver::EWarningLevel::Enum, const QString &, int);

	/// Окончание инициализации.
	void initialized();

public slots:
	/// Инициализация.
	virtual void initialize() = 0;

	/// Идентификация.	
	virtual bool checkExistence() = 0;
};

//--------------------------------------------------------------------------------
