/* @file Константы купюроприемника на протоколе CCNet. */

#pragma once

// Modules
#include "Hardware/Common/ASCII.h"
#include "Hardware/Common/DeviceCodeSpecification.h"
#include "Hardware/Common/WaitingData.h"

// Project
#include "Hardware/CashAcceptors/CashAcceptorStatusCodes.h"

//--------------------------------------------------------------------------------
namespace CCCNet
{
	/// ACK.
	const char ACK = ASCII::NUL;

	/// Первый индекс монет (считая с 0) в списке номиналов для SM-13xx.
	const int MinCoinIndex = 8;

	/// Последний индекс монет (считая с 0) в списке номиналов для SM-13xx.
	const int MaxCoinIndex = 11;

	/// Высокий уровень контроля подлинности (ASCII::NUL - нижкий уровень)
	const char HighSecurityLevel = ASCII::Full;

	/// Размер блока данных 1 номинала в ответе на запрос номиналов.
	const int NominalSize = 5;

	/// Таймауты, [мс].
	namespace Timeouts
	{
		/// После Reset-а.
		const int Reset = 3000;

		/// Выход из initilaize-а (SL).
		const int ExitInitialize = 40 * 1000;

		/// Ожидание выхода из Busy при обработке блока данных в процессе обновления прошивки.
		const int BusyUpdatingStatus = 30 * 1000;

		/// Ожидание получения полла от купюрника перед перепрошивкой.
		const int UpdatingAvailable = 1000;
	}

	/// Ожидание выхода из Busy-подобных состояний, [мс].
	const SWaitingData NotBusyPowerUpWaiting = SWaitingData(100, 4 * 1000);

	/// Ожидание выхода из анабиоза, [мс].
	const SWaitingData AvailableWaiting = SWaitingData(100, 500);

	/// Ожидание на отсеивание других устройств на автоопределении, [мс].
	const SWaitingData FalseAutoDetectionWaiting = SWaitingData(100, 1000);

	/// Ожидание выхода из Busy, [мс].
	const SWaitingData NotBusyWaiting = SWaitingData(100, 4000);

	/// Ожидание выхода из Enabled, [мс].
	const SWaitingData NotEnabled = SWaitingData(100, 1000);

	/// Пауза между командами обновления прошивки, [мс].
	const int UpdatingPause = 200;

	/// Пауза после смены скорости порта, [мс].
	const int ChangingBaudratePause = 300;

	/// Пауза между записью блоков данных и выходоим из режима обновления прошивки, [мс].
	const int ExitUpdatingPause = 10;

	/// Пауза при автопоиске для MFL.
	const int IdentificationPause = 2000;

	/// Максимальное количество повторов при записи блока данных, если нет ответа.
	const int WriteFirmwareDataMaxRepeats = 3;

	/// Базовые индексы прошивок, работающих с монетами.
	namespace FirmwareCoinSupportedMinBase
	{
		const int Horizontal = 1300;    /// для горизонтальной загрузки банкнот.
		const int Vertical   = 1400;    /// для вертикальной загрузки банкнот.
	}

	/// Таймаут ожидания окончания инициализации перед обновлением прошивки.
	const int WaitUpdatingTimeout = 30 * 1000;

	//--------------------------------------------------------------------------------
	/// Адреса устройств
	namespace Addresses
	{
		const char NoDevice     = 0x00;    /// Неизвестное.
		const char BillToBill   = 0x01;    /// B2B (отдает сдачу принятыми ранее купюрами).
		const char CoinChanger  = 0x02;    /// Монетоприемник.
		const char Validator    = 0x03;    /// Купюроприемник.
		const char Cardreader   = 0x04;    /// Кардридер.
	}

	//--------------------------------------------------------------------------------
	/// Команды
	namespace Commands
	{
		const char Reset[]           = "\x30";
		const char SetSecurity[]     = "\x32";
		const char GetStatus[]       = "\x33";
		const char EnableBillTypes[] = "\x34";
		const char Stack[]           = "\x35";
		const char Return[]          = "\x36";
		const char GetVersion[]      = "\x37";
		const char GetParList[]      = "\x41";
		const char UpdateFirmware[]  = "\x50";

		/// Обновление прошивки.
		namespace UpdatingFirmware
		{
			const QByteArray GetStatus = QByteArray::fromRawData("\x50\x00", 2);
			const char GetBlockSize[]  = "\x50\x01";
			const char Write[]         = "\x50\x02";
			const char Exit[]          = "\x50\x03";
			const char SetBaudRate[]   = "\x50\x05";
		}
	}

