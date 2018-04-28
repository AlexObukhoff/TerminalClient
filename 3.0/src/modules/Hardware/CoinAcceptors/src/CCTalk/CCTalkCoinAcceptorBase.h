/* @file Базовый класс монетоприемников на протоколе ccTalk. */

#pragma once

// Modules
#include "Hardware/Protocols/CashAcceptor/CCTalk.h"

// Project
#include "Hardware/CoinAcceptors/CoinAcceptorBase.h"
#include "CCTalkModelData.h"

//--------------------------------------------------------------------------------
class CCTalkCoinAcceptorBase : public CoinAcceptorBase
{
	SET_SERIES("ccTalk")

public:
	CCTalkCoinAcceptorBase();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

protected:
	/// Получение статуса.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Локальный сброс.
	virtual bool processReset();

	/// Загрузка таблицы номиналов из устройства.
	virtual bool loadParTable();

	/// Применить таблицу номиналов.
	virtual bool applyParTable();

	/// Изменение режима приема денег.
	virtual bool enableMoneyAcceptingMode(bool aEnabled);

	/// Выполнить команду.
	virtual TResult execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr);

	/// Распарсить дату.
	QDate parseDate(const QByteArray & aData);

	/// Получить данные для применения таблицы номиналов.
	QByteArray getParTableData();

	/// Базовый год (для парсинга дат).
	int mBaseYear;

	/// Протокол.
	CCTalkCAProtocol mProtocol;

	/// Индекс события.
	int mEventIndex;

	// TODO: в базу.
	/// Последние статус-коды устройства.
	TDeviceCodes mCodes;

	/// Признак включенности на прием денег.
	bool mEnabled;

	/// Номер прошивки.
	double mFWVersion;

	/// Модели данной реализации.
	QStringList mModels;

	/// Данные модели.
	CCCTalk::SModelData mModelData;

	/// Валюта.
	int mCurrency;
};

//--------------------------------------------------------------------------------
