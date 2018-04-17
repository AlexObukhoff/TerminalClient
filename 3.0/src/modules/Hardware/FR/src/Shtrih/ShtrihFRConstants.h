/* @file Общие константы, коды команд и ответов протоколов ФР Штрих. */

#pragma once

// SDK
#include <SDK/Drivers/FR/FiscalDataTypes.h>

// Modules
#include "Hardware/Common/ASCII.h"
#include "Hardware/Printers/Tags.h"

// Project
#include "Hardware/FR/FRErrorDescription.h"

//--------------------------------------------------------------------------------
namespace CShtrihFR
{
	/// Минимальный размер данных ответа.
	const int MinAnswerDataSize = 2;

	/// Максимальное число повторений пакета в случае логической ошибки в ответе.
	const int MaxRepeatPacket = 3;

	/// Максимальный размер строки версии ФР/ФП для вывода в лог.
	const int MaxVersionSize = 3;

	/// Максимальный размер строки билда ФР/ФП для вывода в лог.
	const int MaxBuildSize = 5;

	namespace Fonts
	{
		/// Шрифт по умолчанию.
		const uchar Default   = 0x01;

		/// Шрифт для Штрих-500.
		const uchar Shtrih500 = 0x05;
	}

	/// Печать на чековой ленте (а не на контрольной).
	const char PrintOnChequeTape = 0x02;

	/// Фиксированное количество символов в строке.
	const int FixedStringSize = 40;

	/// Типы фискальных чеков.
	namespace DocumentTypes
	{
		const char Sale     = '\x00';    /// Продажа.
		const char SaleBack = '\x02';    /// Возврат продажи.
	}

	/// Номер отдела для продажи.
	const char SectionNumber = 0x00;

	/// Описание товара.
	const QByteArray UnitName = QByteArray(FixedStringSize, ASCII::NUL);

	/// Максимальное количество строк рекламного текста в чеке.
	const char MaxAdvertStringsSize = 2;

	/// Начисление налогов - на каждую операцию в чеке/налог вычисляется в ФР.
	const char TaxForEachOperationInFR = 0x00;

	/// При печати фискального чека - печать налоговых ставок, оборота, названия, накопления.
	const char PrintAllTaxData = 0x02;

	/// Полная отрезка.
	const char FullCutting = 0x00;

	/// Неполная отрезка.
	const char PartialCutting = 0x01;

	/// При выдачи чека не задействовать презентер.
	/*
	//TODO: протестировать 2 вариант, понять специфику команды
	Тип выдачи (1 байт)
	1 - до срабатывания датчика на выходе из презентера (захватить чек)
	0 - не учитывать датчик (выброс чека)
	*/
	const char PushNoPresenter = 0x00;

	/// Тип протягиваемой бумаги - чековая лента (а не контрольная лента и не ПД).
	const char FeedChequeTape = 0x02;

	/// Длина презентации чека, [позиции], 1 поз ~ 4,2 мм.
	const char DefaultPresentationLength = 9;

	/// Формат представления даты и времени в ответе на длинный запрос статуса.
	const char DateTimeFormat[] = "ddMMyyyyhhmmss";

	/// Пароль администратора по умолчанию.
	const char AdminPassword = 30;

	/// Количество типов оплаты.
	const int PayTypeQuantity = 4;

	/// Налоги на закрытии чека по количеству налоговых  групп. Фиктивные, т.к. используются налоги на позицию.
	const QByteArray ClosingFiscalTaxes = QByteArray(4, ASCII::NUL);

	/// Данные языковых таблиц.
	class CLanguages : public CDescription<uchar>
	{
	public:
		CLanguages()
		{
			append(0, "Russian");
			append(1, "English");

			setDefault("Unknown");
		}
	};

	static CLanguages Languages;

	/// Таймауты, [мс].
	namespace Timeouts
	{
		/// Максимальный ожидания допечати X-отчета.
		const int MaxXReportNoAnswer = 30 * 1000;

		/// Максимальный ожидания выполнения Z-отчета.
		const int MaxZReportNoAnswer = 120 * 1000;

		/// Максимальный ожидания допечати.
		const int MaxWaitForPrintingEnd = 60 * 1000;

		/// Минимальный ожидания допечати.
		const int MinWaitForPrintingEnd = 10 * 1000;
	}

