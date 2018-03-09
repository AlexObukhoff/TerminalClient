/* @file Класс, реализующий приложение для системы обновления. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtWidgets/QApplication>
#include <QtGui/QMovie>
#include <QtCore/QProcess>
#include <QtCore/QDateTime>
#include <QtCore/QTimer>
#include <QtCore/QDir>
#include <QSingleApplication/qtlocalpeer.h>
#include <Common/QtHeadersEnd.h>

// SDK - статусы команды.
#include <SDK/PaymentProcessor/Core/IRemoteService.h>

// Modules
#include <Common/Application.h>
#include <Common/Version.h>
#include <SysUtils/ISysUtils.h>
#include <WatchServiceClient/Constants.h>

// Project
#include "UpdaterApp.h"
#include "UnhandledException.h"

//---------------------------------------------------------------------------
namespace Opt
{
	const char WorkDir[] = "workdir";
	const char Server[] = "server";
	const char UpdateUrl[] = "update-url";
	const char Version[] = "version";
	const char Configuration[] = "conf";
	const char Command[] = "command";
	const char Application[] = "application";
	const char CommandID[] = "id";
	const char Proxy[] = "proxy";
	const char Components[] = "components";
	const char PointID[] = "point";
	const char MD5[] = "md5";
	const char NoRestart[] = "no-restart";
	const char DestinationSubdirs[] = "destination-subdir";
	const char AcceptKeys[] = "accept-keys";
}

namespace Command
{
	const char Config[] = "config";
	const char UserPack[] = "userpack";
	const char Update[] = "update";
	const char Integrity[] = "integrity";
}

//---------------------------------------------------------------------------
UpdaterApp::UpdaterApp(int aArgc, char ** aArgv)
	: BasicQtApplication<SafeQApplication>(CUpdater::Name, Cyberplat::getVersion(), aArgc, aArgv),
	  mState(CUpdaterApp::Download)
{
	CatchUnhandledExceptions();

	mWatchServiceClient = QSharedPointer<IWatchServiceClient>(::createWatchServiceClient(CWatchService::Modules::Updater));

	mWatchServiceClient->subscribeOnCloseCommandReceived(this);
	mWatchServiceClient->subscribeOnDisconnected(this);
}

//---------------------------------------------------------------------------
UpdaterApp::~UpdaterApp()
{
	mWatchServiceClient->stop();

	SDK::PaymentProcessor::IRemoteService::EStatus status;

	// Обновляем отчет.
	switch (mResultCode)
	{
	case CUpdaterApp::ExitCode::NoError:
		status = SDK::PaymentProcessor::IRemoteService::OK;
		break;

	case CUpdaterApp::ExitCode::ContunueExecution:
		status = SDK::PaymentProcessor::IRemoteService::Executing;
		break;

	default:
		status = SDK::PaymentProcessor::IRemoteService::Error;
	}

	QMap<int, QString> descriptions;

	descriptions.insert(CUpdaterApp::ExitCode::ErrorRunFromTempDir, tr("#error_run_from_temp_dir"));
	descriptions.insert(CUpdaterApp::ExitCode::NoWatchService, tr("#error_connection_to_guard"));
	descriptions.insert(CUpdaterApp::ExitCode::UnknownCommand, tr("#error_unknown_command"));
	descriptions.insert(CUpdaterApp::ExitCode::SecondInstance, tr("#error_second_instance"));
	descriptions.insert(CUpdaterApp::ExitCode::UnknownError, tr("#error_unknown"));
	descriptions.insert(CUpdaterApp::ExitCode::NetworkError, tr("#error_network"));
	descriptions.insert(CUpdaterApp::ExitCode::ParseError, tr("#error_parse_response"));
	descriptions.insert(CUpdaterApp::ExitCode::DeployError, tr("#error_deploy"));
	descriptions.insert(CUpdaterApp::ExitCode::Aborted, tr("#error_aborted"));
	descriptions.insert(CUpdaterApp::ExitCode::Blocked, tr("#error_update_blocked"));
	descriptions.insert(CUpdaterApp::ExitCode::FailIntegrity, tr("#error_check_integrity"));

	if (descriptions.contains(mResultCode))
	{
		mReportBuilder.setStatusDescription(descriptions.value(mResultCode));
	}
	mReportBuilder.setStatus(status);
}

//---------------------------------------------------------------------------
QString UpdaterApp::getArgument(const char * aName, const QString & aDafaultValue /*= QString()*/) const
{
	auto arguments = getArguments();

	auto index = arguments.indexOf(QString("--%1").arg(aName), Qt::CaseInsensitive);
	return index < 0 ? aDafaultValue : arguments.value(index + 1, aDafaultValue);
};

