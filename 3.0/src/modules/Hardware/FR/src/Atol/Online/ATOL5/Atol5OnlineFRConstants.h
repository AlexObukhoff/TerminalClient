/* @file Константы, коды команд и ответов онлайн ФР на платформе АТОЛ5. */

#pragma once

// SDK
#include <SDK/Drivers/FR/FiscalDataTypes.h>

// Project
#include "../AtolOnlineFRConstants.h"
#include "Atol5DataTypes.h"

//--------------------------------------------------------------------------------
namespace CAtol5OnlineFR
{
	/// Имена секций.
	inline SDK::Driver::TSectionNames getSectionNames() { SDK::Driver::TSectionNames result;
		for (int i = 1; i <= 5; ++i) result.insert(i, QString::fromUtf8("Секция №") + QString::number(i)); return result; }

	/// Параметры ФР
	namespace FRParameters
	{
		const int NullingSumInCash             =   4;    /// Обнулять сумму в кассе при закрытии смены.
		const int LineSpacing                  =  12;    /// Межстрочный интервал.
		const int PrintSectionNumber           =  16;    /// Печатать номер секции.
		const int BeepOnError                  =  37;    /// Звуковой сигнал при ошибке.
		const int BeepOnPowerOn                =  39;    /// Звуковой сигнал при включении.
		const int DefaultTaxSystem             =  50;    /// СНО по умолчанию.
		const int PrintPayOffSubjectMethodType =  57;    /// Печатать признак способа расчета (1214).
		const int PrintPayOffSubjectType       =  58;    /// Печатать признак предмета расчета (1214).
		const int OFDAddress                   = 273;    /// Адрес ОФД.
		const int OFDPort                      = 274;    /// Порт ОФД.
		const int EnableCutter                 = 300;    /// Включить отрезчик.
		const int FullCutting                  = 322;    /// Полная отрезка.
	}

	//--------------------------------------------------------------------------------
	/// Налоги.
	class CTaxData : public CSpecification<SDK::Driver::TVAT, libfptr_tax_type>
	{
	public:
		CTaxData()
		{
			using namespace SDK::Driver;

			append(18, LIBFPTR_TAX_VAT18);
			append(10, LIBFPTR_TAX_VAT10);
			//append(0, LIBFPTR_TAX_VAT0);
			append(0, LIBFPTR_TAX_NO);
			append(20, LIBFPTR_TAX_VAT20);
		}
	};

	static CTaxData TaxData;

	//--------------------------------------------------------------------------------
	/// Признаки расчета (1054).
	class CPayOffTypeData : public CSpecification<SDK::Driver::EPayOffTypes::Enum, libfptr_receipt_type>
	{
	public:
		CPayOffTypeData()
		{
			using namespace SDK::Driver;

			append(EPayOffTypes::Debit,      LIBFPTR_RT_SELL);
			append(EPayOffTypes::DebitBack,  LIBFPTR_RT_SELL_RETURN);
			append(EPayOffTypes::Credit,     LIBFPTR_RT_BUY);
			append(EPayOffTypes::CreditBack, LIBFPTR_RT_BUY_RETURN);
		}
	};

	static CPayOffTypeData PayOffTypeData;

	//--------------------------------------------------------------------------------
	/// Типы оплаты.
	class CPayTypeData : public CSpecification<SDK::Driver::EPayTypes::Enum, libfptr_payment_type>
	{
	public:
		CPayTypeData()
		{
			using namespace SDK::Driver;

			append(EPayTypes::Cash,         LIBFPTR_PT_CASH);
			append(EPayTypes::EMoney,       LIBFPTR_PT_ELECTRONICALLY);
			append(EPayTypes::PrePayment,   LIBFPTR_PT_PREPAID);
			append(EPayTypes::PostPayment,  LIBFPTR_PT_CREDIT);
			append(EPayTypes::CounterOffer, LIBFPTR_PT_OTHER);
		}
	};

	static CPayTypeData PayTypeData;

