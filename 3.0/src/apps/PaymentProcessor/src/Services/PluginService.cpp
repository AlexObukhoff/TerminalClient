/* @file Реализация менеджера плагинов. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QDirIterator>
#include <QtConcurrent/QtConcurrentRun>
#include <Common/QtHeadersEnd.h>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

// Modules
#include <SysUtils/ISysUtils.h>
#include <WatchServiceClient/Constants.h>

// Modules
#include <Common/Version.h>

// Plugin SDK
#include <SDK/Plugins/IPluginFactory.h>
#include <SDK/Plugins/PluginLoader.h>
#include <SDK/Plugins/IExternalInterface.h>

// Проект
#include "System/SettingsConstants.h"

#include "Services/ServiceNames.h"
#include "Services/PluginService.h"
#include "Services/EventService.h"

namespace CPluginService
{
	const QString CyberplatSignerName = "CYBERPLAT";
}

//------------------------------------------------------------------------------
PluginService * PluginService::instance(IApplication * aApplication)
{
	return static_cast<PluginService *>(aApplication->getCore()->getService(CServices::PluginService));
}

//------------------------------------------------------------------------------
PluginService::PluginService(IApplication * aApplication)
	: ILogable("Plugins"),
	mPluginLoader(nullptr)
{
	mApplication = aApplication;
}

//------------------------------------------------------------------------------
PluginService::~PluginService()
{
}

//------------------------------------------------------------------------------
bool PluginService::initialize()
{
	mPluginLoader = new SDK::Plugin::PluginLoader(this);

	mPluginLoader->addDirectory(mApplication->getPluginPath());
	mPluginLoader->addDirectory(mApplication->getUserPluginPath());

#ifndef _DEBUG
	// Запустим фоновую проверку плагинов на наличие цифровой подписи
	mPluginVerifierSynchronizer.addFuture(QtConcurrent::run(this, &PluginService::verifyPlugins));
#endif

	return true;
}

//------------------------------------------------------------------------------
void PluginService::finishInitialize()
{
}

//---------------------------------------------------------------------------
bool PluginService::canShutdown()
{
	return true;
}

//------------------------------------------------------------------------------
#ifndef SM_SHUTTINGDOWN
#define SM_SHUTTINGDOWN         0x2000
#endif

//------------------------------------------------------------------------------
bool PluginService::shutdown()
{
	// Не выгружаем библиотеки на выходе из ПО в процессе перезагрузки системы. #48972
	if (GetSystemMetrics(SM_SHUTTINGDOWN) == 0)
	{
		toLog(LogLevel::Debug, "Destroy plugins loader...");

		delete static_cast<SDK::Plugin::PluginLoader *>(mPluginLoader);
	}

	mPluginLoader = nullptr;

	toLog(LogLevel::Debug, "Plugin service shutdown OK.");

	return true;
}

//------------------------------------------------------------------------------
QString PluginService::getName() const
{
	return CServices::PluginService;
}

//------------------------------------------------------------------------------
const QSet<QString> & PluginService::getRequiredServices() const
{
	static QSet<QString> requiredResources;
	return requiredResources;
}

//------------------------------------------------------------------------------
QVariantMap PluginService::getParameters() const
{
	return QVariantMap();
}

//------------------------------------------------------------------------------
void PluginService::resetParameters(const QSet<QString> & )
{
}

//------------------------------------------------------------------------------
QString PluginService::getState() const
{
	QStringList result;

	if (mUnsignedPlugins.count())
	{
		result << QString("Unsigned : {%1}").arg(mUnsignedPlugins.join(";"));
	}

	QStringList signedKeys = mSignedPlugins.keys();
	signedKeys.removeDuplicates();

	foreach(QString signerName, signedKeys)
	{
		result << QString("Signed by %1 : {%2}").arg(signerName).arg(QStringList(mSignedPlugins.values(signerName)).join(";"));
	}

	return result.join(";");
}

//------------------------------------------------------------------------------
SDK::Plugin::IPluginLoader * PluginService::getPluginLoader()
{
	return mPluginLoader;
}

//------------------------------------------------------------------------------
ILog * PluginService::getLog(const QString & aName) const
{
	return aName.isEmpty() ? ILogable::getLog() : ILog::getInstance(aName);
}

//------------------------------------------------------------------------------
QString PluginService::getVersion() const
{
	return Cyberplat::getVersion();
}

//------------------------------------------------------------------------------
QString PluginService::getDirectory() const
{
	return mApplication->getWorkingDirectory();
}

//------------------------------------------------------------------------------
QString PluginService::getDataDirectory() const
{
	return mApplication->getUserDataPath();
}

//------------------------------------------------------------------------------
QString PluginService::getLogsDirectory() const
{
	return mApplication->getWorkingDirectory() + "/logs";
}

//------------------------------------------------------------------------------
QString PluginService::getPluginDirectory() const
{
	return mApplication->getPluginPath();
}

//------------------------------------------------------------------------------
bool PluginService::canConfigurePlugin(const QString & /*aInstancePath*/) const
{
	// Не используется.
	return false;
}

