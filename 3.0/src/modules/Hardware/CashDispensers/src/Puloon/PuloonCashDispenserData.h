/* @file Базовые константы диспенсера Puloon. */

#pragma once

#include "Hardware/Common/DeviceCodeSpecification.h"
#include "Hardware/Dispensers/DispenserStatusCodes.h"

//--------------------------------------------------------------------------------
namespace CPuloonLCDM
{
	/// Интервал поллинга в режиме выдачи денег.
	const int DispensingPollingInterval = 300;

	/// Минимальный размер данных ответа.
	const int MinAnswerDataSize = 1;

	/// Максимальный усредненный таймаут выдачи 1 купюры, [мс].
	const int MaxNoteDispensingTimeout = 1000;

	/// Максимальный таймаут для действий купюрника по осознанию пустоты кассеты, [мс].
	const int MaxTimeoutEmptyUnit = 60 * 1000;

	/// Емкость кассеты.
	const int UnitCapacity = 1000;

	/// Количество купюр при почти пустой кассете.
	const int NearEndCount = 30;

	/// Communication ID.
	const char CommunicationId = '\x50';

	/// Rejecting ID.
	const char RejectingID = '\x9C';

	/// Dispensing ID.
	const char DispensingID = '\x8D';

	///Максимальное количество купюр, которое можно выдать за 1 заход из 1 кассеты.
	const int MaxOutputPack = 60;

	/// Команды.
	namespace Commands
	{
		char Reset           = '\x44';    /// Сброс.
		char GetStatus       = '\x46';    /// Статус.
		char UpperDispense   = '\x45';    /// Выдача денег из верхней кассеты.
		char LowerDispense   = '\x55';    /// Выдача денег из нижней кассеты.
		char Dispense        = '\x56';    /// Выдача денег из обеих кассет.
		char GetROMVersion   = '\x47';    /// Получить версию прошивки.

		/// Данные команды.
		struct SData
		{
			int error;      /// Позиция кода ошибки;
			int timeout;    /// Таймаут;

			SData(): error(-1), timeout(0) {}
			SData(int aError, int aTimeout): error(aError), timeout(aTimeout) {}
		};

		/// Данные команд.
		class CData : public CSpecification<char, SData>
		{
		public:
			CData()
			{
				add(Reset, 0, 20 * 1000);
				add(GetROMVersion, -1, 5 * 1000);
				add(GetStatus, 1);
				add(Dispense, 8);
				add(UpperDispense, 4);
				add(LowerDispense, 4);

				setDefault(SData(-1, DispensingPollingInterval));
			}

		private:
			void add(char aCommand, int aError = 0, int aTimeout = DispensingPollingInterval)
			{
				append(aCommand, SData(aError, aTimeout));
			}
		};

		static CData Data;
	}

	const char UpperUnitEmpty  = '\x38';    /// Верхняя кассета пуста.
	const char LowerUnitEmpty  = '\x40';    /// Нижняя кассета пуста.

