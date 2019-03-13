/* @file Базовый ФР на протоколе АТОЛ3. */

#pragma once

// Modules
#include "Hardware/Protocols/FR/Atol3FR.h"

// Project
#include "../AtolFRBase.h"

//--------------------------------------------------------------------------------
class Atol3FRBase : public AtolFRBase
{
	SET_SERIES("ATOL3")

	Atol3FRBase();

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Выполнить команду.
	virtual TResult performCommand(const QByteArray & aCommandData, QByteArray & aAnswer, int aTimeout);

	/// Протокол.
	Atol3FRProtocol mProtocol;

	/// Id задачи.
	uchar mTId;
};

//--------------------------------------------------------------------------------
