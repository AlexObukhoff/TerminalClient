/* @file Сторожевой таймер OSMP 2.5. */

#include "OSMP2.5.h"
#include "OSMP2.5Data.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
OSMP25::OSMP25()
{
	// Данные порта.
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
	mPortParameters[EParameters::Parity].append(EParity::No);

	// Данные устройства.
	mDeviceName = "OSMP 2.5";
	mIOMessageLogging = ELoggingType::None;
	setConfigParameter(CHardware::Watchdog::CanRegisterKey, true);
	setConfigParameter(CHardware::Watchdog::CanWakeUpPC, true);
	mPingTimer.setInterval(COSMP25::PingInterval);
}

//----------------------------------------------------------------------------
bool OSMP25::isConnected()
{
	QByteArray answer;

	if (!processCommand(COSMP25::Commands::GetVersion, &answer))
	{
		return false;
	}

	if (ProtocolUtils::clean(answer).isEmpty() && isAutoDetecting())
	{
		toLog(LogLevel::Error, mDeviceName + ": Unknown device trying to impersonate this device");
		return false;
	}

	mVerified = answer.contains(COSMP25::ModelTag);
	setDeviceParameter(CDeviceData::Version, answer);

	return true;
}

//--------------------------------------------------------------------------------
bool OSMP25::updateParameters()
{
	QByteArray answer;

	if (processCommand(COSMP25::Commands::SerialNumber, &answer))
	{
		QByteArray typeBuffer = answer.left(2);
		uint type = 0;

		for (int i = 0; i < typeBuffer.size(); ++i)
		{
			type += uint(uchar(typeBuffer[i])) << ((typeBuffer.size() - i - 1) * 8);
		}

		setDeviceParameter(CDeviceData::Type, type);
		setDeviceParameter(CDeviceData::SerialNumber, answer.mid(2).toHex());
	}

	for (int i = 0; i < COSMP25::MaxKeys; ++i)
	{
		if (processCommand(COSMP25::Commands::ReadKey, QByteArray(1, uchar(i)), &answer) && (answer.size() > 1))
		{
			QString key = QString("%1_%2").arg(CDeviceData::Watchdogs::Key).arg(i, 2, 10, QChar(ASCII::Zero));
			setDeviceParameter(key, answer.mid(1, 8).toHex());
			setDeviceParameter(CDeviceData::Type, int(uchar(answer[0])), key);
		}
	}

	return processCommand(COSMP25::Commands::SetModemPause,  QByteArray(1, COSMP25::ModemResettingPause)) &&
	       processCommand(COSMP25::Commands::SetPCPause,     QByteArray(1, COSMP25::PCResettingPause)) &&
	       processCommand(COSMP25::Commands::SetPingTimeout, QByteArray(1, COSMP25::PingTimeout));
}

//----------------------------------------------------------------------------
//TODO: сделать свич на линию питания.
bool OSMP25::reset(const QString & aLine)
{
	if (!checkConnectionAbility())
	{
		return false;
	}

	if (!mStatusCollectionHistory.isEmpty() && (mInitialized == ERequestStatus::Fail))
	{
		toLog(LogLevel::Error, QString("%1: Cannot reset line %2").arg(mDeviceName).arg(aLine));
		return false;
	}

	if (!isWorkingThread() || (mInitialized == ERequestStatus::InProcess))
	{
		QMetaObject::invokeMethod(this, "reset", Qt::BlockingQueuedConnection, Q_ARG(QString, aLine));
	}
	else if (aLine == SDK::Driver::LineTypes::Modem)
	{
		return processCommand(COSMP25::Commands::ResetModem);
	}
	else if (aLine == SDK::Driver::LineTypes::Terminal)
	{
		return processCommand(COSMP25::Commands::ResetPC);
	}

	return false;
}

//---------------------------------------------------------------------------
bool OSMP25::getStatus(TStatusCodes & aStatusCodes)
{
	QTime PCWakingUpTime = getConfigParameter(CHardware::Watchdog::PCWakingUpTime).toTime();

	if (!PCWakingUpTime.isNull() && (PCWakingUpTime != mPCWakingUpTime))
	{
		int secsTo = PCWakingUpTime.secsTo(QTime::currentTime());

		if (secsTo < 0)
		{
			secsTo += 24 * 60 * 60;
		}

		int intervals = qRound(double(secsTo) / COSMP25::PCWakingUpInterval);

		if ((secsTo < COSMP25::PCWakingUpInterval) || (std::abs(COSMP25::PCWakingUpInterval * intervals - secsTo) < COSMP25::PCWakingUpLag))
		{
			QByteArray answer;
			bool resetPCWakingUpTimeResult = true;
			bool needResetPCWakeUpTime = !mPCWakingUpTime.isNull();

			if (!needResetPCWakeUpTime)
			{
				if (!processCommand(COSMP25::Commands::PCWakeUpTime, &answer) && !answer.isEmpty())
				{
					resetPCWakingUpTimeResult = false;
					toLog(LogLevel::Error, mDeviceName + ": Cannot get wake up timeout");
				}
				else
				{
					needResetPCWakeUpTime = answer[0];
				}
			}

			if (needResetPCWakeUpTime && resetPCWakingUpTimeResult && !processCommand(COSMP25::Commands::ResetPCWakeUpTime))
			{
				resetPCWakingUpTimeResult = false;
				toLog(LogLevel::Error, mDeviceName + ": Cannot reset wake up timeout");
			}

			if (resetPCWakingUpTimeResult)
			{
				toLog(LogLevel::Normal, QString("%1: Set wake up timeout to %2 hours -> %3").arg(mDeviceName).arg(intervals / 2.0).arg(PCWakingUpTime.toString(COSMP25::TimeLogFormat)));

				if (processCommand(COSMP25::Commands::PCWakeUpTime, QByteArray(1, uchar(intervals))))
				{
					mPCWakingUpTime = PCWakingUpTime;
				}
			}
		}
	}

	return true;
}

//----------------------------------------------------------------------------
TResult OSMP25::execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer)
{
	MutexLocker lock(&mExternalMutex);

	mProtocol.setPort(mIOPort);
	mProtocol.setLog(mLog);

	return mProtocol.processCommand(aCommand + aCommandData, aAnswer);
}

//----------------------------------------------------------------------------
void OSMP25::setPingEnable(bool aEnabled)
{
	WatchdogBase::setPingEnable(aEnabled);

	char command = aEnabled ? COSMP25::Commands::SetPingEnable : COSMP25::Commands::SetPingDisable;
	processCommand(command);
}

//-----------------------------------------------------------------------------
void OSMP25::onPing()
{
	processCommand(COSMP25::Commands::Ping);
}

//--------------------------------------------------------------------------------
void OSMP25::registerKey()
{
	START_IN_WORKING_THREAD(registerKey)

	QByteArray answer;
	bool result = processCommand(COSMP25::Commands::WriteKey, &answer) && (answer.isEmpty() || (answer[1] != COSMP25::KeyRegisteringExpired));

	emit keyRegistered(result);
}

//--------------------------------------------------------------------------------
