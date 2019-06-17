/* @file Система обновления. */

// Stl
#include <numeric>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QScopedPointer>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtXml/QDomDocument>
#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QRegExp>
#include <QtCore/QFileInfo>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/Exception.h>
#include <Common/ScopedPointerLaterDeleter.h>
#include <NetworkTaskManager/FileDownloadTask.h>
#include <NetworkTaskManager/MemoryDataStream.h>
#include <NetworkTaskManager/HashVerifier.h>

// Thirdparty
#if QT_VERSION < 0x050000
#include <Qt5Port/qt5port.h>
#endif

// Проект
#include "Misc.h"
#include "Updater.h"
#include "Package.h"
#include "Folder.h"
#include "WindowsBITS.h"


Q_DECLARE_METATYPE(CUpdaterErrors::Enum)

//---------------------------------------------------------------------------
namespace CUpdater
{
	const int MaxFails = 18;
	const int MinutesBeforeNextTry = 2;

	const QByteArray CyberplatStatusTag = "X-Cyberplat-Status";
	const QByteArray CyberplatAcceptKeysTag = "X-Cyberplat-Accepted-Keys";
	const QByteArray CberplatSignatureTag = "X-signature";
	
	const QByteArray Blocked = "blocked";
	const QByteArray Wait = "wait";

	QString UpdaterConfigurationDir = "/update/";
	QString UpdaterConfiguration = "configuration_%1";

	const QString BitsJobNamePrefix = "TCUpdater_";
}

//---------------------------------------------------------------------------
Updater::Updater(QObject * aParent)
	: QObject(aParent),
	mFailCount(0),
	mAllTasksCount(0),
	mProgressPercent(0),
	mBitsManager(ILog::getInstance(CUpdater::Name)),
	mUseBITS(true),
	mJobPriority(CBITS::HIGH)
{
	mNetworkTaskManager.setLog(ILog::getInstance(CUpdater::Name));

	mNetworkTaskManager.setDownloadSpeedLimit(80);

	connect(&mProgressTimer, SIGNAL(timeout()), this, SLOT(showProgress()));
}

//---------------------------------------------------------------------------
Updater::Updater(const QString & aConfigURL, const QString & aUpdateURL, const QString & aVersion, const QString & aAppId, const QString & aConfiguration, const QString & aPointId)
	: mConfigURL(aConfigURL), 
	mUpdateURL(aUpdateURL+"/"+aAppId+"/"+aConfiguration), 
	mVersion(aVersion), 
	mAppId(aAppId), 
	mConfiguration(aConfiguration), 
	mNetworkTaskManager(ILog::getInstance(CUpdater::Name)),
	mCurrentTaskSize(0),
	mWaitUpdateServer(false),
	mFailCount(0), 
	mAP(aPointId),
	mAllTasksCount(0),
	mProgressPercent(0),
	mBitsManager(ILog::getInstance(CUpdater::Name)),
	mUseBITS(true)
{
	mNetworkTaskManager.setDownloadSpeedLimit(80);

	connect(&mProgressTimer, SIGNAL(timeout()), this, SLOT(showProgress()));
}

//---------------------------------------------------------------------------
void Updater::setProxy(const QString & aProxy)
{
	if (!aProxy.isEmpty())
	{
		QRegExp pattern("(.+):(.*):(.*):(.*):(.+)");

		if (pattern.exactMatch(aProxy))
		{
			QNetworkProxy proxy(static_cast<QNetworkProxy::ProxyType>(pattern.cap(5).toInt()),
				pattern.cap(1), pattern.cap(2).toUShort(), pattern.cap(3), pattern.cap(4));

			mNetworkTaskManager.setProxy(proxy);
		}
		else
		{
			Log(LogLevel::Error, QString("Failed to set up proxy: cannot parse %1.").arg(aProxy));
		}
	}
	else
	{
		Log(LogLevel::Normal, "No proxy.");
	}
}

//---------------------------------------------------------------------------
CUpdaterErrors::Enum Updater::getComponents(Updater::TComponentList & aComponents)
{
	mWaitUpdateServer = false;

	aComponents.clear();

	// Получаем с сервера файл с описанием.
	NetworkTask * task = new NetworkTask();

	QUrl url = mConfigURL;
	url.addQueryItem("name", mAppId);
	url.addQueryItem("conf", mConfiguration);
	url.addQueryItem("rev", mVersion);
	url.addQueryItem("AP", mAP);

	Log(LogLevel::Normal, QString("Downloading component descriptions from '%1'...").arg(url.toString()));

	task->setDataStream(new MemoryDataStream);
	task->setUrl(url);
	task->setType(NetworkTask::Get);
	task->getRequestHeader().insert(CUpdater::CyberplatAcceptKeysTag, mAcceptedKeys.toLatin1());

	mNetworkTaskManager.addTask(task);

	task->waitForFinished();

	if (task->getError() != NetworkTask::NoError)
	{
		Log(LogLevel::Error, QString("Failed to download component description. Error %1.").arg(task->errorString()));

		mWaitUpdateServer = true;

		return CUpdaterErrors::NetworkError;
	}

	NetworkTask::TByteMap & responseHeader = task->getResponseHeader();

	// если установлен флаг блокировки обновления ПО
	if (responseHeader.contains(CUpdater::CyberplatStatusTag))
	{
		QByteArray statusTag = responseHeader.value(CUpdater::CyberplatStatusTag);
		Log(LogLevel::Warning, QString("Download component %1: %2")
			.arg(QString::fromLatin1(CUpdater::CyberplatStatusTag))
			.arg(QString::fromLatin1(statusTag)));

		mWaitUpdateServer = (statusTag == CUpdater::Wait);

		return CUpdaterErrors::UpdateBlocked;
	}

	mComponentsSignature = QByteArray::fromPercentEncoding(responseHeader.value(CUpdater::CberplatSignatureTag, QByteArray()));

	auto dataStream = task->getDataStream();
	mComponentsContent = dataStream->takeAll();
	dataStream->close();

	auto result = loadComponents(mComponentsContent, aComponents, mComponentsRevision);

	if (result == CUpdaterErrors::OK && getSavedConfigurations().isEmpty())
	{
		saveUpdateConfiguration();
	}

	return result;
}

