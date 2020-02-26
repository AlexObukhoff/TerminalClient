/* @file Плагин сценария для тестирования оборудования */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QByteArray>
#include <QtCore/QFile>
#include <QtCore/QPair>
#include <QtCore/QSettings>
#include <Common/QtHeadersEnd.h>

// Stl
#include <string>

// Boost
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>

// Plugin SDK
#include <SDK/Plugins/PluginInitializer.h>
#include <SDK/Plugins/IExternalInterface.h>

// PP SDK
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>
#include <SDK/PaymentProcessor/Core/IPrinterService.h>
#include <SDK/PaymentProcessor/Core/INetworkService.h>
#include <SDK/PaymentProcessor/Core/ICryptService.h>
#include <SDK/PaymentProcessor/Core/IDeviceService.h>
#include <SDK/PaymentProcessor/Core/IDatabaseService.h>
#include <SDK/PaymentProcessor/Scripting/Core.h>
#include <SDK/Drivers/DeviceTypes.h>
#include <SDK/GUI/IGraphicsHost.h>

// Modules
#include <Common/ExitCodes.h>
#include <Crypt/ICryptEngine.h>

// Проект
#include "ScenarioPlugin.h"
#include "MainScenario.h"

//--------------------------------------------------------------------------
namespace
{
	/// Конструктор плагина.
	SDK::Plugin::IPlugin * CreatePlugin(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
	{
		return new Migrator3000::MainScenarioPlugin(aFactory, aInstancePath);
	}
}

REGISTER_PLUGIN(SDK::Plugin::makePath(SDK::PaymentProcessor::Application, PPSDK::CComponents::ScenarioFactory,
	CScenarioPlugin::PluginName), &CreatePlugin);

namespace Migrator3000
{

//---------------------------------------------------------------------------
MainScenario::MainScenario(SDK::PaymentProcessor::ICore * aCore, ILog * aLog)
	: Scenario(CScenarioPlugin::PluginName, aLog),
	mCore(aCore),
	mNetworkService(aCore->getNetworkService()),
	mTerminalService(aCore->getTerminalService()),
	mSettingsService(aCore->getSettingsService()),
	mCryptService(aCore->getCryptService()),
	mDeviceService(aCore->getDeviceService()),
	mMonitoringComandId(-1)
{
	mTerminalSettings = static_cast<PPSDK::TerminalSettings *>
		(mCore->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter));

	Q_ASSERT(mTerminalSettings);

	mScriptingCore = static_cast<SDK::PaymentProcessor::Scripting::Core *>
		(dynamic_cast<SDK::GUI::IGraphicsHost *>(mCore->getGUIService())->
		getInterface<QObject>(SDK::PaymentProcessor::Scripting::CProxyNames::Core));
	
	Q_ASSERT(mScriptingCore);

