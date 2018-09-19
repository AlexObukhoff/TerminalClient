/* @file Константы купюроприемника на протоколе ICT. */

#pragma once

#include "Hardware/CashAcceptors/CashAcceptorStatusCodes.h"
#include "Hardware/Common/DeviceCodeSpecification.h"

//--------------------------------------------------------------------------------
namespace CICTBase
{
	/// Таймаут после Reset-а, [мс].
	const int ResetTimeout = 200;

	/// Ожидание PowerUp-а, [мс].
	const SWaitingData PowerUpWaiting = SWaitingData(100, 5 * 1000);

	//--------------------------------------------------------------------------------
	/// Команды.
	namespace Commands
	{
		const char ACK     = '\x02';
		const char NAK     = '\x0F';
		const char Reset   = '\x30';
		const char Poll    = '\x0C';
		const char Disable = '\x5E';
		const char Enable  = '\x3E';
		const char Identification[] = {ACK, Disable, Poll, Enable, Poll, Disable, Poll};
	}

	/// Ответы.
	namespace Answers
	{
		const char Identification[] = "\x5E\x3E\x5E";
	}

	//--------------------------------------------------------------------------------
	/// Состояния (на которые завязана логика протокола).
	namespace States
	{
		const char Idling        = '\x3E';       /// Включен на прием купюр.
		const char Disabled      = '\x5E';       /// Отключен для приема купюр.
		const char PowerUp[]     = "\x80\x8F";   /// Включили питание.
		const char Escrow        = '\x81';       /// Временное депонирование.
		const char ErrorExlusion = '\x2F';       /// Предыдущая ошибка перестала быть.
	}

	//--------------------------------------------------------------------------------
	/// Спецификация статусов.
	class DeviceCodeSpecification : public CommonDeviceCodeSpecification
	{
	public:
		DeviceCodeSpecification()
		{
			/// OK.
			appendStatus('\x5E', BillAcceptorStatusCode::Normal::Disabled);
			appendStatus('\x3E', BillAcceptorStatusCode::Normal::Enabled);
			appendStatus('\x80', DeviceStatusCode::OK::Initialization, "PowerUp");
			appendStatus('\x81', BillAcceptorStatusCode::BillOperation::Escrow);
			appendStatus('\x10', BillAcceptorStatusCode::BillOperation::Stacked);
			appendStatus('\x23', BillAcceptorStatusCode::Busy::Returned);
			appendStatus('\x29', BillAcceptorStatusCode::Reject::Unknown);
			appendStatus('\x2F', DeviceStatusCode::OK::OK, "Error status exlusion");

			/// Ворнинги.
			appendStatus('\x27', BillAcceptorStatusCode::Warning::Cheated);

			/// Ошибки операций.
			appendStatus('\x11', BillAcceptorStatusCode::OperationError::Stack);
			appendStatus('\x21', BillAcceptorStatusCode::OperationError::Unknown, "Wrong check sum");
			appendStatus('\x2A', BillAcceptorStatusCode::OperationError::HostCommand, "Wrong command");

			/// Ошибки.
			appendStatus('\x25', BillAcceptorStatusCode::SensorError::Optical);

			/// Неисправности.
			appendStatus('\x20', BillAcceptorStatusCode::MechanicFailure::TransportMotor);
			appendStatus('\x22', BillAcceptorStatusCode::MechanicFailure::JammedInValidator);
			appendStatus('\x24', BillAcceptorStatusCode::MechanicFailure::StackerOpen);
			appendStatus('\x26', DeviceStatusCode::Error::Unknown, "Communication error - terminal hasn`t answer for powerUp status");
			appendStatus('\x28', BillAcceptorStatusCode::MechanicFailure::StackerMotor);
		}

		virtual void getSpecification(const QByteArray & aBuffer, TDeviceCodeSpecifications & aSpecifications)
		{
			QByteArray buffer(aBuffer);

			while(!buffer.isEmpty())
			{
				CommonDeviceCodeSpecification::getSpecification(buffer, aSpecifications);
				int size = (aBuffer.startsWith(States::PowerUp) || (aBuffer[0] == States::Escrow)) ? 2 : 1;
				buffer.remove(0, size);
			}
		}

		bool contains(char aCode)
		{
			return mBuffer.contains(aCode);
		}

		//TODO: при рефакторинге - сделать статус PowerUp, завязать на статус-код и сделать фильтр 2-го уровня
		bool isPowerUp(const QByteArray & aBuffer)
		{
			return !aBuffer.isEmpty() && aBuffer.contains('\x80');
		}
	};
}

//--------------------------------------------------------------------------------
