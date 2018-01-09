/* @file Плагин - источник рекламы. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSharedPointer>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/IPlugin.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/GUI/IAdSource.h>

// Modules
#include <AdBackend/Client.h>

//------------------------------------------------------------------------------
class AdSourcePlugin : public QObject, public SDK::GUI::IAdSource, public SDK::Plugin::IPlugin
{
	Q_OBJECT

public:
	AdSourcePlugin(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath);
	virtual ~AdSourcePlugin(void);

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

#pragma region SDK::GUI::IAdSource interface

public:
	/// Получить содержимое рекламного контента
	virtual QString getContent(const QString & aType) const;

	/// Зарегистрировать событие в статистике
	virtual void addEvent(const QString & aType, const QVariantMap & aParameters);

#pragma endregion

protected:
	QSharedPointer<Ad::Client> mClient;

	SDK::Plugin::IEnvironment * mFactory;
	QString mInstancePath;

	SDK::PaymentProcessor::ICore * mCore;
};