	connect(&mTaskWatcher, SIGNAL(finished()), SLOT(onTaskFinished()));
}

//---------------------------------------------------------------------------
MainScenario::~MainScenario()
{
}

//---------------------------------------------------------------------------
bool MainScenario::initialize(const QList<GUI::SScriptObject> & /*aScriptObjects*/)
{
	return true;
}

//---------------------------------------------------------------------------
void MainScenario::start(const QVariantMap & aContext)
{
	setStateTimeout(0);

	mContext = aContext;
	QStringList args = mContext["args"].toString().split(";");
	mKiosk2InstallPath = args.first().split("#").last();
	mMonitoringComandId = args.last().split("#").last().toInt();

	QByteArray sign;
	QString error;

	//check keys
	if (!mCryptService->getCryptEngine()->sign(0, "Hello Cyberplat", sign, error))
	{
		toLog(LogLevel::Error, QString("CHECK keys error, %1").arg(error));
		signalTriggered("abort", QVariantMap());
		return;
	}

	toLog(LogLevel::Normal, QString("CHECK keys OK"));

	//setup connection
	PPSDK::SConnection connection;

	using boost::property_tree::ptree;
	ptree pt;

	QString terminalConfig = mKiosk2InstallPath + "/config/terminal.xml";

	try
	{
		read_xml(terminalConfig.toStdString(), pt);

		BOOST_FOREACH(ptree::value_type const & v, pt.get_child("terminal"))
		{
			if (v.first == "connection")
			{
				connection.type = QString::fromStdString(v.second.get<std::string>("<xmlattr>.type", "")) == "local" ? EConnectionTypes::Unmanaged : EConnectionTypes::Dialup;
				connection.name = QString::fromStdString(v.second.get<std::string>("<xmlattr>.name", ""));

				QNetworkProxy proxy;

				auto proxyType = QString::fromStdString(v.second.get<std::string>("proxy.<xmlattr>.type", ""));

				if (proxyType == "http")
				{
					proxy.setType(QNetworkProxy::HttpProxy);
				}
				else if (proxyType == "http_caching")
				{
					proxy.setType(QNetworkProxy::HttpCachingProxy);
				}
				else if (proxyType == "socks5")
				{
					proxy.setType(QNetworkProxy::Socks5Proxy);
				}
				else
				{
					proxy.setType(QNetworkProxy::NoProxy);
				}

				if (proxy.type() != QNetworkProxy::NoProxy)
				{
					proxy.setUser(QString::fromStdString(v.second.get<std::string>("proxy.<xmlattr>.username", "")));
					proxy.setPassword(QString::fromStdString(v.second.get<std::string>("proxy.<xmlattr>.password", "")));
					proxy.setHostName(QString::fromStdString(v.second.get<std::string>("proxy.<xmlattr>.host", "")));
					proxy.setPort(QString::fromStdString(v.second.get<std::string>("proxy.<xmlattr>.port", "0")).toUShort());
				}

				connection.proxy = proxy;
			}
		}
	}
	catch (boost::property_tree::xml_parser_error & e)
	{
		toLog(LogLevel::Error, QString("PARSING '%1' error, %2").arg(terminalConfig).arg(QString::fromStdString(e.message())));
		signalTriggered("abort", QVariantMap());
		return;
	}

	mNetworkService->setConnection(connection);
	mTerminalSettings->setConnection(connection);

	//test connection
	if (!mNetworkService->testConnection())
	{
		toLog(LogLevel::Error, QString("CHECK connection error, %1").arg(mNetworkService->getLastConnectionError().split(":").last()));
		signalTriggered("abort", QVariantMap());
		return;
	}

	toLog(LogLevel::Normal, QString("CHECK connection OK"));

	// start find devices
	connect(mDeviceService, SIGNAL(deviceDetected(const QString &)), this, SLOT(onDeviceDetected(const QString &)));
	connect(mDeviceService, SIGNAL(detectionStopped()), this, SLOT(onDetectionStopped()));

	toLog(LogLevel::Normal, QString("START autodetect."));

	mFoundedDevices.clear();
	mDeviceService->detect();
}

//---------------------------------------------------------------------------
void MainScenario::stop()
{
	mTimeoutTimer.stop();

	disconnect(mDeviceService, SIGNAL(deviceDetected(const QString &)), this, SLOT(onDeviceDetected(const QString &)));
	disconnect(mDeviceService, SIGNAL(detectionStopped()), this, SLOT(onDetectionStopped()));
}

//---------------------------------------------------------------------------
void MainScenario::pause()
{
	mTimeoutTimer.stop();
}

//---------------------------------------------------------------------------
void MainScenario::resume(const QVariantMap & aContext)
{
}

//---------------------------------------------------------------------------
void MainScenario::signalTriggered(const QString & aSignal, const QVariantMap & /*aArguments*/)
{
	QVariantMap parameters;
	int returnCode = -1;

	if (aSignal == "abort")
	{
		parameters.insert("result", "abort");
		returnCode = ExitCode::Error;

		mTimeoutTimer.stop();
		emit finished(parameters);
	}
	else if (aSignal == "finish")
	{
		returnCode = ExitCode::NoError;
		
		mTimeoutTimer.stop();
		emit finished(parameters);
	}

	//abort/finish - завершаем сценарий, закрываем приложение
	if (returnCode == ExitCode::NoError || returnCode == ExitCode::Error)
	{
		QVariantMap parameters;
		parameters.insert("returnCode", returnCode);
		mScriptingCore->postEvent(PPSDK::EEventType::StopSoftware, parameters);
	}
}

//---------------------------------------------------------------------------
QString MainScenario::getState() const
{
	return QString("main");
}

//---------------------------------------------------------------------------
void MainScenario::onTimeout()
{
	if (mTaskWatcher.isRunning())
	{
		mTaskWatcher.waitForFinished();
	}

	signalTriggered("finish", QVariantMap());
}

//---------------------------------------------------------------------------
void MainScenario::onTaskFinished()
{
	signalTriggered("finish", QVariantMap());
}

//--------------------------------------------------------------------------
bool MainScenario::canStop()
{
	return true;
}

//---------------------------------------------------------------------------
void MainScenario::onDeviceDetected(const QString & aConfigName)
{
	toLog(LogLevel::Normal, QString("DETECT device %1").arg(aConfigName));
	mFoundedDevices.append(aConfigName);
}

//---------------------------------------------------------------------------
void MainScenario::onDetectionStopped()
{
	toLog(LogLevel::Normal, QString("STOP autodetect. WAIT device init."));
	
	// Подождем, чтобы все устройства успели проинициализироваться
	QTimer::singleShot(40000, this, SLOT(finishDeviceDetection()));
}

//---------------------------------------------------------------------------
void MainScenario::finishDeviceDetection()
{
	toLog(LogLevel::Normal, QString("INIT devices done."));

	// update configs
	mDeviceService->saveConfigurations(mFoundedDevices);
	mSettingsService->saveConfiguration();

	// todo check validator/printer config settings
	QStringList configurations = mDeviceService->getConfigurations();
	
	auto isDeviceOK = [=](const QString & aDeviceType) -> bool
	{
		namespace DSDK = SDK::Driver;

		foreach(QString config, configurations)
		{
			if (config.section('.', 2, 2) == aDeviceType)
			{
				auto status = mDeviceService->getDeviceStatus(config);

				return status && status->isMatched(DSDK::EWarningLevel::Warning);
			}
		}

		return false;
	};

	bool validatorOK = isDeviceOK(SDK::Driver::CComponents::BillAcceptor);

	if (!validatorOK)
	{
		toLog(LogLevel::Error, QString("BILL VALIDATOR error or not found."));
		signalTriggered("abort");
		return;
	}

	toLog(LogLevel::Normal, QString("BILL VALIDATOR is OK."));

	bool printerOK = isDeviceOK(SDK::Driver::CComponents::Printer) ||
		isDeviceOK(SDK::Driver::CComponents::DocumentPrinter) ||
		isDeviceOK(SDK::Driver::CComponents::FiscalRegistrator);

	bool blockTerminalByPrinter = mTerminalSettings->getCommonSettings().blockOn(SDK::PaymentProcessor::SCommonSettings::PrinterError);

	if (!printerOK && blockTerminalByPrinter)
	{
		toLog(LogLevel::Error, QString("PRINTER %1, BLOCK terminal by printer = %2.").arg(printerOK ? "OK" : "error").arg(blockTerminalByPrinter ? "YES" : "NO"));
		signalTriggered("abort");
		return;
	}

	toLog(LogLevel::Error, QString("PRINTER is OK."));

	//fix standalone flag
	{
		QSettings ini("client.ini", QSettings::IniFormat);
		ini.setIniCodec("UTF-8");
		ini.setValue("common/standalone", false);

		if (ini.status() != QSettings::NoError)
		{
			toLog(LogLevel::Error, QString("UPDATE standalone flag error: %1.").arg(ini.status() == QSettings::AccessError ? 
				"An access error occurred" : "A format error occurred"));

			signalTriggered("abort");
			return;
		}

		toLog(LogLevel::Normal, QString("UPDATE standalone flag OK."));
	}

	auto queryStr = QString("INSERT INTO `command` (`id`, `type`, `parameters`, `receive_date`, `status`, `last_update`, `on_monitoring`, `description`, `tag`) \
							VALUES(%1, 18, \"""\", \"%2\", 3, %1, 0, \"OK\", 10)")
									.arg(mMonitoringComandId)
									.arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));

	if (!mCore->getDatabaseService()->execQuery(queryStr))
	{
		toLog(LogLevel::Error, QString("UPDATE monitoring command error."));
		signalTriggered("abort");
		return;
	}

	toLog(LogLevel::Normal, QString("UPDATE monitoring command OK."));
	
	signalTriggered("finish", QVariantMap());
}

}

//---------------------------------------------------------------------------
