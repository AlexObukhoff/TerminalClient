/* @file Менеджер для работы с железом */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/IDeviceService.h>

// Project
#include "IConfigManager.h"

namespace SDK {
namespace PaymentProcessor {
	class ICore;
}

namespace Plugin {
	class IEnvironment;
	class IPlugin;
}}

//---------------------------------------------------------------------------
class HardwareManager : public QObject, public IConfigManager
{
	Q_OBJECT

public:
	HardwareManager(SDK::Plugin::IEnvironment * aFactory, SDK::PaymentProcessor::ICore * aCore);
	~HardwareManager();

public:
	/// Конфигурация изменилась?
	virtual bool isConfigurationChanged() const;

	/// Делаем текущую конфигурацию начальной
	virtual void resetConfiguration();

public:
	/// Начать поиск устройств
	void detect(const QString & aDeviceType = QString());
	
	/// Прервать поиск устройств.
	void stopDetection();

	/// Получить список конфигураций
	QStringList getConfigurations() const;

	/// Сохранить список конфигураций
	void setConfigurations(const QStringList & aConfigurations);

	/// Получить полную конфигурацию всех устройстр
	QVariantMap getConfiguration() const;

	/// Сохранить полную конфигурацию в памяти
	bool setConfiguration(const QVariantMap & aConfig);
	
	/// Возвращает конфигурацию устройства для сохранения.
	QVariantMap getDeviceConfiguration(const QString & aConfigName) const;

	/// Устанавливает конфигурацию устройству.
	void setDeviceConfiguration(const QString & aConfigurationName, const QVariantMap & aConfig);

	/// Получение списка всех драйверов.
	QStringList getDriverList() const;
	
	/// Получение списка драверов (поддерживаемых устройств).
	SDK::PaymentProcessor::TModelList getModelList(const QString & aFilter = QString()) const;

	/// Получение списка параметров драйвера.
	SDK::Plugin::TParameterList getDriverParameters(const QString & aDriverPath) const;

	/// Создание устройства.
	Q_INVOKABLE QString createDevice(const QString & aDriverPath, const QVariantMap & aConfig);

	/// Если true, устройство создалось
	bool checkDevice(const QString & aConfigName);

	/// Получить устройство
	SDK::Driver::IDevice * getDevice(const QString & aConfigName);

	/// Удалить устройство по имени конфигурации
	void releaseDevice(const QString & aConfigName);

	/// Освобождает все устройства.
	void releaseAll();

	/// Запрос сигнала с последним статусом.
	void updateStatuses();

	/// Присутствует ли фискальный принтер
	bool isFiscalPrinterPresent(bool aVirtual);

private slots:
	void deviceStatusChanged(const QString & aConfigName, SDK::Driver::EWarningLevel::Enum aLevel, const QString & aDescription);

signals:
	/// Посылается, когда обнаружено очередное устройство.
	void deviceDetected(const QString & aConfigName);

	/// Поиск остановлен
	void detectionStopped();

	/// Сигнал об изменении статуса устройства.
	void deviceStatusChanged(const QString & aConfigName, const QString & aStatusString, const QString & aStatusColor, SDK::Driver::EWarningLevel::Enum aLevel);

private:
	SDK::PaymentProcessor::ICore * mCore;
	SDK::PaymentProcessor::IDeviceService * mDeviceService;
	SDK::Plugin::IEnvironment * mFactory;

	QVariantMap mCurrentConfiguration;
};

//---------------------------------------------------------------------------
