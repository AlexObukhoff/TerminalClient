/* @file Класс приложения для PaymentProcessor. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTimer>
#include <QtCore/QThread>
#include <QtCore/QDir>
#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QThreadPool>
#include <QtWidgets/QDesktopWidget>
#include <QtGui/QPixmap>
#include <QtGui/QImage>
#include <QtGui/QSessionManager>
#include <Common/QtHeadersEnd.h>

// WinAPI
#ifdef Q_OS_WIN
#define NOMINMAX
#include <Windows.h>
#endif

// Модули
#include <Common/ExitCodes.h>
#include <SysUtils/ISysUtils.h>

// Проект
#include "Services/ServiceController.h"
#include "System/SettingsConstants.h"
#include "UnhandledException.h"
#include "PPApplication.h"

namespace PP = SDK::PaymentProcessor;

//------------------------------------------------------------------------
QString IApplication::toAbsolutePath(const QString & aPath)
{
	return QDir::cleanPath(QDir::fromNativeSeparators(
		QDir::isAbsolutePath(aPath) ? aPath : PPApplication::getInstance()->getWorkingDirectory() + "/" + aPath));
}

//------------------------------------------------------------------------
QString IApplication::getWorkingDirectory()
{
	return QDir::fromNativeSeparators(PPApplication::getInstance()->getWorkingDirectory());
}

//------------------------------------------------------------------------
PPApplication::PPApplication(
	const QString & aName, const QString & aVersion, int aArgumentCount, char ** aArguments)
	: BasicQtApplication<SafeQApplication>(aName, aVersion, aArgumentCount, aArguments),
	mProtection("PaymentProcessorProtection")
{
	// Производим проверку на наличие еще одной запущенной копии приложения.
	mProtection.attach();

	if (!mProtection.create(1))
	{
		mProtection.detach();

		ILog::getInstance(aName)->write(LogLevel::Warning, "Another instance of application was terminated.");

		throw std::runtime_error("Can't run application, because another instance already running.");
	}

	getQtApplication().setStyle("plastique");
	getQtApplication().setQuitOnLastWindowClosed(false);

	// Перенаправляем логи.
	QString appDest = getLog()->getDestination();
	ILog::getInstance("CryptEngine")->setDestination(appDest);
	ILog::getInstance("DatabaseProxy")->setDestination(appDest);
	ILog::getInstance("MessageQueue")->setDestination(appDest);
	ILog::getInstance("Exceptions")->setDestination(appDest);
	ILog::getInstance("MessageQueueClient")->setDestination(appDest);
	ILog::getInstance("Plugins")->setDestination(appDest);

	mServiceController = new ServiceController(this);

	connect(mServiceController, SIGNAL(exit(int)), SLOT(exit(int)));
	connect(this, SIGNAL(screenshot()), SLOT(onScreenshot()));

	// Парсим параметры командной строки.
	foreach (const QString & parameter, BasicQtApplication::getArguments())
	{
		mArguments.insert(parameter.section('=', 0, 0), parameter.section('=', 1, 1));
	}

	connect(qApp, SIGNAL(commitDataRequest(QSessionManager &)), this, SLOT(closeBySystemRequest(QSessionManager &)), Qt::DirectConnection);
}

//------------------------------------------------------------------------
PPApplication::~PPApplication()
{
	mProtection.detach();

	delete mServiceController;
}

//------------------------------------------------------------------------
int PPApplication::exec()
{
	// Устанавливаем обработчик системных событий.
	QAbstractEventDispatcher::instance()->installNativeEventFilter(this);

	// блокируем скринсеййвер 
	ISysUtils::disableScreenSaver();

	if (mServiceController->initializeServices())
	{
		return BasicQtApplication::exec();
	}
	else
	{
		LOG(getLog(), LogLevel::Error, "Failed to initialize PaymentProcessor.");

		// Выводим подробный отчет о запуске сервисов.
		mServiceController->dumpFailureReport();

		return ExitCode::Error;
	}
}

//------------------------------------------------------------------------
SDK::PaymentProcessor::ICore * PPApplication::getCore()
{
	return mServiceController;
}

//------------------------------------------------------------------------
void PPApplication::qtMessageHandler(QtMsgType /*aType*/, const QMessageLogContext & /*aContext*/, const QString & aMessage)
{
	static ILog * log = ILog::getInstance("QtMessages");

	log->write(LogLevel::Normal, aMessage);
}