//------------------------------------------------------------------------------
QVariantMap PluginService::getPluginConfiguration(const QString & /*aInstancePath*/) const
{
	// Не используется.
	return QVariantMap();
}

//------------------------------------------------------------------------------
bool PluginService::canSavePluginConfiguration(const QString & /*aInstancePath*/) const
{
	// Не используется.
	return false;
}

//------------------------------------------------------------------------------
bool PluginService::savePluginConfiguration(const QString & /*aInstancePath*/, const QVariantMap & /*aParamenters*/)
{
	// Не используется.
	return false;
}

//------------------------------------------------------------------------------
SDK::Plugin::IExternalInterface * PluginService::getInterface(const QString & aInterface)
{
	if (aInterface == SDK::PaymentProcessor::CInterfaces::ICore)
	{
		return dynamic_cast<SDK::Plugin::IExternalInterface *>(mApplication->getCore());
	}

	throw SDK::PaymentProcessor::ServiceIsNotImplemented(aInterface);
}

//------------------------------------------------------------------------------
void PluginService::verifyPlugins()
{
#ifdef Q_OS_WIN
	mSignedPlugins.clear();
	mUnsignedPlugins.clear();
	
	auto shortPath = [=](const QString & aFullPath) -> QString
	{
		// Удалим расширение
		QString result = aFullPath.left(aFullPath.length() - 4).toLower();

		// Удалим путь к плагину/экзешнику
		return result.contains(mApplication->getUserPluginPath().toLower()) ?
			result.mid(mApplication->getUserPluginPath().length()) + ".u":
			result.mid(QString(mApplication->getWorkingDirectory() + QDir::separator() + (result.contains("plugins") ? "plugins" : "")).length());
	};

	QStringList modules = mPluginLoader->getPluginPathList(QRegExp(".*"));
	
	// Добавим проверку исполняемых файлов

	QStringList exeModules = QStringList()
		<< CWatchService::Modules::WatchService << CWatchService::Modules::PaymentProcessor
		<< CWatchService::Modules::Updater << CWatchService::Modules::WatchServiceController;

	foreach(QString module, exeModules)
	{
		QString file = QString("%1%2%3.exe").arg(mApplication->getWorkingDirectory()).arg(QDir::separator()).arg(module);
		
		if (QFileInfo(file).exists())
		{
			modules.append(file);
		}
	}
	
	foreach(QString fullPath, modules)
	{
		QString plugin = QDir::toNativeSeparators(fullPath).split(QDir::separator()).last();
		qlonglong status = ISysUtils::verifyTrust(QDir::toNativeSeparators(fullPath));

		toLog(LogLevel::Normal, QString("Verifying %1...").arg(fullPath));
			
		if (status == ERROR_SUCCESS)
		{
			ISysUtils::SSignerInfo signer;
			bool result = ISysUtils::getSignerInfo(QDir::toNativeSeparators(fullPath), signer);

			if (result)
			{
				if (signer.name != CPluginService::CyberplatSignerName)
				{
					mSignedPlugins.insertMulti(signer.name, shortPath(fullPath));
				}

				toLog(LogLevel::Normal, QString("Signed. Subject name: %1").arg(signer.name));
			}
			else
			{
				toLog(LogLevel::Warning, QString("Signed. Subject name is unknown."));

				mUnsignedPlugins.append(shortPath(fullPath));
			}
		}
		else 
		{
			toLog(LogLevel::Warning, QString("%1").arg(status == TRUST_E_NOSIGNATURE ?
				"No signature was present in the subject." :
				"Could not verify signer in the subject."));

			mUnsignedPlugins.append(shortPath(fullPath));
		}
	}

	try
	{
		auto * eventService = EventService::instance(mApplication);

		if (mUnsignedPlugins.count())
		{
			eventService->sendEvent(SDK::PaymentProcessor::Event(SDK::PaymentProcessor::EEventType::Warning, getName(),
				QString("Unsigned : {%1}").arg(mUnsignedPlugins.join(";"))));
		}

		QStringList signedKeys = mSignedPlugins.keys();
		signedKeys.removeDuplicates();

		foreach(QString signerName, signedKeys)
		{
			eventService->sendEvent(SDK::PaymentProcessor::Event(SDK::PaymentProcessor::EEventType::Warning, getName(),
				QString("Signed by %1 : {%2}").arg(signerName).arg(QStringList(mSignedPlugins.values(signerName)).join(";"))));
		}
	}
	catch (SDK::PaymentProcessor::ServiceIsNotImplemented & e)
	{
		toLog(LogLevel::Error, "Exception accured while verify plugins.");
	}

#else
	#pragma warning "PluginService::verifyPlugins not implemented on this platfotm."
#endif
}

//------------------------------------------------------------------------------