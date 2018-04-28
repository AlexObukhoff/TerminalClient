/* @file Инициализация и получение сервисов. */

// Boost
#include <boost/cast.hpp>

// SDK
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/EventTypes.h>

// Project
#include "System/IApplication.h"
#include "Services/ServiceNames.h"
#include "Services/EventService.h"
#include "Services/PrintingService.h"
#include "Services/FundsService.h"
#include "Services/HIDService.h"
#include "Services/DeviceService.h"
#include "Services/CryptService.h"
#include "Services/DatabaseService.h"
#include "Services/PluginService.h"
#include "Services/SettingsService.h"
#include "Services/NetworkService.h"
#include "Services/PaymentService.h"
#include "Services/GUIService.h"
#include "Services/TerminalService.h"
#include "Services/RemoteService.h"
#include "Services/HookService.h"
#include "Services/SchedulerService.h"
#include "Services/ServiceController.h"

namespace PP = SDK::PaymentProcessor;

namespace CServiceController
{
	const int ShutdownRetryInterval = 1300;

	const QString RestartParameters = "RESTART_PARAMETERS";
}

//---------------------------------------------------------------------------
ServiceController::ServiceController(IApplication * aApplication) :
	mApplication(aApplication), 
	mFinalizeTimer(nullptr), 
	mReturnCode(0)
{
}

//---------------------------------------------------------------------------
ServiceController::~ServiceController()
{
	shutdownServices();
}

//---------------------------------------------------------------------------
void ServiceController::registerService(PP::IService * aService)
{
	mRegisteredServices.insert(aService->getName(), aService);
}

//---------------------------------------------------------------------------
void ServiceController::onEvent(const PP::Event & aEvent)
{
	switch (aEvent.getType())
	{
		case PPSDK::EEventType::Shutdown:
		{
			shutdownMachine();
			break;
		}

		case PPSDK::EEventType::Reboot:
		{
			rebootMachine();
			break;
		}

		case PPSDK::EEventType::Restart:
		{
			if (aEvent.hasData())
			{
				mUserProperties[CServiceController::RestartParameters] = aEvent.getData();
			}

			restartApplication();
			break;
		}

		case PP::EEventType::CloseApplication:
		case PP::EEventType::TerminateApplication:
		{
			shutdownServices();
			break;
		}

		case PP::EEventType::ReinitializeServices:
		{
			reinitializeServices();
			break;
		}
	
		// Остановка всего набора приложений. Вызывается из сервисного меню.
		case PPSDK::EEventType::StopSoftware:
		{
			mReturnCode = aEvent.getData().toMap().take("returnCode").toInt();

			auto wsClient = TerminalService::instance(mApplication)->getClient();

			if (wsClient && wsClient->isConnected())
			{
				wsClient->stopService();
			}
			else
			{
				EventService::instance(mApplication)->sendEvent(PPSDK::EEventType::CloseApplication, QVariant());
			}
			break;
		}
	}
}

//---------------------------------------------------------------------------
bool ServiceController::initializeServices()
{
	// Создаем EventService.
	EventService * eventService = new EventService();
	eventService->initialize();

	eventService->subscribe(this, SLOT(onEvent(const SDK::PaymentProcessor::Event &)));

	// Создаем необходимые сервисы.
	registerService(eventService);
	registerService(new PrintingService(mApplication));
	registerService(new FundsService(mApplication));
	registerService(new HIDService(mApplication));
	registerService(new SettingsService(mApplication));
	registerService(new DeviceService(mApplication));
	registerService(new DatabaseService(mApplication));
	registerService(new CryptService(mApplication));
	registerService(new PluginService(mApplication));
	registerService(new NetworkService(mApplication));
	registerService(new GUIService(mApplication));
	registerService(new PaymentService(mApplication));
	registerService(new TerminalService(mApplication));
	registerService(new RemoteService(mApplication));
	registerService(new HookService(mApplication));
	registerService(new SchedulerService(mApplication));

	mInitializedServices.clear();
	mFailedServices.clear();
	mShutdownOrder.clear();

	// Запуск процесса инициализации сервисов.
	mInitializedServices.insert(eventService->getName());
	mShutdownOrder.prepend(eventService);

	for (int pass = 0; pass < mRegisteredServices.size(); pass++)
	{
		foreach (PP::IService * service, mRegisteredServices)
		{
			// Если сервис не был инициализирован и инициализированы все зависимости, то инициализируем его.
			if (!mInitializedServices.contains(service->getName()) && !mFailedServices.contains(service->getName())
				&& mInitializedServices.contains(service->getRequiredServices()))
			{
				LOG(mApplication->getLog(), LogLevel::Normal, QString("Initializing %1.").arg(service->getName()));

				if (service->initialize())
				{
					LOG(mApplication->getLog(), LogLevel::Normal, QString("Service %1 was initialized successfully.").arg(service->getName()));

					mInitializedServices.insert(service->getName());
					mShutdownOrder.prepend(service);
				}
				else
				{
					mFailedServices.insert(service->getName());
				}
			}
		}
	}

	if (mRegisteredServices.size() == mInitializedServices.size())
	{
		foreach (PP::IService * service, mRegisteredServices)
		{
			service->finishInitialize();
		}

		auto watchServiceClient = TerminalService::instance(mApplication)->getClient();
		watchServiceClient->subscribeOnDisconnected(this);
		watchServiceClient->subscribeOnCloseCommandReceived(this);

		initializeCoreItems();

		return true;
	}

	return false;
}

