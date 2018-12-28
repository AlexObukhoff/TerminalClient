/* @file Константы, коды команд и ответов протокола ФР АТОЛ. */

#pragma once

// Modules
#include "Hardware/Common/ASCII.h"

// Project
#include "Hardware/Printers/Tags.h"
#include "Hardware/FR/FRErrorDescription.h"
#include "AtolDataTypes.h"

//--------------------------------------------------------------------------------
namespace CAtolFR
{
	/// Количество товара.
	const int GoodsCountByte = 1;

	/// Минимальный размер распакованных данных.
	const int MinUnPacketAnswerSize = 2;

	/// Минимальный код ошибки.
	const uchar MinErrorCode = 0x02;

	/// Маска для парсинга режима и подрежима.
	const uchar ModeMask = 0x0F;

	/// Формат печати X- и Z-отчетов - печатать необнуляемую сумму с момента последней перерегистрации на расширенном отчете.
	const char ReportMode = '\x03';

	/// Маска для обнуления суммы в кассе при закрытии смены.
	const char NullingSumInCashMask = '\x04';

	/// Маска для длинных отчетов.
	const char LongReportMask = '\xE4';

	/// Флаги выполнения фискальных операций.
	namespace FiscalFlags
	{
		const char ExecutionMode  = '\x00';    /// Режим выполнения операций.
		const char CashChecking   = '\x00';    /// Проверять денежную наличность.
		const char TaxForPosition = '\x00';    /// Налог на позицию.
	}

	/// Флаги выполнения продажи - Выполнить операцию + проверить денежную наличность.
	const char SaleFlags = FiscalFlags::ExecutionMode | FiscalFlags::CashChecking;

	/// Номер ряда в таблице 2 - режим работы ККМ.
	const char FRModeTableSeries = 1;

	/// Тип налога - налог на каждую покупку.
	const char CustomSaleTax = 2;

	/// Область действия налога - на регистрацию.
	const char TaxControlArea = 2;

	/// Максимальное количество заполненных по дефолту строк.
	const char MaxDocumentCapStrings = 20;

	/// Размер ИТОГ на фискальном чеке - двойная ширина (еще можно и/или двойную высоту).
	const char ResumeSize = 4;

	/// Флаг полной отрезки чека.
	const char FullCutting = 4;

	/// Сколько строк надо промотать после печати фискального чека.
	const char DefaultFiscalFeedCTC2000 = 4;

	/// Формат представления даты и времени в ответе на длинный запрос статуса.
	const char DateTimeFormat[] = "yyyyMMddhhmmss";

	/// Формат представления даты и времени в ответе на запрос даты-времени сессии.
	const char SessionDTFormat[] = "ddMMyyyyhhmmss";

	/// Подсистемы ФР, имеющие свой софт - константы для запроса версий софта.
	namespace FRSubSystems
	{
		const char FR = 0x01;    /// Фискальная плата.
		const char FM = 0x02;    /// Фискальная память.
		const char BL = 0x03;    /// Загрузчик.
	}

	/// Типы отчетов без гашения.
	namespace Balances
	{
		/// Тип отчета без гашения - X-отчет.
		const char XReport = '\x01';
	}

	/// Таймауты чтения, [мс].
	namespace Timeouts
	{
		/// После окончания печати нефискального чека, чтобы принтер не захлебнулся командами, не начал отрезать по напечатанному и пр.
		const int EndNotFiscalPrint = 500;

		/// Максимальный таймаут для получения ответа после паузы при выполнении Z-отчета.
		const int ZReportNoAnswer = 120000;

		/// Для получения ответа после паузы при выполнении X-отчета.
		const int XReportNoAnswer = 30000;

		/// Пауза между посылками запроса статуса при выполнении Z-отчета, во время которой ждем смены состояний ФР.
		const int ZReportPoll = 500;

		/// Пауза между посылками запроса статуса при выполнении Z-отчета, во время которой ждем смены состояний ФР.
		const int XReportPoll = 500;
	}

	/// Тип фискального документа.
	namespace DocumentTypes
	{
		const char Sale     = 1;    /// Продажа.   
		const char SaleBack = 2;    /// Возврат продажи.
	}

	/// Тип оплаты.
	namespace PaymentSource
	{
		const char Cash  = 1;    /// Наличные.
		const char Type2 = 2;    /// Типы 2..4 - пользовательские.
		const char Type3 = 3;
		const char Type4 = 4;
	}

