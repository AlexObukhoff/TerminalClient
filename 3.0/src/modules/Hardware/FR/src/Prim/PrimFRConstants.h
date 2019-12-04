/* @file Константы, коды команд и ответов протокола ФР ПРИМ. */

#pragma once

// Modules
#include "Hardware/Common/BaseStatusTypes.h"
#include "Hardware/Common/ASCII.h"
#include "Hardware/Printers/Tags.h"

// Project
#include "Hardware/FR/FRStatusCodes.h"
#include "PrimFRDataTypes.h"

//--------------------------------------------------------------------------------
/// Константы, команды и коды состояний устройств на протоколе PRIM.
namespace CPrimFR
{
	/// Байт-разделитель.
	const char Separator = '\x1C';

	/// Разделитель для строковых данных.
	const char StringSeparatorByte = '|';

	/// Минимальный размер пакета для identification.
	const int MinIdentSize = 109;

	/// Формат представления даты и времени в ответе на запрос статуса ФН-а.
	const char FRDateTimeFormat[] = "ddMMyyyyhhmm";

	/// Данные типов фискальных документов.
	class CPayOffTypeData: public CSpecification<SDK::Driver::EPayOffTypes::Enum, QByteArray>
	{
	public:
		CPayOffTypeData()
		{
			using namespace SDK::Driver;

			append(EPayOffTypes::Debit,      "00");
			append(EPayOffTypes::DebitBack,  "02");
			append(EPayOffTypes::Credit,     "04");
			append(EPayOffTypes::CreditBack, "05");
		}
	};

	static CPayOffTypeData PayOffTypeData;

	/// Количество чеков.
	const char ChecksQuantity[] = "01";

	/// Дата и время в команде.
	const bool DateTimeInCommand = true;

	/// ID прошивки для ПРИМ-21 03 на базе принтера Custom VKP-80.
	const char FirmarePRIM21_03[] = "LPC22";

	/// ПФД.

	/// Артикул товара/услуги.
	const char ServiceMarking[] = "1";

	/// Количество наименований товара/услуги.
	const char ServiceQuantity[] = "1";

	/// Единица измерения товара/услуги.
	const char MeasurementUnit[] = " ";

	/// Индекс секции.
	const char SectionIndex[] = "01";

	/// Идентификатор секции.
	const char SectionID[] = "TERMINAL";

	/// Идентификатор (фамилия) оператора.
	const char OperatorID[] = " ";

	/// Шрифт.
	const char Font[] = "00";

	/// Количество копий.
	const char Copies[] = "01";

	/// Шаг сетки копий
	const char CopуGrid[] = "0000";

	/// Шаг сетки строк
	const char LineGrid[] = "00";

	/// Название платежной карты. платежных карт у нас нет, но в посылку надо что-то положить.
	const char PaymentCardName[] = " ";

	/// Не печатать документ.
	const char DontPrintFD[] = "00";

	/// Налоговая ставка куда будет складываться последний снятый Z отчет - последняя, неиспользуемая.
	const int LastTaxRate = 7;

	/// Название налоговой ставки куда будет складываться последний снятый Z отчет.
	const char LastTaxRateName[] = "Z-report number";

	/// Маска печати шапки чека перед ПФД.
	const ushort NeedPrintFiscalCapMask = 0x8000;

	/// Параметр 1 настройки ФР.
	const ushort Parameter1 = 0xF701;

	/// Маска для параметрв онлайновых ФР.
	const ushort Parameter1Mask = 0xFF00;

	/// Параметр 2 настройки ФР.
	const ushort Parameter2 = 0x14A0;

	/// Маска для длинных отчетов.
	const ushort LongReportMask2 = 0x0005;

	/// Маска для обнуления суммы в кассе при закрытии смены (параметр 2).
	const ushort NullingSumInCashMask = 0x0100;

	/// Версия ПО ФР, где присутствуют презентер и ретрактор.
	const char SoftVersionPresenter[] = "LPC22";

	/// Максимальный размер произвольного G поля в ПФД.
	const int MaxLengthGField = 80;

	/// Маски открытой сессии для парсинга текущего статуса ККМ.
	const ushort SessionOpened = 0x0800;

