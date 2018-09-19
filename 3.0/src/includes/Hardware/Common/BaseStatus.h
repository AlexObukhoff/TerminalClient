/* @file Коды состояний устройств общие. */

#pragma once

//--------------------------------------------------------------------------------
namespace DeviceStatusCode
{
	/// OK.
	namespace OK
	{
		const int OK                  = 0;    /// Нет ошибок, готов к работе.
		const int Busy                = 1;    /// Занят чем-то нужным, скоро будет готов.
		const int Initialization      = 2;    /// Инициализация.
		const int Unknown             = 3;    /// Драйвер не знает это состояние.
	}

	/// Предупреждения.
	namespace Warning
	{
		const int Firmware               = 30;    /// Необходимо обновить прошивку.
		const int NeedReboot             = 31;    /// Необходимо перезагрузить устройство по питанию.
		const int ThirdPartyDriver       = 32;    /// Ошибка драйвера стороннего ПО и/или прошивки устройства при выполнении операции.
		const int WrongSwitchesConfig    = 33;    /// Устройство неверно сконфигурировано.
		const int Developing             = 34;    /// Ошибка проектирования (прошивки).
		const int Compatibility          = 35;    /// Версия плагина не соответствует версии PP.
		const int OperationError         = 36;    /// Ошибка выполнения команды.
		const int UnknownDataExchange    = 37;    /// Ошибка при получении данных от устройства.
		const int ModelNotVerified       = 38;    /// Не гарантирована полная совместимость с ПО.
		const int ModelNotCompatible     = 39;    /// Модель соответствует другому плагину.
		const int Unknown                = 40;    /// Неизвестное (подозрительное) состояние.
	}

	/// Ошибки.
	namespace Error
	{
		const int Unknown                = 60;    /// Неизвестная ошибка устройства.
		const int MechanismPosition      = 61;    /// Внутренний механизм не приведен в рабочее положение.
		const int PowerSupply            = 62;    /// Ошибка электропитания.
		const int MemoryStorage          = 63;    /// Повреждена микросхема памяти (Flash/RAM/EEPROM).
		const int NotAvailable           = 64;    /// Недоступен.
		const int Temperature            = 65;    /// Температура не соответствует условиям эксплуатации.
		const int Initialization         = 66;    /// Ошибка инициализации.
		const int Firmware               = 67;    /// Неверная прошивка.
		const int Maintenance            = 68;    /// Необходимо техническое обслуживание.
		const int ThirdPartyDriver       = 69;    /// Ошибка драйвера стороннего ПО.
		const int ThirdPartyDriverFail   = 70;    /// Не удается установить связь с драйвером стороннего ПО.
		const int Driver                 = 69;    /// Ошибка драйвера.
		const int Boot                   = 71;    /// Ошибка загрузчика.
		const int RecoveryMode           = 72;    /// Ограниченная работоспособность.
		const int Electronic             = 73;    /// Ошибка электроники.
		const int CoverIsOpened          = 74;    /// Открыта крышка.
		const int MechanicFailure        = 75;    /// Механическая неисправность.
	}
}

//--------------------------------------------------------------------------------
