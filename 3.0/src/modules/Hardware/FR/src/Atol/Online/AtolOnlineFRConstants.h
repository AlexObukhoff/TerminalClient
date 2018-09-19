/* @file Константы, коды команд и ответов протокола ФР АТОЛ онлайн. */

#pragma once

// Models
#include "Hardware/Common/WaitingData.h"

// Project
#include "../AtolFRConstants.h"

//--------------------------------------------------------------------------------
namespace CAtolOnlineFR
{
	/// Имя модели по умолчанию
	const char DefaultModelName[] = "ATOL online FR";

	/// Снятие Z-отчетов наружу
	const char ZReportOut = '\x00';

	/// Снятие Z-отчетов в буфер
	const char ZReportInBuffer = '\x02';

	/// Автоснятие Z-отчетов в буфер по истечении 24 часов включено.
	const char AutoZReportTimingEnable = '\x02';

	/// Печать X- и Z-отчетов - презентовать.
	const char PresentReports = '\x00';

	/// Печатать ОФД-реквизит.
	const char PrintOFDParameter = '\x01';

	/// Налоги: округление налогов - на весь чек, контроль суммы налогов 10 и 18(20) %.
	const char TaxParameters = '\x02';

	/// Печать параметров Z-отчета: печатать все счетчики и данные о приходах и расходах.
	const char ZReportParameters = '\x00';

	/// Тип чека - приход.
	const char SalingFiscalDocument = '\x01';

	/// Формат представления даты [активизации] ФН в ответе на длинный запрос статуса ФН.
	const char DateFormat[] = "yyyyMMdd";

	/// Формат представления даты для вывода в лог.
	const char TimeLogFormat[] = "hh:mm";

	/// Ожидание оживания ФР при получении фискальных данных после закрытия чека, [мс].
	const SWaitingData GetFiscalWaiting = SWaitingData(200, 1000);

	/// Максимальное количество повторов при ожидании оживания ФР при получении фискальных данных после закрытия чека.
	const int MaxRepeatingFiscalData = 3;

	/// Пауза после перезагрузки по питанию, [мс].
	const int RebootPause = 1000;

	/// Ожидание выхода из анабиоза, [мс].
	const SWaitingData RebootWaiting = SWaitingData(1000, 20000);

	//------------------------------------------------------------------------------------------------
	namespace FRParameters
	{
		using namespace CAtolFR::FRParameters;

		inline SData TaxDescription(int aSeries) { return SData(13, ushort(aSeries), 1); }
		inline SData            Tax(int aSeries) { return SData(13, ushort(aSeries), 2); }

		const SData DocumentPerforming   = SData(2, 1,  98);
		const SData PrintRNM             = SData(2, 1, 106);
		const SData PrinterModel         = SData(2, 1, 112);
		const SData Taxes                = SData(2, 1, 114);
		const SData ZReport              = SData(2, 1, 118);
		const SData SetReportsProcessing = SData(2, 1, 121);

		const SData OFDAddress           = SData(19, 1, 1);
		const SData OFDPort              = SData(19, 1, 2);
		const SData SetAutoZReportTiming = SData(22, 1, 2);
	}

	/// Данные команды начала формирования позиции продажи - выполнить операцию + 2 магических числа
	const QByteArray StartSailingData = QByteArray::fromRawData("\x00\x01\x00", 3);

	/// Флаги выполнения продажи - Выполнить операцию + проверить денежную наличность + налог на конкретную позицию.
	const char SaleFlags = CAtolFR::FiscalFlags::ExecutionMode | CAtolFR::FiscalFlags::CashChecking | CAtolFR::FiscalFlags::TaxForPosition;

	/// Регистры
	namespace Registers
	{
		const char OFDNotSentCount[]   = "not sent fiscal to OFD documents count";
		const char ExtendedErrorData[] = "extended error data";
		const char FFD[] = "FFD";
		const char SessionData[] = "session data";
	}

