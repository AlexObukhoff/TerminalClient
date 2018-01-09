/* @file Плагин c драйверами сторожевых таймеров. */

#include "Hardware/Plugins/CommonParameters.h"
#include "Hardware/Watchdogs/WatchdogDevices.h"

using namespace SDK::Plugin;
using namespace SDK::Driver;

//------------------------------------------------------------------------------
template <class T>
IPlugin * CreatePlugin(IEnvironment * aEnvironment, const QString & aInstancePath)
{
	return new DevicePluginBase<T>("Watchdogs", aEnvironment, aInstancePath);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList defaultParameters(const QString & aModel)
{
	return modifyPriority(createNamedList<T>(aModel), EDetectingPriority::VeryHigh);
}

//------------------------------------------------------------------------------
SPluginParameter addSensor(const QString & aName, const QString & aTranslation)
{
	return SPluginParameter(aName, false, aTranslation, QString(), CHardware::Values::Auto, QStringList() << CHardware::Values::Use << CHardware::Values::NotUse, false);
}

//------------------------------------------------------------------------------
SPluginParameter addSensorAction(const QString & aName, const QString & aTranslation)
{
	return SPluginParameter(aName, false, aTranslation, QString(), CHardware::Values::NotUse, QStringList()
		<< CHardware::Values::NotUse
		<< CHardware::Watchdog::Sensor::ActionValue::LockTerminal
		<< CHardware::Watchdog::Sensor::ActionValue::EnterServiceMenu, false);
}

#define ADD_SENSOR(aName, aSensorTranslation, aSensorActionTranslation) addSensor(CHardware::Watchdog::Sensor::aName, aSensorTranslation)/* \
	<< addSensorAction(CHardware::Watchdog::Sensor::Action::aName, aSensorActionTranslation)*/

//------------------------------------------------------------------------------
QVector<SPluginParameter> AlarmParameters(const QString & aModel)
{
	return defaultParameters<Alarm>(aModel)
		<< ADD_SENSOR(LowerUnit, QT_TRANSLATE_NOOP("WatchdogParameters", "WatchdogParameters#lower_unit_sensor"), QT_TRANSLATE_NOOP("WatchdogParameters", "WatchdogParameters#lower_unit_sensor_action"))
		<< ADD_SENSOR(UpperUnit, QT_TRANSLATE_NOOP("WatchdogParameters", "WatchdogParameters#upper_unit_sensor"), QT_TRANSLATE_NOOP("WatchdogParameters", "WatchdogParameters#upper_unit_sensor_action"))
		<< ADD_SENSOR(Safe,      QT_TRANSLATE_NOOP("WatchdogParameters", "WatchdogParameters#safe_unit_sensor"),  QT_TRANSLATE_NOOP("WatchdogParameters", "WatchdogParameters#safe_unit_sensor_action"));
}

//------------------------------------------------------------------------------
#define WD_PLUGIN_WITH_PARAMETERS(aClassName, aName, aParameters) COMMON_DRIVER_WITH_PARAMETERS(aClassName, &CreatePlugin<aClassName>, std::bind(&aParameters, #aName))
#define WD_PLUGIN(aName) COMMON_DRIVER(aName, std::bind(&defaultParameters<aName>, #aName))

BEGIN_REGISTER_PLUGIN
	WD_PLUGIN_WITH_PARAMETERS(Alarm, Alarm, AlarmParameters)
	WD_PLUGIN(STOD)
	WD_PLUGIN(OSMP)
	WD_PLUGIN(OSMP2)
	WD_PLUGIN_WITH_PARAMETERS(OSMP25, OSMP2.5, defaultParameters<OSMP25>)
	WD_PLUGIN(LDog)
	WD_PLUGIN(Platix)
	//WD_PLUGIN(Shtrih) не закончено тестирование
END_REGISTER_PLUGIN

//------------------------------------------------------------------------------
