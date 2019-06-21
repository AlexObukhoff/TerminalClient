/* @file Базовый класс портов. */

#pragma once

// SDK
#include <SDK/Drivers/IIOPort.h>

// Modules
#include "Hardware/Common/LoggingType.h"
#include "Hardware/Common/MetaDevice.h"

//--------------------------------------------------------------------------------
class IOPortBase : public MetaDevice<SDK::Driver::IIOPort>
{
public:
	IOPortBase();

	static QString getDeviceType();

	/// Возвращает название устройства.
	virtual QString getName() const;

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);

	/// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
	virtual bool release();

	/// Очистить буферы порта.
	virtual bool clear();

	/// Установить параметры порта.
	virtual bool setParameters(const SDK::Driver::TPortParameters & /*aParameters*/) { return true; }

	/// Получить параметры порта.
	virtual void getParameters(SDK::Driver::TPortParameters & /*aParameters*/) {}

	/// Получить тип порта.
	virtual SDK::Driver::EPortTypes::Enum getType();

protected:
	/// Установить таймаут открытия порта.
	void setOpeningTimeout(int aTimeout);

	/// Заносит данные портов в конфигурацию и печатает лог.
	void adjustData(const QStringList & aMine, const QStringList & aOther);

	/// Имя системного порта.
	QString mSystemName;

	/// Тип порта.
	SDK::Driver::EPortTypes::Enum mType;

	/// Логгирование посылок.
	ELoggingType::Enum mDeviceIOLoging;

	/// Название устройства, подключенного к порту.
	QString mConnectedDeviceName;

	/// Таймаут открытия порта
	int mOpeningTimeout;
};

//--------------------------------------------------------------------------------
