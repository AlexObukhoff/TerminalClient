/* @file Набор функционала для работы с системными ресурсами для [V]COM-портов. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <QtCore/QSettings>
#include <Common/QtHeadersEnd.h>

// Project
#include "SerialDeviceUtils.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
TWinDeviceProperties SerialDeviceUtils::getDeviceProperties(const TUuids & aUuids, DWORD aPropertyName, bool aQuick, TIOPortDeviceData * aData)
{
	TWinDeviceProperties deviceProperties;
	QMap<QString, QStringList> sourceDeviceData;

	foreach(const QUuid & uuid, aUuids)
	{
		TWinDeviceProperties uidDeviceProperties;

		if (SystemDeviceUtils::enumerateSystemDevices(uuid, uidDeviceProperties, aPropertyName, aQuick))
		{
			for (auto it = uidDeviceProperties.begin(); it != uidDeviceProperties.end(); ++it)
			{
				deviceProperties.insert(it.key(), it.value());
				sourceDeviceData[it.key()] << uuid.toString();
			}
		}
	}

	if (aUuids == CSerialDeviceUtils::Uuids())
	{
		TWinDeviceProperties deviceRegistryProperties = SystemDeviceUtils::enumerateRegistryDevices(aQuick);
		SystemDeviceUtils::mergeRegistryDeviceProperties(deviceProperties, deviceRegistryProperties, sourceDeviceData);
	}

	if (aData)
	{
		for (auto it = sourceDeviceData.begin(); it != sourceDeviceData.end(); ++it)
		{
			QString outKey  = SystemDeviceUtils::getDeviceOutKey(it.value());
			QString outData = SystemDeviceUtils::getDeviceOutData(deviceProperties[it.key()].data);

			if (!outData.toLower().contains("mouse"))
			{
				aData->insert(outKey, outData);
			}
		}
	}

	return deviceProperties;
}

//--------------------------------------------------------------------------------
SerialDeviceUtils::TData SerialDeviceUtils::getSystemData(bool aForce)
{
	static TData data;

	if (aForce || data.isEmpty())
	{
		TWinDeviceProperties deviceProperties = getDeviceProperties(CSerialDeviceUtils::Uuids(), CSerialDeviceUtils::PathProperty);

		auto isMatched = [&] (const TWinProperties & aProperties, const QStringList & aTags) -> bool
		{ return std::find_if(aProperties.begin(), aProperties.end(), [&] (const QString & aValue) -> bool
		{ return std::find_if(aTags.begin(), aTags.end(), [&] (const QString & aTag) -> bool
		{ return aValue.contains(aTag, Qt::CaseInsensitive); }) != aTags.end(); }) != aProperties.end(); };

		data.clear();
		QRegExp regExp("COM[0-9]+");

		for (auto it = deviceProperties.begin(); it != deviceProperties.end(); ++it)
		{
			int index = -1;

			do
			{
				index = regExp.indexIn(it.key(), ++index);

				if (index != -1)
				{
					EPortTypes::Enum portType = isMatched(it->data, CSerialDeviceUtils::Tags::Virtual()) ? EPortTypes::VirtualCOM :
						(isMatched(it->data, CSerialDeviceUtils::Tags::Emulator()) ? EPortTypes::COMEmulator : EPortTypes::Unknown);
					data.insert(regExp.capturedTexts()[0], portType);
				}
			}
			while (index != -1);
		}

		/*
		// раскомментировать, если для автопоиска порта по GUID-у (ам) будут какие-либо проблемы
		foreach(const QString & port, SystemDeviceUtils::enumerateCOMPorts())
		{
		if (!data.contains(port))
		{
		data.insert(port, true);
		}
		}
		*/
	}

	return data;
}

//--------------------------------------------------------------------------------
