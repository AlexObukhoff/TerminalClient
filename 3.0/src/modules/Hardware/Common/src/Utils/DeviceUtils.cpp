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
