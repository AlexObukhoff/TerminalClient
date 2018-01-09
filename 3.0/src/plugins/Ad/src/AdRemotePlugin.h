/* @file Клиент обновления рекламы. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QMap>
#include <QtCore/QSharedPointer>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/IPlugin.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IRemoteClient.h>

// Modules
#include <Common/ILogable.h>
#include <AdBackend/Client.h>

using SDK::PaymentProcessor::CyberPlat::Request;
using SDK::PaymentProcessor::CyberPlat::Response;
using SDK::PaymentProcessor::CyberPlat::RequestSender;

//------------------------------------------------------------------------------
class AdRemotePlugin : public SDK::PaymentProcessor::IRemoteClient, public SDK::Plugin::IPlugin, public ILogable
{
	Q_OBJECT

public:
	AdRemotePlugin(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath);
	virtual ~AdRemotePlugin();

	#pragma region SDK::Plugin::IPlugin interface

	/// Возвращает название плагина.
	virtual QString getPluginName() const;

	/// Возвращает параметры плагина.
	virtual QVariantMap getConfiguration() const;

	/// Настраивает плагин.
	virtual void setConfiguration(const QVariantMap & aParameters);

	/// Возвращает имя файла конфигурации без расширения (ключ + идентификатор).
	virtual QString getConfigurationName() const;

	/// Сохраняет конфигурацию плагина в постоянное хранилище (.ini файл или хранилище прикладной программы).
	virtual bool saveConfiguration();

	/// Проверяет успешно ли инициализировался плагин при создании.
	virtual bool isReady() const;

	#pragma endregion

public:
	/// Возвращает интерфейс ядра модуля проведения платежей
	SDK::PaymentProcessor::ICore * getCore() const;

	#pragma region SDK::PaymentProcessor::IRemoteClient interface

public:
	/// Запуск клиента
	virtual void enable();

	/// Остановка клиента
	virtual void disable();

	/// Возвращает список дополнительных функций, поддерживаемых мониторингом.
	virtual Capabilities getCapabilities() const;

	/// Использование указанной дополнительной функции.
	virtual bool useCapability(ECapability aCapabilty);

	#pragma endregion

protected slots:
	/// Обработка сигнала о обновленном контенте или что реклама у терминала протухла
	void needRestart();

protected:
	QSharedPointer<Ad::Client> mClient;

	SDK::Plugin::IEnvironment * mFactory;
	QString mInstancePath;

	SDK::PaymentProcessor::ICore * mCore;
};

/// Функция получения ссылки на объект клиента
QSharedPointer<Ad::Client> getAdClientInstance(SDK::Plugin::IEnvironment * aFactory);

//------------------------------------------------------------------------------