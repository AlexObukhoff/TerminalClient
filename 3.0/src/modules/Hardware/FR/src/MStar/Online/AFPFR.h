/* @file Онлайн ФР семейства MStar на протоколе AFP. */

#pragma once

// Modules
#include "Hardware/Protocols/FR/AFPFR.h"

// Project
#include "Hardware/FR/PortFRBase.h"
#include "AFPFRConstants.h"
#include "AFPFRDataTypes.h"

//--------------------------------------------------------------------------------
class AFPFR : public TSerialFRBase
{
	SET_SERIES("AFP")

public:
	AFPFR();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Запросить и вывести в лог критичные параметры ФР.
	virtual void processDeviceData();

	/// Загрузить названия отделов.
	bool loadSectionNames();

	/// Получить дату и время ФР.
	virtual QDateTime getDateTime();

	/// Получить параметр системной таблицы.
	bool getFRParameter(const CAFPFR::FRParameters::SData & aData, QVariant & aValue);

	/// Установить параметр системной таблицы.
	bool setFRParameter(const CAFPFR::FRParameters::SData & aData, QVariant aValue);

	/// Выполнить команду.
	TResult processCommand(char aCommand, CAFPFR::TData * aAnswer = nullptr);
	TResult processCommand(char aCommand, const CAFPFR::TData & aCommandData, CAFPFR::TData * aAnswer = nullptr);
	TResult processCommand(char aCommand, const CAFPFR::TData & aCommandData, CAFPFR::TData * aAnswer, CAFPFR::EAnswerTypes::Enum aAnswerType);
	TResult processCommand(char aCommand, const CAFPFR::TData & aCommandData, CAFPFR::TData * aAnswer, const CAFPFR::TAnswerTypes & aAnswerTypes);

	TResult processCommand(char aCommand, const QVariant & aCommandData, CAFPFR::TData * aAnswer = nullptr);
	TResult processCommand(char aCommand, const QVariant & aCommandData, CAFPFR::TData * aAnswer, CAFPFR::EAnswerTypes::Enum aAnswerType);
	TResult processCommand(char aCommand, const QVariant & aCommandData, CAFPFR::TData * aAnswer, const CAFPFR::TAnswerTypes & aAnswerTypes);

	/// Выполнить команду.
	EResult::Enum performCommand(TStatusCodes & aStatusCodes, char aCommand, CAFPFR::TData & aAnswerData);

	/// Обработка ответа на предыдущей команды. Автоисправление некоторых ошибок.
	virtual bool processAnswer(char aCommand, char aError);

	/// Печать фискального чека.
	virtual bool performFiscal(const QStringList & aReceipt, const SDK::Driver::SPaymentData & aPaymentData, quint32 * aFDNumber = nullptr);

	/// Собрать данные о сессии перед выполнением Z-отчета.
	QVariantMap getSessionOutData();

	/// Напечатать [и выдать] чек.
	virtual bool processReceipt(const QStringList & aReceipt, bool aProcessing = true);

	/// Напечатать строку.
	virtual bool printLine(const QByteArray & aString);

	/// Отрезка.
	virtual bool cut();

	/// Открыть документ.
	virtual bool openDocument(char aType);

	/// Продажа.
	virtual bool sale(const SDK::Driver::SUnitData & aUnitData);

	/// Закрыть документ.
	virtual bool closeDocument(bool aProcessing = true);

	/// Получить состояние смены.
	virtual SDK::Driver::ESessionState::Enum getSessionState();

	/// Получить состояние документа.
	virtual SDK::Driver::EDocumentState::Enum getDocumentState();

	/// Открыть смену.
	virtual bool openSession();

	/// Локальная печать X-отчета.
	virtual bool processXReport();

	/// Печать Z отчета.
	virtual bool performZReport(bool aPrintDeferredReports);

	/// Снять Z-отчет.
	virtual bool execZReport(bool aAuto);

	/// Получить сумму в кассе.
	virtual double getAmountInCash();

	/// Печать выплаты.
	virtual bool processPayout(double aAmount);

	/// Установить флаги по ошибке в ответе.
	void setErrorFlags();

	/// Получить данные об ФР.
	bool getFRData(const CAFPFR::FRInfo::SData & aInfo, CAFPFR::TData & aData);

	/// Включить/выключить режим непечати документов.
	virtual bool setNotPrintDocument(bool aEnabled, bool aZReport = false);

	/// Данные модели.
	CAFPFR::Models::SData mModelData;

	/// Протокол.
	AFPFRProtocol mProtocol;
};

//--------------------------------------------------------------------------------
