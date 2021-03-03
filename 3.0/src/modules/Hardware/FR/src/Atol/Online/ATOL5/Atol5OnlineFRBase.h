/* @file Онлайн ФР на платформе АТОЛ5. */

#pragma once

// Common
#include <Common/PluginConstants.h>

// Modules
#include "Hardware/Common/WorkingThreadProxy.h"

// Project
#include "Hardware/FR/ProtoAtolFR.h"
#include "Wrapper/AtolDriverWrapper.h"
#include "ProtoAtol5FR.h"

//TODO: отвязать наследование от TSerialFRBase-а

#define INT_CALL_ATOL(aMethod, aLog, ...) processIntMethod(std::bind(&Atol::Fptr::Fptr::aMethod, mDriver(), __VA_ARGS__), aLog, #aMethod##"("##""#__VA_ARGS__##")")

//--------------------------------------------------------------------------------
template <class T>
class Atol5OnlineFRBase : public ProtoAtolFR<FRBase<PrinterBase<ProtoAtol5FR<T>>>>
{
	SET_SERIES("ATOL5Online")

public:
	Atol5OnlineFRBase();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

#pragma region SDK::Driver::IDevice interface
	/// Подключает и инициализует устройство.
	virtual void initialize();

	/// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
	virtual bool release();

	/// Переформировывает список параметров для автопоиска и устанавливает 1-й набор параметров из этого списка.
	//virtual SDK::Driver::IDevice::IDetectingIterator * getDetectingIterator();
#pragma endregion

	/// Задаёт лог.
	virtual void setLog(ILog * aLog);

protected:
	/// Инициализировать ресурсы.
	virtual void initializeResources();

	/// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
	virtual bool performRelease();

	/// Проверка возможности выполнения функционала, предполагающего связь с устройством.
	virtual bool checkConnectionAbility();

	/// Отпустить внешний ресурс (порт/драйвер).
	virtual bool releaseExternalResource();

	/// Идентификация.
	virtual bool checkExistence();

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Установить параметры ФР.
	virtual bool setFRParameters();

	/// Получить данные фискализации.
	bool getFiscalizationData(int aField, QString & aValue, const CFR::TFFFormatDataMethod & aMethod);

	/// Запрашивает и выводит в лог критичные параметры ФР.
	virtual void processDeviceData();

	/// Запрашивает версию ПО ФР или драйвера.
	void getVersionData(libfptr_unit_type aType, const QString & aKey);

	/// Получить параметры печати.
	virtual bool getPrintingSettings();

	/// Проверить параметры налогов.
	virtual bool checkTaxes();

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Получить статус режима (режим/подрежим).
	bool getModeStatus();

	/// Полл.
	virtual void doPoll(TStatusCodes & aStatusCodes);

	/// Вызывает int-метод в рабочем потоке, возвращает и обработывает результат.
	TResult processIntMethod(TIntMethod aMethod, const QString & aLog, const QString & aMethodData);

	/// Получить дату и время ФР.
	virtual QDateTime getDateTime();

	/// Получить ключ модели для идентификации.
	virtual bool processModelKey(CAtolFR::TModelKey & aModeKey);

	/// Напечатать [и выдать] чек.
	virtual bool processReceipt(const QStringList & aReceipt, bool aProcessing = true);

	/// Напечатать чек.
	virtual bool printReceipt(const Tags::TLexemeReceipt & aLexemeReceipt);

	/// Напечатать строку.
	virtual bool printLine(const QVariant & aLine);

	/// Обработка чека после печати.
	virtual bool receiptProcessing();

	/// Аварийно завершить фискальный(?) документ.
	bool cancelDocument();

	/// Получить состояние смены.
	virtual SDK::Driver::ESessionState::Enum getSessionState();

	/// Включить/выключить режим непечати документов.
	virtual bool setNotPrintDocument(bool aEnabled, bool aZReport = false);

	/// Выполнить вход оператором.
	bool operatorLogin();

	/// Сформировать составной фискальный документ.
	virtual bool complexFiscalDocument(TBoolMethod aMethod, const QString & aLog);

	/// Печать фискального чека.
	virtual bool performFiscal(const QStringList & aReceipt, const SDK::Driver::SPaymentData & aPaymentData, quint32 * aFDNumber = nullptr);

	/// Печать фискального чека.
	virtual bool execFiscal(const QStringList & aReceipt, const SDK::Driver::SPaymentData & aPaymentData, quint32 * aFDNumber = nullptr);

	/// Открыть чек.
	virtual bool openDocument(SDK::Driver::EPayOffTypes::Enum aPayOffType);

	/// Установить TLV-параметр.
	virtual bool setTLV(int aField, bool aOnSale = false);

	/// Продажа.
	virtual bool sale(const SDK::Driver::SUnitData & aUnitData);

	/// Закрыть чек.
	virtual bool closeDocument(SDK::Driver::EPayTypes::Enum aPayType);

	/// Открыть смену.
	virtual bool openSession();

	/// Печать Z отчета.
	virtual bool performZReport(bool aPrintDeferredReports);

	/// Снять Z-отчет.
	virtual bool execZReport(bool aAuto);

	/// Печать X-отчета. Параметром задаётся набор дополнительных строк для печати (например баланс).
	virtual bool performXReport(const QStringList & aReceipt);

	/// Локальная печать X-отчета.
	virtual bool processXReport();

	/// Печать выплаты.
	virtual bool performEncashment(const QStringList & aReceipt, double aAmount);

	/// Получить сумму в кассе.
	virtual double getAmountInCash();

	/// Печать выплаты.
	virtual bool processPayout(double aAmount);

	/// Обертка над оберткой АТОЛа над драйвером библиотеки fptr.
	AtolDriverWrapper mDriver;

	/// Прослойка для вызова OPOS-функционала в рабочем потоке.
	WorkingThreadProxy mThreadProxy;

	/// Обрабатываемые ошибки.
	CAtol5OnlineFR::ErrorData mErrors;

	/// Пытался поключиться к устройству.
	bool mTriedToConnect;

	/// Мьютекс для операций при старте потока.
	QMutex mStartMutex;

	/// Wait-condition для операций при старте потока.
	QWaitCondition mStartCondition;
};

typedef Atol5OnlineFRBase<SDK::Driver::CInteractionTypes::ItExternalCOM> SerialAtol5OnlineFR;
typedef Atol5OnlineFRBase<SDK::Driver::CInteractionTypes::ItExternalVCOM> VCOMAtol5OnlineFR;

//--------------------------------------------------------------------------------
