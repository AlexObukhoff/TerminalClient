/* @file Бэкэнд для сервисного меню; обеспечивает доступ к основным сервисам ядра */

//Qt
#include "Common/QtHeadersBegin.h"
#include <QtCore/QCryptographicHash>
#include <QtCore/QtConcurrentRun>
#include "Common/QtHeadersEnd.h"

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Core/ITerminalService.h>
#include <SDK/PaymentProcessor/Core/IRemoteService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/PaymentProcessor/Core/IPrinterService.h>
#include <SDK/PaymentProcessor/Core/IFundsService.h>
#include <SDK/Plugins/IPluginEnvironment.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/Plugins/PluginInitializer.h>
#include <SDK/Plugins/IPluginLoader.h>
#include <SDK/GUI/IGraphicsItem.h>

// Project
#include "GUI/ServiceTags.h"
#include "MessageBox.h"
#include "HardwareManager.h"
#include "KeysManager.h"
#include "NetworkManager.h"
#include "PaymentManager.h"
#include "ServiceMenuBackend.h"

namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------
ServiceMenuBackend::ServiceMenuBackend(SDK::Plugin::IEnvironment * aFactory, ILog * aLog)
	: mFactory(aFactory),
	  mLog(aLog),
	  mAutoEncashmentEnabled(false),
	  mAuthorizationEnabled(true)
{
	mCore = dynamic_cast<SDK::PaymentProcessor::ICore *>(mFactory->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));
	
	MessageBox::initialize(mCore->getGUIService());

	mTerminalSettings = static_cast<SDK::PaymentProcessor::TerminalSettings *>(mCore->getSettingsService()->
		getAdapter(SDK::PaymentProcessor::CAdapterNames::TerminalAdapter));

	connect(&mHearthbeatTimer, SIGNAL(timeout()), this, SLOT(sendHearthbeat()));
}

//------------------------------------------------------------------------
ServiceMenuBackend::~ServiceMenuBackend()
{
	MessageBox::shutdown();

	foreach(SDK::Plugin::IPlugin * plugin, mWidgetPluginList)
	{
		mFactory->getPluginLoader()->destroyPlugin(plugin);
	}

	mWidgetPluginList.clear();
}

//------------------------------------------------------------------------
HardwareManager * ServiceMenuBackend::getHardwareManager()
{
	if (mHardwareManager.isNull())
	{
		mHardwareManager = QSharedPointer<HardwareManager>(new HardwareManager(mFactory, mCore));
		mConfigList << mHardwareManager.data();
	}

	return mHardwareManager.data();
}

//------------------------------------------------------------------------
KeysManager * ServiceMenuBackend::getKeysManager()
{
	if (mKeysManager.isNull())
	{
		mKeysManager = QSharedPointer<KeysManager>(new KeysManager(mCore));
		mConfigList << mKeysManager.data();
	}

	return mKeysManager.data();
}

//------------------------------------------------------------------------
NetworkManager * ServiceMenuBackend::getNetworkManager()
{
	if (mNetworkManager.isNull())
	{
		mNetworkManager = QSharedPointer<NetworkManager>(new NetworkManager(mCore));
		mConfigList << mNetworkManager.data();
	}

	return mNetworkManager.data();
}

//------------------------------------------------------------------------
PaymentManager * ServiceMenuBackend::getPaymentManager()
{
	if (mPaymentManager.isNull())
	{
		mPaymentManager = QSharedPointer<PaymentManager>(new PaymentManager(mCore));
	}

	return mPaymentManager.data();
}

//------------------------------------------------------------------------
void ServiceMenuBackend::toLog(const QString & aMessage)
{
	toLog(LogLevel::Normal, aMessage);
}

//------------------------------------------------------------------------
void ServiceMenuBackend::toLog(LogLevel::Enum aLevel, const QString & aMessage)
{
	mLog->write(aLevel, aMessage);
}

//------------------------------------------------------------------------
SDK::PaymentProcessor::ICore * ServiceMenuBackend::getCore() const
{
	return mCore;
}

//------------------------------------------------------------------------
void ServiceMenuBackend::getTerminalInfo(QVariantMap & aTerminalInfo)
{
	aTerminalInfo.clear();
	aTerminalInfo[CServiceTags::TerminalNumber] = mTerminalSettings->getKeys()[0].ap;
	aTerminalInfo[CServiceTags::SoftwareVersion] = mTerminalSettings->getAppEnvironment().version;

	// TODO Исправить на константу
	aTerminalInfo[CServiceTags::TerminalLocked] = dynamic_cast<SDK::PaymentProcessor::ITerminalService *>
		(mCore->getService("TerminalService"))->isLocked();
}

