/* @file Принтеры семейства ПРИМ. */

#pragma once

// Modules
#include "Hardware/FR/PortFRBase.h"
#include "Hardware/Protocols/FR/PrimFR.h"

// Project
#include "PrimModelData.h"
#include "PrimFRConstants.h"

class PrimSeriesType {};

//--------------------------------------------------------------------------------
class PrimFRBase : public TSerialFRBase
{
	SET_SERIES("PRIM")
	typedef PrimSeriesType TSeriesType;

public:
	PrimFRBase();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Напечатать [и выдать] чек.
	virtual bool processReceipt(const QStringList & aReceipt, bool aProcessing = true);

	/// Напечатать [и выдать] чек.
	virtual bool performReceipt(const QStringList & aReceipt, bool aProcessing = true);

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Получить дату и время ФР.
	virtual QDateTime getDateTime();

	/// Напечатать строку.
	virtual bool printLine(const QByteArray & aString);

	/// Печать фискального чека.
	virtual bool performFiscal(const QStringList & aReceipt, const SDK::Driver::SPaymentData & aPaymentData, quint32 * aFDNumber = nullptr);

	/// Преобразование нефискальной квитанции для ПФД.
	void makeAFDReceipt(QStringList & aReceipt);

	/// Заполнить фискальные данные для ПФД.
	virtual void setFiscalData(CPrimFR::TData & aCommandData, CPrimFR::TDataList & aAdditionalAFDData, const SDK::Driver::SPaymentData & aPaymentData, int aReceiptSize);

	/// Печать Z отчета.
	virtual bool performZReport(bool aPrintDeferredReports);

	/// Печать Z отчета.
	virtual bool execZReport(bool aAuto);

	/// Выполнить Z-отчет.
	virtual TResult doZReport(bool aAuto);

	/// Открыть смену.
	virtual bool openSession();

	/// Локальная печать X-отчета.
	virtual bool processXReport();

	/// Печать выплаты.
	virtual bool processPayout(double aAmount);

	/// Получить сумму в кассе.
	virtual double getAmountInCash();

	/// Получить инфо о ресурсах и статусе.
	bool getStatusInfo(TStatusCodes & aStatusCodes, CPrimFR::TData & aAnswer);

	/// Получить проверочный код последнего фискального документа - номер КПК.
	virtual int getVerificationCode();

	/// Установка режима работы принтер/ФР.
	bool setMode(EFRMode::Enum aMode);

	/// Проверить параметры налога.
	virtual bool checkTax(SDK::Driver::TVAT aVAT, CFR::Taxes::SData & aData);

	/// Получить параметры налога.
	bool getTaxData(int aGroup, CPrimFR::Taxes::SData & aData);

	/// Установить параметры налога.
	bool setTaxData(int aGroup, const CPrimFR::Taxes::SData & aData);

	/// Установка начального номера в буфере Z-отчетов.
	bool setStartZReportNumber(int aNumber, const CPrimFR::TData & aExtraData);

	/// Получение начального номера в буфере Z-отчетов.
	int getStartZReportNumber(CPrimFR::TData & aExtraData);

	/// Получение конечного номера в буфере Z-отчетов.
	int getEndZReportNumber();

	/// Печать отложенного Z-отчета.
	bool printDeferredZReport(int aNumber);

	/// Выполнить команду.
	TResult processCommand(char aCommand, CPrimFR::TData * aAnswer = nullptr);
	TResult processCommand(char aCommand, const CPrimFR::TData & aCommandData, CPrimFR::TData * aAnswer = nullptr);

	template <class T>
	TResult processCommand(char aCommand, int aIndex, const QString & aLog, T & aResult);
	template <class T>
	TResult processCommand(char aCommand, const CPrimFR::TData & aCommandData, int aIndex, const QString & aLog, T & aResult);

	/// Распарсить данные ответа.
	template <class T>
	bool parseAnswerData(const CPrimFR::TData & aData, int aIndex, const QString & aLog, T & aResult);

	/// Загрузить данные устройства.
	template <class T>
	void loadDeviceData(const CPrimFR::TData & aData, const QString & aName, const QString & aLog, int aIndex, const QString & aExtensibleName = "");

	/// Проверить ответ.
	TResult checkAnswer(TResult aResult, const QByteArray & aAnswer, CPrimFR::TData & aAnswerData);

	/// Добавить обязательное G-поле ПФД в данные команды.
	CPrimFR::TData addGFieldToBuffer(int aX, int aY);

	/// Сформировать необязательное G-поле ПФД
	CPrimFR::TData addArbitraryFieldToBuffer(int aX, int aY, const QString & aData, bool aNoPrint = false, bool aBarCode = false);

	/// Обработка ответа предыдущей команды. Автоисправление некоторых ошибок.
	virtual bool processAnswer(char aError);

	/// Проверить параметры ФР.
	bool checkParameters();

	/// Получить параметр 3 ФР.
	virtual ushort getParameter3();

	/// Проверить настройки ФР.
	bool checkControlSettings();

	/// Получить реал-тайм статусы принтера.
	void getRTStatuses(TStatusCodes & aStatusCodes);

	/// Получить реал-тайм статусы принтера по реал-тайм коду.
	TStatusCodes getRTStatus(int aCommand);

	/// Распарсить реал-тайм статусы принтера по реал-тайм коду.
	TStatusCodes parseRTStatus(int aCommand, char aAnswer);

	/// Получить состояние смены.
	virtual SDK::Driver::ESessionState::Enum getSessionState();

	/// Получить состояние документа.
	virtual SDK::Driver::EDocumentState::Enum getDocumentState();

	/// Получить ASCII-представление 1-байтного целочисленного числа.
	inline QString int2String(int aValue);
	inline QByteArray int2ByteArray(int aValue);

	/// Режим работы.
	EFRMode::Enum mMode;

	/// Модели данной реализации.
	CPrimFR::TModels mModels;

	/// Id модели.
	CPrimFR::Models::Enum mModel;

	/// Признак нахождения ФР в оффлайне из-за ошибки принтера, фискальная подсистема не отвечает на запросы.
	bool mOffline;

	/// Протокол реал-тайм запросов.
	PrimFRRealTimeProtocol mRTProtocol;

	/// Протокол.
	PrimFRProtocol mProtocol;

	/// Ошибки.
	typedef QSharedPointer<CPrimFR::Errors::ExtraDataBase> PExtraErrorData;
	PExtraErrorData mExtraErrorData;

	/// Таймауты.
	CPrimFR::CommandTimouts mCommandTimouts;

	/// Шрифт для ПФД.
	int mAFDFont;
};

//--------------------------------------------------------------------------------
