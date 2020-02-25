/* @file Коды состояний принтеров. */

#pragma once

#include "Hardware/Common/BaseStatus.h"

//--------------------------------------------------------------------------------
namespace PrinterStatusCode
{
	/// OK.
	namespace OK
	{
		const int PaperInPresenter = 200;    /// Бумага удерживается презентером.
		const int MotorMotion      = 201;    /// Движение мотора.
	}

	/// Предупреждения.
	namespace Warning
	{
		const int PaperNearEnd         = 210;    /// Чековая лента заканчивается.
		const int ControlPaperNearEnd  = 211;    /// Контрольная лента заканчивается.
		const int TonerNearEnd         = 212;    /// Тонер заканчивается.
		const int PaperEndVirtual      = 213;    /// Бумага закончилась по показания датчика скорого окончания бумаги.
		const int WrongFWConfiguration = 214;    /// Неверная конфигурация прошивки.
		const int OFDNotSent           = 215;    /// Нет связи с ОФД сервером (виртуальный статус фискального сервера).
	}

	/// Ошибки.
	namespace Error
	{
		const int PaperEnd              = 220;    /// Чековая лента закончилась.
		const int ControlPaperEnd       = 221;    /// Контрольная лента закончилась.
		const int PaperJam              = 222;    /// Бумага зажевана.
		const int Paper                 = 223;    /// Ошибка в тракте бумаги.
		const int Temperature           = 224;    /// Температурная ошибка.
		const int PrintingHead          = 225;    /// Ошибка печатающей головки принтера.
		const int Cutter                = 226;    /// Ошибка отрезчика.
		const int Presenter             = 227;    /// Ошибка презентера.
		const int Port                  = 228;    /// Ошибка порта.
		const int PrinterFR             = 229;    /// Неизвестная ошибка принтера в составе ФР.
		const int PrinterFRCollapse     = 230;    /// Глобальная ошибка принтера в составе ФР, печать невозможна.
		const int PrinterFRNotAvailable = 231;    /// Принтер в составе ФР недоступен.
		const int Motor                 = 232;    /// Ошибка двигателя.
		const int NoToner               = 233;    /// Тонер закончился.
		const int OutletFull            = 234;    /// Выходной лоток полон.
		const int NeedPaperTakeOut      = 235;    /// Необходимо извлечь бумагу из презентера.
		const int MemoryEnd             = 236;    /// Не хватает памяти.
		const int OFDNotSent            = 237;    /// Нет связи с ОФД сервером (виртуальный статус фискального сервера).
	}
}

//--------------------------------------------------------------------------------
