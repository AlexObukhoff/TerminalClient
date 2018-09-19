/* @file Константы купюроприемника на протоколе V2e. */

#pragma once

// STL
#include <algorithm>

// Modules
#include "Hardware/Common/ASCII.h"
#include "Hardware/Common/DeviceCodeSpecification.h"
#include "Hardware/Common/WaitingData.h"

// Project
#include "Hardware/CashAcceptors/CashAcceptorStatusCodes.h"

//--------------------------------------------------------------------------------
/// Константы, команды и коды состояний устройств на протоколе V2e.
namespace CV2e
{
	/// IRQ.
	const char IRQ = '\x55';

	/// Число байтов id прошивки.
	const int FirmwareBytesAmount = 16;

	/// Таймауты, [мс].
	namespace Timeouts
	{
		/// Выход из Busy.
		const int Busy = 5 * 1000;

		/// Устойчивое не-Busy состояние.
		const int ReliableNonBusy = 1 * 1000;
	}

	/// Интервалы поллинга.
	namespace PollingIntervals
	{
		/// При включении на прием денег.
		const int Enabled = 360;
	}

	/// Режим связи с устройством - поллинг, эскроу разрешен.
	const char CommunicationMode = 0x01;

	/// Разрешить прием купюр любой стороной в любом направлении.
	char AllNoteDirections = ASCII::NUL;

	/// Количество максимальных повторов.
	const int MaxRepeat = 3;

	/// Размер блока данных 1 номинала в ответе на запрос номиналов.
	const int NominalSize = 6;

	/// Байт протокола ID.
	const char ProtocolID = 1;

	/// Команды V2e протокола для валидатора.
	namespace Commands
	{
		const char Poll           = '\xCC';   /// Статус.
		const char Identification = '\xD8';   /// Идентификация.
		const char Stack          = '\x80';   /// Уложить в стекер.
		const char Return         = '\x81';   /// Вернуть.
		const char Reset          = '\x36';   /// Сброс.
		const char SetBillEnables = '\x41';   /// Включить на прием денег.
		const char GetParTable    = '\xE5';   /// Получить таблицу номиналов.
		const char SetCommMode    = '\x40';   /// Установить режим (polling/interrupt) и доступность эскроу.
		const char SetOrientation = '\x43';   /// Установить ориентацию купюры.
		const char Retransmit     = '\x77';   /// Переспросить ответ.
		const char Uninhibit      = '\xF1';   /// Попытаться выйти из Inhibit-а.
		const char ChangeDefault  = '\xF3';   /// Сохранить настройки для Power-up-а.
	}

	//--------------------------------------------------------------------------------
	/// Спецификация статусов.
	class DeviceCodeSpecification : public BitmapDeviceCodeSpecification
	{
	public:
		DeviceCodeSpecification()
		{
			/// Операции с купюрой.
			addStatus(0, 7, BillAcceptorStatusCode::BillOperation::Stacked);
			addStatus(0, 6, BillAcceptorStatusCode::Busy::Returned);
			addStatus(0, 5, BillAcceptorStatusCode::BillOperation::Escrow);

			addStatus(1, 7, BillAcceptorStatusCode::Warning::Cheated);
			addStatus(1, 6, DeviceStatusCode::OK::Initialization, "Reset owing to power-on");
			addStatus(1, 5, BillAcceptorStatusCode::OperationError::Escrow);
			addStatus(1, 4, BillAcceptorStatusCode::Reject::Unknown);
			addStatus(1, 3, BillAcceptorStatusCode::BillOperation::Stacking);
			addStatus(1, 2, BillAcceptorStatusCode::Busy::Returning);
			addStatus(1, 1, BillAcceptorStatusCode::BillOperation::Accepting);
			addStatus(1, 0, BillAcceptorStatusCode::Normal::Enabled);

			addStatus(2, 7, BillAcceptorStatusCode::Normal::Inhibit, "Inhibit state by communication time-out");
			addStatus(2, 6, DeviceStatusCode::Error::Unknown);
			addStatus(2, 5, BillAcceptorStatusCode::OperationError::Communication, "Communication error due to answer absence to IRQ request");
			addStatus(2, 4, DeviceStatusCode::OK::Initialization, "Power-up with bill-in-channel");
			addStatus(2, 3, BillAcceptorStatusCode::MechanicFailure::StackerFull);
			addStatus(2, 2, BillAcceptorStatusCode::MechanicFailure::StackerOpen);
			addStatus(2, 1, BillAcceptorStatusCode::MechanicFailure::JammedInStacker);
			addStatus(2, 0, BillAcceptorStatusCode::MechanicFailure::JammedInValidator);

			addStatus(4, 0, BillAcceptorStatusCode::BillOperation::Stacking, "Bill on stacker conveyor");

			/// Группа дополнительных девайс-кодов.
			mErrors.addStatus('\x07', BillAcceptorStatusCode::Reject::Verification);  // TODO: vs Identification?
			mErrors.addStatus('\x08', BillAcceptorStatusCode::Reject::InhibitNote);
			mErrors.addStatus('\x0B', BillAcceptorStatusCode::Reject::DoubleCorrelation);
			mErrors.addStatus('\x0C', BillAcceptorStatusCode::Reject::MagneticSensor);
			mErrors.addStatus('\x0D', BillAcceptorStatusCode::Reject::Length, "Bill is unable to pass through chamber");
			mErrors.addStatus('\x0E', BillAcceptorStatusCode::OperationError::Accept);
			mErrors.addStatus('\x0F', BillAcceptorStatusCode::OperationError::Return);  // TODO: vs Reject?
			mErrors.addStatus('\x10', BillAcceptorStatusCode::Reject::InhibitNote, "All bills are inhibited");
			mErrors.addStatus('\x11', BillAcceptorStatusCode::OperationError::Stack);
			mErrors.addStatus('\x12', BillAcceptorStatusCode::Busy::Returned, "Bill rejected by controller");
		}

		/// Получить спецификации девайс-кодов по байт-массиву. байт-массив не должен содержать лишних байтов перед статусными байтами.
		virtual void getSpecification(const QByteArray & aBuffer, TDeviceCodeSpecifications & aSpecifications)
		{
			BitmapDeviceCodeSpecification::getSpecification(aBuffer, aSpecifications);

			if ((aBuffer.size() > 1) && (aBuffer[1] & 0x10))
			{
				mErrors.getSpecification(QByteArray(1, aBuffer[1]), aSpecifications);
			}
		}

		//TODO: при рефакторинге - завязать на статус-код
		bool isBusy(const QByteArray & aBuffer)
		{
			TDeviceCodeSpecifications specifications;
			getSpecification(aBuffer, specifications);

			return std::find_if(specifications.begin(), specifications.end(), [&] (const SDeviceCodeSpecification & aSpecification) -> bool { int statusCode = aSpecification.statusCode;
				return (statusCode == DeviceStatusCode::OK::Initialization) ||
				       (statusCode == BillAcceptorStatusCode::OperationError::Communication); }) != specifications.end();
		}

	protected:
		/// Спецификация дополнительных девайс-кодов.
		CommonDeviceCodeSpecification mErrors;
	};
}
//--------------------------------------------------------------------------------
