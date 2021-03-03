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
template <class T, class T2>
struct SPrintersParameters {};

//------------------------------------------------------------------------------
template <class T>
struct SPrintersParameters<T, QString>
{
	TParameterList create(const QString & aModel)
	{
		return createNamedList<T>(aModel);
	}
};

//------------------------------------------------------------------------------
template <class T>
struct SPrintersParameters<T, QStringList>
{
	TParameterList create(const QStringList & aModels)
	{
		return createNamedList<T>(aModels, CComponents::Printer);
	}
};

//------------------------------------------------------------------------------
template <class T, class T2>
TParameterList printerParameters(const T2 & aModels)
{
	return modifyPriority(SPrintersParameters<T, T2>().create(aModels), EDetectingPriority::Low);
};

//------------------------------------------------------------------------------
template <class T, class T2>
TParameterList CitizenCBM1000IIParameters(const T2 & aModels)
{
	return printerParameters<T, T2>(aModels)
		<< setLineSpacing(40, 80, 50, 5);
}

//------------------------------------------------------------------------------
template <class T, class T2>
TParameterList CitizenCTS2000Parameters(const QString & aModels)
{
	return printerParameters<T, T2>(aModels)
		<< setLineSpacing(40, 80, 50, 5);
}

//------------------------------------------------------------------------------
template <class T, class T2>
TParameterList CitizenCPP8001Parameters(const T2 & aModels)
{
	return printerParameters<T, T2>(aModels)
		<< setLineSpacing(45, 80, 55, 5);
}

//------------------------------------------------------------------------------
template <class T, class T2>
TParameterList CustomParameters(const T2 & aModels)
{
	return printerParameters<T, T2>(aModels)
		<< setRemoteSensor(true)
		<< setLineSpacing(45, 85, 55, 5)
		<< setJamSensorEnabled();
}

//------------------------------------------------------------------------------
template <class T, class T2>
TParameterList CommonCustomVKP80Parameters(const T2 & aModels)
{
	return printerParameters<T, T2>(aModels)
		<< setRemoteSensor(true)
		<< setPresentationLength("", 2)
		<< setLeftReceiptTimeout()
		<< setLineSpacing(45, 85, 55, 5)
		<< setJamSensorEnabled();
}

//------------------------------------------------------------------------------
template <class T, class T2>
TParameterList CustomVKP80IIIParameters(const T2 & aModels)
{
	return CommonCustomVKP80Parameters<T, T2>(aModels)
		<< setLeftReceiptAction(PrinterSettings::PreviousAndNotTakenReceipts, true, true, PrinterValues::Retract);
}

//------------------------------------------------------------------------------
template <class T, class T2>
TParameterList CustomVKP80Parameters(const T2 & aModels)
{
	return CommonCustomVKP80Parameters<T, T2>(aModels)
		<< setLoopEnabled("", false)
		<< setLeftReceiptAction(PrinterSettings::PreviousReceipt, true, true, Values::Auto)
		<< setLeftReceiptAction(PrinterSettings::NotTakenReceipt, true, true, Values::Auto)
		<< setCustomCodepage();
}

//------------------------------------------------------------------------------
template <class T, class T2>
TParameterList CitizenPPU700Parameters(const T2 & aModels)
{
	return printerParameters<T, T2>(aModels)
		<< setRemoteSensor(false)
		<< setLineSpacing(42, 122, 52, 5);
}

//------------------------------------------------------------------------------
template <class T, class T2>
TParameterList Sam4sEpsonParameters(const T2 & aModels)
{
	return printerParameters<T, T2>(aModels)
		<< setLineSpacing(45, 75, 50, 5);
}

//------------------------------------------------------------------------------
template <class T, class T2>
TParameterList EpsonEUT400Parameters(const T2 & aModels)
{
	return printerParameters<T, T2>(aModels)
		<< setRemoteSensor(true)
		<< setLineSpacing(20, 50, 30, 5)
		<< setLoopEnabled()
		<< setBackFeed();
}

//------------------------------------------------------------------------------
template <class T, class T2>
TParameterList CitizenPPU231Parameters(const T2 & aModels)
{
	return printerParameters<T, T2>(aModels)
		<< setRemoteSensor(true)
		<< setLineSpacing(40, 80, 50, 5)
		<< setFeedingFactor();
}

//------------------------------------------------------------------------------
template <class T, class T2>
TParameterList StarParameters(const T2 & aModels)
{
	return printerParameters<T, T2>(aModels)
		<< setLineSpacing(3, 4, 3, 1);
}

//------------------------------------------------------------------------------
template <class T, class T2>
TParameterList EjectorStarParameters(const T2 & aModels)
{
	return StarParameters<T, T2>(aModels)
		<< setRemoteSensor(false)
		<< setLoopEnabled()
		<< setLeftReceiptAction(PrinterSettings::NotTakenReceipt, true, false, PrinterValues::Retract)
		<< setLeftReceiptTimeout();
}

//------------------------------------------------------------------------------
template <class T, class T2>
TParameterList SystemPrintersParameters(const T2 & aModels)
{
	return printerParameters<T, T2>(aModels)
		<< setLineSpacing(60, 120, 100, 10)
		<< setFontSize(6, 18, 12, 1);
}

//------------------------------------------------------------------------------
template <class T, class T2>
TParameterList SystemPrinterParameters(const T2 & aModels)
{
	return modifyPriority(SystemPrintersParameters<T, T2>(aModels), EDetectingPriority::Fallback)
		<< setPaginationDisabled()
		<< setLeftMargin();
}

