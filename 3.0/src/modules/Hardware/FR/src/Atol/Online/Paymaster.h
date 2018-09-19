/* @file Онлайн ФР Сенсис Казначей на протоколе АТОЛ. */

#pragma once

#include "AtolOnlineFRBase.h"
#include "../Ejector/AtolVKP80BasedFR.h"

/// Константы Казначея.
namespace CPaymaster
{
	/// Ошибки короткого статуса.
	namespace ShortStatusError
	{
		const char PaperJam  = '\x20';    /// Замятие бумаги.
		const char Presenter = '\x40';    /// Только для PPU-700: Ошибка презентера или в презентере осталась бумага.
	}
}

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

	/// Распарсить флаги короткого статуса.
	virtual void parseShortStatusFlags(char aFlags, TStatusCodes & aStatusCodes);

	/// Установить параметры ФР.
	virtual bool setFRParameters();

	/// Войти в расширенный режим снятия Z-отчетов.
	virtual bool enterExtendedMode();

	/// Печать отложенных Z-отчетов.
	virtual bool printDeferredZReports();

	/// Печать Z отчета.
	virtual bool performZReport(bool aPrintDeferredReports);

	/// Включить/выключить режим непечати документов.
	virtual bool setNotPrintDocument(bool aEnabled, bool aZReport = false);

	/// Получить Id принтера.
	virtual char getPrinterId();
};

//--------------------------------------------------------------------------------
