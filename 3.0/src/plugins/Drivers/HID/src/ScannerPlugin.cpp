/* @file Плагин с драйверами сканеров. */

#include "Hardware/Plugins/CommonParameters.h"
#include "Hardware/Scanners/ScannerDevices.h"

using namespace SDK::Plugin;
using namespace SDK::Driver;

//--------------------------------------------------------------------------------
/// Конструктор плагина.
template <class T>
IPlugin * CreatePlugin(IEnvironment * aEnvironment, const QString & aInstanceName)
{
	return new DevicePluginBase<T>("Scanners", aEnvironment, aInstanceName);
}

//--------------------------------------------------------------------------------
TParameterList OPOSParameters(const QString & aModel)
{
	return createNamedList<OPOSMetrologicScanner>(aModel, QString("OPOS ") + CComponents::Scanner)
		// Удалять префикс полученных данных.
		<< SPluginParameter(CHardware::Scanner::Prefix, SPluginParameter::Bool, true, QT_TRANSLATE_NOOP("ScannerParameters", "ScannerParameters#prefix"), QString(), false);
}

//--------------------------------------------------------------------------------------
TParameterList SerialParameters(const QString & aModel)
{
	return createNamedList<SerialScanner>(aModel, QString("Serial ") + CComponents::Scanner);
}

//--------------------------------------------------------------------------------------
TParameterList USBParameters(const QStringList & aModels)
{
	return createNamedList<USBScanner>(aModels, QString("USB ") + CComponents::Scanner);
}

//--------------------------------------------------------------------------------
BEGIN_REGISTER_PLUGIN
	COMMON_DRIVER(OPOSMetrologicScanner, std::bind(&OPOSParameters, "Metrologic"))
	COMMON_DRIVER(SerialScanner, std::bind(&SerialParameters, "Generic"))
	COMMON_DRIVER(USBScanner, std::bind(&USBParameters, USBScanner::getModelList()))

	//SINGLE_SERIAL_HID_PLUGIN(ACR120, ACR120, Native)
END_REGISTER_PLUGIN

//--------------------------------------------------------------------------------
