/* @file Сторожевой таймер Alarm. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QBuffer>
#include <Common/QtHeadersEnd.h>

// Project
#include "Alarm.h"
#include "AlarmData.h"

using namespace SDK::Driver::IOPort::COM;

//----------------------------------------------------------------------------
Alarm::Alarm()
{
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
	mPortParameters[EParameters::Parity].append(EParity::No);

	mDeviceName = "Alarm";
	mSensorDisabledValue = true;
}

//----------------------------------------------------------------------------
bool Alarm::isConnected()
{
	CAlarm::CCommandIntervals CommandIntervals;

	for (auto it = CommandIntervals.data().begin(); it != CommandIntervals.data().end(); ++it)
	{
		TAnswer answer;

		if (!mIOPort->write(QByteArray(1, it.key())) || !getAnswer(answer) || answer.isEmpty())
		{
			return false;
		}

		CAlarm::TInterval interval = it.value();

		if (std::find_if(answer.begin(), answer.end(), [&] (char state) -> bool { return qBound(it.value().first, uchar(state), it.value().second) == uchar(state); }) == answer.end())
		{
			return false;
		}
	}

	return true;
}

//----------------------------------------------------------------------------
bool Alarm::reset(const QString & /*aLine*/)
{
	if (!checkConnectionAbility())
	{
		return false;
	}

	toLog(LogLevel::Normal, mDeviceName + ": resetting modem");

	return mIOPort->write(QByteArray(1, CAlarm::Commands::ResetModem));
}

//---------------------------------------------------------------------------
bool Alarm::getStatus(TStatusCodes & aStatusCodes)
{
	TAnswer answer;

	if (!getAnswer(answer))
	{
		return false;
	}

	if (answer.isEmpty())
	{
		return isConnected();
	}

	foreach (char state, answer)
	{
		for (auto it = CAlarm::SensorCodeSpecification.data().begin(); it != CAlarm::SensorCodeSpecification.data().end(); ++it)
		{
			if ((state >> it.key()) & 1)
			{
				aStatusCodes << it.value();
			}
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool Alarm::getAnswer(TAnswer & aAnswer)
{
	MutexLocker locker(&mExternalMutex);

	aAnswer.clear();
	QByteArray answer;

	QTime clockTimer;
	clockTimer.restart();

	do
	{
		if (!mIOPort->read(answer, 100))
		{
			return false;
		}
	}
	while ((clockTimer.elapsed() < CAlarm::DefaultTimeout) && answer.isEmpty());

	for (int i = 0; i < answer.size(); ++i)
	{
		aAnswer << answer[i];
	}

	return true;
}

//--------------------------------------------------------------------------------------
