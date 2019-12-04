/* @file Коды состояний фискальных регистраторов. */

#pragma once

#include "Hardware/Printers/PrinterStatusCodes.h"

//--------------------------------------------------------------------------------
namespace FRStatusCode
{
	/// Предупреждения.
	namespace Warning
	{
		const int EKLZNearEnd                = 250;  /// ЭКЛЗ близка к заполнению.
		const int FiscalMemoryNearEnd        = 251;  /// Фискальная память близка к заполнению.
		const int NotFiscalized              = 252;  /// ККМ не фискализирована.
		const int ZBufferFull                = 253;  /// Заполнен буффер Z-отчётов.
		const int OFDNoConnection            = 254;  /// Нет связи с сервером ОФД.
		const int FSNearEnd                  = 255;  /// Срок действия ФН скоро кончится.
		const int OFDData                    = 256;  /// Ошибка данных ОФД в ФР.
		const int FFDMismatch                = 257;  /// Несоответствие версий ФФД ФР и ФН.
		const int FFDFR                      = 258;  /// Необходимо обновить версию ФФД ФР.
		const int FFDFS                      = 259;  /// Необходимо обновить версию ФФД ФН.
		const int FirmwareUpdating           = 260;  /// Невозможно включить автообновление прошивки.
		const int WrongDealerTaxSystem       = 261;  /// Неверно настроена СНО (1! СНО в ФР).
		const int WrongDealerAgentFlag       = 262;  /// Неверно настроен признак агента (1! признак агента в ФР).
		const int WrongFiscalizationSettings = 263;  /// Параметры фискализации некорректны.
		const int WrongTaxOnPayment          = 264;  /// Неверная налоговая ставка на платеже.
		const int NeedTimeSynchronization    = 265;  /// Необходима синхронизация с системным временем.
		const int FSVirtualEnd               = 266;  /// Срок действия ФН должен был закончиться.
		const int DealerSupportPhone         = 267;  /// Телефон техподдержки дилера некорректен.
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
		const int FSClosed             = 280;    /// ФН закрыт.
		const int NoMoney              = 281;    /// Не хватает денег для какой-либо операции.
		const int WrongDealerTaxSystem = 282;    /// Неверно настроена СНО.
		const int WrongDealerAgentFlag = 283;    /// Неверно настроен признак агента.
		const int CashierINN           = 284;    /// Неправильный ИНН кассира.
		const int Taxes                = 285;    /// Налоговые ставки неверны.
	}
}

//--------------------------------------------------------------------------------
