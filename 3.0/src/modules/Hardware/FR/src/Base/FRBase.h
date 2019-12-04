/* @file Базовый фискальный регистратор. */

#pragma once

// SDK
#include <SDK/Drivers/IFiscalPrinter.h>
#include <SDK/Drivers/FR/FiscalPrinterConstants.h>
#include <SDK/Drivers/FR/FRStatus.h>

// Modules
#include "Hardware/FR/ProtoFR.h"

// Project
#include "Hardware/FR/FRBaseConstants.h"
#include "Hardware/FR/FRErrorDescription.h"
#include "FFEngine.h"

//--------------------------------------------------------------------------------
template <class T>
class FRBase : public T
{
public:
	FRBase();

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);

	/// Готов ли к печати.
	virtual bool isDeviceReady(bool aOnline);

	/// Готов ли к обработке данной фискальной команды.
	virtual bool isFiscalReady(bool aOnline, SDK::Driver::EFiscalPrinterCommand::Enum aCommand = SDK::Driver::EFiscalPrinterCommand::Sale);

	/// Печать фискального чека.
	virtual bool printFiscal(const QStringList & aReceipt, const SDK::Driver::SPaymentData & aPaymentData, quint32 * aFDNumber = nullptr);

	/// Получить фискальные теги по номеру документа.
	virtual bool checkFiscalFields(quint32 aFDNumber, SDK::Driver::TFiscalPaymentData & aFPData, SDK::Driver::TComplexFiscalPaymentData & aPSData);

	/// Получить фискальные теги по номеру документа.
	bool processFiscalFields(quint32 aFDNumber, SDK::Driver::TFiscalPaymentData & aFPData, SDK::Driver::TComplexFiscalPaymentData & aPSData);

	/// Выполнить Z-отчет [и распечатать отложенные Z-отчеты].
	virtual bool printZReport(bool aPrintDeferredReports);

	/// Выполнить X-отчет [и распечатать нефискальный чек - баланс].
	virtual bool printXReport(const QStringList & aReceipt);

	/// Выполнить выплату [и распечатать нефискальный чек - инкассацию].
	virtual bool printEncashment(const QStringList & aReceipt);
	virtual bool printEncashment(const QStringList & aReceipt, double aAmount);

	/// Получить состояние смены.
	virtual SDK::Driver::ESessionState::Enum checkSessionState();

	/// Получить состояние документа.
	virtual SDK::Driver::EDocumentState::Enum checkDocumentState();

	/// Находится ли в фискальном режиме.
	virtual bool isFiscal() const;

	/// Является ли онлайновым.
	virtual bool isOnline() const;

	/// Установить лог.
	virtual void setLog(ILog * aLog);

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Завершение инициализации.
	virtual void finaliseInitialization();

	/// Завершение инициализации для онлайновых ФР.
	void finaliseOnlineInitialization();

	/// Инициализировать логику снятия Z-отчетов по таймеру.
	void initializeZReportByTimer();

	/// Получить и обработать статус.
	virtual bool processStatus(TStatusCodes & aStatusCodes);

	/// Фоновая логика при появлении определенных состояний устройства.
	virtual void postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection);

	/// Проверить соответствие налогов нормам 2019 года.
	void checkTaxes2019();

	/// Проверить установки сервера ОФД.
	bool checkOFDData(const QByteArray & aAddressData, const QByteArray & aPortData);

	/// Получить состояние смены.
	virtual SDK::Driver::ESessionState::Enum getSessionState();

	/// Получить состояние документа.
	virtual SDK::Driver::EDocumentState::Enum getDocumentState();

	/// Открыть смену.
	virtual bool openSession() { return false; }

	/// Открыть смену.
	virtual bool openFRSession();

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

	/// Проверить имена товаров на платеже.
	void checkUnitNames(SDK::Driver::SPaymentData & aPaymentData);

	/// Проверить суммы на платеже.
	bool checkAmountsOnPayment(const SDK::Driver::SPaymentData & aPaymentData);

	/// Проверить сумму в кассе на платеже.
	bool checkSumInCash(const SDK::Driver::SPaymentData & aPaymentData);

	/// Проверить налоги на платеже.
	bool checkTaxesOnPayment(const SDK::Driver::SPaymentData & aPaymentData);

	/// Добавить телефон в фискальные теги в конфиг.
	void addPhone(const QString & aField, const QVariant & aData);

	/// Добавить почту в фискальные теги в конфиг.
	void addMail(const QString & aField, const QVariant & aData);

	/// Добавить фискальные теги в платеж.
	void addFiscalFieldsOnPayment(const SDK::Driver::SPaymentData & aPaymentData);

	/// Проверить наличие и необходимость фискальных тегов на платеже.
	void checkFFExistingOnPayment(int aField, bool aAdd, bool aRequired = true);

	/// Добавить данные в фискальный тег из конфига или параметра.
	typedef std::function<void(QString &)> TFFConfigData;
	void addConfigFFData(const QString & aField, const QVariant & aData, const TFFConfigData & aFFConfigData = TFFConfigData());

	/// Проверить тип оплаты на платеже.
	bool checkPayTypeOnPayment(const SDK::Driver::SPaymentData & aPaymentData);

	/// Проверить фискальные теги на платеже.
	bool checkFiscalFieldsOnPayment();

	/// Проверить параметры налогов.
	virtual bool checkTaxes();

	/// Проверить параметры налога.
	virtual bool checkTax(SDK::Driver::TVAT /*aVAT*/, CFR::Taxes::SData & /*aData*/) { return true; }

	/// Включена непечать?
	bool isNotPrinting();

	/// Может не печатать?
	bool canNotPrinting();

	/// Проверить необходимость печати.
	virtual bool isPrintingNeed(const QStringList & aReceipt);

	/// Локальная печать X-отчета.
	virtual bool processXReport() = 0;

	/// Печать фискального чека.
	virtual bool performFiscal(const QStringList & /*aReceipt*/, const SDK::Driver::SPaymentData & /*aPaymentData*/, uint * /*aFDNumber = nullptr*/) { return false; }

	/// Печать фискального чека.
	bool processFiscal(const QStringList & aReceipt, const SDK::Driver::SPaymentData & aPaymentData, uint * aFDNumber);

	/// Получить фискальные теги по номеру документа.
	virtual bool getFiscalFields(quint32 /*aFDNumber*/, SDK::Driver::TFiscalPaymentData & /*aFPData*/, SDK::Driver::TComplexFiscalPaymentData & /*aPSData*/) { return false; }

	/// Установить реквизиты ОФД.
	bool setOFDParameters();

	/// Установить реквизиты ОФД на продаже.
	bool setOFDParametersOnSale(const SDK::Driver::SUnitData & aUnitData);

	/// Установить TLV-параметр.
	virtual bool setTLV(int /*aField*/, bool /*aForSale*/ = false) { return true; }

	/// Проверить Z-отчет по таймеру.
	void checkZReportByTimer();

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

	/// Получить налоговые ставки.
	SDK::Driver::TVATs getDeviceVATs();

	/// Получить все налоговые ставки платежа.
	SDK::Driver::TVATs getVATs(const SDK::Driver::SPaymentData & aPaymentData) const;

	/// Логгировать данные налогов.
	QString getVATLog(const SDK::Driver::TVATs & aVATs) const;

	/// Включить/выключить режим непечати документов.
	virtual bool setNotPrintDocument(bool aEnabled, bool aZReport = false);

	/// Проверить непечать документа.
	bool checkNotPrinting(bool aEnabled = false, bool aZReport = false);

	/// Может работать с буфером Z-отчетов?
	virtual bool canProcessZBuffer();

	/// Получить статус по типу ошибки устройства.
	static int getErrorStatusCode(FRError::EType::Enum aErrorType);

	/// Является ли срок годности ФН 36 месяцев. По умолчанию (не получилось сделать какую-то проверку) - нет.
	bool isFS36() const;

	/// Наличие ЭКЛЗ.
	bool mEKLZ;

	/// Постоянная ошибка ЭКЛЗ.
	bool mEKLZError;

	/// Постоянная ошибка ФП.
	bool mFMError;

	/// Постоянная ошибка ФН.
	bool mFSError;

	/// Исчерпан ресурс хранения отчетов в ФН.
	bool mFSOfflineEnd;

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

	/// Фискальные теги для установки в момент печати фискального чека.
	QSet<int> mOFDFiscalFields;

	/// Фискальные теги для установки в момент печати фискального чека на продаже.
	QSet<int> mOFDFiscalFieldsOnSale;

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
	CFR::FiscalFields::Data mFFData;

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

	/// Ошибка налоговых ставок.
	bool mTaxError;

	/// Необходима синхронизация с системным временем.
	bool mNeedTimeSynchronization;

	/// Неверная налоговая ставка на платеже.
	bool mWrongTaxOnPayment;
};

//--------------------------------------------------------------------------------
