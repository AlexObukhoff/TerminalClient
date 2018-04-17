/* @file Интерфейс обеспечивающий взаимодействие с менеджером устройств. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QSharedPointer>
#include <QtCore/QFile>
#include <Common/QtHeadersEnd.h>

#include <SDK/Drivers/IDevice.h>
#include <SDK/PaymentProcessor/IDeviceTest.h>
#include <SDK/Plugins/PluginParameters.h>

namespace SDK {
namespace PaymentProcessor {

/// Список моделей и драйверов для каждой модели.
typedef QMap<QString, QStringList> TModelList;

//------------------------------------------------------------------------------
class IDeviceStatus;

//------------------------------------------------------------------------------
/// Интерфейс менеджера устройств.
class IDeviceService : public QObject
{
	Q_OBJECT
public:
	enum UpdateFirmwareResult
	{
		OK = 0,
		NoDevice,
		CantUpdate
	};

public:
	/// Неблокирующий поиск всех устройств.
	virtual void detect(const QString & aDeviceType = QString()) = 0;

	/// Прервать поиск устройств.
	virtual void stopDetection() = 0;

	/// Получить полный список конфигураций.
	virtual QStringList getConfigurations(bool aAllowOldConfigs = true) const = 0;

	/// Сохранить cписок конфигураций.
	virtual bool saveConfigurations(const QStringList & aConfigList) = 0;

	/// Добавить список параметров, необходимых для инициализации устройств.
	virtual void setInitParameters(const QString & aDeviceType, const QVariantMap & aParameters) = 0;

	/// Подключение/захват устройства. aDeviceNumber - номер среди одинаковых устройств.
	virtual SDK::Driver::IDevice * acquireDevice(const QString & aInstancePath) = 0;

	/// Создание устройства. Возвращает имя созданной конфигурации.
	virtual QString createDevice(const QString & aDriverPath, const QVariantMap & aConfig) = 0;

	/// Отключение/освобождение указанного устройства.
	virtual void releaseDevice(SDK::Driver::IDevice * aDevice) = 0;

	/// Обновить прошивку устройства.
	virtual UpdateFirmwareResult updateFirmware(const QByteArray & aFirmware, const QString & aDeviceGUID) = 0;

	/// Получение списка параметров драйвера.
	virtual SDK::Plugin::TParameterList getDriverParameters(const QString & aDriverPath) const = 0;

	/// Получить конфигурацию устройства и всех, связанных с ним.
	virtual QVariantMap getDeviceConfiguration(const QString & aConfigName) = 0;

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QString & aConfigName, const QVariantMap & aConfig) = 0;

	/// Получение списка драверов (поддерживаемых устройств).
	virtual TModelList getModelList(const QString & aFilter = QString()) const = 0;

	/// Получение списка всех драйверов.
	virtual QStringList getDriverList() const = 0;

	/// Возвращает список созданных устройств.
	virtual QStringList getAcquiredDevicesList() const = 0;

	/// Получение имя конфигурации по устройству.
	virtual QString getDeviceConfigName(SDK::Driver::IDevice * aDevice) = 0;

	/// Получить статус устройства (уровень тревожности и описание) по имени конфигурации.
	virtual QSharedPointer<IDeviceStatus> getDeviceStatus(const QString & aConfigName) = 0;

	/// Освобождает все устройства.
	virtual void releaseAll() = 0;

signals:
	/// Посылается, когда обнаружено очередное устройство.
	void deviceDetected(const QString & aConfigName);

	/// Посылается когда процесс обнаружения устройств останавливается.
	void detectionStopped();

	/// Сигнал об изменении статуса устройства.
	void deviceStatusChanged(const QString & aConfigName, SDK::Driver::EWarningLevel::Enum aLevel, const QString & aDescription, int aStatus);

	/// Сигнал об изменении конфигурации.
	void configurationUpdated();
};

//------------------------------------------------------------------------------
// Интерфес состояния устройства
class IDeviceStatus
{
public:
	/// Уровень тревожности
	virtual SDK::Driver::EWarningLevel::Enum level() const = 0;

	/// Описание статуса
	virtual const QString & description() const = 0;

	/// Проверить содержимое статуса на удовлетворение определенному уровню
	virtual bool isMatched(SDK::Driver::EWarningLevel::Enum aLevel) const = 0;
};


//------------------------------------------------------------------------------
} // PaymentProcessor
} // SDK