//---------------------------------------------------------------------------
void UpdaterApp::onCloseCommandReceived()
{
	getLog()->write(LogLevel::Normal, QString("Receive close signal from watch service. State=%1.").arg(mState));

	if (mState == CUpdaterApp::Download)
	{
		getLog()->write(LogLevel::Normal, "Receive close signal from watch service. Update aborted.");

		mResultCode = CUpdaterApp::ExitCode::Aborted;

		errorExit();
	}
}

//---------------------------------------------------------------------------
void UpdaterApp::onDisconnected()
{
	switch (mState)
	{
	case CUpdaterApp::Deploy:
		getLog()->write(LogLevel::Normal, "Disconnected from watch service. Go to deploy.");
		QMetaObject::invokeMethod(mUpdater, "deploy", Qt::QueuedConnection);
		break;

	case CUpdaterApp::Finish:
		getLog()->write(LogLevel::Normal, "Disconnected from watch service.");
		break;

	default:
		getLog()->write(LogLevel::Normal, "Disconnected from watch service. Try reconnect.");

		if (!mWatchServiceClient->isConnected())
		{
			if (!mWatchServiceClient->start())
			{
				// не удалось подключиться, пробуем еще раз
				QTimer::singleShot(1000, this, SLOT(onDisconnected()));
			}
			else
			{
				getLog()->write(LogLevel::Normal, "Connect to watch service OK.");
			}
		}
		break;
	}
}

//---------------------------------------------------------------------------
QString UpdaterApp::getWorkingDirectory() const
{
	// Парсим аргументы коммандной строки.
	return getArgument(Opt::WorkDir, BasicQtApplication<SafeQApplication>::getWorkingDirectory());
}

//---------------------------------------------------------------------------
QStringList configStringList(const QVariant & aValue, const char * aDelimeter = ",")
{
	switch (aValue.type())
	{
	case QVariant::StringList:
		return aValue.toStringList();

	default:
		return aValue.toString().split(aDelimeter, QString::SkipEmptyParts);
	}
}

//---------------------------------------------------------------------------
QStringList UpdaterApp::exceptionDirs() const
{
	return QStringList() << "logs" << "backup" << "user" << "update" << "receipts" << "ad";
}