	/// Маски истекшей сессии для парсинга текущего статуса ККМ.
	const ushort SessionExpired = 0x0010;

	/// Маска состояния документа для парсинга текущего статуса ККМ.
	const ushort DocumentMask = 0x0007;

	/// Таймаут входа в фискальный режим, [мс]
	const int SetFiscalModeTimeout = 3 * 1000;

	/// Текст на чеке перед выходом из незапланированного режима принтера
	const char EndPrinterModeText[] = "           PRINTER MODE";

	//----------------------------------------------------------------------------
	/// Шрифт для фискальных документов.
	namespace FiscalFont
	{
		const int Default = 1;
		const int Narrow  = 0;
	}

	//----------------------------------------------------------------------------
	/// Паузы, [мс].
	namespace Pause
	{
		/// Между печатью строк.
		const int LinePrinting = 100;

		/// После команды программирования.
		const int Programming =  1000;
	}

	class CommandTimouts: public CSpecification<char, int>
	{
	public:
		CommandTimouts()
		{
			setDefault(1000);
		}
	};

	//----------------------------------------------------------------------------
	/// Теги.
	class TagEngine : public Tags::Engine
	{
	public:
		TagEngine()
		{
			QByteArray prefix("\x1B\x21");

			appendCommon(Tags::Type::Bold,         prefix, "\x08");
			appendCommon(Tags::Type::DoubleWidth,  prefix, "\x20");
			appendCommon(Tags::Type::DoubleHeight, prefix, "\x10");
			appendCommon(Tags::Type::UnderLine,    prefix, "\x80");
		}
	};

	//----------------------------------------------------------------------------	
	/// Команды.
	namespace Commands
	{
		const char OpenSession           = '\x01';
		const char OpenFRSession         = '\x02';
		const char GetStatus             = '\x03';
		const char CancelDocument        = '\x17';
		const char XReport               = '\x30';
		const char ZReport               = '\x31';
		const char Encashment            = '\x33';
		const char EReport               = '\x34';
		const char EDocument             = '\x3E';
		const char GetLastCVCNumber      = '\x3F';
		const char GetDateTime           = '\x43';
		const char SetMoneyBoxSettings   = '\x44';
		const char GetMoneyBoxSettings   = '\x45';
		const char SetFDTypeNames        = '\x47';
		const char ClearZBuffer          = '\x49';
		const char SetFRParameters       = '\x4C';
		const char GetFRParameters       = '\x4D';
		const char SetTaxRate            = '\x58';
		const char GetTaxRate            = '\x59';
		const char SetEjectorAction      = '\x6F';
		const char SetPrinterMode        = '\x70';
		const char AFD                   = '\x73';
		const char PrintDeferredZReports = '\x7C';
		const char FRControl             = '\x94';
		const char GetFRControlSettings  = '\x95';
		const char GetKKMInfo            = '\x97';

		const char SetFiscalMode[] = "\x1B\x1B";

		/// Дата и время в команде.
		const QSet<char> DateTimeIn = QSet<char>()
			<< OpenSession
			<< OpenFRSession
			<< GetStatus
			<< CancelDocument
			<< XReport
			<< ZReport
			<< Encashment
			<< EReport;
	}

	/// Максимальное количество повторов.
	namespace MaxRepeat
	{
		const int RetractorError = 3;
	}

	//----------------------------------------------------------------------------
	/// Спецификация статусов.
	class CStatusInfo: public CSpecification<int, int>
	{
	public:
		CStatusInfo()
		{
			append(0, DeviceStatusCode::Error::Unknown);
			append(1, FRStatusCode::Error::FM);
			append(2, FRStatusCode::Error::EKLZ);
			append(4, FRStatusCode::Warning::EKLZNearEnd);
			append(5, FRStatusCode::Error::EKLZ);
			append(6, FRStatusCode::Error::EKLZ);
		}
	};

	static CStatusInfo StatusInfo;

