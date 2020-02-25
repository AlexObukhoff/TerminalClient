/* @file Интерфейс порта ввода-вывода. */

#pragma once

// SDK
#include <SDK/Drivers/IDevice.h>

namespace SDK {
namespace Driver {

typedef QMap<int, int> TPortParameters;

/// Типы портов.
namespace EPortTypes
{
	enum Enum
	{
		Unknown,
		COM,
		VirtualCOM,
		COMEmulator,
		USB,
		TCP
	};
}

/// Состояния портов. Если не OK - порт не работает.
namespace EPortState
{
	enum Enum
	{
		OK = 0,
		DriverError,       /// Ошибка 3d-party драйвера.
		NoDevice,          /// Устройство не подключено, порт отсутствует.
		ResetFailed,       /// Сбой сброса.
		SettingsFailed,    /// Сбой установки настроек.
		WrongSettings,     /// Неверные параметры (при установке извне).
		TimeoutExpired,    /// Критичная операция не была выполнена за таймаут.
		Suspended,         /// Критичная операция "зависла".
		Incorrect          /// Некоррестное состояние.
	};
}

//--------------------------------------------------------------------------------
class IIOPort: public IDevice
{
public: // константы
	/// Таймаут чтения по умолчанию, [мс].
	static const unsigned int DefaultReadTimeout = 1000;

	/// Таймаут записи по умолчанию, [мс].
	static const unsigned int DefaultWriteTimeout = 500;

public: // методы
	/// Открыть порт.
	virtual bool open() = 0;

	/// Закрыть порт.
	virtual bool close() = 0;

	/// Очистить буферы порта.
	virtual bool clear() = 0;

	/// Установить параметры порта.
	virtual bool setParameters(const TPortParameters & aParameters) = 0;

	/// Получить параметры порта.
	virtual void getParameters(TPortParameters & aParameters) = 0;

	/// Прочитать данные.
	virtual bool read(QByteArray & aData, int aTimeout = DefaultReadTimeout, int aMinSize = 1) = 0;

	/// Передать данные.
	virtual bool write(const QByteArray & aData) = 0;

	/// Получить тип порта.
	virtual EPortTypes::Enum getType() = 0;

	/// Порт существует?
	virtual bool isExist() = 0;

	/// Подключено новое устройство?
	virtual bool deviceConnected() = 0;

	/// Открыт?
	virtual bool opened() = 0;

protected:
	virtual ~IIOPort() {}
};

}} // namespace SDK::Driver

//--------------------------------------------------------------------------------
