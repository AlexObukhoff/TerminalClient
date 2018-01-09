/* @file Шабон реализации плагина. */

// Проект
#include "PluginTemplate.h"

//------------------------------------------------------------------------------
namespace
{

/// Конструктор плагина.
SDK::Plugin::IPlugin * CreatePlugin(SDK::Plugin::IEnvironment * aEnvironment, const QString & aInstancePath)
{
	return new Plugin(aEnvironment, aInstancePath);
}

/// Перечислитель параметров.
QVector<SDK::Plugin::SPluginParameter> EnumParameters()
{
	#error Опиши параметры!
	QVariantMap param2Values;
	param2Values["one"] = 1;
	param2Values["two"] = 2;
	param2Values["three"] = 3;

	return QVector<SDK::Plugin::SPluginParameter>(2)
		<< SDK::Plugin::SPluginParameter("somenumber", SDK::Plugin::SPluginParameter::Number, true, "#parameter_1", "#parameter_1_howto", 0)
		<< SDK::Plugin::SPluginParameter("somelist", SDK::Plugin::SPluginParameter::Set, false, "#parameter_2", "#parameter_2_howto", "two", param2Values);
}

}

#error Задай путь!
REGISTER_PLUGIN("Vendor.Application.Component.PluginName", &CreatePlugin, &EnumParameters);

//------------------------------------------------------------------------------
Plugin::Plugin(SDK::Plugin::IEnvironment * aEnvironment, const QString & aInstancePath) :
	mEnvironment(aEnvironment),
	mInstancePath(aInstancePath)
{
	setLog(mEnvironment->getLog("PluginName"));
	toLog(LogLevel::Normal, "Plugin start");
	//TODO - initialize plugin
	#error Измени конструктор!
}

//------------------------------------------------------------------------------
QString Plugin::getPluginName() const
{
	return "my name";
}

//------------------------------------------------------------------------------
QString Plugin::getConfigurationName() const
{
	return mInstancePath;
}

//------------------------------------------------------------------------------
QVariantMap Plugin::getConfiguration() const
{
	return mParameters;
}

//------------------------------------------------------------------------------
void Plugin::setConfiguration(const QVariantMap & aParameters)
{
	mParameters = aParameters;
}

//------------------------------------------------------------------------------
bool Plugin::saveConfiguration()
{
	return mFactory->savePluginConfiguration(mInstancePath, mParameters);
}

//------------------------------------------------------------------------------
bool Plugin::isReady() const
{
	return true;
}

//------------------------------------------------------------------------------