	//----------------------------------------------------------------------------	
	/// Ошибки выполнения команд.
	namespace Errors
	{
		/// Коды ошибок.
		const char NeedBeginSession       = '\x07';    /// Необходима команда начало сеанса.
		const char NeedBeginFRSession     = '\x25';    /// Необходима команда "Открытие смены"
		const char FieldOutOfRange        = '\x0C';    /// Значение поля вне диапозона.
		const char InvalidStateForCommand = '\x0D';    /// При данном состоянии документа эта команда недопустима.
		const char MoneyCounterOverflow   = '\x10';    /// Переполнение денежного счетчика.
		const char NeedZReport            = '\x15';    /// Необходимо выполнить Z-отчет.
		const char EKLZIncorrectState     = '\x37';    /// Неверное состояние ЭКЛЗ.
		const char IncorrigibleError      = '\x17';    /// Неисправимая ошибка принтера.
		const char NotReadyForPrint       = '\x18';    /// Принтер не готов к печати.

		class Data: public FRError::Data
		{
		public:
			Data()
			{
				/// Коды ошибок.
				add('\x01', "Неверный формат сообщения");
				add('\x02', "Неверный формат поля ", true);
				add('\x03', "Неверное дата/время. Невозможно установить переданные дату/время");
				add('\x04', "Неверная контрольная сумма");
				add('\x05', "Неверный пароль передачи данных");
				add('\x06', "Нет команды с таким номером");
				add('\x07', "Необходима команда \"Начало сеанса\"");
				add('\x08', "Время изменилось больше чем на часа");
				add('\x09', "Превышена максимальная длина строкового поля ", true);
				add('\x0A', "Превышена максимальная длина сообщения");
				add('\x0B', "Неправильная операция");
				add('\x0C', "Значение вне диапазона, поле ", true);
				add('\x0D', "При данном состоянии документа эта команда не допустима");
				add('\x0E', "Нулевая длина обязательного строкового поля ", true);
				add('\x0F', "Слишком большой результат");
				add('\x10', "Переполнение денежного счетчика. Причина - ", true);
				add('\x11', "Обратная операция невозможна из-за отсутствия прямой");
				add('\x12', "Нет столько наличных для выполнения операции");
				add('\x13', "Обратная операция превысила итог по прямой операции");
				add('\x14', "Необходимо выполнить сертификацию (ввод заводского номера)");
				add('\x15', "Необходимо выполнить Z отчёт");
				add('\x16', "Таймаут при печати");
				add('\x17', "Не исправимая ошибка принтера");
				add('\x18', "Принтер не готов к печати");
				add('\x19', "Бумага близка к концу");
				add('\x1A', "Необходимо провести фискализацию");
				add('\x1B', "Неверный пароль доступа к ФП. Необходимо выполнить команду, например, \"Фискальный отчет\", введя правильный пароль");
				add('\x1C', "ККМ уже сертифицирована");
				add('\x1D', "Исчерпано число фискализаций");
				add('\x1E', "Неверный буфер печати");
				add('\x1F', "Неверное G-поле ", true);
				add('\x20', "Неверный номер типа оплаты");
				add('\x21', "Таймаут приема");
				add('\x22', "Ошибка приема");
				add('\x23', "Неверное состояние ККМ");
				add('\x24', "Слишком много операций в документе. Необходима команда \"Аннулировать\"");
				add('\x25', "Необходима команда \"Открытие смены\"");
				add('\x26', "Необходима печать буфера контрольной ленты");
				add('\x27', "Неверный номер вида платежа");
				add('\x28', "Неверное состояние принтера");
				add('\x29', "Смена уже открыта");
				add('\x2B', "Неверная дата");
				add('\x2C', "Нет места для добавления отдела/составляющей (суммарное их число > 400)");
				add('\x2D', "Индекс отдела/составляющей уже существует");
				add('\x2E', "Невозможно удалить отдела - есть составляющие");
				add('\x2F', "Индекс отдела/составляющей не обнаружен");
				add('\x30', "Фискальная память неисправна");
				add('\x31', "Дата последней существующей записи в фискальной памяти позже, чем дата операции, которую пытались выполнить");
				add('\x32', "Необходима инициализация фискальной памяти");
				add('\x33', "Заполнена вся фискальная память. Блокируются все команды, кроме снятия фискальных отчетов и формирования нефискальных документов");
				add('\x34', "Некорректный стартовый символ на приеме");
				add('\x35', "Неопознанный ответ от ЭКЛЗ");
				add('\x36', "Неизвестная команда ЭКЛЗ (из ЭКЛЗ)");
				add('\x37', "Неверное состояние ЭКЛЗ. Причина - ", true);
				add('\x38', "Таймаут приема от ЭКЛЗ");
				add('\x39', "Таймаут передачи в ЭКЛЗ");
				add('\x3A', "Неверная контрольная сумма ответа ЭКЛЗ");
				add('\x3B', "Аварийное состояние ЭКЛЗ (из ЭКЛЗ)");
				add('\x3C', "Нет свободного места в ЭКЛЗ (из ЭКЛЗ)");
				add('\x3D', "Неверная контрольная сумма в команде ЭКЛЗ");
				add('\x3E', "Контроллер ЭКЛЗ не обнаружен");
				add('\x3F', "Данные в ЭКЛЗ отсутствуют");
				add('\x40', "Данные в ЭКЛЗ не синхронизированы");
				add('\x41', "Аварийное состояние РИК (из ЭКЛЗ)");
				add('\x42', "Неверные дата и время в команде ЭКЛЗ (из ЭКЛЗ)");
				add('\x43', "Закончилось время эксплуатации ЭКЛЗ (из ЭКЛЗ)");
				add('\x44', "Переполнение ЭКЛЗ (из ЭКЛЗ)");
				add('\x45', "Число активизаций исчерпано");
				add('\x51', "Требуется распечатка СКЛ");
				add('\x52', "Аварийное состояние СКЛ");
				add('\x95', "Ошибка при формировании строки для печати, номер процесса ", true);
			}
		};

