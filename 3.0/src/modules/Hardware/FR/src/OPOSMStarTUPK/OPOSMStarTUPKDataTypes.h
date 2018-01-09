/* @file Типы данных MStar TUP-K на OPOS-драйвере. */

#pragma once

// System
#include "WinDef.h"

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

//--------------------------------------------------------------------------------
namespace COPOSMStarTUPK
{
	/// Параметры.
	namespace Parameters
	{
		enum Enum
		{
			AutoCutter,
			TaxesPrint,
			ZBuffer
		};

		struct Data
		{
			QString name;
			DWORD value;

			Data() : name(""), value(0) {}
			Data(const char * aName, DWORD aValue) : name(aName), value(aValue) {}
		};
	}

	struct SErrorData
	{
		QString function;
		int error;

		SErrorData() : error(0) {}
		SErrorData(const QString & aFunction, int aError) : function(aFunction), error(aError) {}

		bool operator==(const SErrorData & aErrorData) const
		{
			return (aErrorData.function == function) && (aErrorData.error == error);
		}
	};
}

//--------------------------------------------------------------------------------
