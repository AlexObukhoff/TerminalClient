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
	return SPluginParameter(PrinterSettings::DocumentCap, false, PPT::DocumentCap, QString(), CHardwareSDK::Values::Auto, QStringList()
		<< CHardwareSDK::Values::Use
		<< CHardwareSDK::Values::NotUse
		<< CHardwareSDK::Values::Auto, false);
}

//------------------------------------------------------------------------------
SPluginParameter setAutoCloseSessionAbility()
{
	return SPluginParameter(CHardware::FR::CanAutoCloseSession, false, PPT::AutoCloseSessionAbility, QString(), CHardwareSDK::Values::Auto, QStringList() << CHardwareSDK::Values::Use << CHardwareSDK::Values::NotUse, true);
}

//------------------------------------------------------------------------------
SPluginParameter setFiscalChequeCreation()
{
	return SPluginParameter(CHardware::FR::FiscalChequeCreation, false, PPT::FiscalChequeCreation, QString(), CHardware::FR::Values::Adaptive, QStringList()
		<< CHardware::FR::Values::Adaptive
		<< CHardware::FR::Values::Discrete, false);
}

//------------------------------------------------------------------------------
SPluginParameter setSessionOpeningTime()
{
	return SPluginParameter(CHardware::FR::SessionOpeningTime, false, PPT::SessionOpeningTime, QString(), QString(), QStringList(), true);
}

//------------------------------------------------------------------------------
SPluginParameter setNotPrinting(bool aVisible)
{
	return SPluginParameter(CHardwareSDK::FR::WithoutPrinting, false, PPT::NotPrinting, QString(), CHardwareSDK::Values::Auto, QStringList() << CHardwareSDK::Values::Use << CHardwareSDK::Values::NotUse, !aVisible);
}

//------------------------------------------------------------------------------
SPluginParameter setPrinterModel(const QStringList & aModels, const QString & aDefault)
{
	return SPluginParameter(CHardware::FR::PrinterModel, false, PPT::PrinterModel, QString(), aDefault, aModels);
}

//------------------------------------------------------------------------------
SPluginParameter setPrinterModel(const QString & aModel)
{
	return setPrinterModel(QStringList() << aModel, aModel);
}

//------------------------------------------------------------------------------
SPluginParameter setNullingSumInCash()
{
	return SPluginParameter(CHardwareSDK::FR::NullingSumInCash, false, PPT::NullingSumInCash, QString(), CHardwareSDK::Values::NotUse, QStringList()
		<< CHardwareSDK::Values::Use
		<< CHardwareSDK::Values::NotUse
		<< CHardwareSDK::Values::Auto);
}

//------------------------------------------------------------------------------
