/* @file Константы, коды команд и ответов протокола ФР Штрих онлайн. */

#pragma once

// Modules
#include "Hardware/Common/WaitingData.h"

// Project
#include "../ShtrihFRConstants.h"

//--------------------------------------------------------------------------------
namespace CShtrihOnlineFR
{
	/// Формат представления даты [активизации] ФН в ответе на длинный запрос статуса ФН.
	const char DateFormat[] = "yyyyMMdd";

	/// Размер сектора на SD-карте, [байт].
	const int SectorSizeSD = 512;

	/// Количество секторов в 1 МБ.
	const int SectorsInMB = 1024 * 1024 / SectorSizeSD;

	/// Статус SD - недоступна.
	const char SDNotConnected = '\xFE';

	/// Минимальные даты прошивкок, начиная с которых возможно выполнение определенного функционала.
	namespace MinFWDate
	{
		/// Снятие Z-отчетов в буфер.
		const QDate ZBuffer = QDate(2017, 6, 29);

		/// Флаги агента для продажи.
		const QDate AgentFlagOnSale = QDate(2017, 12, 29);

		/// Операции V2.
		const QDate V2 = QDate(2017, 5, 17);
	}

	/// Данные налога для продажи - налог вычисляется ФР.
	const char FiscalTaxData[] = "\xFF\xFF\xFF\xFF\xFF";

	/// Сервисные команды (параметры).
	namespace Service
	{
		/// Cофтварная перезагрузка.
		const QByteArray Reboot = QByteArray::fromRawData("\xF3\x00\x00\x00\x00", 5);

		/// Прошивка загрузчика.
		const QByteArray BootFirmware = QByteArray::fromRawData("\xEC\x00\x00\x00\x00", 5);
	}

	/// Минимально рекомендованная версия загрузчика.
	const uint MinBootFirmware = 133;

	/// Пауза после софтварной перезагрузки ФР, [мс].
	const int RebootPause = 5 * 1000;

	/// Таймаут открытия TCP-порта после перезагрузки ФР, [мс].
	const int TCPReopeningTimeout = 30 * 1000;

	/// Печатать все реквизиты пользователя (название юр. лица, адрес и место расчетов).
	const int PrintFullUserData = 7;

	/// Ряд кассира по умолчанию (сисадмин).
	const int CashierSeries = 30;

	/// Количество типов оплаты.
	const int PayTypeQuantity = 16;

	/// Налоги на закрытии чека по количеству налоговых  групп. Фиктивные, т.к. используются налоги на позицию.
	const QByteArray ClosingFiscalTaxes = QByteArray(6 * 5, ASCII::NUL);

	/// Ожидание готовности, [мс].
	const SWaitingData ReadyWaiting = SWaitingData(200, 15 * 1000);

	/// Максимальный ожидания допечати отчета по отделам.
	const int MaxWaitForPrintingSectionReport = 20 * 1000;

	/// Максимальный ожидания допечати отчета по налогам.
	const int MaxWaitForPrintingTaxReport = 20 * 1000;

	/// Максимальный ожидания допечати отчета по кассирам.
	const int MaxWaitForPrintingCashierReport = 20 * 1000;

	/// Маски для парсинга режимо работы.
	namespace OperationModeMask
	{
		const char ExcisableUnitMode = '\x01';     // Торговля подакцизными товарами (1207).
		const char GamblingMode      = '\x02';     // Проведение азартных игр (1193).
		const char LotteryMode       = '\x04';     // Проведение лотереи (1126).
		const char InAutomateMode    = '\x08';     // Признак установки в автомате (1221).
	}

	/// Типы фискальных чеков.
	namespace DocumentTypes
	{
		const char Sale     = '\x01';    /// Продажа.
		const char SaleBack = '\x02';    /// Возврат продажи.
	}

	/// Параметры ФР.
	namespace FRParameters
	{
		using namespace CShtrihFR::FRParameters;