	/// Обновление прошивки.
	namespace UpdatingFirmware
	{
		/// Базовый адрес записи блоков.
		const int BaseAddress = 0xB000;

		/// Минимальная прошивка, начиная с которой отлажен запрос статуса.
		const int FirmwareStatusOK = 1350;

		/// Ответы на команды обновления прошивки.
		namespace Answers
		{
			const char OK = '\xE0';
			const char Error = '\xE1';

			using namespace SDK::Driver;

			/// Описатель ответа
			struct SData
			{
				EWarningLevel::Enum warningLevel;
				QString description; 

				SData() : warningLevel(EWarningLevel::OK) {}
				SData(EWarningLevel::Enum aWarningLevel, const QString & aDescription) : warningLevel(aWarningLevel), description(aDescription) {}
			};

			/// Спецификация статусов.
			class Specifications: public CSpecification<char, SData>
			{
			public:
				Specifications()
				{
					append('\x00', SData(EWarningLevel::OK, "Ready"));
					append('\xE0', SData(EWarningLevel::OK, "OK"));
					append('\xE1', SData(EWarningLevel::Error, "Unknown error"));
					append('\xE2', SData(EWarningLevel::Error, "CRC error"));
					append('\xE3', SData(EWarningLevel::Error, "Access denied (group or sequrity error) or CRC error"));
					append('\xE4', SData(EWarningLevel::Warning, "Busy"));
					append('\xE5', SData(EWarningLevel::Error, "Firmware is not accorded with the device"));

					setDefault(SData(EWarningLevel::Error, "Unknown state"));
				}
			};

			static Specifications Specification;
		}
	}

