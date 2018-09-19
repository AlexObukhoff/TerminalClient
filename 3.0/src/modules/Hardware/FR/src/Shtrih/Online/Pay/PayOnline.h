/* @file ФР PayOnline-01-FA на протоколе Штрих. */

#pragma once

#include "PayFRBase.h"

//--------------------------------------------------------------------------------
template<class T>
class PayOnline : public PayFRBase<T>
{
	SET_SUBSERIES("PayOnline")

public:
	PayOnline();

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);

protected:
	/// Запросить и вывести в лог критичные параметры ФР.
	virtual void processDeviceData();

	/// Включить/выключить режим непечати документов.
	virtual bool setNotPrintDocument(bool aEnabled, bool aZReport = false);

	/// Получить таймаут незабранного чека.
	virtual uchar getLeftReceiptTimeout();

	/// Вытолкнуть чек.
	virtual bool push();
};

typedef PayOnline<ShtrihOnlineFRBase<ShtrihTCPFRBase>> PayOnlineTCP;
typedef PayOnline<ShtrihOnlineFRBase<ShtrihSerialFRBase>> PayOnlineSerial;

//--------------------------------------------------------------------------------
