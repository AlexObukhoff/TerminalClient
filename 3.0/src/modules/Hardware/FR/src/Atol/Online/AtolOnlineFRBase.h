/* @file Онлайн ФР семейства АТОЛ. */

#pragma once

// Modules
#include "Hardware/Common/TCPDeviceBase.h"
#include "Hardware/Printers/PortPrinterBase.h"

// Project
#include "Hardware/FR/AtolOnlinePrinters.h"
#include "../Base/Atol2FRBase.h"
#include "../Base/Atol3/Atol3FRBase.h"
#include "AtolOnlineFRConstants.h"

//--------------------------------------------------------------------------------
template<class T>
class AtolOnlineFRBase : public T
{
public:
	AtolOnlineFRBase();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

protected:
	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Получить ключ модели для идентификации.
	virtual CAtolFR::TModelKey getModelKey(const QByteArray & aAnswer);

	/// Получить параметры печати.
	virtual bool getPrintingSettings();

	/// Запросить и вывести в лог критичные параметры ФР.
	virtual void processDeviceData();

	/// Получить номер смены.
	virtual int getSessionNumber();

	/// Установить параметры ФР.
	virtual bool setFRParameters();

	/// Печать фискального чека.
	virtual bool performFiscal(const QStringList & aReceipt, const SDK::Driver::SPaymentData & aPaymentData, quint32 * aFDNumber = nullptr);

	/// Получить фискальные теги по номеру документа.
	virtual bool getFiscalFields(quint32 aFDNumber, SDK::Driver::TFiscalPaymentData & aFPData, SDK::Driver::TComplexFiscalPaymentData & aPSData);

	/// Выполнить итерационный запрос фискальных тегов.
	TResult getFiscalTLVData(QByteArray & aData);

	/// Печать выплаты.
	virtual bool performEncashment(const QStringList & aReceipt, double aAmount);

	/// Обработка ответа на предыдущей команды. Автоисправление некоторых ошибок.
	virtual bool processAnswer(const QByteArray & aCommand, char aError);

	/// Проверить параметры налогов.
	virtual bool checkTaxes();

	/// Проверить параметры налога.
	virtual bool checkTax(SDK::Driver::TVAT aVAT, CFR::Taxes::SData & aData);

	/// Продажа.
	virtual bool sale(const SDK::Driver::SUnitData & aUnitData);

	/// Установить TLV-параметр.
	virtual bool setTLV(int aField, bool aForSale = false);

	/// Получить TLV-параметр.
	bool getTLV(int aField, QByteArray & aData, uchar aBlockNumber = 0);

	/// Установить флаги по ошибке в ответе.
	virtual void setErrorFlags(const QByteArray & aCommand, char aError);

	/// Получить Id принтера.
	virtual char getPrinterId();

	/// Выполнить функтор с ожиданием ответа без ошибок транспорта.
	TResult processDataWaiting(const std::function<TResult()> & aCommand);

	/// Софтварная перезагрузка.
	bool reboot();

	/// Версия ПО ФР, начиная с которой унифицирован порядок налоговых ставок.
	int mFRBuildUnifiedTaxes;
};

//--------------------------------------------------------------------------------
class Atol2OnlineFRBase: public AtolOnlineFRBase<Atol2FRBase>
{
	SET_SERIES("ATOL2Online")
};

//--------------------------------------------------------------------------------
class Atol3OnlineFRBase: public AtolOnlineFRBase<Atol3FRBase>
{
	SET_SERIES("ATOL3Online")
};

//--------------------------------------------------------------------------------
