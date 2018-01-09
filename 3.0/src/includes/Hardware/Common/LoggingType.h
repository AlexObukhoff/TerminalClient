/* @file Тип логгирования сообщений устройств. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMetaType>
#include <Common/QtHeadersEnd.h>

//--------------------------------------------------------------------------------
namespace ELoggingType
{
	enum Enum
	{
		None = 0,
		Write,
		ReadWrite
	};
}

Q_DECLARE_METATYPE(ELoggingType::Enum);

//--------------------------------------------------------------------------------
