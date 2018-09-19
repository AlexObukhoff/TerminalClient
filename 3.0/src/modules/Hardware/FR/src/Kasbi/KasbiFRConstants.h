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

	/// Формат представления даты для вывода в лог.
	const char TimeLogFormat[] = "hh:mm";

	/// Размер шрифта - уменьшенный.
	const char FontSize = 1;

	/// Ретракцию для отчетов открытия/закрытия смены не выполнять.
	const char SessionReportNoRetraction = '\x00';

	/// Id для определения версии ФН 1.0.
	const char FS10Id[] = "v_1_0";

	/// Id для определения версии ФН 1.1.
	const char FS11Id[] = "v_1_1";

	/// Последняя актуальная прошивка.
	const char LastFirmware[] = "1.0.21";

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
		const char SendAgentData         = '\x2C';    /// Передать данные платежного агента.
		const char Total                 = '\x2D';    /// Итог.
		const char CloseDocument         = '\x24';    /// Закрыть фискальный чек.
		const char CancelDocument        = '\x10';    /// Аннулировать чек.
		const char StartXReport          = '\x27';    /// Начать формирование X-отчета.
		const char EndXReport            = '\x28';    /// Закончить формирование X-отчета.
		const char StartZReport          = '\x29';    /// Начать формирование Z-отчета.
		const char EndZReport            = '\x2A';    /// Закончить формирование Z-отчета.
		const char StartOpeningSession   = '\x21';    /// Начать открытие смены.
		const char OpenSession           = '\x22';    /// Открыть смену.
		const char StartFiscalTLVData    = '\x35';    /// Начать получение данных фискального документа в TLV-формате.
		const char GetFiscalTLVData      = '\x36';    /// Получить данные фискального документа в TLV-формате.

		class CData : public CSpecification<char, int>
		{
		public:
			CData()
			{
				append(CloseDocument, 5 * 1000);
				append(EndXReport,    2 * 1000);
				append(EndZReport,   60 * 1000);
				append(OpenSession,   2 * 1000);
				append(GetStatus,     2 * 1000);

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
			append(1, PrinterStatusCode::Error::PrinterFRNotAvailable);
			append(2, PrinterStatusCode::Error::PaperEnd);
			append(3, PrinterStatusCode::Error::PaperJam);
			append(5,  DeviceStatusCode::Error::CoverIsOpened);
			append(6, PrinterStatusCode::Error::Cutter);
			append(7, PrinterStatusCode::Error::PrinterFR);
		}
	};

	static CStatuses Statuses;

	//------------------------------------------------------------------------------------------------
	/// Фискальные реквизиты.
	namespace FiscalFields
	{
		class Data: public CSSpecification<int, CFR::FiscalFields::SData>
		{
		public:
			QString getTextLog(int aField) const { return QString("fiscal field %1 (%2)").arg(aField).arg(value(aField).textKey.replace("_", " ")); }
		};

		#define ADD_KASBI_FF(aField, aName, aType) ADD_STATIC_DATA(CKasbiFR::FiscalFields::Data, int, aName, aField, \
			CFR::FiscalFields::SData(CFR::FiscalFields::ETypes::aType, #aName, #aName));

		ADD_KASBI_FF(30000, FRDateTime,              ByteArray);    /// Дата/время ККТ
		ADD_KASBI_FF(30005, OFDAddress,              ByteArray);    /// Адрес ОФД
		ADD_KASBI_FF(30006, OFDPort,                 ByteArray);    /// Порт ОФД
		ADD_KASBI_FF(30009, OFDInterval,             UINT16);       /// Интервал таймера ОФД
		ADD_KASBI_FF(30015, FontSize,                Byte);         /// Размер шрифта
		ADD_KASBI_FF(30017, OptionalFiscalParameter, String);       /// Дополнительный параметр на фискальном чеке
		ADD_KASBI_FF(30018, SessionReportRetraction, Byte);         /// Ретракция отчетов открытия/закрытия смены
		ADD_KASBI_FF(30019, PrinterModel,            Byte);         /// Модель принтера
		ADD_KASBI_FF(30020, FullReportType,          Byte);         /// Печатать денежные счетчики перед закрытием смены
		ADD_KASBI_FF(30021, PrinterBaudRate,         UINT32);       /// Скорость подключаемого ПУ
		ADD_KASBI_FF(30034, Reserved,                String);       /// Зарезервирован
		ADD_KASBI_FF(30040, OFDName,                 String);       /// Наименование ОФД
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
				add('\x34', "Данные команды уже были переданы ранее");
				add('\x35', "Аппаратный сбой ККТ");
				add('\x36', "Неверно указан признак расчета, возможные значения: приход, расход, возврат прихода, возврат расхода");
				add('\x37', "Указанный налог не может быть применен");
				add('\x38', "Команда необходима только для платежного агента (указано при регистрации)");
				add('\x39', "Сумма расчета чека не равна сумме следующих значений по чеку: сумма наличными, сумма электронными, сумма предоплатой, сумма постоплатой, сумма встречным предоставлением");
				add('\x3A', "Сумма оплаты соответствующими типами (за исключением наличных) превышает итог чека");
				add('\x3B', "Некорректная разрядность итога чека");
				add('\x3C', "Некорректная разрядность денежных величин");
				add('\x3D', "Превышено максимально допустимое количество предметов расчета в чеке");
				add('\x3E', "Превышено максимально допустимое количество предметов расчета c данными агента в чеке");
				add('\x3F', "Невозможно передать данные агента, допустимы данные агента либо для всего чека, либо данные агента по предметам расчета");
				add('\x40', "Некорректный статус печатающего устройства");
				add('\x42', "Сумма изъятия больше доступной суммы наличных в ККТ");
				add('\x43', "Операция внесения-изъятия денег в ККТ возможна только при открытой смене");
				add('\x44', "Счетчики денег не инициализированы");
				add('\x45', "Сумма по чеку коррекции всеми типами оплаты не равна полной сумме для расчетов по ставкам НДС");
				add('\x46', "Сумма по чеку коррекции всеми типами оплаты не равна итоговой сумме чека коррекции");
				add('\x47', "В чеке коррекции не указано ни одной суммы для расчетов по ставкам НДС");
				add('\x50', "Ошибка сохранения настроек");
				add('\x51', "Передано некорректное значение времени");
				add('\x52', "В чеке не должны присутствовать иные предметы расчета помимо предмета расчета с признаком способа расчета «Оплата кредита»");
				add('\x53', "Переданы не все необходимые данные для агента");
				add('\x54', "Итоговая сумма расчета(в рублях без учета копеек) не равна сумме стоимости всех предметов расчета(в рублях без учета копеек)");
				add('\x55', "Неверно указан признак расчета для чека коррекции, возможные значения: приход, расход");
				add('\x56', "Неверная структура переданных данных для агента");
				add('\x57', "Не указан режим налогообложения");
				add('\x58', "Данная ставка НДС недопустима для агента. Агент не является плательщиком НДС");
				add('\x59', "Не указано или неверно указано значение тэга \"Признак платежного агента\"");
				add('\x5A', "Невозможно внести товарную позицию уже после внесения данных об оплате");
				add('\x5B', "Команда может быть выполнена только при открытом чеке");
				add('\x5C', "Некорректный формат или длина в массиве переданных строк нефискальной информации");
				add('\x5D', "Достигнуто максимальное количество строк нефискальной информации");
				add('\x5E', "Не переданы данные кассира");
				add('\x60', "Номер блока прошивки указан некорректно");
				add('\x70', "Значение не зашито в ККТ");
				add('\x71', "Некорректное значение серийного номера");
				add('\x7F', "Команда не выполнена");
				add('\xE0', "Присутствуют неотправленные в ОФД документы");
				add('\xF3', "Подключенный ФН не соответствует данным регистрации ККТ");
				add('\xF4', "ФН еще не был активирован");
				add('\xF5', "ФН был закрыт");
			}
		};

		static CData Data;
	}

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