	/// Паузы, [мс].
	namespace Pause
	{
		/// Во время печати отложенных Z-отчетов.
		const int ZReportPrintingEnd = 350;

		/// После печати до начала отрезки.
		const int Cutting = 1000;
	}

	/// Интервалы, [мс].
	namespace Interval
	{
		/// Для запроса статуса при выполнении X- и Z-отчета.
		const int ReportPoll = 500;

		/// Для запроса статуса при допечати.
		const int WaitForPrintingEnd = 200;
	}

	/// Режимы (некотрые, используемые).
	namespace InnerModes
	{
		enum Enum
		{
			Work             =  0,      /// Принтер в рабочем режиме.
			DataEjecting     =  1,      /// Выдача данных.
			SessionOpened    =  2,      /// Открытая смена, 24 часа не кончились.
			NeedCloseSession =  3,      /// Открытая смена, 24 часа кончились.
			SessionClosed    =  4,      /// Закрытая смена.
			DocumentOpened   =  8,      /// Открытый документ.
			PrintFullZReport = 11,      /// Печать полного фис. отчета.
			PrintEKLZReport  = 12       /// Печать отчёта ЭКЛЗ.
		};

		/// Маска для режима.
		const char Mask = '\x0F';
	}

	/// Подрежимы (некотрые, используемые).
	namespace InnerSubmodes
	{
		enum Enum
		{
			PaperOn              = 0,   /// Бумага есть.
			PassivePaperOff      = 1,   /// Бумаги нет, ничего не печатали.
			ActivePaperOff       = 2,   /// Бумаги нет, закончилась при печати.
			NeedContinuePrinting = 3,   /// Ждем команду возобновления печати.
			PrintingFullReports  = 4,   /// Идет печать полных Z-отчетов.
			Printing             = 5    /// Идет печать.
		};
	}

	/// Коды команд.
	namespace Commands
	{
		/// Запросы статуса.
		const char GetShortStatus          = '\x10';     /// Короткий запрос состояния ФР.
		const char GetLongStatus           = '\x11';     /// Длинный запрос состояния ФР.

		/// Нефискальные операции.
		const char GetModelInfo            = '\xFC';     /// Получить тип устройства.
		const char GetMoneyRegister        = '\x1A';     /// Запросить денежный регистр.
		const char GetOperationalRegister  = '\x1B';     /// Запросить операционный регистр.
		const char GetFontSettings         = '\x26';     /// Запрос шрифта.
		const char Cut                     = '\x25';     /// Отрезка.
		const char PrintString             = '\x2F';     /// Печать строки данным шрифтом.
		const char Push                    = '\xF1';     /// Вытолкнуть чек (для моделей с ретрактором).
		const char SetFRParameter          = '\x1E';     /// Установить параметр таблицы ФР.
		const char GetFRParameter          = '\x1F';     /// Получить параметр таблицы ФР.
		const char ExtentionPrinting       = '\xB0';     /// Продолжить печать.

		/// Фискальные операции.
		const char OpenDocument            = '\x8D';     /// Открыть фискальный чек.
		const char CloseDocument           = '\x85';     /// Закрыть фискальный чек.
		const char Sale                    = '\x80';     /// Продажа.
		const char SaleBack                = '\x82';     /// Возврат продажи.
		const char CancelDocument          = '\x88';     /// Аннулировать фискальный чек.
		const char XReport                 = '\x40';     /// Снятие X отчета.
		const char ZReport                 = '\x41';     /// Снятие Z отчета.
		const char ZReportInBuffer         = '\xC6';     /// Снятие Z отчета в буфер.
		const char PrintDeferredZReports   = '\xC7';     /// Печать Z отчетов из буфера.
		const char Encashment              = '\x51';     /// Выплата.
		const char OpenFRSession           = '\xE0';     /// Открыть смену.
		const char BreakDataEjecting       = '\x03';     /// Прерывание выдачи данных.
		const char IdentifyVirtual         = '\xFF';     /// Идентифицировать виртуальный ФР.
	}

	/// Коды ошибок (некоторых).
	namespace Errors
	{
		const char NoError = '\x00';    /// Ошибок нет.

