/* @file Движок мем-свичей принтеров STAR. */

#pragma once

// STL
#include <cmath>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSet>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/ASCII.h"

// Project
#include "StarMemorySwitchesBase.h"

using namespace CSTAR;

//--------------------------------------------------------------------------------
bool BaseMemorySwitchUtils::update(const TMemorySwitchTypes & aSTARMemorySwitchTypes, TMemorySwitches & aMemorySwitches, const QVariantMap & aConfiguration)
{
	foreach (ESTARMemorySwitchTypes::Enum aParameterType, aSTARMemorySwitchTypes)
	{
		QStringList parameterValue;

		if (!hasConfigKeys(aParameterType, aConfiguration) || !hasValidData(aParameterType, aMemorySwitches) || !getParameterValue(aParameterType, aConfiguration, parameterValue))
		{
			return false;
		}

		SMSWParameter data = getMSWParameter(aParameterType);

		ushort mask = getMask(aParameterType);
		aMemorySwitches[data.number].value &= ~mask;
		ushort result = data.parameters.key(parameterValue).toUShort(0, 2) << data.index;
		aMemorySwitches[data.number].value |= result;
	}

	return true;
}

//--------------------------------------------------------------------------------
CSTAR::TMemorySwitchTypes BaseMemorySwitchUtils::getMemorySwitchTypes()
{
	CSTAR::TMemorySwitchTypes result;

	for (CSTAR::TMSWData::iterator it = mData.begin(); it != mData.end(); ++it)
	{
		if (it->models.isEmpty() || !(it->models & mModels).isEmpty())
		{
			result << it.key();
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
bool BaseMemorySwitchUtils::update(TMemorySwitches & aMemorySwitches)
{
	return update(getMemorySwitchTypes(), aMemorySwitches, mConfiguration) && setConfiguration(aMemorySwitches);
}

//--------------------------------------------------------------------------------
SMSWParameter BaseMemorySwitchUtils::getMSWParameter(ESTARMemorySwitchTypes::Enum aParameterType)
{
	bool modelsCoincided = false;

	for (auto it = mData.begin(); it != mData.end(); ++it)
	{
		modelsCoincided = modelsCoincided || ((it.key() == aParameterType) && !(it->models & mModels).isEmpty());
	}

	for (auto it = mData.begin(); it != mData.end(); ++it)
	{
		if ((it.key() == aParameterType) && ((!modelsCoincided && it->models.isEmpty()) || !(it->models & mModels).isEmpty()))
		{
			return *it;
		}
	}

	return SMSWParameter();
}

//--------------------------------------------------------------------------------
void BaseMemorySwitchUtils::setModels(const TModels & aModels)
{
	mModels = aModels;
}

//--------------------------------------------------------------------------------
bool BaseMemorySwitchUtils::hasValidData(ESTARMemorySwitchTypes::Enum aParameterType, const TMemorySwitches & aMemorySwitches)
{
	SMSWParameter data = getMSWParameter(aParameterType);

	if (aMemorySwitches.size() <= data.number)
	{
		toLog(LogLevel::Error, QString("No memory switch %1 for parameter %2").arg(data.number).arg(data.description));
		return false;
	}

	int number = data.number;

	if (number == -1)
	{
		toLog(LogLevel::Error, QString("Failed to get data for unknown parameter type %1, no data for model(s): %2")
			.arg(aParameterType).arg(static_cast<QStringList>(mModels.toList()).join(", ")));
		return false;
	}

	if (!aMemorySwitches[number].valid)
	{
		toLog(LogLevel::Error, QString("Memory switch %1 for parameter %2 is not valid").arg(data.number).arg(data.description));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool BaseMemorySwitchUtils::hasConfigKeys(ESTARMemorySwitchTypes::Enum aParameterType, const QVariantMap & aConfiguration)
{
	SMSWParameter data = getMSWParameter(aParameterType);
	QSet<QString> missingKeys = data.dataTypes.toSet() - aConfiguration.keys().toSet();

	if (!missingKeys.isEmpty())
	{
		toLog(LogLevel::Error, QString("No values in the configuration for %1 due to absence of the next parameter(s): %2")
			.arg(data.description).arg(static_cast<QStringList>(missingKeys.toList()).join(", ")));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool BaseMemorySwitchUtils::getParameterValue(ESTARMemorySwitchTypes::Enum aParameterType, const QVariantMap & aConfiguration, QStringList & aValue)
{
	SMSWParameter data = getMSWParameter(aParameterType);
	QStringList parameterKeys = data.dataTypes;
	bool result = true;

	for (int i = 0; i < parameterKeys.size(); ++i)
	{
		QString configValue = aConfiguration[parameterKeys[i]].toString();
		aValue << configValue;

		QSet<QString> possibleParameterValues;
		std::for_each(data.parameters.begin(), data.parameters.end(), [&] (const QStringList & aValueList) { possibleParameterValues << aValueList[i]; });

		if (!possibleParameterValues.contains(configValue))
		{
			toLog(LogLevel::Error, QString("Unknown value \"%1\" of parameter %2 from temporary configuration, it is need smth from list: %3")
				.arg(configValue).arg(data.description).arg(static_cast<QStringList>(possibleParameterValues.toList()).join(", ")));
			result = false;
		}
	}

	if (!result)
	{
		aValue.clear();

		return false;
	}
	else if (!data.parameters.values().contains(aValue))
	{
		QString log;

		for (int i = 0; i < aValue.size(); ++i)
		{
			log += QString("%1%2 = %3").arg(log.isEmpty() ? "" : "; ").arg(data.dataTypes[i]).arg(aValue[i]);
		}

		toLog(LogLevel::Error, QString("No required parameter list \"%1\" for parameter %2 in the configuration").arg(log).arg(data.description));

		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
ushort BaseMemorySwitchUtils::getMask(ESTARMemorySwitchTypes::Enum aParameterType)
{
	SMSWParameter data = getMSWParameter(aParameterType);
	QString mask = QString(16 - data.bitAmount - data.index, '0') + QString(data.bitAmount, '1') + QString(data.index, '0');

	return mask.toUShort(0, 2);
}

//--------------------------------------------------------------------------------
bool BaseMemorySwitchUtils::getConfigParameter(ESTARMemorySwitchTypes::Enum aType, const TMemorySwitches & aMemorySwitches, QVariant & aValue)
{
	if (!hasValidData(aType, aMemorySwitches))
	{
		return false;
	}

	SMSWParameter data = getMSWParameter(aType);
	QString parameterKey = QString("%1").arg(aMemorySwitches[data.number].value & getMask(aType), 16, 2, QChar('0'));
	parameterKey = parameterKey.mid(16 - data.index - data.bitAmount, data.bitAmount);
	QStringList parameterKeys = data.parameters.keys();

	if (!parameterKeys.contains(parameterKey))
	{
		toLog(LogLevel::Error, QString("Unknown key \"%1\" of parameter %2 for memory switch %3 (0x%4), it is need smth from list: %5")
			.arg(parameterKey).arg(data.description).arg(data.number).arg(aMemorySwitches[data.number].value, 4, 16, QChar('0')).arg(parameterKeys.join(", ")));
		return false;
	}

	aValue = data.parameters[parameterKey];

	return true;
}

//--------------------------------------------------------------------------------
void BaseMemorySwitchUtils::setConfiguration(const QVariantMap & aConfiguration)
{
	for (auto it = aConfiguration.begin(); it != aConfiguration.end(); ++it)
	{
		QString value = it.value().toString();

		if (value != CHardwareSDK::Values::Auto)
		{
			mConfiguration.insert(it.key(), value);
		}
	}
}

//--------------------------------------------------------------------------------
bool BaseMemorySwitchUtils::setConfiguration(const TMemorySwitches & aMemorySwitches)
{
	foreach (ESTARMemorySwitchTypes::Enum aParameterType, getMemorySwitchTypes())
	{
		if (!hasValidData(aParameterType, aMemorySwitches))
		{
			return false;
		}

		SMSWParameter data = getMSWParameter(aParameterType);
		CSTAR::SMemorySwitch memorySwitchData = aMemorySwitches[data.number];

		if (!memorySwitchData.valid)
		{
			toLog(LogLevel::Error, QString("Failed to set configuration for parameter %1 (switch number %2) due to wrong data of memory switch")
				.arg(data.description).arg(data.number));
			return false;
		}

		ushort memorySwitch = aMemorySwitches[data.number].value;

		int memorySwitchSize = 8 * sizeof(memorySwitch);
		int bitAmount = data.parameters.begin().key().size();
		QString bits = QString("%1").arg(memorySwitch, memorySwitchSize, 2, QChar(ASCII::Zero)).mid(memorySwitchSize - data.index - bitAmount, bitAmount);

		if (!data.parameters.contains(bits))
		{
			toLog(LogLevel::Error, QString("Unknown key \"%1\" of parameter %2 from configuration, it is need smth from list: %3")
				.arg(bits).arg(data.description).arg(static_cast<QStringList>(data.parameters.keys()).join(", ")));
			return false;
		}

		QStringList keys = data.dataTypes;

		for (int i = 0; i < keys.size(); ++i)
		{
			mConfiguration[keys[i]] = data.parameters[bits][i];
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
void BaseMemorySwitchUtils::add(ESTARMemorySwitchTypes::Enum aParameterType, int aNumber, int aIndex, const QStringList & aDataTypes, const TMSWParameters & aParameters, const QString & aDescription, const TModels & models)
{
	int bitAmount = aParameters.begin().key().size();
	QString description = QString("\"%1\" (MSW %2: %3-%4)").arg(aDescription).arg(aNumber).arg(aIndex).arg(aIndex + bitAmount);

	mData.insertMulti(aParameterType, SMSWParameter(aNumber, aIndex, bitAmount, aDataTypes, aParameters, description, models));
}

//--------------------------------------------------------------------------------
void BaseMemorySwitchUtils::add(ESTARMemorySwitchTypes::Enum aParameterType, int aNumber, int aIndex, const QString & aDataType, const TMSWParameters & aParameters, const QString & aDescription, const TModels & models)
{
	add(aParameterType, aNumber, aIndex, QStringList() << aDataType, aParameters, aDescription, models);
}

//--------------------------------------------------------------------------------
void BaseMemorySwitchUtils::add(ESTARMemorySwitchTypes::Enum aParameterType, int aNumber, int aIndex, bool aInvert, const QString & aDataName, const QString & aDescription, const TModels & models)
{
	TMSWParameters parameters;
	parameters.insert("0", QStringList() << ( aInvert ? CHardwareSDK::Values::Use : CHardwareSDK::Values::NotUse));
	parameters.insert("1", QStringList() << (!aInvert ? CHardwareSDK::Values::Use : CHardwareSDK::Values::NotUse));

	add(aParameterType, aNumber, aIndex, QStringList() << aDataName, parameters, aDescription, models);
}

//--------------------------------------------------------------------------------
void BaseMemorySwitchUtils::fillExcept(TMSWParameters & aParameters, const TMSWSimpleParameters & aKnownParameters, const QString & aExceptValue)
{
	for (auto it = aKnownParameters.begin(); it != aKnownParameters.end(); ++it)
	{
		aParameters.insert(it.key(), QStringList() << it.value()); 
	}

	int size = aKnownParameters.begin().key().size();

	for (int i = 0; i < int(std::pow(2.0, size)); ++i)
	{
		QString key = QString("%1").arg(i, size, 2, QChar(ASCII::Zero));

		if (!aKnownParameters.contains(key))
		{
			aParameters.insert(key, QStringList() << aExceptValue);
		}
	}
}

//--------------------------------------------------------------------------------
void BaseMemorySwitchUtils::fillExcept(TMSWParameters & aParameters, const QString & aValue, const QString & aKey, const QString & aExceptValue)
{
	TMSWSimpleParameters data;
	data.insert(aKey, aValue);

	fillExcept(aParameters, data, aExceptValue);
}

//--------------------------------------------------------------------------------
