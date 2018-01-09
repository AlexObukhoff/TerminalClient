/* @file Сервис для работы с устройствами приема наличных. */

#include <numeric>

// PaymentProcessor SDK
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/EventTypes.h>
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/IChargeProvider.h>
#include <SDK/PaymentProcessor/Core/ServiceParameters.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

// Driver SDK
#include <SDK/Drivers/WarningLevel.h>
#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/HardwareConstants.h>

// Проект
#include "System/IApplication.h"
#include "Services/FundsService.h"
#include "Services/ServiceNames.h"
#include "Services/DeviceService.h"
#include "Services/DatabaseService.h"
#include "Services/SettingsService.h"
#include "Services/PluginService.h"
#include "DatabaseUtils/IHardwareDatabaseUtils.h"
#include "Services/CashAcceptorManager.h"
#include "Services/CashDispenserManager.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
FundsService * FundsService::instance(IApplication * aApplication)
{
	return static_cast<FundsService *>(aApplication->getCore()->getService(CServices::FundsService));
}

//---------------------------------------------------------------------------
FundsService::FundsService(IApplication * aApplication)
	: ILogable(CFundsService::LogName),
	  mApplication(aApplication),
	  mCashDispenserManager(nullptr),
	  mCashAcceptorManager(nullptr)
{
}

//---------------------------------------------------------------------------
FundsService::~FundsService()
{
}

//---------------------------------------------------------------------------
bool FundsService::initialize()
{
	if (!mCashAcceptorManager)
	{
		mCashAcceptorManager = new CashAcceptorManager(mApplication);
		mCashAcceptorManager->setParent(this);
	}

	if (!mCashDispenserManager)
	{
		mCashDispenserManager = new CashDispenserManager(mApplication);
		mCashDispenserManager->setParent(this);
	}

	auto database = DatabaseService::instance(mApplication)->getDatabaseUtils<IPaymentDatabaseUtils>();

	return mCashAcceptorManager->initialize(database) && mCashDispenserManager->initialize(database);
}

//------------------------------------------------------------------------------
void FundsService::finishInitialize()
{
}

//---------------------------------------------------------------------------
bool FundsService::canShutdown()
{
	return true;
}

//---------------------------------------------------------------------------
bool FundsService::shutdown()
{
	if (mCashAcceptorManager)
	{
		mCashAcceptorManager->shutdown();
		delete mCashAcceptorManager;
		mCashAcceptorManager = nullptr;
	}

	if (mCashDispenserManager)
	{
		mCashDispenserManager->shutdown();
		delete mCashDispenserManager;
		mCashDispenserManager = nullptr;
	}

	return true;
}

//---------------------------------------------------------------------------
QString FundsService::getName() const
{
	return CServices::FundsService;
}

//---------------------------------------------------------------------------
const QSet<QString> & FundsService::getRequiredServices() const
{
	static QSet<QString> requiredServices = QSet<QString>()
		<< CServices::SettingsService
		<< CServices::DeviceService
		<< CServices::DatabaseService;

	return requiredServices;
}

//---------------------------------------------------------------------------
QVariantMap FundsService::getParameters() const
{
	QVariantMap parameters;

	parameters[PPSDK::CServiceParameters::Funds::RejectCount] = mCashAcceptorManager->getRejectCount();

	return parameters;
}

//---------------------------------------------------------------------------
void FundsService::resetParameters(const QSet<QString> & aParameters)
{
	if (aParameters.contains(PPSDK::CServiceParameters::Funds::RejectCount))
	{
		auto dbUtils = DatabaseService::instance(mApplication)->getDatabaseUtils<IHardwareDatabaseUtils>();
		dbUtils->setDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal, PPSDK::CDatabaseConstants::Parameters::RejectCount, 0);
	}
}

//------------------------------------------------------------------------------
QString FundsService::getState() const
{
	// Получаем список всех доступных устройств.
	PPSDK::TerminalSettings * settings = SettingsService::instance(mApplication)->getAdapter<PPSDK::TerminalSettings>();
	QStringList deviceList = settings->getDeviceList().filter(QRegExp(QString("(%1|%2)")
		.arg(DSDK::CComponents::BillAcceptor)
		.arg(DSDK::CComponents::CoinAcceptor)));

	QStringList result;

	foreach(const QString & configurationName, deviceList)
	{
		DSDK::IDevice * device = DeviceService::instance(mApplication)->acquireDevice(configurationName);
		if (device)
		{
			QStringList dd = device->getDeviceConfiguration().value(CHardwareSDK::DeviceData).toString().split("\n");
			if (!dd.isEmpty())
			{
				QVariantMap ddMap;
				foreach (QString param, dd)
				{
					if (param.isEmpty()) continue;

					QStringList pp = param.split(":");
					if (!pp.isEmpty())
					{
						ddMap.insert(pp.first().trimmed(), pp.last().trimmed());
					}
				}

				result 
					<< DeviceService::instance(mApplication)->getDeviceConfiguration(configurationName).value(CHardwareSDK::ModelName).toString()
					<< ddMap.value(CHardwareSDK::SerialNumber).toString()
					<< ddMap.value("firmware").toString();
			}
		}
	}	
	
	return result.join(";");
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::ICashAcceptorManager * FundsService::getAcceptor() const
{
	return mCashAcceptorManager;
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::ICashDispenserManager * FundsService::getDispenser() const
{
	return mCashDispenserManager;
}

//---------------------------------------------------------------------------