	/// Коды ошибок (некоторых).
	namespace Errors
	{
		class Data : public FRError::Data<ushort>
		{
		public:
			Data()
			{
				//using namespace FRError;

				add(LIBFPTR_ERROR_CONNECTION_DISABLED, "Соединение не установлено");
				add(LIBFPTR_ERROR_NO_CONNECTION, "Нет связи");
				add(LIBFPTR_ERROR_PORT_BUSY, "Порт занят");
				add(LIBFPTR_ERROR_PORT_NOT_AVAILABLE, "Порт недоступен");
				add(LIBFPTR_ERROR_INCORRECT_DATA, "Некорректные данные от устройства");
				add(LIBFPTR_ERROR_INTERNAL, "Внутренняя ошибка библиотеки");
				add(LIBFPTR_ERROR_UNSUPPORTED_CAST, "Неподдерживаемое преобразование типа параметра");
				add(LIBFPTR_ERROR_NO_REQUIRED_PARAM, "Не найден обязательный параметр");
				add(LIBFPTR_ERROR_INVALID_SETTINGS, "Некорректные настройки");
				add(LIBFPTR_ERROR_NOT_CONFIGURED, "Драйвер не настроен");
				add(LIBFPTR_ERROR_NOT_SUPPORTED, "Не поддерживается в данной версии (драйвера или ККТ)");
				add(LIBFPTR_ERROR_INVALID_MODE, "Не поддерживается в данном режиме");
				add(LIBFPTR_ERROR_INVALID_PARAM, "Нeкорректное значение параметра");
				add(LIBFPTR_ERROR_NOT_LOADED, "Не удалось загрузить библиотеку");
				add(LIBFPTR_ERROR_UNKNOWN, "Неизвестная ошибка");
				add(LIBFPTR_ERROR_INVALID_SUM, "Неверная цена (сумма)");
				add(LIBFPTR_ERROR_INVALID_QUANTITY, "Неверное количество");
				add(LIBFPTR_ERROR_CASH_COUNTER_OVERFLOW, "Переполнение счетчика наличности");
				add(LIBFPTR_ERROR_LAST_OPERATION_STORNO_DENIED, "Невозможно сторно последней операции");
				add(LIBFPTR_ERROR_STORNO_BY_CODE_DENIED, "Сторно по коду невозможно");
				add(LIBFPTR_ERROR_LAST_OPERATION_NOT_REPEATABLE, "Невозможен повтор последней операции");
				add(LIBFPTR_ERROR_DISCOUNT_NOT_REPEATABLE, "Повторная скидка на операцию невозможна");
				add(LIBFPTR_ERROR_DISCOUNT_DENIED, "Невозможно начислить скидку/надбавку");
				add(LIBFPTR_ERROR_INVALID_COMMODITY_CODE, "Неверный код товара");
				add(LIBFPTR_ERROR_INVALID_COMMODITY_BARCODE, "Неверный штрихкод товара");
				add(LIBFPTR_ERROR_INVALID_COMMAND_FORMAT, "Неверный формат команды");
				add(LIBFPTR_ERROR_INVALID_COMMAND_LENGTH, "Неверная длина");
				add(LIBFPTR_ERROR_BLOCKED_IN_DATE_INPUT_MODE, "ККТ заблокирована в режиме ввода даты");
				add(LIBFPTR_ERROR_NEED_DATE_ACCEPT, "Требуется подтверждение ввода даты");
				add(LIBFPTR_ERROR_NO_MORE_DATA, "Нет больше данных");
				add(LIBFPTR_ERROR_NO_ACCEPT_OR_CANCEL, "Нет подтверждения или отмены продажи");
				add(LIBFPTR_ERROR_BLOCKED_BY_REPORT_INTERRUPTION, "Отчет о закрытии смены прерван");
				add(LIBFPTR_ERROR_DISABLE_CASH_CONTROL_DENIED, "Отключение контроля наличности невозможно (не настроены необходимые типы оплаты)");
				add(LIBFPTR_ERROR_MODE_BLOCKED, "Вход в режим заблокирован");
				add(LIBFPTR_ERROR_CHECK_DATE_TIME, "Проверьте дату и время");
				add(LIBFPTR_ERROR_DATE_TIME_LESS_THAN_FS, "Переданные дата/время меньше даты/времени последнего фискального документа");
				add(LIBFPTR_ERROR_CLOSE_ARCHIVE_DENIED, "Невозможно закрыть архив");
				add(LIBFPTR_ERROR_COMMODITY_NOT_FOUND, "Товар не найден");
				add(LIBFPTR_ERROR_WEIGHT_BARCODE_WITH_INVALID_QUANTITY, "Весовой штрихкод с количеством <> 1.000");
				add(LIBFPTR_ERROR_RECEIPT_BUFFER_OVERFLOW, "Переполнение буфера чека");
				add(LIBFPTR_ERROR_QUANTITY_TOO_FEW, "Недостаточное количество товара");
				add(LIBFPTR_ERROR_STORNO_TOO_MUCH, "Сторнируемое количество больше проданного");
				add(LIBFPTR_ERROR_BLOCKED_COMMODITY_NOT_FOUND, "Товар не найден");
				add(LIBFPTR_ERROR_NO_PAPER, "Нет бумаги");
				add(LIBFPTR_ERROR_COVER_OPENED, "Открыта крышка");
				add(LIBFPTR_ERROR_PRINTER_FAULT, "Нет связи с принтером чеков");
				add(LIBFPTR_ERROR_MECHANICAL_FAULT, "Механическая ошибка печатающего устройства");
				add(LIBFPTR_ERROR_INVALID_RECEIPT_TYPE, "Неверный тип чека");
				add(LIBFPTR_ERROR_INVALID_UNIT_TYPE, "Недопустимое целевое устройство");
				add(LIBFPTR_ERROR_NO_MEMORY, "Нет места в массиве картинок/штрихкодов");
				add(LIBFPTR_ERROR_PICTURE_NOT_FOUND, "Неверный номер картинки/штрихкода (картинка/штрихкод отсутствует)");
				add(LIBFPTR_ERROR_NONCACH_PAYMENTS_TOO_MUCH, "Сумма не наличных платежей превышает сумму чека");
				add(LIBFPTR_ERROR_RETURN_DENIED, "Накопление меньше суммы возврата или аннулирования");
				add(LIBFPTR_ERROR_PAYMENTS_OVERFLOW, "Переполнение суммы платежей");
				add(LIBFPTR_ERROR_BUSY, "Предыдущая операция незавершена");
				add(LIBFPTR_ERROR_GSM, "Ошибка GSM-модуля");
				add(LIBFPTR_ERROR_INVALID_DISCOUNT, "Неверная величина скидки / надбавки");
				add(LIBFPTR_ERROR_OPERATION_AFTER_DISCOUNT_DENIED, "Операция после скидки / надбавки невозможна");
				add(LIBFPTR_ERROR_INVALID_DEPARTMENT, "Неверная секция");
				add(LIBFPTR_ERROR_INVALID_PAYMENT_TYPE, "Неверный вид оплаты");
				add(LIBFPTR_ERROR_MULTIPLICATION_OVERFLOW, "Переполнение при умножении");
				add(LIBFPTR_ERROR_DENIED_BY_SETTINGS, "Операция запрещена в таблице настроек");
				add(LIBFPTR_ERROR_TOTAL_OVERFLOW, "Переполнение итога чека");
				add(LIBFPTR_ERROR_DENIED_IN_ANNULATION_RECEIPT, "Открыт чек аннулирования – операция невозможна");
				add(LIBFPTR_ERROR_JOURNAL_OVERFLOW, "Переполнение буфера ЭЖ");
				add(LIBFPTR_ERROR_NOT_FULLY_PAID, "Чек оплачен не полностью");
				add(LIBFPTR_ERROR_DENIED_IN_RETURN_RECEIPT, "Открыт чек возврата – операция невозможна");
				add(LIBFPTR_ERROR_SHIFT_EXPIRED, "Смена превысила 24 часа");
				add(LIBFPTR_ERROR_DENIED_IN_SELL_RECEIPT, "Открыт чек продажи – операция невозможна");
				add(LIBFPTR_ERROR_FISCAL_MEMORY_OVERFLOW, "Переполнение ФП");
				add(LIBFPTR_ERROR_INVALID_PASSWORD, "Неверный пароль");
				add(LIBFPTR_ERROR_JOURNAL_BUSY, "Идет обработка ЭЖ");
				add(LIBFPTR_ERROR_DENIED_IN_CLOSED_SHIFT, "Смена закрыта - операция невозможна");
				add(LIBFPTR_ERROR_INVALID_TABLE_NUMBER, "Неверный номер таблицы");
				add(LIBFPTR_ERROR_INVALID_ROW_NUMBER, "Неверный номер ряда");
				add(LIBFPTR_ERROR_INVALID_FIELD_NUMBER, "Неверный номер поля");
				add(LIBFPTR_ERROR_INVALID_DATE_TIME, "Неверная дата и/или время");
				add(LIBFPTR_ERROR_INVALID_STORNO_SUM, "Неверная сумма сторно");
				add(LIBFPTR_ERROR_CHANGE_CALCULATION, "Подсчет суммы сдачи невозможен");
				add(LIBFPTR_ERROR_NO_CASH, "В ККТ нет денег для выплаты");
				add(LIBFPTR_ERROR_DENIED_IN_CLOSED_RECEIPT, "Чек закрыт – операция невозможна");
				add(LIBFPTR_ERROR_DENIED_IN_OPENED_RECEIPT, "Чек открыт – операция невозможна");
				add(LIBFPTR_ERROR_DENIED_IN_OPENED_SHIFT, "Смена открыта, операция невозможна");
				add(LIBFPTR_ERROR_SERIAL_NUMBER_ALREADY_ENTERED, "Серийный номер / MAC-адрес уже задан");
				add(LIBFPTR_ERROR_TOO_MUCH_REREGISTRATIONS, "Исчерпан лимит перерегистраций");
				add(LIBFPTR_ERROR_INVALID_SHIFT_NUMBER, "Неверный номер смены");
				add(LIBFPTR_ERROR_INVALID_SERIAL_NUMBER, "Недопустимый серийный номер ККТ");
				add(LIBFPTR_ERROR_INVALID_RNM_VATIN, "Недопустимый РНМ и/или ИНН");
				add(LIBFPTR_ERROR_FISCAL_PRINTER_NOT_ACTIVATED, "ККТ не зарегистрирована");
				add(LIBFPTR_ERROR_SERIAL_NUMBER_NOT_ENTERED, "Не задан серийный номер");
				add(LIBFPTR_ERROR_NO_MORE_REPORTS, "Нет отчетов");
				add(LIBFPTR_ERROR_MODE_NOT_ACTIVATED, "Режим не активизирован");
				add(LIBFPTR_ERROR_RECORD_NOT_FOUND_IN_JOURNAL, "Данные документа отсутствуют");
				add(LIBFPTR_ERROR_INVALID_LICENSE, "Некорректный код защиты / лицензия или номер");
				add(LIBFPTR_ERROR_NEED_FULL_RESET, "Требуется выполнение общего гашения");
				add(LIBFPTR_ERROR_DENIED_BY_LICENSE, "Команда не разрешена введенными кодами защиты / лицензиями ККТ");
				add(LIBFPTR_ERROR_DISCOUNT_CANCELLATION_DENIED, "Невозможна отмена скидки/надбавки");
				add(LIBFPTR_ERROR_CLOSE_RECEIPT_DENIED, "Невозможно закрыть чек данным типом оплаты");
				add(LIBFPTR_ERROR_INVALID_ROUTE_NUMBER, "Неверный номер маршрута");
				add(LIBFPTR_ERROR_INVALID_START_ZONE_NUMBER, "Неверный номер начальной зоны");
				add(LIBFPTR_ERROR_INVALID_END_ZONE_NUMBER, "Неверный номер конечной зоны");
				add(LIBFPTR_ERROR_INVALID_RATE_TYPE, "Неверный тип тарифа");
				add(LIBFPTR_ERROR_INVALID_RATE, "Неверный тариф");
				add(LIBFPTR_ERROR_FISCAL_MODULE_EXCHANGE, "Ошибка обмена с фискальным модулем");
				add(LIBFPTR_ERROR_NEED_TECHNICAL_SUPPORT, "Необходимо провести профилактические работы");
				add(LIBFPTR_ERROR_SHIFT_NUMBERS_DID_NOT_MATCH, "Неверные номера смен в ККТ и ФН");
				add(LIBFPTR_ERROR_DEVICE_NOT_FOUND, "Нет устройства, обрабатывающего данную команду");
				add(LIBFPTR_ERROR_EXTERNAL_DEVICE_CONNECTION, "Нет связи с внешним устройством");
				add(LIBFPTR_ERROR_DISPENSER_INVALID_STATE, "Ошибочное состояние ТРК");
				add(LIBFPTR_ERROR_INVALID_POSITIONS_COUNT, "Недопустимое кол-во позиций в чеке");
				add(LIBFPTR_ERROR_DISPENSER_INVALID_NUMBER, "Ошибочный номер ТРК");
				add(LIBFPTR_ERROR_INVALID_DIVIDER, "Неверный делитель");
				add(LIBFPTR_ERROR_FN_ACTIVATION_DENIED, "Активация данного ФН в составе данной ККТ невозможна");
				add(LIBFPTR_ERROR_PRINTER_OVERHEAT, "Перегрев головки принтера");
				add(LIBFPTR_ERROR_FN_EXCHANGE, "Ошибка обмена с ФН на уровне интерфейса I2C");
				add(LIBFPTR_ERROR_FN_INVALID_FORMAT, "Ошибка формата передачи ФН");
				add(LIBFPTR_ERROR_FN_INVALID_STATE, "Неверное состояние ФН");
				add(LIBFPTR_ERROR_FN_FAULT, "Неисправимая ошибка ФН");
				add(LIBFPTR_ERROR_FN_CRYPTO_FAULT, "Ошибка КС ФН");
				add(LIBFPTR_ERROR_FN_EXPIRED, "Закончен срок эксплуатации ФН");
				add(LIBFPTR_ERROR_FN_OVERFLOW, "Архив ФН переполнен");
				add(LIBFPTR_ERROR_FN_INVALID_DATE_TIME, "В ФН переданы неверная дата или время");
				add(LIBFPTR_ERROR_FN_NO_MORE_DATA, "В ФН нет запрошенных данных");
				add(LIBFPTR_ERROR_FN_TOTAL_OVERFLOW, "Переполнение ФН (итог чека / смены)");
				add(LIBFPTR_ERROR_BUFFER_OVERFLOW, "Буфер переполнен");
				add(LIBFPTR_ERROR_PRINT_SECOND_COPY_DENIED, "Невозможно напечатать вторую фискальную копию");
				add(LIBFPTR_ERROR_NEED_RESET_JOURNAL, "Требуется гашение ЭЖ");
				add(LIBFPTR_ERROR_TAX_SUM_TOO_MUCH, "Некорректная сумма налога");
				add(LIBFPTR_ERROR_TAX_ON_LAST_OPERATION_DENIED, "Начисление налога на последнюю операцию невозможно");
				add(LIBFPTR_ERROR_INVALID_FN_NUMBER, "Неверный номер ФН");
				add(LIBFPTR_ERROR_TAX_CANCEL_DENIED, "Сумма сторно налога больше суммы зарегистрированного налога данного типа");
				add(LIBFPTR_ERROR_LOW_BATTERY, "Операция невозможна, недостаточно питания");
				add(LIBFPTR_ERROR_FN_INVALID_COMMAND, "Некорректное значение параметров команды ФН");
				add(LIBFPTR_ERROR_FN_COMMAND_OVERFLOW, "Превышение размеров TLV данных ФН");
				add(LIBFPTR_ERROR_FN_NO_TRANSPORT_CONNECTION, "Нет транспортного соединения ФН");
				add(LIBFPTR_ERROR_FN_CRYPTO_HAS_EXPIRED, "Исчерпан ресурс КС ФН");
				add(LIBFPTR_ERROR_FN_RESOURCE_HAS_EXPIRED, "Ресурс хранения ФД исчерпан");
				add(LIBFPTR_ERROR_INVALID_MESSAGE_FROM_OFD, "Сообщение от ОФД не может быть принято ФН");
				add(LIBFPTR_ERROR_FN_HAS_NOT_SEND_DOCUMENTS, "В ФН есть неотправленные ФД");
				add(LIBFPTR_ERROR_FN_TIMEOUT, "Исчерпан ресурс ожидания передачи сообщения в ФН");
				add(LIBFPTR_ERROR_FN_SHIFT_EXPIRED, "Продолжительность смены ФН более 24 часов");
				add(LIBFPTR_ERROR_FN_INVALID_TIME_DIFFERENCE, "Неверная разница во времени между двумя операциями ФН");
				add(LIBFPTR_ERROR_INVALID_TAXATION_TYPE, "Некорректная СНО");
				add(LIBFPTR_ERROR_INVALID_TAX_TYPE, "Недопустимый номер ставки налога");
				add(LIBFPTR_ERROR_INVALID_COMMODITY_PAYMENT_TYPE, "Недопустимый тип оплаты товара");
				add(LIBFPTR_ERROR_INVALID_COMMODITY_CODE_TYPE, "Недопустимый тип кода товара");
				add(LIBFPTR_ERROR_EXCISABLE_COMMODITY_DENIED, "Недопустима регистрация подакцизного товара");
				add(LIBFPTR_ERROR_FISCAL_PROPERTY_WRITE, "Ошибка программирования реквизита");
				add(LIBFPTR_ERROR_INVALID_COUNTER_TYPE, "Неверный тип счетчика");
				add(LIBFPTR_ERROR_CUTTER_FAULT, "Ошибка отрезчика");
				add(LIBFPTR_ERROR_REPORT_INTERRUPTED, "Снятие отчета прервалось");
				add(LIBFPTR_ERROR_INVALID_LEFT_MARGIN, "Недопустимое значение отступа слева");
				add(LIBFPTR_ERROR_INVALID_ALIGNMENT, "Недопустимое значение выравнивания");
				add(LIBFPTR_ERROR_INVALID_TAX_MODE, "Недопустимое значение режима работы с налогом");
				add(LIBFPTR_ERROR_FILE_NOT_FOUND, "Файл не найден или неверный формат");
				add(LIBFPTR_ERROR_PICTURE_TOO_BIG, "Размер картинки слишком большой");
				add(LIBFPTR_ERROR_INVALID_BARCODE_PARAMS, "Не удалось сформировать штрихкод");
				add(LIBFPTR_ERROR_FISCAL_PROPERTY_DENIED, "Неразрешенные реквизиты");
				add(LIBFPTR_ERROR_FN_INTERFACE, "Ошибка интерфейса ФН");
				add(LIBFPTR_ERROR_DATA_DUPLICATE, "Дублирование данных");
				add(LIBFPTR_ERROR_NO_REQUIRED_FISCAL_PROPERTY, "Не указаны обязательные реквизиты");
				add(LIBFPTR_ERROR_FN_READ_DOCUMENT, "Ошибка чтения документа из ФН");
				add(LIBFPTR_ERROR_FLOAT_OVERFLOW, "Переполнение чисел с плавающей точкой");
				add(LIBFPTR_ERROR_INVALID_SETTING_VALUE, "Неверное значение параметра ККТ");
				add(LIBFPTR_ERROR_HARD_FAULT, "Внутренняя ошибка ККТ");
				add(LIBFPTR_ERROR_FN_NOT_FOUND, "ФН не найден");
				add(LIBFPTR_ERROR_INVALID_AGENT_FISCAL_PROPERTY, "Невозможно записать реквизит агента");
				add(LIBFPTR_ERROR_INVALID_FISCAL_PROPERTY_VALUE_1002_1056, "Недопустимое сочетания реквизитов 1002 и 1056");
				add(LIBFPTR_ERROR_INVALID_FISCAL_PROPERTY_VALUE_1002_1017, "Недопустимое сочетания реквизитов 1002 и 1017");
				add(LIBFPTR_ERROR_SCRIPT, "Ошибка скриптового движка ККТ");
				add(LIBFPTR_ERROR_INVALID_USER_MEMORY_INDEX, "Неверный номер пользовательской ячейки памяти");
				add(LIBFPTR_ERROR_NO_ACTIVE_OPERATOR, "Кассир не зарегистрирован");
				add(LIBFPTR_ERROR_REGISTRATION_REPORT_INTERRUPTED, "Отчет о регистрации ККТ прерван");
				add(LIBFPTR_ERROR_CLOSE_FN_REPORT_INTERRUPTED, "Отчет о закрытии ФН прерван");
				add(LIBFPTR_ERROR_OPEN_SHIFT_REPORT_INTERRUPTED, "Отчет об открытии смены прерван");
				add(LIBFPTR_ERROR_OFD_EXCHANGE_REPORT_INTERRUPTED, "Отчет о состоянии расчетов прерван");
				add(LIBFPTR_ERROR_CLOSE_RECEIPT_INTERRUPTED, "Закрытие чека прервано");
				add(LIBFPTR_ERROR_FN_QUERY_INTERRUPTED, "Получение документа из ФН прервано");
				add(LIBFPTR_ERROR_RTC_FAULT, "Сбой часов");
				add(LIBFPTR_ERROR_MEMORY_FAULT, "Сбой памяти");
				add(LIBFPTR_ERROR_CHIP_FAULT, "Сбой микросхемы");
				add(LIBFPTR_ERROR_TEMPLATES_CORRUPTED, "Ошибка шаблонов документов");
				add(LIBFPTR_ERROR_INVALID_MAC_ADDRESS, "Недопустимое значение MAC-адреса");
				add(LIBFPTR_ERROR_INVALID_SCRIPT_NUMBER, "Неверный тип (номер) шаблона");
				add(LIBFPTR_ERROR_SCRIPTS_FAULT, "Загруженные шаблоны повреждены или отсутствуют");
				add(LIBFPTR_ERROR_INVALID_SCRIPTS_VERSION, "Несовместимая версия загруженных шаблонов");
				add(LIBFPTR_ERROR_INVALID_CLICHE_FORMAT, "Ошибка в формате клише");
				add(LIBFPTR_ERROR_WAIT_FOR_REBOOT, "Требуется перезагрузка ККТ");
				add(LIBFPTR_ERROR_NO_LICENSE, "Подходящие лицензии не найдены");
				add(LIBFPTR_ERROR_INVALID_FFD_VERSION, "Неверная версия ФФД");
				add(LIBFPTR_ERROR_CHANGE_SETTING_DENIED, "Параметр доступен только для чтения");
				add(LIBFPTR_ERROR_INVALID_NOMENCLATURE_TYPE, "Неверный тип кода товара");
				add(LIBFPTR_ERROR_INVALID_GTIN, "Неверное значение GTIN");
				add(LIBFPTR_ERROR_NEGATIVE_MATH_RESULT, "Отрицательный результат математической операции");
				add(LIBFPTR_ERROR_FISCAL_PROPERTIES_COMBINATION, "Недопустимое сочетание реквизитов");
				add(LIBFPTR_ERROR_OPERATOR_LOGIN, "Ошибка регистрации кассира");
				add(LIBFPTR_ERROR_INVALID_INTERNET_CHANNEL, "Данный канал Интернет отсутствует в ККТ");
				add(LIBFPTR_ERROR_DATETIME_NOT_SYNCRONIZED, "Дата и время не синхронизированы");
				add(LIBFPTR_ERROR_JOURNAL, "Ошибка электронного журнала");
				add(LIBFPTR_ERROR_DENIED_IN_OPENED_DOC, "Документ открыт - операция невозможна");
				add(LIBFPTR_ERROR_DENIED_IN_CLOSED_DOC, "Документ закрыт - операция невозможна");
				add(LIBFPTR_ERROR_LICENSE_MEMORY_OVERFLOW, "Нет места для сохранения лицензий");
				add(LIBFPTR_ERROR_NEED_CANCEL_DOCUMENT, "Произошла критичная ошибка, документ необходимо отменить");
				add(LIBFPTR_ERROR_REGISTERS_NOT_INITIALIZED, "Регистры ККТ еще не инициализированы");
				add(LIBFPTR_ERROR_TOTAL_REQUIRED, "Требуется регистрация итога");
				add(LIBFPTR_ERROR_SETTINGS_FAULT, "Сбой таблицы настроек");
				add(LIBFPTR_ERROR_COUNTERS_FAULT, "Сбой счетчиков и регистров ККТ");
				add(LIBFPTR_ERROR_USER_MEMORY_FAULT, "Сбой пользовательской памяти");
				add(LIBFPTR_ERROR_SERVICE_COUNTERS_FAULT, "Сбой сервисных регистров");
				add(LIBFPTR_ERROR_ATTRIBUTES_FAULT, "Сбой реквизитов ККТ");
				add(LIBFPTR_ERROR_ALREADY_IN_UPDATE_MODE, "ККТ уже в режиме обновления конфигурации");
				add(LIBFPTR_ERROR_INVALID_FIRMWARE, "Конфигурация не прошла проверку");
				add(LIBFPTR_ERROR_INVALID_CHANNEL, "Аппаратный канал отсутствует, выключен или ещё не проинициализирован");
				add(LIBFPTR_ERROR_INTERFACE_DOWN, "Сетевой интерфейс не подключен, или на нём не получен IP-адрес");
				add(LIBFPTR_ERROR_INVALID_FISCAL_PROPERTY_VALUE_1212_1030, "Недопустимое сочетание реквизитов 1212 и 1030");
				add(LIBFPTR_ERROR_INVALID_FISCAL_PROPERTY_VALUE_1214, "Некорректный признак способа расчета");
				add(LIBFPTR_ERROR_INVALID_FISCAL_PROPERTY_VALUE_1212, "Некорректный признак предмета расчета");
				add(LIBFPTR_ERROR_SYNC_TIME, "Ошибка синхронизации времени");
				add(LIBFPTR_ERROR_VAT18_VAT20_IN_RECEIPT, "В одном чеке одновременно не может быть позиций с НДС 18% (18/118) и НДС 20% (20/120)");
				add(LIBFPTR_ERROR_PICTURE_NOT_CLOSED, "Картинка не закрыта");
				add(LIBFPTR_ERROR_INTERFACE_BUSY, "Сетевой интерфейс занят");
				add(LIBFPTR_ERROR_INVALID_PICTURE_NUMBER, "Неверный номер картинки");
				add(LIBFPTR_ERROR_INVALID_CONTAINER, "Ошибка проверки контейнера");
				add(LIBFPTR_ERROR_ARCHIVE_CLOSED, "Архив ФН закрыт");
				add(LIBFPTR_ERROR_NEED_REGISTRATION, "Нужно выполнить регистрацию / перерегистрацию");
				add(LIBFPTR_ERROR_DENIED_DURING_UPDATE, "Операция невозможна, идет обновление ПО ККТ");
				add(LIBFPTR_ERROR_INVALID_TOTAL, "Неверный итог чека");
				add(LIBFPTR_ERROR_MARKING_CODE_CONFLICT, "Запрещена одновременная передача КМ и реквизита 1162");
				add(LIBFPTR_ERROR_INVALID_RECORDS_ID, "Набор записей по заданному идентификатору не найден");
				add(LIBFPTR_ERROR_INVALID_SIGNATURE, "Ошибка цифровой подписи");
				add(LIBFPTR_ERROR_INVALID_EXCISE_SUM, "Некорректная сумма акциза");
				add(LIBFPTR_ERROR_NO_DOCUMENTS_FOUND_IN_JOURNAL, "Заданный диапазон документов не найден в БД документов");
				add(LIBFPTR_ERROR_INVALID_SCRIPT_TYPE, "Неподдерживаемый тип скрипта");
				add(LIBFPTR_ERROR_INVALID_SCRIPT_NAME, "Некорректный идентификатор скрипта");
				add(LIBFPTR_ERROR_INVALID_POSITIONS_COUNT_WITH_1162, "Кол-во позиций с реквизитом 1162 в автономном режиме превысило разрешенный лимит");
				add(LIBFPTR_ERROR_INVALID_UC_COUNTER, "Универсальный счетчик с заданными параметрами недоступен");
				add(LIBFPTR_ERROR_INVALID_UC_TAG, "Неподдерживаемый тег для универсальных счетчиков");
				add(LIBFPTR_ERROR_INVALID_UC_IDX, "Некорректный индекс для универсальных счетчиков");
				add(LIBFPTR_ERROR_INVALID_UC_SIZE, "Неверный размер универсального счетчика");
				add(LIBFPTR_ERROR_INVALID_UC_CONFIG, "Неверная конфигурация универсальных счетчиков");
				add(LIBFPTR_ERROR_CONNECTION_LOST, "Соединение с ККТ потеряно");
				add(LIBFPTR_ERROR_UNIVERSAL_COUNTERS_FAULT, "Ошибка универсальных счетчиков");
				add(LIBFPTR_ERROR_INVALID_TAX_SUM, "Некорректная сумма налога");
				add(LIBFPTR_ERROR_INVALID_MARKING_CODE_TYPE, "Некорректное значение типа кода маркировки");
				add(LIBFPTR_ERROR_LICENSE_HARD_FAULT, "Аппаратная ошибка при сохранении лицензии");
				add(LIBFPTR_ERROR_LICENSE_INVALID_SIGN, "Подпись лицензии некорректна");
				add(LIBFPTR_ERROR_LICENSE_INVALID_SERIAL, "Лицензия не подходит для данной ККТ");
				add(LIBFPTR_ERROR_LICENSE_INVALID_TIME, "Срок действия лицензии истёк");
				add(LIBFPTR_ERROR_BASE_WEB, "Base web error");
				add(LIBFPTR_ERROR_RECEIPT_PARSE_ERROR, "Ошибка парсинга чека / запроса");
				add(LIBFPTR_ERROR_INTERRUPTED_BY_PREVIOUS_ERRORS, "Выполнение прервано из-за предыдущих ошибок");
				add(LIBFPTR_ERROR_DRIVER_SCRIPT_ERROR, "Ошибка скрипта драйвера");
			}
		};
	}
}

//--------------------------------------------------------------------------------
