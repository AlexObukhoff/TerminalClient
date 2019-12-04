/* @file Константы, коды команд и ответов онлайн ФР семейства MStar на протоколе AFP. */

#pragma once

// Modules
#include "Hardware/Common/DeviceCodeSpecification.h"
#include "Hardware/FR/FRErrorDescription.h"
#include "Hardware/Printers/Tags.h"

// Project
#include "AFPFRDataTypes.h"

//--------------------------------------------------------------------------------
namespace CAFPFR
{
	/// Минимальный размер данных ответа.
	const int MinAnswerDataSize = 4;

	/// Байт-разделитель.
	const char Separator = '\x1C';

	/// Имя модели.
	const char ModelName[] = "Multisoft MStar-TK2";

	/// Формат представления даты в ответах.
	const char DateFormat[] = "ddMMyyyy";

	/// Формат представления времени в ответах.
	const char TimeFormat[] = "hhmmss";

	/// Количество отделов.
	const int SectionAmount = 16;

	/// Флаг открытой смены.
	const char SessionOpened = '\x04';

	/// Флаг истекшей смены.
	const char SessionExpired = '\x08';

	//------------------------------------------------------------------------------------------------
	/// Статусы.
	namespace Statuses
	{
		/// Cтатусы принтера.
		class CPrinter : public BitmapDeviceCodeSpecification
		{
		public:
			CPrinter()
			{
				addStatus(0, PrinterStatusCode::Error::PrinterFR);
				addStatus(1, PrinterStatusCode::Error::PaperEnd);
				addStatus(2,  DeviceStatusCode::Error::CoverIsOpened);
				addStatus(3, PrinterStatusCode::Error::Cutter);
				addStatus(7, PrinterStatusCode::Error::PrinterFRNotAvailable);
			}
		};

		static CPrinter Printer;

		/// Cтатусы ФР.
		class CFR : public BitmapDeviceCodeSpecification
		{
		public:
			CFR()
			{
				addStatus(1, FRStatusCode::Warning::NotFiscalized);
				addStatus(4, FRStatusCode::Error::FS, "", true);
				addStatus(5, FRStatusCode::Error::FSClosed);
			}
		};

		static CFR FR;
	}

	//------------------------------------------------------------------------------------------------
	/// Данные ФР.
	namespace FRInfo
	{
		const SData SerialNumber     = SData( 1, EAnswerTypes::FInt,    "serial number");                     /// Заводской номер.
		const SData Firmware         = SData( 2, EAnswerTypes::FString, "firmware");                          /// Id прошивки.
		const SData INN              = SData( 3, EAnswerTypes::FInt,    "INN");                               /// ИНН.
		const SData RNM              = SData( 4, EAnswerTypes::FInt,    "RNM");                               /// Регистрационный номер.
		const SData LastFiscalDT     = SData( 5, TAnswerTypes() << EAnswerTypes::Date << EAnswerTypes::Time, "date and time of last fiscal document");    /// Дата и время последней фискальной операции.
		const SData LastRegDate      = SData( 6, EAnswerTypes::Date,    "last registration date");            /// Дата регистрации/перерегистрации.
		const SData TotalCash        = SData( 7, EAnswerTypes::Double,  "total sum in cash");                 /// Сумма наличных в денежном ящике.
		const SData NextFiscalNumber = SData( 8, EAnswerTypes::FInt,    "number of next fiscal document");    /// Номер следующего документа.
		const SData TotalPaySum      = SData(12, EAnswerTypes::Double,  "total sum of payments");             /// Нарастающий итог.
		const SData FFDFR            = SData(14, EAnswerTypes::Int,     "FFD FR");                            /// Код версии ФФД ФР.
		const SData ModelId          = SData(15, EAnswerTypes::FString, "model Id");                          /// Id модели.
		const SData FFDFS            = SData(16, TAnswerTypes() << EAnswerTypes::Int << EAnswerTypes::Int, "FFD FS");    /// Код версии ФФД ФН.
		const SData FirmwareDate     = SData(17, EAnswerTypes::Date, "Firmware date");                        /// Дата сборки прошивки.
	}

