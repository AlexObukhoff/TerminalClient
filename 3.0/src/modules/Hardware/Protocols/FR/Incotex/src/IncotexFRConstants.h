/* @file Константы, коды команд и ответов протокола ФР МСтар. */

#pragma once

#include "Hardware/Common/ASCII.h"

//--------------------------------------------------------------------------------
/// Константы, команды и коды состояний устройств на протоколе Incotex.
namespace CIncotexFR
{
	namespace Constants
	{
		/// Управляющие символы.
		const char CAN = ASCII::ENQ;

		/// Пароль на передачу данных.
		const QString ModelInfoPassword  = "0000";

		/// Начало пакета - фиксированный байт.
		const char Prefix = ASCII::STX;

		/// Байт-разделитель.
		const char Separator = ASCII::NUL;

		/// Конец пакета - фиксированный байт.
		const char Postfix = ASCII::ETX;

		/// Минимальный размер ответного пакета.
		const int MinAnswerSize = 19;

		/// Тип фискального чека - продажа.
		const QString SaleFiscalDocument = "00";

		/// Тип отчета - X-отчет.
		const char XReport = 1;

		/// Тип отчета - Z-отчет.
		const char ZReport = 0;

		/// Номер кассира.
		const char CashierNumber = 0;

		/// Номер кассира.
		const int EKLZStatusRequest = 318;

		/// Длина некоторых полей документа.
		namespace Length
		{
			const int Operation        = 1;     /// Фискальная операция.
			const int ElementsQuantity = 3;     /// Количество передаваемых реквизитов.
			const int X                = 2;     /// Позиция X реквизита.
			const int Y                = 3;     /// Позиция Y реквизита.
			const int Reserved         = 5;     /// Зарезервированное поле реквизита.
			const int Element          = 61;    /// Весь реквизит.

			const int Report           = 1;     /// Отчеты.
			const int Cashier          = 40;    /// Фамилия кассира.
			const int FRParameter      = 3;     /// Номер параметра ФР.

			/// Нефискальные строки.
			namespace ElementData
			{
				const int Default      = 41;    /// По умолчанию для ПФД.
				const int Encashment   = 40;    /// Инкассация.
			}
		}
	}

	/// Позиции.
	namespace Positions
	{
		/// Команда, позиция с начала посылки.
		const int Command = 1;

		/// Количество элементов в буфере данных команды фискального документа.
		const int ElementAmount = 5;

		/// Уплаченная сумма в буфере данных команды фискального документа.
		const int PaidSum = 23;

		/// CRC, позиция с конца посылки.
		const int CRC = 3;

		/// Размер CRC.
		const int CRCsize = 2;
	}

	/// Фискальные данные.
	namespace FiscalData
	{
		/// Тип операции - Продажа.
		const uchar Sale = 0;

		/// Тип операции - Выплата.
		const uchar Encashment = 3;

		/// Количество передаваемых реквизитов.
		const uchar ElementsQuantity = 16;

		/// Реквизиты (некоторые, часто используемые).
		namespace Elements
		{
			const char KKMNumber      = 0;     /// Номер ККМ.
			const char DocumentCap1   = 1;     /// Строка 1 шапки чека.
			const char DocumentCap2   = 2;     /// Строка 2 шапки чека.
			const char DocumentCap3   = 3;     /// Строка 3 шапки чека.
			const char DocumentCap4   = 4;     /// Строка 4 шапки чека.
			const char DateTime       = 5;     /// Дата и время совершения операции.
			const char CashierNumber  = 6;     /// Номер кассира.
			const char DocumentNumber = 7;     /// Номер документа.
			const char ReceiptNumber  = 8;     /// Номер чека.
			const char AccountNumber  = 9;     /// Номер счёта (необязательный).
			const char INN            = 10;    /// ИНН.
			const char Price          = 11;    /// Цена услуги.
			const char TotalSum       = 12;    /// Итоговая сумма.
			const char PaidSum        = 13;    /// Уплаченная сумма.
			const char OddSum         = 14;    /// Сумма сдачи.

			const char Free           = 99;    /// Сумма сдачи.
		}

		/// Флаги операций и реквизитов.
		namespace Flags
		{
			/// Закрыть документ.
			const uchar CloseDocument = 0x04;

			/// Оформить (сформировать) документ.
			const uchar MakeDocument = 0x00;