	//------------------------------------------------------------------------------------------------
	/// Теги.
	class TagEngine : public Tags::Engine
	{
	public:
		TagEngine()
		{
			appendSingle(Tags::Type::DoubleWidth, "", "\x09");
		}
	};

	//------------------------------------------------------------------------------------------------
	/// Коды команд.
	namespace Commands
	{
		/// Команды получения информации об устройстве.
		const char GetModelInfo    = '\xA5';    /// Получить инфо о модели ФР.
		const char GetSoftInfo     = '\x9D';    /// Получить инфо о ПО девайса (ФР, ФП, загрузчик).
		const char GetLongStatus   = '\x3F';    /// Длинный статус.
		const char GetShortStatus  = '\x45';    /// Короткий статус.
		const char GetFRRegister   = '\x91';    /// Прочитать регистр ФР.
		const char GetFRParameter  = '\x46';    /// Прочитать параметр ФР.
		const char SetFRParameters = '\x50';    /// Установить параметр ФР.

		/// Нефискальные служебные команды.
		const char Cut             = '\x75';    /// Отрезка.
		const char PrintString     = '\x4C';    /// Печать строки.
		const char PrinterAccess   = '\x8F';    /// Прямой доступ к принтеру.
		const char EnterToMode     = '\x56';    /// Вход в режим.
		const char ExitFromMode    = '\x48';    /// Выход из режима.

		/// Фискальные команды.
		const char OpenDocument    = '\x92';    /// Открыть документ.
		const char CloseDocument   = '\x4A';    /// Закрыть документ.
		const char Sale            = '\x52';    /// Продажа.
		const char Encashment      = '\x4F';    /// Выплата.
		const char CancelDocument  = '\x59';    /// Аннулировать чек.
		const char OpenFRSession   = '\x9A';    /// Открыть смену.

		/// Фискальные отчеты.
		const char XReport = '\x67';    /// Печать X-отчета.
		const char ZReport = '\x5A';    /// Печать Z отчета.
	}

	/// Коды состояний, возвращаемых в байте флагов на команду 3Fh.
	namespace States
	{
		const char Fiscalized           = 0x01;     /// Признак фискализированности ККМ.
		const char SessionOpened        = 0x02;     /// Признак открытой смены.
		const char CoverIsOpened        = 0x20;     /// Признак открытия крышки ККМ.
	}

	/// Ошибки.
	namespace Errors
	{
		const char EnterToModeIsLocked   = '\x1E';    /// Вход в режим заблокирован.
		const char BadModeForCommand     = '\x66';    /// Команда не может быть выполнена в текущем режиме.
		const char NoPaper               = '\x67';    /// Нет бумаги. Может означать все, что угодно.
		const char CannotExecCommand     = '\x7A';    /// Команда не может быть выполнена данной моделью ФР.
		const char NeedZReport           = '\x88';    /// Необходимо выполнить Z-отчет.
		const char NeedCloseSaleDocument = '\x89';    /// Необходимо закрыть чек продажи.
		const char WrongSeriesNumber     = '\x92';    /// Неверный номер ряда
		const char WrongFieldNumber      = '\x93';    /// Неверный номер поля
		const char NoMoneyForPayout      = '\x98';    /// В ККТ нет денег для выплаты.
		const char NeedCloseDocument     = '\x9B';    /// Необходимо закрыть фискальный документ.
		const char NeedCloseSession      = '\x9C';    /// Смена открыта, операция невозможна
		const char PrinterHeadOverheat   = '\xD1';    /// Перегрев головки принтера.
	}

	class CShortFlags: public CSpecification<char, int>
	{
	public:
		CShortFlags()
		{
			append('\x01', PrinterStatusCode::Error::PaperEnd);
			append('\x02', PrinterStatusCode::Error::PrinterFRNotAvailable);
			append('\x04', PrinterStatusCode::Error::PrintingHead);
			append('\x08', PrinterStatusCode::Error::Cutter);
			append('\x10', PrinterStatusCode::Error::Temperature);
			append('\x40', PrinterStatusCode::Error::PrinterFR);
			append('\x80', PrinterStatusCode::Warning::PaperNearEnd);
		}
	};

	static CShortFlags ShortFlags;

