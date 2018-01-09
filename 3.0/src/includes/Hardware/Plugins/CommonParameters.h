/* @file Общие параметры плагинов. */

#pragma once

// SDK
#include <SDK/Plugins/PluginParameters.h>
#include <SDK/Plugins/PluginInitializer.h>
#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/InteractionTypes.h>
#include <SDK/Drivers/DetectingPriority.h>

// Modules
#include "Hardware/Plugins/DevicePluginBase.h"
#include "Hardware/Protocols/Common/ProtocolNames.h"

// Project
#include "Hardware/Common/HardwareConstants.h"

//------------------------------------------------------------------------
namespace CommonPluginParameterTranslations
{
	static const char * ModelName        = QT_TRANSLATE_NOOP("CommonParameters", "CommonParameters#model_name");
	static const char * OPOSName         = QT_TRANSLATE_NOOP("CommonParameters", "CommonParameters#opos_name");
	static const char * RequiredResource = QT_TRANSLATE_NOOP("CommonParameters", "CommonParameters#required_resource");
	static const char * InteractionType  = QT_TRANSLATE_NOOP("CommonParameters", "CommonParameters#interaction_type");
	static const char * ProtocolName     = QT_TRANSLATE_NOOP("CommonParameters", "CommonParameters#protocol_name");
};

namespace CPPT = CommonPluginParameterTranslations;
namespace DSDKIT = SDK::Driver::CInteractionTypes;

