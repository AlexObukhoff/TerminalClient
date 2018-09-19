/* @file Онлайн ФР семейства ПРИМ. */

#pragma once

#include "../PrimFRBase.h"

//--------------------------------------------------------------------------------
class PrimOnlineFRBase : public PrimFRBase
{
	SET_SERIES("PrimOnline")

public:
	PrimOnlineFRBase();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

protected:
	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Проверить типы оплаты.
	bool checkPayTypes();

	/// Получить статус;
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Получить дату и время ФР.
	virtual QDateTime getDateTime();

	/// Запросить и вывести в лог критичные параметры ФР.
	void processDeviceData();

	/// Печать фискального чека.
	virtual bool performFiscal(const QStringList & aReceipt, const SDK::Driver::SPaymentData & aPaymentData, SDK::Driver::TFiscalPaymentData & aFPData, SDK::Driver::TComplexFiscalPaymentData & aPSData);

	/// Заполнить фискальные данные для ПФД.
	virtual void setFiscalData(CPrimFR::TData & aCommandData, CPrimFR::TDataList & aAdditionalAFDData, const SDK::Driver::SPaymentData & aPaymentData, int aReceiptSize);

	/// Установить данные платежа.
	void setFPData(SDK::Driver::TFiscalPaymentData & aFPData, const CFR::STLV & aTLV);

	/// Печать Z отчета.
	virtual bool performZReport(bool aPrintDeferredReports);

	/// Выполнить Z-отчет.
	virtual TResult doZReport(bool aAuto);

	/// Открыть смену.
	virtual bool openSession();

	/// Обработка ответа предыдущей команды. Автоисправление некоторых ошибок.
	virtual bool processAnswer(char aError);

	/// Получить проверочный код последнего фискального документа - номер КПК.
	virtual int getVerificationCode();

	/// Сформировать необязательное G-поле произвольного фискального документа (ПФД), содержащее фискальный реквизит.
	CPrimFR::TData addFiscalField(int aX, int aY, int aFont, int aFiscalField = 0, const QString & aData = "");

	/// Получить данные регистрации.
	bool getRegTLVData(int aField);
	bool getRegTLVData(int aField, uchar & aData);

	/// Шрифт для ПФД.
	int mAFDFont;
};

//--------------------------------------------------------------------------------
