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

	/// Системный флаг системных настроек-2.
	const int SystemOptions2 = 24;

	/// Номер кассира.
	const int CashierNumber = 1;

	/// Пароль кассира.
	const int CashierPassword = 11111;

	/// Бумага в презентере в ответе на запрос состояния датчиков.
	const char PaperInPresenter = '\x10';

	/// Максимальное количество символов в строке.
	const int LineSize = 40;

	/// Коды команд.
	namespace Commands
	{
		/// Команды получения информации об устройстве.
		const char ENQT = '\x1A';    /// Объединённый статус

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

		class CDescriptions : public CDescription<uchar>
		{
		public:
			CDescriptions()
			{
				setDefault("Неизвестная");

				append(1,   "Некорректный формат или параметр команды");
				append(2,   "Некорректное состояние ЭКЛЗ");
				append(3,   "Авария ЭКЛЗ");
				append(4,   "Авария КС");
				append(5,   "Исчерпан временной ресурс использования ЭКЛЗ");
				append(6,   "ЭКЛЗ переполнена");
				append(7,   "Неверные дата или время");
				append(8,   "Нет запрошенных данных (используется только для внутренних нужд ККТ)");
				append(9,   "Переполнение");
				append(13,  "Ошибка ЭКЛЗ (другая)");
				append(16,  "Нет ответа от ЭКЛЗ");
				append(17,  "Чужая ЭКЛЗ");
				append(18,  "Исчерпан лимит активизаций ЭКЛЗ в ФП");
				append(19,  "Ошибка при записи в ФП при активизации ЭКЛЗ");
				append(20,  "Неверная длина ответа от ЭКЛЗ");
				append(21,  "Исчерпан лимит перерегистраций");
				append(22,  "Активная чужая ЭКЛЗ");
				append(23,  "Переполнение при упаковке данных");
				append(24,  "Нет открытых ЭКЛЗ по данным ФП");
				append(25,  "ЭКЛЗ уже закрыта");
				append(26,  "Запрещенное состояние ЭКЛЗ");
				append(27,  "ЭКЛЗ открыта");
				append(28,  "Нет КПК в ответе ЭКЛЗ");
				append(29,  "ЭКЛЗ содержит дефектные данные");
				append(30,  "ОЗУ и ЭКЛЗ имеют разные данные");
				append(31,  "Время в ОЗУ меньше, чем в ЭКЛЗ");
				append(32,  "Предыдущая ЭКЛЗ не была закрыта");
				append(33,  "ЭКЛЗ закрыта по данным ФП");
				append(34,  "ЭКЛЗ не активизирована");
				append(35,  "В ФП противоречивые данные");
				append(36,  "Расхождение номеров смен между ОЗУ и ЭКЛЗ");
				append(37,  "ФП содержит постороннюю информацию");
				append(38,  "Исчерпан ресурс ФП");
				append(39,  "Запись в ФП не удалась при активизации/закрытии архива ЭКЛЗ");
				append(40,  "Неверная длина команды");
				append(41,  "Пароль не введен или неверный");
				append(42,  "Переполнение, в т.ч. отрицательное значение");
				append(43,  "ККТ закрыта");
				append(44,  "В транзакции");
				append(45,  "Нечисловая информация");
				append(46,  "Кассир не установлен");
				append(47,  "Неверный тип документа");
				append(48,  "Переполнение количества строк");
				append(49,  "Синтаксис команды неверный");
				append(50,  "Подфункция не найдена");
				append(51,  "Неверное платежное средство");
				append(52,  "Запрещено для этого платежного средства");
				append(53,  "Неверный номер отдела");
				append(54,  "Запрещено по программируемому флагу №5");
				append(55,  "Не введены номер кредитной карты и документ авторизации");
				append(56,  "Знак числа неверный");
				append(57,  "Строка содержит непечатный символ");
				append(58,  "Пустая строка");
				append(59,  "Дата не установлена");
				append(60,  "Дата меньше чем дата последнего Z-отчёта");
				append(61,  "Регистрация невозможна");
				append(62,  "Конец бумаги");
				append(63,  "Не снят Z-отчёт");
				append(64,  "Не снят Z2 отчёт");
				append(65,  "Исчерпан лимит записей в ФП");
				append(66,  "Пароль для доступа к ФП не запрограммирован");
				append(67,  "Пароль для нормальной работы не запрограммирован");
				append(68,  "ККТ открыта");
				append(69,  "Параметр уже запрограммирован");
				append(70,  "Два последних символа должны быть пробелами");
				append(71,  "ЭКЛЗ не выходит из режима отчётов");
				append(72,  "Регистрационный номер не запрограммирован");
				append(73,  "Строка содержит запрещённые символы в имени файла");
				append(74,  "Фиск режим уже запущен");
				append(75,  "Этот пароль имеется у другого кассира");
				append(76,  "Дата превышает предыдущую на день");
				append(77,  "Ввод даты не подтвердился");
				append(78,  "В фискальном режиме запрещено");
				append(79,  "В нефискальном режиме запрещено");
				append(80,  "Информация уже занесена в ФП");
				append(81,  "Нет учетной записи в чеке");
				append(82,  "Ошибка обмена с ПК");
				append(83,  "Коррекция на некорректируемую операцию");
				append(84,  "Исчерпан список пл. средств");
				append(85,  "Испорчена дата последнего отчёта");
				append(86,  "Исчерпано время");
				append(87,  "Испорчена таблица в памяти (fstatus)");
				append(88,  "Ошибка записи в ФП");
				append(89,  "Значение выходит за пределы допустимого");
				append(90,  "Неверная команда вне транзакции");
				append(91,  "Неверная команда регистрации продаж");
				append(92,  "Неверная команда ввода оплат");
				append(93,  "Неверная команда внесения/выплаты из кассы");
				append(94,  "Неверная команда режима печати текста");
				append(95,  "Неверная команда регистрации аннулирования");
				append(96,  "Неверная команда ввода сумм выдачи при аннулировании");
				append(97,  "Неверная команда неопознанного режима");
				append(98,  "Запрещено программирование этого дескриптора");
				append(99,  "Не поддерживается обработка этой ситуации");
				append(100, "Запрос не найден");
				append(101, "Ссылка на незапрограммированную ставку");
				append(102, "Не введен кнопочный пароль");
				append(103, "Ввод запрещенного слова");
				append(104, "Чек с нулевым итогом запрещен. Выполнена аварийная отмена чека");
				append(105, "Вводимая дата меньше даты последнего документа в ЭКЛЗ");
				append(106, "Фискализация не доведена до конца");
				append(107, "Поднят рычаг принтера");
				append(108, "Печатающее устройство не обнаружено");
				append(109, "Регистрация невозможна");
				append(110, "Фискальная память не подключена");
				append(111, "ККТ закрыта и/или кассир не установлен");
				append(112, "Номер кассы не запрограммирован");
				append(113, "При включённых дипах не работают основные функции");
				append(114, "Заводской номер не запрограммирован");
				append(115, "ККТ заблокирован при вводе неверного пароля ФП");
				append(116, "Должны быть готовы оба принтера. Остальные биты запрограммированы");
				append(117, "Запрещено по программируемому флагу №9");
				append(118, "Запрещено по программируемому флагу №10");
				append(119, "ИНН не запрограммирован");
				append(120, "Переполнение стека в режиме 0");
				append(121, "Переполнение стека в режиме 1");
				append(122, "Переполнение стека в режиме 2");
				append(123, "Переполнение стека в режиме 3");
				append(124, "Переполнение стека в режиме 4");
				append(125, "Переполнение стека в режиме 5");
				append(126, "Переполнение стека в режиме 6");
				append(127, "Переполнение стека в неопознанном режиме");
				append(128, "Ввод нулевого количества запрещен");
				append(129, "Cбой ОЗУ");
				append(130, "Нет итогов смены в ЭКЛЗ по запросу");
				append(131, "Частичное переполнение, в т.ч. отрицательное значение");
				append(132, "Открыт денежный ящик");
				append(133, "Чек с нулевым итогом запрещен. Невозможно выполнить аварийную отмену чека");
				append(134, "Разные итоги документа в ОЗУ и ЭКЛЗ. Выполнена аварийная отмена чека");
				append(135, "Разные итоги документа в ОЗУ и ЭКЛЗ. Невозможно выполнить аварийную отмену чека");
				append(136, "ЭКЛЗ содержит дефектные данные. Выполнена аварийная отмена чека");
				append(137, "ЭКЛЗ содержит дефектные данные. Невозможно выполнить аварийную отмену чека");
				append(138, "Не хватает денег в кассе для сдачи");
				append(139, "Нет ни одного наличного платёжного средства, из которого можно было бы выдать сдачу");
				append(140, "Не хватает денег по платёжному средству для сдачи");
				append(141, "Не хватает денег по 2 платёжному средству для сдачи");
				append(142, "Не хватает денег по 3 платёжному средству для сдачи");
				append(143, "Не хватает денег по 4 платёжному средству для сдачи");
				append(144, "Не хватает денег по 5 платёжному средству для сдачи");
				append(145, "Не хватает денег по 6 платёжному средству для сдачи");
				append(146, "Не хватает денег по 7 платёжному средству для сдачи");
				append(147, "Не хватает денег по 8 платёжному средству для сдачи");
				append(148, "Ширина форматированной строки ЭКЛЗ не соответствует типу ККТ");
				append(149, "ЭКЛЗ активизирована на другом типе ККТ");
				append(150, "Отчёт по активизации ЭКЛЗ укорочен");
				append(151, "Нет отчёта по активизации ЭКЛЗ");
				append(152, "Отчёт по активизации ЭКЛЗ слишком длинный");
				append(153, "Дата фискализации уже записана в ФП");
				append(154, "Перерегистрация запрещена с незакрытой ЭКЛЗ");
				append(155, "Серийные номера в ФП и в ЭКЛЗ не совпадают");
				append(156, "ИНН и/или номер смены в ФП и в ЭКЛЗ не совпадают");
				append(157, "Регистрационные номера в ФП и в ЭКЛЗ не совпадают");
				append(158, "ФП содержит информацию о регистрациях ЭКЛЗ");
				append(159, "Смена в ЭКЛЗ должна быть закрыта");
				append(160, "Номер смены в ЭКЛЗ слишком велик");
				append(161, "Биты 4..6 флага 4 защищены PF13:0 от модификации или бит 5 флага 3 защищен от сброса");
				append(162, "Переполнение регистрационного буфера");
				append(163, "Подфункция 2-го порядка не найдена");
				append(164, "Программирование нулевого пароля запрещено");
				append(165, "Ненулевой остаток в кассе");
				append(166, "Отключение фискальной памяти");
				append(167, "Напряжение на резервной батарее питания ниже допустимого");
				append(168, "Глубина дерева налогов слишком велика, дерево содержит цикл или обратную ссылку");
				append(169, "Предупреждение: ставки налогов во включенном режиме цепочек не программируются");
				append(170, "Неверный номер ставки в дереве налогов");
				append(171, "Дерево налогов не запрограммировано");
				append(172, "Дерево налогов состоит более, чем из одной компоненты");
				append(173, "Режим налоговых цепочек не включён");
				append(174, "Пароль доступа к ФП уже запрограммирован");
				append(175, "Пароль нормальной работы уже запрограммирован");
				append(176, "ИНН уже запрограммирован");
				append(177, "Серийный номер уже запрограммирован");
				append(178, "Регистрационный номер уже запрограммирован");
				append(179, "Заголовок чека не запрограммирован");
				append(180, "Выполнение отчёта прервано");
				append(181, "ККТ заблокирована для перерегистрации");
				append(182, "Невозможно закрыть смену в ЭКЛЗ");
				append(183, "Невозможно снять запрос состояния тип 2");
				append(184, "Не заданы параметры вводимого графического заголовка");
				append(185, "Ввод графического заголовка не доведен до конца");
				append(186, "Размер этикетки с номером >31 не должен превышать 3*24");
				append(187, "Ложное выключение питания");
				append(188, "Некорректная версия ПЗУ по данным ФП");
				append(189, "Ссылка на незапрограммированный код валюты");
				append(190, "Базовая валюта должна иметь имя");
				append(191, "Ссылка на налог с незапрограммированным именем");
				append(192, "Процедура подготовки к выключению питания не была завершена");
				append(193, "Расхождение данных между ОЗУ и ФП");
				append(194, "При выключенных DIP запрещено");
				append(195, "Дата меньше, чем дата фискализации");
				append(196, "Сбой ЭСПЗУ");
				append(197, "Время не установлено");
				append(198, "Номер фискального документа> количества документов");
				append(199, "Неверная команда режима продолжения печати подкладного документа");
				append(200, "Нулевое количество и цена в итоге чека");
				append(201, "По ставке налогов покупка не выполняется");
				append(202, "Операции с покупкой по карте запрещены");
				append(203, "Переплата сдачи клиенту");
				append(204, "Программирование небазовой валюты при пустой базовой запрещено");
				append(205, "Не все небазовые валюты обнулены");
				append(206, "Попытка обнулить текущую небазовую валюту");
				append(207, "Сдача по карте запрещена");
				append(208, "Предупреждение: установлено только одно наличное платежное средство №8");
				append(209, "Неверная контрольная сумма ПЗУ");
				append(210, "Допустимые команды: Либо завершение чека, либо отмена");
				append(211, "Чек не сформирован");
				append(212, "Неверная контрольная сумма серийного номера");
				append(213, "Неверная контрольная сумма фиск. Реквизитов");
				append(214, "Количество значащих символов меньше трёх");
				append(215, "Скорость обмена с ПК должна превышать скорость обмена с принтером");
				append(216, "Нет ответа от фискального модуля");
				append(217, "Неверный ответ от фискального модуля");
				append(218, "Неверная контр. сумма фискального модуля");
				append(219, "Кол-во пропавших записей в ФП>2. ККТ заблокирована");
				append(220, "Неверная контрольная сумма регистрации ЭКЛЗ");
				append(221, "Неверная контрольная сумма регистраций очистки ОЗУ");
				append(222, "Дата меньше даты активизации ЭКЛЗ");
				append(223, "Переполнен буфер Z-отчётов, текущая смена закрыта");
				append(224, "Не очищен буфер Z-отчётов, текущая смена не закрыта");
				append(225, "Не распечатан буфер Z-отчётов");
				append(226, "Близок конец бумаги");
				append(227, "Переполнение буфера реквизитов");
				append(228, "Технологическая ПЗУ");
				append(229, "Данные не получены от фискального модуля");
				append(230, "Нет ответа от SRAM");
				append(231, "Невозможно очистить буфер USART1");
				append(232, "Некорректное значение стека после выключения питания");
				append(233, "Принтер не готов");
				append(234, "EEPROM не запрограммирована");
				append(235, "Примечание: изменения вступят в силу после выключения питания");
				append(236, "Купюроприёмник отключён");
				append(237, "Невозможно включить приём купюр");
				append(238, "Невозможно отключить приём купюр");
				append(239, "Запрещено в выключенном режиме приёма купюр");
				append(240, "Запрещено во включённом режиме приёма купюр");
				append(241, "Превышение итога документа");
				append(242, "Пустая таблица деноминации в купюроприёмнике");
				append(243, "Запрещено по программируемому флагу 28");
			}
		};

		static CDescriptions Descriptions;
	}

	//--------------------------------------------------------------------------------
	/// Спецификация статусов.
	class CStatus1 : public BitmapDeviceCodeSpecification
	{
	public:
		CStatus1()
		{
			addStatus( 2, DeviceStatusCode::OK::Busy);
			addStatus( 3, FRStatusCode::Error::FiscalMemory);
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

			mErrors.addStatus('\x2C', FRStatusCode::Error::FiscalMemory);
			mErrors.addStatus('\x24', FRStatusCode::Error::FiscalMemory);
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
