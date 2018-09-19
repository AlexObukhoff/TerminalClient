/* @file Плагин c драйверами принтеров. */

#include "Hardware/Plugins/CommonParameters.h"
#include "Hardware/Printers/PrinterPluginParameters.h"
#include "Hardware/Printers/PrinterDevices.h"

using namespace SDK::Plugin;
using namespace SDK::Driver;

namespace PrinterSettings = CHardware::Printer::Settings;
namespace PrinterValues = CHardware::Printer::Values;
namespace Values = CHardwareSDK::Values;

//------------------------------------------------------------------------------
template <class T>
IPlugin * CreatePlugin(IEnvironment * aEnvironment, const QString & aInstancePath)
{
	return new DevicePluginBase<T>(CComponents::Printer, aEnvironment, aInstancePath);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList singleParameters(const QString & aModel)
{
	return modifyPriority(createNamedList<T>(aModel), EDetectingPriority::Low);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList commonParameters(const QStringList & aModels)
{
	return modifyPriority(createNamedList<T>(aModels, CComponents::Printer), EDetectingPriority::Low);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList CitizenCBM1000IIParameters(const QString & aModel)
{
	return singleParameters<T>(aModel)
		<< setLineSpacing(40, 80, 50, 5);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList CitizenCTS2000Parameters(const QString & aModel)
{
	return singleParameters<T>(aModel)
		<< setLineSpacing(40, 80, 50, 5);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList CitizenCPP8001Parameters(const QString & aModel)
{
	return singleParameters<T>(aModel)
		<< setLineSpacing(45, 80, 55, 5);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList CustomParameters(const QStringList & aModels)
{
	return commonParameters<T>(aModels)
		<< setRemoteSensor(true)
		<< setLineSpacing(45, 85, 55, 5)
		<< setJamSensorEnabled();
}

//------------------------------------------------------------------------------
template <class T>
TParameterList CommonCustomVKP80Parameters(const QString & aModel)
{
	return singleParameters<T>(aModel)
		<< setRemoteSensor(true)
		<< setPresentationLength("", 2)
		<< setLeftReceiptTimeout()
		<< setLineSpacing(45, 85, 55, 5)
		<< setJamSensorEnabled();
}

//------------------------------------------------------------------------------
template <class T>
TParameterList CustomVKP80IIIParameters(const QString & aModel)
{
	return CommonCustomVKP80Parameters<T>(aModel)
		<< setLeftReceiptAction(PrinterSettings::PreviousAndNotTakenReceipts, true, true, PrinterValues::Retract);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList CustomVKP80Parameters(const QString & aModel)
{
	return CommonCustomVKP80Parameters<T>(aModel)
		<< setLoopEnabled("", false)
		<< setLeftReceiptAction(PrinterSettings::PreviousReceipt, true, true, Values::Auto)
		<< setLeftReceiptAction(PrinterSettings::NotTakenReceipt, true, true, Values::Auto)
		<< setCustomCodepage();
}

//------------------------------------------------------------------------------
template <class T>
TParameterList CitizenPPU700Parameters(const QString & aModel)
{
	return singleParameters<T>(aModel)
		<< setRemoteSensor(false)
		<< setLineSpacing(1, 40, 10, 5);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList EpsonEUT400Parameters(const QString & aModel)
{
	return singleParameters<T>(aModel)
		<< setRemoteSensor(true)
		<< setLineSpacing(20, 50, 30, 5)
		<< setLoopEnabled()
		<< setBackFeed();
}

//------------------------------------------------------------------------------
template <class T>
TParameterList CitizenPPU231Parameters(const QString & aModel)
{
	return singleParameters<T>(aModel)
		<< setRemoteSensor(true)
		<< setLineSpacing(40, 80, 50, 5)
		<< setFeedingFactor();
}

//------------------------------------------------------------------------------
template <class T>
TParameterList StarParameters(const QStringList & aModels)
{
	return commonParameters<T>(aModels)
		<< setLineSpacing(3, 4, 3, 1);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList EjectorStarParameters(const QStringList & aModels)
{
	return StarParameters<T>(aModels)
		<< setRemoteSensor(false)
		<< setLoopEnabled()
		<< setLeftReceiptAction(PrinterSettings::NotTakenReceipt, true, false, PrinterValues::Retract)
		<< setLeftReceiptTimeout();
}

//------------------------------------------------------------------------------
template <class T>
TParameterList SystemPrintersParameters(const QString & aModel)
{
	return modifyPriority(singleParameters<T>(aModel), EDetectingPriority::Fallback)
		<< setLineSpacing(60, 120, 100, 10)
		<< setFontSize(6, 18, 12, 1);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList SystemPrinterParameters(const QString & aModel)
{
	return SystemPrintersParameters<T>(aModel)
		<< setPaginationDisabled();
}

//------------------------------------------------------------------------------
template <class T>
TParameterList DefaultPOSPrinterParameters(const QString & aModel)
{
	return modifyPriority(singleParameters<T>(aModel), EDetectingPriority::Fallback);
}

//------------------------------------------------------------------------------
#define SINGLE_PRINTER_PLUGIN(aClassName, aParameters, aName) COMMON_DRIVER(aClassName, std::bind(&aParameters<aClassName>, #aName))
#define CUSTOM_PRINTER_PLUGIN(aClassName, aParameters, aName) COMMON_DRIVER(aClassName, std::bind(&aParameters<aClassName>, QStringList() << #aName))
#define COMMON_PRINTER_PLUGIN(aClassName, aParameters)        COMMON_DRIVER(aClassName, std::bind(&aParameters<aClassName>, aClassName::getModelList()))

//------------------------------------------------------------------------------
BEGIN_REGISTER_PLUGIN
	//SINGLE_PRINTER_PLUGIN(EpsonPrinter, Epson Printer, singleParameters) // Автоопределяется вместо Citizen PPU-700. Не протестирован в 3.0
	//SINGLE_PRINTER_PLUGIN(GeBe, GeBE Printer, singleParameters) // Не протестирован в 3.0
	//SINGLE_PRINTER_PLUGIN(PrimexNP2511, Primex Printer, singleParameters)   // Не протестирован в 3.0
	//SINGLE_PRINTER_PLUGIN(SwecoinPrinter, Swecoin Printer, singleParameters) // Не протестирован в 3.0

	SINGLE_PRINTER_PLUGIN(POSPrinter, DefaultPOSPrinterParameters, Default POS Printer)
	COMMON_PRINTER_PLUGIN(CustomPrinter, CustomParameters)
	CUSTOM_PRINTER_PLUGIN(CustomTG2480H, CustomParameters, Custom TG2480H)
	SINGLE_PRINTER_PLUGIN(CitizenCBM1000II, CitizenCBM1000IIParameters, Citizen CBM-1000II)
	SINGLE_PRINTER_PLUGIN(CitizenCPP8001, CitizenCPP8001Parameters, Citizen CPP-8001)
	SINGLE_PRINTER_PLUGIN(CitizenCTS2000, CitizenCTS2000Parameters, Citizen CTS-2000)
	SINGLE_PRINTER_PLUGIN(CustomVKP80, CustomVKP80Parameters, Custom VKP-80)
	SINGLE_PRINTER_PLUGIN(CustomVKP80III, CustomVKP80IIIParameters, Custom VKP-80 III)
	SINGLE_PRINTER_PLUGIN(CitizenPPU700, CitizenPPU700Parameters, Citizen PPU-700) // TODO: добавить параметры как у Custom VKP-80
	SINGLE_PRINTER_PLUGIN(CitizenPPU231, CitizenPPU231Parameters, Citizen PPU-231)
	SINGLE_PRINTER_PLUGIN(CitizenCTS310II, singleParameters, Citizen CT-S310II)
	SINGLE_PRINTER_PLUGIN(EpsonEUT400, EpsonEUT400Parameters, Epson EU-T400)

	COMMON_PRINTER_PLUGIN(StarPrinter, StarParameters)
	COMMON_PRINTER_PLUGIN(EjectorStarPrinter, EjectorStarParameters)
	COMMON_PRINTER_PLUGIN(AV268, commonParameters)
	SINGLE_PRINTER_PLUGIN(SystemPrinter, SystemPrinterParameters, System)
	SINGLE_PRINTER_PLUGIN(SunphorPOS58IV, SystemPrintersParameters, Sunphor POS58IV)
	SINGLE_PRINTER_PLUGIN(ICTGP83, SystemPrintersParameters, ICT GP83)
END_REGISTER_PLUGIN

//------------------------------------------------------------------------------
