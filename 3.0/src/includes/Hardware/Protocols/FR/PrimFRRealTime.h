/* @file Протокол реал-тайм запросов ФР ПРИМ. */

#pragma once

#include "Hardware/Common/ProtocolBase.h"
#include "Hardware/Common/ASCII.h"

/// Маски ответов на запрос статуса выполнения команды.
namespace CPrimFR { namespace CommandResultMask
{
	const char CommandVerified = '\x04';    /// Команда распознана.
	const char CommandComplete = '\x08';    /// Команда выполнена.
	const char PrinterError    = '\x20';    /// Ошибка принтера.
	const char PrinterMode     = '\x40';    /// Работа в режиме принтера.
}}

//--------------------------------------------------------------------------------
class PrimFRRealTimeProtocol : public ProtocolBase
{
public:
	TResult processCommand(int aCommand, char & aAnswer)
	{
		QByteArray command = ASCII::DLE + QByteArray::number(aCommand);
		QByteArray answer;

		if (!mPort->write(command) ||
			!mPort->read(answer, 50)) return CommandResult::Port;
		if (answer.isEmpty())         return CommandResult::NoAnswer;

		if (answer.size() > 1)
		{
			QByteArray data;
			QTime timer;
			timer.start();

			do
			{
				if (!mPort->read(data, 50))
				{
					return CommandResult::Port;
				}
			}
			while (!data.isEmpty() && (timer.elapsed() < 1000));

			if (!data.isEmpty())          return CommandResult::Answer;
			if (!mPort->write(command) ||
				!mPort->read(answer, 50)) return CommandResult::Port;
			if (answer.isEmpty())         return CommandResult::NoAnswer;
			if (answer.size() != 1)       return CommandResult::Answer;
		}

		aAnswer = answer[0];

		return CommandResult::OK;
	}
};

//--------------------------------------------------------------------------------