//---------------------------------------------------------------------------
QByteArray Updater::loadUpdateConfiguration(const QString & aRevision)
{
	QFile file(QDir(mWorkingDir + CUpdater::UpdaterConfigurationDir)
		.absoluteFilePath(CUpdater::UpdaterConfiguration.arg(aRevision) + ".xml"));

	if (file.open(QIODevice::ReadOnly))
	{
		return file.readAll();
	}

	Log(LogLevel::Error, QString("Failed open file '%1': %2.").arg(file.fileName()).arg(file.errorString()));
	return QByteArray();
}

//---------------------------------------------------------------------------
void Updater::saveUpdateConfiguration()
{
	QDir dir(mWorkingDir + CUpdater::UpdaterConfigurationDir);

	if (mUpdateComponents.isEmpty())
	{
		// Обновили полностью дистрибутив - чистим старые конфигурации обновления
		foreach (const auto file, dir.entryInfoList(QStringList(CUpdater::UpdaterConfiguration.arg("*.*"))))
		{
			QFile::remove(file.absoluteFilePath());
		}
	}

	QFile file(dir.absoluteFilePath(CUpdater::UpdaterConfiguration.arg(mComponentsRevision + ".xml")));
	if (file.open(QIODevice::WriteOnly))
	{
		file.write(mComponentsContent);
		file.close();
	}

	QFile fileSignature(dir.absoluteFilePath(CUpdater::UpdaterConfiguration.arg(mComponentsRevision + ".ipriv")));
	if (fileSignature.open(QIODevice::WriteOnly))
	{
		fileSignature.write(mComponentsSignature);
		fileSignature.close();
	}
}

//---------------------------------------------------------------------------
QStringList Updater::getSavedConfigurations()
{
	QDir dir(mWorkingDir + CUpdater::UpdaterConfigurationDir);
	QRegExp rx(QString(CUpdater::UpdaterConfiguration).arg("(.*)\\.xml"));
	QStringList revisions;

	foreach (auto cfg, dir.entryList(QStringList(QString(CUpdater::UpdaterConfiguration).arg("*.xml"))))
	{
		if (rx.exactMatch(cfg))
		{
			revisions << rx.cap(1);
		}
	}

	return revisions;
}

//---------------------------------------------------------------------------
void Updater::setWorkingDir(const QString & aDir)
{
	mWorkingDir = aDir;

	QDir dir = QDir::currentPath();

	if (!dir.exists(mWorkingDir))
	{
		dir.mkpath(mWorkingDir);
	}
}

//---------------------------------------------------------------------------
void Updater::addComponentForUpdate(const QStringList & aComponents)
{
	mUpdateComponents.append(aComponents);
}

//---------------------------------------------------------------------------
TFileList Updater::getWorkingDirStructure() const throw (Exception)
{
	return getWorkingDirStructure("");
}

//---------------------------------------------------------------------------
void Updater::addExceptionDirs(const QStringList & aDirs)
{
	foreach (auto dir, aDirs)
	{
		mExceptionDirs.push_back(QString("/") + QString(dir).remove(QRegExp("^/+|/+$")));
	}
}

//---------------------------------------------------------------------------
TFileList Updater::getWorkingDirStructure(const QString & aDir) const throw (Exception)
{
	TFileList list;

	if (mExceptionDirs.contains(aDir, Qt::CaseInsensitive))
	{
		return list;
	}

	QDir current(mWorkingDir + "/" + aDir);

	foreach (auto fileInfo, current.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files))
	{
		if (fileInfo.isFile())
		{
			// Вычисляем контрольную сумму.
			QString path = current.filePath(fileInfo.fileName());
			QFile file(path);

			if (file.open(QIODevice::ReadOnly))
			{
				auto filePath = aDir + "/" + fileInfo.fileName();

#if QT_VERSION >= 0x050000
				list.insert(File(filePath.remove(QRegExp("^/+")),
					QString::fromLatin1(QCryptographicHash::hash(file.readAll(), QCryptographicHash::Sha256).toHex()),
					"",
					fileInfo.size()));
#else
				list.insert(File(filePath.remove(QRegExp("^/+")),
					QString::fromLatin1(CCryptographicHash::hash(file.readAll(), CCryptographicHash::Sha256).toHex()),
					"",
					fileInfo.size()));
#endif
			}
			else
			{
				throw Exception(QString("Failed to calculate checksum for file %1.").arg(fileInfo.filePath()));
			}
		}
		else if (fileInfo.isDir())
		{
			list += getWorkingDirStructure(aDir + "/" + fileInfo.fileName());
		}
	}
	
	return list;
}

