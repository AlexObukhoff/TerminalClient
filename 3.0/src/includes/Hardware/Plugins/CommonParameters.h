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
	static const char * ProtocolType     = QT_TRANSLATE_NOOP("CommonParameters", "CommonParameters#protocol_type");
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
inline TParameterList modifyValue(const TParameterList & aParameterList, const QString & aName, const QVariant & aValue, const QString & aOldValue = "")
{
	TParameterList parameterList(aParameterList);

	auto it = std::find_if(parameterList.begin(), parameterList.end(), [&aName] (const SPluginParameter & aParameter) { return aParameter.name == aName; });

	if (it != parameterList.end())
	{
		it->defaultValue = aValue;
		QVariantMap & possibleValues = it->possibleValues;

		if (possibleValues.contains(aName))
		{
			possibleValues[aName] = aValue;
		}
		else if (possibleValues.contains(aOldValue) && (possibleValues[aOldValue] == aOldValue))
		{
			possibleValues.remove(aOldValue);
			possibleValues.insert(aValue.toString(), aValue);
		}
	}

	return parameterList;
}

//------------------------------------------------------------------------------
inline TParameterList modifyPriority(const TParameterList & aParameterList, SDK::Driver::EDetectingPriority::Enum aPriority)
{
	return modifyValue(aParameterList, CHardwareSDK::DetectingPriority, aPriority);
}

//------------------------------------------------------------------------------
/// Создать список параметров с именем модели.
template <class T>
inline TParameterList createNamedList(const QStringList & aModels, const QString & aDefault)
{
	return SNamedList<T, T::TIType>().create(aModels, aDefault);
}

//------------------------------------------------------------------------------
template <class T>
inline TParameterList createSimpleNamedList(const QStringList & aModels, const QString & aDefault)
{
	QString interactionType = T::getInteractionType();
	QVariantMap modifiedValues;
	modifiedValues.insert("no change", CHardwareSDK::Values::Auto);
	modifiedValues.insert("not use",   CHardwareSDK::Values::NotUse);

	return TParameterList()
		<< SPluginParameter(CHardwareSDK::ModelName, false, CPPT::ModelName, QString(), aDefault, aModels, true)
		<< SPluginParameter(CHardwareSDK::InteractionType, true, CPPT::InteractionType, QString(), interactionType, QStringList() << interactionType, false, SDK::Plugin::EImportanceLevel::High)
		<< setModifiedValues("", modifiedValues)
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

		TParameterList result = createSimpleNamedList<T1>(aModels, aDefault)
			<< SPluginParameter(CHardwareSDK::RequiredResource, SPluginParameter::Text, false, CPPT::RequiredResource, QString(), "Common.Driver.IOPort.System.COM", QVariantMap(), true)
			<< SPluginParameter(CHardwareSDK::OptionalPortSettings, false, QString(), QString(), optionalPortSettings[0], optionalPortSettings, true);

		QString VCOMType = T1::getVCOMType();
		QStringList VCOMTags = T1::getVCOMTags();
		QString VCOMConnectionType = T1::getVCOMConnectionType();

		if (!VCOMConnectionType.isEmpty())
		{
			using namespace SDK::Driver::VCOM;

			QVariantMap VCOMData;
			VCOMData.insert(CHardwareSDK::VCOMType, VCOMType);
			VCOMData.insert(CHardwareSDK::VCOMTags, VCOMTags);
			VCOMData.insert(CHardwareSDK::VCOMConnectionType, VCOMConnectionType);

			result << SPluginParameter(CHardwareSDK::VCOMData, false, QString(), QString(), VCOMData, QStringList(), true);
		}

		return result;
	}
};

//------------------------------------------------------------------------------
template <class T1>
struct SNamedList<T1, DSDKIT::ItExternalCOM>
{
	TParameterList create(const QStringList & aModels, const QString & aDefault)
	{
		return createSimpleNamedList<T1>(aModels, aDefault)
			<< SPluginParameter(CHardwareSDK::RequiredResource, SPluginParameter::Text, false, CPPT::RequiredResource, QString(), "Common.Driver.IOPort.System.COM", QVariantMap(), true);
	}
};

