/* @file Константы, коды команд и ответов онлайн ФР семейства ПРИМ. */

#pragma once

#include "Hardware/FR/FRBaseConstants.h"
#include "../PrimFRConstants.h"

//--------------------------------------------------------------------------------
namespace CPrimOnlineFR
{
	/// Параметр снятия Z-отчета наружу.
	const char ZReportOut[] = "00";

	/// Параметр снятия Z-отчета в буфер.
	const char ZReportInBuffer[] = "01";

	/// Формат представления даты и времени в ответе на запрос статуса ФН-а.
	const char FSDateTimeFormat[] = "ddMMyyyyhhmm";

	/// По умолчанию использовать последнюю регистрацию.
	const char LastRegistration[] = "00";

	/// Количество типов оплаты.
	const int PayTypeAmount = '\x0F';

	/// Данные для парсинга TLV-структур фискального чека.
	const char RegExpTLVData[] = "<([0-9]+)>(.*)";

	/// Формат представления даты-времени в теге 1012 при получении данных о тегах фисклаьного документа.
	const char FFDateTimeFormat[] = "dd.MM.yyyy hh:mm";

	//----------------------------------------------------------------------------
	/// Настройки для ПФД.
	namespace AFD
	{
		/// Длина поля
		namespace LineSize
		{
			const int GField = 40;    /// Произвольное поле.
			const int Unit   = 39;    /// Названия товара.
		}
	}

	/// Получить версию ФФД по номеру билда прошивки.
	inline EFFD::Enum getFFD(double aBuild)
	{
		if (aBuild <  60) return EFFD::F10Beta;
		if (aBuild < 100) return EFFD::F10;
		if (aBuild < 200) return EFFD::F105;
		if (aBuild < 300) return EFFD::F11;

		return EFFD::Unknown;
	}

	/// Получить актуальные версии прошивок для разных версий ФФД.
	inline double getActualFirmware(EFFD::Enum aFFD)
	{
		if (aFFD == EFFD::F10)  return  64.0;
		if (aFFD == EFFD::F105) return 104.5;

		return 0;
	}

	/// Получить варианты поддержки кодов (Bar- и QR-).
	inline QString getCodes(const QString & aFirmware)
	{
		int digit = aFirmware.left(1).toInt();

		if (digit == 1) return "PFD417";
		if (digit == 2) return "QR";
		if (digit == 3) return "PFD417 and QR";

		return "";
	}

	//----------------------------------------------------------------------------
	/// Спецификация типов оплаты по тегам итогов типов оплаты.
	class CPayTypeData: public CSpecification<int, SDK::Driver::EPayTypes::Enum>
	{
	public:
		CPayTypeData()
		{
			append(CFR::FiscalFields::CashFiscalTotal,         SDK::Driver::EPayTypes::Cash);
			append(CFR::FiscalFields::CardFiscalTotal,         SDK::Driver::EPayTypes::EMoney);
			append(CFR::FiscalFields::PrePaymentFiscalTotal,   SDK::Driver::EPayTypes::PrePayment);
			append(CFR::FiscalFields::PostPaymentFiscalTotal,  SDK::Driver::EPayTypes::PostPayment);
			append(CFR::FiscalFields::CounterOfferFiscalTotal, SDK::Driver::EPayTypes::CounterOffer);
		}
	};

	static CPayTypeData PayTypeData;

	//----------------------------------------------------------------------------
	/// Команды.
	namespace Commands
	{
		const char GetFSStatus          = '\x29';    /// Получить статус ФН.
		const char GetOFDNotSentCount   = '\x39';    /// Получить количество неотправленных документов в ОФД.
		const char GetPayTypeData       = '\x4B';    /// Получить данные о виде платежа.
		const char GetOFDData           = '\x67';    /// Получить параметры обмена с ОФД.
		const char GetRegTLVData        = '\x88';    /// Получить TLV-параметр регистрации.
		const char GetFiscalTLVData     = '\x8B';    /// Получить TLV-параметры ФД.
		const char GetRegistrationTotal = '\x8F';    /// Получить итоги регистрации.
	}

	//----------------------------------------------------------------------------
	/// Флаги команды получения фискальных тегов.
	namespace FiscalTLVDataFlags
	{
		const int Start = 3;
		const int Get   = 4;
	}