	//------------------------------------------------------------------------------------------------
	/// Команды.
	namespace Commands
	{
		/// Фискальные команды.
		const char StartSale             = '\xEA';    /// Начать формирование позиции.
		const char EndSale               = '\xEB';    /// Завершить формирование позиции.
		const char SetOFDParameter       = '\xE8';    /// Запись реквизита.
		const char GetOFDParameter       = '\xE9';    /// Получение реквизита.
		const char PrintDeferredZReports = '\x95';    /// Распечатать отложенные Z-отчеты.
		const char ClearZBuffer          = '\x97';    /// Очистить буфер Z-отчетов.
		const char Reboot                = '\xCE';    /// Перезагрузить ФР по питанию.
		const char GetInternalFirmware[] = "\x9D\x91";    /// Получить внутренний номер прошивки.

		/// Коды команд ФН-а.
		namespace FS
		{
			const char GetStatus[]             = "\xA4\x30";    /// Получить статус ФН.
			const char GetNumber[]             = "\xA4\x31";    /// Получить номер ФН.
			const char GetValidity[]           = "\xA4\x32";    /// Получить срок действия ФН.
			const char GetVersion[]            = "\xA4\x33";    /// Получить версию ФН.
			const char GetFiscalizationTotal[] = "\xA4\x43";    /// Получить итог фискализации.
			const char GetFDbyNumber[]         = "\xA4\x40";    /// Получить фискальный документ по его номеру.
			const char StartFiscalTLVData[]    = "\xA4\x45";    /// Начать получение данных фискального документа в TLV-формате.
			const char GetFiscalTLVData[]      = "\xA4\x46";    /// Получить данные фискального документа в TLV-формате.
		}
	}

	//------------------------------------------------------------------------------------------------
	/// Ошибки.
	namespace Errors
	{
		const char NoRequiedDataInFS     = '\xDA';    /// В ФН нет запрошенных данных.
		const char FSOfflineEnd          = '\xEB';    /// Исчерпан ресурс хранения ФН.
		const char NeedExtendedErrorCode = '\xEE';    /// Запросить расширенный код ошибки в регистре 55.