//-------------------------------------------------------------------------
void Updater::copyFiles(const QString & aSrcDir, const QString & aDstDir, const TFileList & aFiles, bool aIgnoreError) throw (Exception)
{
	Log(LogLevel::Normal, QString("Copy files from '%1' to '%2'.").arg(aSrcDir).arg(aDstDir));

	foreach (auto file, aFiles)
	{
		QString dstFilePath = aDstDir + "/" + file.name();

		// Создаем директорию назначения.
		if (!QDir().mkpath(dstFilePath.section("/", 0, -2)))
		{
			if (aIgnoreError)
			{
				Log(LogLevel::Warning, QString("Failed to create destination path %1.").arg(dstFilePath));
			}
			else
			{
				throw Exception(QString("Failed to create destination path %1.").arg(dstFilePath));
			}
		}

		// Копируем файл.
		if (!QFile::copy(aSrcDir + "/" + file.name(), aDstDir + "/" + file.name()))
		{
			if (aIgnoreError)
			{
				Log(LogLevel::Warning, QString("Failed to copy file %1.").arg(file.name()));
			}
			else
			{
				throw Exception(QString("Failed to copy file %1").arg(file.name()));
			}
		}
	}
}

//---------------------------------------------------------------------------
void Updater::deleteFiles(const TFileList & aFiles, bool aIgnoreError) throw (Exception)
{
	foreach (auto file, aFiles)
	{
		if (QFile::exists(mWorkingDir + "/" + file.name()) &&
			!mExceptionDirs.contains("/" + file.dir(), Qt::CaseInsensitive))
		{
			Log(LogLevel::Normal, QString("Deleting file %1.").arg(file.name()));

			if (!QFile::remove(mWorkingDir + "/" + file.name()))
			{
				if (aIgnoreError)
				{
					Log(LogLevel::Warning, QString("Failed to remove file %1.").arg(file.name()));
				}
				else
				{
					throw Exception(QString("Failed to remove file %1").arg(file.name()));
				}
			}
		}
	}
}

//---------------------------------------------------------------------------
void Updater::download()
{
	// Если список пуст, инициируем следующий шаг обновления.
	if (mActiveTasks.empty())
	{
		mProgressTimer.stop();

		Log(LogLevel::Normal, "Download complete.");

		emit downloadAccomplished();

		return;
	}

	if (!bitsDownload())
	{
		auto task = mActiveTasks.front();
		task->connect(task, SIGNAL(onComplete()), this, SLOT(downloadComplete()), Qt::UniqueConnection);

		mCurrentTaskSize = task->getDataStream()->size();
		mNetworkTaskManager.addTask(task);

		Log(LogLevel::Normal, QString("%1 downloading...").arg(task->getUrl().toString()));
	}
}

//---------------------------------------------------------------------------
void closeFileTask(NetworkTask * aTask)
{
	auto fileTask = qobject_cast<FileDownloadTask *>(aTask);
	if (fileTask)
	{
		fileTask->closeFile();
	}
}

//---------------------------------------------------------------------------
void Updater::downloadComplete()
{
	auto task = mActiveTasks.front();
	task->disconnect(this, SLOT(downloadComplete()));

	auto goToNextFile = [&]()
	{
		// Удаляем старое задание.
		task->getDataStream()->close();
		mActiveTasks.pop_front();
		mFailCount = 0;

		// Продолжаем закачку другого файла.
		QMetaObject::invokeMethod(this, "download", Qt::QueuedConnection);
	};

	if (!task->getError() || task->getError() == NetworkTask::TaskFailedButVerified)
	{
		Log(LogLevel::Normal, QString("File %1 downloaded successfully.").arg(task->getUrl().toString()));

		closeFileTask(task);

		return goToNextFile();
	}

	int nextTryTimeout = CUpdater::MinutesBeforeNextTry;

	Log(LogLevel::Error, QString("Failed to download file %1. Error: %2. Http code: %3")
		.arg(task->getUrl().toString()).arg(task->errorString()).arg(task->getHttpError()));

	bool haveNewData = (mCurrentTaskSize != task->getDataStream()->size());
	bool retryCountReached = ++mFailCount >= CUpdater::MaxFails;

	if (task->getError() == NetworkTask::VerifyFailed || task->getHttpError() == 416) // 416 - Requested Range Not Satisfiable
	{
		checkTaskVerifierResult(task);

		nextTryTimeout = 1;

		if (task->property(CComponent::OptionalTask()).toBool())
		{
			Log(LogLevel::Normal, QString("File %1 is optional. Skip it and continue to download.").arg(task->getUrl().toString()));
			return goToNextFile();
		}
	}
	else
	{
		retryCountReached = retryCountReached && !haveNewData;
	}

	if (!retryCountReached)
	{
		if ((task->getHttpError() / 100) == 2) // HTTP 2xx
		{
			// При успешном ответе сервера продолжаем докачку файла незамедлительно
			Log(LogLevel::Error, "Continue download...");

			nextTryTimeout = 1000;
		}
		else
		{
			Log(LogLevel::Error, QString("Waiting %1 minutes before next try...").arg(nextTryTimeout));

			nextTryTimeout *= 60 * 1000;
		}

		// Делаем повторную попытку скачать файл через несколько минут.
		QTimer::singleShot(nextTryTimeout, this, SLOT(download()));
	}
	else
	{
		if (task->property(CComponent::OptionalTask()).toBool())
		{
			Log(LogLevel::Normal, QString("File %1 is optional. Skip it and continue to download.").arg(task->getUrl().toString()));

			QMetaObject::invokeMethod(task, "resetFile", Qt::DirectConnection);
			return goToNextFile();
		}

		// Закачка была прервана.
		Log(LogLevel::Error, QString("Download terminated after %1 attempts.").arg(CUpdater::MaxFails));

		mProgressTimer.stop();
		emit done(CUpdaterErrors::NetworkError);
	}
}

