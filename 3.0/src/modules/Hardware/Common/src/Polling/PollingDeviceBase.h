/* @file Базовый класс устройств с поллингом. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTimer>
#include <Common/QtHeadersEnd.h>

// Project
#include "Hardware/Common/DeviceBase.h"

namespace CPollingDeviceBase
{
	/// Таймаут остановки поллинга по-умолчанию, [мс].
	const int DefaultStopTimeout = 15 * 1000;
}

//---------------------------------------------------------------------------
template <class T>
class PollingDeviceBase : public DeviceBase<T>
{
public:
	PollingDeviceBase();

	/// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
	virtual bool release();

protected:
	/// Завершение инициализации.
	virtual void finaliseInitialization();

	/// Есть ли ошибка инициализации при фильтрации статусов.
	bool isInitializationError(TStatusCodes & aStatusCodes);

	/// Запуск поллинга.
	void startPolling(bool aNotWaitFirst = false);

	/// Останов поллинга.
	void stopPolling(bool aWait = true);

	/// Установить таймаут.
	void setPollingInterval(int aPollingInterval);

	/// Ожидание состояния или выполнения полла.
	bool waitCondition(TBoolMethod aCondition, int aPollingInterval, int aTimeout);

	/// Запуск/останов поллинга.
	virtual void setPollingActive(bool aActive);

	/// Переинициализация в рамках фоновой логики пост-поллинга.
	virtual void reInitialize();

	/// Останавливает функционал поллинга, возвращается в состояние до вызова initialize().
	void releasePolling();

	/// Таймер для поллинга.
	QTimer mPolling;

	/// Интервал поллинга, [мс].
	int mPollingInterval;

	/// Поллинг активирован. QTimer::isActive() не всегда работает корректно.
	bool mPollingActive;
};

//--------------------------------------------------------------------------------
