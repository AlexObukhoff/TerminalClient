/* @file Константы, коды команд и ответов базового протокола ФР АТОЛ (не онлайн). */

#pragma once

#include "../AtolFRConstants.h"

//--------------------------------------------------------------------------------
namespace CAtolFRBase
{
	/// Тип запроса ЭКЛЗ - итог документа.
	const char SessionResume = 0x01;

	/// Таймауты чтения, [мс].
	namespace Timeouts
	{
		/// Пауза между командами после ошибки ЭКЛЗ.
		const int EKLZErrorPause = 100;
	}

	/// Запросы ЭКЛЗ - некоторые, используемые.
	namespace EKLZRequests
	{
		const char GetStatus    = 0x07;    /// Запрос состояния ЭКЛЗ.
		const char GetRegNumber = 0x14;    /// Запрос регистрационного номера ЭКЛЗ.
	}

	/// Регистры
	namespace Registers
	{
		const char FreeReregistrations[]       = "free reregistrations";
		const char FreeFMSessions[]            = "free fiscal memory sessions";
		const char EKLZActivizationResources[] = "EKLZ activization resources";
		const char EKLZRegistrationData[]      = "EKLZ registration data";
		const char EKLZActivizationData[]      = "EKLZ activization data";
	}

	//------------------------------------------------------------------------------------------------
	/// Команды.
	namespace Commands
	{
		/// Команды получения информации об устройстве.
		const char EKLZRequest              = '\xAF';    /// Прямой запрос в ЭКЛЗ.
		const char GetSoftEKLZStatus        = '\xAE';    /// Мягкий запрос состояния ЭКЛЗ (но нет нужных статусов).
		const char PrintDeferredZReports    = '\xB5';    /// Печать отложенных Z отчетов.
		const char EnterDeferredZReportMode = '\xB4';    /// Вход в режим отложенных Z-отчетов.
	}

	//------------------------------------------------------------------------------------------------
	/// Ошибки.
	namespace Errors
	{
		/// Коды состояний ЭКЛЗ, возвращаемых в байте флагов на команду AFh.
		namespace EKLZ
		{
			const char NoError = '\x00';    /// Нет ошибок.
			const char Error   = '\x03';    /// Авария ЭКЛЗ.
			const char CCError = '\x04';    /// Авария КС (Криптографического сопроцессора) ЭКЛЗ.
			const char End     = '\x05';    /// Исчерпан временной ресурс.
			const char NearEnd = '\x80';    /// Исчерпан временной ресурс.
		}

		const char I2CInterface = '\xD2';    /// Ошибка обмена с ЭКЛЗ на уровне интерфейса I2C.
		const char EKLZOverflow = '\xD7';    /// Исчерпан временной ресурс ЭКЛЗ.

