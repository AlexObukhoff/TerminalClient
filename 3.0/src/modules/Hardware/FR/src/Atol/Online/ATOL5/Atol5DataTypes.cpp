/* @file Типы данных онлайн ФР на платформе АТОЛ5. */

#pragma once

// STL
#include <algorithm>

// Project
#include "Atol5DataTypes.h"

using namespace CAtol5OnlineFR;

//--------------------------------------------------------------------------------
CIgnoredCommands::CIgnoredCommands()
{
}

//--------------------------------------------------------------------------------
CIgnoredCommands::CIgnoredCommands(const TIgnoredCommands & aOther)
{
	operator = (aOther);
}

//--------------------------------------------------------------------------------
CIgnoredCommands & CIgnoredCommands::operator = (const TIgnoredCommands & aOther)
{
	TIgnoredCommands * pSelf = dynamic_cast<TIgnoredCommands *>(this);
	*pSelf = aOther;

	return *this;
}

//--------------------------------------------------------------------------------
CIgnoredCommands & CIgnoredCommands::operator << (const QByteArray & aData)
{
	append(aData);

	return *this;
}

//--------------------------------------------------------------------------------
CIgnoredCommands & CIgnoredCommands::operator << (char aData)
{
	append(QByteArray(1, aData));

	return *this;
}

//--------------------------------------------------------------------------------
bool CIgnoredCommands::containsData(const QByteArray & aData) const
{
	return std::find_if(begin(), end(), [&] (const QByteArray & aOwnData) -> bool { return aData.startsWith(aOwnData); }) != end();
}

//--------------------------------------------------------------------------------
