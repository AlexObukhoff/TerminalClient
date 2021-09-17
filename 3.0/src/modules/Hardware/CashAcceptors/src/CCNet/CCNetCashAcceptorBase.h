/* @file Базовый купюроприемник на протоколе CCNet. */

#pragma once

// Modules
#include "Hardware/Protocols/CashAcceptor/CCNet.h"

// Project
#include "Hardware/CashAcceptors/SerialCashAcceptor.h"
#include "CCNetModelData.h"
#include "CCNetCashAcceptorDataTypes.h"

//--------------------------------------------------------------------------------
class CCNetCashAcceptorBase : public SerialCashAcceptor
{
	SET_SERIES("CCNet")
	SET_VCOM_DATA(Types::Adapter, ConnectionTypes::Dual, AdapterTags::FTDI)

public:
	CCNetCashAcceptorBase();

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

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Анализирует коды статусов кастомных устройств и фильтрует несуществующие статусы для нижней логики.
	virtual void cleanSpecificStatusCodes(TStatusCodes & aStatusCodes);

	/// Изменение режима приема денег.
	virtual bool enableMoneyAcceptingMode(bool aEnabled);

	/// Загрузка таблицы номиналов из устройства.
	virtual bool loadParTable();

	/// Получить статус.
	virtual bool checkStatus(QByteArray & aAnswer);

	/// Попытка самоидентификации.
	bool checkConnection(QByteArray & aAnswer);

	/// Локальный сброс.
	virtual bool processReset();

	/// Состояние - не Busy или PowerUp, но может быть Initialization.
	bool isNotBusyPowerUp();

	/// Выполнить команду.
	virtual TResult execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr);
	virtual TResult performCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr);

	/// Обновить прошивку.
	virtual bool performUpdateFirmware(const QByteArray & aBuffer);

	/// Отправить блок данных обновления прошивки.
	bool processBlockUpdating(uint aAddress, const QByteArray & aBuffer, int & aRepeat, int & aIndex);

	/// Изменить скорость работы.
	bool changeBaudRate(bool aHigh);

	/// Изменить скорость работы.
	virtual bool performBaudRateChanging(const SDK::Driver::TPortParameters & aPortParameters);

	/// Отправить буфер данных обновления прошивки.
	virtual bool processUpdating(const QByteArray & aBuffer, int aSectionSize);

	/// Получить статус процесса обновления прошивки.
	char getUpdatingStatus();

	/// Ждать выхода из Busy-подобных состояний.
	bool waitNotBusyPowerUp();

	/// Получить имя модели по ответу на запрос идентификации.
	SBaseModelData getModelData(const QByteArray & aAnswer);

	/// Запросить и сохранить параметры устройства.
	virtual void processDeviceData(QByteArray & aAnswer);

	/// Поддерживается ли монетоприемник.
	bool isCoinAcceptorSupported() const;

	/// Протокол.
	CCNetProtocol mProtocol;

	/// Номер прошивки.
	int mFirmware;

	/// Список поддерживаемых плагином моделей.
	QStringList mSupportedModels;

	/// Валюта.
	int mCurrencyCode;

	/// Последний ответ.
	QByteArray mLastAnswer;

	/// Данные команд и ответов.
	CCCNet::Commands::Data mCommandData;

	/// Нужно менять скорость при перепрошивке?
	bool mNeedChangeBaudrate;

	/// Дата и время начала режекта.
	QDateTime mRejectStartDT;
};

//--------------------------------------------------------------------------------
