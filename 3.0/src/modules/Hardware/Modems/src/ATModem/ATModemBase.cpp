/* @file Базовый класс AT-совместимого модема. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <Common/QtHeadersEnd.h>

// Project
#include "ATModemBase.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//---------------------------------------------------------------------------------
ATModemBase::ATModemBase()
{
	// данные порта
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);   // preferable for work, but not works in autobauding mode
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR57600);    // default after Mobile Equipment (ME) restart
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR38400);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);

	mPortParameters[EParameters::Parity].append(EParity::No);

	// Отключено - записи в лог дублируются.
	//mIOMessageLogging = true;

	mModemConfigTimeout = CATGSMModem::Timeouts::Config;
}

//--------------------------------------------------------------------------------
bool ATModemBase::checkAT(int aTimeout)
{
	mIOPort->clear();

	int index = 0;
	bool result = false;

	do
	{
		result = processCommand(AT::Commands::AT, aTimeout);
	}
	while ((++index < 5) && !result);

	return result;
}

//--------------------------------------------------------------------------------
bool ATModemBase::isConnected()
{
	if (!checkAT(CATGSMModem::Timeouts::Default))
	{
		mIOPort->close();
		return false;
	}

	enableLocalEcho(false);

	QByteArray answer;

	if (!processCommand(AT::Commands::ATI, answer))
	{
		mIOPort->close();
		return false;
	}
	
	setDeviceName(answer);

	// Выводим конфигурацию модема в лог.
	if (processCommand(AT::Commands::ATandV, answer, mModemConfigTimeout))
	{
		toLog(LogLevel::Normal, QString("Modem configuration: %1").arg(QString::fromLatin1(answer)));
	}

	QString modemInfo;
	getInfo(modemInfo);

	mIOPort->close();

	return true;
}

//--------------------------------------------------------------------------------
void ATModemBase::setDeviceName(const QByteArray & aFullName)
{
	if (!isAutoDetecting())
	{
		toLog(LogLevel::Normal, QString("Full modem info: %1").arg(QString::fromLatin1(aFullName)));
	}

	QString deviceName = aFullName.simplified();

	if (!deviceName.isEmpty())
	{
		mDeviceName = deviceName;
	}
}

//--------------------------------------------------------------------------------
bool ATModemBase::reset()
{
	if (!checkConnectionAbility())
	{
		return false;
	}

	enableLocalEcho(false);

	// Сбрасываем модем.
	toLog(LogLevel::Normal, "Resetting modem to factory defaults...");
	bool result = processCommand(AT::Commands::ATandF0);

	onPoll();

	mIOPort->close();

	return result;
}

//--------------------------------------------------------------------------------
bool ATModemBase::setInitString(const QString & aInitString)
{
	toLog(LogLevel::Normal, QString("Setting initialization string '%1'.").arg(aInitString));

	if (!checkConnectionAbility())
	{
		return false;
	}

	QRegExp regExp("^AT");
	QString initString = QString(aInitString).remove(regExp);

	enableLocalEcho(false);

	bool result = processCommand(initString.toLatin1());

	mIOPort->close();

	return result;
}

//--------------------------------------------------------------------------------
bool ATModemBase::enableLocalEcho(bool aEnable)
{
	QByteArray command = QByteArray(AT::Commands::ATE) + (aEnable ? "1" : "0");

	return processCommand(command);
}

//--------------------------------------------------------------------------------
QByteArray ATModemBase::makeCommand(const QString & aCommand)
{
	return QByteArray("AT").append(aCommand).append(ASCII::CR);
}

//--------------------------------------------------------------------------------
bool ATModemBase::processCommand(const QByteArray & aCommand, int aTimeout)
{
	QByteArray answer;

	return processCommand(aCommand, answer, aTimeout);
}

//--------------------------------------------------------------------------------
bool ATModemBase::processCommand(const QByteArray & aCommand, QByteArray & aAnswer, int aTimeout)
{
	aAnswer.clear();

	if (!mIOPort->write(makeCommand(aCommand)))
	{
		return false;
	}

	bool result = false;

	QTime commandTimer;
	commandTimer.start();

	do
	{
		QByteArray data;

		if (mIOPort->read(data))
		{
			aAnswer.append(data);

			// Ищем сообщения о положительном результате.
			int pos = aAnswer.indexOf("OK");

			if (pos >= 0)
			{
				aAnswer.chop(aAnswer.size() - pos);
				result = true;

				break;
			}

			// Ищем сообщения о отрицательном результате.
			pos = aAnswer.indexOf("ERROR");

			if (pos >= 0)
			{
				aAnswer.chop(aAnswer.size() - pos);
				result = false;

				break;
			}
		}

		SleepHelper::msleep(CATGSMModem::Pauses::AnswerAttempt);
	}
	while (commandTimer.elapsed() < aTimeout);

	// Обязательный таймаут в 100 мс. после каждой операции.
	SleepHelper::msleep(CATGSMModem::Pauses::Answer);

	if (!result)
	{
		toLog(LogLevel::Error, QString("Bad answer : %1").arg(aAnswer.data()));
	}

	return result;
}

//--------------------------------------------------------------------------------