//------------------------------------------------------------------------
void ServiceMenuBackend::sendEvent(SDK::PaymentProcessor::EEventType::Enum aEventType)
{
	SDK::PaymentProcessor::Event e(aEventType, "");
	mCore->getEventService()->sendEvent(e);
}

//------------------------------------------------------------------------
void ServiceMenuBackend::sendEvent(SDK::PaymentProcessor::EEventType::Enum aEventType, const QVariantMap & aParameters)
{
	SDK::PaymentProcessor::Event e(aEventType, "", QVariant::fromValue(aParameters));
	mCore->getEventService()->sendEvent(e);
}

//------------------------------------------------------------------------
bool ServiceMenuBackend::isConfigurationChanged()
{
	bool result = false;

	foreach (IConfigManager * manager, mConfigList)
	{
		if (manager->isConfigurationChanged())
		{
			manager->resetConfiguration();
			result = true;
		}
	}

	return result;
}

//------------------------------------------------------------------------
bool ServiceMenuBackend::saveConfiguration()
{
	if (isConfigurationChanged())
	{
		try 
		{
			mCore->getSettingsService()->saveConfiguration();
		}
		catch(const std::exception &e) 
		{
			toLog(QString("Save configuration error (%1)").arg(e.what()));
		}
	}

	return true;
}

//------------------------------------------------------------------------
void ServiceMenuBackend::setConfiguration(const QVariantMap & aParameters)
{
	mParameters = aParameters;
}

//------------------------------------------------------------------------
QVariantMap ServiceMenuBackend::getConfiguration() const
{
	return mParameters;
}

//------------------------------------------------------------------------
void ServiceMenuBackend::saveDispenserUnitState()
{
	mCashUnitsState = mCore->getFundsService()->getDispenser()->getCashUnitsState();
}

//------------------------------------------------------------------------
void ServiceMenuBackend::printDispenserDiffState()
{
	PPSDK::TCashUnitsState curCashUnitsState = mCore->getFundsService()->getDispenser()->getCashUnitsState();
	if (mCashUnitsState != curCashUnitsState)
	{
		QVariantMap parameters;
		
		QStringList units;
		units << "FIRST" << "SECOND";

		for (int i = 0; i < units.count(); i++)
		{
			PPSDK::SCashUnit beforeUnit, afterUnit;

			const auto & beforeList = mCashUnitsState.values();
			if (beforeList.size())
			{
				if (beforeList.first().size() > i)
				{
					beforeUnit = beforeList.first()[i];
				}
			}

			const auto & afterList = curCashUnitsState.values();
			if (afterList.size())
			{
				if (afterList.first().size() > i)
				{
					afterUnit = afterList.first()[i];
				}
			}

			parameters.insert(QString("CU_%1").arg(units[i]), i + 1);
			parameters.insert(QString("CU_%1_NOMINAL_BEFORE").arg(units[i]), beforeUnit.nominal);
			parameters.insert(QString("CU_%1_COUNT_BEFORE").arg(units[i]), beforeUnit.count);
			parameters.insert(QString("CU_%1_AMOUNT_BEFORE").arg(units[i]), beforeUnit.amount());
			parameters.insert(QString("CU_%1_NOMINAL_AFTER").arg(units[i]), afterUnit.nominal);
			parameters.insert(QString("CU_%1_COUNT_AFTER").arg(units[i]), afterUnit.count);
			parameters.insert(QString("CU_%1_AMOUNT_AFTER").arg(units[i]), afterUnit.amount());
			parameters.insert(QString("CU_%1_DIFF").arg(units[i]), afterUnit.amount() - beforeUnit.amount());

			toLog("-------------------------------------------");
			toLog(QString("Cash unit %1").arg(i + 1));
			toLog(QString("Before: %1 * %2 = %3")
				.arg(beforeUnit.nominal)
				.arg(beforeUnit.count)
				.arg(beforeUnit.amount())
			);
			toLog(QString("After: %1 * %2 = %3")
				.arg(afterUnit.nominal)
				.arg(afterUnit.count)
				.arg(afterUnit.amount())
				);
			
			toLog(QString("Diff: %1").arg(afterUnit.amount() - beforeUnit.amount()));
		}

		QtConcurrent::run(mCore->getPrinterService(), &PPSDK::IPrinterService::printReceipt, QString(""), parameters, QString("dispenser_diff"), false, true);
	}
	else
	{
		toLog("Dispenser cash units state are not changed.");
	}
}

//------------------------------------------------------------------------
void ServiceMenuBackend::needUpdateConfigs()
{
	// TODO: убрать константу.
	dynamic_cast<SDK::PaymentProcessor::ITerminalService *>
		(mCore->getService("TerminalService"))->needUpdateConfigs();
}

