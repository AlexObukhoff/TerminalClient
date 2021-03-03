/* @file Базовый протокол. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDateTime>
#include <QtCore/QtEndian>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/IIOPort.h>

// Common
#include <Common/ILogable.h>
#include <Common/SleepHelper.h>

// Project
#include "Hardware/Common/CommandResults.h"
#include "Hardware/Protocols/Common/ProtocolUtils.h"
#include "Hardware/Protocols/Common/ProtocolNames.h"

//--------------------------------------------------------------------------------
class ProtocolBase : public ILogable
{
public:
	ProtocolBase() : mPort(nullptr) {}

	/// Установить порт.
	void setPort(SDK::Driver::IIOPort * aPort)
	{
		mPort = aPort;
	}

protected:
	/// Порт.
	SDK::Driver::IIOPort * mPort;
};

//--------------------------------------------------------------------------------
