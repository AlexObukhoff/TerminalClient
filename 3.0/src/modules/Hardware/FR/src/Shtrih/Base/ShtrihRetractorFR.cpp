/* @file ФР семейства Штрих с эжектором на COM-порту. */

// Modules
#include "Hardware/FR/FRStatusCodes.h"

// Project
#include "ShtrihRetractorFR.h"

//--------------------------------------------------------------------------------
ShtrihRetractorFR::ShtrihRetractorFR()
{
	mDeviceName = CShtrihFR::Models::RetractorDefault;
	mSupportedModels = getModelList();
	setConfigParameter(CHardware::Printer::RetractorEnable, true);
}

//--------------------------------------------------------------------------------
QStringList ShtrihRetractorFR::getModelList()
{
	using namespace CShtrihFR::Models;

	return CData().getModelList(CShtrihFR::TIds() << ID::ShtrihKioskFRK << ID::Yarus02K);
}

//--------------------------------------------------------------------------------
bool ShtrihRetractorFR::retract()
{
	return processCommand(CShtrihFR::Commands::Cut, QByteArray(1, CShtrihFR::FullCutting));
}

//--------------------------------------------------------------------------------
bool ShtrihRetractorFR::processPayout(double aAmount)
{
	if (!ShtrihSerialFR::processPayout(aAmount))
	{
		return false;
	}

	cut();

	return true;
}

//--------------------------------------------------------------------------------
