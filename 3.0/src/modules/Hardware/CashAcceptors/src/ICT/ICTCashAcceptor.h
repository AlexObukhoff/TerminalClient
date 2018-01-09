/* @file Купюроприемник на протоколе ICT. */

#pragma once

#include "Hardware/CashAcceptors/PortCashAcceptor.h"

//--------------------------------------------------------------------------------
class ICTCashAcceptor : public TSerialCashAcceptor
{
	SET_SERIES("ICT")

public:
	ICTCashAcceptor();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

	/// Принять купюру.
	virtual bool stack();

	/// Вернуть купюру.
	virtual bool reject();

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Фоновая логика при появлении определенных состояний устройства.
	virtual void postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection);

	/// Проверка возможности применения буфера статусов.
	virtual bool isStatusesReplaceable(TStatusCodes & aStatusCodes);

	/// Изменение режима приема денег.
	virtual bool enableMoneyAcceptingMode(bool aEnabled);

	/// Загрузка таблицы номиналов из устройства.
	virtual bool loadParTable();

	/// Получить статус.
	virtual bool checkStatus(QByteArray & aAnswer);

	/// Ответить на команду Reset.
	bool answerToReset();

	/// Локальный сброс.
	virtual bool processReset();
};

//--------------------------------------------------------------------------------
