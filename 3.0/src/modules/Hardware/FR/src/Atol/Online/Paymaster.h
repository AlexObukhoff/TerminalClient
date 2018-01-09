/* @file Онлайн ФР Сенсис Казначей на протоколе АТОЛ. */

#pragma once

#include "AtolOnlineFRBase.h"
#include "../Ejector/AtolVKP80BasedFR.h"

//--------------------------------------------------------------------------------
namespace CPaymaster
{
	class CPrinterModels: public CDescription<char>
	{
	public:
		CPrinterModels()
		{
			append( 1, "ATOL");
			append( 2, "Custom VKP-80");
			append( 3, "Citizen PPU-700");
			append( 4, "Citizen CT-S2000");
			append( 5, "Custom TG-2480");
			append( 6, "Epson");
			append( 7, "Memory");
			append( 8, "Custom VKP-80SX");
			append( 9, "SNBC BT-080");
			append(10, "SNBC BK-T680");
		}
	};

	static CPrinterModels PrinterModels;
}

//--------------------------------------------------------------------------------
typedef AtolVKP80BasedFR<AtolOnlineFRBase> TPaymaster;

class Paymaster : public TPaymaster
{
	SET_SUBSERIES("Paymaster")

public:
	Paymaster();

protected:
	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Запросить и вывести в лог критичные параметры ФР.
	virtual void processDeviceData();

	/// Установить параметры ФР.
	virtual bool setFRParameters();

	/// Войти в расширенный режим снятия Z-отчетов.
	virtual bool enterExtendedMode();

	/// Печать отложенных Z-отчетов.
	virtual bool printDeferredZReports();

	/// Печать Z отчета.
	virtual bool performZReport(bool aPrintDeferredReports);
};

//--------------------------------------------------------------------------------
