/* @file ФР СПАРК. */

#pragma once

// Modules
#include "Hardware/FR/PortFRBase.h"
#include "Hardware/Protocols/FR/SparkFR.h"
#include "Hardware/Protocols/FR/FiscalChequeStates.h"

// Project
#include "SparkModelData.h"
#include "SparkFRConstants.h"

//--------------------------------------------------------------------------------
class SparkFR : public TSerialFRBase
{
	SET_SERIES("SPARK")

public:
	SparkFR();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Проверить параметры налогов.
	typedef QList<QByteArray> TTaxData;
	bool checkTaxFlags(const TTaxData & aTaxData);

	/// Проверить параметры налогов во флагах.
	typedef QList<QByteArray> TTaxData;
	bool checkSystemFlags(QByteArray & aFlagData);

	/// Проверить установки системного флага.
	bool checkSystemFlag(const QByteArray & aFlagBuffer, int aNumber);

	/// Получить системные флаги.
	bool getSystemFlags(QByteArray & aData, TTaxData * aTaxes = nullptr);

	/// Получить дату и время ФР.
	virtual QDateTime getDateTime();

	/// Напечатать строку.
	virtual bool printLine(const QByteArray & aString);

	/// Применить теги.
	virtual void execTags(Tags::SLexeme & aTagLexeme, QVariant & aLine);

	/// Печать фискального чека.
	virtual bool performFiscal(const QStringList & aReceipt, const SDK::Driver::SPaymentData & aPaymentData, SDK::Driver::TFiscalPaymentData & aFPData, SDK::Driver::TComplexFiscalPaymentData & aPSData);

	/// Печать Z отчета.
	virtual bool performZReport(bool aPrintDeferredReports);

	/// Локальная печать X-отчета.
	virtual bool processXReport();

	/// Печать выплаты.
	virtual bool processPayout(double aAmount);

	/// Получить сумму в кассе.
	virtual double getAmountInCash();

	/// Отрезка.
	virtual bool cut();

	/// Забрать чек в ретрактор.
	virtual bool retract();

	/// Выполнить команду.
	TResult processCommand(const QByteArray & aCommand, QByteArray * aAnswer = nullptr, int aTimeout = CSparkFR::Timeouts::Default);
	TResult processCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr, int aTimeout = CSparkFR::Timeouts::Default);

	/// Получить состояние Z-буффера.
	void getZBufferState();

	/// Аварийно завершить фискальный(?) документ в случае ошибки фискальной части.
	bool cancelDocument(bool aDocumentIsOpened);

	/// Запросить и вывести в лог критичные параметры ФР.
	void processDeviceData();

	/// Получить данные о ККМ.
	typedef QList<QByteArray> TKKMInfoData;
	bool getKKMData(TKKMInfoData & aData);

	/// Получить дату и время из запроса данных о ККМ.
	QDateTime parseDateTime(TKKMInfoData & aData);

	/// Сессия истекла?
	bool isSessionExpired();

	/// Обработка ответа на предыдущей команды. Автоисправление некоторых ошибок.
	bool processAnswer(char aError);

	/// Выполнить Z-отчет.
	virtual bool execZReport(bool aAuto);

	/// Узнать, открыта ли смена.
	virtual SDK::Driver::ESessionState::Enum getSessionState();

	/// Получить номер последнего фискального документа.
	int getLastDocumentNumber();

	/// Адаптивный ли способ формирования фискального чека.
	bool isAdaptiveFCCreation();

	/// Установить фискальные реквизиты ПФД.
	bool setFiscalParameters(const QStringList & aReceipt);

	/// Внесение/выплата.
	bool payIO(double aAmount, bool aIn);

	/// Продажа.
	bool sale(const SDK::Driver::SUnitData & aUnitData);

	/// Извлечь данные из специфичного BCD-формата.
	template <class T>
	T fromBCD(const QByteArray & aData);
	char fromBCD(char aData);

	/// Подождать готовность эжектора.
	bool waitEjectorReady();

	/// Подождать окончание печати следующего документа.
	bool waitNextPrinting();

	/// Протокол.
	SparkFRProtocol mProtocol;

	/// Список поддерживаемых плагином моделей.
	QStringList mSupportedModels;

	/// Последняя ошибка.
	char mDocumentState;

	/// Системные флаги.
	CSparkFR::SystemFlags::Data mSystemFlags;

	/// Дата и время начала смены.
	QDateTime mSessionOpeningDT;

	/// Количество Z-отчетов в буфере.
	int mZReports;

	/// Можно ли проверять статус в нештатных ситуациях при выполнении команды.
	bool mCheckStatus;

	/// Налоги.
	typedef QList<SDK::Driver::TVAT> TTaxes;
	TTaxes mTaxes;
};

//--------------------------------------------------------------------------------