		const SData Cashier             = SData( 2,  2);    /// Кассир по умолчанию (сисадмин).
		const SData NotPrintDocument    = SData( 7, 17);    /// Настройка печати любого документа.
		const SData PrintEndToEndNumber = SData( 9, 17);    /// Печатать сквозной номер документа.
		const SData PrintOFDData        = SData(10, 17);    /// Печатать данные ОФД.
		const SData PrintUserData       = SData(12, 17);    /// Печатать реквизиты [суб]дилера.
		const SData FFDFR               = SData(17, 17);    /// ФФД ФР.
		const SData PrintCustomFields   = SData(25, 17);    /// Автопечать тегов, вводимых на платеже.
		const SData SerialNumber        = SData( 1, 18);    /// Серийный номер.
		const SData INN                 = SData( 2, 18);    /// ИНН.
		const SData RNM                 = SData( 3, 18);    /// РНМ.
		const SData TaxSystem           = SData( 5, 18);    /// СНО.
		const SData LegalOwner          = SData( 7, 18);    /// Наименование юр. лица владельца.
		const SData PayOffAddress       = SData( 9, 18);    /// Адрес расчетов.
		const SData OFDName             = SData(10, 18);    /// Наименование ОФД.
		const SData OFDURL              = SData(11, 18);    /// Aдрес сайта ОФД.
		const SData FTSURL              = SData(13, 18);    /// Адрес сайта ФНС.
		const SData PayOffPlace         = SData(14, 18);    /// Место расчетов.
		const SData AgentFlag           = SData(16, 18);    /// Признак агента.
		const SData OperationModes      = SData(21, 18);    /// Режимы работы.
		const SData OFDAddress          = SData( 1, 19);    /// Aдрес ОФД.
		const SData OFDPort             = SData( 2, 19);    /// Порт ОФД.
		const SData AutomaticNumber     = SData( 1, 24);    /// Номер автомата.
		const SData PrinterModel        = SData( 2, 24);    /// Модель подключенного принтера.
		const SData QRCode              = SData( 8, 24);    /// Формировать QR-код (средствами ФР, а не принтера).

		/// Параметры автообновления.
		namespace FirmwareUpdating
		{
			const SData Working  = SData(1, 23);    /// Работать/не работать с сервером.
			const SData Interval = SData(3, 23);    /// Период опроса сервера.
			const SData Enabling = SData(5, 23);    /// Включить/выключить.
			const SData Single   = SData(6, 23);    /// Однократное/неоднократное обновление.
		}

		/// Параметры SD-карты.
		namespace SD
		{
			const SData Status          = SData( 1, 14);    /// Статус.
			const SData ClusterSize     = SData( 2, 14);    /// Размер кластера.
			const SData TotalSize       = SData( 3, 14);    /// Общий размер.
			const SData FreeSize        = SData( 4, 14);    /// Размер свободного места.
			const SData IOErrors        = SData( 5, 14);    /// Количество ошибок чтения/записи.
			const SData RetryCount      = SData( 6, 14);    /// Количество попыток доступа (?).
		}
	}

	/// Параметры автообновления.
	namespace FirmwareUpdating
	{
		const int Working  = 1;      /// Работать с сервером автообновления.
		const int Interval = 600;    /// Интервал опроса сервера автообновления, [c].
		const int Enabling = 1;      /// Разрешить автообновление.
		const int Single   = 0;      /// Многократное обновление.
	}

	/// Коды команд.
	namespace Commands
	{
		const char GetPrinterStatus  = '\xD1';    /// Получить статус принтера.
		const char Service           = '\xFE';    /// Сервисная команда.

