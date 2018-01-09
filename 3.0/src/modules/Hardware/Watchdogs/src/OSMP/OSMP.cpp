/* @file Сторожевой таймер OSMP. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <Common/QtHeadersEnd.h>

// Project
#include "OSMP.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
OSMP::OSMP()
{
	// Данные порта.
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
	mPortParameters[EParameters::Parity].append(EParity::No);

	// Данные устройства.
	mDeviceName = "OSMP Watchdog";

	// Реальное время таймаута = 25 минут, пинг раз в 7 минут.
	mPingTimer.setInterval(7 * 60 * 1000);

	mData[EOSMPCommandId::IdentificationData] = "1.00";

	// Команды.
	mData[EOSMPCommandId::Identification] = "OSP\x01";
	mData[EOSMPCommandId::ResetModem] = "OSP\x02";
	mData[EOSMPCommandId::StartTimer] = "OSP\x03";
	mData[EOSMPCommandId::StopTimer]  = "OSP\x04";
	mData[EOSMPCommandId::Ping]       = "OSP\x05";
	mData[EOSMPCommandId::RebootPC]   = "OSP\xAE";
}

//----------------------------------------------------------------------------
bool OSMP::isConnected()
{
	QByteArray answer;

	if (!performCommand(mData[EOSMPCommandId::Identification], &answer))
	{
		return false;
	}

	answer = ProtocolUtils::clean(answer);
	QRegExp regExp("WDT.*v([0-9\\.]+)");

	if ((regExp.indexIn(answer) == -1) || (regExp.capturedTexts()[1] != mData[EOSMPCommandId::IdentificationData]))
	{
		return false;
	}

	if (!mConnected)
	{
		if (performCommand(COSMP::WrongDeviceCheck, &answer) && !answer.isEmpty())
		{
			toLog(LogLevel::Error, mDeviceName + ": Unknown device trying to impersonate the device based on OSMP protocol.");
			return false;
		}

		mIOPort->close();
		mIOPort->open();
		SleepHelper::msleep(COSMP::ReopenPortPause);
	}

	return true;
}

//----------------------------------------------------------------------------
//TODO: сделать свич на линию питания.
bool OSMP::reset(const QString & aLine)
{
	if (!checkConnectionAbility())
	{
		return false;
	}

	if (aLine == SDK::Driver::LineTypes::Modem)
	{
		return performCommand(mData[EOSMPCommandId::ResetModem]);
	}
	else if (aLine == SDK::Driver::LineTypes::Terminal)
	{
		return performCommand(mData[EOSMPCommandId::RebootPC]);
	}

	return false;
}

//----------------------------------------------------------------------------
bool OSMP::performCommand(const QByteArray & aCommand, QByteArray * aAnswer)
{
	MutexLocker lock(&mExternalMutex);

	QByteArray data;
	QByteArray & answer = aAnswer ? *aAnswer : data;
	answer.clear();

	if (!mIOPort->write(aCommand))
	{
		return false;
	}

	SleepHelper::msleep(100);

	if ((aCommand != mData[EOSMPCommandId::Identification]) &&
		(aCommand != mData[EOSMPCommandId::RebootPC]) &&
		(aCommand != mData[EOSMPCommandId::GetSensorStatus]))
	{
		return true;
	}

	return mIOPort->read(answer) && !answer.isEmpty();
}

//----------------------------------------------------------------------------
void OSMP::setPingEnable(bool aEnabled)
{
	WatchdogBase::setPingEnable(aEnabled);

	EOSMPCommandId::Enum commandId = aEnabled ? EOSMPCommandId::StartTimer : EOSMPCommandId::StopTimer;
	performCommand(mData[commandId]);
}

//-----------------------------------------------------------------------------
void OSMP::onPing()
{
	performCommand(mData[EOSMPCommandId::Ping]);
}

//--------------------------------------------------------------------------------