	//----------------------------------------------------------------------------	
	/// Ошибки.
	namespace Errors
	{
		/// Коды ошибок.
		const char MoneyCounterOverflow = '\x10';    /// Переполнение денежного счетчика.
		const char Code2DErrors         = '\x60';    /// Ошибка 2D-кода.
		const char NoRequiedData        = '\x78';    /// Нет запрошенных данных.
		const char FSOfflineEnd         = '\x84';    /// Исчерпан ресурс хранения документов для ОФД.

		class Specification : public CPrimFR::Errors::SpecificationBase
		{
		public:
			Specification()
			{
				using namespace FRError;

				/// Коды ошибок.
				add('\x01', EType::Unknown, "Неверный формат сообщения");
				add('\x02', EType::Unknown, "Неверный формат поля ", true);
				add('\x03', EType::Unknown, "Неверное дата /время. Невозможно установить переданные дату/время");
				add('\x04', EType::Retry,   "Неверная контрольная сумма (BCC)");
				add('\x05', EType::Unknown, "Неверный пароль передачи данных. Пароль по умолчанию \"AERF\"");
				add('\x06', EType::Unknown, "Ошибка кода команды");
				add('\x07', EType::Unknown, "Необходима команда \"Начало сеанса\"");
				add('\x08', EType::Retry,   "Время изменилось больше чем на 24 часа");
				add('\x09', EType::Unknown, "Превышена максимальная длина строкового поля ", true);
				add('\x0A', EType::Unknown, "Превышена максимальная длина сообщения");
				add('\x0B', EType::Unknown, "Неправильная операция");
				add('\x0C', EType::Unknown, "Значение вне диапазона, поле ", true);
				add('\x0D', EType::Unknown, "При данном состоянии документа эта команда не допустима");
				add('\x0E', EType::Unknown, "Обязательное строковое поле имеет нулевую длину, поле ", true);
				add('\x0F', EType::FR,      "Слишком большой результат");
				add('\x10', EType::Unknown, "Переполнение денежного счетчика, причина - ");
				add('\x11', EType::Unknown, "Обратная операция невозможна из-за отсутствия прямой");
				add('\x12', EType::Unknown, "Нет столько наличных для выполнения операции");
				add('\x13', EType::Unknown, "Обратная операция превысила итог по прямой операции");
				add('\x14', EType::FR,      "Необходимо выполнить сертификацию (ввод заводского номера ККТ)");
				add('\x15', EType::Unknown, "Необходимо выполнить отчёт закрытия смены");
				add('\x16', EType::Printer, "Таймаут при печати");
				add('\x17', EType::Printer, "Неисправимая ошибка принтера");
				add('\x18', EType::Printer, "Принтер не готов к печати");
				add('\x19', EType::Printer, "Бумага близка к концу");
				add('\x1A', EType::Unknown, "Необходимо провести регистрацию");
				add('\x1C', EType::FR,      "ККТ уже сертифицирована");
				add('\x1D', EType::FR,      "Исчерпано число регистраций");
				add('\x1E', EType::Unknown, "Неверный буфер печати");
				add('\x1F', EType::Unknown, "Неверное G-поле ", true);
				add('\x20', EType::Unknown, "Неверный номер типа оплаты");
				add('\x21', EType::Printer, "Таймаут приема");
				add('\x22', EType::Printer, "Ошибка приема");
				add('\x23', EType::FR,      "Неверное состояние ККТ");
				add('\x24', EType::Unknown, "Слишком много операций в документе. Необходима команда \"Аннулирование\"");
				add('\x25', EType::Unknown, "Необходима команда \"Открытие смены\"");
				add('\x26', EType::Unknown, "Необходима печать буфера контрольной ленты");
				add('\x27', EType::Unknown, "Неверный номер вида платежа");
				add('\x28', EType::Printer, "Неверное состояние принтера");
				add('\x29', EType::Unknown, "Смена уже открыта");
				add('\x2B', EType::Unknown, "Неверная дата");
				add('\x2C', EType::Unknown, "Нет места для добавления отдела/составляющей");
				add('\x2D', EType::Unknown, "Индекс отдела/составляющей уже существует");
				add('\x2E', EType::Unknown, "Невозможно удалить отдел, т.к. есть составляющие отдела");
				add('\x2F', EType::Unknown, "Индекс отдела/составляющей не обнаружен");
				add('\x32', EType::FR,      "Необходима инициализация ФН");
				add('\x34', EType::FR,      "Некорректный стартовый символ на приеме");
				add('\x50', EType::FR,      "Неверное состояние РПКУ");
				add('\x51', EType::Unknown, "Требуется распечатка РПКУ");
				add('\x52', EType::Unknown, "Ошибка РПКУ");
				add('\x60', EType::Unknown, "Ошибка 2D-кода, причина - ");
				add('\x61', EType::FR,      "Недопустимый тег ", true);
				add('\x62', EType::FR,      "Отсутствует обязательный тег");
				add('\x63', EType::FR,      "Индекс налога вне диапазона 0, 10, 18");
				add('\x64', EType::FR,      "Неверное контрольное число регистрационного номера ККТ");
				add('\x65', EType::FR,      "Ошибка формата тега, поле ");
				add('\x70', EType::FS,      "Общая ошибка ФН");
				add('\x71', EType::FR,      "Неизвестная команда, неверный формат посылки или неизвестные параметры");
				add('\x72', EType::FS,      "Неверное состояние ФН");
				add('\x73', EType::FS,      "Ошибка ФН");
				add('\x74', EType::FS,      "Ошибка КС");
				add('\x75', EType::FS,      "Закончен срок эксплуатации ФН");
				add('\x76', EType::FS,      "Архив ФН переполнен");
				add('\x77', EType::FR,      "Неверные дата и/или время");
				add('\x78', EType::FR,      "Нет запрошенных данных");
				add('\x79', EType::FR,      "Некорректное значение параметров команды");
				add('\x80', EType::FR,      "Превышение размеров TLV данных");
				add('\x81', EType::FR,      "Нет транспортного соединения");
				add('\x82', EType::FS,      "Исчерпан ресурс КС");
				add('\x84', EType::FS,      "Исчерпан ресурс хранения документов для ОФД");
				add('\x85', EType::FS,      "Исчерпан ресурс ожидания передачи сообщения в ОФД");
				add('\x86', EType::FS,      "Продолжительность смены более 24-х часов");
				add('\x87', EType::FS,      "Неверная разница во времени между 2 операциями");
				add('\x90', EType::FR,      "Сообщение от ОФД не может быть принято");

				/// Причины переполнения денежного счетчика.
				mMoneyCounterOverflows.append('\x01', "Сумма составляющих не равна общей сумме");
				mMoneyCounterOverflows.append('\x02', "Сумма по видам оплат не равна общей сумме");
				mMoneyCounterOverflows.append('\xA0', "Ошибка умножения");
				mMoneyCounterOverflows.append('\xA1', "Ошибка деления");
				mMoneyCounterOverflows.append('\xFA', "Переполнение для процента скидки/наценки (превышает 999.99%)");
				mMoneyCounterOverflows.append('\xFB', "Переполнение для счетчиков накопления (нарастающий итог)");
				mMoneyCounterOverflows.append('\xFC', "Переполнение для суммы наличных в кассе");
				mMoneyCounterOverflows.append('\xFD', "Переполнение для дневного денежного счетчика по операциям");
				mMoneyCounterOverflows.append('\xFE', "Переполнение для итоговой суммы документа");
				mMoneyCounterOverflows.append('\xFF', "Переполнение для суммы операции");
				mMoneyCounterOverflows.setDefault("Неизвестна");

				/// Причины некорректного состояния ЭКЛЗ.
				mCode2DErrors.append('\x01', "Длина поля вне диапазона");
				mCode2DErrors.append('\x02', "Неверные параметры построения");
				mCode2DErrors.append('\x03', "Аппаратная ошибка модуля QR");
				mCode2DErrors.append('\x0A', "2D-код (PDF417 или QR) не создан");
				mCode2DErrors.append('\x0B', "Неверные параметры для печати (размеры PDF417 или QR-кода больше области печати)");
				mCode2DErrors.setDefault("Неизвестна");
			}

			virtual QString getDescription(ushort aFullErrorCode)
			{
				char errorCode   = char(aFullErrorCode);
				char errorReason = char(aFullErrorCode >> 8);

				QString result = CPrimFR::Errors::SpecificationBase::operator[](errorCode);

				if (errorCode == Errors::MoneyCounterOverflow)
				{
					result += mCode2DErrors[errorReason];
				}
				else if (errorCode == Errors::Code2DErrors)
				{
					result += mMoneyCounterOverflows[errorReason];
				}
				else if (mBuffer[errorCode].extraData)
				{
					result += QString::number(uchar(errorReason));
				}

				return result;
			}

		protected:
			CDescription<char> mMoneyCounterOverflows;
			CDescription<char> mCode2DErrors;
		};
	}
}

//--------------------------------------------------------------------------------
