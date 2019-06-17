/* @file Константы, коды команд и ответов протокола ФР СПАРК. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDateTime>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/Specifications.h"
#include "Hardware/Common/ASCII.h"
#include "Hardware/Common/DeviceCodeSpecification.h"
#include "Hardware/Common/WaitingData.h"
#include "Hardware/Protocols/FR/FiscalChequeStates.h"

// Project
#include "Hardware/Printers/Tags.h"

//--------------------------------------------------------------------------------
namespace CSparkFR
{
	/// Таймауты, [мс].
	namespace Timeouts
	{
		/// Ответ на 1-байтовую команду.
		const int Control = 300;

		/// По умолчанию для нетяжелых команд.
		const int Default = 3 * 1000;

		/// Печать Z-отчета из буфера и снятие X-отчета
		const int Report = 5 * 1000;

		/// Cнятие Z-отчета
		const int ZReport = 30 * 1000;
	}

	/// Ожидание очистки эжектора, [мс].
	const SWaitingData EjectorWaiting = SWaitingData(300, 5 * 1000, true);

	/// Ожидание очистки эжектора, [мс].
	const SWaitingData PrintingWaiting = SWaitingData(300, 5 * 1000);

	/// Разделитель в ответе.
	const char Separator = '\x0A';

	/// Формат представления времени в ответе на длинный запрос статуса.
	const char TimeFormat[] = "hhmmss";

	/// Номер перерегистрации, если ЭКЛЗ не активирована.
	const char NoReregistrationNumber = '\xAF';

	/// Тип оплаты - наличные.
	const int CashPaymentType = 8;

	/// Без налогов.
	const char NoTaxes[] = "  ";

	/// Максимально возможное количество налоговых ставок.
	const int TaxRateCount = 4;

	/// Снимать отчет за текущую смену.
	const char SessionReport = '\xE0';

	/// Типы отчетов по функционалу.
	/// X-отчет.
	const char XReport = 'X';

	/// Z-отчет.
	const char ZReport = 'Z';

	/// Дата и время неоткрытой сессии.
	const QDateTime ClosedSession = QDateTime(QDate(1, 1, 1), QTime(0, 0));

	/// Управляющий пароль по умолчению.
	const int Password = 0;

	/// Z-отчёт выталкивается вперёд перед печатью следующего.
	const char PushZReport[] = "2";

	/// Системные флаги.
	namespace SystemFlags
	{
		const int ZReportsAndFiscal =  7;    /// Настройки Z-отчёта и чека.
		const int SystemOptions2    = 24;    /// Системные настройки-2.
	}

	/// Маска для длинных отчетов.
	const char LongReportMask = '\x1F';

	/// Номер кассира.
	const int CashierNumber = 1;

	/// Пароль кассира.
	const int CashierPassword = 11111;

	/// Бумага в презентере в ответе на запрос состояния датчиков.
	const char PaperInPresenter = '\x10';

	/// Максимальное количество символов в строке.
	const int LineSize = 40;

	/// Флаг истекшей смены.
	const char SessionExpired = '\x20';

	/// Коды команд.
	namespace Commands
	{
		/// Команды получения информации об устройстве.
		const char ENQT[] = "\x1A";    /// Объединённый статус

		const char GetFWVersion[]         =    "SV";    /// Версия прошивки
		const char KKMInfo[]              =    "S1";    /// Информация о ресурсах и настройках
		const char EKLZInfo[]             =   "S\"";    /// Информация об ЭКЛЗ
		const char GetEKLZError[]         =    "S!";    /// Номер фискализации и код фатальной ошибки 
		const char ZBufferSpace[]         =    "SA";    /// Инфо о свободном/занятом месте в Z-буфере
		const char ZBufferCount[]         =   "SAA";    /// Количество Z-отчетов в Z-буфере
		const char PrintLine[]            =    "PP";    /// Печать строки 
		const char Cut[]                  =   "Pp0";    /// Отрезка
		const char TaxesAndFlags[]        =    "S3";    /// Налоги и системные флаги
		const char SetFlag[]              =    "PJ";    /// Установка системного флага
		const char Payin[]                =    "91";    /// Внесение
		const char ClosePayIO[]           =     "t";    /// Закрыть внесение/выплату.
		const char Payout[]               =    "90";    /// Выплата
		const char Sale0[]                =     " ";    /// Продажа без налогов
		const char Sale1[]                =     "!";    /// Продажа c налогом 1
		const char Sale2[]                =    "\"";    /// Продажа c налогом 2
		const char Sale3[]                =     "#";    /// Продажа c налогом 3
		const char Sale4[]                =     "$";    /// Продажа c налогом 4
		const char SetTaxes[]             =    "PT";    /// Установить налоги
		const char AcceptTaxes[]          =    "Pt";    /// Подтвердить налоги
		const char CloseFiscal[]          =     "1";    /// Закрыть фискальный чек.
		const char CancelFiscal[]         =     "7";    /// Аннулировать фискальный чек.
		const char Reports[]              =     "I";    /// Снять отчет.
		const char PrintZBuffer[]         =  "PALL";    /// Печать буфера Z-отчётов.
		const char EnterPassword[]        =   "W  ";    /// Ввести управляющий пароль.
		const char GetTotal[]             =    "Sg";    /// Получить накопительные итогах.
		const char GetCashAcceptorTotal[] = "S|bf3";    /// Получить количество денег в кассете купюроприёмника.
		const char Push[]                 =  "S|-1";    /// Вытолкнуть вперед.
		const char Retract[]              =  "S|-0";    /// Вытолкнуть назад.
		const char OpenKKM[]              =     "o";    /// Открыть систему.
		const char CloseKKM[]             =    "zz";    /// Закрыть систему.
		const char SetCashier[]           =     "5";    /// Установить кассира.
		const char GetSensorState[]       =   "S|R";    /// Получить состояние датчиков.
		const char SetTextProperty[]      =    "P@";    /// Установить текстовый реквизит.
		const char SetTextPropertyName[]  =    "PL";    /// Установить наименование текстового реквизита.
		const char GetTextPropertyName[]  =    "SY";    /// Получить наименование текстового реквизита.

		struct SData
		{
			QByteArray answer;    /// Код команды в ответе. Может отличаться от кода команды в запросе.
			bool sending;         /// Отправляющая/получающая команда.
			bool password;        /// Нужен ввод пароля управления ККТ.

			SData() : sending(false), password(false) {}
			SData(const QByteArray & aAnswer, bool aSending, bool aPassword): answer(aAnswer), sending(aSending), password(aPassword) {}
		};

		class CData : public CSpecification<QByteArray, SData>
		{
		public:
			CData()
			{
				set(GetFWVersion, false);
				set(KKMInfo, false);
				add(EKLZInfo, "");
				set(GetEKLZError, false);
				set(ZBufferSpace, false);
				add(ZBufferCount, "SA");
				set(TaxesAndFlags, false);
				set(GetTotal, false);
				set(GetCashAcceptorTotal, false);
				set(GetSensorState, false);

				set(PrintLine, true);
				set(Cut, true);
				set(SetFlag, true);
				set(Payin, true);
				set(Payout, true);
				set(Sale0, true);
				set(Sale1, true);
				set(Sale2, true);
				set(Sale3, true);
				set(Sale4, true);
				set(CloseFiscal, true);
				set(Reports, true);
				add(PrintZBuffer, "", true, true);
				set(EnterPassword, true);
				set(Push, true);
				set(Retract, true);
				set(OpenKKM, true, true);
				set(CloseKKM, true, true);
				set(SetCashier, true);
				set(SetTextProperty, true);
			}

			virtual SData value(const QByteArray & aCommand) const
			{
				return mBuffer.contains(aCommand) ? mBuffer[aCommand] : SData(aCommand, true, false);
			}

		private:
			void add(const QByteArray & aCommand, const QByteArray & aAnswer, bool aSending = false, bool aPassword = false)
			{
				append(aCommand, SData(aAnswer, aSending, aPassword));
			}

			void set(const QByteArray & aCommand, bool aSending, bool aPassword = false)
			{
				append(aCommand, SData(aCommand, aSending, aPassword));
			}

			void set(char aCommand, bool aSending = false)
			{
				append(QByteArray(1, aCommand), SData("", aSending, false));
			}
		};

		static CData Data;
	}

	//--------------------------------------------------------------------------------
	/// Состояния открытого документа (некотрые, используемые).
	namespace DocumentStates
	{
		const char Closed   = '0';    /// Нет открытого документа.
		const char Sale     = '1';    /// Регистрация продаж.
		const char Closing  = '2';    /// Ввод оплат при завершении чека.
		const char PayIO    = '3';    /// Внесение/выплата из кассы.
		const char Printing = '4';    /// Режим печати текста.
	}

	//--------------------------------------------------------------------------------
	/// Ошибки.
	namespace Errors
	{
		const char KKMClosed     = 43;    /// ККТ закрыта
		const char CashierNotSet = 46;    /// Кассир не установлен
		const char NeedZReport   = 63;    /// Не снят Z-отчёт
		const char KKMOpened     = 68;    /// ККТ открыта
		const char TimeOff       = 86;    /// Не снят Z-отчёт
		const char NeedSaleOnly  = 91;    /// Нужна только продажа
		const char NeedPayIOOnly = 93;    /// Нужна только выплата/внесение
		const char WrongTextModeCommand = 94;    /// Неверная команда режима печати текста

		class Data : public FRError::Data
		{
		public:
			Data()
			{
				addData(  1, "Некорректный формат или параметр команды");
				addData(  2, "Некорректное состояние ЭКЛЗ");
				addData(  3, "Авария ЭКЛЗ");
				addData(  4, "Авария КС");
				addData(  5, "Исчерпан временной ресурс использования ЭКЛЗ");
				addData(  6, "ЭКЛЗ переполнена");
				addData(  7, "Неверные дата или время");
				addData(  8, "Нет запрошенных данных (используется только для внутренних нужд ККТ)");
				addData(  9, "Переполнение");
				addData( 13, "Ошибка ЭКЛЗ (другая)");
				addData( 16, "Нет ответа от ЭКЛЗ");
				addData( 17, "Чужая ЭКЛЗ");
				addData( 18, "Исчерпан лимит активизаций ЭКЛЗ в ФП");
				addData( 19, "Ошибка при записи в ФП при активизации ЭКЛЗ");
				addData( 20, "Неверная длина ответа от ЭКЛЗ");
				addData( 21, "Исчерпан лимит перерегистраций");
				addData( 22, "Активная чужая ЭКЛЗ");
				addData( 23, "Переполнение при упаковке данных");
				addData( 24, "Нет открытых ЭКЛЗ по данным ФП");
				addData( 25, "ЭКЛЗ уже закрыта");
				addData( 26, "Запрещенное состояние ЭКЛЗ");
				addData( 27, "ЭКЛЗ открыта");
				addData( 28, "Нет КПК в ответе ЭКЛЗ");
				addData( 29, "ЭКЛЗ содержит дефектные данные");
				addData( 30, "ОЗУ и ЭКЛЗ имеют разные данные");
				addData( 31, "Время в ОЗУ меньше, чем в ЭКЛЗ");
				addData( 32, "Предыдущая ЭКЛЗ не была закрыта");
				addData( 33, "ЭКЛЗ закрыта по данным ФП");
				addData( 34, "ЭКЛЗ не активизирована");
				addData( 35, "В ФП противоречивые данные");
				addData( 36, "Расхождение номеров смен между ОЗУ и ЭКЛЗ");
				addData( 37, "ФП содержит постороннюю информацию");
				addData( 38, "Исчерпан ресурс ФП");
				addData( 39, "Запись в ФП не удалась при активизации/закрытии архива ЭКЛЗ");
				addData( 40, "Неверная длина команды");
				addData( 41, "Пароль не введен или неверный");
				addData( 42, "Переполнение, в т.ч. отрицательное значение");
				addData( 43, "ККТ закрыта");
				addData( 44, "В транзакции");
				addData( 45, "Нечисловая информация");
				addData( 46, "Кассир не установлен");
				addData( 47, "Неверный тип документа");
				addData( 48, "Переполнение количества строк");
				addData( 49, "Синтаксис команды неверный");
				addData( 50, "Подфункция не найдена");
				addData( 51, "Неверное платежное средство");
				addData( 52, "Запрещено для этого платежного средства");
				addData( 53, "Неверный номер отдела");
				addData( 54, "Запрещено по программируемому флагу №5");
				addData( 55, "Не введены номер кредитной карты и документ авторизации");
				addData( 56, "Знак числа неверный");
				addData( 57, "Строка содержит непечатный символ");
				addData( 58, "Пустая строка");
				addData( 59, "Дата не установлена");
				addData( 60, "Дата меньше чем дата последнего Z-отчёта");
				addData( 61, "Регистрация невозможна");
				addData( 62, "Конец бумаги");
				addData( 63, "Не снят Z-отчёт");
				addData( 64, "Не снят Z2 отчёт");
				addData( 65, "Исчерпан лимит записей в ФП");
				addData( 66, "Пароль для доступа к ФП не запрограммирован");
				addData( 67, "Пароль для нормальной работы не запрограммирован");
				addData( 68, "ККТ открыта");
				addData( 69, "Параметр уже запрограммирован");
				addData( 70, "Два последних символа должны быть пробелами");
				addData( 71, "ЭКЛЗ не выходит из режима отчётов");
				addData( 72, "Регистрационный номер не запрограммирован");
				addData( 73, "Строка содержит запрещённые символы в имени файла");
				addData( 74, "Фиск режим уже запущен");
				addData( 75, "Этот пароль имеется у другого кассира");
				addData( 76, "Дата превышает предыдущую на день");
				addData( 77, "Ввод даты не подтвердился");
				addData( 78, "В фискальном режиме запрещено");
				addData( 79, "В нефискальном режиме запрещено");
				addData( 80, "Информация уже занесена в ФП");
				addData( 81, "Нет учетной записи в чеке");
				addData( 82, "Ошибка обмена с ПК");
				addData( 83, "Коррекция на некорректируемую операцию");
				addData( 84, "Исчерпан список пл. средств");
				addData( 85, "Испорчена дата последнего отчёта");
				addData( 86, "Исчерпано время");
				addData( 87, "Испорчена таблица в памяти (fstatus)");
				addData( 88, "Ошибка записи в ФП");
				addData( 89, "Значение выходит за пределы допустимого");
				addData( 90, "Неверная команда вне транзакции");
				addData( 91, "Неверная команда регистрации продаж");
				addData( 92, "Неверная команда ввода оплат");
				addData( 93, "Неверная команда внесения/выплаты из кассы");
				addData( 94, "Неверная команда режима печати текста");
				addData( 95, "Неверная команда регистрации аннулирования");
				addData( 96, "Неверная команда ввода сумм выдачи при аннулировании");
				addData( 97, "Неверная команда неопознанного режима");
				addData( 98, "Запрещено программирование этого дескриптора");
				addData( 99, "Не поддерживается обработка этой ситуации");
				addData(100, "Запрос не найден");
				addData(101, "Ссылка на незапрограммированную ставку");
				addData(102, "Не введен кнопочный пароль");
				addData(103, "Ввод запрещенного слова");
				addData(104, "Чек с нулевым итогом запрещен. Выполнена аварийная отмена чека");
				addData(105, "Вводимая дата меньше даты последнего документа в ЭКЛЗ");
				addData(106, "Фискализация не доведена до конца");
				addData(107, "Поднят рычаг принтера");
				addData(108, "Печатающее устройство не обнаружено");
				addData(109, "Регистрация невозможна");
				addData(110, "Фискальная память не подключена");
				addData(111, "ККТ закрыта и/или кассир не установлен");
				addData(112, "Номер кассы не запрограммирован");
				addData(113, "При включённых дипах не работают основные функции");
				addData(114, "Заводской номер не запрограммирован");
				addData(115, "ККТ заблокирован при вводе неверного пароля ФП");
				addData(116, "Должны быть готовы оба принтера. Остальные биты запрограммированы");
				addData(117, "Запрещено по программируемому флагу №9");
				addData(118, "Запрещено по программируемому флагу №10");
				addData(119, "ИНН не запрограммирован");
				addData(120, "Переполнение стека в режиме 0");
				addData(121, "Переполнение стека в режиме 1");
				addData(122, "Переполнение стека в режиме 2");
				addData(123, "Переполнение стека в режиме 3");
				addData(124, "Переполнение стека в режиме 4");
				addData(125, "Переполнение стека в режиме 5");
				addData(126, "Переполнение стека в режиме 6");
				addData(127, "Переполнение стека в неопознанном режиме");
				addData(128, "Ввод нулевого количества запрещен");
				addData(129, "Cбой ОЗУ");
				addData(130, "Нет итогов смены в ЭКЛЗ по запросу");
				addData(131, "Частичное переполнение, в т.ч. отрицательное значение");
				addData(132, "Открыт денежный ящик");
				addData(133, "Чек с нулевым итогом запрещен. Невозможно выполнить аварийную отмену чека");
				addData(134, "Разные итоги документа в ОЗУ и ЭКЛЗ. Выполнена аварийная отмена чека");
				addData(135, "Разные итоги документа в ОЗУ и ЭКЛЗ. Невозможно выполнить аварийную отмену чека");
				addData(136, "ЭКЛЗ содержит дефектные данные. Выполнена аварийная отмена чека");
				addData(137, "ЭКЛЗ содержит дефектные данные. Невозможно выполнить аварийную отмену чека");
				addData(138, "Не хватает денег в кассе для сдачи");
				addData(139, "Нет ни одного наличного платёжного средства, из которого можно было бы выдать сдачу");
				addData(140, "Не хватает денег по платёжному средству для сдачи");
				addData(141, "Не хватает денег по 2 платёжному средству для сдачи");
				addData(142, "Не хватает денег по 3 платёжному средству для сдачи");
				addData(143, "Не хватает денег по 4 платёжному средству для сдачи");
				addData(144, "Не хватает денег по 5 платёжному средству для сдачи");
				addData(145, "Не хватает денег по 6 платёжному средству для сдачи");
				addData(146, "Не хватает денег по 7 платёжному средству для сдачи");
				addData(147, "Не хватает денег по 8 платёжному средству для сдачи");
				addData(148, "Ширина форматированной строки ЭКЛЗ не соответствует типу ККТ");
				addData(149, "ЭКЛЗ активизирована на другом типе ККТ");
				addData(150, "Отчёт по активизации ЭКЛЗ укорочен");
				addData(151, "Нет отчёта по активизации ЭКЛЗ");
				addData(152, "Отчёт по активизации ЭКЛЗ слишком длинный");
				addData(153, "Дата фискализации уже записана в ФП");
				addData(154, "Перерегистрация запрещена с незакрытой ЭКЛЗ");
				addData(155, "Серийные номера в ФП и в ЭКЛЗ не совпадают");
				addData(156, "ИНН и/или номер смены в ФП и в ЭКЛЗ не совпадают");
				addData(157, "Регистрационные номера в ФП и в ЭКЛЗ не совпадают");
				addData(158, "ФП содержит информацию о регистрациях ЭКЛЗ");
				addData(159, "Смена в ЭКЛЗ должна быть закрыта");
				addData(160, "Номер смены в ЭКЛЗ слишком велик");
				addData(161, "Биты 4..6 флага 4 защищены PF13:0 от модификации или бит 5 флага 3 защищен от сброса");
				addData(162, "Переполнение регистрационного буфера");
				addData(163, "Подфункция 2-го порядка не найдена");
				addData(164, "Программирование нулевого пароля запрещено");
				addData(165, "Ненулевой остаток в кассе");
				addData(166, "Отключение фискальной памяти");
				addData(167, "Напряжение на резервной батарее питания ниже допустимого");
				addData(168, "Глубина дерева налогов слишком велика, дерево содержит цикл или обратную ссылку");
				addData(169, "Предупреждение: ставки налогов во включенном режиме цепочек не программируются");
				addData(170, "Неверный номер ставки в дереве налогов");
				addData(171, "Дерево налогов не запрограммировано");
				addData(172, "Дерево налогов состоит более, чем из одной компоненты");
				addData(173, "Режим налоговых цепочек не включён");
				addData(174, "Пароль доступа к ФП уже запрограммирован");
				addData(175, "Пароль нормальной работы уже запрограммирован");
				addData(176, "ИНН уже запрограммирован");
				addData(177, "Серийный номер уже запрограммирован");
				addData(178, "Регистрационный номер уже запрограммирован");
				addData(179, "Заголовок чека не запрограммирован");
				addData(180, "Выполнение отчёта прервано");
				addData(181, "ККТ заблокирована для перерегистрации");
				addData(182, "Невозможно закрыть смену в ЭКЛЗ");
				addData(183, "Невозможно снять запрос состояния тип 2");
				addData(184, "Не заданы параметры вводимого графического заголовка");
				addData(185, "Ввод графического заголовка не доведен до конца");
				addData(186, "Размер этикетки с номером >31 не должен превышать 3*24");
				addData(187, "Ложное выключение питания");
				addData(188, "Некорректная версия ПЗУ по данным ФП");
				addData(189, "Ссылка на незапрограммированный код валюты");
				addData(190, "Базовая валюта должна иметь имя");
				addData(191, "Ссылка на налог с незапрограммированным именем");
				addData(192, "Процедура подготовки к выключению питания не была завершена");
				addData(193, "Расхождение данных между ОЗУ и ФП");
				addData(194, "При выключенных DIP запрещено");
				addData(195, "Дата меньше, чем дата фискализации");
				addData(196, "Сбой ЭСПЗУ");
				addData(197, "Время не установлено");
				addData(198, "Номер фискального документа> количества документов");
				addData(199, "Неверная команда режима продолжения печати подкладного документа");
				addData(200, "Нулевое количество и цена в итоге чека");
				addData(201, "По ставке налогов покупка не выполняется");
				addData(202, "Операции с покупкой по карте запрещены");
				addData(203, "Переплата сдачи клиенту");
				addData(204, "Программирование небазовой валюты при пустой базовой запрещено");
				addData(205, "Не все небазовые валюты обнулены");
				addData(206, "Попытка обнулить текущую небазовую валюту");
				addData(207, "Сдача по карте запрещена");
				addData(208, "Предупреждение: установлено только одно наличное платежное средство №8");
				addData(209, "Неверная контрольная сумма ПЗУ");
				addData(210, "Допустимые команды: Либо завершение чека, либо отмена");
				addData(211, "Чек не сформирован");
				addData(212, "Неверная контрольная сумма серийного номера");
				addData(213, "Неверная контрольная сумма фиск. Реквизитов");
				addData(214, "Количество значащих символов меньше трёх");
				addData(215, "Скорость обмена с ПК должна превышать скорость обмена с принтером");
				addData(216, "Нет ответа от фискального модуля");
				addData(217, "Неверный ответ от фискального модуля");
				addData(218, "Неверная контр. сумма фискального модуля");
				addData(219, "Кол-во пропавших записей в ФП>2. ККТ заблокирована");
				addData(220, "Неверная контрольная сумма регистрации ЭКЛЗ");
				addData(221, "Неверная контрольная сумма регистраций очистки ОЗУ");
				addData(222, "Дата меньше даты активизации ЭКЛЗ");
				addData(223, "Переполнен буфер Z-отчётов, текущая смена закрыта");
				addData(224, "Не очищен буфер Z-отчётов, текущая смена не закрыта");
				addData(225, "Не распечатан буфер Z-отчётов");
				addData(226, "Близок конец бумаги");
				addData(227, "Переполнение буфера реквизитов");
				addData(228, "Технологическая ПЗУ");
				addData(229, "Данные не получены от фискального модуля");
				addData(230, "Нет ответа от SRAM");
				addData(231, "Невозможно очистить буфер USART1");
				addData(232, "Некорректное значение стека после выключения питания");
				addData(233, "Принтер не готов");
				addData(234, "EEPROM не запрограммирована");
				addData(235, "Примечание: изменения вступят в силу после выключения питания");
				addData(236, "Купюроприёмник отключён");
				addData(237, "Невозможно включить приём купюр");
				addData(238, "Невозможно отключить приём купюр");
				addData(239, "Запрещено в выключенном режиме приёма купюр");
				addData(240, "Запрещено во включённом режиме приёма купюр");
				addData(241, "Превышение итога документа");
				addData(242, "Пустая таблица деноминации в купюроприёмнике");
				addData(243, "Запрещено по программируемому флагу 28");
			}

			private:
				void addData(uchar aKey, const char * aDescription)
				{
					add(char(aKey), aDescription);
				}
		};
	}

	//--------------------------------------------------------------------------------
	/// Спецификация статусов.
	class CStatus1 : public BitmapDeviceCodeSpecification
	{
	public:
		CStatus1()
		{
			addStatus( 2, DeviceStatusCode::OK::Busy);
			addStatus( 3, FRStatusCode::Error::FM);
			addStatus( 4, FRStatusCode::Warning::FiscalMemoryNearEnd);
			addStatus( 6, DeviceStatusCode::Warning::Unknown, "", true);
			addStatus( 7, DeviceStatusCode::Warning::Unknown);
		}
	};

	//--------------------------------------------------------------------------------
	class CStatus2 : public BitmapDeviceCodeSpecification
	{
	public:
		CStatus2()
		{
			addStatus( 0, PrinterStatusCode::Error::PaperEnd);
			addStatus( 1, PrinterStatusCode::Error::PrinterFR);
			addStatus( 6, DeviceStatusCode::Warning::Unknown, "", true);
			addStatus( 7, DeviceStatusCode::Warning::Unknown);

			mErrors.addStatus('\x2C', FRStatusCode::Error::FM);
			mErrors.addStatus('\x24', FRStatusCode::Error::FM);
			mErrors.addStatus('\x20', DeviceStatusCode::Warning::OperationError);
			mErrors.addStatus('\x1C', DeviceStatusCode::Warning::OperationError);
			mErrors.addStatus('\x18', DeviceStatusCode::Warning::OperationError);
			mErrors.addStatus('\x14', DeviceStatusCode::Warning::OperationError);
			mErrors.addStatus('\x10', DeviceStatusCode::Warning::OperationError);
			mErrors.addStatus('\x30', DeviceStatusCode::Warning::OperationError);
			mErrors.addStatus('\x08', PrinterStatusCode::Error::PaperEnd);
			mErrors.addStatus('\x0C', PrinterStatusCode::Error::PaperEnd);
		}

		/// Получить спецификации девайс-кодов по байт-массиву. байт-массив не должен содержать лишних байтов перед статусными байтами.
		virtual void getSpecification(char aAnswerData, TStatusCodes & aStatusCodes)
		{
			BitmapDeviceCodeSpecification::getSpecification(aAnswerData, aStatusCodes);

			mErrors.getSpecification(aAnswerData & '\x3C', aStatusCodes);
		}

	protected:
		/// Спецификация дополнительных девайс-кодов.
		CommonDeviceCodeSpecification mErrors;
	};

	//--------------------------------------------------------------------------------
	class CStatus3 : public BitmapDeviceCodeSpecification
	{
	public:
		CStatus3()
		{
			addStatus( 1, PrinterStatusCode::Warning::PaperNearEnd);
			addStatus( 3, FRStatusCode::Error::EKLZ);
			addStatus( 6, DeviceStatusCode::Warning::Unknown, "", true);
			addStatus( 7, DeviceStatusCode::Warning::Unknown);
		}
	};

	static CStatus1 Status1;
	static CStatus2 Status2;
	static CStatus3 Status3;

	//--------------------------------------------------------------------------------
	/// Теги.
	class TagEngine : public Tags::Engine
	{
	public:
		TagEngine()
		{
			appendSingle(Tags::Type::DoubleWidth, "", "\x7F");
			set(Tags::Type::DoubleHeight);
			set(Tags::Type::UnderLine);
		}
	};

	/// Теги для обработки постфиксом команды.
	namespace Tag
	{
		const char DoubleHeight = 2;
		const char UnderLine    = 1;
	}
}

//--------------------------------------------------------------------------------
