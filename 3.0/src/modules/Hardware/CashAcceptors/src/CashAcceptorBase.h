/* @file Мета-устройство приема денег. */

#pragma once

// SDK
#include <SDK/Drivers/CashAcceptor/CashAcceptorStatus.h>

// Modules
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/SerialDeviceBase.h"
#include "Hardware/CashAcceptors/ProtoCashAcceptor.h"
#include "Hardware/CashAcceptors/BillTable.h"
#include "Hardware/CashAcceptors/CashAcceptorBaseConstants.h"

// Project
#include "CurrencyErrors.h"
#include "CashAcceptorStatusData.h"

//--------------------------------------------------------------------------------
namespace CCashAcceptor
{
	typedef StatusCache<SDK::Driver::ECashAcceptorStatus::Enum> TStatuses;

	/// Качественный описатель последнего состояния.
	struct SStatusSpecification
	{
		/// Уровень тревожности статуса.
		SDK::Driver::EWarningLevel::Enum warningLevel;

		/// Буфер статусов, сопряженных с статус-кодами.
		TStatuses statuses;

		SStatusSpecification(): warningLevel(SDK::Driver::EWarningLevel::OK) {}

		bool operator==(const SStatusSpecification & aStatusSpecification) const
		{
			return (statuses == aStatusSpecification.statuses);
		}

		bool operator!=(const SStatusSpecification & aStatusSpecification) const
		{
			return !operator==(aStatusSpecification);
		}
	};
}

//--------------------------------------------------------------------------------
typedef QList<int> TStatusCodesHistory;
typedef QList<TStatusCodesHistory> TStatusCodesHistoryList;
typedef QList<SDK::Driver::SPar> TPars;

//--------------------------------------------------------------------------------
template<class T>
class CashAcceptorBase : public T
{
public:
	CashAcceptorBase();

	/// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
	virtual bool release();

	/// Готов ли к работе (инициализировался успешно, ошибок нет).
	virtual bool isDeviceReady();

	/// Установить новую информацию для таблицы номиналов.
	virtual void setParList(const SDK::Driver::TParList & aParList);

protected:
	/// Устанавливает запрещения списка номиналов.
	virtual void employParList();

	/// Получение и обработка списка номиналов.
	ECurrencyError::Enum processParTable();

	/// Загрузка таблицы номиналов из устройства.
	virtual bool loadParTable() { return false; }

	/// Анализирует коды статусов устройства и фильтрует несуществующие статусы для нижней логики.
	virtual void cleanStatusCodes(TStatusCodes & aStatusCodes);

	/// Анализирует коды статусов кастомных устройств и фильтрует несуществующие статусы для нижней логики.
	virtual void cleanSpecificStatusCodes(TStatusCodes & /*aStatusCodes*/) {}

	/// Анализирует коды статусов устройства и фильтрует несуществующие статусы для нижней логики на основе истории статусов.
	void cleanStatusCodes(TStatusCodes & aStatusCodes, const TStatusCodesHistoryList & aBaseHistoryList, TStatusCodesHistory aReplaceableHistory, int aReplacingStatusCode);

	/// Отправка статусов.
	virtual void sendStatuses(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection);

	/// Сохранение последних статусов.
	void saveStatuses(const CCashAcceptor::TStatuses & aStatuses, SDK::Driver::ECashAcceptorStatus::Enum aTargetStatus, const CCashAcceptor::TStatusSet aSourceStatuses = CCashAcceptor::TStatusSet());

	/// Получение последних статусов.
	CCashAcceptor::TStatuses getLastStatuses(int aLevel = 1) const;

	/// Получение признака возможности отключения купюрника.
	bool canDisable() const;

	/// Включен на прием купюр?
	bool isEnabled(const CCashAcceptor::TStatuses & aStatuses = CCashAcceptor::TStatuses()) const;

	/// Не включен на прием купюр?
	bool isNotEnabled() const;

	/// Отключен на прием купюр?
	bool isDisabled() const;

	/// Не отключен на прием купюр?
	bool isNotDisabled() const;

	/// Инициализируется?
	bool isInitialize() const;

	/// Доступен?
	bool isAvailable();

	/// Получение признака возможности возврата купюры.
	bool canReturning(bool aOnline);

	/// Вывод в лог таблицы номиналов.
	virtual void logEnabledPars();

	/// Отправка статусов.
	void emitStatuses(CCashAcceptor::SStatusSpecification & aSpecification, const CCashAcceptor::TStatusSet & aSet);

	/// Список номиналов.
	SDK::Driver::TParList mParList;

	/// История актуальных отправленных статусов.
	typedef HystoryList<CCashAcceptor::SStatusSpecification> TStatusHistory;
	TStatusHistory mStatusHistory;

	/// Кэш последних статусов для бизнес-логики.
	CCashAcceptor::TStatuses mStatuses;

	/// Логические ошибки работы с валютой.
	ECurrencyError::Enum mCurrencyError;

	/// Купюры/монеты в эскроу/стекеде.
	typedef QList<SDK::Driver::SPar> TPars;
	TPars mEscrowPars;

	/// Тип устройства приема денег.
	QString mDeviceType;

	/// Мьютекс для блокировки запросов к списку номиналов.
	QMutex mParListMutex;

	/// Список включенных номиналов для реализаций без протоколов.
	CBillTable mEscrowParTable;

	/// Готов ли к работе (инициализировался успешно, ошибок нет).
	bool mReady;
};

//--------------------------------------------------------------------------------
