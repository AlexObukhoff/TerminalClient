/* @file Утилиты для логики устройств. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

namespace DeviceUtils
{
	/// Проверить сложную (вида x.y.z.0...) прошивку на актуальность.
	bool isComplexFirmwareOld(const QString & aFirmware, const QString & aActualFirmware);
}

//--------------------------------------------------------------------------------
