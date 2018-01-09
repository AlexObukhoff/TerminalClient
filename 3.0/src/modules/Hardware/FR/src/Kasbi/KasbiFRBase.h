/* @file ФР АТОЛ и Пэй Киоск. */

#pragma once

// Modules
#include "Hardware/FR/PortFRBase.h"
#include "Hardware/Protocols/FR/KasbiFR.h"

// Project
#include "KasbiFRConstants.h"

class KasbiSeriesType {};

//--------------------------------------------------------------------------------
class KasbiFRBase : public TSerialFRBase
{
	SET_SERIES("KasbiOnline")

public:
	KasbiFRBase();

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Получить статусы.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Получить дату и время ФР.
	virtual QDateTime getDateTime();

	/// Выполнить команду.
	virtual TResult execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr);

	/// Обработка ответа на предыдущей команды. Автоисправление некоторых ошибок.
	bool processAnswer(char aCommand, char aError);

	/// Напечатать строку.
	virtual bool printLine(const QByteArray & aString);

	/// Отрезка.
	virtual bool cut();

	/// Печать фискального чека.
	virtual bool performFiscal(const QStringList & aReceipt, const SDK::Driver::SPaymentData & aPaymentData, SDK::Driver::TFiscalPaymentData & aFPData, SDK::Driver::TComplexFiscalPaymentData & aPSData);

	/// Печать Z отчета.
	virtual bool performZReport(bool aPrintDeferredReports);

	/// Локальная печать X-отчета.
	virtual bool processXReport();

	/// Запросить и вывести в лог критичные параметры ФР.
	void processDeviceData(const QByteArray & aRegistrationData);

	/// Открыта ли смена. Если будет ошибка - по умолчанию открыта.
	bool isSessionOpened();

	/// Открыт ли документ. Если будет ошибка - по умолчанию закрыт.
	bool isDocumentOpened();

	/// Получить данные ФН.
	bool getFSData(CKasbiFR::SFSData & aData);

	/// Выполнить Z-отчет.
	bool execZReport(bool aAuto);

	/// Открыть смену.
	bool openSession();

	/// Продажа.
	bool sale(const SDK::Driver::SAmountData & aAmountData);

	/// Проверить настройки печати.
	bool checkPrintingParameters();

	/// Протокол.
	KasbiFRProtocol mProtocol;
};

//--------------------------------------------------------------------------------
