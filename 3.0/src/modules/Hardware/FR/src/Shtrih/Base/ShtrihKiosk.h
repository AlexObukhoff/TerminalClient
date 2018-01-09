/* @file Штрих Киоск-ФР-К. */

#pragma once

#include "ShtrihRetractorFR.h"

//--------------------------------------------------------------------------------
class ShtrihKioskFRK : public ShtrihRetractorFR
{
	SET_SUBSERIES("Kiosk")

public:
	ShtrihKioskFRK()
	{
		mDeviceName = CShtrihFR::Models::CData()[CShtrihFR::Models::ID::ShtrihKioskFRK_2].name;
		mSupportedModels = QStringList() << mDeviceName;
		setConfigParameter(CHardware::Printer::PresenterEnable, true);
	}

protected:
	/// Вытолкнуть чек.
	virtual bool push()
	{
		return processCommand(CShtrihFR::Commands::Push, QByteArray(1, CShtrihFR::PushNoPresenter));
	}
};

//--------------------------------------------------------------------------------
