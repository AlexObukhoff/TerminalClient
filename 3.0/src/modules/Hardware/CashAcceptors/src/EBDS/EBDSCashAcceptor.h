/* @file Купюроприемник на протоколе EBDS. */

#pragma once

// Modules
#include "Hardware/Protocols/CashAcceptor/EBDS.h"

// Project
#include "Hardware/CashAcceptors/SerialCashAcceptor.h"

/*
софт MEI структурно состоит из 3 программ:

1. в интерфейсной плате находится Boot loader (загрузчик), он умеет общаться с программатором и
загружать прошивку себя, головы и биллсета.
2. В голове находится 2 программы - Application programm (распознает купюры) и BillSet (по европейски, по американски - Variant) - образы купюр,
уникальные для каждого набора купюр. Может быть загружено сразу несколько билл-сетов
*/

//--------------------------------------------------------------------------------
class EBDSCashAcceptor : public SerialCashAcceptor
{
	SET_SERIES("EBDS")

public:
	EBDSCashAcceptor();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

	/// Принять купюру.
	virtual bool stack();

	/// Вернуть купюру.
	virtual bool reject();

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Запросить и сохранить параметры устройства.
	virtual void processDeviceData();

	/// Анализирует коды статусов кастомных устройств и фильтрует несуществующие статусы для нижней логики.
	virtual void cleanSpecificStatusCodes(TStatusCodes & aStatusCodes);

	/// Применить таблицу номиналов.
	virtual bool applyParTable();

	/// Изменение режима приема денег.
	virtual bool enableMoneyAcceptingMode(bool aEnabled);

	/// Загрузка таблицы номиналов из устройства.
	virtual bool loadParTable();

	/// Установить эскроу-данные.
	virtual bool setLastPar(const QByteArray & aAnswer);

	/// Получить статус.
	virtual bool checkStatus(QByteArray & aAnswer);

	/// Локальный сброс.
	virtual bool processReset();

	/// Выполнить команду.
	virtual TResult execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr);

	/// Получить статус.
	TResult poll(char aAction, QByteArray * aAnswer = nullptr);

	/// Получить информацию о номинале из буфера.
	SDK::Driver::SPar getPar(const QByteArray & aData);

	/// Протокол.
	EBDSProtocol mProtocol;

	/// Признак почти-заполненности стекера.
	bool mStackerNearFull;

	/// Признак включенности на прием купюр.
	bool mEnabled;
};

//--------------------------------------------------------------------------------
