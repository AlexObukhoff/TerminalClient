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

	foreach (const QUuid & uuid, aUuids)
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

			if (!SystemDeviceUtils::containsTag(outData, CSerialDeviceUtils::Tags::Unimportant()))
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

		data.clear();
		QRegExp regExp("COM[0-9]+");
		DeviceWinProperties deviceWinProperties;

		for (auto it = deviceProperties.begin(); it != deviceProperties.end(); ++it)
		{
			int index = -1;

			do
			{
				index = regExp.indexIn(it.key(), ++index);

				if (index != -1)
				{
					TWinProperties & propertyData = it->data;
					EPortTypes::Enum portType = EPortTypes::Unknown;

					     if (SystemDeviceUtils::containsTag(propertyData[deviceWinProperties[SPDRP_ENUMERATOR_NAME]], CSerialDeviceUtils::Tags::COM())) portType = EPortTypes::COM;
					else if (SystemDeviceUtils::containsTag(propertyData, CSerialDeviceUtils::Tags::Virtual() + CSerialDeviceUtils::AllVCOMTags())) portType = EPortTypes::VirtualCOM;
					else if (SystemDeviceUtils::containsTag(propertyData, CSerialDeviceUtils::Tags::Emulator())) portType = EPortTypes::COMEmulator;

					if (!SystemDeviceUtils::containsTag(propertyData, CSerialDeviceUtils::Tags::Unimportant()))
					{
						data.insert(regExp.capturedTexts()[0], portType);
					}
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