//---------------------------------------------------------------------------
void Updater::checkTaskVerifierResult(NetworkTask * aTask)
{
	IHashVerifier * verifier = dynamic_cast<IHashVerifier *>(aTask->getVerifier());

	if (verifier)
	{
		Log(LogLevel::Error, QString("Failed verify. Downloaded file hash:%1 required_hash:%2. Remove temporary file")
			.arg(verifier->calculatedHash()).arg(verifier->referenceHash()));
	}
	else
	{
		Log(LogLevel::Error, "Failed verify downloaded file. Remove temporary file");
	}

	QMetaObject::invokeMethod(aTask, "resetFile", Qt::DirectConnection);
	
	mCurrentTaskSize = 0;
}

//---------------------------------------------------------------------------
void Updater::showProgress()
{
	if (mAllTasksCount > 0)
	{
		mProgressPercent = (mAllTasksCount-mActiveTasks.size()) * 100 / mAllTasksCount;
	}
	else
	{
		mProgressPercent = ++mProgressPercent > 100 ? 1 : mProgressPercent;
	}

	emit progress(mProgressPercent);
}

//---------------------------------------------------------------------------
void Updater::downloadComponents(const TComponentList & aComponents)
{
	mComponents = aComponents;

	try
	{
		Log(LogLevel::Normal, "Calculating local files checksum.");
		TFileList currentStructure = getWorkingDirStructure();

		// Формируем список файлов для загрузки.
		foreach (auto comp, mComponents)
		{
			mActiveTasks += comp->download(mUpdateURL, comp->getFiles().intersect(currentStructure));
		}

		mAllTasksCount = mActiveTasks.size();
		mProgressTimer.start(3 * 60 * 1000);

		// Запускаем загрузку.
		QMetaObject::invokeMethod(this, "download", Qt::QueuedConnection);
	}
	catch (Exception & e)
	{
		Log(LogLevel::Fatal, e.getMessage());
	}
}

//---------------------------------------------------------------------------
bool Updater::haveSkippedComponents() const
{
	return !mUpdateComponents.isEmpty();
}

//---------------------------------------------------------------------------
TFileList Updater::intersectByName(const TFileList & aList1, const TFileList & aList2)
{
	QSet<QString> list2Names;

	foreach (auto file, aList2)
	{
		list2Names.insert(file.name());
	}

	TFileList result;

	foreach (auto file, aList1)
	{
		if (list2Names.contains(file.name()))
		{
			result.insert(file);
		}
	}

	return result;
}

//---------------------------------------------------------------------------
void Updater::substractByName(TFileList & aList1, const TFileList & aList2)
{
	QSet<QString> list2Names;

	foreach (auto file, aList2)
	{
		list2Names.insert(file.name());
	}

	TFileList result;

	foreach (auto file, aList1)
	{
		if (list2Names.contains(file.name()))
		{
			result.insert(file);
		}
	}

	aList1.subtract(result);
}

//---------------------------------------------------------------------------
void Updater::deploy()
{
	TFileList downloadedFiles;
	TFileList oldFiles;
	TFileList newFiles;

	Log(LogLevel::Normal, "Start deploy.");

	QString backupDir = mWorkingDir + "/backup/" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss");

	try
	{
		Log(LogLevel::Normal, QString("Backup into '%1'.").arg(backupDir));

		// Создаем временную папку для бекапов.
		if (!QDir().mkpath(backupDir))
		{
			throw Exception(QString("Failed to create path %1.").arg(backupDir));
		}

		// Делаем резервную копию.
		foreach (auto comp, mComponents)
		{
			downloadedFiles += comp->getFiles();
		}

		// Формируем список файлов для удаления (старые версии + мусор).
		auto currentFiles = getWorkingDirStructure();

		if (haveSkippedComponents())
		{
			// при частичном обновлении отдельных компонент мы заменяем только файлы, содержащиеся в скачиваемых компонентах
			oldFiles = intersectByName(currentFiles, downloadedFiles);
		}
		else
		{
			oldFiles = currentFiles;
		}
		oldFiles.subtract(downloadedFiles);

		// убираем из списка старых файлов все файлы компонент, имеющих флаг skip_existing
		foreach (auto comp, mComponents)
		{
			if (comp->skipExisting() || comp->optional())
			{
				substractByName(oldFiles, comp->getFiles());
			}
		}

		newFiles = downloadedFiles;
		newFiles.subtract(currentFiles);

		// Делаем резервные копии.
		copyFiles(mWorkingDir, backupDir, oldFiles);
	}
	catch (Exception & e)
	{
		// На этом этапе дальнейшая установка обновления невозможна.
		Log(LogLevel::Fatal, e.getMessage());

		emit done(CUpdaterErrors::UnknownError);

		return;
	}

	// Производим установку обновления.
	try
	{
		Log(LogLevel::Normal, "Removing old files.");

		// Удаляем файлы.
		deleteFiles(oldFiles);

		Log(LogLevel::Normal, "Deploy new files.");

		// Копируем новые файлы.
		foreach (auto comp, mComponents)
		{
			comp->deploy(comp->getFiles().intersect(newFiles), mWorkingDir);
		}

		Log(LogLevel::Normal, "Applying post actions.");

		// Выполняем общие действия.
		foreach (auto comp, mComponents)
		{
			comp->applyPostActions(mWorkingDir);
		}

		Log(LogLevel::Normal, "Removing empty folders.");

		removeEmptyFolders(mWorkingDir);

		// Сохраняем успешную конфигурацию на диск
		saveUpdateConfiguration();

		// Завершаем работу приложения.
		emit done(CUpdaterErrors::OK);
	}
	catch (Exception & e)
	{
		Log(LogLevel::Fatal, QString("Failed to deploy update: %1.").arg(e.getMessage()));

		Log(LogLevel::Normal, "Restoring backup.");

		try
		{
			Log(LogLevel::Normal, "Deleting new files.");
			// Удаляем установленные файлы.
			deleteFiles(newFiles, true);

			Log(LogLevel::Normal, "Restoring old files from backup.");
			// Восстанавливаем удаленные файлы.
			copyFiles(backupDir, mWorkingDir, oldFiles, true);
		}
		catch (Exception & e)
		{
			Log(LogLevel::Fatal, QString("Failed to restore backup. %1.").arg(e.getMessage()));
		}

		emit done(CUpdaterErrors::DeployError);
	}
}

