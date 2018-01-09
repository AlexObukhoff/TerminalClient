/* @file Cross-platform tool for content management on post build step. */

// Stl
#include <iostream>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <QtCore/QSet>
#include <QtCore/QSettings>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>
#include <QtCore/QProcessEnvironment>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <Common/QtHeadersEnd.h>

// Модули
#include <Common/Application.h>
#include <Common/ScopedPointerLaterDeleter.h>
#include <NetworkTaskManager/NetworkTaskManager.h>
#include <NetworkTaskManager/FileDownloadTask.h>

// Проект
#include "RegExpList.h"

//---------------------------------------------------------------------------
const int OK = 0;
const int INVALID_ARGUMENTS = 1;
const int INVALID_SETTINGS_FILE = 2;
const int INVALID_NODE = 3;
const int INVALID_BASE_SCRIPT = 4;

#define PRETTY_PATH(path) QDir::toNativeSeparators(QDir::cleanPath(path))

//---------------------------------------------------------------------------
QMap<QString, QString> gEnvironment;
QString gSettingsFile;
QString gSourceDir;
QString gTargetDir;
QScriptEngine * gScriptEngine;

NetworkTaskManager gNetworkTaskManager;

//---------------------------------------------------------------------------
QString expandMacros(const QString & aSource, bool aCleanUnknown = true)
{
	// Разворачиваем макросы
	QString result = aSource;
	QRegExp rx("\\{(\\w+)\\}");
	QSet<QString> macros;
	int pos = 0;

	while ((pos = rx.indexIn(aSource, pos)) != -1)
	{
		macros << rx.cap(1);
		pos += rx.matchedLength();
	}

	foreach (QString macro, macros)
	{
		if (aCleanUnknown)
		{
			result.replace(QRegExp(QString("\\{%1\\}").arg(macro)), gEnvironment.value(macro, ""));
		}
		else if (gEnvironment.contains(macro))
		{
			result.replace(QRegExp(QString("\\{%1\\}").arg(macro)), gEnvironment.value(macro));
		}
	}

	return result;
}

//---------------------------------------------------------------------------
bool testCondition(const QString & aCondition)
{
	if (aCondition.isEmpty())
	{
		return true;
	}

	QString script = expandMacros(aCondition);

	if (script.isEmpty())
	{
		std::cout << " condition \"" << aCondition.toStdString() << " is empty after macros expanding." << std::endl;
		return false;
	}

	std::cout << " testing condition \"" << aCondition.toStdString() << "\" >> \"" << script.toStdString() << "\"" << std::endl;
	QScriptValue result = gScriptEngine->evaluate(script);

	if (gScriptEngine->hasUncaughtException())
	{
		std::cout << " failed to evaluate the condition script: " << gScriptEngine->uncaughtException().toString().toStdString() << std::endl;
		return false;
	}

	if (!result.isBool())
	{
		std::cout << " the result of evaluating the condition script is not a boolean value!" << std::endl;
		return false;
	}

	return result.toBool();
}

//---------------------------------------------------------------------------
QString buildFullPath(const QString & aSource, const QString & aRootDir)
{
	QString result = expandMacros(aSource);

	if (!QDir::isAbsolutePath(result)
// Путь, начинающийся со слэша считается в Qt абсолютным даже в винде, а это неправильно, т.к. не учитывается диск
#ifdef Q_OS_WIN
		|| result.startsWith(QDir::separator())
#endif
	)
	{
		result = aRootDir + QDir::separator() + result;
	}

	return PRETTY_PATH(result);
}

