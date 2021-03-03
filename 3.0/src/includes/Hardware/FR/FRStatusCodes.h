/* @file Коды состояний фискальных регистраторов. */

#pragma once

#include "Hardware/Printers/PrinterStatusCodes.h"

//--------------------------------------------------------------------------------
namespace FRStatusCode
{
	/// Предупреждения.
	namespace Warning
	{
		const int EKLZNearEnd                = 250;    /// ЭКЛЗ близка к заполнению.
		const int FiscalMemoryNearEnd        = 251;    /// Фискальная память близка к заполнению.
		const int FRNotRegistered            = 252;    /// ККТ не зарегистрирована.
		const int FSNotFiscalized            = 253;    /// ФН не фискализирована.
		const int ZBufferFull                = 254;    /// Заполнен буффер Z-отчётов.
		const int OFDNoConnection            = 255;    /// Нет связи с сервером ОФД.
		const int FSNearEnd                  = 256;    /// Срок действия ФН скоро кончится.
		const int OFDData                    = 257;    /// Ошибка данных ОФД в ФР.
		const int FFDMismatch                = 258;    /// Несоответствие версий ФФД ФР и ФН.
		const int FFDFR                      = 259;    /// Необходимо обновить версию ФФД ФР.
		const int FFDFS                      = 260;    /// Необходимо обновить версию ФФД ФН.
		const int FirmwareUpdating           = 261;    /// Невозможно включить автообновление прошивки.
		const int WrongDealerTaxSystem       = 262;    /// Неверно настроена СНО (1! СНО в ФР).
		const int WrongDealerAgentFlag       = 263;    /// Неверно настроен признак агента (1! признак агента в ФР).
		const int WrongFiscalizationSettings = 264;    /// Параметры фискализации некорректны.
		const int WrongTaxOnPayment          = 265;    /// Неверная налоговая ставка на платеже.
		const int NeedTimeSynchronization    = 266;    /// Необходима синхронизация с системным временем.
		const int FSVirtualEnd               = 267;    /// Срок действия ФН должен был закончиться.
		const int DealerSupportPhone         = 268;    /// Телефон техподдержки дилера некорректен.
	}

	/// Ошибки.
	namespace Error
	{
		const int FR                   = 270;    /// Неизвестная ошибка фискальной доработки.
		const int EKLZ                 = 271;    /// Ошибка ЭКЛЗ.
		const int FiscalCollapse       = 272;    /// Глобальная ошибка фискальной части ФР, печать невозможна.
		const int FM                   = 273;    /// Ошибка фискальной памяти.
		const int ZBuffer              = 274;    /// Ошибка буфера Z-отчетов ФР.
		const int ZBufferOverflow      = 275;    /// Переполнен буфер z-отчётов.
		const int NeedCloseSession     = 276;    /// Необходимо выполнить Z-отчет.
		const int FSEnd                = 277;    /// Срок действия ФН кончился.
		const int NeedOFDConnection    = 278;    /// Необходимо подключение к серверу ОФД.
		const int FS                   = 279;    /// Ошибка ФН.
		const int NoFS                 = 280;    /// ФН отсутствует.
		const int FSClosed             = 281;    /// ФН закрыт.
		const int FSOverflow           = 282;    /// Память ФН переполнена.
		const int NoMoney              = 283;    /// Не хватает денег для какой-либо операции.
		const int WrongDealerTaxSystem = 284;    /// Неверно настроена СНО.
		const int WrongDealerAgentFlag = 285;    /// Неверно настроен признак агента.
		const int Cashier              = 286;    /// Данные кассира неверны.
		const int Taxes                = 287;    /// Налоговые ставки неверны.
	}
}

//--------------------------------------------------------------------------------
