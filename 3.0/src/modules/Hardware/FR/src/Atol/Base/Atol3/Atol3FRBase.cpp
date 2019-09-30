/* @file Базовый ФР на протоколе АТОЛ3. */

#pragma once

#include "Atol3FRBase.h"
#include "Atol3FRBaseConstants.h"

using namespace ProtocolUtils;

//--------------------------------------------------------------------------------
Atol3FRBase::Atol3FRBase(): mTId(0)
{}

//--------------------------------------------------------------------------------
bool Atol3FRBase::isConnected()
{
	mProtocol.setPort(mIOPort);
	mProtocol.setLog(mLog);
	mProtocol.cancel();

	return AtolFRBase::isConnected();
}

//--------------------------------------------------------------------------------
TResult Atol3FRBase::performCommand(const QByteArray & aCommandData, QByteArray & aAnswer, int aTimeout)
{
	mProtocol.setPort(mIOPort);
	mProtocol.setLog(mLog);

	QTime clockTimer;
	clockTimer.start();

	mTId = (mTId == uchar(CAtol3FR::LastTId)) ? ASCII::NUL : ++mTId;
	TResult result = mProtocol.processCommand(mTId, aCommandData, aAnswer);

	using namespace CAtol3FR;

	if (aAnswer[0] == States::InProgress)
	{
		do
		{
			result = mProtocol.waitForAnswer(aAnswer, CAtol3FR::Timeouts::WaitForAnswer);
		}
		while ((clockTimer.elapsed() < aTimeout) && ((result == CommandResult::NoAnswer) || (aAnswer[0] == States::InProgress)));
	}

	if (!result)
	{
		mProtocol.cancel();

		return result;
	}

	auto answerResult = [&] (const QString aLog) -> TResult { if (!aLog.isEmpty()) toLog(LogLevel::Error, mDeviceName + QString(": %1, aborting").arg(aLog));
		mProtocol.cancel(); return CommandResult::Answer; };

	if (aAnswer.isEmpty())
	{
		toLog(LogLevel::Error, mDeviceName + ": No task state, trying to get the result");

		if (!mProtocol.getResult(mTId, aAnswer) || aAnswer.isEmpty())
		{
			return answerResult("No task state again");
		}
	}

	char state = aAnswer[0];
	aAnswer = aAnswer.mid(1);

	if ((state == States::AsyncResult) || (state == States::AsyncError))
	{
		if (!aAnswer.isEmpty())
		{
			uchar TId = uchar(aAnswer[0]);

			if (mTId != TId)
			{
				return answerResult(QString("Invalid task Id = %1, need %2").arg(toHexLog(TId)).arg(toHexLog(mTId)));
			}

			aAnswer = aAnswer.mid(1);
		}
		else
		{
			toLog(LogLevel::Error, mDeviceName + ": No task Id, trying to get the result");

			if (!mProtocol.getResult(mTId, aAnswer) || aAnswer.isEmpty())
			{
				return answerResult("No task Id again");
			}
		}
	}

	if ((state == States::Result) || (state == States::AsyncResult))
	{
		mProtocol.sendACK(mTId);

		return CommandResult::OK;
	}
	else if ((state == States::Error) || (state == States::AsyncError))
	{
		mProtocol.cancel();

		return CommandResult::OK;
	}

	return answerResult(QString("Task %1 state = %2").arg(toHexLog(mTId)).arg(toHexLog(state)));
}

//--------------------------------------------------------------------------------