		class CData : public FRError::CData
		{
		public:
			CData()
			{
				add('\x01', "Контрольная лента обработана без ошибок");
				add('\x08', "Неверная цена (сумма)");
				add('\x0A', "Неверное количество");
				add('\x0B', "Переполнение счетчика наличности");
				add('\x0C', "Невозможно сторно последней операции");
				add('\x0D', "Сторно по коду невозможно (в чеке зарегистрировано меньшее количество товаров с указанным кодом)");
				add('\x0E', "Невозможен повтор последней операции");
				add('\x0F', "Повторная скидка на операцию невозможна");
				add('\x10', "Скидка/надбавка на предыдущую операцию невозможна");
				add('\x11', "Неверный код товара");
				add('\x12', "Неверный штрихкод товара");
				add('\x13', "Неверный формат");
				add('\x14', "Неверная длина");
				add('\x15', "ККТ заблокирована в режиме ввода даты");
				add('\x16', "Требуется подтверждение ввода даты");
				add('\x18', "Нет больше данных для передачи ПО ККТ");
				add('\x19', "Нет подтверждения или отмены продажи");
				add('\x1A', "Отчет с гашением прерван. Вход в режим невозможен.");
				add('\x1B', "Отключение контроля наличности невозможно (не настроены необходимые типы оплаты).");
				add('\x1E', "Вход в режим заблокирован");
				add('\x1F', "Проверьте дату и время");
				add('\x20', "Дата и время в ККТ меньше чем в ФН");
				add('\x21', "Невозможно закрыть архив");
				add('\x3D', "Товар не найден");
				add('\x3E', "Весовой штрихкод с количеством != 1.000");
				add('\x3F', "Переполнение буфера чека");
				add('\x40', "Недостаточное количество товара");
				add('\x41', "Сторнируемое количество больше проданного");
				add('\x42', "Заблокированный товар не найден в буфере чека");
				add('\x43', "Данный товар не продавался в чеке, сторно невозможно");
				add('\x46', "Неверная команда от ККТ");
				add('\x66', "Команда не реализуется в данном режиме ККТ");
				add('\x67', "Нет бумаги");
				add('\x68', "Нет связи с принтером чеков");
				add('\x69', "Механическая ошибка печатающего устройства");
				add('\x6A', "Неверный тип чека");
				add('\x6B', "Нет больше строк картинки/штрихкода");
				add('\x6C', "Неверный номер регистра");
				add('\x6D', "Недопустимое целевое устройство");
				add('\x6E', "Нет места в массиве картинок/штрихкодов");
				add('\x6F', "Неверный номер картинки/штрихкода (картинка/штрихкод отсутствует)");
				add('\x70', "Сумма сторно больше, чем было получено данным типом оплаты");
				add('\x71', "Сумма не наличных платежей превышает сумму чека");
				add('\x72', "Сумма платежей меньше суммы чека");
				add('\x73', "Накопление меньше суммы возврата или аннулирования");
				add('\x75', "Переполнение суммы платежей");
				add('\x76', "Предыдущая операция незавершена");
				add('\x77', "Ошибка GSM-модуля");
				add('\x7A', "Данная модель ККТ не может выполнить команду");
				add('\x7B', "Неверная величина скидки / надбавки");
				add('\x7C', "Операция после скидки / надбавки невозможна");
				add('\x7D', "Неверная секция");
				add('\x7E', "Неверный вид оплаты");
				add('\x7F', "Переполнение при умножении");
				add('\x80', "Операция запрещена в таблице настроек");
				add('\x81', "Переполнение итога чека");
				add('\x82', "Открыт чек аннулирования – операция невозможна");
				add('\x84', "Переполнение буфера контрольной ленты");
				add('\x86', "Вносимая клиентом сумма меньше суммы чека");
				add('\x87', "Открыт чек возврата – операция невозможна");
				add('\x88', "Смена превысила 24 часа");
				add('\x89', "Открыт чек продажи – операция невозможна");
				add('\x8A', "Переполнение ФП");
				add('\x8C', "Неверный пароль");
				add('\x8D', "Буфер контрольной ленты не переполнен");
				add('\x8E', "Идет обработка контрольной ленты");
				add('\x8F', "Обнуленная касса (повторное гашение невозможно)");
				add('\x91', "Неверный номер таблицы");
				add('\x92', "Неверный номер ряда");
				add('\x93', "Неверный номер поля");
				add('\x94', "Неверная дата");
				add('\x95', "Неверное время");
				add('\x96', "Сумма чека по секции меньше суммы сторно");
				add('\x97', "Подсчет суммы сдачи невозможен");
				add('\x98', "В ККТ нет денег для выплаты");
				add('\x9A', "Чек закрыт – операция невозможна");
				add('\x9B', "Чек открыт – операция невозможна");
				add('\x9C', "Смена открыта, операция невозможна");
				add('\x9E', "Заводской номер уже задан");
				add('\xA2', "Неверный номер смены");
				add('\xA3', "Неверный тип отчета");
				add('\xA4', "Недопустимый пароль");
				add('\xA5', "Недопустимый заводской номер ККТ");
				add('\xA6', "Недопустимый РНМ");
				add('\xA7', "Недопустимый ИНН");
				add('\xA8', "ККТ не фискализирована");
				add('\xA9', "Не задан заводской номер");
				add('\xAA', "Нет отчетов");
				add('\xAB', "Режим не активизирован");
				add('\xAC', "Нет указанного чека в КЛ");
				add('\xAD', "Нет больше записей КЛ");
				add('\xAE', "Некорректный код или номер кода защиты ККТ");
				add('\xAF', "Отсутствуют данные в буфере ККТ");
				add('\xB0', "Требуется выполнение общего гашения");
				add('\xB1', "Команда не разрешена введенными кодами защиты ККТ");
				add('\xB2', "Невозможна отмена скидки/надбавки");
				add('\xB3', "Невозможно закрыть чек данным типом оплаты (в чеке присутствуют операции без контроля наличных)");
				add('\xB4', "Неверный номер маршрута");
				add('\xB5', "Неверный номер начальной зоны");
				add('\xB6', "Неверный номер конечной зоны");
				add('\xB7', "Неверный тип тарифа");
				add('\xB8', "Неверный тариф");
				add('\xBA', "Ошибка обмена с фискальным модулем");
				add('\xBE', "Необходимо провести профилактические работы");
				add('\xBF', "Неверные номера смен в ККТ и ФН");
				add('\xC8', "Нет устройства, обрабатывающего данную команду");
				add('\xC9', "Нет связи с внешним устройством");
				add('\xCA', "Ошибочное состояние ТРК");
				add('\xCB', "Больше одной регистрации в чеке");
				add('\xCC', "Ошибочный номер ТРК");
				add('\xCD', "Неверный делитель");
				add('\xD0', "Активизация данного ФН в составе данной ККТ невозможна");
				add('\xD1', "Перегрев головки принтера");
				add('\xD2', "Ошибка обмена с ФН на уровне интерфейса I2C", FRError::EType::FS);
				add('\xD3', "Ошибка формата передачи ФН",                  FRError::EType::FS);
				add('\xD4', "Неверное состояние ФН");
				add('\xD5', "Неисправимая ошибка ФН",                      FRError::EType::FS);
				add('\xD6', "Ошибка КС ФН",                                FRError::EType::FS);
				add('\xD7', "Закончен срок эксплуатации ФН",               FRError::EType::FS);
				add('\xD8', "Архив ФН переполнен",                         FRError::EType::FS);
				add('\xD9', "В ФН переданы неверная дата или время",       FRError::EType::FS);
				add('\xDA', "В ФН нет запрошенных данных",                 FRError::EType::FS);
				add('\xDB', "Переполнение ФН (итог чека)",                 FRError::EType::FS);
				add('\xDC', "Буфер переполнен");
				add('\xDD', "Невозможно напечатать вторую фискальную копию");
				add('\xDE', "Требуется гашение ЭЖ");
				add('\xDF', "Сумма налога больше суммы регистраций по чеку и/или итога или больше суммы регистрации");
				add('\xE0', "Начисление налога на последнюю операцию невозможно");
				add('\xE1', "Неверный номер ФН");
				add('\xE4', "Сумма сторно налога больше суммы зарегистрированного налога данного типа");
				add('\xE5', "Ошибка SD");
				add('\xE6', "Операция невозможна, недостаточно питания");
				add('\xE7', "Некорректное значение параметров команды ФН", FRError::EType::FS);
				add('\xE8', "Превышение размеров TLV данных ФН",           FRError::EType::FS);
				add('\xE9', "Нет транспортного соединения ФН",             FRError::EType::FS);
				add('\xEA', "Исчерпан ресурс КС ФН",                       FRError::EType::FS);
				add('\xEB', "Исчерпан ресурс хранения ФН",                 FRError::EType::FS);
				add('\xEC', "Сообщение от ОФД не может быть принято ФН");
				add('\xED', "В ФН есть неотправленные ФД");
				add('\xEE', "Запросить расширенный код ошибки в регистре 55");
				add('\xEF', "Исчерпан ресурс ожидания передачи сообщения в ФН");
				add('\xF0', "Продолжительность смены ФН более 24 часов");
				add('\xF1', "Неверная разница во времени между 2 операциями ФН");
			}
		};

