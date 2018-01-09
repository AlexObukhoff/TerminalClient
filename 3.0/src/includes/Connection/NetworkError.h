/* @file Класс исключений сетевого соединения. */

#pragma once

// Модули
#include <Common/Exception.h>

//--------------------------------------------------------------------------------
class NetworkError : public Exception
{
public:
	NetworkError(ECategory::Enum aCategory, ESeverity::Enum aSeverity, int aCode, const QString & aMessage = QString())
		: Exception(aCategory, aSeverity, aCode, aMessage) {}
};

//--------------------------------------------------------------------------------
