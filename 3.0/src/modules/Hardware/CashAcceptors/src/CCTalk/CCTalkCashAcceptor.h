/* @file Купюроприемник на протоколе ccTalk. */

#pragma once

// Modules
#include "Hardware/Acceptors/CCTalkAcceptorBase.h"

// Project
#include "CCTalkCashAcceptorModelData.h"

//--------------------------------------------------------------------------------
typedef CCTalkAcceptorBase<TSerialCashAcceptor> TCCTalkCashAcceptor;

class CCTalkCashAcceptor : public TCCTalkCashAcceptor
{
public:
	CCTalkCashAcceptor();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

	/// Принять купюру.
	virtual bool stack();

	/// Вернуть купюру.
	virtual bool reject();

protected:
	/// Запросить и сохранить параметры устройства.
	virtual void processDeviceData();

	/// Получить буферизованные статусы.
	virtual bool getBufferedStatuses(QByteArray & aAnswer);

	/// Распарсить данные о купюре.
	virtual void parseCreditData(uchar aCredit, uchar aError, TStatusCodes & aStatusCodes);

	/// Локальный сброс.
	virtual bool processReset();

	/// Установка параметров по умолчанию.
	virtual bool setDefaultParameters();

	/// Анализирует коды статусов кастомных устройств и фильтрует несуществующие статусы для нижней логики.
	virtual void cleanSpecificStatusCodes(TStatusCodes & aStatusCodes);

	/// Распарсить данные прошивки.
	virtual double parseFWVersion(const QByteArray & aAnswer);

	/// Загрузка таблицы номиналов из устройства.
	virtual bool loadParTable();

	/// Получить данные валюты.
	virtual bool parseCurrencyData(const QByteArray & aData, CCCTalk::SCurrencyData & aCurrencyData);

	/// Вернуть/уложить купюру.
	bool route(bool aDirection);

	/// Коэффициенты для вычисления номиналов. Зависят от кода валюты.
	QMap<QByteArray, double> mScalingFactors;

	/// Получить данные моделей.
	virtual CCCTalk::CModelDataBase * getModelData();

	/// Виртуальный статус движения купюры после эскроу.
	struct SRouting
	{
		bool direction;
		bool active;

		SRouting(): direction(false), active(false) {}
	};

	SRouting mVirtualRouting;
};

//--------------------------------------------------------------------------------
