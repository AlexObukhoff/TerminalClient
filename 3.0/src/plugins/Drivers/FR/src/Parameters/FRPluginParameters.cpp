/* @file Параметры плагинов для ФР. */

// Modules
#include "Hardware/Common/HardwareConstants.h"

// Project
#include "FRPluginParameters.h"
#include "FRPluginParameterTranslations.h"

using namespace SDK::Plugin;

namespace PrinterSettings = CHardware::Printer::Settings;
namespace PPT = PluginParameterTranslations;

//------------------------------------------------------------------------------
SPluginParameter setFiscalModeEnabled()
{
	return SPluginParameter(CHardware::FR::FiscalMode, SPluginParameter::Bool, true, PPT::FiscalMode, QString(), true);
}

//------------------------------------------------------------------------------
SPluginParameter setDocumentCap()
{
	return SPluginParameter(PrinterSettings::DocumentCap, false, PPT::DocumentCap, QString(), CHardware::Values::NoChange, QStringList()
		<< CHardware::Values::Use
		<< CHardware::Values::NotUse
		<< CHardware::Values::NoChange, false);
}

//------------------------------------------------------------------------------
SPluginParameter setAutoCloseSessionAbility()
{
	return SPluginParameter(CHardware::FR::CanAutoCloseSession, false, PPT::AutoCloseSessionAbility, QString(), CHardware::Values::Auto, QStringList() << CHardware::Values::Use << CHardware::Values::NotUse, true);
}

//------------------------------------------------------------------------------
SPluginParameter setFiscalChequeCreation()
{
	return SPluginParameter(CHardware::FR::FiscalChequeCreation, false, PPT::FiscalChequeCreation, QString(), CHardware::FR::Values::Adaptive, QStringList()
		<< CHardware::FR::Values::Adaptive
		<< CHardware::FR::Values::Discrete, false);
}

//------------------------------------------------------------------------------