	namespace FRParameters
	{
		const SData TaxType                     = SData(2, 1, 11);
		const SData PrintSectionName            = SData(2, 1, 15);
		const SData ReportMode                  = SData(2, 1, 18);
		const SData EjectorParameters           = SData(2, 1, 22);
		const SData Cut                         = SData(2, 1, 24);
		const SData ResumeSize                  = SData(2, 1, 25);
		const SData PrintCashier                = SData(2, 1, 26);
		const SData ContinuousDocumentNumbering = SData(2, 1, 27);
		const SData AutoNullingChequeCounter    = SData(2, 1, 28);
		const SData LineSpacing                 = SData(2, 1, 30);
		const SData DocumentCapStringsAmount    = SData(2, 1, 36);
		const SData PrintSectionNumber          = SData(2, 1, 42);
		const SData OpeningSessionDocument      = SData(2, 1, 43);
		const SData PrintNotFiscalData          = SData(2, 1, 51);
		const SData PrintingSettings            = SData(2, 1, 55);

		inline SData    DocumentCap(int aSeries) { return SData( 6, ushort(aSeries), 1); }
		inline SData    SectionName(int aSeries) { return SData( 7, ushort(aSeries), 1); }
		inline SData            Tax(int aSeries) { return SData( 8, ushort(aSeries), 1); }
		inline SData TaxDescription(int aSeries) { return SData(13, ushort(aSeries), 1); }
	}

	/// Регистры
	namespace Registers
	{
		const char PaymentAmount[]     = "amount of payments";
		const char PaymentCount[]      = "count of successful payments";
		const char MoneyInCash[]       = "money in cash";
		const char CurrentDateTime[]   = "current date and time";
		const char SessionInfo[]       = "last session info";
		const char SerialNumber[]      = "serial number";
		const char NonNullableAmount[] = "non-nullable amount";
		const char PrintingSettings[]  = "printing settings";
	}

	/// Режимы.
	namespace InnerModes
	{
		const char NoMode       = '\xFF';
		const char Choice       = '\x00';
		const char Register     = '\x01';
		const char NotCancel    = '\x02';
		const char Cancel       = '\x03';
		const char Programming  = '\x04';
		const char AccessToFP   = '\x05';
		const char AccessToEKLZ = '\x06';
		const char ExtraCommand = '\x07';
	}

	/// Подрежимы.
	namespace InnerSubmodes
	{
		const char NoSubmode         = '\xFF';
		const char EnterDate         = '\x05';
		const char EnterTime         = '\x06';
		const char FMDataTimeError   = '\x0B';
		const char FMDataTimeConfirm = '\x0C';
		const char EKLZError         = '\x0E';
	}

	/// Пользователи. По умолчанию пароль для входа в режим совпадает с номером пользователя.
	namespace Users
	{
		const uchar Admin    = 0x29;
		const uchar SysAdmin = 0x30;
	}

	/// Данные языковых таблиц.
	typedef QMap<uchar, QString> TLanguages;

	struct SLanguages
	{
		TLanguages mLanguages;

		SLanguages()
		{
			mLanguages.insert( 0, "Russian");
			mLanguages.insert( 1, "Armenian");
			mLanguages.insert( 2, "Moldavian");
			mLanguages.insert( 3, "Ukrainian");
			mLanguages.insert( 4, "Lithuanian");
			mLanguages.insert( 5, "Turkmen");
			mLanguages.insert( 6, "Mongolian");
			mLanguages.insert( 7, "Belarus");
			mLanguages.insert( 8, "Latvian");
			mLanguages.insert( 9, "Georgian");
			mLanguages.insert(10, "Kazakh");
			mLanguages.insert(11, "Estonian");
			mLanguages.insert(12, "Azerbaijan");
			mLanguages.insert(13, "Kirghiz");
			mLanguages.insert(14, "Tadjik");
			mLanguages.insert(15, "Uzbek");
			mLanguages.insert(16, "Polish");
			mLanguages.insert(17, "Romanian");
			mLanguages.insert(18, "Bulgarian");
			mLanguages.insert(19, "English");
			mLanguages.insert(20, "Finnish");
		}

		QString operator[](const uchar languageKey) const
		{
			return mLanguages.contains(languageKey) ? mLanguages[languageKey] : "Unknown";
		}
	};

	static SLanguages Languages;
}

//--------------------------------------------------------------------------------