//---------------------------------------------------------------------------
bool copyDirectory(const QString & aFrom, const QString & aTo, bool aRecursive = false, const RegExpList & aIgnoreList = RegExpList(), int aLogLevel = 2)
{
	QString logPrefix(aLogLevel, QChar(' '));

	QDir dir(aFrom);
	QStringList entries;

	if (!dir.mkpath(aTo))
	{
		std::cout << logPrefix.toStdString() << "failed to create path " << aTo.toStdString() << "." << std::endl;
		return false;
	}

	entries = dir.entryList(QDir::NoDotAndDotDot | QDir::Files);

	// Файлы
	foreach (const QString entry, entries)
	{
		if (aIgnoreList.contains(entry))
		{
			std::cout << "Ignore " << entry.toStdString() << std::endl;
			continue;
		}

		QString source = PRETTY_PATH(aFrom + QDir::separator() + entry);
		QString target = PRETTY_PATH(aTo + QDir::separator() + entry);

		std::cout << logPrefix.toStdString() << "copying " << entry.toStdString() << " ... ";

		if (QFile::exists(target))
		{
			QFile::remove(target);
		}

		if (!QFile::copy(source, target))
		{
			std::cout << "failed!" << std::endl;
			return false;
		}

		std::cout << "ok." << std::endl;
	}

	// Папки
	if (aRecursive)
	{
		++aLogLevel;

		entries = dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs);

		foreach (const QString entry, entries)
		{
			if (aIgnoreList.contains(entry))
			{
				std::cout << logPrefix.toStdString() << "path /" << entry.toStdString() << " skipping." << std::endl;

				continue;
			}

			QString source = PRETTY_PATH(aFrom + QDir::separator() + entry);
			QString target = PRETTY_PATH(aTo + QDir::separator() + entry);

			std::cout << logPrefix.toStdString() << "path: /" << entry.toStdString();

			QDir dir(source);

			if (!dir.exists(target))
			{
				std::cout << " (creating ... ";

				if (!dir.mkpath(target))
				{
					std::cout << "failed!)" << std::endl;
					return false;
				}

				std::cout << "ok)";
			}

			std::cout << std::endl;

			if (!copyDirectory(source, target, aRecursive, aIgnoreList, aLogLevel))
			{
				std::cout << logPrefix.toStdString() << "failed to copy subdirectory." << std::endl;
				return false;
			}
		}
	}

	return true;
}

//---------------------------------------------------------------------------
bool parseDirectoryNode(const QDomElement & aNode)
{
	std::cout << "Processing directory..." << std::endl;

	QString condition = aNode.attribute("if");
	if (!testCondition(condition))
	{
		std::cout << " skipping due to \"" << condition.toStdString() << "\" condition." << std::endl;
		return true;
	}

	RegExpList ignoreList;

	// По умолчанию всегда игнорируем служебную папку SVN.
	ignoreList.add(".svn");

	QDomElement ignoreElement = aNode.firstChildElement("exclude");
	while (!ignoreElement.isNull())
	{
		ignoreList.add(ignoreElement.text());

		ignoreElement = ignoreElement.nextSiblingElement();
	}

	QString source = aNode.attribute("source");
	QString target = aNode.attribute("target");

	if (source.isEmpty())
	{
		std::cout << "	<source> attribute can't be empty." << std::endl;
		return false;
	}

	source = buildFullPath(source, gSourceDir);
	target = buildFullPath(target, gTargetDir);

	bool recursive = QVariant(aNode.attribute("recursive", "false")).toBool();

	std::cout << "  source: " << source.toStdString() << std::endl;
	std::cout << "  target: " << target.toStdString() << std::endl;
	std::cout << "  recursive : " << recursive << std::endl << std::endl;

	if (!copyDirectory(source, target, recursive, ignoreList))
	{
		return false;
	}

	std::cout << "Done." << std::endl;

	return true;
}

//---------------------------------------------------------------------------
bool parseFileNode(const QDomElement & aNode)
{
	std::cout << "Processing file..." << std::endl;

	QString condition = aNode.attribute("if");
	if (!testCondition(condition))
	{
		std::cout << " skipping due to \"" << condition.toStdString() << "\" condition." << std::endl;
		return true;
	}

	QString content = aNode.nodeValue().trimmed();
	QString source = aNode.attribute("source");
	QString target = aNode.attribute("target");

	// Для разбора внутреннего CDATA
	if (aNode.hasChildNodes() && content.isEmpty())
	{
		content = aNode.firstChild().nodeValue();
	}

	if ((content.isEmpty() && source.isEmpty()) || target.isEmpty())
	{
		std::cout << " <content> or <source> and <target> attribute can't be empty." << std::endl;
		return false;
	}

	QString sourceFullPath = buildFullPath(source, gSourceDir);
	QString targetFullPath = buildFullPath(target, gTargetDir);

	QFileInfo info(targetFullPath);
	if (!info.dir().exists())
	{
		if (!info.dir().mkpath(info.absolutePath()))
		{
			std::cout << "failed to create target path." << std::endl;
			return false;
		}
	}
	else
	{
		if (QFile::exists(targetFullPath))
		{
			QFile::remove(targetFullPath);
		}
	}

	if (source.indexOf(QRegExp("^(http:|https:)", Qt::CaseInsensitive)) >= 0)
	{
		// Скачиваем содержимое файла по сети
		std::cout << " download " << source.toStdString() << " >>> " << target.toStdString() << " ... ";

		// Получаем с сервера файл с описанием.
		QScopedPointer<FileDownloadTask, ScopedPointerLaterDeleter<FileDownloadTask>> task(new FileDownloadTask(source, targetFullPath));
		gNetworkTaskManager.addTask(task.data());

		task->waitForFinished();

		if (task->getError() != NetworkTask::NoError)
		{
			std::cout << "failed to download. Error: " << task->errorString().toStdString() << std::endl;

			return false;
		}
	}
	else if (source.isEmpty() && !content.isEmpty())
	{
		// Содержимое файла содержится внутри файла конфигурации
		std::cout << " write >>> " << target.toStdString() << " ... ";

		QFile f(targetFullPath);
		if (!f.open(QIODevice::WriteOnly))
		{
			std::cout << "failed open target to wtite." << std::endl;
			return false;
		}
		f.write(content.toUtf8());
	}
	else
	{
		// обычное копирование файлов
		std::cout << " copying " << source.toStdString() << " >>> " << target.toStdString() << " ... ";

		if (!QFile::copy(sourceFullPath, targetFullPath))
		{
			std::cout << "failed." << std::endl;
			return false;
		}
	}

	std::cout << "ok." << std::endl;

	return true;
}