		/// Коды команд ФН.
		namespace FS
		{
			const char GetStatus[]               = "\xFF\x01";    /// Получить статус ФН.
			const char GetNumber[]               = "\xFF\x02";    /// Получить номер ФН.
			const char GetValidity[]             = "\xFF\x03";    /// Получить срок действия ФН.
			const char GetVersion[]              = "\xFF\x04";    /// Получить версию ФН.
			const char GetFiscalizationTotal[]   = "\xFF\x09";    /// Получить итог фискализации.
			const char GetFDbyNumber[]           = "\xFF\x0A";    /// Получить фискальный документ по его номеру.
			const char SetOFDParameter[]         = "\xFF\x0C";    /// Передать произвольную TLV структуру (реквизит для ОФД).
			const char GetOFDInterchangeStatus[] = "\xFF\x39";    /// Получить статус информационного обмена c ОФД.
			const char StartFiscalTLVData[]      = "\xFF\x3A";    /// Начать получение данных фискального документа в TLV-формате.
			const char GetFiscalTLVData[]        = "\xFF\x3B";    /// Получить данные фискального документа в TLV-формате.
			const char GetSessionParameters[]    = "\xFF\x40";    /// Получить параметры текущей смены.
			const char CloseDocument[]           = "\xFF\x45";    /// Закрыть фискальный чек.
			const char Sale[]                    = "\xFF\x46";    /// Продажа.
			const char SetOFDParameterLinked[]   = "\xFF\x4D";    /// Передать произвольную TLV структуру (реквизит для ОФД), привязанную к операции.
		}
	}

	/// Коды ошибок (некоторых).
	namespace Errors
	{
		const char WrongFSState      = '\x02';    /// Неверное состояние ФН.
		const char NoRequiedDataInFS = '\x08';    /// Нет запрошенных данных.
		const char FSOfflineEnd      = '\x14';    /// ФН Исчерпан ресурс хранения.
		const char NeedZReport       = '\x16';    /// ФН Продолжительность смены более 24 часов.

