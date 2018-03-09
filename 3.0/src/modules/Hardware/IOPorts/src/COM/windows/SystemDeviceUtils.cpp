/* @file Набор функционала для работы с системными ресурсами с использованием SetupDi. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <QtCore/QSettings>
#include <Common/QtHeadersEnd.h>

// Project
#include "SystemDeviceUtils.h"

//--------------------------------------------------------------------------------
QStringList SystemDeviceUtils::enumerateCOMPorts()
{
	TCHAR buffer[USHRT_MAX];
	memset(buffer, NULL, sizeof(buffer));
	int bufferSize = QueryDosDevice(NULL, &buffer[0], USHRT_MAX);

	QStringList data = QString::fromWCharArray(buffer, bufferSize).split(QChar(ASCII::NUL)).filter(QRegExp("COM[0-9]+"));
	QRegExp regExp("[^0-9a-zA-Z](COM[0-9]+)[^a-zA-Z]");
	QStringList result;

	foreach(auto port, data)
	{
		if (regExp.indexIn(" " + port + " ") != -1)
		{
			result << regExp.cap(0).simplified();
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
QStringList SystemDeviceUtils::getSerialDeviceNames()
{
	HKEY hKey = nullptr;

	if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, CRegistrySerialPort::QuickInitialPath, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
	{
		return QStringList();
	}

	QStringList result;
	DWORD i = 0;
	QByteArray key(MAXSHORT, ASCII::NUL);
	QByteArray value;
	DWORD valueSize = 0;

	forever
	{
		DWORD keySyze = MAXSHORT / sizeof(wchar_t);
		LONG ret = ::RegEnumValue(hKey, i, LPTSTR(key.data()), &keySyze, nullptr, nullptr, LPBYTE(value.data()), &valueSize);

		if (ret == ERROR_MORE_DATA)
		{
			value.resize(valueSize);
		}
		else if (ret == ERROR_SUCCESS)
		{
			QString data = QString::fromWCharArray(LPTSTR(value.data()), value.size() / sizeof(wchar_t));
			int size = result.indexOf(QChar(0));

			result << data.left(size).replace(QChar(ASCII::NUL), "");
			i++;
		}
		else
		{
			break;
		}
	}

	::RegCloseKey(hKey);

	result.sort();

	return result;
}

//--------------------------------------------------------------------------------
QString SystemDeviceUtils::getRelevantPortName(const QString & aPortName, const QStringList & aPortNames)
{
	auto getPropertyData = [] (const QString & aData) -> QString { return " " + aData.simplified().toLower() + " "; };
	auto getRegexData = [] (const QString & aData) -> QString { return QString("%2%1%2").arg(getScreenedData(aData.simplified().toLower())).arg("[^a-z0-9]+"); };
	QString pathPropertyData = getPropertyData(aPortName);
	QRegExp pathPropertyRegex = QRegExp(getRegexData(aPortName));

	auto it = std::find_if(aPortNames.begin(), aPortNames.end(), [&] (const QString & portName) -> bool { return !portName.isEmpty() &&
		(pathPropertyData.contains(QRegExp(getRegexData(portName))) || getPropertyData(portName).contains(pathPropertyRegex)); });

	return (it == aPortNames.end()) ? "" : *it;
}

//--------------------------------------------------------------------------------
QString SystemDeviceUtils::getRelevantWinProperty(const QString & aWinProperty, const QStringList & aWinProperties)
{
	auto getData = [] (const QString & aData) -> QString { return aData.simplified().toLower().replace(CDeviceWinProperties::Prefix, "", Qt::CaseInsensitive).replace("_", ""); };
	QString winProperty = getData(aWinProperty);

	auto it = std::find_if(aWinProperties.begin(), aWinProperties.end(), [&] (const QString & aProperty) -> bool { return (winProperty == getData(aProperty)); });

	return (it == aWinProperties.end()) ? "" : *it;
}

//--------------------------------------------------------------------------------
TWinDeviceProperties SystemDeviceUtils::enumerateRegistryDevices(bool aQuick)
{
	QStringList portNames = getSerialDeviceNames();
	TWinDeviceProperties result;

	if (aQuick)
	{
		foreach (const QString & portName, portNames)
		{
			result[portName.simplified()];
		}

		return result;
	}

	QSettings settings(CRegistrySerialPort::InitialPath, QSettings::NativeFormat);
	QStringList allKeys = settings.allKeys().filter(CRegistrySerialPort::PathProperty);

	foreach (const QString & key, allKeys)
	{
		QString pathPropertyValue = settings.value(key).toString();

		if (!getRelevantPortName(pathPropertyValue, portNames).isEmpty())
		{
			QStringList sections = key.split("/");
			QStringList groupSections = sections.mid(0, sections.size() - 1);
			QString group = groupSections.join("/");

			settings.beginGroup(group);

			TWinProperties properties;

			foreach (const QString & propertyKey, settings.childKeys())
			{
				properties.insert(propertyKey, settings.value(propertyKey).toString());
			}

			settings.endGroup();

			if (properties.contains(CRegistrySerialPort::PathProperty))
			{
				result[pathPropertyValue].path = QString("%1\\%2").arg(CRegistrySerialPort::InitialPath).arg(group.replace("/", "\\"));
				result[pathPropertyValue].data = properties;
			}
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
QString SystemDeviceUtils::getProperty(const HDEVINFO & aHDevInfo, SP_DEVINFO_DATA & aDeviceInfoData, DWORD aProperty)
{
	DWORD DataT;
	LPTSTR buffer = NULL;
	DWORD buffersize = 0;

	while (!SetupDiGetDeviceRegistryProperty(aHDevInfo, &aDeviceInfoData, aProperty, &DataT, (PBYTE)buffer, buffersize, &buffersize))
	{
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			if (buffer)
			{
				LocalFree(buffer);
			}

			buffer = (LPTSTR) LocalAlloc(LPTR, buffersize * 2);
		}
		else
		{
			break;
		}
	}

	QString result = QString::fromWCharArray(buffer);

	if (buffer)
	{
		LocalFree(buffer);
	}

	return result;
}

//--------------------------------------------------------------------------------
QString SystemDeviceUtils::getRegKeyValue(HKEY key, LPCTSTR aProperty)
{
	DWORD size = 0;
	RegQueryValueEx(key, aProperty, NULL, NULL, NULL, &size);

	BYTE * buffer = new BYTE[size+1];
	ZeroMemory(buffer, size+1);

	QString value;

	if (RegQueryValueEx(key, aProperty, NULL, NULL, buffer, &size) == ERROR_SUCCESS)
	{
		value = QString::fromUtf16((ushort *)(buffer));
	}

	delete [] buffer;

	return value;
}

//--------------------------------------------------------------------------------
bool SystemDeviceUtils::enumerateSystemDevices(const QUuid & aUuid, TWinDeviceProperties & aDeviceProperties, DWORD aPathProperty, bool aQuick)
{
	GUID guid = aUuid;
	HDEVINFO deviceInfo = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (deviceInfo == INVALID_HANDLE_VALUE)
	{
		qCritical("SetupDiGetClassDevs failed. Error code: %ld", GetLastError());
		return false;
	}

	//enumerate the devices
	BOOL OK = TRUE;
	SP_DEVICE_INTERFACE_DATA interfaceData;
	interfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	SP_DEVICE_INTERFACE_DETAIL_DATA * detailedData = nullptr;
	DWORD detailedDataSize = 0;
	DWORD olddetailedDataSize = 0;

	for (DWORD i = 0; OK; i++)
	{
		OK = SetupDiEnumDeviceInterfaces(deviceInfo, NULL, &guid, i, &interfaceData);

		if (OK)
		{
			SP_DEVINFO_DATA deviceData = { sizeof(SP_DEVINFO_DATA) };

			//check for required detData size
			SetupDiGetDeviceInterfaceDetail(deviceInfo, &interfaceData, NULL, 0, &detailedDataSize, & deviceData);

			//if larger than old detData size then reallocate the buffer
			if (detailedDataSize > olddetailedDataSize)
			{
				delete [] detailedData;

				detailedData = (SP_DEVICE_INTERFACE_DETAIL_DATA *) new char[detailedDataSize];
				detailedData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
				olddetailedDataSize = detailedDataSize;
			}

			//check the details
			if (!SetupDiGetDeviceInterfaceDetail(deviceInfo, &interfaceData, detailedData, detailedDataSize, NULL, &deviceData)) 
			{
				qCritical("SetupDiGetDeviceInterfaceDetail failed. Error code: %ld", GetLastError());
				return false;
			}

			QString pathProperty = getProperty(deviceInfo, deviceData, aPathProperty);
			QString path = QString::fromWCharArray(detailedData->DevicePath);
			aDeviceProperties[pathProperty].path = path;

			QRegExp regexp("vid_([0-9a-fA-F]+)");
			bool parseOK;

			if (regexp.indexIn(path) != -1)
			{
				QString data = regexp.capturedTexts()[1];
				qint32 value = data.toInt(&parseOK, 16);

				if (parseOK)
				{
					aDeviceProperties[pathProperty].VID = value;
				}
			}

			regexp = QRegExp("pid_([0-9a-fA-F]+)");

			if (regexp.indexIn(path) != -1)
			{
				QString data = regexp.capturedTexts()[1];
				qint32 value = data.toInt(&parseOK, 16);

				if (parseOK)
				{
					aDeviceProperties[pathProperty].PID = value;
				}
			}

			if (!aQuick)
			{
				DeviceWinProperties deviceWinProperties;

				foreach (const QString & winProperty, deviceWinProperties.data().values())
				{
					aDeviceProperties[pathProperty].data.insert(winProperty, getProperty(deviceInfo, deviceData, deviceWinProperties.key(winProperty)));
				}
			}
		}
		else if (GetLastError() != ERROR_NO_MORE_ITEMS)
		{
			qCritical("SetupDiEnumDeviceInterfaces failed. Error code: %ld", GetLastError());
			break;
		}
	}

	SetupDiDestroyDeviceInfoList(deviceInfo);

	delete [] detailedData;

	return true;
}

//--------------------------------------------------------------------------------
void SystemDeviceUtils::mergeRegistryDeviceProperties(TWinDeviceProperties & aDeviceProperties, const TWinDeviceProperties & aDeviceRegistryProperties, TSourceDeviceData & aSourceDeviceData)
{
	QStringList devicePortNames = aDeviceProperties.keys();

	for (auto it = aDeviceRegistryProperties.begin(); it != aDeviceRegistryProperties.end(); ++it)
	{
		QString portName = getRelevantPortName(it.key(), devicePortNames);

		if (!portName.isEmpty())
		{
			aSourceDeviceData[portName] << it->path;

			TWinProperties & winDeviceProperties = aDeviceProperties[portName].data;
			QStringList winDevicePropertyNames = winDeviceProperties.keys();

			for (auto jt = it->data.begin(); jt != it->data.end(); ++jt)
			{
				QString propertyName = getRelevantWinProperty(jt.key(), winDevicePropertyNames);

				if (propertyName.isEmpty())
				{
					winDeviceProperties.insert(jt.key(), jt.value());
				}
				else
				{
					QString propertyData = winDeviceProperties[propertyName].simplified().toLower();
					QString registryData = jt.value().simplified().toLower();

					bool oneEmpty = propertyData.isEmpty() != registryData.isEmpty();
					bool allEmpty = propertyData.isEmpty() && registryData.isEmpty();
					bool contains = propertyData.contains(registryData) || registryData.contains(propertyData);

					if (oneEmpty || (!allEmpty && !contains))
					{
						winDeviceProperties.insert(jt.key(), jt.value());
					}
				}
			}
		}
		else
		{
			aDeviceProperties.insert(it.key(), it.value());
			aSourceDeviceData[it.key()] << it->path;
		}
	}
}

//--------------------------------------------------------------------------------
QString SystemDeviceUtils::getDeviceOutKey(const QStringList & aKeys)
{
	int maxNameSize = getMaxSize(aKeys);
	QString result;

	foreach(const QString & key, aKeys)
	{
		if (maxNameSize == CRegistrySerialPort::UuidSize)
		{
			result += QString("\n%1 %2 %1").arg(QString(CRegistrySerialPort::LineSize, '-')).arg(key);
		}
		else
		{
			if (CRegistrySerialPort::MaxUuidSize > maxNameSize)
			{
				int line1Size = (CRegistrySerialPort::MaxUuidSize - key.size() - 2) / 2;
				int line2Size = CRegistrySerialPort::MaxUuidSize - line1Size - key.size() - 2;

				result += QString("\n%1 %2 %3").arg(QString(line1Size, '-')).arg(key).arg(QString(line2Size, '-'));
			}
			else if (key.size() < maxNameSize)
			{
				int line1Size = (maxNameSize - key.size() - 2) / 2;
				int line2Size = maxNameSize - line1Size - key.size() - 2;

				result += QString("\n%1 %2 %3").arg(QString(line1Size, '-')).arg(key).arg(QString(line2Size, '-'));
			}
			else
			{
				result += "\n" + key;
			}
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
QString SystemDeviceUtils::getDeviceOutData(const TWinProperties & aWinPropertyData)
{
	QStringList dataKeys = aWinPropertyData.keys();

	QStringList outDataKeys = dataKeys.filter(CDeviceWinProperties::Prefix);
	dataKeys = (dataKeys.toSet() - outDataKeys.toSet()).toList();
	dataKeys.sort();
	outDataKeys.sort();
	outDataKeys += dataKeys;

	QString result;
	int maxNameSize = getMaxSize(outDataKeys);

	foreach(const QString & outKey, outDataKeys)
	{
		result += QString("\n%1 = %2").arg(outKey, maxNameSize).arg(aWinPropertyData[outKey]);
	}

	return result;
}

//--------------------------------------------------------------------------------
int SystemDeviceUtils::getMaxSize(const QStringList & aBuffer)
{
	int result = 0;

	foreach(auto element, aBuffer)
	{
		result = qMax(result, element.size());
	}

	return result;
}

//--------------------------------------------------------------------------------
QString SystemDeviceUtils::getScreenedData(const QString & aData)
{
	QString result = aData;

	for (int i = 0; i < sizeof(CRegistrySerialPort::RegexSymbols); ++i)
	{
		QChar ch(CRegistrySerialPort::RegexSymbols[i]);
		result = result.replace(ch, ASCII::BackSlash + ch);
	}

	return result;
}

//--------------------------------------------------------------------------------
bool operator !=(const COMMTIMEOUTS & aLeft, const COMMTIMEOUTS & aRight)
{
	return (aLeft.ReadIntervalTimeout         != aRight.ReadIntervalTimeout)         ||
	       (aLeft.ReadTotalTimeoutMultiplier  != aRight.ReadTotalTimeoutMultiplier)  ||
	       (aLeft.ReadTotalTimeoutConstant    != aRight.ReadTotalTimeoutConstant)    ||
	       (aLeft.WriteTotalTimeoutMultiplier != aRight.WriteTotalTimeoutMultiplier) ||
	       (aLeft.WriteTotalTimeoutConstant   != aRight.WriteTotalTimeoutConstant);
}

//--------------------------------------------------------------------------------
bool operator ==(const DCB & aLeft, const DCB & aRight)
{
	return (aLeft.BaudRate    == aRight.BaudRate)    &&
	       (aLeft.ByteSize    == aRight.ByteSize)    &&
	       (aLeft.StopBits    == aRight.StopBits)    &&
	       (aLeft.fRtsControl == aRight.fRtsControl) &&
	       (aLeft.fDtrControl == aRight.fDtrControl) &&
	       (aLeft.Parity      == aRight.Parity);
}

//--------------------------------------------------------------------------------