//---------------------------------------------------------------------------
int Updater::checkInterity()
{
	QMultiMap<QString, QString> files;

	foreach(QString revision, getSavedConfigurations())
	{
		Log(LogLevel::Normal, QString("Loading package description... Revision: %1.").arg(revision));

		Updater::TComponentList components;
		auto result = loadComponents(loadUpdateConfiguration(revision), components, revision);

		if (result != CUpdaterErrors::OK)
		{
			Log(LogLevel::Error, QString("Failed to load package description revision: %1.").arg(revision));

			return -1;
		}

		foreach(auto comp, components)
		{
			// Выкидываем все опциональные и конфигурационные пакеты
			if (!comp->optional() && !comp->skipExisting())
			{
				foreach(const File & file, comp->getFiles())
				{
					files.insert(file.name(), file.hash());
				}
			}
		}
	}

	if (files.isEmpty())
	{
		Log(LogLevel::Error, "Failed to load package description.");

		return -1;
	}

	try
	{
		Log(LogLevel::Normal, "Calculating local files checksum.");
		TFileList currentStructure = getWorkingDirStructure();
		int diffFilesCount = 0;

		// Пересчитываем отличающиеся файлы.
		foreach (const File & file, currentStructure)
		{
			if (files.contains(file.name()))
			{
				if (!files.values(file.name()).contains(file.hash()))
				{
					Log(LogLevel::Error, QString("Different local file: %1.").arg(file.name()));

					diffFilesCount++;
				}

				files.remove(file.name());
				currentStructure.remove(file);
			}
		}

		// Пересчитываем лишние файлы.
		foreach(const File & file, currentStructure)
		{
			Log(LogLevel::Warning, QString("Unwanted local file: %1.").arg(file.name()));
		}

		return diffFilesCount;
	}
	catch (Exception & e)
	{
		Log(LogLevel::Fatal, e.getMessage());

		return -1;
	}
}

//---------------------------------------------------------------------------
void Updater::useBITS(bool aUseBITS, int aJobPriority)
{
	mUseBITS = aUseBITS;
	mJobPriority = aJobPriority;
}

//---------------------------------------------------------------------------
void Updater::runUpdate()
{
	Log(LogLevel::Normal, "Downloading package description...");

	Updater::TComponentList components;
	auto error = getComponents(components);

	switch (error)
	{
	case CUpdaterErrors::OK:
		Log(LogLevel::Normal, QString("Updating components:%1").arg
			(std::accumulate(components.begin(), components.end(), QString(),
			[](const QString & str, const QSharedPointer<Component> & comp){ return str + " " + comp->getId(); })));

		downloadComponents(components);
		break;

	default:
		if (mWaitUpdateServer && mFailCount < CUpdater::MaxFails)
		{
			++mFailCount;

			emit updateSystemIsWaiting();
		}
		else
		{
			Log(LogLevel::Error, "Failed to download package description.");
		
			emit done(error);
		}
		break;
	}
}

//---------------------------------------------------------------------------
int Updater::removeEmptyFolders(const QString & aDir)
{
	QDir current(aDir);

	int numFiles = 0;

	foreach (auto fileInfo, current.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files))
	{
		if (fileInfo.isDir())
		{
			int n = removeEmptyFolders(aDir + "/" + fileInfo.fileName());

			if (n == 0)
			{
				current.rmpath(aDir + "/" + fileInfo.fileName());
			}
			else
			{
				numFiles++;
			}
		}
		else
		{
			numFiles++;
		}
	}

	return numFiles;
}


//---------------------------------------------------------------------------
void Updater::setMD5(const QString & aMD5)
{
	mMD5 = aMD5;
}

//---------------------------------------------------------------------------
void Updater::downloadPackage()
{
	Package * package = nullptr;
	QList<NetworkTask *> tasks;

	if (!mConfigURL.contains("=") && !mConfigURL.contains("?"))
	{
		// Фиктивный список файлов (реальный состав может отличаться).
		QFileInfo fileInfo = mConfigURL.section("/", -1, -1);
		auto fileList = TFileList() << File("config.xml", "", "");
		package = new Package(fileInfo.completeBaseName(), "1", fileList, QStringList(), "", mMD5, 0);
		tasks = package->download(mConfigURL + "?", TFileList());
	}
	else
	{
		// Обманываем алгоритм для скачивания файла через запрос к скрипту
		auto fileList = TFileList() << File("config.xml", "", "");
		package = new Package(mMD5, "1", fileList, QStringList(), "", mMD5, 0);
		tasks = package->download(mConfigURL, TFileList());
	}

	if (tasks.isEmpty())
	{
		// файл уже скачан
		Log(LogLevel::Normal, QString("File %1.zip was already downloaded.").arg(package->getId()));

		QMetaObject::invokeMethod(this, "deployDownloadedPackage",  Qt::QueuedConnection, Q_ARG(QObject *, package));
	}
	else
	{
		auto task = tasks.at(0);

		mMapper.setMapping(task, package);
		mMapper.connect(task, SIGNAL(onComplete()), SLOT(map()));
		connect(&mMapper, SIGNAL(mapped(QObject *)), SLOT(packageDownloaded(QObject *)), Qt::UniqueConnection);

		Log(LogLevel::Normal, QString("Downloading file %1...").arg(mConfigURL));

		// Запускаем закачку.
		mCurrentTaskSize = task->getDataStream()->size();
		mNetworkTaskManager.addTask(task);
	}
}