		const char WrongParametersInCommand = '\x33';      /// Некорректные параметры в команде.
		const char NotEnoughMoney           = '\x46';      /// Не хватает наличности в кассе.
		const char DocumentIsOpened         = '\x4A';      /// Открыт чек - операция невозможна.
		const char ChequeBufferOverflow     = '\x4B';      /// Буфер чеков переполнен.
		const char NeedWaitForPrinting      = '\x50';      /// Ожидание окончания печати предыдущей команды.
		const char NeedExtentionPrinting    = '\x58';      /// Ожидание команды продолжения печати.
		const char BadModeForCommand        = '\x73';      /// Команда не поддерживается в данном режиме.
		const char RAM                      = '\x74';      /// Ошибка ОЗУ.
		const char NeedZReport              = '\x4E';      /// Смена превысила 24 часа.
		const char Cutter                   = '\x71';      /// Ошибка отрезчика.
		const char BadModelForCommand       = '\x37';      /// Команда не поддерживается в данной реализации ФР.
	}

	/// Коды статусов (некоторых).
	namespace Statuses
	{
		/// Весовой датчик.
		namespace WeightSensor
		{
			const ushort NoControlPaper = 0x0001;    /// Рулон операционного журнала (0 – нет, 1 – есть).
			const ushort NoChequePaper  = 0x0002;    /// Рулон чековой ленты (0 – нет, 1 – есть).
		}

		/// Оптический датчик.
		namespace OpticalSensor
		{
			const ushort NoControlPaper = 0x0040;    /// Рулон операционного журнала (0 – нет, 1 – есть).
			const ushort NoChequePaper  = 0x0080;    /// Рулон чековой ленты (0 – нет, 1 – есть).
		}

		const ushort ControlLeverNotDropped = 0x0100;    /// Рычаг термоголовки контрольной ленты (0 – поднят, 1 – опущен).
		const ushort PaperLeverNotDropped   = 0x0200;    /// Рычаг термоголовки чековой ленты (0 – поднят, 1 – опущен).
		const ushort CoverNotClosed         = 0x0400;    /// Крышка корпуса ФР (0 – опущена, 1 – поднята).
		const ushort PaperInPresenter       = 0x0800;    /// Бумага на выходе из презентера.
	}

	/// Режим работы ККМ. Приходит в ответе на запрос FCh (получить тип устройства). Изменяться не может.
	namespace Types
	{
		const int NoType     =-1;

		const int KKM        = 0;
		const int Printer    = 5;
	}

	/// Параметры ФР.
	namespace FRParameters
	{
		/// Налоги.
		namespace Taxes
		{
			/// Значения налоговой группы
			const SData Value = FRParameters::SData(1, 6, "tax value");

			/// Описание налоговой группы
			const SData Description = FRParameters::SData(2, 6, "tax description");
		}

		/// Названия секций
		const SData SectionName = FRParameters::SData(1, 7, "section name");
	}

	/// Регистры.
	namespace Registers
	{
		/// Денежные регистры.
		const TRegisterId PaymentAmount    = TRegisterId(121, ERegisterType::Money);
		const TRegisterId TotalCashSum     = TRegisterId(241, ERegisterType::Money);
		const TRegisterId ZReportsQuantity = TRegisterId(184, ERegisterType::Money);

		/// Операционные регистры.
		const TRegisterId PaymentCount     = TRegisterId( 72, ERegisterType::Operational);
		const TRegisterId SalesCount       = TRegisterId(144, ERegisterType::Operational);
		const TRegisterId SalesBackCount   = TRegisterId(146, ERegisterType::Operational);

		struct SData
		{
			QString description;
			QString typeDescription;

			SData(const QString & aDescription, const QString & aTypeDescription): description(aDescription), typeDescription(aTypeDescription) {}
		};

		class CData : public CDescription<TRegisterId>
		{
		public:
			CData()
			{
				append(PaymentAmount,    "payment amount in 1-st section");
				append(TotalCashSum,     "total sum in cash");
				append(ZReportsQuantity, "quantity of Z-reports in buffer");

				append(PaymentCount,     "payment count in 1-st section");
				append(SalesCount,       "quantity of fiscal sales document in session");
				append(SalesBackCount,   "quantity of fiscal sales back document in session");
			}

			SData getInfo(const TRegisterId & aRegister)
			{
				QString typeDescription = "Unknown";
				     if (aRegister.second == ERegisterType::Money)       typeDescription = "Money";
				else if (aRegister.second == ERegisterType::Operational) typeDescription = "Operational";

				return SData(data().value(aRegister), typeDescription);
			}
		};

		static CData Data;
	}
}

//--------------------------------------------------------------------------------