//---------------------------------------------------------------------------
void UpdaterApp::run()
{
	// Парсим аргументы коммандной строки и конфигурацию.
	QSettings & settings = getSettings();

	QString workingDir = getArgument(Opt::WorkDir);
	// переходим в рабочую папку, как было указано в параметрах командной строки
	if (!workingDir.isEmpty() && !QDir::setCurrent(workingDir))
	{
		getLog()->write(LogLevel::Error, QString("Error change current dir: %1.").arg(workingDir));
	}

	QString server = getArgument(Opt::Server);
	QString updateBaseUrl = getArgument(Opt::UpdateUrl, server);
	QString version = getArgument(Opt::Version);
	QString configuration = getArgument(Opt::Configuration);
	QString command = getArgument(Opt::Command);
	QString app = getArgument(Opt::Application);
	QString cmndId = getArgument(Opt::CommandID);
	QString proxy = getArgument(Opt::Proxy);
	QStringList components = getArgument(Opt::Components).split(",", QString::SkipEmptyParts);
	QString pointId = getArgument(Opt::PointID);
	QString acceptKeys = getArgument(Opt::AcceptKeys);
	QString md5 = getArgument(Opt::MD5);
	// Параметр запрещающий перезагрузку ТК после скачивания файла
	bool woRestart = getArgument(Opt::NoRestart).compare("true", Qt::CaseInsensitive) == 0;
	// подпапка, в которую распаковываем архив
	QString subdir = getArgument(Opt::DestinationSubdirs);

	// Создаем файл с отчетом.
	mReportBuilder.open(cmndId, server, md5);
	mReportBuilder.setStatus(SDK::PaymentProcessor::IRemoteService::Executing);

	// Проверка на запуск второй копии программы.
	QtLocalPeer * peer = new QtLocalPeer(this, getName());
	if (peer->isClient())
	{
		getLog()->write(LogLevel::Warning, "Another instance is already running.");
		mResultCode = CUpdaterApp::ExitCode::SecondInstance;
		return;
	}

	// Запускаем клиент сторожевого сервиса.
	if (!mWatchServiceClient->start())
	{
		getLog()->write(LogLevel::Error, "Cannot connect to watch service.");
		mResultCode = CUpdaterApp::ExitCode::NoWatchService;
		return;
	}

	QTimer * tooLongDownloadTimer = new QTimer(this);
	connect(tooLongDownloadTimer, SIGNAL(timeout()), this, SLOT(tooLondToDownload()));
	tooLongDownloadTimer->start(CUpdaterApp::MaxDownloadTime);

	mUpdater = new Updater(server, updateBaseUrl, version, app, configuration, pointId);
	
	mUpdater->setParent(this);
	mUpdater->setProxy(proxy);
	mUpdater->setAcceptedKeys(acceptKeys);
	connect(mUpdater, SIGNAL(progress(int)), &mReportBuilder, SLOT(setProgress(int)));

	// Создаем файл отчета.
	if (command == Command::Config)
	{
		// Устанавливаем рабочую папку.
		mUpdater->setWorkingDir("./user");
		mUpdater->setMD5(md5);

		connect(mUpdater, SIGNAL(done(CUpdaterErrors::Enum)), SLOT(onConfigReady(CUpdaterErrors::Enum)));
		connect(mUpdater, SIGNAL(deployment()), SLOT(onDeployment()));

		// Команда на закачку конфигов.
		mUpdater->downloadPackage();
	}
	else if (command == Command::UserPack)
	{
		mUpdater->setWorkingDir(subdir.isEmpty() ? "." : QString("./%1").arg(subdir));
		mUpdater->setMD5(md5);

		connect(mUpdater, SIGNAL(done(CUpdaterErrors::Enum)), woRestart ? SLOT(onPackReady(CUpdaterErrors::Enum)) : SLOT(onConfigReady(CUpdaterErrors::Enum)));
		connect(mUpdater, SIGNAL(deployment()), SLOT(onDeployment()));

		// Команда на закачку конфигов.
		mUpdater->downloadPackage();
	}
	else if (command == Command::Update)
	{
		if (workingDir.isEmpty())
		{
			if (reRunFromTempDirectory())
			{
				getLog()->write(LogLevel::Normal, QString("Run updater from temp path: '%1' is OK.").arg(getUpdaterTempDir()));
				mResultCode = CUpdaterApp::ExitCode::ContunueExecution;
				return;
			}
			else
			{
				getLog()->write(LogLevel::Error, QString("Failed run updater from temp path: '%1'.").arg(getUpdaterTempDir()));
				mResultCode = CUpdaterApp::ExitCode::ErrorRunFromTempDir;
				return;
			}
		}
		else
		{
			// Устанавливаем рабочую папку.
			mUpdater->setWorkingDir(workingDir);
		}

		// Добавляем директории в список исключений. Файлы в этих папках не участвуют в обновлении.
		mUpdater->addExceptionDirs(exceptionDirs());
		mUpdater->addExceptionDirs(configStringList(settings.value("directory/ignore")));

		mUpdater->addComponentForUpdate(components);
		mUpdater->setOptionalComponents(configStringList(settings.value("component/optional")));
		mUpdater->setConfigurationRequiredFiles(configStringList(settings.value("validator/required_files")));

		// Подписываемся на нужные сигналы от движка обновлений.
		connect(mUpdater, SIGNAL(updateSystemIsWaiting()), this, SLOT(updateSystemIsWaiting()));
		connect(mUpdater, SIGNAL(downloadAccomplished()), this, SLOT(onDownloadComplete()));
		connect(mUpdater, SIGNAL(done(CUpdaterErrors::Enum)), this, SLOT(onUpdateComplete(CUpdaterErrors::Enum)));

		QMetaObject::invokeMethod(mUpdater, "runUpdate", Qt::QueuedConnection);
	}
	else if (command == Command::Integrity)
	{
		// Устанавливаем рабочую папку.
		mUpdater->setWorkingDir("./");

		// Добавляем директории в список исключений. Файлы в этих папках не участвуют в обновлении.
		mUpdater->addExceptionDirs(exceptionDirs());
		mUpdater->addExceptionDirs(configStringList(settings.value("directory/ignore")));

		mUpdater->setOptionalComponents(configStringList(settings.value("component/optional")));
		mUpdater->setConfigurationRequiredFiles(configStringList(settings.value("validator/required_files")));

		int result = mUpdater->checkInterity();
		
		if (result < 0)
		{
			mResultCode = CUpdaterApp::ExitCode::UnknownCommand;
		}
		else if (result > 0)
		{
			mResultCode = CUpdaterApp::ExitCode::FailIntegrity;
			mReportBuilder.setStatusDescription(tr("#error_check_integrity").arg(result));
		}
		else
		{
			mResultCode = CUpdaterApp::ExitCode::NoError;
		}

		return;
	}
	else
	{
		getLog()->write(LogLevel::Error, QString("Unknown command: %1.").arg(command));
		mResultCode = CUpdaterApp::ExitCode::UnknownCommand;
		return;
	}

	mResultCode = exec();
}

