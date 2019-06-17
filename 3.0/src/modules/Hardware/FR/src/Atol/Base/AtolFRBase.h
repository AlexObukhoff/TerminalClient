/* @file Базовый ФР на протоколе АТОЛ. */

#pragma once

// Modules
#include "Hardware/FR/PortFRBase.h"

// Project
#include "../AtolModelData.h"
#include "../AtolFRConstants.h"

class AtolSeriesType {};

//--------------------------------------------------------------------------------
class AtolFRBase : public TSerialFRBase
{
public:
	typedef AtolSeriesType TSeriesType;

	AtolFRBase();

	/// Готов ли к обработке данной фискальной команды.
	virtual bool isFiscalReady(bool aOnline, SDK::Driver::EFiscalPrinterCommand::Enum aCommand = SDK::Driver::EFiscalPrinterCommand::Sale);

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Получить ключ модели для идентификации.
	virtual CAtolFR::TModelKey getModelKey(const QByteArray & /*aAnswer*/) { return CAtolFR::TModelKey(); }

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Завершение инициализации.
	virtual void finaliseInitialization();

	/// Получить дату и время ФР.
	virtual QDateTime getDateTime();

	/// Установить начальные параметры.
	virtual void setInitialData();

	/// Получить параметры печати.
	virtual bool getPrintingSettings();

	/// Напечатать [и выдать] чек.
	virtual bool processReceipt(const QStringList & aReceipt, bool aProcessing = true);

	/// Напечатать строку.
	virtual bool printLine(const QByteArray & aString);

	/// Печать фискального чека.
	virtual bool performFiscal(const QStringList & aReceipt, const SDK::Driver::SPaymentData & aPaymentData, quint32 * aFDNumber = nullptr);

	/// Печать Z отчета.
	virtual bool performZReport(bool aPrintDeferredReports);

	/// Печать отложенных Z-отчетов.
	virtual bool printDeferredZReports();

	/// Локальная печать X-отчета.
	virtual bool processXReport();

	/// Печать выплаты.
	virtual bool processPayout(double aAmount);

	/// Получить сумму в кассе.
	virtual double getAmountInCash();

	/// Применить теги.
	virtual void execTags(Tags::SLexeme & aTagLexeme, QVariant & aLine);

	/// Открыть смену.
	virtual bool openSession();

	/// Отрезка.
	virtual bool cut();

	/// Получить общие для всех ФР статусы.
	virtual bool getCommonStatus(TStatusCodes & aStatusCodes);

	/// Выполнить команду.
	virtual TResult execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr);

	/// Выполнить команду.
	virtual TResult performCommand(const QByteArray & /*aCommandData*/, QByteArray & /*aAnswer*/, int /*aTimeout*/) { return CommandResult::NoAnswer; }

	/// Войти в расширенный режим снятия Z-отчетов.
	virtual bool enterExtendedMode() { return true; }

	/// Открыть чек.
	virtual bool openDocument(SDK::Driver::EPayOffTypes::Enum aPayOffType);

	/// Закрыть чек.
	bool closeDocument(SDK::Driver::EPayTypes::Enum aPayType);

	/// Продажа.
	virtual bool sale(const SDK::Driver::SUnitData & aUnitData);

	/// Установка количества строк шапки.
	bool setDocumentCapAmount(char aAmount);

	/// Аварийно завершить фискальный(?) документ в случае ошибки фискальной части.
	virtual void cancelDocument(bool aDocumentIsOpened);

	/// Запросить и вывести в лог критичные параметры ФР.
	virtual void processDeviceData();

	/// Проверить налоговую ставку.
	bool checkTaxValue(SDK::Driver::TVAT aVAT, CFR::Taxes::SData & aData, const CAtolFR::FRParameters::TData & aFRParameterData, bool aCanCorrectTaxValue);

	/// Получить данные о текущей смене.
	bool getLastSessionDT(QDateTime & aLastSessionDT);

	/// Обработка ответа на предыдущей команды. Автоисправление некоторых ошибок.
	virtual bool processAnswer(const QByteArray & aCommand, char aError);

	/// Проверить параметры налогов.
	virtual bool checkTaxes();

	/// Получить состояние фискального чека.
	bool getFiscalDocumentState(EFiscalDocumentState::Enum & aState);

	/// Получить состояние документа.
	virtual SDK::Driver::EDocumentState::Enum getDocumentState();

	/// Войти в режим.
	bool enterInnerMode(char aInnerMode);

	/// Выйти из режима.
	bool exitInnerMode(bool aForce = false);

	/// Выполнить Z-отчет.
	virtual bool execZReport(bool aAuto);

	/// Установить параметры ФР.
	virtual bool setFRParameters();

	/// Получить параметр системной таблицы.
	bool getFRParameter(const CAtolFR::FRParameters::SData & aData, QByteArray & aValue);

	/// Установить параметр системной таблицы.
	bool setFRParameter(const CAtolFR::FRParameters::SData & aData, const QVariant & aValue);

	/// Вернуть регистр ФР.
	TResult getRegister(const QString & aRegister, QByteArray & aData, char aParameter1 = ASCII::NUL, char aParameter2 = ASCII::NUL);

	/// После подачи команды X-отчета ждем смены режима.
	bool waitForChangeXReportMode();

	/// После подачи команды Z-отчета ждем смены режима.
	bool waitForChangeZReportMode();

	/// Запросить версию софта ФР.
	bool getSoftVersion(char aSoftType, CAtolFR::SSoftInfo & aSoftInfo);

	/// Проверить состояние буфера Z-отчетов.
	void checkZBufferState();

	/// Получить состояние смены.
	virtual SDK::Driver::ESessionState::Enum getSessionState();

	/// Получить короткий статус.
	virtual bool getShortStatus(TStatusCodes & aStatusCodes);

	/// Распарсить флаги короткого статуса.
	virtual void parseShortStatusFlags(char aFlags, TStatusCodes & aStatusCodes);

	/// Получить длинный статус.
	bool getLongStatus(QByteArray & aData);

	/// Установить флаги по ошибке в ответе.
	virtual void setErrorFlags(const QByteArray & aCommand, char aError);

	/// Получить BCD из int-а.
	QByteArray getBCD(double aValue, int aSize, int aPrecision = 0, int aMantissa = 0) const;

	/// Получить ошибку из ответа.
	char getError(const QByteArray & aCommand, const QByteArray & aAnswer);
	char getError(char aCommand, const QByteArray & aAnswer);

	/// Данные устройства.
	CAtolFR::SModelData mModelData;

	/// Номер сборки прошивки ПО ФР.
	int mFRBuild;

	/// Текущий режим.
	char mMode;

	/// Текущий подрежим.
	char mSubmode;

	/// Заблокирован ли ФР.
	bool mLocked;

	/// Список поддерживаемых плагином моделей.
	QStringList mSupportedModels;

	/// Количество рекламных строк.
	char mDocumentCapStrings;

	/// Необнуляемая сумма;
	double mNonNullableAmount;

	/// Данные команд.
	CAtolFR::CommandData mCommandData;

	/// Данные регистров.
	CAtolFR::Registers::CData mRegisterData;
};

//--------------------------------------------------------------------------------
