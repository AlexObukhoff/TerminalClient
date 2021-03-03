/* @file Константы купюроприемника на протоколе SSP. */

#pragma once

// Modules
#include "Hardware/Common/DeviceCodeSpecification.h"
#include "Hardware/Common/WaitingData.h"
#include "Hardware/Protocols/CashAcceptor/SSPDataTypes.h"

// Project
#include "Hardware/CashAcceptors/CashAcceptorStatusCodes.h"

//--------------------------------------------------------------------------------
namespace CSSP
{
	/// Адреса устройств
	namespace Addresses
	{
		const char Validator = 0x00;    /// Купюроприемник, диспенсер (пристегивается к купюрнику).
		const char Hopper    = 0x10;    /// Хоппер.
		const char Printer1  = 0x40;    /// Принтер Smart Ticket (печать по шаблонам или "на лету" (?).
		const char Printer2  = 0x41;    /// Принтер Coupon Printer (печатает на фальцованной бумаге) и Flatbed Printer (обычный термопринтер).
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

	/// Ожидание отвала после резета, [мс].
	const SWaitingData NotReadyWaiting = SWaitingData(150, 1000);

	/// Ожидание готовности, [мс].
	const SWaitingData ReadyWaiting = SWaitingData(150, 20 * 1000);

	/// Виртуальный статус Enabled.
	const char EnabledStatus[] = "Enabled status";

	/// Виртуальный статус Disabled.
	const char DisabledStatus[] = "Disabled status";

	/// Минимальное количество запрещаемых каналов для NV200.
	const int NV200MinInhibitedChannels = 16;

	/// Признак подключения USB-кабеля к головной части.
	const char HeadConnectionId[] = "ITL";

	/// Скорости для перепрошивки.
	namespace EBaudRate
	{
		enum Enum
		{
			BR9600   = 0,
			BR38400  = 1,
			BR115200 = 2
		};
	}

	class CBaudRateData : public CSpecification<EBaudRate::Enum, SDK::Driver::IOPort::COM::EBaudRate::Enum>
	{
	public:
		CBaudRateData()
		{
			append(EBaudRate::BR9600,   SDK::Driver::IOPort::COM::EBaudRate::BR9600);
			append(EBaudRate::BR38400,  SDK::Driver::IOPort::COM::EBaudRate::BR38400);
			append(EBaudRate::BR115200, SDK::Driver::IOPort::COM::EBaudRate::BR115200);
		}
	};

	static CBaudRateData BaudRateData;

	/// Обновление прошивки.
	namespace UpdatingFirmware
	{
		/// Id пакета обновления прошивки.
		const char Id[] = "ITL";

		/// ACK.
		const char ACK = '\x32';

		/// Изменение скорости останется после резета.
		const char ContinuousBaudrate = 1;

		/// Размер заголовка.
		const int HeaderSize = 128;

		/// Размер блока данных.
		const int BlockSize = 128;

		/// Дефолтный таймаут ответа при перепрошивке, [мс].
		const int DefaultTimeout = 1000;

		/// Ожидание выхода из анабиоза по завершении перепрошивки, [мс].
		const SWaitingData FinalizationWaiting = SWaitingData(500, 3 * 1000);

		/// Паузы, [мс].
		namespace Pause
		{
			/// После смены скорости порта.
			const int ChangingBaudrate = 1000;

			/// Для применения загруженого сегмента RAM.
			const int RAMApplying = 500;

			/// Перед записью блока данных.
			const int BlockWriting = 10;

			/// После успешной записи прошивки для её применения.
			const int EndApplying = 15 * 1000;
		}
	}

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
		const char GetFirmware        = '\x20';    /// Запрос версии прошивки.
		const char GetDataset         = '\x21';    /// Запрос версии биллсета.
		const char Stack              = '\x43';    /// Уложить в стекер.
		const char SetBaudrate        = '\x4D';    /// Установить скорость порта.
		const char GetBuild           = '\x4F';    /// Запрос версии билда.

		/// Обновление прошивки.
		namespace UpdatingFirmware
		{
			const char GetBlockSize[]  = "\x0B\x03";
		}

		class CData: public CSpecification<QByteArray, SData>
		{
		public:
			CData()
			{
				add(GetSetupData, 1000);
				add(SetBaudrate, 1000);
				add(Sync, true);
			}

		private:
			void add(char aCommand, int aTimeout)  { append(QByteArray(1, aCommand), SData(aTimeout, false)); }
			void add(char aCommand, bool aSetSync) { append(QByteArray(1, aCommand), SData(DefaultTimeout, aSetSync)); }
		};

		static CData Data;
	}

	//--------------------------------------------------------------------------------
	/// Escrow или Accepting.
	const char EAStatusCode = '\xEF';

	/// Escrow или Accepting.
	const char StackingStarted = '\xEE';

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
			addStatus('\xEF', BillAcceptorStatusCode::BillOperation::Escrow, "reading note");
			addStatus('\xEE', BillAcceptorStatusCode::BillOperation::Stacked);    // только уложена
			addStatus('\xCC', BillAcceptorStatusCode::BillOperation::Stacking);
			addStatus('\xEB', BillAcceptorStatusCode::Normal::Enabled, "stacking completed");    // как бы финальный Stacked, выдается одновременно с последним Stacked-ом.
			addStatus('\xED', BillAcceptorStatusCode::Reject::Rejecting);
			addStatus('\xEC', BillAcceptorStatusCode::Reject::Unknown);

			/// Ошибки.
			addStatus('\xE6', BillAcceptorStatusCode::Warning::Cheated);
			addStatus('\xE9', BillAcceptorStatusCode::MechanicFailure::JammedInValidator, "unsafely jammed");
			addStatus('\xE7', BillAcceptorStatusCode::MechanicFailure::StackerFull);
			addStatus('\xE3', BillAcceptorStatusCode::MechanicFailure::StackerOpen);

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
				aSpecifications.insert("", SDeviceCodeSpecification(BillAcceptorStatusCode::Normal::Enabled, ""));
			}
			else if (aBuffer == DisabledStatus)
			{
				aSpecifications.insert("", SDeviceCodeSpecification(BillAcceptorStatusCode::Normal::Disabled, ""));
			}
			else
			{
				CommonDeviceCodeSpecification::getSpecification(aBuffer, aSpecifications);

				if (!aSpecifications.isEmpty())
				{
					int & statusCode = aSpecifications.begin()->statusCode;

					if ((statusCode == BillAcceptorStatusCode::BillOperation::Escrow) && (aBuffer.size() >= 2) && !aBuffer[1])
					{
						statusCode = BillAcceptorStatusCode::BillOperation::Accepting;
					}
				}
			}
		}
	};
}

//--------------------------------------------------------------------------------
