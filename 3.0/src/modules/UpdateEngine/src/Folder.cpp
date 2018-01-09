/* @file Компонент дистрибутива - индексный файл. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QStringList>
#include <QtCore/QProcess>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/Exception.h>
#include <Common/ScopedPointerLaterDeleter.h>
#include <NetworkTaskManager/FileDownloadTask.h>
#include <NetworkTaskManager/DataStream.h>
#include <NetworkTaskManager/HashVerifier.h>

// Проект
#include "Misc.h"
#include "Folder.h"

//---------------------------------------------------------------------------
Folder::Folder(const QString & aName, const QString & aVersion, const TFileList & aFiles, const QStringList & aPostActions, const QString & aURL)
	: Component(aName, aVersion, aFiles, aPostActions, aURL)
{
}

//---------------------------------------------------------------------------
QList<NetworkTask *> Folder::download(const QString & aBaseURL, const TFileList & aExceptions)
{
	QList<NetworkTask *> tasks;

	// Формируем список загружаемых файлов.
	foreach (auto fileInfo, getFiles())
	{
		if (aExceptions.contains(fileInfo))
			continue;

		QString fileURL = getURL(fileInfo, aBaseURL);
		QString dstFilePath = getTemporaryFolder() + "/" + fileInfo.name();

		// Создаем директорию назначения.
		QString dstPath = fileInfo.name().section("/", 0, -2);

		if (!dstPath.isEmpty() && !QDir(getTemporaryFolder()).mkpath(dstPath))
		{
			throw Exception(QString("Failed to create destination folder %1.").arg(dstFilePath));
		}

		switch (fileInfo.verify(dstFilePath))
		{
		case File::OK: // качать не нужно
			Log(LogLevel::Normal, QString("File '%1' already downloaded into temp folder. Skip download.").arg(fileInfo.name()));
			break;

		case File::Error:
			Log(LogLevel::Warning, QString("Wrong file '%1' in temp folder. Remove it and renew download.").arg(fileInfo.name()));
			QFile::remove(dstFilePath);
			// break тут не нужен!

		default:
			auto task = new FileDownloadTask(fileURL, dstFilePath);
			task->setVerifier(new Sha256Verifier(fileInfo.hash()));
			task->setProperty(CComponent::OptionalTask(), optional());
			tasks.append(task);
		}
	}

	// Добавляем в список исполняемые файлы для post-actionов.
	foreach (auto actionFile, getPostActions())
	{
		QString fileURL = getURL(actionFile, aBaseURL);
		auto task = new FileDownloadTask(fileURL, getTemporaryFolder() + "/" + actionFile);
		tasks.append(task);
	}

	return tasks;
}

//---------------------------------------------------------------------------
void Folder::ensureTargetDirectory(const QString & targetDir, const QString & targetFileName)
{
	QDir r(targetDir);

	QString path = QFileInfo(targetFileName).path();

	if (!path.isEmpty() && !r.exists(path))
	{
		r.mkpath(path);
	}
}

//---------------------------------------------------------------------------
void Folder::deploy(const TFileList & aFiles, const QString & aDestination) throw (std::exception)
{
	// Копируем файлы в директорию назначения.
	foreach (auto file, getFiles().intersect(aFiles))
	{
		Log(LogLevel::Normal, QString("Deploying file %1...").arg(file.name()));

		QString srcFileName = getTemporaryFolder() + "/" + file.name();
		QString destinationFileName = aDestination + "/" + file.name();

		if (optional() && QFile(srcFileName).size() <= 0)
		{
			Log(LogLevel::Warning, QString("Skip optional file %1...").arg(file.name()));
		}
		else
		{
			if (QFile::exists(destinationFileName))
			{
				if (mSkipExisting)
				{
					continue;
				}

				QFile::remove(destinationFileName);
			}
			else
			{
				ensureTargetDirectory(aDestination, file.name());
			}

			if (!QFile::copy(srcFileName, destinationFileName))
			{
				throw Exception(QString("Failed to copy file %1.").arg(file.name()));
			}
		}
	}
}

//---------------------------------------------------------------------------
void Folder::applyPostActions(const QString & aWorkingDir) throw (std::exception)
{
	// Выполняем post-actionы.
	foreach (auto action, getPostActions())
	{
		QProcess runAction;

		runAction.start(getTemporaryFolder() + "/" + action, QStringList() << aWorkingDir);

		//TODO: возможно стоит сделать ограничение по времени, которое отводится на работу процесса. Сейчас по умолчанию 30 секунд.
		if (!runAction.waitForFinished())
		{
			throw Exception(QString("Failed to start %1.").arg(action));
		}

		if (runAction.exitCode() != 0)
		{
			throw Exception(QString("Process %1 exited with error. Code %2.").arg(action).arg(runAction.exitCode()));
		}
	}
}

//---------------------------------------------------------------------------
