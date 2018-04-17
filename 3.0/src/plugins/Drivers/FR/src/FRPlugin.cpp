/* @file Плагин c драйверами фискальных регистраторов. */

// Modules
#include "Hardware/Plugins/CommonParameters.h"
#include "Hardware/Printers/PrinterPluginParameters.h"

// Project
#include "FRPlugin.h"
#include "Parameters/FRPluginParameters.h"
#include "Parameters/FRPluginParameterTranslations.h"

using namespace SDK::Plugin;
using namespace SDK::Driver;

namespace PrinterSettings = CHardware::Printer::Settings;
namespace PrinterValues = CHardware::Printer::Values;
namespace FRValues = CHardware::FR::Values;
namespace Values = CHardware::Values;
namespace PPT = PluginParameterTranslations;

//------------------------------------------------------------------------------
template <class T>
IPlugin * CreatePlugin(IEnvironment * aEnvironment, const QString & aInstancePath)
{
	return new FRPluginBase<T>(aEnvironment, aInstancePath);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList defaultParameters(const QStringList & aModels, const QString & aDefaultModelName)
{
	return modifyPriority(createNamedList<T>(aModels, aDefaultModelName), EDetectingPriority::Low)
		<< setFiscalModeEnabled()
		<< setSessionOpeningTime();
}

//------------------------------------------------------------------------------
template <class T>
TParameterList PrimParametersBase(const QStringList & aModels)
{
	return defaultParameters<T>(aModels, CComponents::FiscalRegistrator)
		<< setProtocol(ProtocolNames::FR::PRIM)
		<< setAutoCloseSessionAbility()
		<< setDocumentCap();
}

//------------------------------------------------------------------------------
template <class T>
TParameterList PrimParameters(const QStringList & aModels)
{
	return PrimParametersBase<T>(aModels)
		<< setLineSpacing(15, 55, 25, 5);
}


//------------------------------------------------------------------------------
template <class T>
TParameterList PresenterPrimParameters(const QStringList & aModels)
{
	return PrimParameters<T>(aModels)
		<< setLoopEnabled()
		<< setBackFeed(PPT::ForNonFiscalDocuments);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList EjectorPrimParameters(const QStringList & aModels)
{
	return PrimParametersBase<T>(aModels)
		<< setLineSpacing(15, 55, 25, 5, PPT::ForNonFiscalDocuments)
		<< setLoopEnabled(PPT::ForNonFiscalDocuments, false)
		<< setRemoteSensor(true)
		<< setPresentationLength()
		<< setLeftReceiptAction(PrinterSettings::PreviousReceipt, true, true, PrinterValues::Retract, false)
		<< setLeftReceiptAction(PrinterSettings::NotTakenReceipt, true, true, PrinterValues::Retract, false)
		<< setLeftReceiptTimeout();
}

//------------------------------------------------------------------------------
template <class T>
TParameterList ShtrihParameters(const QStringList & aModels)
{
	QVariantMap modelNames;
	modelNames.insert("Shtrih-M Shtrih FR-F", "Shtrih-M Shtrih-FR-F");
	modelNames.insert("Shtrih-M Shtrih FR-K", "Shtrih-M Shtrih-FR-K");
	modelNames.insert("Shtrih-M Shtrih Mini-FR-K",  "Shtrih-M Mini-FR-K");
	modelNames.insert("Shtrih-M Shtrih Combo-FR-K", "Shtrih-M Combo-FR-K");
	modelNames.insert("Shtrih-M Shtrih Kiosk-FR-K", "Shtrih-M Kiosk-FR-K");
	modelNames.insert("Shtrih-M Shtrih Light-FR-K", "Shtrih-M Light-FR-K");
	modelNames.insert("Shtrih-M Shtrih FR-F-Kazakhstan", "Shtrih-M Shtrih-FR-F Kazakhstan");
	modelNames.insert("Shtrih-M Shtrih Kiosk-FR-K based on Nippon NP-325", "Shtrih-M Kiosk-FR-K (Nippon)");

	return defaultParameters<T>(aModels, CComponents::FiscalRegistrator)
		<< setProtocol(ProtocolNames::FR::Shtrih)
		<< setModifiedValues(CHardwareSDK::ModelName, modelNames)
		<< setWeightSensorsEnabled();
}

//------------------------------------------------------------------------------
template <class T>
TParameterList ShtrihRetractorFRParameters(const QStringList & aModels)
{
	return ShtrihParameters<T>(aModels)
		<< setLeftReceiptAction(PrinterSettings::PreviousReceipt, true, false, Values::NoChange)
		<< setLeftReceiptAction(PrinterSettings::NotTakenReceipt, true, false, Values::NoChange)
		<< setLeftReceiptTimeout();
}

//------------------------------------------------------------------------------
template <class T>
TParameterList Yarus01KParameters(const QStringList & aModels)
{
	return ShtrihRetractorFRParameters<T>(aModels)
		<< setPresentationLength();
}

//------------------------------------------------------------------------------
template <class T>
TParameterList ShtrihKioskFRKParameters(const QStringList & aModels)
{
	return ShtrihParameters<T>(aModels)
		<< setLeftReceiptAction(PrinterSettings::PreviousReceipt, true, true, Values::NoChange)
		<< setLeftReceiptAction(PrinterSettings::NotTakenReceipt, true, true, Values::NoChange)
		<< setLeftReceiptTimeout();
}

//------------------------------------------------------------------------------
template <class T>
TParameterList ShtrihOnlineParameters(const QStringList & aModels)
{
	return ShtrihParameters<T>(aModels)
		<< setNotPrinting();
}

//------------------------------------------------------------------------------
template <class T>
TParameterList ShtrihOnlineSerialFRParameters(const QStringList & aModels)
{
	return modifyPriority(ShtrihOnlineParameters<T>(aModels), EDetectingPriority::High);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList PayOnlineParameters(const QStringList & aModels)
{
	return modifyPriority(ShtrihOnlineParameters<T>(aModels), EDetectingPriority::High)
		<< setLoopEnabled("", false)
		<< setPresentationLength()
		<< setLeftReceiptTimeout(true)
		<< setRemoteSensor(true);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList MStarTK2Parameters(const QStringList & aModels)
{
	return modifyPriority(ShtrihOnlineParameters<T>(aModels), EDetectingPriority::High)
		//<< setRemoteSensor(true)
		<< setLoopEnabled("", false)
		<< setPresentationLength()
		<< setLeftReceiptTimeout(true);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList AtolParameters(const QStringList & aModels, const QString & aDeviceType)
{
	return defaultParameters<T>(aModels, aDeviceType)
		<< setProtocol(ProtocolNames::FR::ATOL);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList AtolLSParameters(const QStringList & aModels, const QString & aDeviceType)
{
	return AtolParameters<T>(aModels, aDeviceType)
		<< setLineSpacing(1, 9, 3, 3);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList PayParameters(const QStringList & aModels)
{
	return AtolLSParameters<T>(aModels, CComponents::FiscalRegistrator);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList PayVKP80Parameters(const QStringList & aModels)
{
	return PayParameters<T>(aModels)
		<< setLoopEnabled(PPT::ForFiscalDocuments, false)
		<< setLeftReceiptAction(PrinterSettings::PreviousReceipt, true, true, PrinterValues::Retract, false, PPT::ForFiscalDocuments)
		<< setPresentationLength("", 2, 14);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList PayPPU700Parameters(const QStringList & aModels)
{
	return PayParameters<T>(aModels)
		<< SPluginParameter(CHardware::FR::EjectorParameters, false, PPT::EjectorParameters, QString(), FRValues::LoopAndPushNotTakenOnTimeout, QStringList()
			<< FRValues::LoopAndPushNotTakenOnTimeout
			<< FRValues::NoLoopAndPushNotTakenOnTimeout
			<< FRValues::NoLoopAndRetractNotTakenOnTimeout, false)
		<< setLeftReceiptTimeout();
}

//------------------------------------------------------------------------------
template <class T>
TParameterList AtolOnlineParameters(const QStringList & aModels, const QString & aDeviceType)
{
	return AtolLSParameters<T>(aModels, aDeviceType)
		<< setNotPrinting();
}

//------------------------------------------------------------------------------
template <class T>
TParameterList PaymasterParameters(const QStringList & aModels)
{
	QVariantMap modelNames;
	modelNames.insert("Sensis Paymaster", "Sensis Kaznachej");

	return PayVKP80Parameters<T>(aModels)
		<< setNotPrinting()
		<< setPaymasterPrinterModel()
		<< setRemoteSensor(true)
		<< setModifiedValues(CHardwareSDK::ModelName, modelNames);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList OPOSFRParameters(const QStringList & aModels)
{
	return defaultParameters<T>(aModels, CComponents::FiscalRegistrator)
		<< setAutoCloseSessionAbility();
}

//------------------------------------------------------------------------------
template <class T>
TParameterList SparkParameters(const QStringList & aModels)
{
	return defaultParameters<T>(aModels, CComponents::FiscalRegistrator)
		<< setProtocol(ProtocolNames::FR::SPARK)
		<< setLeftReceiptAction(PrinterSettings::PreviousReceipt, true, false, PrinterValues::Retract)
		<< setLeftReceiptAction(PrinterSettings::NotTakenReceipt, true, false, PrinterValues::Retract)
		<< setLeftReceiptTimeout()
		<< setFiscalChequeCreation();
}

//------------------------------------------------------------------------------
template <class T>
TParameterList MStarParameters(const QStringList & aModels)
{
	return defaultParameters<T>(aModels, CComponents::FiscalRegistrator)
		<< setProtocol(ProtocolNames::FR::Incotex);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList KasbiParameters(const QStringList & aModels)
{
	return defaultParameters<T>(aModels, CComponents::FiscalRegistrator)
		<< setProtocol(ProtocolNames::FR::Kasbi);
}

//------------------------------------------------------------------------------
#define SINGLE_FR_PLUGIN(aClassName, aParameters, aName) COMMON_DRIVER(aClassName, std::bind(&aParameters<aClassName>, QStringList() << #aName))
#define COMMON_FR_PLUGIN(aClassName, aParameters)        COMMON_DRIVER(aClassName, std::bind(&aParameters<aClassName>, aClassName::getModelList()))

#define ATOL_FR_PLUGIN(aClassName, aDeviceType, aParameters) COMMON_DRIVER(aClassName, std::bind(&aParameters<aClassName>, aClassName::getModelList(), CComponents::aDeviceType))

//------------------------------------------------------------------------------
BEGIN_REGISTER_PLUGIN
	COMMON_FR_PLUGIN(PrimFRBase, PrimParameters)
	COMMON_FR_PLUGIN(PrimPresenterFRBase, PresenterPrimParameters)
	COMMON_FR_PLUGIN(PrimEjectorFRBase,   EjectorPrimParameters)
	COMMON_FR_PLUGIN(PrimOnlineFRBase,    PrimParameters)
	COMMON_FR_PLUGIN(PrimEjectorOnlineFR, EjectorPrimParameters)

	ATOL_FR_PLUGIN(AtolFR,       FiscalRegistrator, AtolLSParameters)
	ATOL_FR_PLUGIN(AtolFRSingle, FiscalRegistrator, AtolParameters)
	ATOL_FR_PLUGIN(AtolDP,       DocumentPrinter,   AtolLSParameters)
	ATOL_FR_PLUGIN(AtolDPSingle, DocumentPrinter,   AtolParameters)

	SINGLE_FR_PLUGIN(PayVKP80,   PayVKP80Parameters,  PayVKP-80K)
	SINGLE_FR_PLUGIN(PayPPU700,  PayPPU700Parameters, PayPPU-700K)
	SINGLE_FR_PLUGIN(PayCTS2000, PayParameters,       PayCTS-2000K)

	ATOL_FR_PLUGIN(AtolOnlineFRBase, FiscalRegistrator, AtolOnlineParameters)
	SINGLE_FR_PLUGIN(Paymaster,  PaymasterParameters, Sensis Kaznachej)

	COMMON_FR_PLUGIN(ShtrihSerialFR, ShtrihParameters)
	COMMON_FR_PLUGIN(ShtrihRetractorFR, ShtrihRetractorFRParameters)
	SINGLE_FR_PLUGIN(Yarus01K, Yarus01KParameters, Shtrih-M Yarus-01K)
	SINGLE_FR_PLUGIN(ShtrihKioskFRK, ShtrihKioskFRKParameters, Shtrih-M Kiosk-FR-K)
	SINGLE_FR_PLUGIN(VirtualShtrihFR, ShtrihParameters, NeoService)

	COMMON_FR_PLUGIN(PayOnlineTCPFR, PayOnlineParameters)
	COMMON_FR_PLUGIN(PayOnlineSerialFR, PayOnlineParameters)
	COMMON_FR_PLUGIN(ShtrihOnlineTCPFR, ShtrihOnlineParameters)
	COMMON_FR_PLUGIN(ShtrihOnlineSerialFR, ShtrihOnlineSerialFRParameters)
	SINGLE_FR_PLUGIN(MStarTK2FR, MStarTK2Parameters,  Multisoft MStar-TK2)

	//COMMON_FR_PLUGIN(MStarPrinters, MStarParameters)
	COMMON_FR_PLUGIN(SparkFR, SparkParameters)
	SINGLE_FR_PLUGIN(OPOSMStarTUPK, OPOSFRParameters, Multisoft MStar-TUP-K)
	SINGLE_FR_PLUGIN(KasbiFRBase, KasbiParameters, Kasbi Terminal-FA)
END_REGISTER_PLUGIN

//------------------------------------------------------------------------------
template <class T>
FRPluginBase<T>::FRPluginBase(IEnvironment * aEnvironment, const QString & aInstancePath) :
	DevicePluginBase<T>("FR", aEnvironment, aInstancePath)
{
}

//------------------------------------------------------------------------------
template <class T>
void FRPluginBase<T>::setConfiguration(const QVariantMap & aParameters)
{
	QVariantMap parameters(aParameters);
	addConfiguration<T::TSeriesType>(parameters);

	//фискальный режим - по умолчанию включаем
	if (!parameters.contains(CHardware::FR::FiscalMode))
	{
		parameters.insert(CHardware::FR::FiscalMode, true);
	}

	T::setDeviceConfiguration(parameters);
}

//------------------------------------------------------------------------------
