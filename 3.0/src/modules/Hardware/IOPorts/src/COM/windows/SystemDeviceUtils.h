/* @file Набор функционала для работы с системными ресурсами с использованием SetupDi. */

#pragma once

// Windows
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <setupapi.h>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QUuid>
#include <Common/QtHeadersEnd.h>

// Project
#include "Hardware/IOPorts/DeviceWinProperties.h"
#include "Hardware/Common/ASCII.h"

//--------------------------------------------------------------------------------
typedef QMap<QString, QString> TWinProperties;

struct SWinDeviceProperties
{
	QString path;
	quint32 VID;
	quint32 PID;
	TWinProperties data;

	SWinDeviceProperties() : VID(0), PID(0) {}
};

typedef QMap<QString, SWinDeviceProperties> TWinDeviceProperties;
typedef QMap<QString, QStringList> TSourceDeviceData;

/// Работа с реестром.
namespace CRegistrySerialPort
{
	/// Начальный путь для поиска
	const char InitialPath[] = "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Enum";

	/// Начальный путь для быстрого поиска
	const wchar_t QuickInitialPath[] = L"HARDWARE\\DEVICEMAP\\SERIALCOMM";

	/// Cвойство для формирования пути для открытия порта.
	const char PathProperty[] = "FriendlyName";

	/// Зарезервированные символы QRegExp.
	const char RegexSymbols[] = { '+', '-', '*', '?', ')', '(', '{', '}', '^', '.'};

	/// Размер Uuid-а в строковом представлении.
	const int UuidSize = QUuid().toString().size();

	/// Размер линии ключа гуида.
	const int LineSize = 21;

	/// Размер ключа Uuid-а.
	const int MaxUuidSize = UuidSize + 2 * (LineSize + 1);
}

//--------------------------------------------------------------------------------
namespace SystemDeviceUtils
{
	/// Формирует список COM-портов.
	QStringList enumerateCOMPorts();

	/// Формирует список последовательных портов на основе реестра.
	QStringList getSerialDeviceNames();

	/// Формирует список пропертей для устройств по GUID-у.
	bool enumerateSystemDevices(const QUuid & aUuid, TWinDeviceProperties & aDeviceProperties, DWORD aPathProperty, bool aQuick = false);

	/// Формирует список пропертей для устройств поиском по реестру.
	TWinDeviceProperties enumerateRegistryDevices(bool aQuick = false);

	/// Вмерживает недостающие проперти девайсов, полученные из реестра, в проперти, полученные по гуидам.
	void mergeRegistryDeviceProperties(TWinDeviceProperties & aDeviceProperties, const TWinDeviceProperties & aDeviceRegistryProperties, TSourceDeviceData & aSourceDeviceData);

	/// Получает имя порта из группы имен портов, соответствующее имени заданного порта.
	QString getRelevantPortName(const QString & aPortName, const QStringList & aPortNames);

	/// Получает свойство порта из группы свойств порта, соответствующее заданному свойству порта.
	QString getRelevantWinProperty(const QString & aWinProperty, const QStringList & aWinProperties);

	/// Получает значение свойства реестра по хэндлу ключа и id свойства.
	QString getRegKeyValue(HKEY key, LPCTSTR aProperty);

	/// Возвращает свойство системного устройства.
	QString getProperty(const HDEVINFO & hDevInfo, SP_DEVINFO_DATA & aDeviceInfoData, DWORD aProperty);

	/// Получить выходные ключи, по которым были получены данные по устройству.
	QString getDeviceOutKey(const QStringList & aKeys);

	/// Получить выходные данные по устройству.
	QString getDeviceOutData(const TWinProperties & aWinPropertyData);

	/// Получить максимальный размер данных в контейнере.
	int getMaxSize(const QStringList & aBuffer);

	/// Получить данные для регэкспа с экранированными символами.
	QString getScreenedData(const QString & aData);
}

bool operator!=(const COMMTIMEOUTS & aTimeouts_1, const COMMTIMEOUTS & aTimeouts_2);
bool operator==(const DCB & aMine, const DCB & aTheirs);

//--------------------------------------------------------------------------------
