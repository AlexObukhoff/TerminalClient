/* @file Бэкенд для визуализации QML виджетов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QSharedPointer>
#include <QtDeclarative/QDeclarativeEngine>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/IPlugin.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Scripting/Core.h>
#include <SDK/GUI/IGraphicsBackend.h>

// Проект
#include "QMLGraphicsItem.h"

//------------------------------------------------------------------------------
class QMLBackend: public QObject, public SDK::Plugin::IPlugin, public SDK::GUI::IGraphicsBackend
{
	Q_OBJECT

public:
	QMLBackend(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath);

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

	#pragma region SDK::GUI::IGraphicsBackend interface

	/// Инициализируем
	virtual bool initialize(SDK::GUI::IGraphicsEngine * aContainer);

	/// Очищаем ресурсы
	virtual void shutdown();

	/// Создаёт (или возвращает из кэша) графический элемент по описанию.
	virtual SDK::GUI::IGraphicsItem * getItem(const SDK::GUI::GraphicsItemInfo & aInfo);

	/// Возвращает тип движка.
	virtual QString getType() const;

	/// Возвращает список экранов, с которыми работает бэкэнд
	virtual QList<SDK::GUI::GraphicsItemInfo> getItemList();

	#pragma endregion

private slots:
	void onWarnings(const QList<QDeclarativeError> & aWarnings);

private:
	typedef QMap<QString, QSharedPointer<QMLGraphicsItem> > TGraphicItemsCache;

	QString mInstancePath;
	QVariantMap mParameters;

	SDK::Plugin::IEnvironment * mFactory;
	SDK::GUI::IGraphicsEngine * mEngine;
	SDK::PaymentProcessor::ICore * mCore;

	QDeclarativeEngine mQMLEngine;
	TGraphicItemsCache mCachedItems;
};

//------------------------------------------------------------------------------