//---------------------------------------------------------------------------
bool parseLocaleNode(const QDomElement & aNode)
{
	std::cout << "Processing locale..." << std::endl;

	QString condition = aNode.attribute("if");
	if (!testCondition(condition))
	{
		std::cout << " skipping due to \"" << condition.toStdString() << "\" condition." << std::endl;
		return true;
	}

	QString source = aNode.attribute("source");
	QString target = aNode.attribute("target");

	if (source.isEmpty() || target.isEmpty())
	{
		std::cout << " <source> and <target> attribute can't be empty." << std::endl;
		return false;
	}

	QString sourceFullPath = buildFullPath(source, gSourceDir);
	QString targetFullPath = buildFullPath(target, gTargetDir);

	QFileInfo info(targetFullPath);
	if (!info.dir().exists())
	{
		if (!info.dir().mkpath(info.absolutePath()))
		{
			std::cout << "failed to create target path." << std::endl;
			return false;
		}
	}
	else
	{
		if (QFile::exists(targetFullPath))
		{
			QFile::remove(targetFullPath);
		}
	}

	// обычное копирование файлов
	std::cout << " compiling " << source.toStdString() << " >>> " << target.toStdString() << " ... " << std::endl;

	if (QProcess::execute("lrelease", QStringList() << sourceFullPath << "-qm" << targetFullPath))
	{
		std::cout << "failed." << std::endl;
		return false;
	}

	std::cout << "ok." << std::endl;

	return true;
}

//---------------------------------------------------------------------------
bool parseRenameNode(const QDomElement & aNode)
{
	std::cout << "Processing rename..." << std::endl;

	QString condition = aNode.attribute("if");
	if (!testCondition(condition))
	{
		std::cout << " skipping due to \"" << condition.toStdString() << "\" condition." << std::endl;
		return true;
	}

	QString source = aNode.attribute("source");
	QString target = aNode.attribute("target");

	if (source.isEmpty() || target.isEmpty())
	{
		std::cout << " <source> and <target> attribute can't be empty." << std::endl;
		return false;
	}

	QString sourceFullPath = buildFullPath(source, gSourceDir);
	QString targetFullPath = buildFullPath(target, gTargetDir);

	// обычное копирование файлов
	std::cout << " renaming " << source.toStdString() << " >>> " << target.toStdString() << " ... " << std::endl;

	if (QFileInfo(sourceFullPath).isFile())
	{
		QFile file(sourceFullPath);
		if (!file.rename(targetFullPath))
		{
			std::cout << " error rename file from '" << sourceFullPath.toStdString() << "' to '" << targetFullPath.toStdString() << "': " << file.errorString().toStdString() << std::endl;
			return false;
		}
	}
	else
	{
		QDir dir;
		if (!dir.rename(sourceFullPath, targetFullPath))
		{
			std::cout << " error rename dir from '" << sourceFullPath.toStdString() << "' to '" << targetFullPath.toStdString() << "'." << std::endl;
			return false;
		}
	}

	std::cout << "ok." << std::endl;

	return true;
}

//---------------------------------------------------------------------------
bool parseOptionNode(const QDomElement & aNode)
{
	std::cout << "Processing option..." << std::endl;

	QString condition = aNode.attribute("if");
	if (!testCondition(condition))
	{
		std::cout << " skipping due to \"" << condition.toStdString() << "\" condition." << std::endl;
		return true;
	}

	QString target = aNode.attribute("target");
	QString key = aNode.attribute("key");
	QString value = expandMacros(aNode.attribute("value"), false);

	if (target.isEmpty() || key.isEmpty())
	{
		std::cout << "<target> and <key> tags can't be empty." << std::endl;
		return false;
	}

	std::cout << " target: " << target.toStdString();
	std::cout << " key: " << key.toStdString();
	std::cout << " value: " << value.toStdString() << std::endl;

	QSettings settings(buildFullPath(target, gTargetDir), QSettings::IniFormat);
	settings.setIniCodec("UTF-8");
	if (value.contains(","))
	{
		// Записываем как string list, т.к. иначе получим в ini значение в кавычках
		settings.setValue(key, value.split(","));
	}
	else
	{
		settings.setValue(key, value);
	}

	return true;
}