	/// Ошибки.
	class CDeviceCodeSpecification : public CommonDeviceCodeSpecification
	{
	public:
		CDeviceCodeSpecification()
		{
			addStatus('\x30', DeviceStatusCode::OK::OK);
			addStatus('\x31', DeviceStatusCode::OK::OK);

			addStatus('\x32', DeviceStatusCode::Error::Unknown, "Pickup error");
			addStatus('\x33', DispenserStatusCode::Error::Jammed, "JAM at CHK 1,2 sensor");
			addStatus('\x34', DeviceStatusCode::Warning::Unknown, "Overflow bill");
			addStatus('\x35', DispenserStatusCode::Error::Jammed, "JAM at EXIT sensor or EJT Sensor");
			addStatus('\x36', DispenserStatusCode::Error::Jammed, "JAM at DIV sensor");
			addStatus('\x37', DeviceStatusCode::Error::Firmware, "Undefined command");
			addStatus('\x38', DispenserStatusCode::Warning::Unit0Empty, "Upper bill-end");
			addStatus('\x3A', DeviceStatusCode::Error::Unknown, "Counting error (between CHK 3,4 sensor and DIV sensor)");
			addStatus('\x3B', DeviceStatusCode::Error::Unknown, "Note request error");
			addStatus('\x3C', DeviceStatusCode::Error::Unknown, "Counting Error(between DIV Sensor and EJT Sensor)");
			addStatus('\x3D', DeviceStatusCode::Error::Unknown, "Counting Error(between EJT Sensor and EXIT Sensor)");
			addStatus('\x3F', DeviceStatusCode::Warning::Unknown, "Reject Tray is not recognized");
			addStatus('\x40', DispenserStatusCode::Warning::Unit1Empty, "Lower bill-end");
			addStatus('\x41', DeviceStatusCode::OK::Unknown, "Motor Stop");
			addStatus('\x42', DispenserStatusCode::Error::Jammed, "JAM at DIV Sensor");
			addStatus('\x43', DeviceStatusCode::OK::Unknown, "Timeout (from DIV Sensor to EJT Sensor)");
			addStatus('\x44', DeviceStatusCode::OK::Unknown, "Over Reject");
			addStatus('\x45', DeviceStatusCode::OK::Unknown, "Upper cassette is not recognized");
			addStatus('\x46', DeviceStatusCode::OK::Unknown, "Lower cassette is not recognized");
			addStatus('\x47', DeviceStatusCode::Warning::Unknown, "Dispensing timeout");
			addStatus('\x48', DispenserStatusCode::Error::Jammed, "Jam at EJT sensor");
			addStatus('\x49', DeviceStatusCode::Error::Unknown, "Diverter solenoid or SOL Sensor error");
			addStatus('\x4A', DeviceStatusCode::Error::Unknown, "SOL Sensor error");
			addStatus('\x4B', DeviceStatusCode::Error::Unknown, "Different quantity of the detected biil between check sensor and divert sensor");
			addStatus('\x4C', DispenserStatusCode::Error::Jammed, "Jam at CHK3,4 sensor");
			addStatus('\x4E', DispenserStatusCode::Error::Jammed, "Purge error (Jam at DIV Sensor)");
			addStatus('\x4F', DeviceStatusCode::Error::Unknown, "Bill being detected by a sensor in another cash box");
			addStatus('\x50', DeviceStatusCode::OK::Unknown, "Timeout between CHK sensor and DIV Sensor)");
		}
	};

	static CDeviceCodeSpecification DeviceCodeSpecification;

	/// Сенсоры.
	class CSensorSpecification : public BitmapDeviceCodeSpecification
	{
	public:
		CSensorSpecification()
		{
			addStatus( 0, DeviceStatusCode::Warning::Unknown, "Check sensor 1");
			addStatus( 1, DeviceStatusCode::Warning::Unknown, "Check sensor 2");
			addStatus( 2, DeviceStatusCode::Warning::Unknown, "Divertor sensor 1");
			addStatus( 3, DeviceStatusCode::Warning::Unknown, "Divertor sensor 2");
			addStatus( 4, DeviceStatusCode::Warning::Unknown, "Eject sensor");
			addStatus( 5, DeviceStatusCode::Warning::Unknown, "Exit sensor");
			addStatus( 6, DispenserStatusCode::Warning::Unit0NearEmpty, "Near-end sensor 0");
			addStatus( 7, DeviceStatusCode::Warning::UnknownDataExchange, "Default 1", true);
			addStatus( 8, DeviceStatusCode::Warning::Unknown, "Solenoid sensor");
			addStatus( 9, DispenserStatusCode::Error::Unit0Opened, "Unit 0 sensor");
			addStatus(10, DispenserStatusCode::Error::Unit1Opened, "Unit 1 sensor");
			addStatus(11, DeviceStatusCode::Warning::Unknown, "Check sensor 3");
			addStatus(12, DeviceStatusCode::Warning::Unknown, "Check sensor 4");
			addStatus(13, DispenserStatusCode::Warning::Unit1NearEmpty, "Near-end sensor 1");
			addStatus(14, DispenserStatusCode::Error::RejectingOpened, "Reject tray S/W");
		}
	};

	static CSensorSpecification SensorSpecification;

	/// Режекты.
	class CRejectingSpecification : public CDescription<char>
	{
	public:
		CRejectingSpecification()
		{
			append('\x33', "distance is too close");
			append('\x3F', "One more banknote is picked and followed in the final pickup trial");
			append('\x36', "One more banknote is picked up during the processing the dispense command");
			append('\x38', "skewed");
			append('\x3C', "stuck together");
			append('\x3D', "thickness is too thin");
			append('\x35', "length is too long");
			append('\x3E', "length is too short");
			append('\x80', "checking");
			append('\x9E', "The length measured on check sensors is not out of limit");
			append('\x9F', "The length measured on check sensors is not out of limit");
			append('\x9D', "The banknote passed on check sensors is considered as too much skewed");
			append('\x03', "The banknote are not normally passing on the check sensor because of fast consequential pickup");
			append('\x00', "The banknote are not normally passing on the check sensor because of fast consequential pickup");

			setDefault("unknown");
		}
	};

	static CRejectingSpecification RejectingSpecification;
}

//--------------------------------------------------------------------------------
