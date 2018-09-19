/* @file Константы принтеров STAR. */

#pragma once

// Modules
#include "Hardware/Common/HardwareConstants.h"
#include "Hardware/Common/WaitingData.h"
#include "Hardware/Protocols/Common/ProtocolUtils.h"

// Project
#include "Hardware/Printers/PrinterStatusCodes.h"
#include "Hardware/Printers/Tags.h"

//--------------------------------------------------------------------------------
/// Константы, команды и коды состояний принтеров STAR.
namespace CSTAR
{
	/// Минимальная "версия" прошивки (число, характеризующее прошивку а Header 2 в ответе на запрос статуса).
	const int MinVersionNumber = 3;

	/// Маска для определения статуса эжектора.
	const char PresenterStatusMask = 0x0E;

	/// Максимальный размер [ответа на запрос, ASB] статуса.
	const int MaxStatusAnswerSize = 9;

	/// Высота линии изображения в байтах.
	const int LineHeight = 3;

	/// Высота линии изображения в пикселях (1 пиксель == 1 бит).
	const int ImageHeight = LineHeight * 8;

	//----------------------------------------------------------------------------
	/// Таймауты, [мс].
	namespace Timeouts
	{
		/// По умолчанию на получение ответа от устройства.
		const int Default = 1500;

		/// Ответ на запрос статуса.
		const int Status = 300;

		/// Переинициализация принтера после сброса.
		const int Reset = 3000;

		/// Ожидание допечати чека.
		const int ReceiptProcessing = 5 * 1000;

		/// Повторное чтение мем-свича при приходе ASB-статуса.
		const int MSWGettingASB = 100;

		/// Установка мем-свича.
		const int MSWSetting = 100;

		/// Запись мем-свича.
		const int MSWWriting = 5000;

		/// Пауза поллинга в период печати.
		const int Printing = 10 * 1000;
	}

	/// Ожидание операций с чеком для принтеров с эжектором.
	const SWaitingData EjectorWaiting = SWaitingData(300, 20 * 1000);

	//----------------------------------------------------------------------------
	/// Команды.
	namespace Commands
	{
		const char Initilize[]     = "\x1B\x40";            /// Переинициализация логики.
		const char Reset[]         = "\x1B\x06\x18";        /// Сброс (логика + механика).
		const char ASBStatus[]     = "\x1B\x06\x01";        /// Статус (ASB).
		const char ETBMark[]       = "\x17";                /// Метка (ETB) для ASB статуса.
		const char SetASB[]        = "\x1B\x1E\x61\x03";    /// Установить ASB.
		const char PrintImage[]    = "\x1B\x58";            /// Печать изображения.
		const char FeedImageLine[] = "\x1B\x49\x18";        /// Промотка линии изображения.
		const QByteArray WaitForPrintingEnd = QByteArray::fromRawData("\x1B\x1D\x03\x01\x00\x00", 6);    /// Дождаться окончания печати и получить счетчик чеков.

		/// Команды работы с мем-свичами
		template<class T>
		inline QString int2ASCII(T aValue, int aDigits) { return QByteArray(QString("%1").arg(aValue, aDigits, 16, QChar(ASCII::Zero)).toUpper().toLatin1()).toHex(); }

		inline QByteArray getMemorySwitch(char aNumber) { return ProtocolUtils::getBufferFromString(
			QString("1B 23 %1 3F 30 30 30 30 0A 00").arg(int2ASCII(aNumber, 1))); }    /// Получить значение memory switch-а.

		inline QByteArray setMemorySwitch(char aNumber, ushort aValue) { return ProtocolUtils::getBufferFromString(
			QString("1B 1D 23 2C %1 %2 0A 00").arg(int2ASCII(aNumber, sizeof(aNumber))).arg(int2ASCII(aValue, 4))); }    /// Установить значение memory switch-а.

		const QByteArray WriteMemorySwitches = QByteArray::fromRawData("\x1B\x1D\x23\x57\x30\x30\x30\x30\x30\x0A\x00", 11);    /// Записать мем-свичи.
		const QByteArray GetModelData = QByteArray::fromRawData("\x1B\x23\x2A\x0A\x00", 5);    /// Получить Id модели.
	}

	//----------------------------------------------------------------------------
	/// Кодовые страницы.
	class CCodepage : public CSpecification<QString, uchar>
	{
	public:
		CCodepage()
		{
			using namespace CHardware::Codepages;

			//append(CP437, 3);    // отсутствует в Qt
			//append(CP858, 4);    // отсутствует в Qt
			//append(CP852, 5);    // отсутствует в Qt
			append(CP866, 10);

			append(Win1252, 32);
			append(Win1250, 33);
			append(Win1251, 34);

			setDefault(value(CP866));
		}

		const QByteArray operator[] (const QString & aCodepage) const { return QByteArray("\x1B\x1D\x74").append(value(aCodepage)); }
	};

	static CCodepage Codepage;

	//----------------------------------------------------------------------------
	/// Статусы.
	typedef QMap<int, int> TStatus;

	class CASBStatus : public CSpecification<int, TStatus>
	{
	public:
		CASBStatus()
		{
			data()[1].insert(3, DeviceStatusCode::Error::Unknown);
			data()[1].insert(5, DeviceStatusCode::Error::CoverIsOpened);

			data()[2].insert(2, PrinterStatusCode::Error::Temperature);
			data()[2].insert(3, PrinterStatusCode::Error::Cutter);
			data()[2].insert(5, DeviceStatusCode::Error::Unknown);
			data()[2].insert(6, PrinterStatusCode::Error::Temperature);

			data()[3].insert(1, DeviceStatusCode::Error::PowerSupply);
			data()[3].insert(2, PrinterStatusCode::Error::PaperJam);

			data()[4].insert(2, PrinterStatusCode::Warning::PaperNearEnd);
			data()[4].insert(3, PrinterStatusCode::Error::PaperEnd);
		}
	};

	static CASBStatus ASBStatus;

	//----------------------------------------------------------------------------
	/// Теги.
	class TagEngine : public Tags::Engine
	{
	public:
		TagEngine()
		{
			appendSingle(Tags::Type::Bold,         "\x1B",     "\x45", "\x46");
			appendSingle(Tags::Type::UnderLine,    "\x1B\x2D", "\x01");
			appendSingle(Tags::Type::DoubleWidth,  "\x1B\x57", "\x01");
			appendSingle(Tags::Type::DoubleHeight, "\x1B\x68", "\x01");

			set(Tags::Type::Image);
		}
	};
};

//--------------------------------------------------------------------------------