//------------------------------------------------------------------------------
template <class T, class T2>
TParameterList DefaultPOSPrinterParameters(const T2 & aModels)
{
	return modifyPriority(printerParameters<T, T2>(aModels), EDetectingPriority::Fallback);
}

//------------------------------------------------------------------------------
#define SINGLE_PRINTER_PLUGIN(aClassName, aParameters, aName) COMMON_DRIVER(aClassName, std::bind(&aParameters<aClassName, QString>, #aName))
#define CUSTOM_PRINTER_PLUGIN(aClassName, aParameters, aName) COMMON_DRIVER(aClassName, std::bind(&aParameters<aClassName, QStringList>, QStringList() << #aName))
#define COMMON_PRINTER_PLUGIN(aClassName, aParameters)        COMMON_DRIVER(aClassName, std::bind(&aParameters<aClassName, QStringList>, aClassName::getModelList()))

//------------------------------------------------------------------------------
BEGIN_REGISTER_PLUGIN
	//SINGLE_PRINTER_PLUGIN(EpsonPrinter, Epson Printer, singleParameters) // Автоопределяется вместо Citizen PPU-700. Не протестирован в 3.0
	//SINGLE_PRINTER_PLUGIN(GeBe, GeBE Printer, singleParameters) // Не протестирован в 3.0
	//SINGLE_PRINTER_PLUGIN(PrimexNP2511, Primex Printer, singleParameters)   // Не протестирован в 3.0
	//SINGLE_PRINTER_PLUGIN(SwecoinPrinter, Swecoin Printer, singleParameters) // Не протестирован в 3.0

	SINGLE_PRINTER_PLUGIN(TSerialPOSPrinter, DefaultPOSPrinterParameters, Default POS Printer)
	COMMON_PRINTER_PLUGIN(SerialCustomPrinter, CustomParameters)
	COMMON_PRINTER_PLUGIN(SerialCustomPrinterIII, CustomParameters)
	CUSTOM_PRINTER_PLUGIN(SerialCustomTG2480H, CustomParameters, Custom TG2480H)
	CUSTOM_PRINTER_PLUGIN(SerialCustomTG2480HIII, CustomParameters, Custom TG2480HIII)
	CUSTOM_PRINTER_PLUGIN(LibUSBCustomTG2480H, CustomParameters, Custom TG2480H)
	//CUSTOM_PRINTER_PLUGIN(LibUSBCustomTG2480HIII, CustomParameters, Custom TG2480HIII)
	SINGLE_PRINTER_PLUGIN(SerialCitizenCBM1000II, CitizenCBM1000IIParameters, Citizen CBM-1000II)
	SINGLE_PRINTER_PLUGIN(CitizenCPP8001, CitizenCPP8001Parameters, Citizen CPP-8001)
	SINGLE_PRINTER_PLUGIN(SerialCitizenCTS2000, CitizenCTS2000Parameters, Citizen CTS-2000)
	SINGLE_PRINTER_PLUGIN(SerialCustomVKP80, CustomVKP80Parameters, Custom VKP-80)
	SINGLE_PRINTER_PLUGIN(LibUSBCustomVKP80, CustomVKP80Parameters, Custom VKP-80)
	SINGLE_PRINTER_PLUGIN(SerialCustomVKP80III, CustomVKP80IIIParameters, Custom VKP-80 III)
	//SINGLE_PRINTER_PLUGIN(LibUSBCustomVKP80III, CustomVKP80IIIParameters, Custom VKP-80 III)
	SINGLE_PRINTER_PLUGIN(SerialCitizenPPU700, CitizenPPU700Parameters, Citizen PPU-700) // TODO: добавить параметры как у Custom VKP-80
	SINGLE_PRINTER_PLUGIN(SerialCitizenPPU700II, CitizenPPU700Parameters, Citizen PPU-700II)
	SINGLE_PRINTER_PLUGIN(LibUSBCitizenPPU700II, CitizenPPU700Parameters, Citizen PPU-700II)
	SINGLE_PRINTER_PLUGIN(CitizenPPU231, CitizenPPU231Parameters, Citizen PPU-231)
	SINGLE_PRINTER_PLUGIN(CitizenCTS310II, printerParameters, Citizen CT-S310II)
	COMMON_PRINTER_PLUGIN(Sam4sEpsonSerialPrinter, Sam4sEpsonParameters)
	COMMON_PRINTER_PLUGIN(Sam4sEpsonTCPPrinter, Sam4sEpsonParameters)
	SINGLE_PRINTER_PLUGIN(EpsonEUT400, EpsonEUT400Parameters, Epson EU-T400)

	COMMON_PRINTER_PLUGIN(StarPrinter, StarParameters)
	COMMON_PRINTER_PLUGIN(EjectorStarPrinter, EjectorStarParameters)
	COMMON_PRINTER_PLUGIN(AV268, printerParameters)

	SINGLE_PRINTER_PLUGIN(SystemPrinter, SystemPrinterParameters, System)
	SINGLE_PRINTER_PLUGIN(SunphorPOS58IV, SystemPrintersParameters, Sunphor POS58IV)
	SINGLE_PRINTER_PLUGIN(ICTGP83, SystemPrintersParameters, ICT GP83)
	COMMON_PRINTER_PLUGIN(Sam4s, SystemPrintersParameters)
END_REGISTER_PLUGIN

//------------------------------------------------------------------------------