namespace SDK {
namespace Plugin {

template <class T>
inline QString makeDriverPath()
{
	QString result = makePath(SDK::Driver::Application, SDK::Driver::CComponents::Driver, T::getDeviceType(), T::getInteractionType());

	QString series = T::getSeries();
	QString subSeries = T::getSubSeries();

	if    (!series.isEmpty()) result += "." + series;
	if (!subSeries.isEmpty()) result += "." + subSeries;

	return result;
}

//------------------------------------------------------------------------------
template<class T>
inline QStringList sortParameters(QList<T> (* aGetParameters)())
{
	QList<T> data = (*aGetParameters)();
	QStringList result;

	foreach(T item, data)
	{
		result << QString("%1").arg(item);
	}

	result.removeDuplicates();
	qSort(result);

	if (result.isEmpty())
	{
		result << "";
	}

	return result;
}

//------------------------------------------------------------------------------
inline QStringList sortParameters(QStringList (* aGetParameters)())
{
	return sortParameters<QString>(reinterpret_cast<QList<QString> (* )()> (aGetParameters));
}

//------------------------------------------------------------------------------
/// Создать список параметров с именем модели.
template <class T>
inline TParameterList createNamedList(const QStringList & aModels, const QString & aDefault)
{
	return SNamedList<T, T::TIType>().create(aModels, aDefault)
		<< setNormalPriority();
}

//------------------------------------------------------------------------------
template <class T>
inline TParameterList createSimpleNamedList(const QStringList & aModels, const QString & aDefault)
{
	QString interactionType = T::getInteractionType();

	return TParameterList()
		<< SPluginParameter(CHardwareSDK::ModelName, false, CPPT::ModelName, QString(), aDefault, aModels, true)
		<< SPluginParameter(CHardwareSDK::InteractionType, true, CPPT::InteractionType, QString(), interactionType, QStringList() << interactionType)
		<< setNormalPriority();
}

//------------------------------------------------------------------------------
template <class T1, class T2>
struct SNamedList
{
	TParameterList create(const QStringList & aModels, const QString & aDefault)
	{
		return createSimpleNamedList<T1>(aModels, aDefault);
	}
};

//------------------------------------------------------------------------------
template <class T1>
struct SNamedList<T1, DSDKIT::ItCOM>
{
	TParameterList create(const QStringList & aModels, const QString & aDefault)
	{
		QStringList optionalPortSettings = sortParameters(&T1::getOptionalPortSettings);

		return createSimpleNamedList<T1>(aModels, aDefault)
			<< SPluginParameter(CHardwareSDK::RequiredResource, SPluginParameter::Text, false, CPPT::RequiredResource, QString(), "Common.Driver.IOPort.System.COM", QVariantMap(), true)
			<< SPluginParameter(CHardwareSDK::OptionalPortSettings, false, QString(), QString(), optionalPortSettings[0], optionalPortSettings, true);
	}
};

//------------------------------------------------------------------------------
template <class T1>
struct SNamedList<T1, DSDKIT::ItUSB>
{
	TParameterList create(const QStringList & aModels, const QString & aDefault)
	{
		return createSimpleNamedList<T1>(aModels, aDefault)
			<< setMultipleExistence();
	}
};

//------------------------------------------------------------------------------
template <class T1>
struct SNamedList<T1, DSDKIT::ItTCP>
{
	TParameterList create(const QStringList & aModels, const QString & aDefault)
	{
		return createSimpleNamedList<T1>(aModels, aDefault)
			<< SPluginParameter(CHardwareSDK::RequiredResource, SPluginParameter::Text, false, CPPT::RequiredResource, QString(), "Common.Driver.IOPort.System.TCP", QVariantMap(), true)
			<< setMultipleExistence();
	}
};

//------------------------------------------------------------------------------
template <class T1>
struct SNamedList<T1, DSDKIT::ItOPOS>
{
	TParameterList create(const QStringList & aModels, const QString & aDefault)
	{
		QStringList possibleNames = sortParameters(&T1::getProfileNames);

		return createSimpleNamedList<T1>(aModels, aDefault)
			<< setMultipleExistence()
			<< SPluginParameter(CHardware::OPOSName, true, CPPT::OPOSName, QString(), possibleNames[0], possibleNames);
	}
};

//------------------------------------------------------------------------------
/// Создать список параметров с именем модели.
template <class T>
inline TParameterList createNamedList(const QString & aModel, const QString & aDefault)
{
	return createNamedList<T>(QStringList() << aModel, aDefault);
}

//------------------------------------------------------------------------------
/// Создать простой список параметров с именем модели.
template <class T>
inline TParameterList createNamedList(const QString & aModel)
{
	return createNamedList<T>(aModel, aModel);
}

//------------------------------------------------------------------------------
/// Протокол.
inline SPluginParameter setProtocol(const QString & aProtocol)
{
	return SPluginParameter(CHardwareSDK::ProtocolName, false, CPPT::ProtocolName, QString(), aProtocol, QStringList() << aProtocol);
}

//------------------------------------------------------------------------------
/// Приоритет при автопоиске.
inline SPluginParameter setNormalPriority()
{
	QVariantMap possibleValues;
	possibleValues.insert(CHardwareSDK::DetectingPriority, SDK::Driver::EDetectingPriority::Normal);

	return SPluginParameter(CHardwareSDK::DetectingPriority, SPluginParameter::Text, false, QString(), QString(), SDK::Driver::EDetectingPriority::Normal, possibleValues, true);
}

inline TParameterList modifyPriority(const TParameterList & aParameterList, SDK::Driver::EDetectingPriority::Enum aPriority)
{
	TParameterList parameterList(aParameterList);

	auto it = std::find_if(parameterList.begin(), parameterList.end(), [] (const SPluginParameter & aParameter) { return aParameter.name == CHardwareSDK::DetectingPriority; });

	if (it != parameterList.end())
	{
		it->defaultValue = aPriority;
		it->possibleValues[CHardwareSDK::DetectingPriority] = aPriority;
	}

	return parameterList;
}

//------------------------------------------------------------------------------
/// Модифицированные значения параметров.
inline SPluginParameter setModifiedValues(const QString & aParameterValue, const QVariantMap & aPossibleValues)
{
	return SPluginParameter(CPlugin::ModifiedValues, SPluginParameter::Set, false, aParameterValue, QString(), QString(), aPossibleValues, true);
}

//------------------------------------------------------------------------------
/// Множественный тип атвопоиска устройства.
inline SPluginParameter setMultipleExistence()
{
	return SPluginParameter(CHardwareSDK::Existence, false, QString(), QString(), CHardwareSDK::ExistenceTypes::Multiple, QStringList() << CHardwareSDK::ExistenceTypes::Multiple, true);
}

//------------------------------------------------------------------------------
/// Модифицированные имена параметров.
inline SPluginParameter setModifiedKeys(const QString & aOldParameterName, const QString & aNewParameterName)
{
	QVariantMap modifiedKeys;
	modifiedKeys.insert(aOldParameterName, aNewParameterName);

	return SPluginParameter(CPlugin::ModifiedKeys, SPluginParameter::Set, false, QString(), QString(), QString(), modifiedKeys, true);
}

//------------------------------------------------------------------------------
#define CREATE_PLUGIN(aPluginName, aClassName) IPlugin * CreatePlugin_##aClassName(IEnvironment * aEnvironment, const QString & aInstancePath) \
	{ return new DevicePluginBase<aClassName>(aPluginName, aEnvironment, aInstancePath); }

#define REGISTER_DRIVER(aPluginName, aClassName, aParameters) CREATE_PLUGIN(aPluginName, aClassName) \
	REGISTER_PLUGIN_WITH_PARAMETERS(makeDriverPath<aClassName>(), &CreatePlugin_##aClassName, aParameters)

#define REGISTER_DRIVER_WITH_PARAMETERS(aClassName, aConstructor, aParameters) REGISTER_PLUGIN_WITH_PARAMETERS(makeDriverPath<aClassName>(), aConstructor, aParameters)

#define COMMON_DRIVER_WITH_PARAMETERS(aClassName, aConstructor, aParameters) PLUGIN_WITH_PARAMETERS(makeDriverPath<aClassName>(), aConstructor, aParameters)
#define COMMON_DRIVER(aClassName, aParameters) COMMON_DRIVER_WITH_PARAMETERS(aClassName, &CreatePlugin<aClassName>, aParameters)
#define SIMPLE_COMMON_DRIVER(aClassName, aParameters) COMMON_DRIVER(aClassName, &aParameters<aClassName>)

}} // namespace SDK::Plugin

//------------------------------------------------------------------------------