//------------------------------------------------------------------------
bool ServiceMenuBackend::hasAnyPassword() const
{
	SDK::PaymentProcessor::SServiceMenuPasswords serviceMenuSettings = mTerminalSettings->getServiceMenuPasswords();
	bool admin = serviceMenuSettings.passwords[SDK::PaymentProcessor::CServiceMenuPasswords::Service].isEmpty();
	bool tech = serviceMenuSettings.passwords[SDK::PaymentProcessor::CServiceMenuPasswords::Technician].isEmpty();
	bool encash = serviceMenuSettings.passwords[SDK::PaymentProcessor::CServiceMenuPasswords::Collection].isEmpty();
	
	return !(admin && tech && encash);
}

//---------------------------------------------------------------------------
bool ServiceMenuBackend::authorize(const QString & aPassword)
{
	bool result = true;
	mAccessRights.clear();

	QString hash = QString::fromUtf8(QCryptographicHash::hash(aPassword.toLatin1(), QCryptographicHash::Md5).toHex());
	SDK::PaymentProcessor::SServiceMenuPasswords serviceMenuSettings = mTerminalSettings->getServiceMenuPasswords();

	// Роль администратора
	if (hash == serviceMenuSettings.passwords[SDK::PaymentProcessor::CServiceMenuPasswords::Service].toLower())
	{
		mAccessRights
			<< ServiceMenuBackend::Diagnostic
			<< ServiceMenuBackend::SetupHardware
			<< ServiceMenuBackend::SetupNetwork
			<< ServiceMenuBackend::SetupKeys
			<< ServiceMenuBackend::ViewPaymentSummary
			<< ServiceMenuBackend::ViewPayments
			<< ServiceMenuBackend::PrintReceipts
			<< ServiceMenuBackend::Encash
			<< ServiceMenuBackend::StopApplication
			<< ServiceMenuBackend::RebootTerminal
			<< ServiceMenuBackend::LockTerminal;

		mUserRole = CServiceTags::UserRole::RoleAdministrator;
	}
	// Роль техника
	else if (hash == serviceMenuSettings.passwords[SDK::PaymentProcessor::CServiceMenuPasswords::Technician].toLower())
	{
		mAccessRights
			<< ServiceMenuBackend::Diagnostic
			<< ServiceMenuBackend::SetupHardware
			<< ServiceMenuBackend::SetupNetwork
			<< ServiceMenuBackend::SetupKeys
			<< ServiceMenuBackend::StopApplication
			<< ServiceMenuBackend::RebootTerminal
			<< ServiceMenuBackend::LockTerminal;

		mUserRole = CServiceTags::UserRole::RoleTechnician;
	}
	// Роль инкассатора
	else if (hash == serviceMenuSettings.passwords[SDK::PaymentProcessor::CServiceMenuPasswords::Collection].toLower())
	{
		mAccessRights << ServiceMenuBackend::Encash;
		mUserRole = CServiceTags::UserRole::RoleCollector;
	}
	else
	{
		result = false;
	}

	return result;
}

//---------------------------------------------------------------------------
ServiceMenuBackend::TAccessRights ServiceMenuBackend::getAccessRights() const
{
	return mAccessRights;
}

//---------------------------------------------------------------------------
bool ServiceMenuBackend::isAuthorizationEnabled() const
{
	return mAuthorizationEnabled;
}

//------------------------------------------------------------------------
QList<QWidget *> ServiceMenuBackend::getExternalWidgets(bool aReset)
{
	QStringList plugins = mFactory->getPluginLoader()->getPluginList(QRegExp("PaymentProcessor\\.GraphicsItem\\..*\\ServiceMenuWidget"));

	if (mWidgetPluginList.isEmpty())
	{
		foreach(const QString & widget, plugins)
		{
			SDK::Plugin::IPlugin * plugin = mFactory->getPluginLoader()->createPlugin(widget);
			mWidgetPluginList << plugin;
		}
	}

	QList<QWidget *> widgetList;
	
	foreach(SDK::Plugin::IPlugin * plugin, mWidgetPluginList)
	{
		SDK::GUI::IGraphicsItem * itemObject = dynamic_cast<SDK::GUI::IGraphicsItem *>(plugin);
		if (itemObject)
		{
			if (aReset)
			{
				itemObject->reset(QVariantMap());
			}

			widgetList << itemObject->getNativeWidget();
		}
	}	

	return widgetList;
}

//------------------------------------------------------------------------
void ServiceMenuBackend::startHeartbeat()
{
	mHearthbeatTimer.start(CServiceMenuBackend::HeartbeatTimeout);
}

//------------------------------------------------------------------------
void ServiceMenuBackend::stopHeartbeat()
{
	mHearthbeatTimer.stop();
}

//------------------------------------------------------------------------
void ServiceMenuBackend::sendHearthbeat()
{
	mCore->getRemoteService()->sendHeartbeat();
}

//------------------------------------------------------------------------