/* Реализация протокола с EFTPOS 3.0 компании Uniteller. */

#pragma once

// Project headers
#include "API.h"

namespace Uniteller
{

//---------------------------------------------------------------------------
QString API::translateErrorMessage(const QString & aError, const QString & aMessage) const
{
	QString fullErrorCode = aError;

	if (aError == "09" || aError == "0B" || aError == "0C")
	{
		fullErrorCode = QString("%01%02").arg(aError).arg(aMessage.left(2));
	}

	if (fullErrorCode == "0G")
	{
		return QObject::tr("#0G00");
	}
	else if (fullErrorCode == "0X")
	{
		return QObject::tr("#0X00");
	}

	switch (fullErrorCode.toInt(nullptr, 16))
	{
		case 0x0100: return QObject::tr("#0100");
		case 0x0200: return QObject::tr("#0200");
		case 0x0300: return QObject::tr("#0300");
		case 0x0400: return QObject::tr("#0400");
		case 0x0500: return QObject::tr("#0500");
		case 0x0600: return QObject::tr("#0600");
		case 0x0700: return QObject::tr("#0700");
		case 0x0701: return QObject::tr("#0701");
		case 0x0702: return QObject::tr("#0702");
		case 0x0703: return QObject::tr("#0703");
		case 0x0800: return QObject::tr("#0800");
		case 0x0900: return QObject::tr("#0900");
		case 0x0901: return QObject::tr("#0901");
		case 0x0902: return QObject::tr("#0902");
		case 0x0903: return QObject::tr("#0903");
		case 0x0904: return QObject::tr("#0904");
		case 0x0A00: return QObject::tr("#0A00");
		case 0x0B00: return QObject::tr("#0B00");
		case 0x0B01: return QObject::tr("#0B01");
		case 0x0B02: return QObject::tr("#0B02");
		case 0x0B03: return QObject::tr("#0B03");
		case 0x0B04: return QObject::tr("#0B04");
		case 0x0B05: return QObject::tr("#0B05");
		case 0x0B06: return QObject::tr("#0B06");
		case 0x0B07: return QObject::tr("#0B07");
		case 0x0B08: return QObject::tr("#0B08");
		case 0x0B09: return QObject::tr("#0B09");
		case 0x0B0A: return QObject::tr("#0B0A");
		case 0x0B0B: return QObject::tr("#0B0B");
		case 0x0B0C: return QObject::tr("#0B0C");
		case 0x0B0D: return QObject::tr("#0B0D");
		case 0x0B0E: return QObject::tr("#0B0E");
		case 0x0B0F: return QObject::tr("#0B0F");
		case 0x0B10: return QObject::tr("#0B10");
		case 0x0B11: return QObject::tr("#0B11");
		case 0x0B12: return QObject::tr("#0B12");
		case 0x0B13: return QObject::tr("#0B13");
		case 0x0B14: return QObject::tr("#0B14");
		case 0x0B15: return QObject::tr("#0B15");
		case 0x0B16: return QObject::tr("#0B16");
		case 0x0B17: return QObject::tr("#0B17");
		case 0x0B18: return QObject::tr("#0B18");
		case 0x0B19: return QObject::tr("#0B19");
		case 0x0B1A: return QObject::tr("#0B1A");
		case 0x0B1B: return QObject::tr("#0B1B");
		case 0x0B1C: return QObject::tr("#0B1C");
		case 0x0B1D: return QObject::tr("#0B1D");
		case 0x0B1E: return QObject::tr("#0B1E");
		case 0x0B1F: return QObject::tr("#0B1F");
		case 0x0B20: return QObject::tr("#0B20");
		case 0x0B21: return QObject::tr("#0B21");
		case 0x0B22: return QObject::tr("#0B22");
		case 0x0B23: return QObject::tr("#0B23");
		case 0x0B24: return QObject::tr("#0B24");
		case 0x0B25: return QObject::tr("#0B25");
		case 0x0B26: return QObject::tr("#0B26");
		case 0x0B27: return QObject::tr("#0B27");
		case 0x0B28: return QObject::tr("#0B28");
		case 0x0B29: return QObject::tr("#0B29");
		case 0x0B2A: return QObject::tr("#0B2A");
		case 0x0B2B: return QObject::tr("#0B2B");
		case 0x0B2C: return QObject::tr("#0B2C");
		case 0x0B2D: return QObject::tr("#0B2D");
		case 0x0B2E: return QObject::tr("#0B2E");
		case 0x0B2F: return QObject::tr("#0B2F");
		case 0x0B30: return QObject::tr("#0B30");
		case 0x0B31: return QObject::tr("#0B31");
		case 0x0B32: return QObject::tr("#0B32");
		case 0x0B33: return QObject::tr("#0B33");
		case 0x0B34: return QObject::tr("#0B34");
		case 0x0B35: return QObject::tr("#0B35");
		case 0x0B36: return QObject::tr("#0B36");
		case 0x0B37: return QObject::tr("#0B37");
		case 0x0B38: return QObject::tr("#0B38");
		case 0x0B39: return QObject::tr("#0B39");
		case 0x0B3A: return QObject::tr("#0B3A");
		case 0x0B3B: return QObject::tr("#0B3B");
		case 0x0B3C: return QObject::tr("#0B3C");
		case 0x0B3D: return QObject::tr("#0B3D");
		case 0x0B3E: return QObject::tr("#0B3E");
		case 0x0B3F: return QObject::tr("#0B3F");
		case 0x0B40: return QObject::tr("#0B40");
		case 0x0B41: return QObject::tr("#0B41");
		case 0x0B42: return QObject::tr("#0B42");
		case 0x0C00: return QObject::tr("#0C00");
		case 0x0C01: return QObject::tr("#0C01");
		case 0x0D00: return QObject::tr("#0D00");
		case 0x0E00: return QObject::tr("#0E00");
		case 0x1E00: return QObject::tr("#1E00");
		case 0x0F00: return QObject::tr("#0F00");
		case 0x1000: return QObject::tr("#1000");
		case 0x1100: return QObject::tr("#1100");
		case 0x3100: return QObject::tr("#3100");
		case 0x6200: return QObject::tr("#6200");
	}
	
	return QString();
}

} // Uniteller

//---------------------------------------------------------------------------
