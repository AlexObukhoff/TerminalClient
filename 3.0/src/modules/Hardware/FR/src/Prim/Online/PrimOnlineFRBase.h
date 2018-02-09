/* @file Онлайн ФР семейства ПРИМ. */

#pragma once

#include "../PrimFRBase.h"

//--------------------------------------------------------------------------------
class PrimOnlineFRBase : public PrimFRBase
{
	SET_SERIES("PrimOnline")

public:
	PrimOnlineFRBase();

protected:
	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Получить дату и время ФР.
	virtual QDateTime getDateTime();

	/// Запросить и вывести в лог критичные параметры ФР.
	void processDeviceData();

	/// Заполнить фискальные данные для ПФД.
	virtual void setFiscalData(CPrimFR::TData & aCommandData, CPrimFR::TDataList & aAdditionalAFDData, const SDK::Driver::SPaymentData & aPaymentData, int aReceiptSize);

	/// Печать Z отчета.
	virtual bool performZReport(bool aPrintDeferredReports);

	/// Выполнить Z-отчет.
	virtual TResult doZReport(bool aAuto);

	/// Открыть смену.
	virtual bool openSession();

	/// Получить проверочный код последнего фискального документа - номер КПК.
	virtual int getVerificationCode();

	/// Сформировать необязательное G-поле произвольного фискального документа (ПФД), содержащее фискальный реквизит
	CPrimFR::TData addFiscalField(int aX, int aY, int aFiscalField = 0, const QString & aData = "");
};

//--------------------------------------------------------------------------------
