/* @file Список компонентов для разработки плагинов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace Driver {

//---------------------------------------------------------------------------
/// Драйверы являются общим расширением.
const char Application[] = "Common";

//---------------------------------------------------------------------------
/// Список возможных компонентов.
namespace CComponents
{
	const char Driver[] = "Driver";
	const char IOPort[] = "IOPort";
	const char BillAcceptor[] = "BillAcceptor";
	const char CoinAcceptor[] = "CoinAcceptor";
	const char Dispenser[] = "Dispenser";
	const char Printer[] = "Printer";
	const char FiscalRegistrator[] = "FiscalRegistrator";
	const char DocumentPrinter[] = "DocumentPrinter";
	const char Watchdog[] = "Watchdog";
	const char Modem[] = "Modem";
	const char Scanner[] = "Scanner";
	const char CardReader[] = "CardReader";
	const char Health[] = "Health";
	const char Camera[] = "Camera";
	const char Token[] = "Token";

	inline bool isPrinter(const QString & aDeviceType)
	{
		return aDeviceType == Printer ||
			aDeviceType == DocumentPrinter ||
			aDeviceType == FiscalRegistrator;
	}
}

//---------------------------------------------------------------------------
}} // SDK::Driver

//---------------------------------------------------------------------------