		class CExtendedData : public CDescription<ushort>
		{
		public:
			CExtendedData()
			{
				append(0x0101, "Недопустимо более одной регистрации в чеке коррекции");
				append(0x0102, "Недопустимо передавать более 10 регистраций при наличии в чеке кода товарной номенклатуры в автономном режиме");
				append(0x0103, "Некорректная СНО");
				append(0x0104, "Недопустимый номер ставки налога");
				append(0x0105, "Недопустимый тип оплаты товара");
				append(0x0106, "Недопустимый тип кода товара");
				append(0x0107, "Недопустима регистрация подакцизного товара");
				append(0x0108, "При передаче налога на единицу запрещены скидки на позицию");
				append(0x0109, "При передаче скидки для печати запрещена регистрация скидки");
				append(0x0201, "Реквизит уже был записан в чеке, повтор запрещен");
				append(0x0202, "Программирование реквизитов запрещено в данном режиме работы ФН (ФН отсутствует или закрыт архив)");
				append(0x0203, "Реквизит недопустимо перепрограммировать для перерегистрации");
				append(0x0204, "Недопустимое изменение состояния реквизита");
				append(0x0205, "Недопустимое сочетание реквизитов");
				append(0x0206, "Не задан необходимый реквизит для совершения операции");
				append(0x0207, "Невозможно записать данные в буфер позиции (не была подана команда EAh)");
				append(0x0208, "Невозможно записать реквизит чека, начато формирование позиции (была подана команда EAh)");
				append(0x0209, "Невозможно запрограммировать реквизит, он уже непечатан");

				setDefault("Неизвестная");
			}
		};

		static CExtendedData ExtendedData;
	}
}

//--------------------------------------------------------------------------------