//---------------------------------------------------------------------------
void Updater::packageDownloaded(QObject * aPackage)
{
	auto task = qobject_cast<NetworkTask *>(mMapper.mapping(aPackage));
	auto package = qobject_cast<Package *>(aPackage);

	bool haveNewData = (mCurrentTaskSize != task->getDataStream()->size());
	bool retryCountReached = ++mFailCount >= CUpdater::MaxFails;

	if (!task->getError() || task->getError() == NetworkTask::TaskFailedButVerified)
	{
		Log(LogLevel::Normal, QString("File %1.zip was downloaded successfully.").arg(package->getId()));

		closeFileTask(task);

		deployDownloadedPackage(package);
	}
	else
	{
		Log(LogLevel::Error, QString("Failed to download file %1. Network error: %2.").arg(package->getId()).arg(task->errorString()));

		if (task->getError() && task->getHttpError() == 416) // 416 - Requested Range Not Satisfiable
		{
			checkTaskVerifierResult(task);
		}

		if (retryCountReached)
		{
			emit done(CUpdaterErrors::NetworkError);
		}
		else
		{
			int timeout = CUpdater::MinutesBeforeNextTry * 60 * 1000;

			if ((task->getHttpError() / 100) == 2)
			{
				timeout = 1000;
				Log(LogLevel::Error, "Continue download package...");
			}
			else
			{
				Log(LogLevel::Error, QString("Waiting %1 minutes before next try...").arg(CUpdater::MinutesBeforeNextTry));
			}

			closeFileTask(task);
			QTimer::singleShot(timeout, this, SLOT(downloadPackage()));
		}
	}

	task->deleteLater();
	package->deleteLater();
}

//---------------------------------------------------------------------------
void Updater::deployDownloadedPackage(QObject * aPackage)
{
	emit deployment();

	auto package = qobject_cast<Package *>(aPackage);

	// Распаковываем архив.
	try
	{
		package->deploy(package->getFiles(), mWorkingDir);

		Log(LogLevel::Normal, QString("File %1.zip was sucessfully unpacked.").arg(package->getId()));

		emit done(CUpdaterErrors::OK);
	}
	catch (Exception & e)
	{
		Log(LogLevel::Fatal, QString("Failed to deploy file %1.zip. (%2)").arg(package->getId()).arg(e.getMessage()));

		emit done(CUpdaterErrors::DeployError);
	}
}

//---------------------------------------------------------------------------
void Updater::setOptionalComponents(const QStringList & aComponents)
{
	mOptionalComponents = aComponents;
}

//---------------------------------------------------------------------------
void Updater::setConfigurationRequiredFiles(const QStringList & aRequredFiles)
{
	mRequredFiles = aRequredFiles;
}

//---------------------------------------------------------------------------
bool Updater::validateConfiguration(const TComponentList & aComponents)
{
	if (aComponents.isEmpty())
	{
		Log(LogLevel::Error, "Update configuration not valid: empty component list.");

		return false;
	}

	foreach (auto requiredFile, mRequredFiles)
	{
		bool exist = false;

		foreach(auto component, aComponents)
		{
			auto files = component->getFiles();

			exist |= (std::find_if(files.begin(), files.end(), 
				[&](const File & aFile) -> bool { return aFile.name().endsWith(requiredFile, Qt::CaseInsensitive); }) != files.end());
		}

		if (!exist)
		{
			Log(LogLevel::Error, QString("Update configuration not valid: not exist '%1' file.").arg(requiredFile));

			return false;
		}
	}

	return true;
}

//---------------------------------------------------------------------------
bool Updater::bitsDownload()
{
	if (!mBitsManager.isReady() || !mUseBITS)
	{
		return false;
	}

	if (bitsInProgress())
	{
		return true;
	}

	bitsCleanupOldTasks();

	// Save job info into ini file with current updater arguments
	bitsSaveState();

	// Устанавливаем после выполнения задачи запуск себя с теми же самыми параметрами
	mBitsManager.setNotify(qApp->applicationFilePath(), QString("--command bits --workdir %1").arg(mWorkingDir));

	CBITS::SJob job;
	if (mBitsManager.createJob(bitsJobName(), job, mJobPriority))
	{
		foreach (auto task, mActiveTasks)
		{
			auto fileTask = dynamic_cast<FileDownloadTask *>(task);
			if (fileTask)
			{
				fileTask->closeFile();
				if (!mBitsManager.addTask(fileTask->getUrl(), fileTask->getPath()))
				{
					Log(LogLevel::Error, "Error add task to BITS job.");

					mBitsManager.shutdown();
					// перейти обратно к схеме скачивания вручную
					return false;
				}
			}
		}

		// Запускаем на скачивание задание		
		if (mBitsManager.resume())
		{
			Log(LogLevel::Normal, QString("BITS job '%1' create successful.").arg(bitsJobName()));

			// закрываем updater с кодом - "команда выполняется"
			emit done(CUpdaterErrors::BitsInProgress);
			return true;
		}
		else
		{
			Log(LogLevel::Error, QString("Error resume BITS job '%1'.").arg(bitsJobName()));
		}
	}
	else
	{
		Log(LogLevel::Error, "Error create BITS job.");
	}

	mBitsManager.shutdown();

	// перейти обратно к схеме скачивания вручную
	return false;
}

