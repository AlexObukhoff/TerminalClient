/* @file ФР Пэй на протоколе Штрих. */

#pragma once

#include "../Retractor/ShtrihRetractorFRLite.h"

namespace CPayOnlineFR
{
	class CPrinterModels : public CDescription<char>
	{
	public:
		CPrinterModels()
		{
			append(  1, "Custom VKP-80");
			append(  2, "Custom TG-2460");
			append(  3, "Custom TG-2480");
			append(  4, "Citizen PPU-700");
			append(  5, "Citizen CT-S2000");
			append(  6, "Axiohm KPSL1200-00H");
			append(  7, "SANEI SK1-21");
			append(  8, "NCR F309");
			append(  9, "Custom VKP-80 44mm");
			append(100, "Virtual");
		}
	};

	static CPrinterModels PrinterModels;
}

//--------------------------------------------------------------------------------
template<class T>
class PayOnlineFR : public ShtrihRetractorFRLite<T>
{
public:
	PayOnlineFR();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

protected:
	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Запросить и вывести в лог критичные параметры ФР.
	virtual void processDeviceData();

	/// Снять Z-отчет.
	virtual bool execZReport(bool aAuto);
};

typedef PayOnlineFR<ShtrihOnlineFRBase<ShtrihTCPFRBase>> PayOnlineTCPFR;
typedef PayOnlineFR<ShtrihOnlineFRBase<ShtrihSerialFRBase>> PayOnlineSerialFR;

//--------------------------------------------------------------------------------
