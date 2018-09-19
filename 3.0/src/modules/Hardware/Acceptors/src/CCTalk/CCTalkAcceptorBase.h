/* @file Базовый класс устройства приема денег на протоколе ccTalk. */

#pragma once

// Modules
#include "Hardware/Protocols/CashAcceptor/CCTalk.h"
#include "Hardware/CashAcceptors/PortCashAcceptor.h"
#include "Hardware/CoinAcceptors/CoinAcceptorBase.h"

// Project
#include "CCTalkModelData.h"
#include "CCTalkCurrencyData.h"
#include "CCTalkAcceptorConstants.h"

//--------------------------------------------------------------------------------
template <class T>
class CCTalkAcceptorBase : public T
{
	SET_SERIES("ccTalk")

public:
	CCTalkAcceptorBase();

protected:
	/// Запросить и сохранить параметры устройства.
	virtual void processDeviceData();

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Получить буферизованные статусы.
	virtual bool getBufferedStatuses(QByteArray & /*aAnswer*/) { return false; }

	/// Распарсить буферизованные статусы.
	virtual void parseBufferedStatuses(const QByteArray & aAnswer, TStatusCodes & aStatusCodes);

	/// Распарсить данные о купюре.
	virtual void parseCreditData(uchar /*aCredit*/, uchar /*aError*/, TStatusCodes & /*aStatusCodes*/) {}

	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Попытка самоидентификации.
	bool checkConnection();

	/// Распарсить данные прошивки.
	virtual double parseFWVersion(const QByteArray & aAnswer);

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

	/// Получить данные валюты.
	virtual bool parseCurrencyData(const QByteArray & aData, CCCTalk::SCurrencyData & aCurrencyData);

	/// Можно ли применить простые статус-код (включен/отключен).
	virtual bool canApplySimpleStatusCodes(const TStatusCodes & aStatusCodes);

	/// Получить данные моделей.
	virtual CCCTalk::CModelDataBase * getModelData() { return nullptr; }

	/// Базовый год (для парсинга дат).
	int mBaseYear;

	/// Протокол.
	CCTalkCAProtocol mProtocol;

	/// Индекс события.
	int mEventIndex;

	// TODO: в базу.
	/// Последние девайс-коды устройства.
	TDeviceCodes mCodes;

	/// Признак включенности на прием денег.
	bool mEnabled;

	/// Номер прошивки.
	double mFWVersion;

	/// Модели данной реализации.
	QStringList mModels;

	/// Данные модели.
	CCCTalk::SModelData mModelData;

	/// Валюта. Нужна для установки предупреждения о старой прошивке.
	int mCurrency;

	/// Адрес устройства.
	uchar mAddress;

	/// Данные ошибок.
	CCCTalk::ErrorDataBase * mErrorData;
};

//--------------------------------------------------------------------------------
