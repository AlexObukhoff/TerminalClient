/* @file Диспенсер купюр Puloon. */

#pragma once

// Modules
#include "Hardware/CashDevices/CCTalkDeviceBase.h"

// Project
#include "Hardware/Dispensers/PortDispenser.h"

//--------------------------------------------------------------------------------
namespace CSuzo
{
	/// Статус выдачи монет.
	struct SStatus
	{
		uchar Id;
		int remains;
		int paid;
		int unpaid;

		SStatus(): Id(0), remains(0), paid(0), unpaid(0) {}
		SStatus(char aId, int aRemains, int aPaid, int aUnpaid): Id(uchar(aId)), remains(uchar(aRemains)), paid(uchar(aPaid)), unpaid(uchar(aUnpaid)) {}
	};
}

//--------------------------------------------------------------------------------
class SuzoHopper : public CCTalkDeviceBase<PortDispenser>
{
	SET_SUBSERIES("Suzo")

public:
	SuzoHopper();

	/// Получить поддерживаемые типы протоколов.
	static QStringList getProtocolTypes();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

protected:
	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Сброс.
	virtual bool reset();

	/// Установить режим выдачи: 1 / набор.
	bool setSingleMode(bool aEnable);

	/// Активировать/деактивировать приём.
	bool setEnable(bool aEnabled);

	/// Получить статус выдачи монет.
	bool getDispensingStatus(CSuzo::SStatus & aStatus);

	/// Выдать.
	virtual void performDispense(int aUnit, int aItems);

	/// Режим выдачи монет.
	bool mSingleMode;
};

//--------------------------------------------------------------------------------
