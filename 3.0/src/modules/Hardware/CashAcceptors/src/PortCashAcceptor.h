/* @file Базовый класс устройств приема денег на порту. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSharedPointer>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/DeviceCodeSpecification.h"

// Project
#include "CashAcceptorBase.h"

/// Ожидание после резета
namespace EResetWaiting
{
	enum Enum
	{
		No = 0,
		Available,
		Full
	};
}

//--------------------------------------------------------------------------------
namespace CPortCashAcceptor
{
	/// Ожидание завершения выполнения функционала для фильтрации/отправки накопленных статусов.
	const SWaitingData RestoreStatusesWaiting = SWaitingData(1, 10 * 1000);
}

//--------------------------------------------------------------------------------
template<class T>
class PortCashAcceptor : public CashAcceptorBase<T>
{
public:
	PortCashAcceptor();

	/// Активировать/деактивировать приём с учетом отложенного выполнения.
	virtual bool setEnable(bool aEnabled);

protected:
	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Запросить и сохранить параметры устройства.
	virtual void processDeviceData() {}

	/// Получить и обработать статус.
	virtual bool processStatus(TStatusCodes & aStatusCodes);

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Получить статус.
	virtual bool checkStatus(QByteArray & /*aAnswer*/) { return false; }

	/// Получить статусы.
	typedef QList<QByteArray> TStatusData;
	virtual bool checkStatuses(TStatusData & aData);

	/// Фоновая логика при появлении определенных состояний устройства.
	virtual void postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection);

	/// Извещает верхнюю логику о завершении отключения устройства приема денег.
	virtual void onSendDisabled();

	/// Восстановление статусов для отправки наверх после отключения поллинга.
	virtual void restoreStatuses();

	/// Выполняет физические действия по включению/выключению устрйоства.
	virtual void processEnable(bool aEnabled);

	/// Установка параметров по умолчанию.
	virtual bool setDefaultParameters() { return true; }

	/// Ждет определенного состояния купюрника.
	bool processAndWait(const TBoolMethod & aCommand, TBoolMethod aCondition, int aTimeout, bool aNeedReset = true, bool aRestartPolling = true, TBoolMethod aErrorCondition = TBoolMethod());

	/// Сброс.
	bool reset(bool aWait);

	/// Локальный сброс.
	virtual bool processReset() { return false; }

	/// Завершен ли сброс.
	bool isResetCompleted(bool aWait);

	/// Применить таблицу номиналов.
	virtual bool applyParTable() { return true; }

	/// Проверка возможности применения буфера статусов.
	virtual bool canApplyStatusBuffer();

	/// Завершение инициализации.
	virtual void finaliseInitialization();

	/// Изменение режима приема денег.
	virtual bool enableMoneyAcceptingMode(bool /*aEnabled*/) { return false; }

	/// Повтор изменения режима приема денег.
	bool reenableMoneyAcceptingMode();

	/// Извещает верхнюю логику о завершении включения устройства приема денег.
	virtual void sendEnabled();

	/// Установить начальные параметры.
	virtual void setInitialData();

	/// Установить эскроу-данные.
	virtual bool setLastPar(const QByteArray & aAnswer);

	/// Получить интервал поллинга в режиме приема денег.
	int getPollingInterval(bool aEnabled);

	/// Признак контроля отключения купюрника.
	bool mCheckDisable;

	/// Устройству необходим резет на идентификации.
	bool mResetOnIdentification;

	/// Номер эскроу-байта в распакованной посылке.
	int mEscrowPosition;

	/// Описание для КОДОВ протоколов.
	typedef QSharedPointer<IDeviceCodeSpecification> PDeviceCodeSpecification;
	PDeviceCodeSpecification mDeviceCodeSpecification;

	/// Последние девайс-коды устройства.
	typedef QSet<QByteArray> TDeviceCodeBuffers;
	TDeviceCodeBuffers mDeviceCodeBuffers;

	/// Приходит ли в событии Stacked информация о купюре.
	bool mParInStacked;

	/// Интервал опроса (время между посылками запроса статуса), девайс выключен на прием денег.
	int mPollingIntervalDisabled;

	/// Интервал опроса, девайс включен на прием денег.
	int mPollingIntervalEnabled;

	/// Ждать доступности после резета.
	EResetWaiting::Enum mResetWaiting;

	/// Возможна ли перепрошивка.
	bool mUpdatable;
};

typedef PortCashAcceptor<SerialDeviceBase<PortPollingDeviceBase<ProtoCashAcceptor>>> TSerialCashAcceptor;

//--------------------------------------------------------------------------------