			/// Использовать шрифты по умолчанию.
			const ushort DefaultFont  = 0x0020;

			/// Продажа товара за денежную сумму.
			const ushort SaleForMoney = 0x0002;

			/// Краткий отчет (без печати нулевых счётчиков).
			const char ShortReport = 0x04;
		}
	}

	/// Таймауты, [мс].
	namespace Timeouts
	{
		/// Дефолтный таймаут для чтения символов.
		const int Default = 200;

		/// Максимальный таймаут на получение произвольных символов.
		const int MaxWork = 1000;

		/// Максимальный таймаут на получение ответа.
		const int MaxAnswer = 120000;
	}

	/// Коды команд.
	namespace Commands
	{
		const char Sale                = '\x53';
		const char Report              = '\x5F';
		const char GetStatus           = '\x44';
		const char GetModelInfo        = '\x45';
		const char SetPrinterMode      = '\x36';
		const char GetFRParameters     = '\x5C';
		const char CashierRegistration = '\x31';
		const char GetRegister         = '\x55';

		const char SetFiscalMode[]     = "\x1B\x1B";
		const char Cut[] = "\x1B\x64\x30";
	}

	/// Регистры - не все, используемые.
	namespace Registers
	{
		const int SumInCash = 11;
	}

	namespace Errors
	{
		/// Коды обрабатываемых ошибок.
		namespace Codes
		{
			const unsigned short CashierNotRegistered = 0x000D;
			const unsigned short NeedCloseSession     = 0x0002;
		}

		class CDescriptions : public CDescription<unsigned short>
		{
		public:
			CDescriptions()
			{
				setDefault("Неизвестная");

				append(0x0001, "Ошибка в фискальных данных, аппарат блокирован");
				append(0x0002, "Не закрыта смена");
				append(0x0003, "Исчерпан ресурс сменных записей в фискальную память");
				append(0x0004, "Превышена длина поля команды");
				append(0x0005, "Неверный формат поля команды");
				append(0x0006, "Ошибка чтения таймера");
				append(0x0007, "Неверная дата");
				append(0x0008, "Неверное время");
				append(0x0009, "Дата меньше последней даты, зарегистрированной в фискальной памяти");
				append(0x000A, "Операция прервана пользователем. Документ аннулирован");
				append(0x000B, "Запрещенная команда ПУ");
				append(0x000C, "Не открыта смена");
				append(0x000D, "Кассир не зарегистрирован");
				append(0x000E, "Переполнение приёмного буфера");
				append(0x000F, "Ошибка записи в фискальную память");
				append(0x0010, "Ошибка установки таймера");
				append(0x0011, "Неверный пароль налогового инспектора");
				append(0x0012, "Неверный пароль на связь");
				append(0x0013, "Исчерпан ресурс перерегистраций");
				append(0x0014, "Аппарат не фискализирован");
				append(0x0015, "Значение поля команды вне диапазона");
				append(0x0016, "Ошибка чтения фискальной памяти");
				append(0x0017, "Переполнение или отрицательный результат счётчика");
				append(0x0018, "Обязательное поле команды имеет нулевую длину");
				append(0x0019, "Неверный формат команды");
				append(0x001A, "Дата или время последнего документа в смене меньше предыдущего");
				append(0x001B, "Не используется");
				append(0x001C, "Ошибка в расположении реквизитов (пересечение или выход за область печати)");
				append(0x001D, "Нет такой команды");
				append(0x001E, "Неверная контрольная сумма BCC");
				append(0x001F, "Нет фискальных записей");
				append(0x0021, "Оформление документа прервано по окончанию времени ожидания готовности принтера");
				append(0x0024, "Буфер ответа пуст");
				append(0x0025, "Услуга не введена");
				append(0x0029, "Дублирование обязательных реквизитов документа");
				append(0x002A, "Текущее состояние ККМ не позволяет выполнить операцию");
				append(0x002B, "Ошибка в данных энергонезависимой памяти. Аппарат блокирован");
				append(0x002C, "Невозможно выполнить инициализацию ФП. ФП уже инициализирована");
				append(0x002D, "Вывод прерван по окончанию времени ожидания готовности дисплея");
				append(0x002E, "Ошибка записи FLASH памяти");
				append(0x002F, "Ошибка. Нет записей");
				append(0x0030, "Ошибка связи с ЭКЛЗ");
				append(0x0031, "Некорректный формат или параметр команды ЭКЛЗ");
				append(0x0032, "Некорректное состояние ЭКЛЗ");
				append(0x0033, "Авария ЭКЛЗ");
				append(0x0034, "Авария криптографического процессора ЭКЛЗ");
				append(0x0035, "Исчерпан временной ресурс использования ЭКЛЗ");
				append(0x0036, "ЭКЛЗ переполнена");
				append(0x0037, "Неверные дата или время в ЭКЛЗ");
				append(0x0038, "Нет запрошенных данных в ЭКЛЗ");
				append(0x0039, "Переполнение счётчиков ЭКЛЗ");
				append(0x0046, "Ошибка протокола обмена ЭКЛЗ");
				append(0x0047, "Переполнение приёмного буфера ЭКЛЗ");
				append(0x0048, "Неверная контрольная сумма ЭКЛЗ");
				append(0x0049, "ЭКЛЗ активизирована в составе другой ККМ. Аппарат блокирован");
				append(0x004A, "ЭКЛЗ не активизирована");
				append(0x004B, "Неисправимая ошибка ЭКЛЗ");
				append(0x004C, "Исчерпан ресурс активизаций ЭКЛЗ");
				append(0x004D, "ЭКЛЗ уже активизирована");
				append(0x004E, "Превышено количество секций в документе");
				append(0x004F, "Архив ЭКЛЗ закрыт или переполнение архива");
				append(0x0050, "Ошибка. Данные фискальной памяти и ЭКЛЗ различаются");
			}
		};

