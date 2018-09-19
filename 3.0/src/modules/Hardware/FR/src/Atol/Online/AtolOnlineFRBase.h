/* @file Онлайн ФР семейства АТОЛ. */

#pragma once

// Modules
#include "Hardware/Common/TCPDeviceBase.h"
#include "Hardware/Printers/PortPrinterBase.h"

// Project
#include "Hardware/FR/AtolOnlinePrinters.h"
#include "../Base/AtolFRBase.h"
#include "AtolOnlineFRConstants.h"

//--------------------------------------------------------------------------------
class AtolOnlineFRBase : public AtolFRBase
{
	SET_SERIES("AtolOnline")

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
	virtual bool performFiscal(const QStringList & aReceipt, const SDK::Driver::SPaymentData & aPaymentData, SDK::Driver::TFiscalPaymentData & aFPData, SDK::Driver::TComplexFiscalPaymentData & aPSData);

	/// Печать выплаты.
	virtual bool performEncashment(const QStringList & aReceipt, double aAmount);

	/// Обработка ответа на предыдущей команды. Автоисправление некоторых ошибок.
	virtual bool processAnswer(const QByteArray & aCommand, char aError);

	/// Проверить параметры налогов.
	virtual bool checkTaxes();

	/// Проверить параметры налога.
	virtual bool checkTax(SDK::Driver::TVAT aVAT, const CFR::Taxes::SData & aData);

	/// Продажа.
	virtual bool sale(const SDK::Driver::SUnitData & aUnitData);

	/// Установить TLV-параметр.
	virtual bool setTLV(int aField, bool aForSale = false);

	/// Получить TLV-параметр.
	bool getTLV(int aField, QByteArray & aData, uchar aBlockNumber = 0);

	/// Установить флаги по ошибке в ответе.
	virtual void setErrorFlags(char aError, const QByteArray & aCommand);

	/// Получить Id принтера.
	virtual char getPrinterId();

	/// Софтварная перезагрузка.
	bool reboot();

	/// Версия ПО ФР, начиная с которой унифицирован порядок налоговых ставок.
	int mFRBuildUnifiedTaxes;
};

//--------------------------------------------------------------------------------
