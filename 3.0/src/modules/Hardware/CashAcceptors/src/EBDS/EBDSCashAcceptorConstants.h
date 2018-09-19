/* @file Константы купюроприемника на протоколе EBDS. */

#pragma once

// Modules
#include "Hardware/Common/ASCII.h"
#include "Hardware/Common/DeviceCodeSpecification.h"

// Project
#include "Hardware/CashAcceptors/CashAcceptorStatusCodes.h"

//--------------------------------------------------------------------------------
namespace CEBDS
{
	/// Таймаут после команды Reset, [мс].
	const int ResetTimeout = 8000;

	/// Тег для идентификации Advanced-модели.
	const char AdvancedModelTag[] = "SCN";

	/// Направление приема - любое (2-3 биты).
	const char Orientation = 0x0C;

	/// Escrow - разрешено.
	const char Escrow = 0x10;

	/// 1-й байт по дефолту - складывается из направления приема и escrow.
	const char Byte1 = Orientation | Escrow;

	/// 2-й байт по дефолту - включен расширенный режим, "push" режим.
	const char Byte2 = 0x10;

	/// Биты команд.
	const char Stack  = 0x20;
	const char Return = 0x40;

	/// Размер используемой части блока данных 1 номинала в ответе на запрос номиналов.
	const int NominalSize = 16;

	/// Количество номиналов.
	const int NominalCount = 50;

	/// Сообщения.
	namespace Commands
	{
		const char Host2Validator   = 0x10;   /// Standard Host to Acceptor messages.
		const char Validator2Host   = 0x20;   /// Standard Acceptor to Host messages.
		const char BookmarkSelected = 0x30;   /// Bookmark selected.
		const char CalibrateMode    = 0x40;   /// Calibrate Mode.
		const char FlashDownload    = 0x50;   /// Flash Download.
		const char Control          = 0x60;   /// Request CRC, Get Cash in Box, Soft Reset.
		const char Extended         = 0x70;   /// Extended message set.

		/// Для типа Control.
		const QByteArray Reset              = QByteArray::fromRawData("\x60\xFF\xFF\xFF", 4);    /// Сброс.
		const QByteArray GetType            = QByteArray::fromRawData("\x60\x00\x00\x04", 4);    /// Тип купюроприемника (что это такое - только в MEI знают).
		const QByteArray GetSerialNumber    = QByteArray::fromRawData("\x60\x00\x00\x05", 4);    /// Серийник.
		const QByteArray GetBootSoftVersion = QByteArray::fromRawData("\x60\x00\x00\x06", 4);    /// Софт загрузчика интерфейсной платы.
		const QByteArray GetAppSoftVersion  = QByteArray::fromRawData("\x60\x00\x00\x07", 4);    /// Софт приложения головы.
		const QByteArray GetVariantName     = QByteArray::fromRawData("\x60\x00\x00\x08", 4);    /// Название билл-сета (вариант, по-канадски).
		const QByteArray GetVariantVersion  = QByteArray::fromRawData("\x60\x00\x00\x09", 4);    /// Версия билл-сета.

		/// Для типа Extended.
		const char GetPar[]      = "\x70\x02";    /// Получить номинал по индексу.
		const char SetInhibits[] = "\x70\x03";    /// Установить запрещения номиналов.

		class SDescription : public CDescription<QByteArray>
		{
		public:
			SDescription()
			{
				append(GetType, "type of validator");
				append(GetSerialNumber, "serial of validator");
				append(GetBootSoftVersion, "boot software version");
				append(GetAppSoftVersion, "application software version");
				append(GetVariantName, "billset name");
				append(GetVariantVersion, "billset version");

				append(GetPar, "nominal by index");
				append(SetInhibits, "inhibits");
			}
		};
	}

	//--------------------------------------------------------------------------------
	/// Спецификация статусов.
	class DeviceCodeSpecification : public BitmapDeviceCodeSpecification
	{
	public:
		DeviceCodeSpecification()
		{
			addStatus( 0, BillAcceptorStatusCode::BillOperation::Unknown, "", true);
			addStatus( 1, BillAcceptorStatusCode::BillOperation::Accepting);
			addStatus( 2, BillAcceptorStatusCode::BillOperation::Escrow);
			addStatus( 3, BillAcceptorStatusCode::BillOperation::Stacking);
			addStatus( 4, BillAcceptorStatusCode::BillOperation::Stacked);
			addStatus( 5, BillAcceptorStatusCode::Busy::Returning);
			addStatus( 6, BillAcceptorStatusCode::Busy::Returned);

			addStatus( 8, BillAcceptorStatusCode::Warning::Cheated);
			addStatus( 9, BillAcceptorStatusCode::Reject::Unknown);
			addStatus(10, BillAcceptorStatusCode::MechanicFailure::JammedInStacker);
			addStatus(11, BillAcceptorStatusCode::MechanicFailure::StackerFull);
			addStatus(12, BillAcceptorStatusCode::MechanicFailure::StackerOpen, "", true);
			addStatus(13, BillAcceptorStatusCode::Busy::Pause);
			addStatus(14, BillAcceptorStatusCode::Busy::Calibration);

			addStatus(16, DeviceStatusCode::OK::Initialization, "PowerUp");
			addStatus(17, BillAcceptorStatusCode::OperationError::Unknown);
			addStatus(18, DeviceStatusCode::Error::Unknown);
		}
	};
}

//--------------------------------------------------------------------------------
