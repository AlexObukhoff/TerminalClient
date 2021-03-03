/* @file Прото-ФР семейства Штрих на порту. */

#pragma once

// Modules
#include "Hardware/Protocols/FR/ShtrihFR.h"

// Project
#include "../ShtrihFRDataTypes.h"
#include "../ShtrihFRConstants.h"
#include "../Base/ShtrihSerialFRBase.h"
#include "../Online/ShtrihTCPFRBase.h"
#include "../ModelData.h"

//--------------------------------------------------------------------------------
template <class T>
class ProtoShtrihFR : public T
{
	SET_SERIES("Shtrih")

public:
	ProtoShtrihFR();

protected:
	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Получить длинный статус.
	TResult getLongStatus(QByteArray & aData);
	TResult getLongStatus();

	/// Добавить общие статусы.
	virtual void appendStatusCodes(ushort aFlags, TStatusCodes & aStatusCodes);

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Фоновая логика при появлении определенных состояний устройства.
	virtual void postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection);

	/// Получить дату и время ФР.
	virtual QDateTime getDateTime();

	/// Получить параметры печати.
	virtual bool getPrintingSettings();

	/// Выполнить команду.
	virtual TResult execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr);

	/// Ошибка?
	virtual bool isNotError(char aCommand);

	/// Обработка ответа на предыдущей команды. Автоисправление некоторых ошибок.
	virtual bool processAnswer(const QByteArray & aCommand, char aError);

	/// Печать выплаты.
	virtual bool processPayout(double aAmount);

	/// Получить сумму в кассе.
	virtual double getAmountInCash();

	/// Напечатать строку.
	virtual bool printLine(const QByteArray & aString);

	/// Печать фискального чека.
	virtual bool performFiscal(const QStringList & aReceipt, const SDK::Driver::SPaymentData & aPaymentData, quint32 * aFDNumber = nullptr);

	/// Отмена фискального чека.
	virtual bool cancelFiscal();

	/// Открыть чек.
	bool openDocument(SDK::Driver::EPayOffTypes::Enum aPayOffType);

	/// Продажа.
	virtual bool sale(const SDK::Driver::SUnitData & aUnitData, SDK::Driver::EPayOffTypes::Enum aPayOffType);

	/// Закрыть чек.
	virtual bool closeDocument(double aSum, SDK::Driver::EPayTypes::Enum aPayType);

	/// Проверить название продажи.
	virtual void checkSalesName(QString & aName);

	/// Установить флаги по ошибке в ответе.
	virtual void setErrorFlags() {}

	/// Печать Z отчета.
	virtual bool performZReport(bool aPrintDeferredReports);

	/// Снять Z-отчет.
	virtual bool execZReport(bool aAuto);

	/// Проверка готовности фискальника к Z-отчету.
	virtual bool prepareZReport(bool aAuto, QVariantMap & aOutData);

	/// Собрать данные о сессии перед выполнением Z-отчета.
	QVariantMap getSessionOutData(const QByteArray & aLongStatusData);

	/// Локальная печать X-отчета.
	virtual bool processXReport();

	/// После подачи команды X-отчета ждем окончания формирования X-отчета.
	bool waitForChangeXReportMode();

	/// После подачи команды, связанной с печатью ждем окончания печати.
	//TODO: зачем ждем перед ожиданием?
	bool waitForPrintingEnd(bool aCanBeOff = false, int aTimeout = CShtrihFR::Timeouts::MaxWaitForPrintingEnd);

	/// Проверить параметры налога.
	virtual bool checkTax(SDK::Driver::TVAT aVAT, CFR::Taxes::SData & aData);

	/// Установить значение налога.
	virtual bool setTaxValue(SDK::Driver::TVAT aVAT, int aGroup);

	/// Отрезка.
	virtual bool cut();

	/// Получить состояние смены.
	virtual SDK::Driver::ESessionState::Enum getSessionState();

	/// Установить параметр системной таблицы.
	bool setFRParameter(const CShtrihFR::FRParameters::SData & aData, const QVariant & aValue, char aSeries = 1, bool aCleanLogValue = false);

	/// Получить параметр системной таблицы.
	bool getFRParameter(const CShtrihFR::FRParameters::SData & aData, QByteArray & aValue, char aSeries = 1);

	/// Установить параметры ФР.
	virtual void setFRParameters();

	/// Получить регистр по номеру и типу.
	bool getRegister(const CShtrihFR::TRegisterId & aRegister, QByteArray & aFRRegister);

	/// Запросить и вывести в лог критичные параметры ФР.
	virtual void processDeviceData();

	/// Распарсить данные ФР, полученные из длинного статуса.
	virtual void parseDeviceData(const QByteArray & aData);

	/// Получить состояние документа.
	virtual SDK::Driver::EDocumentState::Enum getDocumentState();

	/// Определяет по ID модели, есть ли в ФР весовой датчик для чековой ленты.
	bool isPaperWeightSensor() const;

	/// Определяет по ID модели, есть ли в ФР оптический датчик для чековой ленты.
	bool isPaperOpticalSensor() const;

	/// Определяет по ID модели, есть ли в ФР рычаг, прижимающий чековую ленту к голове.
	bool isPaperLeverExist() const;

	/// Определяет по ID модели, есть ли в ФР крышка сенсора.
	bool isCoverSensor() const;

	/// Протокол.
	ShtrihFRProtocol mProtocol;

	/// ID типа устройства.
	int mType;

	/// ID модели устройства.
	int mModel;

	/// Текущий режим.
	char mMode;

	/// Текущий подрежим.
	char mSubmode;

	/// Необнуляемая сумма.
	double mNonNullableAmount;

	/// Данные модели.
	CShtrihFR::SModelData mModelData;

	/// Параметры системных таблиц.
	CShtrihFR::FRParameters::SFields mParameters;

	/// Данные команд.
	CShtrihFR::Commands::Data mCommandData;

	/// Номер шрифта.
	char mFontNumber;

	/// Таймаут технологических посылок.
	int mTransportTimeout;

	/// На отмене фискального документа необходима [промотка и] отрезка чека.
	bool mNeedReceiptProcessingOnCancel;
};

//--------------------------------------------------------------------------------