	//--------------------------------------------------------------------------------
	/// Спецификация статусов.
	class DeviceCodeSpecification: public CommonDeviceCodeSpecification
	{
	public:
		DeviceCodeSpecification()
		{
			/// Обычные состояния.
			addStatus('\x19', BillAcceptorStatusCode::Normal::Disabled);
			addStatus('\x10', DeviceStatusCode::OK::Initialization, "PowerUp");
			addStatus('\x11', DeviceStatusCode::OK::Initialization, "PowerUp with bill in validator");
			addStatus('\x12', DeviceStatusCode::OK::Initialization, "PowerUp with bill in stacker");
			addStatus('\x13', DeviceStatusCode::OK::Initialization);

			/// Состояния с купюрой.
			addStatus('\x14', BillAcceptorStatusCode::Normal::Enabled);
			addStatus('\x15', BillAcceptorStatusCode::BillOperation::Accepting);
			addStatus('\x80', BillAcceptorStatusCode::BillOperation::Escrow);
			addStatus('\x17', BillAcceptorStatusCode::BillOperation::Stacking);
			addStatus('\x81', BillAcceptorStatusCode::BillOperation::Stacked);
			addStatus('\x18', BillAcceptorStatusCode::Busy::Returning);
			addStatus('\x82', BillAcceptorStatusCode::Busy::Returned);
			addStatus('\x46', BillAcceptorStatusCode::Busy::Pause);

			/// Состояния редко встречаются или встречаются в B2B и пр. такого типа устройствах.
			addStatus('\x1E', BillAcceptorStatusCode::BillOperation::Unloading);
			addStatus('\x26', BillAcceptorStatusCode::BillOperation::Unloaded);
			addStatus('\x21', BillAcceptorStatusCode::Busy::SettingStackerType);
			addStatus('\x29', BillAcceptorStatusCode::Busy::SetStackerType);
			addStatus('\x1D', BillAcceptorStatusCode::BillOperation::Dispensing);
			addStatus('\x25', BillAcceptorStatusCode::BillOperation::Dispensed);
			addStatus('\x1B', DeviceStatusCode::OK::Initialization);
			addStatus('\x1A', BillAcceptorStatusCode::BillOperation::Holding);
			addStatus('\x28', BillAcceptorStatusCode::OperationError::Unknown, "Invalid bill number");
			addStatus('\x30', BillAcceptorStatusCode::OperationError::Unknown, "Illegal command");

			/// Ошибки.
			addStatus('\x41', BillAcceptorStatusCode::MechanicFailure::StackerFull);
			addStatus('\x42', BillAcceptorStatusCode::MechanicFailure::StackerOpen);
			addStatus('\x43', BillAcceptorStatusCode::MechanicFailure::JammedInValidator);
			addStatus('\x44', BillAcceptorStatusCode::MechanicFailure::JammedInStacker);
			addStatus('\x45', BillAcceptorStatusCode::Warning::Cheated);

			/// Выбросы.
			setExtraCodeDefault('\x1C', BillAcceptorStatusCode::Reject::Unknown);
			addStatus('\x60', BillAcceptorStatusCode::Reject::Insertion);
			addStatus('\x61', BillAcceptorStatusCode::Reject::Dielectric);
			addStatus('\x62', BillAcceptorStatusCode::Reject::PreviousOperating);
			addStatus('\x63', BillAcceptorStatusCode::Reject::Compensation);
			addStatus('\x64', BillAcceptorStatusCode::Reject::Transport);
			addStatus('\x65', BillAcceptorStatusCode::Reject::Identification);
			addStatus('\x66', BillAcceptorStatusCode::Reject::Verification);
			addStatus('\x67', BillAcceptorStatusCode::Reject::OpticalSensor);
			addStatus('\x68', BillAcceptorStatusCode::Reject::InhibitNote);
			addStatus('\x69', BillAcceptorStatusCode::Reject::CapacitanceSensor);
			addStatus('\x6A', BillAcceptorStatusCode::Reject::Operation);
			addStatus('\x6C', BillAcceptorStatusCode::Reject::Length);
			addStatus('\x92', BillAcceptorStatusCode::Reject::Identification, "Unrecognised bill");
			addStatus('\x6D', BillAcceptorStatusCode::Reject::UVSensor);
			addStatus('\x93', BillAcceptorStatusCode::Reject::Barcode);
			addStatus('\x94', BillAcceptorStatusCode::Reject::Barcode, "Error in barcode beginning");
			addStatus('\x95', BillAcceptorStatusCode::Reject::Barcode, "Error in barcode end");

			/// Неисправности.
			setExtraCodeDefault('\x47', DeviceStatusCode::Error::Unknown);
			addStatus('\x10', DeviceStatusCode::Warning::Developing, "Unable to create object");
			addStatus('\x11', DeviceStatusCode::Warning::Developing, "Object timeout");
			addStatus('\x12', DeviceStatusCode::Warning::Developing, "Object access error");
			addStatus('\x13', DeviceStatusCode::Warning::Developing, "Timer access error");
			addStatus('\x14', DeviceStatusCode::Warning::Developing, "Task access error");
			addStatus('\x15', DeviceStatusCode::Error::MemoryStorage);
			addStatus('\x22', DeviceStatusCode::Error::RecoveryMode);
			addStatus('\x23', BillAcceptorStatusCode::Error::Calibration);
			addStatus('\x30', DeviceStatusCode::Error::Boot);
			addStatus('\x50', BillAcceptorStatusCode::MechanicFailure::StackerMotor);
			addStatus('\x51', BillAcceptorStatusCode::MechanicFailure::TransportMotor, "Transport motor speed error");
			addStatus('\x52', BillAcceptorStatusCode::MechanicFailure::TransportMotor);
			addStatus('\x53', BillAcceptorStatusCode::MechanicFailure::AligningMotor);
			addStatus('\x54', BillAcceptorStatusCode::MechanicFailure::Stacker, "Initial stacker status is not correct");
			addStatus('\x55', BillAcceptorStatusCode::SensorError::Optical);
			addStatus('\x56', BillAcceptorStatusCode::SensorError::Magnetic);
			addStatus('\x57', DeviceStatusCode::Error::Electronic, "Upper optical sensor board error");
			addStatus('\x58', DeviceStatusCode::Error::Electronic, "Lower optical sensor board error");
			addStatus('\x59', DeviceStatusCode::Error::Electronic, "Upper peripheral sensor board error");
			addStatus('\x5F', BillAcceptorStatusCode::SensorError::Dielectric);
			addStatus('\x60', DeviceStatusCode::Error::Electronic, "Lower peripheral sensor board error");
			addStatus('\xF4', BillAcceptorStatusCode::MechanicFailure::Stacker, "Unknown stacker failure");
			addStatus('\xFE', DeviceStatusCode::Error::CoverIsOpened);
		}

		//TODO: при рефакторинге - завязать на статус-код
		bool isBusy(const QByteArray & aBuffer)
		{
			return !aBuffer.isEmpty() && (aBuffer[0] == '\x1B');
		}

		//TODO: при рефакторинге - сделать статус PowerUp, завязать на статус-код и сделать фильтр 2-го уровня
		bool isPowerUp(const QByteArray & aBuffer)
		{
			return !aBuffer.isEmpty() && ((aBuffer[0] == '\x10') || (aBuffer[0] == '\x11') || (aBuffer[0] == '\x12'));
		}
	};
}

//--------------------------------------------------------------------------------