//---------------------------------------------------------------------------
void UpdaterApp::updateSystemIsWaiting()
{
	Updater * updater = dynamic_cast<Updater*>(sender());

	int nextUpdateTimeout = 10*60;

	getLog()->write(LogLevel::Normal, QString("Run update in %1 sec.").arg(nextUpdateTimeout));

	QTimer::singleShot( nextUpdateTimeout * 1000, updater, SLOT(runUpdate()));
}

//---------------------------------------------------------------------------
void UpdaterApp::onDeployment()
{
	mState = CUpdaterApp::Deploy;
}

//---------------------------------------------------------------------------
void UpdaterApp::onDownloadComplete()
{
	onDeployment();

	getLog()->write(LogLevel::Normal, "Download complete. Close modules...");

	// Останавливаем tray.exe (watch_service_controller)
	mWatchServiceClient->closeModule(CWatchService::Modules::WatchServiceController);

	// Останавливаем client.exe
	mWatchServiceClient->subscribeOnModuleClosed(this);
	mWatchServiceClient->closeModule(CWatchService::Modules::PaymentProcessor);

	getLog()->write(LogLevel::Normal, QString("Waiting PaymentProcessor exit... (%1 sec)").arg(CUpdaterApp::ErrorExitTimeout/1000));

	// На всякий случай, если клиент не закроется в течении 15 минут
	startErrorTimer();
}

//---------------------------------------------------------------------------
void UpdaterApp::startErrorTimer()
{
	mErrorStopTimer = new QTimer(this);
	mErrorStopTimer->setSingleShot(true);
	connect(mErrorStopTimer.data(), SIGNAL(timeout()), this, SLOT(onFailStopClient()));
	mErrorStopTimer->start(CUpdaterApp::ErrorExitTimeout);
}

//---------------------------------------------------------------------------
void UpdaterApp::stopErrorTimer()
{
	if (mErrorStopTimer)
	{
		mErrorStopTimer->stop();
		mErrorStopTimer->deleteLater();
	}
}

//---------------------------------------------------------------------------
void UpdaterApp::onFailStopClient()
{
	getLog()->write(LogLevel::Error, "Fail stop PaymentProcessor.");

	mWatchServiceClient->startModule(CWatchService::Modules::PaymentProcessor);

	QTimer::singleShot(10, this, SLOT(errorExit()));
}

//---------------------------------------------------------------------------
void UpdaterApp::tooLondToDownload()
{
	getLog()->write(LogLevel::Error, "Too long to downloading update. Try to close updater.");

	if (mState == CUpdaterApp::Download)
	{
		errorExit();
	}
	else
	{
		getLog()->write(LogLevel::Fatal, "Fail to close. Updater in deploy stage.");
	}
}

//---------------------------------------------------------------------------
void UpdaterApp::onModuleClosed(const QString & aModuleName)
{
	if (aModuleName == CWatchService::Modules::PaymentProcessor)
	{
		// отсоединяемся от сигнала о закрытии модулей
		disconnect(this, SLOT(onModuleClosed(const QString &)));

		getLog()->write(LogLevel::Normal, "PaymentProcessor closed. Wait to close watch service...");

		stopErrorTimer();

		// Показываем экран блокировки.
		mSplashScreen.showMaximized();
		mSplashScreen.showFullScreen();

		// Останавливаем Watch-Service.
		mWatchServiceClient->stopService();
		// после остановки WatchService мы попадём в onDisconnected()
	}
}

