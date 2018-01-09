/* @file Коды состояний устройств приема денег. */

#pragma once

#include "Hardware/Common/BaseStatus.h"

//--------------------------------------------------------------------------------
namespace BillAcceptorStatusCode
{
	/// Устройство ничего не делает. OK.
	namespace Normal
	{
		const int Disabled           = 600;    /// Отключен на прием денег.
		const int Inhibit            = 601;    /// Выключен на прием денег, не может бы включен до отработки определенных действий.
		const int Enabled            = 602;    /// Включен на прием денег.
	}

	/// Работа с купюрой/монетой. OK.
	namespace BillOperation
	{
		const int Accepting          = 620;    /// Принимает купюру. Необходимо дать валидатору команду на укладывание в стекер или выброс.
		const int Escrow             = 621;    /// Принял купюру, и ждет дальнейшего решения: stack или return. Дается инфа о купюре.
		const int Stacking           = 622;    /// Укладывает купюру в стекер.
		const int Stacked            = 623;    /// Уложил купюру в стекер.
		const int Unloading          = 624;    /// Выгружает купюру из диспенсера.
		const int Unloaded           = 625;    /// Выгрузил купюру из диспенсера.
		const int Dispensing         = 626;    /// Перегружает купюру в диспенсер.
		const int Dispensed          = 627;    /// Перегрузил купюру в диспенсер.
		const int Holding            = 628;    /// Купюра удерживается в валидаторе (не уложена в стекер).
		const int PackCashingIn      = 629;    /// Принимает пачку денег.
		const int Unknown            = 630;    /// Что-то делаем с купюрой.
	}

	/// Занят, технологические операции. OK, но если долго висит - предупреждение.
	namespace Busy
	{
		const int SettingStackerType = 640;    /// Устанавливает тип кассеты.
		const int SetStackerType     = 641;    /// Установил тип кассеты.
		const int Pause              = 642;    /// Попытка сунуть вторую купюру, пока первая ещё не проглочена стекером.
		const int Calibration        = 643;    /// Калибруется.
		const int Returning          = 644;    /// Возвращает купюру.
		const int Returned           = 645;    /// Вернул купюру обратно.
		const int Unknown            = 646;    /// Занят чем-то непонятным. Может быть включен на прием денег.
	}

	/// Ошибка выполенения команды. Предупреждение, наверх не шлем.
	namespace OperationError
	{
		const int NoteRecognize      = 660;    /// Ошибка при распознавании купюр.
		const int Unknown            = 661;    /// Неверная команда.
		const int Reset              = 662;    /// Ошибка выполнения команды RESET.
		const int Stack              = 663;    /// Ошибка выполнения команды STACK.
		const int Return             = 664;    /// Ошибка выполнения команды RETURN.
		const int Escrow             = 665;    /// Ошибка операции после Escrow (Hold, Stack, Return).
		const int Accept             = 666;    /// Ошибка принятия купюры вследствие удержания извне //TODO: проверить, что это так.
		const int EmptyInlet         = 667;    /// Нет купюр на входе купюроприемника.
		const int HostCommand        = 668;    /// Неправильный формат команды хоста.
		const int Communication      = 669;    /// Ошибка связи с устройством (CRC, таймаут ожидания пакета либо тех. посылки, внутренние ошибки).
		const int Separating         = 671;    /// Ошибка разделения купюр, может приводить к потере денег.
		const int Initialize         = 672;    /// Warning. Ошибка стороннего ПО и/или прошивки устройства (например, работа с памятью).
		const int SetEnable          = 673;    /// Ошибка включения на прием денег.
		const int CashCalculation    = 674;    /// Неправильно посчитана сумма транзакции.
		const int NoteLowerLocation  = 675;    /// Неправильное расположение купюры в районе входа в стекер.
		const int NoteMiddleLocation = 676;    /// Неправильное расположение купюры в середине конвейера.
		const int NoteHighLocation   = 677;    /// Неправильное расположение купюры в начале конвейера или около входа в конвейер.
	}

	/// Механическая ошибка. При платеже может быть потеря денег.
	namespace MechanicFailure
	{
		const int JammedInValidator  = 681;    /// Зажевало купюру в валидаторе.
		const int JammedInStacker    = 682;    /// Зажевало купюру в стекере.
		const int StickInExitChannel = 683;    /// Купюра застряла в выходном канале.
		const int JammedCoin         = 684;    /// Застряла монета.
		const int StackerFull        = 685;    /// Стекер полон.
		const int StackerOpen        = 686;    /// Стекер открыт или вытащен.
		const int HeadRemoved        = 687;    /// Убрана голова валидатора.
		const int StackerMotor       = 688;    /// Неисправен мотор стекера.
		const int TransportMotor     = 689;    /// Неисправен мотор конвейера.
		const int AligningMotor      = 690;    /// Неисправен мотор механизма выравнивания.
		const int SeparatingMotor    = 691;    /// Неисправен мотор сепаратора (для купюроприемников с несколькими стекерами).
		const int Stacker            = 692;    /// Кассета неисправна.
		const int Stacker1           = 693;    /// Кассета1 неисправна.
		const int Stacker2           = 694;    /// Кассета2 неисправна.
		const int Stacker3           = 695;    /// Кассета3 неисправна.
		const int Stacker4           = 696;    /// Кассета4 неисправна.
		const int ReturnMechanism    = 697;    /// Возвратный механизм поврежден.
		const int COSMechanism       = 698;    /// Поврежден механизм защиты от ленточного мошенничества.
		const int DCEChute           = 699;    /// Поврежден желоб одновременной подачи 2-х монет.
		const int NoStackers         = 700;    /// Все кассеты либо сняты, либо запрещены к приему денег.
		const int CoinGateStuck      = 701;    /// Заклинило задвижку приема монет.
	}

