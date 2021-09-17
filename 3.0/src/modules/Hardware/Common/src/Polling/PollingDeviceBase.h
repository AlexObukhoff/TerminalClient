/* @file Базовый класс устройств с поллингом. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTimer>
#include <Common/QtHeadersEnd.h>

// Project
#include "Hardware/Common/DeviceBase.h"
#include "Hardware/Common/ExternalPortDeviceBase.h"

//---------------------------------------------------------------------------
namespace CPollingDeviceBase
{
	/// Ожидание останова поллинга, [мс].
	const SWaitingData StopWaiting = SWaitingData(1, 15 * 1000);
}

/// Список задач.
typedef QList<TVoidMethod> TTaskList;

//---------------------------------------------------------------------------
template <class T>
class PollingDeviceBase : public T
{
public:
	PollingDeviceBase();

	/// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
	virtual bool release();

protected:
	/// Завершение инициализации.
	virtual void finaliseInitialization();

	/// Фоновая логика при появлении определенных состояний устройства.
	virtual void postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection);

	/// Есть ли ошибка инициализации при фильтрации статусов.
	bool isInitializationError(TStatusCodes & aStatusCodes);

	/// Запуск поллинга.
	void startPolling(bool aNotWaitFirst = false);

	/// Останов поллинга.
	void stopPolling(bool aWait = true);

	/// Установить таймаут.
	void setPollingInterval(int aPollingInterval);

	/// Ожидание состояния или выполнения полла.
	bool waitCondition(TBoolMethod aCondition, const SWaitingData & aWaitingData);

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

	/// Список задач для выполнения после поллинга, если нет ошибкок.
	TTaskList mPPTaskList;

	/// Принудительно не ждать первого полла на иницаилизации.
	bool mForceNotWaitFirst;
};

//--------------------------------------------------------------------------------