		class ExtraData : public ExtraDataBase
		{
		public:
			ExtraData()
			{
				/// Причины переполнения денежного счетчика.
				mMoneyCounterOverflows.append('\x01', "Сумма составляющих не равна общей сумме");
				mMoneyCounterOverflows.append('\x02', "Сумма по видам оплат не равна общей сумме");
				mMoneyCounterOverflows.append('\xA0', "Ошибка умножения");
				mMoneyCounterOverflows.append('\xA1', "Ошибка деления");
				mMoneyCounterOverflows.append('\xFA', "Переполнение для процента");
				mMoneyCounterOverflows.append('\xFB', "Переполнение для счетчиков накопления");
				mMoneyCounterOverflows.append('\xFC', "Переполнение для суммы в кассе");
				mMoneyCounterOverflows.append('\xFD', "Переполнение для дневного счетчика");
				mMoneyCounterOverflows.append('\xFE', "Переполнение для суммы документа");
				mMoneyCounterOverflows.append('\xFF', "Переполнение для суммы операции");
				mMoneyCounterOverflows.setDefault("Неизвестна");

				/// Причины некорректного состояния ЭКЛЗ.
				mEKLZIncorrectStateReason.append('\x00', "Требуется активизация ЭКЛЗ или ответ 02 из ЭКЛЗ");
				mEKLZIncorrectStateReason.append('\x01', "Попытка фискализации при активизированной ЭКЛЗ");
				mEKLZIncorrectStateReason.append('\x09', "Повторная активизация активизированной ЭКЛЗ");
				mEKLZIncorrectStateReason.append('\x0A', "Не опознан номер ЭКЛЗ");
				mEKLZIncorrectStateReason.append('\x40', "Ошибка, связанная с ЭКЛЗ (подробности в трассовой информации)");
				mEKLZIncorrectStateReason.setDefault("Неизвестна");
			}

			virtual QString value(char aErrorCode, char aErrorReason)
			{
				     if (aErrorCode == Errors::EKLZIncorrectState)   return mEKLZIncorrectStateReason[aErrorReason];
				else if (aErrorCode == Errors::MoneyCounterOverflow) return mMoneyCounterOverflows[aErrorReason];

				return ExtraDataBase::value(aErrorCode, aErrorReason);
			}

		private:
			CDescription<char> mMoneyCounterOverflows;
			CDescription<char> mEKLZIncorrectStateReason;
		};
	}
}

//--------------------------------------------------------------------------------
