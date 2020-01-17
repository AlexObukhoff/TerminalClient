/* @file Плагин c драйверами фискальных регистраторов. */

// Modules
#include "Hardware/Plugins/CommonParameters.h"
#include "Hardware/Printers/PrinterPluginParameters.h"

// Project
#include "Parameters/FRPluginParameters.h"
#include "Parameters/FRPluginParameterTranslations.h"
#include "FRPlugin.h"

using namespace SDK::Plugin;
using namespace SDK::Driver;

namespace PrinterSettings = CHardware::Printer::Settings;
namespace PrinterValues = CHardware::Printer::Values;
namespace FRValues = CHardware::FR::Values;
namespace Values = CHardwareSDK::Values;
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
		<< setDocumentCap()
		<< setNullingSumInCash();
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
		<< setBackFeed(PPT::ForNonFiscalDocuments)
		<< setPrinterModel("Epson EU-422");
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
		<< setLeftReceiptTimeout()
		<< setPrinterModel("Custom VKP-80");
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
		<< setWeightSensorsEnabled()
		<< setNullingSumInCash();
}

//------------------------------------------------------------------------------
template <class T>
TParameterList ShtrihRetractorFRParameters(const QStringList & aModels)
{
	return ShtrihParameters<T>(aModels)
		<< setLeftReceiptAction(PrinterSettings::PreviousReceipt, true, false, Values::Auto)
		<< setLeftReceiptAction(PrinterSettings::NotTakenReceipt, true, false, Values::Auto)
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
		<< setLeftReceiptAction(PrinterSettings::PreviousReceipt, true, true, Values::Auto)
		<< setLeftReceiptAction(PrinterSettings::NotTakenReceipt, true, true, Values::Auto)
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
TParameterList PayFAParameters(const QStringList & aModels)
{
	return modifyPriority(ShtrihOnlineParameters<T>(aModels), EDetectingPriority::High)
		<< setLoopEnabled("", false)
		<< setPresentationLength()
		<< setRemoteSensor(true);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList PayVKP80FAParameters(const QStringList & aModels)
{
	return PayFAParameters<T>(aModels)
		<< setLeftReceiptTimeout(true);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList PayOnlineParameters(const QStringList & aModels)
{
	return PayFAParameters<T>(aModels)
		<< setLeftReceiptAction(PrinterSettings::PreviousReceipt, true, true, Values::Auto)
		<< setLeftReceiptAction(PrinterSettings::NotTakenReceipt, true, true, Values::Auto)
		<< setLeftReceiptTimeout(false)
		<< setPrinterModel(CPayPrinters::CModels().getNames(), CPayPrinters::Default);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList MStarTK2Parameters(const QStringList & aModels)
{
	return modifyPriority(ShtrihOnlineParameters<T>(aModels), EDetectingPriority::High)
		//<< setRemoteSensor(true)
		<< setLoopEnabled("", false)
		<< setPresentationLength();
}

//------------------------------------------------------------------------------
template <class T>
TParameterList AtolParameters(const QStringList & aModels, const QString & aDeviceType, const QString & aProtocol)
{
	return defaultParameters<T>(aModels, aDeviceType)
		<< setProtocol(aProtocol)
		<< setNullingSumInCash()
		<< setModifiedValues(CHardwareSDK::ProtocolName, "ATOL", ProtocolNames::FR::ATOL2);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList AtolLSParameters(const QStringList & aModels, const QString & aDeviceType, const QString & aProtocol)
{
	return AtolParameters<T>(aModels, aDeviceType, aProtocol)
		<< setLineSpacing(1, 9, 3, 3);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList AtolOnlineParameters(const QStringList & aModels, const QString & aDeviceType, const QString & aProtocol)
{
	return AtolLSParameters<T>(aModels, aDeviceType, aProtocol)
		<< setNotPrinting(true);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList PayParameters(const QStringList & aModels)
{
	return AtolLSParameters<T>(aModels, CComponents::FiscalRegistrator, ProtocolNames::FR::ATOL2);
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
TParameterList PaymasterParameters(const QStringList & aModels, const QString & aProtocol)
{
	return AtolLSParameters<T>(aModels, CComponents::FiscalRegistrator, aProtocol)
		<< setLoopEnabled(PPT::ForFiscalDocuments, false)
		<< setLeftReceiptAction(PrinterSettings::PreviousReceipt, true, true, PrinterValues::Retract, false, PPT::ForFiscalDocuments)
		<< setPresentationLength("", 2, 14)
		<< setNotPrinting()
		<< setPrinterModel(CAtolOnlinePrinters::CModels().getNames(), CAtolOnlinePrinters::Default)
		<< setRemoteSensor(true)
		<< setModifiedValues(CHardwareSDK::ModelName, "Sensis Paymaster", "Sensis Kaznachej")
		<< setJamSensorEnabled();
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
		<< setNotPrinting()
		<< setProtocol(ProtocolNames::FR::Kasbi)
		<< setPrinterModel(CKasbiPrinters::CModels().getNames(), CKasbiPrinters::Default);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList AFPParameters(const QStringList & aModels)
{
	return modifyPriority(defaultParameters<T>(aModels, CComponents::FiscalRegistrator), EDetectingPriority::High)
		<< setNullingSumInCash()
		<< setProtocol(ProtocolNames::FR::AFP);
}

//------------------------------------------------------------------------------
#define SINGLE_FR_PLUGIN(aClassName, aParameters, aName) COMMON_DRIVER(aClassName, std::bind(&aParameters<aClassName>, QStringList() << #aName))
#define COMMON_FR_PLUGIN(aClassName, aParameters)        COMMON_DRIVER(aClassName, std::bind(&aParameters<aClassName>, aClassName::getModelList()))

#define SINGLE_ATOL_PLUGIN(aClassName, aParameters, aName, aProtocol) COMMON_DRIVER(aClassName, std::bind(&aParameters<aClassName>, QStringList() << #aName, ProtocolNames::FR::aProtocol))
#define COMMON_ATOL_PLUGIN(aClassName, aDeviceType, aParameters, aProtocol) COMMON_DRIVER(aClassName, std::bind(&aParameters<aClassName>, aClassName::getModelList(), CComponents::aDeviceType, ProtocolNames::FR::aProtocol))
#define COMMON_ATOL2_PLUGIN(aClassName, aDeviceType, aParameters) COMMON_ATOL_PLUGIN(aClassName, aDeviceType, aParameters, ATOL2)
#define COMMON_ATOL3_PLUGIN(aClassName, aDeviceType, aParameters) COMMON_ATOL_PLUGIN(aClassName, aDeviceType, aParameters, ATOL3)

//------------------------------------------------------------------------------
BEGIN_REGISTER_PLUGIN
	COMMON_FR_PLUGIN(PrimFRBase,            PrimParameters)
	COMMON_FR_PLUGIN(PrimPresenterFRBase,   PresenterPrimParameters)
	COMMON_FR_PLUGIN(PrimEjectorFRBase,     EjectorPrimParameters)
	COMMON_FR_PLUGIN(PrimOnlineFRBase,      PrimParameters)
	COMMON_FR_PLUGIN(PrimOnlineFR68,        PrimParametersBase)
	SINGLE_FR_PLUGIN(PrimPresenterOnlineFR, PresenterPrimParameters, Iskra PRIM 21-FA)
	SINGLE_FR_PLUGIN(PrimEjectorOnlineFR,   EjectorPrimParameters,   Iskra PRIM 21-FA)

	COMMON_ATOL2_PLUGIN(AtolFR,       FiscalRegistrator, AtolLSParameters)
	COMMON_ATOL2_PLUGIN(AtolFRSingle, FiscalRegistrator, AtolParameters)
	COMMON_ATOL2_PLUGIN(AtolDP,       DocumentPrinter,   AtolLSParameters)
	COMMON_ATOL2_PLUGIN(AtolDPSingle, DocumentPrinter,   AtolParameters)

	SINGLE_FR_PLUGIN(PayVKP80,   PayVKP80Parameters,  PayVKP-80K)
	SINGLE_FR_PLUGIN(PayPPU700,  PayPPU700Parameters, PayPPU-700K)
	SINGLE_FR_PLUGIN(PayCTS2000, PayParameters,       PayCTS-2000K)

	COMMON_ATOL2_PLUGIN(Atol2OnlineFRBase, FiscalRegistrator, AtolOnlineParameters)
	COMMON_ATOL3_PLUGIN(Atol3OnlineFRBase, FiscalRegistrator, AtolOnlineParameters)
	SINGLE_ATOL_PLUGIN(Paymaster2,  PaymasterParameters, Sensis Kaznachej, ATOL2)
	SINGLE_ATOL_PLUGIN(Paymaster3,  PaymasterParameters, Sensis Kaznachej, ATOL3)

	COMMON_FR_PLUGIN(ShtrihSerialFR, ShtrihParameters)
	COMMON_FR_PLUGIN(ShtrihRetractorFR, ShtrihRetractorFRParameters)
	SINGLE_FR_PLUGIN(Yarus01K, Yarus01KParameters, Shtrih-M Yarus-01K)
	SINGLE_FR_PLUGIN(ShtrihKioskFRK, ShtrihKioskFRKParameters, Shtrih-M Kiosk-FR-K)
	SINGLE_FR_PLUGIN(VirtualShtrihFR, ShtrihParameters, NeoService)

	SINGLE_FR_PLUGIN(PayOnlineTCP,     PayOnlineParameters,  PayOnline-01-FA)
	SINGLE_FR_PLUGIN(PayOnlineSerial,  PayOnlineParameters,  PayOnline-01-FA)
	SINGLE_FR_PLUGIN(PayVKP80FATCP,    PayVKP80FAParameters, PayVKP-80K-FA)
	SINGLE_FR_PLUGIN(PayVKP80FASerial, PayVKP80FAParameters, PayVKP-80K-FA)
	COMMON_FR_PLUGIN(ShtrihOnlineTCPFR, ShtrihOnlineParameters)
	COMMON_FR_PLUGIN(ShtrihOnlineSerialFR, ShtrihOnlineSerialFRParameters)
	SINGLE_FR_PLUGIN(MStarTK2FR, MStarTK2Parameters,  Multisoft MStar-TK2)

	//COMMON_FR_PLUGIN(MStarPrinters, MStarParameters)
	COMMON_FR_PLUGIN(SparkFR, SparkParameters)
	SINGLE_FR_PLUGIN(OPOSMStarTUPK, OPOSFRParameters, Multisoft MStar-TUP-K)
	SINGLE_FR_PLUGIN(KasbiFRBase, KasbiParameters, Kasbi Terminal-FA)
	COMMON_FR_PLUGIN(AFPFR, AFPParameters)
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