//------------------------------------------------------------------------
QList<QImage> PPApplication::getScreenshot()
{
	mScreenshots.clear();

	if (QThread::currentThread() != this->thread())
	{
		mScreenshotMutex.lock();

		emit screenshot();

		if (!mScreenshotCondition.wait(&mScreenshotMutex, 5000))
		{
			LOG(getLog(), LogLevel::Error, "Failed to get screenshot, the application is busy.");
		}

		mScreenshotMutex.unlock();
	}
	else
	{
		onScreenshot();
	}

	return mScreenshots;
}

//------------------------------------------------------------------------
void PPApplication::onScreenshot()
{
	QMutexLocker locker(&mScreenshotMutex);

#ifdef Q_OS_WIN
	auto desktop = QApplication::desktop();

	for (int i = 0; i < desktop->numScreens(); ++i)
	{
		auto geometry = desktop->screenGeometry(i);

		mScreenshots << QPixmap::grabWindow(desktop->winId(), geometry.left(), geometry.top(), geometry.width(), geometry.height()).toImage();
	}
#else
	#error Screenshot command is not implemented on this platform.
#endif // Q_OS_WIN

	mScreenshotCondition.wakeAll();
}

//------------------------------------------------------------------------
QVariantMap PPApplication::getArguments() const
{
	return mArguments;
}

//------------------------------------------------------------------------
IApplication::AppInfo PPApplication::getAppInfo() const
{
	AppInfo inf;
	inf.version = BasicQtApplication::getVersion();
	inf.appName = mName;
	inf.configuration = getSettings().value("common/configuration").toString();
	return inf;
}

//------------------------------------------------------------------------
QSettings & PPApplication::getSettings() const
{
	return BasicQtApplication::getSettings();
}

//------------------------------------------------------------------------
ILog * PPApplication::getLog() const
{
	return BasicQtApplication::getLog();
}

//------------------------------------------------------------------------
QString PPApplication::getUserDataPath() const
{
	QString dataDir = getSettings().contains(CSettings::UserDataPath) ? getSettings().value(CSettings::UserDataPath).toString() : "user";

	return QDir::cleanPath(QDir::fromNativeSeparators(QDir::isAbsolutePath(dataDir) ? dataDir : BasicApplication::getWorkingDirectory() + "/" + dataDir));
}

//------------------------------------------------------------------------
QString PPApplication::getPluginPath() const
{
	QString pluginDir = getSettings().contains(CSettings::PluginsPath) ? getSettings().value(CSettings::PluginsPath).toString() : "plugins";

	return QDir::cleanPath(QDir::fromNativeSeparators(QDir::isAbsolutePath(pluginDir) ? pluginDir : BasicApplication::getWorkingDirectory() + "/" + pluginDir));
}

//------------------------------------------------------------------------
QString PPApplication::getUserPluginPath() const
{
	return QString("%1/bin").arg(getUserDataPath());
}

//------------------------------------------------------------------------
bool PPApplication::nativeEventFilter(const QByteArray & aEventType, void * aMessage, long * aResult)
{
#ifdef Q_OS_WIN
	MSG * message = (MSG *)aMessage;

	if (message)
	{
		switch (message->message)
		{
		case WM_SYSCOMMAND:
			if (message->wParam == SC_SCREENSAVE)
			{
				return true;
			}

			if (message->wParam == SC_MONITORPOWER)
			{
				/*
				Nothing todo ;)

				Sets the state of the display. This command supports devices that have power-saving features, such as a battery-powered personal computer.

				The lParam parameter can have the following values:
				-1 (the display is powering on)
				1 (the display is going to low power)
				2 (the display is being shut off)
				*/

				return true;
			}
			break;

		case WM_POWERBROADCAST:
			break;
		}
	}
#else
	#error Handling display screensaver/power management is not implemented for this platform!
#endif // Q_OS_WIN

	return false;
}

//------------------------------------------------------------------------
void PPApplication::exit(int aResultCode)
{
	LOG(getLog(), LogLevel::Debug, QString("Exit application with %1 code.").arg(aResultCode));

	qApp->exit(aResultCode);
}

//------------------------------------------------------------------------
void PPApplication::closeBySystemRequest(QSessionManager & aSessionManager)
{
	// блокируем остановку системы.
	aSessionManager.cancel();

	LOG(getLog(), LogLevel::Warning, "SHUTDOWN service by system request.");

	// останавливаем систему самостоятельно
	getCore()->getEventService()->sendEvent(PP::Event(PP::EEventType::Shutdown));
}

//------------------------------------------------------------------------
