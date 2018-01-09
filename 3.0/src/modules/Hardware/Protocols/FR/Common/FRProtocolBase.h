/* @file Базовый протокол фискальников. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDateTime>
#include <QtCore/QTime>
#include <Common/QtHeadersEnd.h>

// Devices
#include "Hardware/Common/CodecDescriptions.h"
#include "Hardware/Printers/PrinterConstants.h"
#include "Hardware/Protocols/Common/DeviceProtocolBase.h"
#include "Hardware/Protocols/FR/IFRProtocol.h"
#include "Hardware/FR/FRStatusCodes.h"

//--------------------------------------------------------------------------------
/// Базовый класс протокола FRProtocolBase.
class FRProtocolBase : public DeviceProtocolBase<IFRProtocol>
{
public:
	FRProtocolBase();

	/// Открыта ли сессия.
	virtual bool isSessionOpened();

protected:
	/// Локальный кодек.
	QTextCodec * mCodec;

	/// Открыта ли сессия.
	bool mSessionOpened;

	/// Необходимо выполнить Z-отчет (для ФР без буфера).
	bool mNeedCloseSession;
};

//--------------------------------------------------------------------------------