//---------------------------------------------------------------------------
void UpdaterApp::onUpdateComplete(CUpdaterErrors::Enum aError)
{
	if (mState == CUpdaterApp::Deploy)
	{
		// Запускаем tray
		if (!QProcess::startDetached(getWorkingDirectory() + QDir::separator() + CWatchService::Modules::WatchServiceController + ".exe"))
		{
			getLog()->write(LogLevel::Error, "Failed to launch tray app.");
		}

		// Заново запускаем watchservice и убеждаемся, что он успешно запустился.
		QProcess * startService = new QProcess();
		startService->start(CWatchService::Modules::WatchService);

		if (!startService->waitForStarted())
		{
			// Фатальная ошибка.
			getLog()->write(LogLevel::Fatal, "Failed to launch watch service. Updater showing splash screen.");

			// Не выходим из приложения.
			return;
		}
	}

	// Завершаем работу приложения через 10 секунд, что бы guard успел отобразить окно блокировки
	delayedExit(10, aError);
}

//---------------------------------------------------------------------------
void UpdaterApp::errorExit()
{
	getQtApplication().exit(mResultCode ? mResultCode : CUpdaterApp::ExitCode::UnknownError);
}

//---------------------------------------------------------------------------
int UpdaterApp::getResultCode() const
{
	return mResultCode;
}

//---------------------------------------------------------------------------
void UpdaterApp::onConfigReady(CUpdaterErrors::Enum aError)
{
	// Удаляем из конфигурации шаблоны чеков
	cleanDir(QDir::currentPath() + "/user/templates");
	QDir(QDir::currentPath() + "/user").rmdir("templates");

	// Перезапускаем ПО.
	mWatchServiceClient->restartService(QStringList());

	// Завершаем работу приложения через 5 секунд, что бы guard успел получить сигнал перезагрузки ТК
	delayedExit(5, aError);
}

//---------------------------------------------------------------------------
void UpdaterApp::onPackReady(CUpdaterErrors::Enum aError)
{
	delayedExit(1, aError);
}

//---------------------------------------------------------------------------
void UpdaterApp::delayedExit(int aTimeout, CUpdaterErrors::Enum aError)
{
	mState = CUpdaterApp::Finish;

	setResultCode(aError);
	
	getLog()->write(aError ? LogLevel::Error : LogLevel::Normal, QString("Closing after %1 seconds.").arg(aTimeout));

	if (aError)
	{
		QTimer::singleShot(aTimeout * 1000, this, SLOT(errorExit()));
	}
	else
	{
		QTimer::singleShot(aTimeout * 1000, qApp, SLOT(quit()));
	}
}

//---------------------------------------------------------------------------
QString UpdaterApp::getUpdaterTempDir() const
{
	const QString tempDirName = "cyberplat_updater_temp";

	QDir tempDir = QDir::tempPath();
	
	if (!tempDir.exists(tempDirName))
	{
		if (!tempDir.mkdir(tempDirName))
		{
			getLog()->write(LogLevel::Fatal, QString("Error mkdir '%1' in '%2'.").arg(tempDirName).arg(QDir::tempPath()));
			return QString();
		}
	}

	tempDir.cd(tempDirName);
	return tempDir.canonicalPath();
}

//---------------------------------------------------------------------------
bool UpdaterApp::reRunFromTempDirectory()
{
	getLog()->write(LogLevel::Normal, QString("Trying run updater from temp path: '%1'.").arg(getUpdaterTempDir()));
	if (CopyToTempPath())
	{
		QString program = QDir(getUpdaterTempDir()).absoluteFilePath(QFileInfo(QCoreApplication::arguments()[0]).fileName());
		QStringList arguments = QStringList() << getArguments() << "--workdir" << getWorkingDirectory();

		{
			QString programSettingsFile = QDir(getUpdaterTempDir()).absoluteFilePath(QFileInfo(QCoreApplication::arguments()[0]).baseName()) + ".ini";
			QSettings tempSettings(programSettingsFile, QSettings::IniFormat);

			tempSettings.setValue("common/working_directory", getWorkingDirectory());
		}
		//удаляем первый аргумент с именем файла
		arguments.takeFirst(); 

		return QProcess::startDetached(program, arguments, getUpdaterTempDir());
	}

	return false;
}