		class Data : public FRError::Data
		{
		public:
			Data()
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
				add('\x12', "Неверный штрих-код товара");
				add('\x13', "Неверный формат");
				add('\x14', "Неверная длина");
				add('\x15', "ККМ заблокирована в режиме ввода даты");
				add('\x16', "Требуется подтверждение ввода даты");
				add('\x18', "Нет больше данных для передачи ПО ККМ");
				add('\x19', "Нет подтверждения или отмены продажи");
				add('\x1A', "Отчет с гашением прерван. Вход в режим невозможен.");
				add('\x1B', "Отключение контроля наличности невозможно (не настроены необходимые типы оплаты).");
				add('\x1E', "Вход в режим заблокирован");
				add('\x1F', "Проверьте дату и время");
				add('\x20', "Дата и время в ККМ меньше чем в ЭКЛЗ");
				add('\x21', "Невозможно закрыть архив");
				add('\x3D', "Товар не найден");
				add('\x3E', "Весовой штрих-код с количеством != 1.000");
				add('\x3F', "Переполнение буфера чека");
				add('\x40', "Недостаточное количество товара");
				add('\x41', "Сторнируемое количество больше проданного");
				add('\x42', "Заблокированный товар не найден в буфере чека");
				add('\x43', "Данный товар не продавался в чеке, сторно невозможно");
				add('\x44', "Memo PlusTM 3TM заблокировано с ПК");
				add('\x45', "Ошибка контрольной суммы таблицы настроек Memo PlusTM 3TM");
				add('\x46', "Неверная команда от ККМ");
				add('\x66', "Команда не реализуется в данном режиме ККМ");
				add('\x67', "Нет бумаги");
				add('\x68', "Нет связи с принтером чеков");
				add('\x69', "Механическая ошибка печатающего устройства");
				add('\x6A', "Неверный тип чека");
				add('\x6B', "Нет больше строк картинки");
				add('\x6C', "Неверный номер регистра");
				add('\x6D', "Недопустимое целевое устройство");
				add('\x6E', "Нет места в массиве картинок");
				add('\x6F', "Неверный номер картинки или картинка отсутствует");
				add('\x70', "Сумма сторно больше, чем было получено данным типом оплаты");
				add('\x71', "Сумма не наличных платежей превышает сумму чека");
				add('\x72', "Сумма платежей меньше суммы чека");
				add('\x73', "Накопление меньше суммы возврата или аннулирования");
				add('\x75', "Переполнение суммы платежей");
				add('\x7A', "Данная модель ККМ не может выполнить команду");
				add('\x7B', "Неверная величина скидки/надбавки");
				add('\x7C', "Операция после скидки/надбавки невозможна");
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
				add('\x98', "В ККМ нет денег для выплаты");
				add('\x9A', "Чек закрыт – операция невозможна");
				add('\x9B', "Чек открыт – операция невозможна");
				add('\x9C', "Смена открыта, операция невозможна");
				add('\x9D', "ККМ заблокирована, ждет ввода пароля доступа к ФП");
				add('\x9E', "Заводской номер уже задан");
				add('\x9F', "Количество перерегистраций не может быть более 4");
				add('\xA0', "Ошибка Ф.П.");
				add('\xA2', "Неверная смена");
				add('\xA3', "Неверный тип отчета");
				add('\xA4', "Недопустимый пароль");
				add('\xA5', "Недопустимый заводской номер ККМ");
				add('\xA6', "Недопустимый РНМ");
				add('\xA7', "Недопустимый ИНН");
				add('\xA8', "ККМ не фискализирована");
				add('\xA9', "Не задан заводской номер");
				add('\xAA', "Нет отчетов");
				add('\xAB', "Режим не активизирован");
				add('\xAC', "Нет указанного чека в КЛ");
				add('\xAD', "Нет больше записей КЛ");
				add('\xAE', "Некорректный код или номер кода защиты ККМ");
				add('\xB0', "Требуется выполнение общего гашения");
				add('\xB1', "Команда не разрешена введенными кодами защиты ККМ");
				add('\xB2', "Невозможна отмена скидки/надбавки");
				add('\xB3', "Невозможно закрыть чек данным типом оплаты (в чеке присутствуют операции без контроля наличных)");
				add('\xBA', "Ошибка обмена с фискальным модулем");
				add('\xBe', "Необходимо провести профилактические работы");
				add('\xC8', "Нет устройства, обрабатывающего данную команду");
				add('\xC9', "Нет связи с внешним устройством");
				add('\xCA', "Ошибочное состояние ТРК");
				add('\xCB', "Больше одной регистрации в чеке");
				add('\xCC', "Ошибочный номер ТРК");
				add('\xCD', "Неверный делитель");
				add('\xCF', "В ККМ произведено 20 активизаций");
				add('\xD0', "Активизация данной ЭКЛЗ в составе данной ККМ невозможна");
				add('\xD1', "Перегрев головки принтера");
				add('\xD2', "Ошибка обмена с ЭКЛЗ на уровне интерфейса I2C");
				add('\xD3', "Ошибка формата передачи ЭКЛЗ");
				add('\xD4', "Неверное состояние ЭКЛЗ");
				add('\xD5', "Неисправимая ошибка ЭКЛЗ");
				add('\xD6', "Авария крипто-процессора ЭКЛЗ");
				add('\xD7', "Исчерпан временной ресурс ЭКЛЗ");
				add('\xD8', "ЭКЛЗ переполнена");
				add('\xD9', "В ЭКЛЗ переданы неверная дата или время");
				add('\xDA', "В ЭКЛЗ нет запрошенных данных");
				add('\xDB', "Переполнение ЭКЛЗ (итог чека)");
				add('\xDC', "Буфер переполнен");
				add('\xDD', "Невозможно напечатать вторую фискальную копию");
			}
		};
	}

	/// Структура для парсинга ответа на запросы ЭКЛЗ.
	struct SEKLZData
	{
		bool error;          /// Есть ли ошибка ЭКЛЗ.
		bool nearEnd;        /// ЭКЛЗ скоро кончится?
		qlonglong serial;    /// Серийный номер ЭКЛЗ.

		SEKLZData() : error(false), nearEnd(false), serial(0) {}
	};
}

//--------------------------------------------------------------------------------
