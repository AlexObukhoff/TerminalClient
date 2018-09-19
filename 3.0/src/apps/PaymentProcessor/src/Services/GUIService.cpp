/* @file Менеджер для работы с новым интерфейсом. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDir>
#include <QtGui/QApplication>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/GUI/IGraphicsBackend.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/EventTypes.h>
#include <SDK/PaymentProcessor/Payment/Step.h>
#include <SDK/PaymentProcessor/Scripting/Core.h>

#include <SysUtils/ISysUtils.h>

// Project
#include "System/IApplication.h"
#include "System/SettingsConstants.h"

#include "Services/ServiceNames.h"
#include "Services/PluginService.h"
#include "Services/TerminalService.h"
#include "Services/GUIService.h"
#include "Services/EventService.h"
#include "Services/IdleScenario.h"
#include "Services/FirmwareUploadScenario.h"

namespace PPSDK = SDK::PaymentProcessor;

namespace CGUIService
{
	const int DefaultScreenWidth = 1280;
	const int DefaultScreenHeight = 1024;

	const int StartDragDistance = 10;

	const QString DefaultScenario = "menu";
}

//---------------------------------------------------------------------------
GUIService * GUIService::instance(IApplication * aApplication)
{
	try
	{
		auto core = aApplication->getCore();

		if (core->getService(CServices::GUIService))
		{
			return static_cast<GUIService *>(core->getGUIService());
		}
	}
	catch (PPSDK::ServiceIsNotImplemented)
	{
		return nullptr;
	}

	return nullptr;
}

//---------------------------------------------------------------------------
GUIService::GUIService(IApplication * aApplication)
	: ILogable(CGUIService::LogName),
	  mApplication(aApplication),
	  mPluginService(nullptr),
	  mEventManager(nullptr),
	  mScriptingCore(nullptr),
	  mDisabled(false),
	  mWidth(0),
	  mHeight(0)
{
}

//---------------------------------------------------------------------------
GUIService::~GUIService()
{
}

//---------------------------------------------------------------------------
bool GUIService::initialize()
{
	// Выводим стандартный заголовок в лог
	getLog()->adjustPadding(-99);
	getLog()->write(LogLevel::Normal, QString("Initializing GUI Service."));
	getLog()->write(LogLevel::Normal, QString("*").repeated(58));
	
	mScriptingCore = new PPSDK::Scripting::Core(mApplication->getCore());
	mScriptingCore->setLog(getLog());

	mEventManager = mApplication->getCore()->getEventService();
	mEventManager->subscribe(this, SLOT(onEvent(const SDK::PaymentProcessor::Event &)));

	mCheckTopmostTimer.setInterval(CGUIService::CheckTopmostWindowTimeout);
	connect(&mCheckTopmostTimer, SIGNAL(timeout()), this, SLOT(bringToFront()));

	// Получаем директорию с файлами интерфейса из настроек.
	QString interfacePath = IApplication::toAbsolutePath(mApplication->getSettings().value(CSettings::InterfacePath).toString());

	connect(&mGraphicsEngine, SIGNAL(userActivity()), &mScenarioEngine, SLOT(resetTimeout()));
	connect(&mGraphicsEngine, SIGNAL(intruderActivity()), SLOT(onIntruderActivity()), Qt::QueuedConnection);
	connect(&mGraphicsEngine, SIGNAL(closed()), SLOT(onMainWidgetClosed()));
	connect(&mGraphicsEngine, SIGNAL(keyPressed(QString)), SLOT(onKeyPressed(QString)));	

	mScenarioEngine.injectScriptObject(PPSDK::Scripting::CProxyNames::Core, mScriptingCore);
	mScenarioEngine.injectScriptObject<PPSDK::EEventType>(PPSDK::Scripting::CProxyNames::EventType);
	mScenarioEngine.injectScriptObject<PPSDK::EPaymentStep>(PPSDK::Scripting::CProxyNames::PaymentStep);

	mScenarioEngine.addDirectory(interfacePath);
	mScenarioEngine.addDirectory(":/Scenario");
	mScenarioEngine.initialize();

	mGraphicsEngine.addContentDirectory(interfacePath);
	mGraphicsEngine.addContentDirectory(":/GraphicsItems");

	mGraphicsEngine.setGraphicsHost(this);

	mPluginService = PluginService::instance(mApplication);
	
	loadScriptObjects();
	loadBackends();
	loadNativeScenarios();
	loadAdSources();

	QVariantMap unitedSettings;

	auto parseIni = [&](const QString & aIniFile) 
	{
		QSettings settings(aIniFile, QSettings::IniFormat);
		settings.setIniCodec("UTF-8");

		foreach (auto key, settings.allKeys())
		{
			mConfig.insert(key, settings.value(key));
		}
	};

	// Загружаем настройки интерфейса
	parseIni(interfacePath + "/interface.ini");

	// Обновляем пользовательскими настройками
	parseIni(mApplication->getUserDataPath() + "/user.ini");

	mDefaultScenario = mConfig.value("interface/default_scenario").toString();
	if (mDefaultScenario.isEmpty())
	{
		toLog(LogLevel::Warning, QString("Default scenario is not specified. Set scenario name '%1'").arg(CGUIService::DefaultScenario));
		mDefaultScenario = CGUIService::DefaultScenario;
	}

	int display = mConfig.value("interface/display", 0).toInt();
	mWidth = mConfig.value("interface/width", CGUIService::DefaultScreenWidth).toInt();
	mHeight = mConfig.value("interface/height", CGUIService::DefaultScreenHeight).toInt();

	// Установка чувствительности для события mouse drag
	qApp->setStartDragDistance(mConfig.value("interface/sensivity", CGUIService::StartDragDistance).toInt());

	bool showCursor = mConfig.value("interface/show_mouse_cursor", false).toBool();
	bool showDebugInfo = mConfig.value("interface/show_debug_info", false).toBool();

	QVariantMap scenarios = getUiSettings("scenarios");
	if (!scenarios.isEmpty())
	{
		mExternalScenarios.clear();
		QStringList handledKeyList;
		
		foreach (QString key, scenarios.keys())
		{
			mExternalScenarios.insert(scenarios.value(key).toString(), key);
			handledKeyList << scenarios.value(key).toString();
		}

		mGraphicsEngine.addHandledKeys(handledKeyList);
	}

	if (!mGraphicsEngine.initialize(display, mWidth, mHeight, showCursor, showDebugInfo))
	{
		LOG(mApplication->getLog(), LogLevel::Error, "Failed to initialize graphics engine.");

		// GUI не будет отображаться, но платежная логика продолжит работу.
		return true;
	}
	else
	{
		// Добавляем сценарий перепрошивки устройств
		mScenarioEngine.addScenario(new FirmwareUploadScenario(mApplication));

		// Добавляем основной сценарий и запускаем его.
		GUI::Scenario * idle = new IdleScenario(mApplication);
		mScenarioEngine.addScenario(idle);

#ifndef _DEBUG
		// Запускаем проверку окна поверх всех
		mCheckTopmostTimer.start();
#endif		
		mGraphicsEngine.start();

		QVariantMap noGui;
		noGui.insert("no_gui", mConfig.value("interface/no_gui", false).toBool());
		if (!mScenarioEngine.startScenario(idle->getName(), noGui))
		{
			toLog(LogLevel::Error, "Failed to start idle scenario.");
			return false;
		}
	}

	mDisabled = mDisabled || TerminalService::instance(mApplication)->isLocked();

	return true;
}

//------------------------------------------------------------------------------
void GUIService::finishInitialize()
{
}

//---------------------------------------------------------------------------
bool GUIService::canShutdown()
{
	return mScenarioEngine.canStop();
}

//---------------------------------------------------------------------------
bool GUIService::shutdown()
{
	mGraphicsEngine.stop();
	mGraphicsEngine.finalize();
	mScenarioEngine.finalize();

	foreach (SDK::Plugin::IPlugin * plugin, mBackendPluginList)
	{
		dynamic_cast<SDK::GUI::IGraphicsBackend *>(plugin)->shutdown();
		mPluginService->getPluginLoader()->destroyPlugin(plugin);
	}

	mBackendPluginList.clear();

	delete mScriptingCore;
	mScriptingCore = nullptr;

	mEventManager->unsubscribe(this, SLOT(onEvent(const SDK::PaymentProcessor::Event &)));

	disconnect(&mCheckTopmostTimer, SIGNAL(timeout()), this, SLOT(bringToFront()));
	disconnect(&mGraphicsEngine, SIGNAL(userActivity()), &mScenarioEngine, SLOT(resetTimeout()));
	disconnect(&mGraphicsEngine, SIGNAL(closed()), this, SLOT(onMainWidgetClosed()));
	disconnect(&mGraphicsEngine, SIGNAL(keyPressed(const QString &)), this, SLOT(onKeyPressed(const QString &)));

	foreach (auto adSource, mAdSourceList)
	{
		mPluginService->getPluginLoader()->destroyPlugin(dynamic_cast<SDK::Plugin::IPlugin *>(adSource));
	}
	
	mAdSourceList.clear();

	return true;
}

//---------------------------------------------------------------------------
QString GUIService::getName() const
{
	return CServices::GUIService;
}

//---------------------------------------------------------------------------
const QSet<QString> & GUIService::getRequiredServices() const
{
	static QSet<QString> requiredServices = QSet<QString>()
		<< CServices::EventService
		<< CServices::PluginService
		<< CServices::FundsService
		<< CServices::SettingsService
		<< CServices::TerminalService
		<< CServices::PrintingService;

	return requiredServices;
}

//---------------------------------------------------------------------------
QVariantMap GUIService::getParameters() const
{
	return QVariantMap();
}

//---------------------------------------------------------------------------
void GUIService::resetParameters(const QSet<QString> &)
{
}

//---------------------------------------------------------------------------
QStringList GUIService::getInterfacesName() const
{
	QStringList result;

	result << PPSDK::CInterfaces::ICore
		<< PPSDK::Scripting::CProxyNames::Core;

	result.append(mBackendScenarioObjects.keys());

	return result;
}

//---------------------------------------------------------------------------
void * GUIService::getInterface(const QString & aInterface)
{
	void * object = nullptr;

	if (aInterface == PPSDK::CInterfaces::ICore)
	{
		object = mApplication->getCore();
	}
	else if (aInterface == PPSDK::Scripting::CProxyNames::Core)
	{
		object = mScriptingCore;
	}
	else if (mBackendScenarioObjects.contains(aInterface))
	{
		object = mBackendScenarioObjects.value(aInterface).data();
	}

	return object;
}

//---------------------------------------------------------------------------
void GUIService::onEvent(const SDK::PaymentProcessor::Event & aEvent)
{
	switch (aEvent.getType())
	{
		// Какое-то события сценария.
		case PPSDK::EEventType::UpdateScenario:
		{
			QString signal;
			QVariantMap parameters;

			if (aEvent.getData().type() == QVariant::String)
			{
				signal = aEvent.getData().toString();
			}
			else
			{
				parameters = aEvent.getData().value<QVariantMap>();
				signal = parameters["signal"].toString();
			}

			// FIXME: нужен другой тип события для попапов.
			if (signal == "popup_notify")
			{
				mGraphicsEngine.popupNotify(signal, parameters);
			}
			else
			{
				mScenarioEngine.signalTriggered(signal, parameters);
			}

			break;
		}

		case PPSDK::EEventType::StartScenario:
		{
			// Запуск сценария. Передаем параметрами имя сценария и контекст активации (параметры сценария).
			QVariantMap eventData = aEvent.getData().value<QVariantMap>();

			if (eventData.contains("name"))
			{
				QString scenarioName = eventData["name"].toString();

				mScenarioEngine.startScenario(scenarioName, eventData);
			}
			else
			{
				if (mDefaultScenario != CGUIService::IdleScenarioName)
				{
					// Пытаемся запустить дефолтный сценарий.
					mScenarioEngine.startScenario(mDefaultScenario);
				}
				else
				{
					//default_scenario=idle
					//idle сценарий уже запущен
				}
			}

			break;
		}

		case PPSDK::EEventType::StopScenario:
		{
			mScenarioEngine.stopScenario();
			break;
		}

		case PPSDK::EEventType::StateChanged:
			break;

		case PPSDK::EEventType::StartGraphics:
#ifndef _DEBUG
			// Запускаем проверку окна поверх всех
			mCheckTopmostTimer.start();
#endif
			mGraphicsEngine.start();
			break;

		case PPSDK::EEventType::PauseGraphics:
			mGraphicsEngine.pause();
			mCheckTopmostTimer.stop();
			break;

		case PPSDK::EEventType::StopGraphics:
			mGraphicsEngine.stop();
			break;
	}
}

//---------------------------------------------------------------------------
void GUIService::onKeyPressed(const QString & aKeyText)
{
	QString scenario = mExternalScenarios.value(aKeyText).toString();
	if (!scenario.isEmpty())
	{
		mScenarioEngine.startScenario(scenario);
	}
}

//---------------------------------------------------------------------------
void GUIService::disable(bool aDisable)
{
	if (mDisabled != aDisable)
	{
		mDisabled = aDisable;

		QVariantMap parameters;
		parameters["signal"] = aDisable ? CGUISignals::StopGUI : CGUISignals::StartGUI;
		EventService::instance(mApplication)->sendEvent(PPSDK::EEventType::UpdateScenario, parameters);
	}
	// Каждый раз заново посылаем сигнал на disabled=true, даже если уже заблокирован, т.к. причина блокировки могла измениться
	else if (mDisabled)
	{
		QVariantMap parameters;
		parameters["signal"] = CGUISignals::UpdateGUI;
		EventService::instance(mApplication)->sendEvent(PPSDK::EEventType::UpdateScenario, parameters);
	}
}

//---------------------------------------------------------------------------
void GUIService::onHIDData(const QVariant & aData)
{
	// TODO
	QVariantMap arguments;
	arguments["msisdn"] = aData;

	mScenarioEngine.signalTriggered("datapending", arguments);
}

//---------------------------------------------------------------------------
bool GUIService::show(const QString & aScene, const QVariantMap & aParameters)
{
	return mGraphicsEngine.show(aScene, aParameters);
}

//---------------------------------------------------------------------------
bool GUIService::showPopup(const QString & aWidget, const QVariantMap & aParameters)
{
	return mGraphicsEngine.showPopup(aWidget, aParameters);
}

//---------------------------------------------------------------------------
QVariantMap GUIService::showModal(const QString & aWidget, const QVariantMap & aParameters)
{
	return mGraphicsEngine.showModal(aWidget, aParameters);
}

//---------------------------------------------------------------------------
bool GUIService::hidePopup(const QVariantMap & aParameters)
{
	return mGraphicsEngine.hidePopup(aParameters);
}

//---------------------------------------------------------------------------
void GUIService::notify(const QString & aEvent, const QVariantMap & aParameters)
{
	mGraphicsEngine.notify(aEvent, aParameters);
}

//---------------------------------------------------------------------------
void GUIService::onMainWidgetClosed()
{
	mEventManager->sendEvent(PPSDK::Event(PPSDK::EEventType::CloseApplication));
}

//---------------------------------------------------------------------------
void GUIService::onIntruderActivity()
{
	auto settings = dynamic_cast<PPSDK::TerminalSettings *>(
		mApplication->getCore()->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter))->getCommonSettings();

	auto event = PPSDK::EEventType::OK;
	auto message = tr("#penetration_detected");

	switch (settings.penetrationEventLevel)
	{
	case PPSDK::EEventType::Critical:
		event = settings.penetrationEventLevel;
		message += " #alarm";

		if (settings.blockOn(PPSDK::SCommonSettings::Penetration))
		{
			mEventManager->sendEvent(PPSDK::Event(PPSDK::EEventType::TerminalLock, CGUIService::LogName, message));
		}
		break;

	case PPSDK::EEventType::Warning:
		message += " #alarm";
		break;
	}

	mEventManager->sendEvent(PPSDK::Event(event, CGUIService::LogName, message));
}

//---------------------------------------------------------------------------
bool GUIService::isDisabled() const
{
	return mDisabled;
}

//---------------------------------------------------------------------------
void GUIService::reset()
{
	mGraphicsEngine.reset();
}

//---------------------------------------------------------------------------
QRect GUIService::getScreenSize(int aIndex) const
{
	return aIndex ? mGraphicsEngine.getDisplayRectangle(aIndex) : QRect(0, 0, mWidth, mHeight);
}

//---------------------------------------------------------------------------
QPixmap GUIService::getScreenshot()
{
	return mGraphicsEngine.getScreenshot();
}

//---------------------------------------------------------------------------
QVariantMap GUIService::getUiSettings(const QString & aSection) const
{
	QVariantMap result;

	foreach (QString key, mConfig.keys())
	{
		if (!key.contains(aSection))
		{
			continue;
		}

		result.insert(key.split("/").last(), mConfig.value(key));
	}

	return result;
}

//---------------------------------------------------------------------------
void GUIService::loadAdSources()
{
	QStringList adSources = mPluginService->getPluginLoader()->getPluginList(QRegExp("PaymentProcessor\\.AdSource\\..*"));

	foreach (const QString & source, adSources)
	{
		auto plugin = mPluginService->getPluginLoader()->createPlugin(source);
		auto adSource = dynamic_cast<SDK::GUI::IAdSource *>(plugin);

		if (adSource)
		{
			mAdSourceList << adSource;
		}
		else
		{
			mPluginService->getPluginLoader()->destroyPlugin(plugin);
		}
	}
}

//---------------------------------------------------------------------------
void GUIService::loadNativeScenarios()
{
	QStringList scenarios = mPluginService->getPluginLoader()->getPluginList(QRegExp("PaymentProcessor\\.ScenarioFactory\\..*"));

	foreach (const QString & scenario, scenarios)
	{
		auto plugin = mPluginService->getPluginLoader()->createPlugin(scenario);
		auto factory = dynamic_cast<SDK::Plugin::IFactory<GUI::Scenario> *>(plugin);

		if (factory)
		{
			// Создаем сценарии.
			foreach (auto className, factory->getClassNames())
			{
				GUI::Scenario * scenarioObject = factory->create(className);
				mScenarioEngine.addScenario(scenarioObject);
			}
		}
		else
		{
			LOG(mApplication->getLog(), LogLevel::Error, QString("Bad scenario plugin %1.").arg(scenario));
		}

		mPluginService->getPluginLoader()->destroyPlugin(plugin);
	}
}

//---------------------------------------------------------------------------
void GUIService::loadBackends()
{
	QStringList backends = mPluginService->getPluginLoader()->getPluginList(QRegExp("PaymentProcessor\\.GraphicsBackend\\..*"));

	foreach (const QString & backend, backends)
	{
		SDK::Plugin::IPlugin * plugin = mPluginService->getPluginLoader()->createPlugin(backend);

		SDK::GUI::IGraphicsBackend * backendObject = dynamic_cast<SDK::GUI::IGraphicsBackend *>(plugin);
		if (backendObject)
		{
			backendObject->initialize(&mGraphicsEngine);
			mGraphicsEngine.addBackend(backendObject);

			mBackendPluginList << plugin;
		}
		else
		{
			LOG(mApplication->getLog(), LogLevel::Error, QString("Bad backend plugin %1.").arg(backend));
			mPluginService->getPluginLoader()->destroyPlugin(plugin);
		}
	}
}

//---------------------------------------------------------------------------
void GUIService::loadScriptObjects()
{
	QStringList scriptObjects = mPluginService->getPluginLoader()->getPluginList(QRegExp("PaymentProcessor\\.ScriptFactory\\..*"));

	foreach(const QString & scriptPluginName, scriptObjects)
	{
		auto plugin = mPluginService->getPluginLoader()->createPlugin(scriptPluginName);
		auto factory = dynamic_cast<SDK::Plugin::IFactory<PPSDK::Scripting::IBackendScenarioObject> *>(plugin);

		if (factory)
		{
			foreach(auto className, factory->getClassNames())
			{
				PPSDK::Scripting::IBackendScenarioObject * scriptObject = factory->create(className);

				toLog(LogLevel::Normal, QString("Register scenario backend object '%1' from '%2'.")
					.arg(scriptObject->getName()).arg(scriptPluginName));

				QString objectName = CGUIService::BackedObjectPrefix + scriptObject->getName();
				mScenarioEngine.injectScriptObject(objectName, scriptObject);
				mBackendScenarioObjects.insert(objectName, QWeakPointer<QObject>(scriptObject));
			}
		}
		else
		{
			LOG(mApplication->getLog(), LogLevel::Error, QString("Bad script object plugin %1.").arg(scriptPluginName));
			mPluginService->getPluginLoader()->destroyPlugin(plugin);
		}
	}
}

//---------------------------------------------------------------------------
SDK::GUI::IAdSource * GUIService::getAdSource() const
{
	return mAdSourceList.count() ? mAdSourceList.first() : nullptr;
}

//------------------------------------------------------------------------
QObject * GUIService::getBackendObject(const QString & aName) const
{
	QString fullName = CGUIService::BackedObjectPrefix + aName;
	return mBackendScenarioObjects.keys().contains(fullName) ? mBackendScenarioObjects.value(fullName).data() : nullptr;
}

//------------------------------------------------------------------------
void GUIService::bringToFront()
{
	foreach(QWidget * widget, QApplication::topLevelWidgets())
	{
		if (!widget->isHidden())
		{
			ISysUtils::bringWindowToFront(widget->winId());
		}
	}

	auto getTopmostWindowsTitle = [](QSettings & aSettings) -> QStringList {
		QStringList topmostWindows;

		aSettings.beginGroup("topmost");
		foreach(auto const key, aSettings.allKeys())
		{
			QVariant v = aSettings.value(key);
			switch (v.type())
			{
			case QVariant::StringList:
				topmostWindows.append(v.toStringList());
				break;
			case QVariant::String:
				topmostWindows.push_back(v.toString());
				break;
			}
		}
		aSettings.endGroup();

		return topmostWindows;
	};

	static QStringList topmostWindows = getTopmostWindowsTitle(mApplication->getSettings());

	foreach(auto title, topmostWindows)
	{
		ISysUtils::bringWindowToFront(title);
	}
}

//---------------------------------------------------------------------------
