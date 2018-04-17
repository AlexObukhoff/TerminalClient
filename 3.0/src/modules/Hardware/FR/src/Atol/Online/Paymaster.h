/* @file Онлайн ФР Сенсис Казначей на протоколе АТОЛ. */

#pragma once

#include "AtolOnlineFRBase.h"
#include "../Ejector/AtolVKP80BasedFR.h"

//--------------------------------------------------------------------------------
typedef AtolVKP80BasedFR<AtolOnlineFRBase> TPaymaster;

class Paymaster : public TPaymaster
{
	SET_SUBSERIES("Paymaster")

public:
	Paymaster();

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);

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

	/// Получить Id принтера.
	virtual char getPrinterId();
};

//--------------------------------------------------------------------------------
