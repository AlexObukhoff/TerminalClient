/* @file Купюроприемник на протоколе ID003. */

#pragma once

// Modules
#include "Hardware/Protocols/CashAcceptor/SSP.h"

// Project
#include "Hardware/CashAcceptors/SerialCashAcceptor.h"
#include "SSPModelDataTypes.h"

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

	/// Запросить и сохранить параметры устройства.
	virtual void processDeviceData();

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

	/// Обновить прошивку.
	virtual bool performUpdateFirmware(const QByteArray & aBuffer);

	/// Изменить скорость работы.
	bool performBaudRateChanging(bool aUp);

	/// Записать сегмент RAM.
	bool writeRAMData(const QByteArray & aData);

	/// Записать сегмент RAM.
	bool writeDatasetData(const QByteArray & aData, int aBlockSize);

	/// Получить CRC блока данных.
	char getCRC(const QByteArray & aData);

	/// Протокол.
	SSPProtocol mProtocol;

	/// Удачна ли была последняя транзакция связи с устройством.
	bool mLastConnectionOK;

	/// Признак включенности на прием денег.
	bool mEnabled;

	/// Номер прошивки.
	double mFirmware;

	/// Данные модели.
	CSSP::Models::SData mModelData;
};

//--------------------------------------------------------------------------------
