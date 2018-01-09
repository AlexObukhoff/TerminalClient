/* @file Купюроприемник на протоколе ID003. */

#pragma once

// Modules
#include "Hardware/Protocols/CashAcceptor/SSP.h"

// Project
#include "Hardware/CashAcceptors/PortCashAcceptor.h"

//--------------------------------------------------------------------------------
class SSPCashAcceptor : public TSerialCashAcceptor
{
	SET_SERIES("SSP")

public:
	SSPCashAcceptor();

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

	/// Применить таблицу номиналов.
	virtual bool applyParTable();

	/// Изменение режима приема денег.
	virtual bool enableMoneyAcceptingMode(bool aEnabled);

	/// Загрузка таблицы номиналов из устройства.
	virtual bool loadParTable();

	/// Получить статусы.
	virtual bool checkStatuses(TStatusData & aData);

	/// Локальный сброс.
	virtual bool processReset();

	/// Выполнить команду.
	virtual TResult execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr);

	/// Протокол.
	SSPProtocol mProtocol;
};

//--------------------------------------------------------------------------------