	//------------------------------------------------------------------------------------------------
	/// Типы документов.
	class CPayOffTypeData: public CSpecification<SDK::Driver::EPayOffTypes::Enum, char>
	{
	public:
		CPayOffTypeData()
		{
			using namespace SDK::Driver;

			append(EPayOffTypes::Debit,      2);
			append(EPayOffTypes::DebitBack,  3);
			append(EPayOffTypes::Credit,     6);
			append(EPayOffTypes::CreditBack, 7);
		}
	};

	static CPayOffTypeData PayOffTypeData;

	namespace DocumentTypes
	{
		const char NonFiscal = 1;    /// Нефискальный документ.
		const char PayIn     = 4;    /// Внесение.
		const char PayOut    = 5;    /// Выплата.
	}

	//------------------------------------------------------------------------------------------------
	/// Данные ФН.
	namespace FSData
	{
		/// Режим - фискальный.
		const int FiscalMode = 3;
	}

	//------------------------------------------------------------------------------------------------
	/// Параметры.
	namespace FRParameters
	{
		const SData NotPrintDocument = SData( 2, EAnswerTypes::Int,    "cheque parameters: not print document",        0);    /// Непечать  документов.
		const SData PrintingOnClose  = SData( 2, EAnswerTypes::Int,    "cheque parameters: print document on closing", 6);    /// Печать документов по закрытию чека.
		const SData OFDAddress       = SData(67, EAnswerTypes::String, "OFD address");    /// Адрес ОФД.
		const SData OFDPort          = SData(68, EAnswerTypes::String, "OFD port");       /// Порт ОФД.

		#define ADD_AFP_FF(aNumber, aField) const SData aField = SData(aNumber, CFR::FiscalFields::aField)

		ADD_AFP_FF(76, SenderMail);                 // 1117 (Электронная почта отправителя чека).
		ADD_AFP_FF(78, TransferOperatorPhone);      // 1075 (Телефон оператора перевода).
		ADD_AFP_FF(79, AgentOperation);             // 1044 (Операция платежного агента).
		ADD_AFP_FF(80, AgentPhone);                 // 1073 (Телефон платежного агента).
		ADD_AFP_FF(81, ProcessingPhone);            // 1074 (Телефон оператора по приему платежей).
		ADD_AFP_FF(82, TransferOperatorName);       // 1026 (Наименование оператора перевода).
		ADD_AFP_FF(83, TransferOperatorAddress);    // 1005 (Адрес оператора перевода).
		ADD_AFP_FF(84, TransferOperatorINN);        // 1016 (ИНН оператора перевода).
		ADD_AFP_FF(85, ProviderPhone);              // 1171 (Телефон поставщика).

		/// Имена отделов.
		inline SData SectionName(int aIndex) { return SData(50, EAnswerTypes::String, QString("section %1 name").arg(aIndex), NoBit, aIndex); }
	}

	//------------------------------------------------------------------------------------------------
	/// Команды.
	namespace Commands
	{
		const char GetFRData                = '\x02';    /// Запрос сведений.
		const char GetPrinterStatus         = '\x04';    /// Запрос статуса ПУ.
		const char GetFRStatus              = '\x05';    /// Запрос статуса ФР.
		const char GetFRParameter           = '\x11';    /// Получить параметр таблицы.
		const char SetFRParameter           = '\x12';    /// Установить параметр таблицы.
		const char GetFRDateTime            = '\x13';    /// Чтение даты-времени.
		const char XReport                  = '\x20';    /// Печать X-отчета.
		const char ZReport                  = '\x21';    /// Печать Z-отчета.
		const char OpenDocument             = '\x30';    /// Открыть документ.
		const char CloseDocument            = '\x31';    /// Закрыть документ.
		const char CancelDocument           = '\x32';    /// Аннулировать документ.
		const char PrintLine                = '\x40';    /// Печать строки.
		const char Sale                     = '\x42';    /// Продажа.
		const char Total                    = '\x47';    /// Оплата.
		const char PayIO                    = '\x48';    /// Внесение/выплата.
		const char OpenSession              = '\x54';    /// Открыть смену.
		const char GetFSStatus              = '\xB0';    /// Запрос статуса ФН.
		const char GetOFDStatus             = '\xB2';    /// Запрос параметров обмена с сервером ОФД.
		const char GetFiscalizationTotal    = '\xB6';    /// Получить итоги регистрации.
		const char SetUserContact           = '\xB8';    /// Установить телефон/электронный адрес покупателя.
		const char SetTaxSystem             = '\xBF';    /// Установить СНО.
		const char SetAgentFlag             = '\xC0';    /// Установить флаг агента.
		const char GetFiscalTLVData         = '\xC4';    /// Получить данные фискального документа в TLV-формате.
		const char GetLastFiscalizationData = '\xC7';    /// Получить данные последней фискализации из архива ФН.
	}

