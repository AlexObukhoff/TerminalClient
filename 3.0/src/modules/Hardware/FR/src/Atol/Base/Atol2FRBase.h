/* @file Базовый ФР на протоколе АТОЛ2. */

#pragma once

// Modules
#include "Hardware/Protocols/FR/Atol2FR.h"

// Project
#include "AtolFRBase.h"

//--------------------------------------------------------------------------------
class Atol2FRBase : public AtolFRBase
{
	SET_SERIES("ATOL2")

protected:
	/// Выполнить команду.
	virtual TResult performCommand(const QByteArray & aCommandData, QByteArray & aAnswer, int aTimeout)
	{
		mProtocol.setPort(mIOPort);
		mProtocol.setLog(mLog);

		return mProtocol.processCommand(aCommandData, aAnswer, aTimeout);
	}

	/// Протокол.
	Atol2FRProtocol mProtocol;
};

//--------------------------------------------------------------------------------
