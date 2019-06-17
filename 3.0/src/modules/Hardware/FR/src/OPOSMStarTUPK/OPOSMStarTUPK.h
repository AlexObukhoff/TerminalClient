/* @file Принтер MStar TUP-K на OPOS-драйвере. */

#pragma once

// OPOS
#import "OPOSFiscalPrinter.tlb" rename("ResetPrinter", "ResetOPOSPrinter")

#include <Common/QtHeadersBegin.h>
#pragma warning(disable: 4100) // warning C4100: 'identifier' : unreferenced formal parameter
#include <OPOS/QtWrappers/FiscalPrinter.h>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/PollingDeviceBase.h"
#include "Hardware/Common/OPOSPollingDeviceBase.h"
#include "Hardware/Printers/PrinterBase.h"
#include "Hardware/FR/FRBase.h"
#include "Hardware/FR/ProtoFR.h"

// Project
#include "OPOSMStarTUPKDataTypes.h"

//--------------------------------------------------------------------------------
typedef OPOSPollingDeviceBase<ProtoFR, OPOS::OPOSFiscalPrinter> TPollingOPOSFR;
typedef FRBase<PrinterBase<TPollingOPOSFR>> TOPOSFR;

class OPOSMStarTUPK : public TOPOSFR
{
	SET_SERIES("Multisoft")

public:
	OPOSMStarTUPK();

	/// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
	virtual bool release();

	/// Возвращает список сконфигурированных OPOS устройств.
	static QStringList getProfileNames();

protected:
	/// Инициализировать ресурсы.
	virtual void initializeResources();

	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Проверка возможности выполнения функционала, предполагающего связь с устройством.
	virtual bool checkConnectionAbility();

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Фоновая логика при появлении определенных состояний устройства.
	virtual void postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection);

	/// Проверить параметры налога.
	virtual bool checkTax(SDK::Driver::TVAT aVAT, CFR::Taxes::SData & aData);

	/// Напечатать [и выдать] чек.
	virtual bool processReceipt(const QStringList & aReceipt, bool aProcessing = true);

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Печать фискального чека.
	virtual bool performFiscal(const QStringList & aReceipt, const SDK::Driver::SPaymentData & aPaymentData, quint32 * aFDNumber = nullptr);

	/// Печать Z отчета.
	virtual bool performZReport(bool aPrintDeferredReports);

	/// Локальная печать X-отчета.
	virtual bool processXReport();

	/// Печать выплаты.
	virtual bool processPayout(double aAmount);

	/// Получить сумму в кассе.
	virtual double getAmountInCash();

	/// Печать X-отчета. Параметром задаётся набор дополнительных строк для печати (например баланс).
	virtual bool performXReport(const QStringList & aReceipt);

	/// Печать выплаты.
	virtual bool performEncashment(const QStringList & aReceipt, double aAmount);

	//TODO: посмотреть, можно ли это сделать закрытием чека - фискального или нефискального или есть просто отрезка.
	/// Обработка чека после печати.
	virtual bool receiptProcessing();

	/// Напечатать строку.
	virtual bool printLine(const QVariant & aLine);

	/// Вызывает int-метод в рабочем потоке, возвращает и обработывает результат.
	virtual SOPOSResult processIntMethod(TIntMethod aMethod, const QString & aFunctionData);

	/// Получить состояние смены.
	virtual SDK::Driver::ESessionState::Enum getSessionState();

	/// Получить состояние документа.
	virtual SDK::Driver::EDocumentState::Enum getDocumentState();

	/// Сделать фискальный чек.
	bool makeFiscal(const SDK::Driver::SPaymentData & aPaymentData);

	/// Отмена открытого фискального/нефискального документа.
	bool abortDocument();

	/// Включить доступность параметра.
	void setEnable(COPOSMStarTUPK::Parameters::Enum aParameter, bool aEnable);

	/// Возвращает свободных мест или отчетов в буфере Z-отчётов.
	bool getZBufferSlots(int & aSlots, bool aFilled);

	/// Z отчет.
	virtual bool execZReport(bool aAuto);

	/// Печать отложенных Z отчетов.
	bool printDeferredZReports();

	/// Фиксирование ошибки в контексте выполнения вызвавшего ее функционала.
	bool fixError(SDK::Driver::EFiscalPrinterCommand::Enum aCommand, TBoolMethod aFunction);

	/// Получить описание ошибки.
	virtual QString getErrorDescription();

	/// Нативный интерфейс драйвера.
	typedef OposFiscalPrinter_CCO::IOPOSFiscalPrinter_1_12 TNativeDriver;
	TNativeDriver * mNativeDriver;

	/// Флаг ошибки NAND-флеш памяти.
	bool mMemoryError;

	/// Последняя операция.
	QList<TBoolMethod> mLastHandledOperations;

	/// Обрабатываемые ошибки.
	QList<COPOSMStarTUPK::SErrorData> mErrors;

	/// Версия прошивки.
	int mFWVersion;
};

/// Перевод денежных сумм из представления с плавающей точкой в формат ФР.
inline CY sum2CY(double arg);

//--------------------------------------------------------------------------------
