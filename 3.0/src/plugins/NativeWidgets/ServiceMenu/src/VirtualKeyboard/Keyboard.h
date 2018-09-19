/* @file Плагин виртуальной клавиатуры */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtWidgets/QGraphicsProxyWidget>
#include <Common/QtHeadersEnd.h>

// Plugin SDK
#include <SDK/Plugins/IPlugin.h>
#include <SDK/GUI/IGraphicsItem.h>

class KeyboardWindow;

//--------------------------------------------------------------------------
class Keyboard : public SDK::Plugin::IPlugin, public SDK::GUI::IGraphicsItem
{
public:
	Keyboard(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath);
	~Keyboard();

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

#pragma region SDK::GUI::IGraphicsItem interface

	/// Вызывается перед отображением виджета.
	virtual void show();

	/// Вызывается для сброса/настройки виджета.
	virtual void reset(const QVariantMap & aParameters);

	/// Вызывается перед сокрытием виджета.
	virtual void hide();

	/// Посылает уведомление виджету.
	virtual void notify(const QString & aReason, const QVariantMap & aParameters);

	/// Проверяет готов ли виджет.
	virtual bool isValid() const;

	/// Возвращает описание ошибки.
	virtual QString getError() const;

	// Возвращает виджет.
	virtual QQuickItem * getWidget() const;

	virtual QWidget * getNativeWidget() const { return mKeyboardWindow; }

	/// Возвращает контекст виджета.
	virtual QVariantMap getContext() const;

#pragma endregion

private:
	bool mIsReady;
	QString mInstancePath;
	SDK::Plugin::IEnvironment * mEnvironment;
	QVariantMap mParameters;

	QGraphicsProxyWidget * mMainWidget;
	KeyboardWindow * mKeyboardWindow;
};

//--------------------------------------------------------------------------
