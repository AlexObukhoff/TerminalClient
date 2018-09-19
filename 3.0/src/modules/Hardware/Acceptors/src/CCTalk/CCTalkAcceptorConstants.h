/* @file Константы устройств на протоколе ccTalk. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QByteArray>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/Specifications.h"
#include "Hardware/Common/WaitingData.h"

// Project
#include "Hardware/CashAcceptors/CashAcceptorStatusCodes.h"
#include "Hardware/Acceptors/CCTalkData.h"

//--------------------------------------------------------------------------------
namespace CCCTalk
{
	/// ACK.
	const char ACK  = '\x00';

	/// Маска для сортировщика по умолчанию (использовать предустановленные настройки).
	const char DefaultSorterMask = '\xFF';

	/// Пауза перед идентификацией.
	const int IdentificationPause = 100;

	/// Количество номиналов.
	const int NominalCount = 16;

	class CDeviceTypeIds : public CDescription<char>
	{
	public:
		CDeviceTypeIds()
		{
			append(Address::CoinAcceptor, "coinacceptor");
			append(Address::Payout, "");
			append(Address::BillAcceptor, "billvalidator");
			append(Address::CardReader, "");
		}
	};

	static CDeviceTypeIds DeviceTypeIds;

	/// Команды.
	namespace Command
	{
		/// Общие.
		namespace Core
		{
			const uchar SimplePoll   = 254;
			const uchar VendorID     = 246;
			const uchar DeviceTypeID = 245;
			const uchar ModelName    = 244;
			const uchar BuildCode    = 192;
		}

		/// Общие +.
		namespace CorePlus
		{
			const uchar Serial       = 242;
			const uchar SoftVersion  = 241;
			const uchar ProtocolID   =   4;
			const uchar Reset        =   1;
		}

		const uchar Status                     = 248;
		const uchar DBVersion                  = 243;
		const uchar TestCoils                  = 240;
		const uchar SelfCheck                  = 232;
		const uchar PartialEnable              = 231;
		const uchar GetBufferedCoinStatuses    = 229;
		const uchar AllSetEnable               = 228;
		const uchar CreationDate               = 196;
		const uchar SoftLastDate               = 195;
		const uchar GetCoinID                  = 184;
		const uchar SetSecurity                = 181;
		const uchar BaseYear                   = 170;
		const uchar ModifyInhibitsAndRegesters = 162;
		const uchar GetBufferedBillStatuses    = 159;
		const uchar GetBillID                  = 157;
		const uchar GetCountryScalingFactor    = 156;
		const uchar RouteBill                  = 154;
		const uchar ModifyBillOperatingMode    = 153;
		const uchar GetCurrencyRevision        = 145;

		namespace EAnswerType
		{
			enum Enum
			{
				ACK,
				ASCII,
				Date,
				Data
			};
		}

		struct SData
		{
			EAnswerType::Enum type;
			const char * description;
			int size;

			SData() : type(EAnswerType::ACK), description(""), size(1) {}
			SData(EAnswerType::Enum aType, const char * aDescription, int aSize) : type(aType), description(aDescription), size(aSize) {}
		};

		class CDescriptions : public CSpecification<char, SData>
		{
		public:
			CDescriptions()
			{
				append(Core::SimplePoll,           EAnswerType::ACK,   "perform simple poll");
				append(Core::VendorID,             EAnswerType::ASCII, "get vendor ID");
				append(Core::DeviceTypeID,         EAnswerType::ASCII, "get device type ID");
				append(Core::ModelName,            EAnswerType::ASCII, "get model name");
				append(Core::BuildCode,            EAnswerType::ASCII, "get build code");

				append(CorePlus::Serial,           EAnswerType::Data,  "get serial number", 3);
				append(CorePlus::SoftVersion,      EAnswerType::ASCII, "get soft version");
				append(CorePlus::ProtocolID,       EAnswerType::Data,  "get protocol ID", 3);
				append(CorePlus::Reset,            EAnswerType::ACK,   "perform reset");

				append(Status,                     EAnswerType::Data,  "get generic status", 1);
				append(DBVersion,                  EAnswerType::Data,  "get DB version", 1);
				append(TestCoils,                  EAnswerType::ACK,   "test coils");
				append(SelfCheck,                  EAnswerType::Data,  "perform selfcheck", 1);
				append(PartialEnable,              EAnswerType::ACK,   "enable/disable nominals");
				append(GetBufferedCoinStatuses,    EAnswerType::Data,  "get buffered coin statuses", 1);
				append(AllSetEnable,               EAnswerType::ACK,   "set enable for all nominals");
				append(CreationDate,               EAnswerType::Date,  "get device creation date", 2);
				append(SoftLastDate,               EAnswerType::Date,  "get last modification date (actually soft)", 2);
				append(GetCoinID,                  EAnswerType::Data,  "get coin nominal data", 6);
				append(SetSecurity,                EAnswerType::ACK,   "set security level");
				append(BaseYear,                   EAnswerType::Data,  "get a year, which is the base for other dates", 4);
				append(ModifyInhibitsAndRegesters, EAnswerType::ACK,   "modify inhibits and override registers for sequence of coins");
				append(GetBufferedBillStatuses,    EAnswerType::Data,  "get buffered bill statuses", 1);
				append(GetBillID,                  EAnswerType::Data,  "get bill nominal data", 7);
				append(GetCountryScalingFactor,    EAnswerType::Data,  "get country scaling factor", 3);
				append(RouteBill,                  EAnswerType::Data,  "route bill");
				append(ModifyBillOperatingMode,    EAnswerType::ACK,   "modify bill operating mode");
				append(GetCurrencyRevision,        EAnswerType::Data,  "get currency revision", 2);
			}

		protected:
			void append(char aCommand, EAnswerType::Enum aType, const char * aDescription, int aSize = -1)
			{
				switch(aType)
				{
					case EAnswerType::ACK  : aSize = 1; break;
					case EAnswerType::Date : aSize = 2; break;
				}

				mBuffer.insert(aCommand, SData(aType, aDescription, aSize));
			}
		};

		static CDescriptions Description;
	}

	//--------------------------------------------------------------------------------
	/// Ошибки.
	struct SErrorData
	{
		int statusCode;
		const char * description;
		bool isRejected;

		SErrorData() : statusCode(DeviceStatusCode::OK::Unknown), description(""), isRejected(false) {}
		SErrorData(int aStatusCode, const char * aDescription, bool aIsRejected) : statusCode(aStatusCode), description(aDescription), isRejected(aIsRejected) {}
	};

	class ErrorDataBase : public CSpecification<uchar, SErrorData>
	{
	public:
		ErrorDataBase()
		{
			setDefault(SErrorData(DeviceStatusCode::OK::Unknown, "unknown device code", false));
		}

	protected:
		void add(uchar aError, int aStatusCode, bool aIsRejected = false, const char * aDescription = "")
		{
			append(aError, SErrorData(aStatusCode, aDescription, aIsRejected));
		}
	};

	/// Фиктивныe девайс-коды устройства, для функционала, связанного с повтором таких статус-кодов
	const uchar EscrowDeviceCode  = 200;
	const uchar StackedDeviceCode = 201;

	//--------------------------------------------------------------------------------
	/// Неисправности.
	struct SFault
	{
		int statusCode;
		const char * description;
		const char * extraData;

		SFault() : statusCode(DeviceStatusCode::OK::OK), description(""), extraData("") {}
		SFault(int aStatusCode, const char * aDescription, const char * aExtraData) : statusCode(aStatusCode), description(aDescription), extraData(aExtraData) {}
	};

	/// статусы от неподдерживаемых устройств - с комментариями - будут поддержаны при реализации этих устройств
	class CFault : public CSpecification<uchar, SFault>
	{
	public:
		CFault()
		{
			append( 0, DeviceStatusCode::OK::OK);
			append( 1, DeviceStatusCode::Error::MemoryStorage, "EEPROM checksum corrupted");
			append( 2, BillAcceptorStatusCode::Reject::UserDefined, "Fault on inductive coils", "Coil number");
			append( 3, BillAcceptorStatusCode::SensorError::Credit);
			append( 4, BillAcceptorStatusCode::SensorError::Piezo);
			append( 5, BillAcceptorStatusCode::SensorError::Reflective);
			append( 6, BillAcceptorStatusCode::SensorError::Diameter);
			append( 7, BillAcceptorStatusCode::SensorError::WakeUp);
			append( 8, BillAcceptorStatusCode::SensorError::Sorter, "Fault on sorter exit sensors", "Sensor number");
			append( 9, DeviceStatusCode::Error::MemoryStorage, "NVRAM checksum corrupted");
			append(10, BillAcceptorStatusCode::SensorError::Dispensing);
			append(11, DeviceStatusCode::Error::Unknown, "Low level sensor error", "Hopper or tube number");  // ошибка другого устройства
			append(12, DeviceStatusCode::Error::Unknown, "High level sensor error", "Hopper or tube number"); // ошибка другого устройства
			append(13, BillAcceptorStatusCode::OperationError::CashCalculation, "Coin counting error");
			append(14, DeviceStatusCode::Error::Unknown, "Keypad error", "Key number"); // ошибка другого устройства
			append(15, DeviceStatusCode::Error::Unknown, "Button error");   // ошибка другого устройства
			append(16, DeviceStatusCode::Error::Unknown, "Display error");  // ошибка другого устройства
			append(17, BillAcceptorStatusCode::OperationError::CashCalculation, "Coin auditing error");
			append(18, BillAcceptorStatusCode::SensorError::Reject, "Fault on reject sensor");
			append(19, BillAcceptorStatusCode::MechanicFailure::ReturnMechanism, "Fault on coin return mechanism");
			append(20, BillAcceptorStatusCode::MechanicFailure::COSMechanism, "Fault on C.O.S. mechanism");
			append(21, BillAcceptorStatusCode::SensorError::Rim);
			append(22, BillAcceptorStatusCode::SensorError::Thermo);
			append(23, DeviceStatusCode::Error::Unknown, "Payout motor fault", "Hopper number");  // ошибка другого устройства
			append(24, DeviceStatusCode::Error::Unknown, "Payout timeout", "Hopper or tube number");  // ошибка другого устройства
			append(25, DeviceStatusCode::Error::Unknown, "Payout jammed", "Hopper or tube number");  // ошибка другого устройства
			append(26, DeviceStatusCode::Error::Unknown, "Payout sensor fault", "Hopper or tube number");  // ошибка другого устройства
			append(27, DeviceStatusCode::Error::Unknown, "Level sensor error", "Hopper or tube number");  // ошибка другого устройства
			append(28, DeviceStatusCode::Error::Unknown, "Personality module not fitted");  // ошибка интеграционного модуля ccTalk: недоступен
			append(29, DeviceStatusCode::Error::Unknown, "Personality checksum corrupted");  // ошибка интеграционного модуля ccTalk: CRC посылки
			append(30, DeviceStatusCode::Error::MemoryStorage, "ROM checksum mismatch");
			append(31, DeviceStatusCode::Error::Unknown, "Missing slave device", "Slave address");  // ошибка slave-устройства - отсутствует
			append(32, DeviceStatusCode::Error::Unknown, "Internal comms bad", "Slave address");  // ошибка slave-устройства - отсутствует
			append(33, DeviceStatusCode::Error::PowerSupply);
			append(34, DeviceStatusCode::Error::Temperature);
			append(35, BillAcceptorStatusCode::MechanicFailure::DCEChute);
			append(36, BillAcceptorStatusCode::SensorError::Validation, "Fault on bill validation sensor", "Sensor number");
			append(37, BillAcceptorStatusCode::MechanicFailure::TransportMotor);
			append(38, BillAcceptorStatusCode::MechanicFailure::JammedInStacker);
			append(39, BillAcceptorStatusCode::MechanicFailure::JammedInValidator);
			append(40, DeviceStatusCode::Error::MemoryStorage, "RAM test fail");
			append(41, BillAcceptorStatusCode::SensorError::String);
			append(42, BillAcceptorStatusCode::MechanicFailure::CoinGateStuck, "Accept gate failed open");
			append(43, BillAcceptorStatusCode::MechanicFailure::CoinGateStuck, "Accept gate failed closed");
			append(44, BillAcceptorStatusCode::MechanicFailure::StackerOpen);
			append(45, BillAcceptorStatusCode::MechanicFailure::StackerFull);
			append(46, DeviceStatusCode::Error::MemoryStorage, "Flash memory erase fail");
			append(47, DeviceStatusCode::Error::MemoryStorage, "Flash memory write fail");
			append(48, DeviceStatusCode::Error::Unknown, "Slave device not responding", "Device number");  // ошибка slave-устройства - отсутствует
			append(49, BillAcceptorStatusCode::SensorError::Optical, "Fault on opto sensor", "Opto number");
			append(50, DeviceStatusCode::Error::PowerSupply, "Battery fault");
			append(51, DeviceStatusCode::Error::MechanicFailure, "Door open");  // скорее всего, ошибка относится к купюроприемнику с дверцей
			append(52, DeviceStatusCode::Warning::WrongSwitchesConfig, "Microswitch fault");
			append(53, BillAcceptorStatusCode::Error::Clock);
			append(255, DeviceStatusCode::Error::Unknown, "Unspecified fault code", "Further information");
		}

		QString getDescription(QByteArray & aError) const
		{
			uchar error = uchar(aError[0]);
			uchar extraData = uchar(aError[1]);

			if (aError.isEmpty() || !mBuffer.contains(error) || (aError.size() > 2))
			{
				return "Unknown code";
			}

			if ((aError.size() == 2) && !QString(mBuffer[error].extraData).isEmpty())
			{
				if (error == 35)
				{
					return QString("%1, reason = %2")
						.arg(mBuffer[error].description)
						.arg((extraData == 1) ? "coin" : ((extraData == 2) ? "token" : "unknown"));
				}

				return QString("%1, %2 = %3")
					.arg(mBuffer[error].description)
					.arg(mBuffer[error].extraData)
					.arg(extraData);
			}

			return QString(mBuffer[error].description);
		}

	protected:
		void append(uchar aErrorCode, int aStatusCode, const char * aDescription = "", const char * aExtraData = "") { mBuffer.insert(aErrorCode, SFault(aStatusCode, aDescription, aExtraData)); }
	};

	static CFault Fault;
}

//--------------------------------------------------------------------------------
