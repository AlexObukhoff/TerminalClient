/* @file Константы кардридеров на протоколе Creator. */

#pragma once

// SDK
#include <SDK/Drivers/ICardReader.h>

// Modules
#include "Hardware/Common/Specifications.h"

// Project
#include "CreatorReaderDataTypes.h"

//--------------------------------------------------------------------------------
namespace CCreatorReader
{
	/// Разделитель прочитанных данных карты
	const char DataSeparator = '\x7E';

	/// Маркеры.
	namespace Markers
	{
		const char Command = 'C';    /// Команда.
		const char OK      = 'P';    /// Ошибок нет.
		const char Error   = 'N';    /// Ошибка.
	}

	//--------------------------------------------------------------------------------
	/// Команды.
	namespace Commands
	{
		const char LockInitialize[]   = "\x30\x30";    /// Инициализация с блокировкой карты.
		const char UnLockInitialize[] = "\x30\x31";    /// Инициализация с разблокировкой карты.
		const char GetSerialNumber[]  = "\xA2\x30";    /// Получение серийного нормера.
		const char GetStatus[]        = "\x31\x30";    /// Получение статуса.
		const char IdentifyIC[]       = "\x50\x30";    /// Автоидентификация IC-карты.
		const char IdentifyRF[]       = "\x50\x31";    /// Автоидентификация RF-карты.
		const char SetMCReadingMode[] = "\x36\x30\x30\x31\x37\x30";    /// Установка режима стения карт с магнитной полосой, читать все треки в ASCII-формате.
		const char ReadMSData[]       = "\x36\x31\x30\x37";    /// Чтение данных с магнитной полосы, читать все треки в ASCII-формате.
		const char PowerReset[]       = "\x51\x30\x30";    /// Сброс аппаратный для EMV-карт (cold reset).
		const char ADPUT0[]           = "\x51\x33";    /// ADPU запрос для карты с протоколом CPU T = 0.
		const char ADPUT1[]           = "\x51\x34";    /// ADPU запрос для карты с протоколом CPU T = 1.
	}

	//--------------------------------------------------------------------------------
	/// Положение карты по отношению к кардридеру.
	namespace CardPosition
	{
		const int Unknown   = -1;

		const int Ejected   = 0;
		const int InProcess = 1;
		const int Inserted  = 2;

		const char ST1 = 1;    /// Постоянная составляющая статуса - ST1.
	}

	//--------------------------------------------------------------------------------
	/// Типы карт.
	namespace CardTypes
	{
		const int Unknown = 0;    /// Неизвестен.

		namespace CardType = SDK::Driver::ECardType;

		/// Описатель для типов карт.
		class CDescriptions : public CDescription<CardType::Enum>
		{
		public:
			CDescriptions()
			{
				setDefault("Unknown");

				append(CardType::MS,     "Magnetic strip");
				append(CardType::MSIC,   "IC with magnetic strip");
				append(CardType::MSICRF, "RF + IC with magnetic strip");
				append(CardType::IC, "IC");
				append(CardType::RF, "RF");
			}
		};

		static CDescriptions Description;

		//--------------------------------------------------------------------------------
		/// Описатель для IC-карт по типу CPU.
		class CICCPUDescriptions : public CDescription<EICCPU::Enum>
		{
		public:
			CICCPUDescriptions()
			{
				setDefault(Unknown);

				append(EICCPU::Unknown, "Unknown");
				append(EICCPU::T0, "T=0 CPU");
				append(EICCPU::T1, "T=1 CPU");
			}
		};

		static CICCPUDescriptions ICCPUDescription;

		//--------------------------------------------------------------------------------
		/// Описатель для IC-карт.
		class CICDescriptions : public CDescription<int>
		{
		public:
			CICDescriptions()
			{
				setDefault(Unknown);

				append(10, "T=0 CPU");
				append(11, "T=1 CPU");
				append(20, "SL4442");
				append(21, "SL4428");
				append(30, "AT24C01");
				append(31, "AT24C02");
				append(32, "AT24C04");
				append(33, "AT24C08");
				append(34, "AT24C16");
				append(35, "AT24C32");
				append(36, "AT24C64");
				append(37, "AT24C128");
				append(38, "AT24C256");
			}
		};

		static CICDescriptions ICDescription;

		//--------------------------------------------------------------------------------
		/// Описатель для RF-карт.
		class CRFDescriptions : public CDescription<int>
		{
		public:
			CRFDescriptions()
			{
				setDefault(Unknown);

				append(10, "Mifare one S50");
				append(11, "Mifare one S70");
				append(12, "Mifare one UL");
				append(20, "Type A CPU");
				append(30, "Type B CPU");
			}
		};

		static CRFDescriptions RFDescription;
	}

	//--------------------------------------------------------------------------------
	/// Ошибки.
	class CErrorDescriptions : public CDescription<int>
	{
	public:
		CErrorDescriptions()
		{
			setDefault("Unknown");

			append(0,  "CM (Command Character) Error");
			append(1,  "PM (Parameter Character) Error");
			append(2,  "Command can not be executed");
			append(3,  "Out of hardware support");
			append(4,  "Command data error");
			append(11, "Card latch operation failure");
			append(15, "EEPROM error");
			append(20, "Read magnetic card error(Exclusive-or bit error)");
			append(21, "Read magnetic card error");
			append(30, "Power down");
			append(41, "IC card module operation failure");
			append(60, "Short circuit of IC card power supply");
			append(61, "IC card initialization failure");
			append(62, "Out of IC card support command");
			append(63, "IC card does not response ");
			append(64, "Other than 63");
			append(65, "Non-initialized of IC card ");
			append(66, "Card type out of reader support");
			append(69, "Not support EMV mode");
		}
	};

	static CErrorDescriptions ErrorDescriptions;

	//--------------------------------------------------------------------------------
	/// Ошибки чтения магнитной полосы карты.
	class CMSErrorDescriptions : public CDescription<int>
	{
	public:
		CMSErrorDescriptions()
		{
			setDefault("Unknown");

			append(20, "Exclusive-or parity error");
			append(23, "Only have start sentinel, end sentinel and LRC bit");
			append(24, "Blank Magnetic track");
			append(26, "No start sentinel");
			append(27, "No end sentinel");
			append(28, "LRC error");
		}
	};

	static CMSErrorDescriptions MSErrorDescription;
}

//--------------------------------------------------------------------------------
