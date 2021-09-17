/* @file Купюроприемник на протоколе V2e. */

#pragma once

// Modules
#include "Hardware/Protocols/CashAcceptor/V2e.h"

// Project
#include "Hardware/CashAcceptors/SerialCashAcceptor.h"

//--------------------------------------------------------------------------------
class V2eCashAcceptor : public SerialCashAcceptor
{
	SET_SERIES("V2e")

public:
	V2eCashAcceptor();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

	/// Принять купюру.
	virtual bool stack();

	/// Вернуть купюру.
	virtual bool reject();

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Установка параметров по умолчанию.
	virtual bool setDefaultParameters();

	/// Анализирует коды статусов кастомных устройств и фильтрует несуществующие статусы для нижней логики.
	virtual void cleanSpecificStatusCodes(TStatusCodes & aStatusCodes);

	/// Изменение режима приема денег.
	virtual bool enableMoneyAcceptingMode(bool aEnabled);

	/// Загрузка таблицы номиналов из устройства.
	virtual bool loadParTable();

	/// Получить статус.
	virtual bool checkStatus(QByteArray & aAnswer);

	/// Локальный сброс.
	virtual bool processReset();

	/// Выполнить команду.
	virtual TResult execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr);

	/// Подождать смены BUSY-состояния.
	bool waitForBusy(bool aBusy);

	/// Протокол.
	V2eProtocol mProtocol;
};

//--------------------------------------------------------------------------------
