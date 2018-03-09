/* @file Компонент дистрибутива - архив. */

#include <numeric>

//Qt 
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/Exception.h>
#include <Common/ScopedPointerLaterDeleter.h>
#include <NetworkTaskManager/NetworkTaskManager.h>
#include <NetworkTaskManager/FileDownloadTask.h>
#include <NetworkTaskManager/IVerifier.h>
#include <NetworkTaskManager/HashVerifier.h>
#include <Packer/Packer.h>

// Проект
#include "Misc.h"
#include "Package.h"

//---------------------------------------------------------------------------
// Класс для проверки целостности архива.
class ZipArchiveVerifier : public IVerifier
{
	ILog * mLog;

public:
	ZipArchiveVerifier(ILog * aLog) 
		: mLog(aLog)
	{
	}

	virtual bool verify(NetworkTask * aTask, const QByteArray & /*aData*/)
	{
		FileDownloadTask * task = dynamic_cast<FileDownloadTask *>(aTask);

		if (task)
		{
			// Закрываем файл для корректной работы 7-Zipа.
			task->closeFile();

			Packer zip("", mLog);
			return zip.test(task->getPath());
		}

		return false;
	}
};

//---------------------------------------------------------------------------
Package::Package(const QString & aName, const QString & aVersion, const TFileList & aFiles, const QStringList & aPostActions, const QString & aURL, const QString & aHash, int aSize)
	: Component(aName, aVersion, aFiles, aPostActions, aURL), mHash(aHash), mSize(aSize)
{
}

//---------------------------------------------------------------------------
QList<NetworkTask *> Package::download(const QString & aBaseURL, const TFileList & aExceptions)
{
	QString URL = getURL(getId() + ".zip", aBaseURL);
	QString filePath = getTemporaryFolder() + "/" + getId() + ".zip";

	QList<NetworkTask *> tasks;

	if (!aExceptions.contains(getFiles()) || getPostActions().size() > 0)
	{
		File componentFile("", mHash, URL, mSize);

		switch (componentFile.verify(filePath))
		{
		case File::OK: // качать не нужно
			Log(LogLevel::Normal, QString("File '%1' already downloaded into temp folder. Skip download.").arg(filePath));
			return tasks;

		case File::Error:
			Log(LogLevel::Warning, QString("Wrong file '%1' in temp folder. Remove it and renew download.").arg(filePath));
			QFile::remove(filePath);
			// break тут не нужен!
		
		default:
			auto task = new FileDownloadTask(URL, filePath);
			if (mHash.isEmpty())
			{
				task->setVerifier(new ZipArchiveVerifier(Log()));
			}
			else if (mHash.size() == CHashVerifier::MD5HashSize)
			{
				task->setVerifier(new Md5Verifier(mHash));
			}
			else
			{
				task->setVerifier(new Sha256Verifier(mHash));
			}

			task->setProperty(CComponent::OptionalTask(), optional());
			tasks << task;
		}
	}

	return tasks;
}

//---------------------------------------------------------------------------
void Package::deploy(const TFileList & aFiles, const QString & aDestination) throw (std::exception)
{
	if (!aFiles.isEmpty())
	{
		Log(LogLevel::Normal, QString("Deploying package %1%2...").arg(getId()).arg(mSkipExisting ? " with skip existing" : ""));

		// Распаковываем архив в папку назначения.
		Packer unzip("", Log());

		if (optional() && QFile(QDir::toNativeSeparators(getTemporaryFolder() + "/" + getId() + ".zip")).size() <= 0)
		{
			Log(LogLevel::Warning, QString("Skip optional component %1...").arg(getId()));
		}
		else
		{
			if (!unzip.unpack(QDir::toNativeSeparators(getTemporaryFolder() + "/" + getId() + ".zip"), QDir::toNativeSeparators(aDestination), mSkipExisting))
			{
				throw Exception(QString("Failed to unzip archive %1.zip error %2. Output: %3").arg(getId()).arg(unzip.exitCode()).arg(QString(unzip.messages())));
			}

			Log(LogLevel::Normal, QString("Deploying %1 OK:\n%2").arg(getId()).arg(unzip.messages()));
		}
	}
}

//---------------------------------------------------------------------------
void Package::applyPostActions(const QString & aWorkingDir) throw (std::exception)
{
	auto postActions = getPostActions();

	// Распаковываем файлы из архива во временную папку.
	if (!postActions.empty())
	{
		Log(LogLevel::Normal, QString("Extraction post actions %1...").arg(getId()));

		Packer unzip("", Log());

		if (!unzip.unpack(QDir::toNativeSeparators(getTemporaryFolder() + "/" + getId() + ".zip"), QDir::toNativeSeparators(getTemporaryFolder()), false, postActions))
		{
			throw Exception(QString("Failed to unzip files from archive %1.zip error %2. Output: %3.").arg(getId()).arg(unzip.exitCode()).arg(QString(unzip.messages())));
		}
	}

	// Запускаем их.
	foreach (auto action, postActions)
	{
		QProcess runAction;

		Log(LogLevel::Normal, QString("Running post action %1...").arg(action));

		runAction.start(getTemporaryFolder() + "/" + action, QStringList() << QDir::toNativeSeparators(aWorkingDir));
		
		//TODO: возможно стоит сделать ограничение по времени, которое отводится на работу процесса. Сейчас по умолчанию 30 секунд.
		if (!runAction.waitForFinished(CPackage::PostActionTimeout * 1000))
		{
			throw Exception(QString("Failed to run process %1.").arg(action));
		}

		if (runAction.exitCode() != 0)
		{
			throw Exception(QString("Process %1 exited with error. Return code: %2, output: %3.").arg(action).arg(runAction.exitCode()).arg(QString(runAction.readAllStandardOutput())));
		}
		else
		{
			Log(LogLevel::Normal, QString("Process %1 run OK, output: %2.").arg(action).arg(QString(runAction.readAllStandardOutput())));
		}
	}
}

//---------------------------------------------------------------------------
QString Package::getURL(const QString & aFileName, const QString & aDefaultUrl) const
{
	if (aDefaultUrl.contains("?"))
	{
		return aDefaultUrl;
	}

	return Component::getURL(aFileName, aDefaultUrl);
}

//---------------------------------------------------------------------------
