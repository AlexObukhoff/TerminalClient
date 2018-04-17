/* @file Кросс-устройство управления питанием Штрих 3.0. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDate>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/Specifications.h"
#include "Hardware/Common/DeviceDataConstants.h"

//--------------------------------------------------------------------------------
namespace CShtrih
{
	namespace Constants
	{
		/// Префикс.
		const char Prefix = 2;

		/// Постфикс.
		const char Postfix = 3;

		/// Адрес для широковещательной команды.
		const uchar BroadcastAddress = 0;

		/// Адрес несуществующего устройства.
		const uchar NoAddress = uchar(-1);

		/// Параметр управления питанием - мягкая перезагрузка PC.
		const char SmartRebootPC = 1;

		/// Номер реле модема (не путать с номером бита!).
		const char ModemRelay = 2;

		/// Время импульса, [мс].
		const ushort RelayPulseDelay = 3000;

		/// Минимальный размер ответа.
		const char MinAnswerSize = 6;

		/// Формат представления даты для вывода в лог.
		const QString DateFormatLog = "dd.MM.yy";

		/// Максимальное число повторений пакета в случае ошибки.
		const int MaxRepeatPacket = 10;
	}

	/// Позиции составных частей ответа в пакете.
	namespace Position
	{
		/// Позиции составных частей ответа в пакете.
		namespace Bytes
		{
			const int Address = 2;    /// Адрес устройства.
			const int Command = 4;    /// Команда.
		}

		/// Позиции битов датчиков в байте флагов.
		namespace Sensors
		{
			const int PowerControlLogic  = 0;      /// Логика поддержки питания.
			const int AdvancedPowerLogic = 1;      /// Расширенная логика анализа датчиков питания.
			const int Door  = 2;      /// Дверь.
			const int Power = 3;      /// Питание.
		}
	}

	/// Позиции байтов в пакете.
	namespace Sensors
	{
		const char Address = 2;      /// Адрес устройства.
		const char Command = 4;      /// Команда.
	}

	/// Таймауты, [мс].
	namespace Timeouts
	{
		/// Дефолтный между запросом и ответом.
		const int Default = 300;

		/// Между ответами разхных девайсов для широковещательной команды.
		const int Broadcast = 100;
	}

	namespace Devices
	{
		struct SSoftInfo
		{
			double version;
			double build;
			QDate date;

			SSoftInfo(): version(0), build(0) {}
			SSoftInfo(double aVersion, double aBuild, const QDate & aDate):
				version(aVersion), build(aBuild), date(aDate) {}
		};

		struct SData
		{
			QString name;
			uchar address;
			qlonglong serial;
			SSoftInfo softInfo;

			SData(): address(Constants::NoAddress), serial(0) {}
			SData(const QString & aName, uchar aAddress): name(aName), address(aAddress), serial(0) {}

			void setData(const QString & aName, uchar aAddress, qlonglong aSerial, const SSoftInfo & aSoftInfo)
			{
				name     = aName;
				address  = aAddress;
				serial   = aSerial;
				softInfo = aSoftInfo;
			}
		};

		namespace Type
		{
			enum Enum
			{
				General,
				CrossDevice,
				PowerInterrupter
			};
		}

		namespace Name
		{
			const QString CrossDevice = "cross";
			const QString PowerInterrupter = "power";
		}
	}

	typedef QMap<Devices::Type::Enum, Devices::SData> TDevicesData;

	/// Константы установок презентера и ретрактора.
	struct SDevicesData
	{
		TDevicesData devicesData;

		SDevicesData()
		{
			using namespace CDeviceData::Watchdogs::Sub;

			devicesData.insert(Devices::Type::General, Devices::SData(All, Constants::BroadcastAddress));
			devicesData.insert(Devices::Type::CrossDevice, Devices::SData(CrossUSBCard, 0x51));
			devicesData.insert(Devices::Type::PowerInterrupter, Devices::SData(PowerSupply, 0x55));
		}

		const Devices::SData operator[] (const Devices::Type::Enum aDeviceType) const {return devicesData[aDeviceType];}
		Devices::SData & operator[](const Devices::Type::Enum aDeviceType) {return devicesData[aDeviceType];}
	};

	namespace Commands
	{
		namespace General
		{
			const char Identification = '\x01';     /// Идентификация.
		}

		namespace CrossDevice
		{
			const char PollExtended = '\x76';      /// Длинный запрос статуса.
			const char PowerControl = '\x77';      /// Управление кнопками включения/выключения.
		}

		namespace PowerInterrupter
		{
			const char PulseRelay = '\x78';        /// Импульс реле.
		}

		/// Описатель команд.
		class CDescriptions : public CDescription<char>
		{
		public:
			CDescriptions()
			{
				/// Общие команды.
				append(General::Identification, "identification");

				/// Кросс-плата.
				append(CrossDevice::PollExtended, "extended poll");
				append(CrossDevice::PowerControl, "power buttons control");

				/// Power-interrupter.
				append(PowerInterrupter::PulseRelay, "pulse relay");
			}
		};

		static CDescriptions Description;
	}

	typedef QList<QByteArray> TAnswersBuffer;

	/// Структура для распарсивания данных ответа; на разные команды в ответе приходят разные данные.
	struct SUnpackedData
	{
		bool door;      /// Аларм двери.
		bool power;     /// Аларм питания.

		QByteArray answer;

		SUnpackedData() :
			door(false),
			power(false) {}
	};

	namespace Errors
	{
		class CDescriptions : public CDescription<char>
		{
		public:
			CDescriptions()
			{
				append('\x33', "Некорректные параметры в команде");
				append('\x37', "Команда не поддерживается в данной реализации");
				append('\x5D', "Таблица не определена");
				append('\x5E', "Строка не определена");
				append('\x5F', "Поле не определено");
				append('\x7A', "Поле не редактируется");
				append('\x7E', "Неверное значение в поле длины");

				setDefault("Unknown");
			}
		};

		static CDescriptions Description;
	}
}

//--------------------------------------------------------------------------------
