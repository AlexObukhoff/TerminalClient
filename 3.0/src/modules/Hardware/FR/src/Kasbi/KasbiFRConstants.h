/* @file Константы, коды команд и ответов протокола ФР АТОЛ. */

#pragma once

// Modules
#include "Hardware/Common/ASCII.h"

// Project
#include "Hardware/Printers/Tags.h"
#include "Hardware/FR/FRErrorDescription.h"

//--------------------------------------------------------------------------------
namespace CKasbiFR
{
	/// Минимальный размер распакованных данных.
	const int MinUnpackedAnswerSize = 1;

	/// Минимальный размер распакованных данных.
	const int MinUnpackedErrorSize = 2;

	/// Признак способа расчета - полная предварительная оплата.
	const char FullPrepaymentSettlement = '\x01';

	/// Формат представления даты для вывода в лог.
	const char TimeLogFormat[] = "hh:mm";

	/// Размер шрифта - уменьшенный.
	const char FontSize = 1;

	/// Ретракцию для отчетов открытия/закрытия смены не выполнять.
	const char NoSessionReportRetraction = '\x00';

	/// Признак вывода на печать.
	namespace Print
	{
		const char Yes = '\x00';    /// Печатать
		const char No  = '\x01';    /// Не печатать
	}

	/// Таймауты чтения, [мс].
	namespace Timeouts
	{
		/// По умолчанию.
		const int Default = 500;
	}

	/// Признаки способа расчета.
	namespace SettlementTypes
	{
		const char Income          = 1;    /// Приход
		const char IncomeReturning = 2;    /// Возврат прихода
	}

	/// Данные ФН.
	struct SFSData
	{
		bool documentOpened;
		bool sessionOpened;
		char flags;
		qulonglong number;
		uint lastFDNumber;

		SFSData() : documentOpened(false), sessionOpened(false), flags(ASCII::NUL), number(0), lastFDNumber(0) {}
		SFSData(bool aDocumentOpened, bool aSessionOpened, char aFlags, qulonglong aNumber, uint aLastFDNumber) :
			documentOpened(aDocumentOpened), sessionOpened(aSessionOpened), flags(aFlags), number(aNumber), lastFDNumber(aLastFDNumber) {}
	};

	//------------------------------------------------------------------------------------------------
	/// Режимы работы ФР.
	class Modes : public CBitmapDescription<char>
	{
	public:
		Modes()
		{
			addBit(0, "encryption");
			addBit(1, "offline");
			addBit(2, "automatic");
			addBit(3, "service sector");
			addBit(4, "strict accountancy documents (SAD)");
			addBit(5, "internet");
		}

		virtual QString getValues(char aValue)
		{
			QString result = CBitmapDescription<char>::getValues(aValue);

			if (~aValue & (1 << 4))
			{
				result += QString(result.isEmpty() ? "" : ", ") + "cash register receipt";
			}

			return result;
		}
	};

	//------------------------------------------------------------------------------------------------
	/// Коды команд.
	namespace Commands
	{
		// Команды получения информации об устройстве
		const char GetStatus             = '\x01';    /// Статус ККТ
		const char GetSerial             = '\x02';    /// Серийный номер ККТ
		const char GetFirmware           = '\x03';    /// Получить инфо о ПО девайса
		const char GetModelInfo          = '\x04';    /// Модель ККТ
		const char GetFSSerial           = '\x05';    /// Серийный номер ФН
		const char GetFSVersion          = '\x06';    /// Версия ПО ФН
		const char GetFSData             = '\x07';    /// Данные ФН
		const char GetFSStatus           = '\x08';    /// Статус ФН
		const char GetRegistrationData   = '\x0A';    /// Получить параметры регистрации ККТ
		const char GetVersion            = '\x0B';    /// Получить версию ПО
		const char GetOFDNotSentCount    = '\x32';    /// Получить количество неотправленных в ОФД документов
		const char GetFRDateTime         = '\x73';    /// Получить дату\время из ККТ
		const char GetOFDData            = '\x77';    /// Получить настройки ОФД
		const char SetPrintingParameters = '\x78';    /// Установить настройки печати
		const char GetPrintingParameters = '\x79';    /// Получить настройки печати

