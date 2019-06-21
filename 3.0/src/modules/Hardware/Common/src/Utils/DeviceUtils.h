/* @file Утилиты для логики устройств. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <Common/QtHeadersEnd.h>

// Project
#include "Hardware/Common/FunctionTypes.h"

//---------------------------------------------------------------------------
// Класс для выполнения функционала при выходе из области видимости.
// Если отпадает необъходимость в его вызове - вызвать reset().
class ExitAction
{
public:
	ExitAction(const TVoidMethod & aAction) : mAction(aAction) {}
	~ExitAction() { if (mAction) mAction(); }

	bool reset() { mAction = TVoidMethod(); return true; }

private:
	TVoidMethod mAction;
};

/// Данные устройств.
typedef QMap<QString, QString> TDeviceData;

//---------------------------------------------------------------------------
namespace DeviceUtils
{
	/// Проверить сложную (вида x.y.z.0...) прошивку на актуальность.
	bool isComplexFirmwareOld(const QString & aFirmware, const QString & aActualFirmware);

	/// Получение заданной компоненты параметров устройства.
	QString getPartDeviceData(const TDeviceData & aData, bool aHideEmpty = true, int aLevel = 0);
}

//--------------------------------------------------------------------------------
