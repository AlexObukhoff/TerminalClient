/* @file Настройки расширений. */

// STL
#include <array>

// Boost
#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

// PaymentProcessor SDK
#include <SDK/PaymentProcessor/Settings/Provider.h>

// Проект
#include "ExtensionsSettings.h"

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
QString ExtensionsSettings::getAdapterName()
{
	return CAdapterNames::Extensions;
}

//---------------------------------------------------------------------------
ExtensionsSettings::ExtensionsSettings(TPtree & aProperties)
	: mProperties(aProperties.get_child(CAdapterNames::Extensions, aProperties))
{
	auto loadExtensions = [&](const char * aChildPropertyName) -> void 
	{
		TPtree empty;
		SRange range;

		BOOST_FOREACH (const TPtree::value_type & record, mProperties.get_child(aChildPropertyName, empty))
		{
			if (record.first == "<xmlattr>")
			{
				continue;
			}

			try
			{
				QString extensionName = record.second.get<QString>("<xmlattr>.name");

				QMap<QString,QString> extensionParams;
				BOOST_FOREACH (const TPtree::value_type & parameter, record.second)
				{
					if (parameter.first == "<xmlattr>")
					{
						continue;
					}

					extensionParams.insert(
						parameter.second.get<QString>("<xmlattr>.name"), 
						parameter.second.get_value<QString>());
				}

				mExtensionSettings.insert(extensionName, extensionParams);
			}
			catch (std::exception & e)
			{
				toLog(LogLevel::Error, QString("Skipping broken extension: %1.").arg(e.what()));
			}
		}
	};

	loadExtensions("extensions");
	loadExtensions("config.extensions");
}

//---------------------------------------------------------------------------
ExtensionsSettings::~ExtensionsSettings()
{
}

//---------------------------------------------------------------------------
bool ExtensionsSettings::isValid() const
{
	return true;
}

//---------------------------------------------------------------------------
TStringMap ExtensionsSettings::getSettings(const QString & aExtensionName) const
{
	TStringMap result;

	if (mExtensionSettings.contains(aExtensionName))
	{
		result = mExtensionSettings.value(aExtensionName);
	}

	return result;
}

//---------------------------------------------------------------------------
}} // SDK::PaymentProcessor