		// Печать
		const char PrintLine             = '\x61';    /// Печать строки
		const char Cut                   = '\x62';    /// Отрезка

		// Фискальные операции
		const char OpenDocument          = '\x23';    /// Открыть фискальный чек.
		const char Sale                  = '\x2B';    /// Продажа.
		const char Total                 = '\x2D';    /// Итог.
		const char CloseDocument         = '\x24';    /// Закрыть фискальный чек.
		const char CancelDocument        = '\x10';    /// Аннулировать чек.
		const char StartXReport          = '\x27';    /// Начать формирование X-отчета.
		const char EndXReport            = '\x28';    /// Закончить формирование X-отчета.
		const char StartZReport          = '\x29';    /// Начать формирование Z-отчета.
		const char EndZReport            = '\x2A';    /// Закончить формирование Z-отчета.
		const char StartOpeningSession   = '\x21';    /// Начать открытие смены.
		const char OpenSession           = '\x22';    /// Открыть смену.

		class CData : public CSpecification<char, int>
		{
		public:
			CData()
			{
				append(CloseDocument, 5 * 1000);
				append(EndXReport,    2 * 1000);
				append(EndZReport,   60 * 1000);
				append(OpenSession,   2 * 1000);

				setDefault(Timeouts::Default);
			}
		};

		static CData Data;
	}

	class CStatuses: public CSpecification<char, int>
	{
	public:
		CStatuses()
		{
			append(1, PrinterStatusCode::Error::PrinterFR);
			append(2, PrinterStatusCode::Error::PaperEnd);
			append(3, PrinterStatusCode::Error::PaperJam);
			append(5, DeviceStatusCode::Error::CoverIsOpened);
			append(6, PrinterStatusCode::Error::Cutter);
			append(7, PrinterStatusCode::Error::PrinterFR);
		}
	};

	static CStatuses Statuses;

	//------------------------------------------------------------------------------------------------
	/// Фискальные реквизиты.
	namespace FiscalFields
	{
		const int FRDateTime              = 30000;    // Дата/время ККТ
		const int OFDAddress              = 30005;    // Адрес ОФД
		const int OFDPort                 = 30006;    // Порт ОФД
		const int FontSize                = 30015;    // Размер шрифта
		const int OptionalFiscalParameter = 30017;    // Дополнительный параметр на фискальном чеке
		const int SessionReportRetraction = 30018;    // Ретракция отчетов открытия/закрытия смены
		const int PrinterModel            = 30019;    // Модель принтера
	}

	//------------------------------------------------------------------------------------------------
	/// Ошибки.
	namespace Errors
	{
		const char WrongFSState     = '\x02';    /// Данная команда требует другого состояния ФН
		const char NeedZReport      = '\x16';    /// Продолжительность смены более 24 часов
		const char Protocol         = '\x25';    /// Неверная структура команды, либо неверная контрольная сумма
		const char UnknownCommand   = '\x26';    /// Неизвестная команда
		const char WrongTotalSum    = '\x39';    /// Итоговая сумма оплаты не равна стоимости предметов расчета
		const char NeedAgentData    = '\x53';    /// Переданы не все необходимые данные для агента
		const char WrongVATForAgent = '\x58';    /// Данная ставка НДС недопустима для агента. Агент не является плательщиком НДС