//------------------------------------------------------------------------------
template <class T1>
struct SNamedList<T1, DSDKIT::ItExternalVCOM>
{
	TParameterList create(const QStringList & aModels, const QString & aDefault)
	{
		return createSimpleNamedList<T1>(aModels, aDefault)
			<< SPluginParameter(CHardwareSDK::RequiredResource, SPluginParameter::Text, false, CPPT::RequiredResource, QString(), "Common.Driver.IOPort.System.COM", QVariantMap(), true);
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
struct SNamedList<T1, DSDKIT::ItLibUSB>
{
	TParameterList create(const QStringList & aModels, const QString & aDefault)
	{
		return modifyValue(createSimpleNamedList<T1>(aModels, aDefault), CHardwareSDK::InteractionType, CInteractionTypes::USB, CInteractionTypes::LibUSB)
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

//------------------------------------------------------------------------------
/// Исключенные параметры.
inline SPluginParameter setExcludedParameters(const QStringList & aParameters)
{
	return SPluginParameter(CHardwareSDK::ExcludedParameters, false, QString(), QString(), aParameters, aParameters, true);
}

//------------------------------------------------------------------------------
/// Исключенные параметры для устройства на COM-порту.
inline SPluginParameter setSerialPortExcludedParameters(const QString & aIncludedParameter = "")
{
	QStringList serialPortParameters = QStringList()
		<< COMPortSDK::BaudRate
		<< COMPortSDK::Parity
		<< COMPortSDK::RTS
		<< COMPortSDK::DTR
		<< COMPortSDK::ByteSize
		<< COMPortSDK::StopBits;

	serialPortParameters.removeAll(aIncludedParameter);

	return setExcludedParameters(serialPortParameters);
}

//------------------------------------------------------------------------------
/// Модифицированные имена ключей.
inline SPluginParameter setModifiedKeys(const QString & aOldParameterName, const QString & aNewParameterName)
{
	QVariantMap modifiedKeys;
	modifiedKeys.insert(aOldParameterName, aNewParameterName);

	return SPluginParameter(CPlugin::ModifiedKeys, SPluginParameter::Set, false, QString(), QString(), QString(), modifiedKeys, true);
}

//------------------------------------------------------------------------------
/// Модифицированные значения параметров.
inline SPluginParameter setModifiedValues(const QString & aParameterValue, const QVariantMap & aPossibleValues)
{
	return SPluginParameter(CPlugin::ModifiedValues, SPluginParameter::Set, false, aParameterValue, QString(), QString(), aPossibleValues, true);
}

//------------------------------------------------------------------------------
/// Модифицированные значения параметров.
inline SPluginParameter setModifiedValues(const QString & aParameterValue, const QString & aValueFrom, const QString & aValueTo)
{
	QVariantMap possibleValues;
	possibleValues.insert(aValueFrom, aValueTo);

	return SPluginParameter(CPlugin::ModifiedValues, SPluginParameter::Set, false, aParameterValue, QString(), QString(), possibleValues, true);
}

//------------------------------------------------------------------------------
/// Множественный тип атвопоиска устройства.
inline SPluginParameter setMultipleExistence()
{
	return SPluginParameter(CHardwareSDK::Existence, false, QString(), QString(), CHardwareSDK::ExistenceTypes::Multiple, QStringList() << CHardwareSDK::ExistenceTypes::Multiple, true);
}

//------------------------------------------------------------------------------
// Тип протокола.
inline SPluginParameter setProtocolType(const QString & aDefaultType, const QStringList & aPossibleTypes)
{
	return SPluginParameter(CHardware::ProtocolType, false, CPPT::ProtocolType, QString(), aDefaultType, aPossibleTypes);
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
