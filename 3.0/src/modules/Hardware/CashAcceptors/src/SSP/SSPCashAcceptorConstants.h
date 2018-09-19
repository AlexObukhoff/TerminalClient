/* @file Константы купюроприемника на протоколе SSP. */

#pragma once

// Modules
#include "Hardware/Common/DeviceCodeSpecification.h"
#include "Hardware/Common/WaitingData.h"

// Project
#include "Hardware/CashAcceptors/CashAcceptorStatusCodes.h"

//--------------------------------------------------------------------------------
namespace CSSP
{
	/// Адреса устройств
	namespace Addresses
	{
		const char Validator    = 0x00;    /// Купюроприемник.
		const char Hopper       = 0x10;    /// Хоппер.
		const char Printer      = 0x40;    /// Принтер Smart Ticket.
	}

	namespace Result
	{
		struct SData
		{
			int code;
			QString description;

			SData() : code(CommandResult::OK) {}
			SData(int aCode, const QString & aDescription) : code(aCode), description(aDescription) {}
		};

		class CData: public CSpecification<char, SData>
		{
		public:
			CData()
			{
				append('\xF0', SData(CommandResult::OK,     "OK"));
				append('\xF2', SData(CommandResult::Driver, "Unknown command"));
				append('\xF3', SData(CommandResult::Driver, "Wrong parameter quantity"));
				append('\xF4', SData(CommandResult::Driver, "Wrong parameter(s)"));
				append('\xF5', SData(CommandResult::Device, "Runtime error"));
				append('\xF6', SData(CommandResult::Device, "Firmware error"));
				append('\xF8', SData(CommandResult::Device, "Command fail"));
				append('\xFA', SData(CommandResult::Device, "Key not set"));

				setDefault(SData(CommandResult::Driver, "Unknown error"));
			}
		};

		static CData Data;
	}

	/// Размер блока данных 1 номинала в ответе на запрос номиналов.
	const int NominalSize = 4;

	/// Стартовый номер протокола при поиске актуальной поддерживаемой версии протокола.
	const int StartingProtocolNumber = 4;

	/// Таймаут после Reset-а, [мс].
	const double NominalMultiplier = 0.01;

	/// Ожидание готовности, [мс].
	const SWaitingData ReadyWaiting = SWaitingData(150, 3 * 1000);

	/// Виртуальный статус Enabled.
	const char EnabledStatus[] = "Enabled status";

	//--------------------------------------------------------------------------------
	/// Команды.
	namespace Commands
	{
		const char GetBillTable       = '\x00';    /// Запрос таблицы номиналов.
		const char Reset              = '\x01';    /// Перезагрузка.
		const char SetInhibit         = '\x02';    /// Установить запрещения номиналов.
		const char GetSetupData       = '\x05';    /// Получить данные устройства.
		const char SetProtocolVersion = '\x06';    /// Попробовать установить версию протокола.
		const char Poll               = '\x07';    /// Запрос статуса.
		const char Return             = '\x08';    /// Выбросить купюру.
		const char Disable            = '\x09';    /// Выключить на прием денег.
		const char Enable             = '\x0A';    /// Включить на прием денег.
		const char GetSerial          = '\x0C';    /// Запрос серийного номера.
		const char Sync               = '\x11';    /// Синхронизация байта последовательности.
		const char GetVersion         = '\x20';    /// Запрос версии прошивки.
		const char Stack              = '\x43';    /// Уложить в стекер.
	}