//---------------------------------------------------------------------------
void ServiceController::initializeCoreItems()
{
	auto pluginLoader = PluginService::instance(mApplication)->getPluginLoader();
	QStringList corePlugins = pluginLoader->getPluginList(QRegExp(QString("PaymentProcessor\\.%1\\..*").arg(PPSDK::CComponents::CoreItem)));

	foreach(const QString & pluginName, corePlugins)
	{
		LOG(mApplication->getLog(), LogLevel::Normal, QString("Create core item: %1.").arg(pluginName));

		auto plugin = pluginLoader->createPlugin(pluginName);

		if (plugin)
		{
			mCorePluginList << plugin;
		}
	}
}

//---------------------------------------------------------------------------
void ServiceController::onDisconnected()
{
	try
	{
		getEventService()->sendEvent(PPSDK::Event(PPSDK::EEventType::CloseApplication));
	}
	catch (std::bad_cast)
	{
		LOG(mApplication->getLog(), LogLevel::Fatal, "Event service was destroyed. Unable to send event 'CloseApplication' by Disconnected.");
	}
}

//---------------------------------------------------------------------------
void ServiceController::onCloseCommandReceived()
{
	try
	{
		getEventService()->sendEvent(PPSDK::Event(PPSDK::EEventType::CloseApplication));
	}
	catch (std::bad_cast)
	{
		LOG(mApplication->getLog(), LogLevel::Fatal, "Event service was destroyed. Unable to send event 'CloseApplication' by CloseCommand.");
	}
}

//---------------------------------------------------------------------------
bool ServiceController::finalizeServices(const char * aRetrySlot)
{
	auto delayFinalize = [&](const PP::IService * aService) -> bool
	{
		if (aService)
		{
			LOG(mApplication->getLog(), LogLevel::Warning, QString("Service %1 cannot be shutdown now, will try later.").arg(aService->getName()));
		}

		// Если не получилось, повторяем попытку через некоторое время.
		mFinalizeTimer = new QTimer(this);
		mFinalizeTimer->setSingleShot(true);
		QObject::connect(mFinalizeTimer, SIGNAL(timeout()), this, aRetrySlot);

		mFinalizeTimer->start(CServiceController::ShutdownRetryInterval);
		return false;
	};

	if (mFinalizeTimer)
	{
		delete mFinalizeTimer;
		mFinalizeTimer = nullptr;
	}

	if (!canShutdown())
	{
		return delayFinalize(nullptr);
	}

	finalizeCoreItems();

	// Пробуем остановить сервис.
	while (!mShutdownOrder.empty())
	{
		PP::IService * service = mShutdownOrder.front();
		QString serviceName = service->getName();

		LOG(mApplication->getLog(), LogLevel::Normal, QString("Trying to shutdown service %1...").arg(serviceName));

		if (service->shutdown())
		{
			mRegisteredServices.remove(service->getName());
			mShutdownOrder.pop_front();
			delete service;
			service = nullptr;

			LOG(mApplication->getLog(), LogLevel::Debug, QString("Service %1 shutdown OK...").arg(serviceName));
		}
		else
		{
			return delayFinalize(service);
		}
	}

	return true;
}

//---------------------------------------------------------------------------
void ServiceController::finalizeCoreItems()
{
	if (mCorePluginList.isEmpty())
	{
		return;
	}

	PluginService * ps = PluginService::instance(mApplication);
	
	if (ps)
	{
		auto pluginLoader = ps->getPluginLoader();

		foreach(auto coreItem, mCorePluginList)
		{
			LOG(mApplication->getLog(), LogLevel::Normal, QString("Destroy core item: %1.").arg(coreItem->getPluginName()));

			pluginLoader->destroyPlugin(coreItem);
		}
	}

	mCorePluginList.clear();
}

//---------------------------------------------------------------------------
bool ServiceController::canShutdown()
{
	foreach(auto service, mShutdownOrder)
	{
		if (!service->canShutdown())
		{
			LOG(mApplication->getLog(), LogLevel::Warning, QString("Service %1 cannot be shutdown now, will try later.").arg(service->getName()));

			return false;
		}
	}

	return true;
}

//---------------------------------------------------------------------------
void ServiceController::shutdownServices()
{
	if (finalizeServices(SLOT(shutdownServices())))
	{
		LOG(mApplication->getLog(), LogLevel::Normal, "Exit from ServiceController.");

		emit exit(mReturnCode);
	}
}

//---------------------------------------------------------------------------
void ServiceController::reinitializeServices()
{
	if (finalizeServices(SLOT(reinitializeServices())))
	{
		initializeServices();
	}
}

