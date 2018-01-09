/* @file Монетоприемник на протоколе NPSTalk. */

#pragma once

#include "Hardware/CashAcceptors/PortCashAcceptor.h"
#include "Hardware/Protocols/CashAcceptor/NPSTalk.h"

//--------------------------------------------------------------------------------
class NPSTalkCoinAcceptor : public TSerialCashAcceptor
{

	typedef QMap<uchar, uchar> TCoinsByChannel;

public:
	NPSTalkCoinAcceptor();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

	/// Получение статуса.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Принять купюру.
	virtual bool stack();

	/// Вернуть купюру. Правильный термин - return (ключевое слово).
	virtual bool reject();

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