		class CData : public FRError::CData
		{
		public:
			CData()
			{
				add('\x01', "Неверный формат команды");
				add('\x02', "Данная команда требует другого состояния ФН");
				add('\x03', "Ошибка ФН");
				add('\x04', "Ошибка KC");
				add('\x05', "Закончен срок эксплуатации ФН");
				add('\x06', "Архив ФН переполнен");
				add('\x07', "Дата и время операции не соответствуют логике работы ФН");
				add('\x08', "Запрошенные данные отсутствуют в Архиве ФН");
				add('\x09', "Параметры команды имеют правильный формат, но их значение не верно");
				add('\x10', "Превышение размеров TLV данных");
				add('\x12', "Исчерпан ресурс КС. Требуется закрытие фискального режима");
				add('\x14', "Ресурс хранения документов для ОФД исчерпан");
				add('\x15', "Превышено время ожидания передачи сообщения (30 дней)");
				add('\x16', "Продолжительность смены более 24 часов");
				add('\x17', "Неверная разница во времени между 2 операциями (более 5 минут)");
				add('\x20', "Сообщение от ОФД не может быть принято");
				add('\x25', "Неверная структура команды, либо неверная контрольная сумма");
				add('\x26', "Неизвестная команда");
				add('\x27', "Неверная длина параметров команды");
				add('\x28', "Неверный формат или значение параметров команды");
				add('\x30', "Нет связи с ФН");
				add('\x31', "Неверные дата/время в ККТ");
				add('\x32', "Переданы не все необходимые данные");
				add('\x33', "РНМ сформирован неверно, проверка на данной ККТ не прошла");
				add('\x34', "Данные уже были переданы ранее");
				add('\x35', "Аппаратный сбой ККТ");
				add('\x36', "Неверно указан признак расчета, возможные значения: приход, расход, возврат прихода, возврат расхода");
				add('\x37', "Указанный налог не может быть применен");
				add('\x38', "Данные необходимы только для платежного агента (указано при регистрации)");
				add('\x39', "Итоговая сумма оплаты не равна стоимости предметов расчета");
				add('\x40', "Некорректный статус печатающего устройства");
				add('\x50', "Ошибка сохранения настроек");
				add('\x51', "Передано некорректное значение времени");
				add('\x52', "В чеке не должны присутствовать иные предметы расчета помимо предмета расчета с признаком способа расчета \"Оплата кредита\"");
				add('\x53', "Переданы не все необходимые данные для агента");
				add('\x54', "Итоговая сумма чека не равна сумме оплаты всеми видами");
				add('\x55', "Неверно указан признак расчета для чека коррекции, возможные значения: приход, расход");
				add('\x56', "Неверная структура переданных данных для агента");
				add('\x57', "Не указан режим налогообложения");
				add('\x58', "Данная ставка НДС недопустима для агента. Агент не является плательщиком НДС");
				add('\x59', "Некорректно указано значение тэга \"Признак платежного агента\"");
				add('\x60', "Номер блока прошивки указан некорректно");
				add('\xE0', "Присутствуют неотправленные в ОФД документы");
			}
		};

		static CData Data;
	}

	//--------------------------------------------------------------------------------
	/// Модели принтеров.
	class CPrinterModels : public CDescription<char>
	{
	public:
		CPrinterModels()
		{
			append(0, "Auto");
			append(1, "Custom VKP-80");
			append(2, "Custom TG-2480");
			append(3, "Citizen CT-S2000");
			append(4, "Citizen PPU-700");
			append(5, "Star TSP");
			append(6, "Star TUP");
			append(7, "Epson EU-422");
		}
	};

	static CPrinterModels PrinterModels;

	//--------------------------------------------------------------------------------
	/// Теги.
	class TagEngine : public Tags::Engine
	{
	public:
		TagEngine()
		{
			set(Tags::Type::DoubleHeight);
			set(Tags::Type::DoubleWidth);
			set(Tags::Type::Italic);
			set(Tags::Type::UnderLine);
			set(Tags::Type::Center);
		}
	};

	/// Теги для обработки постфиксом команды.
	class CTags: public CSpecification<Tags::Type::Enum, char>
	{
	public:
		CTags()
		{
			append(Tags::Type::Bold,         '\x08');
			append(Tags::Type::DoubleHeight, '\x10');
			append(Tags::Type::DoubleWidth,  '\x20');
			append(Tags::Type::Italic,       '\x40');
			append(Tags::Type::UnderLine,    '\x80');
		}
	};

	static CTags Tags;

	/// Тег центрования.
	const char CenterTag = '\x01';
}

//--------------------------------------------------------------------------------