		static CDescriptions Descriptions;
	}

	/// Структура, в которую парсится ответ. На разные команды в ответе приходят разные данные.
	struct SUnpackedData : SUnpackedDataBase
	{
		unsigned char  command;
		unsigned short commandResult;

		/// Статусы ККМ.
		bool FiscalMemoryNearEnd;
		bool FiscalMemoryEnd;
		bool EKLZNearEnd;
		bool EKLZError;

		/// Статусы принтера.
		bool TechnoMode;
		bool PrinterError;
		bool CutterOff;
		bool PaperEnd;
		bool OfflineError;

		QByteArray modelName;
		QByteArray vendorName;
		QByteArray softVersion;

		/// Сумма фискального регистра.
		double Register;

		SUnpackedData() : command(0), commandResult(0),
			FiscalMemoryNearEnd(false), FiscalMemoryEnd(false), EKLZNearEnd(false), EKLZError(false),
			TechnoMode(false), PrinterError(false), CutterOff(false), PaperEnd(false), OfflineError(false),
			Register(0) {}
	};

	/// Ответ.
	namespace Answer
	{
		namespace Identification
		{
			/// Названия модели - "MSTAR-TK".
			const QByteArray MStarTKModel  = "\x4D\x53\x54\x41\x52\x2D\x54\x4B";

			/// Название производителя - "Мультисофт Системз".
			const QByteArray MSoftVendor  = "\x8C\xE3\xAB\xEC\xE2\xA8\xE1\xAE\xE4\xE2\x20\x91\xA8\xE1\xE2\xA5\xAC\xA7";
		}

		/// Секции в сплит-посылке для разных ответов.
		namespace Sections
		{
			/// Команда.
			const int Command       = 0;

			/// Текущий статус ККМ.
			const int FRStatus      = 1;

			/// Результат выполнения команды.
			const int CommandResult = 2;

			/// Текущий статус принтера.
			const int PrinterStatus = 3;

			/// Наименование модели.
			const int ModelName     = 5;

			/// Наименование вендора.
			const int VendorName    = 6;

			/// Версия ПО.
			const int SoftVersion   = 7;

			/// Параметр ФР.
			const int ParameterFR   = 4;

			/// Статус ЭКЛЗ.
			const int StatusEKLZ    = 5;

			/// Денежный регистр ФР.
			const int Register      = 6;
		}

		/// Номера битов ошибок.
		namespace Errors
		{
			/// Статусы ККМ.
			const int FiscalMemoryNearEnd = 5;
			const int FiscalMemoryEnd = 6;
			const int EKLZNearEnd = 15;
			const int EKLZError = 7;

			/// Статусы принтера.
			const int TechnoMode   = 2;
			const int PrinterError = 3;
			const int CutterOff    = 4;
			const int PaperEnd     = 5;
			const int OfflineError = 7;
		}
	}
}

//--------------------------------------------------------------------------------