//---------------------------------------------------------------------------
bool UpdaterApp::CopyToTempPath()
{
	QString tempDirPath = getUpdaterTempDir();
	if (tempDirPath.isEmpty())
	{
		return false;
	}

	getLog()->write(LogLevel::Normal, QString("Clean directory '%1'.").arg(tempDirPath));
	// чистим папку
	if (!cleanDir(tempDirPath))
	{
		return false;
	}

	QFileInfo updaterFileInfo = QFileInfo(getArguments()[0]);

	QStringList needFiles = QStringList()
		<< updaterFileInfo.fileName()
		<< updaterFileInfo.absoluteDir().entryList(QStringList() << updaterFileInfo.baseName() + "*.qm")
		<< updaterFileInfo.absoluteDir().entryList(QStringList() << updaterFileInfo.baseName() + ".*") // for pdb
		<< "7za.exe"
		<< "Qt5Core.dll"
		<< "Qt5Gui.dll"
		<< "Qt5Widgets.dll"
		<< "Qt5Xml.dll"
		<< "Qt5Network.dll"
		<< "ssleay32.dll"
		<< "libeay32.dll"
		<< "icudt51.dll"
		<< "icuin51.dll"
		<< "icuuc51.dll"
		<< "libEGL.dll"
		<< "libGLESv2.dll";	

	foreach (auto file, QDir(QCoreApplication::applicationDirPath()).entryInfoList(needFiles, QDir::Files))
	{
		QString fileName = QFileInfo(file).fileName();
		QString dstFileName = tempDirPath + QDir::separator() + fileName;
		getLog()->write(LogLevel::Normal, QString("Copy: '%1'.").arg(fileName));
		
		if (!QFile::copy(QCoreApplication::applicationDirPath() + QDir::separator() + fileName, dstFileName))
		{
			getLog()->write(LogLevel::Fatal, QString("Error copy from '%1' to '%2'.").arg(file.fileName()).arg(dstFileName));
			return false;
		}
	}

	QDir(tempDirPath).mkdir("platforms");
	QDir(tempDirPath).mkdir("imageformats");

	return copyFiles(updaterFileInfo.path() + "/platforms", "*.dll", tempDirPath + "/platforms") && 
		copyFiles(updaterFileInfo.path() + "/imageformats", "*.dll", tempDirPath + "/imageformats");
}

//---------------------------------------------------------------------------
bool UpdaterApp::copyFiles(const QString & from, const QString & mask, const QString & to)
{
	QDir fromDir = QDir(from);

	Q_FOREACH (QString file, fromDir.entryList(QStringList() << mask))
	{
		QString dstFileName = to + "/" + QFileInfo(file).fileName();
		getLog()->write(LogLevel::Normal, QString("Copy: '%1'.").arg(file));
		if (!QFile::copy(fromDir.filePath(file), dstFileName))
		{
			getLog()->write(LogLevel::Fatal, QString("Error copy from '%1' to '%2'.").arg(file).arg(dstFileName));
			return false;
		}
	}

	return true;
}

//------------------------------------------------------------------------
void UpdaterApp::qtMessageHandler(QtMsgType aType, const QMessageLogContext & aContext, const QString & aMessage)
{
	static ILog * log = ILog::getInstance(CUpdater::Name);

	log->write(LogLevel::Normal, QString("QtMessages: %1").arg(aMessage));
}

//---------------------------------------------------------------------------
bool UpdaterApp::cleanDir(const QString & dirName)
{
	bool result = true;
	QDir dir(dirName);

	if (dir.exists(dirName)) 
	{
		Q_FOREACH (auto info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
		{
			if (info.isDir()) 
			{
				result = cleanDir(info.absoluteFilePath()) && dir.rmdir(info.absoluteFilePath());
			}
			else 
			{
				result = QFile::remove(info.absoluteFilePath());
			}

			if (!result) 
			{
				getLog()->write(LogLevel::Fatal, QString("Error remove '%1'.").arg(info.absoluteFilePath()));
				return result;
			}
		}
	}

	return result;
}

//---------------------------------------------------------------------------
void UpdaterApp::setResultCode(CUpdaterErrors::Enum aError)
{
	switch (aError)
	{
	case CUpdaterErrors::UnknownError:
		mResultCode = CUpdaterApp::ExitCode::UnknownError;
		break;

	case CUpdaterErrors::NetworkError:
		mResultCode = CUpdaterApp::ExitCode::NetworkError;
		break;

	case CUpdaterErrors::ParseError:
		mResultCode = CUpdaterApp::ExitCode::ParseError;
		break;

	case CUpdaterErrors::DeployError:
		mResultCode = CUpdaterApp::ExitCode::DeployError;
		break;

	case CUpdaterErrors::UpdateBlocked:
		mResultCode = CUpdaterApp::ExitCode::Blocked;
		break;

	default:
		break;
	}
}

//---------------------------------------------------------------------------