//---------------------------------------------------------------------------
void ServiceController::rebootMachine()
{
	if (canShutdown())
	{
		TerminalService::instance(mApplication)->getClient()->rebootMachine();
	}
	else
	{
		// Если не получилось, повторяем попытку через некоторое время.
		QTimer::singleShot(CServiceController::ShutdownRetryInterval, this, SLOT(rebootMachine()));
	}
}

//---------------------------------------------------------------------------
void ServiceController::restartApplication()
{
	if (canShutdown())
	{
		TerminalService::instance(mApplication)->getClient()->restartService(mUserProperties[CServiceController::RestartParameters].toStringList());
	}
	else
	{
		// Если не получилось, повторяем попытку через некоторое время.
		QTimer::singleShot(CServiceController::ShutdownRetryInterval, this, SLOT(restartApplication()));
	}
}

//---------------------------------------------------------------------------
void ServiceController::shutdownMachine()
{
	if (canShutdown())
	{
		TerminalService::instance(mApplication)->getClient()->shutdownMachine();
	}
	else
	{
		// Если не получилось, повторяем попытку через некоторое время.
		QTimer::singleShot(CServiceController::ShutdownRetryInterval, this, SLOT(shutdownMachine()));
	}
}

//---------------------------------------------------------------------------
void ServiceController::dumpFailureReport()
{
	QString failureInfo;
	foreach (const QString & serviceName, mFailedServices)
	{
		failureInfo += QString(" ") + serviceName;
	}

	// Выводим отчет о зависимостях.
	QString details;

	foreach (const PP::IService * service, mRegisteredServices.values())
	{
		QSet<QString> requiredSet = service->getRequiredServices();

		requiredSet.intersect(mFailedServices);

		if (!requiredSet.empty())
		{
			details += "\nService " + service->getName() + " requires service ";

			// Выводим список неинициализированных зависимостей.
			foreach (const QString & serviceName, requiredSet)
			{
				details += serviceName + ",";
			}

			details += " initialization skipped.";
		}
	}

	LOG(mApplication->getLog(), LogLevel::Fatal, QString("Services: %1 failed to initialize due to internal errors. %2").arg(failureInfo).arg(details));
}

//---------------------------------------------------------------------------
QSet<SDK::PaymentProcessor::IService *> ServiceController::getServices() const
{
	return mRegisteredServices.values().toSet();
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::IRemoteService * ServiceController::getRemoteService() const
{
	return boost::polymorphic_cast<PP::IRemoteService *>(mRegisteredServices.value(CServices::RemoteService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::IPaymentService * ServiceController::getPaymentService() const
{
	return boost::polymorphic_cast<PP::IPaymentService *>(mRegisteredServices.value(CServices::PaymentService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::IFundsService * ServiceController::getFundsService() const
{
	return boost::polymorphic_cast<PP::IFundsService *>(mRegisteredServices.value(CServices::FundsService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::IPrinterService * ServiceController::getPrinterService() const
{
	return boost::polymorphic_cast<PP::IPrinterService *>(mRegisteredServices.value(CServices::PrintingService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::IHIDService * ServiceController::getHIDService() const
{
	return boost::polymorphic_cast<PP::IHIDService *>(mRegisteredServices.value(CServices::HIDService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::INetworkService * ServiceController::getNetworkService() const
{
	return boost::polymorphic_cast<PP::INetworkService *>(mRegisteredServices.value(CServices::NetworkService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::IEventService * ServiceController::getEventService() const
{
	return boost::polymorphic_cast<PP::IEventService *>(mRegisteredServices.value(CServices::EventService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::IGUIService * ServiceController::getGUIService() const
{
	return boost::polymorphic_cast<PP::IGUIService *>(mRegisteredServices.value(CServices::GUIService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::IDeviceService * ServiceController::getDeviceService() const
{
	return boost::polymorphic_cast<PP::IDeviceService *>(mRegisteredServices.value(CServices::DeviceService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::ICryptService * ServiceController::getCryptService() const
{
	return boost::polymorphic_cast<PP::ICryptService *>(mRegisteredServices.value(CServices::CryptService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::ISettingsService * ServiceController::getSettingsService() const
{
	return boost::polymorphic_cast<PP::ISettingsService *>(mRegisteredServices.value(CServices::SettingsService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::IDatabaseService * ServiceController::getDatabaseService() const
{
	return boost::polymorphic_cast<PP::IDatabaseService *>(mRegisteredServices.value(CServices::DatabaseService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::ITerminalService * ServiceController::getTerminalService() const
{
	return boost::polymorphic_cast<PP::ITerminalService *>(mRegisteredServices.value(CServices::TerminalService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::IService * ServiceController::getService(const QString & aServiceName) const
{
	if (mRegisteredServices.isEmpty())
	{
		return nullptr;
	}
	
	if (mRegisteredServices.contains(aServiceName))
	{
		return mRegisteredServices.value(aServiceName);
	}

	throw PPSDK::ServiceIsNotImplemented(aServiceName);
}

//---------------------------------------------------------------------------
QVariantMap & ServiceController::getUserProperties()
{
	return mUserProperties;
}

//---------------------------------------------------------------------------