		class Data : public FRError::Data
		{
		public:
			Data()
			{
				using namespace FRError;

				add('\x01', "Неизвестная команда, неверный формат посылки или неизвестные параметры");
				add('\x02', "Неверное состояние ФН");
				add('\x03', "Ошибка ФН",                                      EType::FS);
				add('\x04', "Ошибка КС",                                      EType::FS);
				add('\x05', "Закончен срок эксплуатации ФН",                  EType::FS);
				add('\x06', "Архив ФН переполнен",                            EType::FS);
				add('\x07', "ФН Неверные дата и/или время",                   EType::FS);
				add('\x08', "Нет запрошенных данных",                         EType::FS);
				add('\x09', "Некорректное значение параметров команды",       EType::FS);
				add('\x10', "Превышение размеров TLV данных",                 EType::FS);
				add('\x11', "Нет транспортного соединения",                   EType::FS);
				add('\x12', "ФН Исчерпан ресурс КС",                          EType::FS);
				add('\x14', "ФН Исчерпан ресурс хранения",                    EType::FS);
				add('\x15', "ФН Исчерпан ресурс ожидания передачи сообщения", EType::FS);
				add('\x16', "ФН Продолжительность смены более 24 часов");
				add('\x17', "ФН Неверная разница во времени между 2 операцими");
				add('\x20', "ФН Сообщение от ОФД не может быть принято");
				add('\x2F', "Таймаут обмена с ФН",                            EType::FS);
				add('\x30', "ФН не отвечает",                                 EType::FS);
				add('\x33', "Некорректные параметры в команде");
				add('\x34', "Нет данных");
				add('\x35', "Некорректный параметр при данных настройках");
				add('\x36', "Некорректные параметры в команде для данной реализации ККТ");
				add('\x37', "Команда не поддерживается в данной реализации ККТ");
				add('\x38', "Ошибка в ПЗУ");
				add('\x39', "Внутренняя ошибка ПО ККТ");
				add('\x3A', "Переполнение накопления по надбавкам в смене");
				add('\x3C', "Смена открыта – операция не возможна");
				add('\x3D', "Смена не открыта – операция не возможна");
				add('\x3E', "Переполнение накопления по секциям в смене");
				add('\x3F', "Переполнение накопления по скидкам в смене");
				add('\x40', "Переполнение диапазона скидок");
				add('\x41', "Переполнение диапазона оплаты наличными");
				add('\x42', "Переполнение диапазона оплаты типом 2");
				add('\x43', "Переполнение диапазона оплаты типом 3");
				add('\x44', "Переполнение диапазона оплаты типом 4");
				add('\x45', "Сумма всех типов оплаты меньше итога чека");
				add('\x46', "Не хватает наличности в кассе");
				add('\x47', "Переполнение накопления по налогам в смене");
				add('\x48', "Переполнение итога чека");
				add('\x49', "Операция невозможна в открытом чеке данного типа");
				add('\x4A', "Открыт чек – операция невозможна");
				add('\x4B', "Буфер чека переполнен");
				add('\x4C', "Переполнение накопления по обороту налогов в смене");
				add('\x4D', "Вносимая безналичной оплатой сумма больше суммы чека");
				add('\x4E', "Смена превысила 24 часа");
				add('\x4F', "Неверный пароль");
				add('\x50', "Идет печать результатов выполнения предыдущей команды");
				add('\x51', "Переполнение накоплений наличными в смене");
				add('\x52', "Переполнение накоплений по типу оплаты 2 в смене");
				add('\x53', "Переполнение накоплений по типу оплаты 3 в смене");
				add('\x54', "Переполнение накоплений по типу оплаты 4 в смене");
				add('\x55', "Чек закрыт – операция невозможна");
				add('\x56', "Нет документа для повтора");
				add('\x58', "Ожидание команды продолжения печати");
				add('\x59', "Документ открыт другим оператором");
				add('\x5B', "Переполнение диапазона надбавок");
				add('\x5C', "Понижено напряжение 24В");
				add('\x5D', "Таблица не определена");
				add('\x5E', "Неверная операция");
				add('\x5F', "Отрицательный итог чека");
				add('\x60', "Переполнение при умножении");
				add('\x61', "Переполнение диапазона цены");
				add('\x62', "Переполнение диапазона количества");
				add('\x63', "Переполнение диапазона отдела");
				add('\x65', "Не хватает денег в секции");
				add('\x66', "Переполнение денег в секции");
				add('\x68', "Не хватает денег по обороту налогов");
				add('\x69', "Переполнение денег по обороту налогов");
				add('\x6A', "Ошибка питания в момент ответа по I2C");
				add('\x6B', "Нет чековой ленты");
				add('\x6D', "Не хватает денег по налогу");
				add('\x6E', "Переполнение денег по налогу");
				add('\x6F', "Переполнение по выплате в смене");
				add('\x71', "Ошибка отрезчика");
				add('\x72', "Команда не поддерживается в данном подрежиме");
				add('\x73', "Команда не поддерживается в данном режиме");
				add('\x74', "Ошибка ОЗУ");
				add('\x75', "Ошибка питания");
				add('\x77', "Ошибка принтера: нет сигнала с датчиков");
				add('\x78', "Замена ПО");
				add('\x7A', "Поле не редактируется");
				add('\x7B', "Ошибка оборудования");
				add('\x7C', "Не совпадает дата");
				add('\x7D', "Неверный формат даты");
				add('\x7E', "Неверное значение в поле длины");
				add('\x7F', "Переполнение диапазона итога чека");
				add('\x84', "Переполнение наличности");
				add('\x85', "Переполнение по продажам в смене");
				add('\x86', "Переполнение по покупкам в смене");
				add('\x87', "Переполнение по возвратам продаж в смене");
				add('\x88', "Переполнение по возвратам покупок в смене");
				add('\x89', "Переполнение по внесению в смене");
				add('\x8A', "Переполнение по надбавкам в чеке");
				add('\x8B', "Переполнение по скидкам в чеке");
				add('\x8C', "Отрицательный итог надбавки в чеке");
				add('\x8D', "Отрицательный итог скидки в чеке");
				add('\x8E', "Нулевой итог чека");
				add('\x90', "Поле превышает размер, установленный в настройках");
				add('\x91', "Выход за границу поля печати при данных настройках шрифта");
				add('\x92', "Наложение полей");
				add('\x93', "Восстановление ОЗУ прошло успешно");
				add('\x94', "Исчерпан лимит операций в чеке");
				add('\xC0', "Контроль даты и времени (подтвердите дату и время)");
				add('\xC2', "Превышение напряжения в блоке питания");
				add('\xC4', "Несовпадение номеров смен");
				add('\xC7', "Поле не редактируется в данном режиме");
				add('\xC8', "Нет связи с принтером или отсутствуют импульсы от таходатчика");
			}
		};
	}
}

//--------------------------------------------------------------------------------