	//--------------------------------------------------------------------------------
	/// Спецификация статусов.
	class DeviceCodeSpecification : public CommonDeviceCodeSpecification
	{
	public:
		DeviceCodeSpecification()
		{
			/// Состояния.
			addStatus('\xE0', BillAcceptorStatusCode::BillOperation::Unknown, "Note path open");
			addStatus('\xE4', DeviceStatusCode::OK::Initialization, "Stacker replaced");
			addStatus('\xE1', DeviceStatusCode::OK::Initialization, "Note cleared from front (power up with note in validator)");
			addStatus('\xE2', DeviceStatusCode::OK::Initialization, "Note cleared into stacker (power up with note about in stacker)");
			addStatus('\xB6', DeviceStatusCode::OK::Initialization);
			addStatus('\xF1', DeviceStatusCode::OK::Initialization, "Reseting after power up");
			addStatus('\xE8', BillAcceptorStatusCode::Normal::Disabled);
			addStatus('\xB5', BillAcceptorStatusCode::Normal::Disabled, "all note channels have been inhibited");
			addStatus('\xEF', BillAcceptorStatusCode::BillOperation::Accepting, "reading note");
			addStatus('\xEE', BillAcceptorStatusCode::BillOperation::Escrow);
			addStatus('\xCC', BillAcceptorStatusCode::BillOperation::Stacking);
			addStatus('\xEB', BillAcceptorStatusCode::BillOperation::Stacked);
			addStatus('\xED', BillAcceptorStatusCode::Busy::Returning);
			addStatus('\xEC', BillAcceptorStatusCode::Busy::Returned);

			/// Ошибки.
			addStatus('\xE6', BillAcceptorStatusCode::Warning::Cheated);
			addStatus('\xEA', BillAcceptorStatusCode::MechanicFailure::JammedInStacker, "safely jammed");
			addStatus('\xE9', BillAcceptorStatusCode::MechanicFailure::JammedInValidator, "unsafely jammed");
			addStatus('\xE7', BillAcceptorStatusCode::MechanicFailure::StackerFull);
			addStatus('\xE3', BillAcceptorStatusCode::MechanicFailure::StackerOpen);
			addStatus('\xE3', DeviceStatusCode::OK::OK, "after stacker open");

			/// Режекты.
			addStatus('\x01', BillAcceptorStatusCode::Reject::Length);
			addStatus('\x02', BillAcceptorStatusCode::Reject::Verification);
			addStatus('\x03', BillAcceptorStatusCode::Reject::Verification);
			addStatus('\x04', BillAcceptorStatusCode::Reject::Verification);
			addStatus('\x05', BillAcceptorStatusCode::Reject::Verification);
			addStatus('\x06', BillAcceptorStatusCode::Reject::InhibitNote);
			addStatus('\x07', BillAcceptorStatusCode::Reject::LengthDoubling);
			addStatus('\x08', BillAcceptorStatusCode::Busy::Returned);
			addStatus('\x09', BillAcceptorStatusCode::Reject::Verification);
			addStatus('\x0A', BillAcceptorStatusCode::Reject::Verification, "Invalid note read");
			addStatus('\x0B', BillAcceptorStatusCode::Reject::Length, "Note too long");
			addStatus('\x0C', BillAcceptorStatusCode::Reject::UserDefined, "Validator disabled");
			addStatus('\x0D', BillAcceptorStatusCode::Reject::Cheated, "Mechanism slow/stalled");
			addStatus('\x0E', BillAcceptorStatusCode::Reject::Cheated, "Strimming attempt");
			addStatus('\x0F', BillAcceptorStatusCode::Reject::InhibitNote, "Fraud channel reject");
			addStatus('\x10', BillAcceptorStatusCode::Reject::Insertion, "No notes inserted");
			addStatus('\x11', BillAcceptorStatusCode::Reject::Verification, "Peak detect fail");
			addStatus('\x12', BillAcceptorStatusCode::Reject::WidthDoubling);
			addStatus('\x13', BillAcceptorStatusCode::Reject::EscrowTimeout);
			addStatus('\x14', BillAcceptorStatusCode::Reject::Barcode);
			addStatus('\x15', BillAcceptorStatusCode::Reject::Verification, "Invalid note read");
			addStatus('\x16', BillAcceptorStatusCode::Reject::Verification, "Invalid note read");
			addStatus('\x17', BillAcceptorStatusCode::Reject::Verification, "Invalid note read");
			addStatus('\x18', BillAcceptorStatusCode::Reject::Verification, "Invalid note read");
			addStatus('\x19', BillAcceptorStatusCode::Reject::Width);
			addStatus('\x1A', BillAcceptorStatusCode::Reject::Length, "Note too short");
		}

		/// Получить спецификации девайс-кодов по байт-массиву. байт-массив не должен содержать лишних байтов перед статусными байтами.
		virtual void getSpecification(const QByteArray & aBuffer, TDeviceCodeSpecifications & aSpecifications)
		{
			if (aBuffer == EnabledStatus)
			{
				aSpecifications.insert(aBuffer.toHex().data(), SDeviceCodeSpecification(BillAcceptorStatusCode::Normal::Enabled, ""));
			}
			else
			{
				CommonDeviceCodeSpecification::getSpecification(aBuffer, aSpecifications);
			}
		}
	};
}

//--------------------------------------------------------------------------------
