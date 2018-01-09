/* @file Купюроприемник на протоколе ID003. */

#pragma once

// Modules
#include "Hardware/Protocols/CashAcceptor/ID003.h"

// Project
#include "Hardware/CashAcceptors/PortCashAcceptor.h"

//--------------------------------------------------------------------------------
class ID003CashAcceptor : public TSerialCashAcceptor
{
	SET_SERIES("ID003")

public:
	ID003CashAcceptor();

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

	/// Получить статус.
	virtual bool checkStatus(QByteArray & aAnswer);

	/// Локальный сброс.
	virtual bool processReset();

	/// Выполнить команду.
	virtual TResult execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr);

	/// Протокол.
	ID003Protocol mProtocol;
};

//--------------------------------------------------------------------------------