	/// Неисправность датчиков. Ошибка, не ведет к потере денег.
	namespace SensorError
	{
		const int Optical        = 710;    /// Неисправен оптический датчик (на просвет).
		const int Reflective     = 711;    /// Неисправен оптический датчик  (на отражение).
		const int Magnetic       = 712;    /// Неисправен магнитный датчик.
		const int Capacitance    = 713;    /// Неисправен емкостный датчик.
		const int Dielectric     = 714;    /// Неисправен датчик диэлектрической проницаемости.
		const int Credit         = 715;    /// Неисправен датчик наличия средств.
		const int Piezo          = 716;    /// Неисправен пьезоэлектрический датчик.
		const int Diameter       = 717;    /// Неисправен датчик диаметра.
		const int WakeUp         = 718;    /// Неисправен датчик обнаружения.
		const int Sorter         = 719;    /// Неисправен датчик сортировки.
		const int Dispensing     = 720;    /// Неисправен датчик выдачи.
		const int Validation     = 721;    /// Неисправен датчик распознавания.
		const int Reject         = 722;    /// Неисправен датчик выброса.
		const int Thermo         = 723;    /// Неисправен термо-датчик (термистор).
		const int String         = 724;    /// Неисправен датчик "ленточного" мошенничества (выдергивание монеты лентой).
		const int Rim            = 725;    /// Неисправен краевой датчик проверки монет.
	}

	/// Ошибка. Не ведет к потере денег.
	namespace Error
	{
		const int WrongCurrency     = 730;     /// Валюта не поддерживается.
		const int NoParsAvailable   = 731;     /// Нет доступных номиналов.
		const int ParTableLoading   = 732;     /// Не загружена таблица номиналов.
		const int Firmware          = 733;     /// Ошибка прошивки.
		const int Clock             = 734;     /// Неисправны часы реального времени.
		const int Calibration       = 735;     /// Ошибка калибровки датчиков.
	}

	/// Предупреждение. Наверх шлем.
	namespace Warning
	{
		const int Cheated           = 740;     /// Попытка мошеничества.
		const int StackerNearFull   = 741;     /// Стекер почти полон.
		const int ParInhibitions    = 742;     /// Не установлены запрещения номиналов.
	}

	/// Выброс. Предупреждение, но если долго висит - ошибка.
	namespace Reject
	{
		const int Insertion         = 750;     /// Купюра неверно вставлена.
		const int Dielectric        = 751;     /// Выброс по диэлектрическому каналу.
		const int PreviousOperating = 752;     /// Идет прием предыдущей купюры или монеты.
		const int Compensation      = 753;     /// Засунуто несколько купюр.
		const int Transport         = 754;     /// Помеха на конвейере.
		const int Identification    = 755;     /// Идентификация купюры. TODO: прояснить режекты в CCNet.
		const int Verification      = 756;     /// Верификация купюры. TODO: прояснить режекты в CCNet.
		const int InhibitNote       = 757;     /// Запрещенная купюра.
		const int Operation         = 758;     /// Ошибка операции.
		const int DataProcessing    = 759;     /// Ошибка обработки данных.
		const int Length            = 760;     /// Ошибка длины купюры.
		const int LengthDoubling    = 761;     /// Задваивание по длине до эскроу на аппаратах с 2-мя моторами.
		const int Width             = 762;     /// Ошибка ширины купюры.
		const int WidthDoubling     = 763;     /// Задваивание по ширине.
		const int Unrecognised      = 764;     /// Не распознана. TODO: прояснить режекты в CCNet.
		const int MagneticSensor    = 765;     /// Магнитный канал.
		const int CapacitanceSensor = 766;     /// Емкостный канал.
		const int OpticalSensor     = 767;     /// Оптический канал.
		const int UVSensor          = 768;     /// Ультрафиолетовый канал.
		const int DoubleCorrelation = 769;     /// Двойная корреляция.
		const int Barcode           = 770;     /// Не совпадает штрих-код с номиналом.
		const int Diverter          = 771;     /// Ошибка направляющих конвейера.
		const int UserDefined       = 772;     /// Инициирован пользователем.
		const int Cheated           = 773;     /// Попытка мошенничества.
		const int EscrowTimeout     = 774;     /// Таймаут ожидания действий после эскроу.
		const int Unknown           = 775;     /// Причина неизвестна.
	}
}

//--------------------------------------------------------------------------------
