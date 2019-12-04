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

	/// По умолчанию использовать последнюю регистрацию.
	const char LastRegistration[] = "00";

	/// Количество типов оплаты.
	const int PayTypeAmount = '\x0F';

	/// Данные для парсинга TLV-структур фискального чека.
	const char RegExpTLVData[] = "<([0-9]+)>(.*)";

	/// Формат представления даты-времени в теге 1012 при получении данных о тегах фискального документа.
	const char FFDateTimeFormat[] = "dd.MM.yyyy hh:mm";

	/// Версия ФН без возможности запроса флага агента.
	const char FSNoAgentFlags[] = "fn_v_1_0";

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

	/// Получить версию УПД по версии прошивки.
	inline int getDTD(const QString & aFirmware, EFFD::Enum aFFDFR)
	{
		int result = aFirmware.mid(1, 1).toInt(0, 16);

		if ((aFFDFR >= EFFD::F105) && (result < 6))
		{
			result += 16;
		}

		return result;
	}

	/// Получить актуальные версии прошивок для разных версий ФФД.
	inline double getActualFirmware(EFFD::Enum aFFD)
	{
		if (aFFD == EFFD::F10)  return  64.0;
		if (aFFD == EFFD::F105) return 107.3;

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

		class Data: public FRError::Data
		{
		public:
			Data()
			{
				using namespace FRError;

				/// Коды ошибок.
				add('\x01', "Неверный формат сообщения");
				add('\x02', "Неверный формат поля ", true);
				add('\x03', "Неверное дата /время. Невозможно установить переданные дату/время");
				add('\x04', "Неверная контрольная сумма (BCC)",                  EType::Retry);
				add('\x05', "Неверный пароль передачи данных. Пароль по умолчанию \"AERF\"");
				add('\x06', "Ошибка кода команды");
				add('\x07', "Необходима команда \"Начало сеанса\"");
				add('\x08', "Время изменилось больше чем на 24 часа",            EType::Retry);
				add('\x09', "Превышена максимальная длина строкового поля ", true);
				add('\x0A', "Превышена максимальная длина сообщения");
				add('\x0B', "Неправильная операция");
				add('\x0C', "Значение вне диапазона, поле ", true);
				add('\x0D', "При данном состоянии документа эта команда не допустима");
				add('\x0E', "Обязательное строковое поле имеет нулевую длину, поле ", true);
				add('\x0F', "Слишком большой результат");
				add('\x10', "Переполнение денежного счетчика, причина - ");
				add('\x11', "Обратная операция невозможна из-за отсутствия прямой");
				add('\x12', "Нет столько наличных для выполнения операции");
				add('\x13', "Обратная операция превысила итог по прямой операции");
				add('\x14', "Необходимо выполнить сертификацию (ввод заводского номера ККТ)");
				add('\x15', "Необходимо выполнить отчёт закрытия смены");
				add('\x16', "Таймаут при печати",                                EType::Printer);
				add('\x17', "Неисправимая ошибка принтера",                      EType::Printer);
				add('\x18', "Принтер не готов к печати",                         EType::Printer);
				add('\x19', "Бумага близка к концу",                             EType::Printer);
				add('\x1A', "Необходимо провести регистрацию");
				add('\x1C', "ККТ уже сертифицирована");
				add('\x1D', "Исчерпано число регистраций");
				add('\x1E', "Неверный буфер печати");
				add('\x1F', "Неверное G-поле ", true);
				add('\x20', "Неверный номер типа оплаты");
				add('\x21', "Таймаут приема",                                    EType::Printer);
				add('\x22', "Ошибка приема",                                     EType::Printer);
				add('\x23', "Неверное состояние ККТ");
				add('\x24', "Слишком много операций в документе. Необходима команда \"Аннулирование\"");
				add('\x25', "Необходима команда \"Открытие смены\"");
				add('\x26', "Необходима печать буфера контрольной ленты");
				add('\x27', "Неверный номер вида платежа");
				add('\x28', "Неверное состояние принтера",                       EType::Printer);
				add('\x29', "Смена уже открыта");
				add('\x2B', "Неверная дата");
				add('\x2C', "Нет места для добавления отдела/составляющей");
				add('\x2D', "Индекс отдела/составляющей уже существует");
				add('\x2E', "Невозможно удалить отдел, т.к. есть составляющие отдела");
				add('\x2F', "Индекс отдела/составляющей не обнаружен");
				add('\x32', "Необходима инициализация ФН");
				add('\x34', "Некорректный стартовый символ на приеме");
				add('\x50', "Неверное состояние РПКУ");
				add('\x51', "Требуется распечатка РПКУ");
				add('\x52', "Ошибка РПКУ");
				add('\x60', "Ошибка 2D-кода, причина - ");
				add('\x61', "Недопустимый тег ", true);
				add('\x62', "Отсутствует обязательный тег");
				add('\x63', "Индекс налога вне диапазона 0, 10, 18");
				add('\x64', "Неверное контрольное число регистрационного номера ККТ");
				add('\x65', "Ошибка формата тега, поле ");
				add('\x70', "Общая ошибка ФН",                                   EType::FS);
				add('\x71', "Неизвестная команда, неверный формат посылки или неизвестные параметры");
				add('\x72', "Неверное состояние ФН",                             EType::FS);
				add('\x73', "Ошибка ФН",                                         EType::FS);
				add('\x74', "Ошибка КС",                                         EType::FS);
				add('\x75', "Закончен срок эксплуатации ФН",                     EType::FS);
				add('\x76', "Архив ФН переполнен",                               EType::FS);
				add('\x77', "Неверные дата и/или время");
				add('\x78', "Нет запрошенных данных");
				add('\x79', "Некорректное значение параметров команды");
				add('\x80', "Превышение размеров TLV данных");
				add('\x81', "Нет транспортного соединения");
				add('\x82', "Исчерпан ресурс КС",                                EType::FS);
				add('\x84', "Исчерпан ресурс хранения документов для ОФД",       EType::FS);
				add('\x85', "Исчерпан ресурс ожидания передачи сообщения в ОФД", EType::FS);
				add('\x86', "Продолжительность смены более 24-х часов",          EType::FS);
				add('\x87', "Неверная разница во времени между 2 операциями",    EType::FS);
				add('\x90', "Сообщение от ОФД не может быть принято");
			}
		};

		class ExtraData : public CPrimFR::Errors::ExtraDataBase
		{
		public:
			ExtraData()
			{
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

			virtual QString value(char aErrorCode, char aErrorReason)
			{
				if (aErrorCode == Errors::Code2DErrors)         return mCode2DErrors[aErrorReason];
				if (aErrorCode == Errors::MoneyCounterOverflow) return mMoneyCounterOverflows[aErrorReason];

				return ExtraDataBase::value(aErrorCode, aErrorReason);
			}

		private:
			CDescription<char> mMoneyCounterOverflows;
			CDescription<char> mCode2DErrors;
		};
	}
}

//--------------------------------------------------------------------------------