//---------------------------------------------------------------------------
void Updater::bitsCompleteAllJobs(int & aCount, int & aCountComplete, int & aCountError)
{
	auto jobs = mBitsManager.getJobs(bitsJobName());
	aCount = jobs.size();
	aCountComplete = 0;
	aCountError = 0;

	auto complete = [this](const CBITS::SJob & job) -> bool {
		if (!mBitsManager.openJob(job))
		{
			Log(LogLevel::Error, QString("BITS job '%1' error open.").arg(job.mName));

			return false;
		}

		if (!mBitsManager.complete())
		{
			Log(LogLevel::Error, QString("BITS job '%1' failed complete.").arg(job.mName));

			return false;
		}

		return true;
	};

	foreach(QString jobName, jobs.keys())
	{
		// Проверяем состояние таска
		auto job = jobs.value(jobName);

		if (job.isComplete())
		{
			Log(LogLevel::Normal, QString("BITS job '%1' download complete.").arg(job.mName));

			if (!complete(job))
			{
				aCountError++;
			}
			else
			{
				aCountComplete++;
			}
		}
		else if (job.isFatal())
		{
			// Всё равно коммитим то, что удалось скачать
			Log(LogLevel::Error, QString("BITS job '%1' failed.").arg(job.mName));

			complete(job);

			aCountError++;
		}
		else
		{
			Log(LogLevel::Normal, QString("BITS job '%1' in progress.").arg(job.mName));
		}
	}
}

//---------------------------------------------------------------------------
bool Updater::bitsInProgress()
{
	int count = 0;
	int countComplete = 0;
	int countError = 0;

	bitsCompleteAllJobs(count, countError, countComplete);

	if (countError)
	{
		bitsCleanupOldTasks();

		// возвращаем ошибку скачивания задания
		emit done(CUpdaterErrors::NetworkError);
		return true;
	}
	else if (count && countComplete == count)
	{
		// передаем управление дальше на распаковку
		emit downloadAccomplished();
		return true;
	}
	else if (count)
	{
		// Скачивание в процессе, просто выходим со статусом "в процессе"
		emit done(CUpdaterErrors::BitsInProgress);
		return true;
	}

	return false;
}

//---------------------------------------------------------------------------
void Updater::bitsCleanupOldTasks()
{
	Log(LogLevel::Normal, QString("Cancel all bits jobs."));

	// Останавливаем все наши таски от предыдущих версий.
	auto jobs = mBitsManager.getJobs(CUpdater::BitsJobNamePrefix);

	foreach(const QString & jobName, jobs.keys())
	{
		if (jobName.startsWith(CUpdater::BitsJobNamePrefix) &&
			mBitsManager.openJob(jobs[jobName]))
		{
			Log(LogLevel::Normal, QString("Cancel old bits job '%1'.").arg(jobName));

			mBitsManager.cancel();
		}
	}
}

//---------------------------------------------------------------------------
QString Updater::bitsJobName() const
{
	return CUpdater::BitsJobNamePrefix + QString("%1_%2_%3").arg(mAppId).arg(mConfiguration).arg(mVersion);
}

//---------------------------------------------------------------------------
CUpdaterErrors::Enum Updater::loadComponents(const QByteArray & aContent, Updater::TComponentList & aComponents, QString & aRevision)
{
	QDomDocument description;

	if (!description.setContent(aContent))
	{
		Log(LogLevel::Error, "Failed to parse component description.");

		return CUpdaterErrors::ParseError;
	}

	QDomElement application = description.documentElement();

	QString revision = application.attribute("revision", "");

	if (revision.isEmpty())
	{
		Log(LogLevel::Error, "Revision number is missing.");
		return CUpdaterErrors::ParseError;
	}

	aRevision = revision;

	// Все компоненты из файла
	Updater::TComponentList  allComponents;

	QRegExp leadingSlash("^[\\\\/]");

	// Получаем список компонент.
	for (QDomNode node = application.firstChild(); !node.isNull(); node = node.nextSibling())
	{
		QDomElement component = node.toElement();

		if (component.tagName() == "component")
		{
			auto componentType = component.attribute("type");
			auto componentName = component.attribute("name");
			auto componentUrl = component.attribute("url");
			auto componentSize = component.attribute("size").toInt();
			auto componentHash = component.attribute("hash_sha256");
			auto skipExisting = component.attribute("skip_existing");
			auto componentOptional = component.attribute("optional");

			// Получаем список файлов и действий.
			TFileList files;
			QStringList actions;

			for (QDomNode node = component.firstChild(); !node.isNull(); node = node.nextSibling())
			{
				auto record = node.toElement();

				if (record.tagName() == "file")
				{
					files.insert(File(record.attribute("path").remove(leadingSlash), record.attribute("hash_sha256"),
						record.attribute("url"), record.attribute("size").toInt()));
					continue;
				}

				if (record.tagName() == "post-action")
				{
					auto name = record.attribute("path").remove(leadingSlash);
					// auto url = record.attribute("url");

					actions.append(name);
					continue;
				}
			}

			QSharedPointer<Component> newComponent;

			// Создаем нужный класс в зависимости от типа компонента.
			if (componentType == "package")
			{
				newComponent = QSharedPointer<Component>(new Package(componentName, aRevision, files, actions, componentUrl, componentHash, componentSize));
			}
			else if (componentType == "folder")
			{
				newComponent = QSharedPointer<Component>(new Folder(componentName, aRevision, files, actions, componentUrl));
			}

			if (newComponent)
			{
				newComponent->setOptional(mOptionalComponents.contains(componentName, Qt::CaseInsensitive) || componentOptional.contains("true", Qt::CaseInsensitive));
				newComponent->setSkipExisting(skipExisting.contains("true", Qt::CaseInsensitive));

				// если список разрешенных компонент пустой или в нем есть текущая компонента, то добавляем её в список
				if (mUpdateComponents.isEmpty() || mUpdateComponents.contains(componentName, Qt::CaseInsensitive))
				{
					aComponents.append(newComponent);
				}

				allComponents.append(newComponent);

				continue;
			}

			Log(LogLevel::Error, QString("Component %1 type is unknown: %2.").arg(componentName).arg(componentType));
		}
	}

	return validateConfiguration(allComponents) ? CUpdaterErrors::OK : CUpdaterErrors::ParseError;
}