//---------------------------------------------------------------------------
int parseRuntime(const QString & aFile)
{
	QFile xmlFile(aFile);
	if (!xmlFile.open(QIODevice::ReadOnly))
	{
		std::cout << "Failed to open settings file. Error message: " << xmlFile.errorString().toStdString();
		return INVALID_SETTINGS_FILE;
	}

	QDomDocument xml;
	xml.setContent(xmlFile.readAll());

	// Проверим на наследование от базового сценария
	if (xml.documentElement().hasAttribute("base"))
	{
		QString baseRuntime = xml.documentElement().attribute("base");

		std::cout << "Executing base script: " << baseRuntime.toStdString() << std::endl;

		QFileInfo info(aFile);

		if (parseRuntime(PRETTY_PATH(info.absolutePath() + QDir::separator() + baseRuntime + ".xml")) != OK)
		{
			std::cout << "Failed to execute base script." << std::endl;
			return INVALID_BASE_SCRIPT;
		}

		std::cout << "Base script executed: " << baseRuntime.toStdString() << std::endl;
	}

	QDomNodeList nodes = xml.documentElement().childNodes();

	for (int i = 0; i < nodes.count(); ++i)
	{
		int result = true;

		QDomElement element = nodes.at(i).toElement();
		if (!element.isNull())
		{
			if (element.tagName() == "file")
			{
				result = parseFileNode(element);
			}
			else if (element.tagName() == "directory")
			{
				result = parseDirectoryNode(element);
			}
			else if (element.tagName() == "option")
			{
				result = parseOptionNode(element);
			}
			else if (element.tagName() == "locale")
			{
				result = parseLocaleNode(element);
			}
			else if (element.tagName() == "rename")
			{
				result = parseRenameNode(element);
			}
		}

		if (!result)
		{
			std::cout << "Failed to process node." << std::endl;
			return INVALID_NODE;
		}
	}

	return OK;
}

//---------------------------------------------------------------------------
void prepareEnviroment()
{
	auto enviroment = QProcessEnvironment::systemEnvironment();

	foreach(auto key, enviroment.keys())
	{
		gEnvironment.insert(key, enviroment.value(key));
	}

	gEnvironment.insert("COMPOSER_SOURCE_DIR", gSourceDir);
	gEnvironment.insert("COMPOSER_TARGET_DIR", gTargetDir);
}

//---------------------------------------------------------------------------
int main(int argc, char * argv[])
{
	BasicQtApplication<QCoreApplication> a("Composer", "1", argc, argv);

	if (a.getQtApplication().arguments().count() < 4)
	{
		std::cout << "Wrong argument count. Expected arguments: <settings_file> <source_dir> <target_dir>." << std::endl;

		return INVALID_ARGUMENTS;
	}

	gScriptEngine = new QScriptEngine(&a.getQtApplication());
	gSettingsFile = a.getQtApplication().arguments().at(1);
	gSourceDir = a.getQtApplication().arguments().at(2);
	gTargetDir = a.getQtApplication().arguments().at(3);
	gNetworkTaskManager.setLog(ILog::getInstance(a.getName(), LogType::Console));

	if (!QDir::isAbsolutePath(gSettingsFile))
	{
		gSettingsFile = a.getQtApplication().applicationDirPath() + QDir::separator() + gSettingsFile;
	}

	if (!QDir::isAbsolutePath(gSourceDir))
	{
		gSourceDir = a.getQtApplication().applicationDirPath() + QDir::separator() + gSourceDir;
	}

	if (!QDir::isAbsolutePath(gTargetDir))
	{
		gTargetDir = a.getQtApplication().applicationDirPath() + QDir::separator() + gTargetDir;
	}

	gSettingsFile = PRETTY_PATH(gSettingsFile);
	gSourceDir = PRETTY_PATH(gSourceDir);
	gTargetDir = PRETTY_PATH(gTargetDir);

	std::cout << "Settings file: " << gSettingsFile.toStdString() << std::endl;
	std::cout << "Source dir: " << gSourceDir.toStdString() << std::endl;
	std::cout << "Target dir: " << gTargetDir.toStdString() << std::endl;
	std::cout << "--------------------------------------------------" << std::endl;

	prepareEnviroment();

	return parseRuntime(gSettingsFile);
}

//---------------------------------------------------------------------------
