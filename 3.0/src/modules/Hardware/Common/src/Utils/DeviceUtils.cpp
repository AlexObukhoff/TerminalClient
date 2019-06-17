/* @file Утилиты для логики устройств. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <QtCore/QRegExp>
#include <Common/QtHeadersEnd.h>

// Project
#include "Hardware/Common/ASCII.h"
#include "DeviceUtils.h"

//--------------------------------------------------------------------------------
bool DeviceUtils::isComplexFirmwareOld(const QString & aFirmware, const QString & aActualFirmware)
{
	QStringList actualFirmwareData = QString(aActualFirmware).split(ASCII::Dot);
	QStringList devicefirmwareData = QString(aFirmware).replace(QRegExp("[^0-9\\.]+"), "").split(ASCII::Dot);

	for (int i = 0; i < actualFirmwareData.size(); ++i)
	{
		if (i == devicefirmwareData.size())
		{
			QStringList actualFirmwareRest = actualFirmwareData.mid(i);
			bool actualFirmwareRestGreater = std::find_if(actualFirmwareRest.begin(), actualFirmwareRest.end(), [] (const QString & aSection) -> bool
				{ return aSection.toInt(); }) != actualFirmwareRest.end();

			return (actualFirmwareData[i - 1].toInt()  > devicefirmwareData[i - 1].toInt()) ||
			      ((actualFirmwareData[i - 1].toInt() == devicefirmwareData[i - 1].toInt()) && actualFirmwareRestGreater);
		}

		if (actualFirmwareData[i].toInt() > devicefirmwareData[i].toInt())
		{
			return true;
		}
	}

	return false;
}

//--------------------------------------------------------------------------------
QString DeviceUtils::getPartDeviceData(const TDeviceData & aData, bool aHideEmpty, int aLevel)
{
	QStringList keys = aData.keys();
	int maxSize = 0;

	foreach(auto key, keys)
	{
		maxSize = qMax(maxSize, key.size());
	}

	keys.sort();
	QString result;

	for (int i = 0; i < keys.size(); ++i)
	{
		QString key = keys[i];
		QString value = aData[key];

		if (!aHideEmpty || !value.isEmpty())
		{
			key = QString(aLevel, ASCII::TAB) + key + QString(maxSize - key.size(), QChar(ASCII::Space));
			result += QString("\n%1 : %2").arg(key).arg(value);
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