//---------------------------------------------------------------------------
void Updater::setAcceptedKeys(const QString & aAcceptedKeys)
{
	mAcceptedKeys = aAcceptedKeys;
}

//---------------------------------------------------------------------------
void Updater::bitsSaveState()
{
	QSettings settings(QDir(mWorkingDir + CUpdater::UpdaterConfigurationDir).absoluteFilePath("bits.ini"), QSettings::IniFormat);

	settings.beginGroup("bits");
	settings.setValue("job_name", bitsJobName());
	settings.setValue("create_stamp", QDateTime::currentMSecsSinceEpoch());
	settings.setValue("create_stamp_for_user", QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
	settings.endGroup();

	settings.beginGroup("updater");
	settings.setValue("working_dir", mWorkingDir);
	settings.setValue("config_url", mConfigURL);
	settings.setValue("update_url", mUpdateURL);
	settings.setValue("version", mVersion);
	settings.setValue("app_id", mAppId);
	settings.setValue("configuration", mConfiguration);
	settings.setValue("ap", mAP);
	settings.endGroup();

	auto params = qApp->arguments();
	params.takeFirst();

	settings.beginWriteArray("parameters");
	for (int i = 0; i < params.size(); i++)
	{
		settings.setArrayIndex(i);
		settings.setValue("arg", params[i]);
	}
	settings.endArray();
}

//---------------------------------------------------------------------------
bool Updater::bitsLoadState(QStringList * aParameters)
{
	QSettings settings(QDir(mWorkingDir + CUpdater::UpdaterConfigurationDir).absoluteFilePath("bits.ini"), QSettings::IniFormat);

	if (settings.status() != QSettings::NoError)
	{
		return false;
	}

	settings.beginGroup("updater");
	mWorkingDir = settings.value("working_dir").toString();
	mConfigURL = settings.value("config_url").toString();
	mUpdateURL = settings.value("update_url").toString();
	mVersion = settings.value("version").toString();
	mAppId = settings.value("app_id").toString();
	mConfiguration = settings.value("configuration").toString();
	mAP = settings.value("ap").toString();
	settings.endGroup();

	if (aParameters)
	{
		int count = settings.beginReadArray("parameters");
		for (int i = 0; i < count; i++)
		{
			settings.setArrayIndex(i);
			aParameters->append(settings.value("arg").toString());
		}
		settings.endArray();
	}

	return !mAppId.isEmpty() && !mConfiguration.isEmpty() && !mVersion.isEmpty();
}

//---------------------------------------------------------------------------
bool Updater::bitsIsComplete()
{
	Log(LogLevel::Normal, QString("BITS job name: %1.").arg(bitsJobName()));

	auto jobs = mBitsManager.getJobs(bitsJobName());
	int countComplete = 0;

	foreach(QString jobName, jobs.keys())
	{
		// Проверяем состояние таска
		auto job = jobs.value(jobName);

		Log(LogLevel::Normal, QString("JOB: %1 has state=%2.").arg(job.mName).arg(job.mState));

		if (job.isComplete())
		{
			Log(LogLevel::Normal, QString("BITS job '%1' complete.").arg(job.mName));

			countComplete++;
		}
	}

	return jobs.count() && jobs.count() == countComplete;
}

//---------------------------------------------------------------------------
bool Updater::bitsIsError()
{
	auto jobs = mBitsManager.getJobs(bitsJobName());
	int badJobs = 0;

	foreach(QString jobName, jobs.keys())
	{
		// Проверяем состояние таска
		auto job = jobs.value(jobName);

		if (job.isFatal())
		{
			Log(LogLevel::Normal, QString("BITS job '%1' failed.").arg(job.mName));

			if (mBitsManager.openJob(job))
			{
				mBitsManager.complete();
			}

			badJobs++;
		}
	}

	if (badJobs)
	{
		// Если есть плохие, то остальные закрываем в любом случае.

		foreach(QString jobName, jobs.keys())
		{
			// Проверяем состояние таска
			auto job = jobs.value(jobName);

			if (!job.isFatal())
			{
				if (mBitsManager.openJob(job))
				{
					mBitsManager.complete();
				}
			}
		}
	}

	return badJobs > 0;
}

//---------------------------------------------------------------------------
