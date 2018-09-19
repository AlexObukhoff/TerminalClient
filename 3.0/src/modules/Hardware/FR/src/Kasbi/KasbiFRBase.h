/* @file ФР АТОЛ и Пэй Киоск. */

#pragma once

// Modules
#include "Hardware/FR/PortFRBase.h"
#include "Hardware/Protocols/FR/KasbiFR.h"

// Project
#include "KasbiFRConstants.h"
#include "Hardware/FR/KasbiPrinters.h"

class KasbiSeriesType {};

//--------------------------------------------------------------------------------
class KasbiFRBase : public TSerialFRBase
{
	SET_SERIES("KasbiOnline")

public:
	KasbiFRBase();

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Получить статус.
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
	void processDeviceData();

	/// Открыта ли смена. Если будет ошибка - по умолчанию открыта.
	virtual SDK::Driver::ESessionState::Enum getSessionState();

	/// Открыт ли документ. Если будет ошибка - по умолчанию закрыт.
	bool isDocumentOpened();

	/// Получить данные ФН.
	bool getFSData(CKasbiFR::SFSData & aData);

	/// Выполнить Z-отчет.
	virtual bool execZReport(bool aAuto);

	/// Открыть смену.
	virtual bool openSession();

	/// Продажа.
	bool sale(const SDK::Driver::SUnitData & aUnitData);

	/// Проверить настройки печати.
	bool checkPrintingParameters(const CFR::TTLVList & aRequiredTLVs);

	/// Включить/выключить режим непечати документов.
	virtual bool setNotPrintDocument(bool aEnabled, bool aZReport = false);

	/// Протокол.
	KasbiFRProtocol mProtocol;
};

//--------------------------------------------------------------------------------