	//------------------------------------------------------------------------------------------------
	/// Запросы.
	namespace Requests
	{
		class CData : public CDataBase
		{
		public:
			CData()
			{
				using namespace EAnswerTypes;
				using namespace Commands;

				add(GetFRData,                TAnswerTypes() << Int  << Unknown, 2000);
				add(GetPrinterStatus,         TAnswerTypes() << Int, 7 * 1000);    //TODO: если нет бумаги - запрос статуса идет 5000..5500 мс
				add(GetFRStatus,              TAnswerTypes() << Int  << Int);
				add(GetFRParameter,           TAnswerTypes() << Unknown);
				add(GetFRDateTime,            TAnswerTypes() << Date << Time);
				add(GetFSStatus,              TAnswerTypes() << Int  << Int  << Int  << Int  << Int  << Date << Time << String << Int << String << Int << Date << Int << Int);
				add(GetOFDStatus,             TAnswerTypes() << Int  << Int  << Int  << Int  << Date << Time, 3 * 1000);
				add(GetFiscalizationTotal,    TAnswerTypes() << Date << Time << FInt << FInt << FInt << Int  << Int  << FInt   << FInt);
				add(GetFiscalTLVData,         TAnswerTypes() << String, 5000);
				add(GetLastFiscalizationData, TAnswerTypes() << Unknown);
				//                                               0        1      2       3       4       5       6       7         8       9        10     11      12     13

				add(ZReport,       10 * 1000);
				add(OpenDocument,   3 * 1000);
				add(CloseDocument, 20 * 1000);
				add(OpenSession,    3 * 1000);
			}
		};

		static CData Data;
	}

	//------------------------------------------------------------------------------------------------
	/// Ошибки.
	namespace Errors
	{
		const char WrongState     = '\x01';    /// Функция невыполнима при данном статусе.
		const char UnknownCommand = '\x03';    /// Некорректный формат или параметр команды.
		const char NeedZReport    = '\x0A';    /// Текущая смена больше 24 часов. Установка даты времени больше чем на 24 часа.

		class Data: public FRError::Data
		{
		public:
			Data()
			{
				using namespace FRError;

				// Ошибки выполнения команд
				add('\x01', "Функция невыполнима при данном статусе");
				add('\x02', "В команде указан неверный номер функции");
				add('\x03', "Некорректный формат или параметр команды");

				// Ошибки протокола передачи данных
				add('\x04', "Переполнение буфера коммуникационного порта");
				add('\x05', "Таймаут при передаче байта информации");
				add('\x06', "В протоколе указан неверный пароль");
				add('\x07', "Ошибка контрольной суммы в команде");

				// Ошибки печатающего устройства
				add('\x08', "Конец бумаги",             EType::Printer);
				add('\x09', "Принтер/дисплей не готов", EType::Printer);

				// Ошибки даты/времени
				add('\x0A', "Текущая смена больше 24 часов. Установка даты времени больше чем на 24 часа");
				add('\x0B', "Разница во времени, внутренних часов и указанной в команде начала работы, больше 8 минут");
				add('\x0C', "Вводимая дата более ранняя, чем дата последней фискальной операции");

				// Прочие ошибки
				add('\x0D', "Неверный пароль доступа к ФП");
				add('\x0E', "Отрицательный результат");
				add('\x0F', "Для выполнения команды необходимо закрыть смену");

				// Фатальные ошибки
				add('\x20', "Фатальная ошибка");
				add('\x21', "Нет свободного места в фискальной памяти");
			}
		};
	}

	//--------------------------------------------------------------------------------
	/// Теги для обработки флагом команды.
	class CTags: public CSpecification<Tags::Type::Enum, char>
	{
	public:
		CTags()
		{
			append(Tags::Type::Bold,         '\x08');
			append(Tags::Type::DoubleHeight, '\x10');
			append(Tags::Type::DoubleWidth,  '\x20');
			append(Tags::Type::UnderLine,    '\x80');
		}
	};

	static CTags Tags;
}

//--------------------------------------------------------------------------------
