/* @file Базовый фискальный регистратор. */

#pragma once

// SDK
#include <SDK/Drivers/IFiscalPrinter.h>
#include <SDK/Drivers/FR/FiscalPrinterConstants.h>
#include <SDK/Drivers/FR/FRStatus.h>

// Project
#include "Hardware/FR/FRBaseConstants.h"
#include "FFEngine.h"

//--------------------------------------------------------------------------------
template <class T>
class FRBase : public T
{
public:
	FRBase();

	/// Подключает и инициализует устройство. Обертка для вызова функционала в рабочем потоке.
	virtual void initialize();

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);

	/// Готов ли к печати.
	virtual bool isDeviceReady(bool aOnline);

	/// Готов ли к обработке данной фискальной команды.
	virtual bool isFiscalReady(bool aOnline, SDK::Driver::EFiscalPrinterCommand::Enum aCommand = SDK::Driver::EFiscalPrinterCommand::Sale);

	/// Печать фискального чека.
	virtual bool printFiscal(const QStringList & aReceipt, const SDK::Driver::SPaymentData & aPaymentData, SDK::Driver::TFiscalPaymentData & aFPData, SDK::Driver::TComplexFiscalPaymentData & aPSData);

	/// Выполнить Z-отчет [и распечатать отложенные Z-отчеты].
	virtual bool printZReport(bool aPrintDeferredReports);

	/// Выполнить X-отчет [и распечатать нефискальный чек - баланс].
	virtual bool printXReport(const QStringList & aReceipt);

	/// Выполнить выплату [и распечатать нефискальный чек - инкассацию].
	virtual bool printEncashment(const QStringList & aReceipt);
	virtual bool printEncashment(const QStringList & aReceipt, double aAmount);

	/// Находится ли в фискальном режиме.
	virtual bool isFiscal() const;

	/// Является ли онлайновым.
	virtual bool isOnline() const;

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Завершение инициализации.
	virtual void finaliseInitialization();

	/// Запрос статуса.
	virtual bool processStatus(TStatusCodes & aStatusCodes);

	/// Проверить установки сервера ОФД.
	bool checkOFDData(const QByteArray & aAddressData, const QByteArray & aPortData);

	/// Открыта ли сессия.
	virtual SDK::Driver::ESessionState::Enum getSessionState();

	/// Открыть смену.
	virtual bool openSession() { return false; }

	/// Открыть смену.
	bool openFRSession();

	/// Установить начальные параметры.
	virtual void setInitialData();

	/// Анализирует коды статусов устройства и фильтрует лишние.
	virtual void cleanStatusCodes(TStatusCodes & aStatusCodes);

	/// Получить дату и время ФР.
	virtual QDateTime getDateTime() { return QDateTime(); }

	/// Получить номер смены.
	virtual int getSessionNumber() { return 0; }

	/// Загрузить СНО.
	bool checkTaxSystems(char aData);

	/// Загрузить признаки агента.
	bool checkAgentFlags(char aData);

	/// Загрузить режимы работы.
	bool checkOperationModes(char aData);

	/// Проверить суммы на платеже.
	bool checkAmountsOnPayment(const SDK::Driver::SPaymentData & aPaymentData);

	/// Проверить сумму в кассе на платеже.
	bool checkSumInCash(const SDK::Driver::SPaymentData & aPaymentData);

	/// Проверить налоги на платеже.
	bool checkVATsOnPayment(const SDK::Driver::SPaymentData & aPaymentData);

	/// Добавить фискальные теги в платеж.
	void addFiscalFieldsOnPayment(const SDK::Driver::SPaymentData & aPaymentData);

	/// Проверить тип оплаты на платеже.
	bool checkPayType(const SDK::Driver::SPaymentData & aPaymentData);

	/// Проверить параметры налогов.
	virtual bool checkTaxes();

	/// Проверить параметры налога.
	virtual bool checkTax(SDK::Driver::TVAT /*aVAT*/, const CFR::Taxes::SData & /*aData*/) { return true; }

	/// Локальная печать X-отчета.
	virtual bool processXReport() = 0;

	/// Печать фискального чека.
	virtual bool performFiscal(const QStringList & /*aReceipt*/, const SDK::Driver::SPaymentData & /*aPaymentData*/, SDK::Driver::TFiscalPaymentData & /*aFPData*/, SDK::Driver::TComplexFiscalPaymentData & /*aPSData*/) { return false; }

	/// Установить реквизиты ОФД.
	bool setOFDParameters();

	/// Установить реквизиты ОФД на продаже.
	bool setOFDParametersOnSale(const SDK::Driver::SUnitData & aUnitData);

	/// Установить TLV-параметр.
	virtual bool setTLV(int /*aField*/, bool /*aForSale*/ = false) { return true; }

	/// Проверить Z-отчет по таймеру.
	void checkZReportOnTimer();

	/// Выполнить Z-отчет.
	virtual void onExecZReport();

	/// Выполнить Z-отчет.
	virtual bool execZReport(bool /*aAuto*/) { return false; }

	/// Печать Z-отчета.
	virtual bool performZReport(bool aPrintDeferredReports) = 0;

	/// Печать X-отчета. Параметром задаётся набор дополнительных строк для печати (например баланс).
	virtual bool performXReport(const QStringList & aReceipt);

	/// Печать выплаты.
	virtual bool performEncashment(const QStringList & aReceipt, double aAmount);

	/// Составной фискальный документ.
	bool complexFiscalDocument(TBoolMethod aMethod, const QString & aLog);

	/// Печать выплаты.
	virtual bool processPayout(double /*aAmount*/) { return false; }

	/// Выполнить выплату [и распечатать нефискальный чек - инкассацию].
	bool processEncashment(const QStringList & aReceipt, double aAmount = DBL_MAX);

	/// Получить сумму в кассе.
	virtual double getAmountInCash() { return -1; }

	/// Проверить количество неотправленных в ОФД документов.
	void checkOFDNotSentCount(int aOFDNotSentCount, TStatusCodes & aStatusCodes);

	/// Проверить флаги ФН.
	void checkFSFlags(char aFlags, TStatusCodes & aStatusCodes);

	/// Проверить отклонение даты-времени ФР от системного.
	void checkDateTime();

	/// Получить сумму итога для закрытия чека.
	SDK::Driver::TSum getTotalAmount(const SDK::Driver::SPaymentData & aPaymentData) const;

	/// Получить налоговые ставки региона.
	SDK::Driver::TVATs getActualVATs() const;

	/// Получить все налоговые ставки платежа.
	SDK::Driver::TVATs getVATs(const SDK::Driver::SPaymentData & aPaymentData) const;

	/// Логгировать данные налогов.
	QString getVATLog(const SDK::Driver::TVATs & aVATs) const;

	/// Включить/выключить режим непечати документов.
	virtual bool setNotPrintDocument(bool aEnabled);

	/// Проверить непечать документа.
	bool checkNotPrinting(bool aEnabled = false);

	/// Наличие ЭКЛЗ.
	bool mEKLZ;

	/// Постоянная ошибка ЭКЛЗ.
	bool mEKLZError;

	/// Постоянная ошибка ФП.
	bool mFMError;

	/// Постоянная ошибка ФН.
	bool mFSError;

	/// Буфер Z-отчетов заполнен.
	bool mZBufferFull;

	/// Буфер Z-отчетов переполнен.
	bool mZBufferOverflow;

	/// Нужна ли обработка текущего чека, если следом пойдет фискальный документ.
	bool mNextReceiptProcessing;

	/// Глобальная ошибка принтерной части части ФР, печать невозможна.
	bool mPrinterCollapse;

	/// Признак фискализированности ККМ.
	bool mFiscalized;

	/// Ошибка буфера Z-отчетов.
	int mZBufferError;

	/// Глобальная ошибка фискальной части ФР, печать невозможна.
	bool mFiscalCollapse;

	/// Свободное место в буфере Z-отчетов.
	int mWhiteSpaceZBuffer;

	/// Необходимо выполнить Z-отчет (для ФР без буфера).
	bool mNeedCloseSession;

	/// Время последнего открытия смены.
	QDateTime mLastOpenSession;

	/// Является ли онлайновым.
	bool mIsOnline;

	/// Ошибка данных ОФД в ФР: 1. данные ФР некорректны по формату; 2. это точно тестовый сервер; 3. URL или IP точно не соответствует порту
	bool mOFDDataError;

	/// Регион.
	ERegion::Enum mRegion;

	/// Реквизиты ОФД для установки в момент печати фискального чека.
	QSet<int> mOFDFiscalParameters;

	/// Реквизиты ОФД для установки в момент печати фискального чека на продаже.
	QSet<int> mOFDFiscalParametersOnSale;

	/// Количество неотправленных документов в ОФД.
	int mOFDNotSentCount;

	/// Метка даты-времени связи с ОФД.
	QDateTime mOFDDTMark;

	/// Данные налогов.
	CFR::Taxes::Data mTaxData;

	/// Данные типов оплаты.
	CFR::PayTypeData mPayTypeData;

	/// Системы налогообложения (СНО).
	TTaxSystems mTaxSystems;

	/// Признаки агента.
	TAgentFlags mAgentFlags;

	/// Режимы работы.
	TOperationModes mOperationModes;

	/// Серийный номер ФР.
	QString mSerial;

	/// Серийный номер ФН.
	QString mFSSerialNumber;

	/// РНМ.
	QString mRNM;

	/// ИНН.
	QString mINN;

	/// ФФД ФР.
	EFFD::Enum mFFDFR;

	/// ФФД ФН.
	EFFD::Enum mFFDFS;

	/// Данные фискальных реквизитов.
	CFR::FiscalFields::CData mFiscalFieldData;

	/// Может работать с буфером Z-отчетов.
	bool mCanProcessZBuffer;

	/// Параметры фискализации некорректны?
	bool mWrongFiscalizationSettings;

	/// Экземляр движка фискальных тегов.
	FFEngine mFFEngine;

	/// Ошибка установки непечати.
	bool mNotPrintingError;

	/// Ошибка ИНН кассира.
	bool mCashierINNError;
};

//--------------------------------------------------------------------------------
