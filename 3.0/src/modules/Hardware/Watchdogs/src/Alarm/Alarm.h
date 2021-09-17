/* @file Сторожевой таймер Alarm. */

#pragma once

#include "Hardware/Watchdogs/WatchdogBase.h"

//----------------------------------------------------------------------------
class Alarm: public WatchdogBase
{
	SET_SERIES("Alarm")
	SET_VCOM_DATA(Types::Adapter, ConnectionTypes::VCOMOnly, AdapterTags::FTDI)

public:
	Alarm();

	/// Размыкание линии питания модема.
	virtual bool reset(const QString & aLine);

protected:
	/// Идентифицирует устройство.
	virtual bool isConnected();

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	typedef QSet<char> TAnswer;
	bool getAnswer(TAnswer & aAnswer);
};

//----------------------------------------------------------------------------
