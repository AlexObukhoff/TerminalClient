/* @file Принтер MStar TUP-K на OPOS-драйвере. */

#pragma once

// Project
#include "Hardware/Printers/Tags.h"
#include "OPOSMStarTUPKData.h"

//--------------------------------------------------------------------------------
namespace COPOSMStarTUPK
{
	const char ModelName[] = "Multisoft MStar-TUP-K";    /// название модели.
	const char DeviceNameTag[] = "MSTAR-TUP-K";    /// Тег для идентификации.
	const char OPOSName[] = "mstartk";    /// OPOS-имя.

	const int MaxLineLength = 41;     /// Максимальное количество символов в строке.
	const int MaxOPOSResult = 200;    /// Максимальное значение для OPOS-результата.
	const int MinResetWatchdogFirmware = 101028;    /// Минимальная прошивка, чтобы можно было применять жесткий перезагруз принтера через вочдог фискальника.
	const int MinRecommendedFirmware   = 101050;    /// Рекомендованная прошивка.

	const wchar_t NonFiscalCancellation[] = L"НЕФИСКАЛЬНЫЙ ДОКУМЕНТ АННУЛИРОВАН";

	/// Коды ошибочных статусов.
	namespace ExtendedErrors
	{
		/// Для принтера.
		namespace Printer
		{
			const int Offline      = 0x0401;    /// Offline-mode принтера.
			const int Timeout      = 0x0402;    /// Превышен таймаут ожидания принтера.
			const int Unknown      = 0x0405;    /// Общая ошибка принтера.
			const int Mechanical   = 0x040C;    /// Механическая ошибка.
			const int Cutter       = 0x040D;    /// Ошибка автоотрезчика.
			const int HeadOverheat = 0x040E;    /// Перегрев головки.
			const int PapperJam    = 0x040F;    /// Замятие бумаги.
		}

		/// Для ФР.
		namespace FR
		{
			const int EKLZ   = 0x0C00;    /// Общая ошибка ЭКЛЗ.
			const int NoEKLZ = 0x8C02;    /// ЭКЛЗ отсутствует.
			const int MemoryFlag = 7;     /// Признак ошибки NAND-флэш.
		}
	}

	/// Коды ошибок.
	namespace ErrorCode
	{
		const int UserNotRegistered     = 2310;    /// Кассир не зарегестрирован.
		const int TransactionInProgress = 2053;    /// Транзакция в состоянии выполнения.
	}

	/// Коды ошибок.
	namespace PrinterCommands
	{
		const char Feed[] = "1b6103";    /// Промотка, количество строк для промотки по умолчанию.
		const char Cut[]  = "1b6402";    /// Отрезка.
	}

	/// Данные команд прямого доступа к принтеру.
	namespace DirectIO
	{
		/// Получение данных.
		namespace GetData
		{
			const int Command            = 17;    /// Команда.
			const int TotalEncash        =  2;    /// Сумма в кассе.
			const int FreeZBufferSlots   =  3;    /// Количество свободных мест в Z-буфере.
			const int FilledZBufferSlots =  4;    /// Количество занятых мест в Z-буфере.
		}

		/// Печать Z-отчета.
		namespace PrintZReport
		{
			const int Command = 10;    /// Команда.
			const int Default = 0;     /// Обычная печать Z-отчета.
		}

		/// Прямой доступ к принтеру.
		namespace Printer
		{
			const int Command = 16;    /// Команда.
			const int Default = 0;     /// Объект доступа - принтер.
		}

		/// Перезагрузка через вочдог.
		namespace Reboot
		{
			const int Command = 21;             /// Команда.
			const int Default = 0xAACCFFFE;     /// Объект доступа - вочдог.
		}
	}

	//----------------------------------------------------------------------------
	/// Параметры.
	namespace Parameters
	{
		class CSpecifications : public CSpecification<Enum, Data>
		{
		public:
			CSpecifications()
			{
				append(AutoCutter, Data("CutterInfo", 0x82));
				append(TaxesPrint, Data("PrintFlags", 0x01));
				append(ZBuffer, Data("ZReportFlags", 0x200));
			}
		};

		static CSpecifications Specification;
	}

	//----------------------------------------------------------------------------
	/// Теги.
	class TagEngine : public Tags::Engine
	{
	public:
		TagEngine()
		{
			appendSingle(Tags::Type::Bold, "\x1B", "\x45", "\x46");
		}
	};

	const QSet<QString> NotProcessResult = QSet<QString>() << "Open" << "ClaimDevice" << "Close" << "Release";
}

//--------------------------------------------------------------------------------
