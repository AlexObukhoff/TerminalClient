/* @file Менеджер для работы с новым интерфейсом. */

#pragma once

// Modules
#include <Common/ILogable.h>

// SDK
#include <SDK/Plugins/IPlugin.h>
#include <SDK/Plugins/IFactory.h>
#include <SDK/GUI/IGraphicsHost.h>
#include <SDK/PaymentProcessor/Scripting/Core.h>
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>
#include <SDK/PaymentProcessor/Scripting/IBackendScenarioObject.h>
#include <SDK/GUI/IAdSource.h>

// Modules
#include <GraphicsEngine/GraphicsEngine.h>
#include <ScenarioEngine/ScenarioEngine.h>

class IApplication;
class PluginService;
class GuardService;

namespace CGUIService
{
	const char LogName[] = "Interface";
	const char BackedObjectPrefix[] = "Backend$";
	const char IdleScenarioName[] = "idle";
	const int CheckTopmostWindowTimeout = 5 * 1000;
}

//---------------------------------------------------------------------------
class GUIService :
	public QObject,
	public SDK::PaymentProcessor::IGUIService,
	public SDK::PaymentProcessor::IService,
	public SDK::GUI::IGraphicsHost,
	private ILogable
{
	Q_OBJECT

public:
	GUIService(IApplication * aApplication);

	virtual ~GUIService();

#pragma region IService interface
	/// Инициализация сервиса.
	virtual bool initialize();

	/// IService: Закончена инициализация всех сервисов.
	virtual void finishInitialize();

	/// Возвращает false, если сервис не может быть остановлен в текущий момент.
	virtual bool canShutdown();

	/// Завершение работы сервиса.
	virtual bool shutdown();

	/// Возвращает имя сервиса.
	virtual QString getName() const;

	/// Список зависимостей.
	virtual const QSet<QString> & getRequiredServices() const;

	/// Получить параметры сервиса.
	virtual QVariantMap getParameters() const;

	/// Сброс служебной информации.
	virtual void resetParameters(const QSet<QString> & aParameters);
#pragma endregion

#pragma region SDK::PaymentProcessor::IGUIService interface
	/// Отображает виджет.
	virtual bool show(const QString & aWidget, const QVariantMap & aParameters);

	/// Отображает popup-виджет (на уровнь выше текущего).
	virtual bool showPopup(const QString & aWidget, const QVariantMap & aParameters);

	/// Отображает модальный виджет.
	virtual QVariantMap showModal(const QString & aWidget, const QVariantMap & aParameters);

	/// Скрывает текущий popup или модальный виджет.
	virtual bool hidePopup(const QVariantMap & aParameters = QVariantMap());

	/// Оповещает виджет.
	virtual void notify(const QString & aEvent, const QVariantMap & aParameters);

	/// Остановлен ли интерфейс?
	virtual bool isDisabled() const;

	/// Ширина и высота экрана в пикселях.
	virtual QRect getScreenSize(int aIndex) const;

	virtual QPixmap getScreenshot();

	/// Возвращаем мапу параметров секции aSection из файла interface.ini
	virtual QVariantMap getUiSettings(const QString & aSection) const;

	virtual SDK::GUI::IAdSource * getAdSource() const;

	virtual QObject * getBackendObject(const QString & aName) const;

#pragma endregion

#pragma region IGraphicsHost
	/// Возвращает список имеющихся интерфейсов
	virtual QStringList getInterfacesName() const;

protected:
	/// Возвращает указатель на сервис с именем aInterface.
	virtual void * getInterface(const QString & aInterface);
#pragma endregion

public:
	/// Проверика возможносьт остановки графического интерфейса
	bool canDisable() const { return mScenarioEngine.canStop(); }

	/// Остановка интерфейса.
	void disable(bool aDisable);

	/// Получить инстанс.
	static GUIService * instance(IApplication * aApplication);

signals:
	void idling();

private slots:
	/// Обработка событий.
	void onEvent(const SDK::PaymentProcessor::Event & aEvent);

	void onKeyPressed(const QString & aKeyText);

	/// Обработка данных от HID устройств.
	void onHIDData(const QVariant & aData);

	/// Закрыть виджет, на котором рисует GraphicsEngine.
	void onMainWidgetClosed();

	/// Реакция на подозрительную работу терминала
	void onIntruderActivity();

	/// Переместить главное окно поверх всех окон
	void bringToFront();

private:
	/// Загрузка графических бэкендов.
	void loadBackends();

	/// Загрузка нативных сценариев.
	void loadNativeScenarios();

	/// Загрузка плагинов рекламы.
	void loadAdSources();

	/// Загрузка отдельных объектов для интерфейса
	void loadScriptObjects();

private:
	IApplication * mApplication;
	PluginService * mPluginService;
	SDK::PaymentProcessor::IEventService * mEventManager;

	SDK::PaymentProcessor::Scripting::Core * mScriptingCore;
	QMap<QString, QWeakPointer<QObject>> mBackendScenarioObjects;

	GUI::GraphicsEngine mGraphicsEngine;
	GUI::ScenarioEngine mScenarioEngine;

	QList<SDK::Plugin::IPlugin *> mBackendPluginList;

	QVariantMap mModalResult;
	QString mDefaultScenario;

	bool mDisabled;
	int mWidth;
	int mHeight;

	// Секции из interface.ini
	QVariantMap mConfig;

	/// Список поставщиков рекламы
	QList<SDK::GUI::IAdSource *> mAdSourceList;

	// Клавиша - сценарий
	QVariantMap mExternalScenarios;

	/// Для проверки положения главного окна
	QTimer mCheckTopmostTimer;
};

//---------------------------------------------------------------------------
