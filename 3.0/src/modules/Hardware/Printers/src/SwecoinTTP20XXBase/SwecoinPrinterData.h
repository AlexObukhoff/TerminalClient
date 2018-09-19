/* @file Константы принтеров Swecoin. */

#pragma once

// STL
#include <functional>

// Models
#include "Hardware/Common/DeviceDataConstants.h"

// Project
#include "Hardware/Printers/PrinterStatusCodes.h"
#include "Hardware/Printers/Tags.h"

//--------------------------------------------------------------------------------
namespace CSwecoinPrinter
{
	const int Pause = 500;  /// Таймаут порта после инициализации.
	const int MaxReadIdTimeout = 10 * 1000;  /// Максимальный таймаут для чтения ответа на команду получения Id.
	const int MinReadIdTimeout = 2 * 1000;   /// Минимальный таймаут для чтения данных на команду получения Id.

	const char UnknownModelName[] = "Unknown Swecoin Printer";  /// Имя принтера по умолчанию.
	
	/// Команды.
	namespace Commands
	{
		const char Initilize[] = "\x1B\x40";           /// Инициализация.
		const char SetParameter[] = "\x1B\x26\x50";    /// Часть команды установки параметра.
		const char GetData[] = "\x1B\x05";             /// Часть команды получения данных об устройстве.
		const char GetModelId[] = "\x1B\x05\x63";      /// Получение Id устройства.
		const char GetStatus[] = "\x1B\x05\x01";       /// Статус.
		const char GetPaperNearEndData[] = "\x1B\x05\x02";
		const char GetSensorData[] = "\x1B\x05\x05";
	}

	//----------------------------------------------------------------------------
	/// Статусы.
	class CStatuses : public CSpecification<char, int>
	{
	public:
		CStatuses()
		{
			//TODO: влияет ли BM error на печать, если BM сенсор включен? Разобраться с index error - что это и зачем это.
			append('\x01', PrinterStatusCode::Error::PaperJam);
			append('\x02', PrinterStatusCode::Error::Cutter);
			append('\x03', PrinterStatusCode::Error::PaperEnd);
			append('\x04', PrinterStatusCode::Error::PrintingHead);
			append('\x05', PrinterStatusCode::Error::PaperJam);
			append('\x06', PrinterStatusCode::Error::Temperature);
			append('\x07', PrinterStatusCode::Error::Presenter);
			append('\x08', PrinterStatusCode::Error::PaperJam);
			append('\x0D', PrinterStatusCode::Error::Port);
			append('\x0E', DeviceStatusCode::Error::Firmware);
			append('\x0F', DeviceStatusCode::Error::Firmware);
			append('\x11', DeviceStatusCode::Warning::OperationError);    // Paused to avoid overheating
			append('\xFF', DeviceStatusCode::Error::Unknown);

			setDefault(DeviceStatusCode::OK::OK);
		}
	};

	static CStatuses Statuses;

	const char PaperInPresenterMask = '\x20';

	//----------------------------------------------------------------------------
	/// Теги.
	class TagEngine : public Tags::Engine
	{
	public:
		TagEngine()
		{
			appendSingle(Tags::Type::Bold,         "\x1B\x42", "\x01");
			appendSingle(Tags::Type::Italic,       "\x1B\x69", "\x01");
			appendSingle(Tags::Type::UnderLine,    "\x1B\x75", "\x01");
			appendSingle(Tags::Type::DoubleWidth,  "\x1B\x77", "\x01");
			appendSingle(Tags::Type::DoubleHeight, "\x1B\x68", "\x01");
		}
	};

	//----------------------------------------------------------------------------
	/// Данные устройства.
	typedef std::function<QString(QByteArray)> THandler;

	struct SDeviceParameters
	{
		int size;
		THandler handler;
		QString description;

		SDeviceParameters() : size(0) {}
		SDeviceParameters(int aSize, const QString & aDescription, THandler aHandler) : size(aSize), handler(aHandler), description(aDescription) {}
	};

	typedef QMap<char, SDeviceParameters> TDeviceParameters;

	class CDeviceParameters : public CSpecification<char, SDeviceParameters>
	{
	public:
		CDeviceParameters()
		{
			auto processData = [] (const QByteArray & aAnswer, int aDigits) -> QString { QString result; for (int i = 0; i < aAnswer.size(); ++i)
				result += QString("%1").arg(uint(aAnswer[i]), aDigits, 10, QChar(ASCII::Zero)); return result; };
			auto processDigit = [&] (const QByteArray & aAnswer) -> QString { return processData(aAnswer, 3); };
			THandler processFloat = [&] (const QByteArray & aAnswer) -> QString { return QString("%1").arg(processData(aAnswer, 2).toInt()/100.0, 5, 'f', 2, QChar(ASCII::Space)).simplified(); };
			auto getAnswer = [] (const QByteArray & aAnswer) -> QString { return aAnswer; };

			append('\x09', SDeviceParameters(6, CDeviceData::SerialNumber, processDigit));
			append('\x07', SDeviceParameters(2, CDeviceData::Firmware,     processFloat));
			append('\x0A', SDeviceParameters(1, CDeviceData::BoardVersion, getAnswer));
			append('\x0C', SDeviceParameters(2, CDeviceData::BootVersion,  processFloat));
		}
	};
}

//--------------------------------------------------------------------------------
