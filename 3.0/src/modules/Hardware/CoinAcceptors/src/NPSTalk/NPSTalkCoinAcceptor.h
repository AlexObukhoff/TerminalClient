/* @file Монетоприемник на протоколе NPSTalk. */

#pragma once

// Modules
#include "Hardware/Protocols/CashAcceptor/NPSTalk.h"

// Project
#include "Hardware/CoinAcceptors/CoinAcceptorBase.h"

//--------------------------------------------------------------------------------
class NPSTalkCoinAcceptor : public CoinAcceptorBase
{
	SET_SERIES("NPS")

	typedef QMap<uchar, uchar> TCoinsByChannel;

public:
	NPSTalkCoinAcceptor();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

	/// Получение статуса.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Локальный сброс.
	virtual bool processReset();

	/// Установка параметров по умолчанию.
	virtual bool setDefaultParameters();

	/// Загрузка таблицы номиналов из устройства.
	virtual bool loadParTable();

	/// Изменение режима приема денег.
	virtual bool enableMoneyAcceptingMode(bool aEnabled);

	/// Выполнить команду.
	virtual TResult execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr);

	/// Протокол.
	NPSTalkProtocol mProtocol;

	// TODO: в базу.
	/// Последние статус-коды устройства.
	TDeviceCodes mCodes;

	/// Количество монет на канал.
	TCoinsByChannel mCoinsByChannel;
};

//--------------------------------------------------------------------------------
