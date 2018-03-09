/* @file Реализация фабрики плагинов. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDirIterator>
#include <QtCore/QCoreApplication>
#include <QtCore/QTranslator>
#include <Common/QtHeadersEnd.h>

#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
#endif

// Plugin SDK
#include <SDK/Plugins/IPluginFactory.h>

#include "PluginLoader.h"

namespace SDK {
namespace Plugin {

//------------------------------------------------------------------------------
PluginLoader::PluginLoader(IKernel * aKernel)
	: mKernel(aKernel),
	  mAccessMutex(QMutex::Recursive)
{
}

//------------------------------------------------------------------------------
PluginLoader::~PluginLoader()
{
	foreach (QSharedPointer<QPluginLoader> library, mLibraries)
	{
		mKernel->getLog()->write(LogLevel::Debug, QString("Plugin '%1' will be unloaded.").arg(library->fileName()));

		// Не делаем этого, т.к. из главного модуля могут быть ссылки на данные/функции внутри библиотеки (а они есть - баг Qt).
		//library->unload();
	}
}

//------------------------------------------------------------------------------
QStringList PluginLoader::getPluginList(const QRegExp & aFilter) const
{
	QMutexLocker lock(&mAccessMutex);

	getPluginPathList(aFilter);

	return QStringList(mPlugins.keys()).filter(aFilter);
}

//------------------------------------------------------------------------------
QStringList PluginLoader::getPluginPathList(const QRegExp & aFilter) const
{
	QMutexLocker lock(&mAccessMutex);

	QStringList result;
	
	foreach(QSharedPointer<QPluginLoader> p, mLibraries)
	{
		result << p->fileName();
	}

	return QStringList(result).filter(aFilter);
}

//------------------------------------------------------------------------------
QVariantMap PluginLoader::getPluginInstanceConfiguration(const QString & aInstancePath, const QString & aConfigPath)
{
	if (!mPlugins.contains(aInstancePath))
	{
		mKernel->getLog()->write(LogLevel::Error, QString("No such plugin %1.").arg(aInstancePath));
		return QVariantMap();
	}

	QString configPath = aConfigPath.section(CPlugin::InstancePathSeparator, 0, 0);
	QString configPostfix = aConfigPath.section(CPlugin::InstancePathSeparator, 1, 1);

	return mPlugins[aInstancePath]->getPluginInstanceConfiguration(configPath, configPostfix);
}

//------------------------------------------------------------------------------
IPlugin * PluginLoader::createPlugin(const QString & aInstancePath, const QString & aConfigInstancePath)
{
	QMutexLocker lock(&mAccessMutex);

	SDK::Plugin::IPlugin * plugin = 0;

	QString path = aInstancePath.section(CPlugin::InstancePathSeparator, 0, 0);

	if (mPlugins.contains(path))
	{
		QString configInstancePath = aConfigInstancePath.isEmpty() ? aInstancePath : aConfigInstancePath;
		plugin = mPlugins[path]->createPlugin(aInstancePath, configInstancePath);

		if (plugin)
		{
			mCreatedPlugins[plugin] = mPlugins[path];
		}
	}
	else
	{
		mKernel->getLog()->write(LogLevel::Error, QString("No such plugin %1.").arg(aInstancePath));
	}

	return plugin;
}

//------------------------------------------------------------------------------
bool PluginLoader::destroyPlugin(IPlugin * aPlugin)
{
	QMutexLocker lock(&mAccessMutex);

	if (mCreatedPlugins.contains(aPlugin))
	{
		mCreatedPlugins[aPlugin]->destroyPlugin(aPlugin);
		mCreatedPlugins.remove(aPlugin);

		return true;
	}
	else
	{
		mKernel->getLog()->write(LogLevel::Error, QString("Failed to destroy plugin 0x%1, doesn't exist.").arg(qlonglong(aPlugin), 0, 16));

		return false;
	}
}

//------------------------------------------------------------------------------
int PluginLoader::addDirectory(const QString & aDirectory)
{
	QMutexLocker lock(&mAccessMutex);

	mDirectories << aDirectory;

	QStringList fileNameFilter;
	fileNameFilter << "*.dll";

	QDirIterator dirEntry(aDirectory, fileNameFilter, QDir::Files, QDirIterator::Subdirectories);

	// Загрузка библиотек
	while (dirEntry.hasNext())
	{
		dirEntry.next();

#ifdef Q_OS_WIN
		// Путь для неявной загрузки сторонних dll, которые лежат не в корне дистрибутива
		SetDllDirectory(dirEntry.fileInfo().absolutePath().toStdWString().data());
#else
	#error Handling path for implicit dll loading is not implemented for this platform!
#endif

		QSharedPointer<QPluginLoader> library(new QPluginLoader(dirEntry.filePath()));
		QObject * rootObject = library->instance();

		if (rootObject)
		{
			SDK::Plugin::IPluginFactory * factory = qobject_cast<SDK::Plugin::IPluginFactory *>(rootObject);

			if (factory)
			{
				if (factory->initialize(mKernel, dirEntry.fileInfo().absolutePath()))
				{
					mKernel->getLog()->write(LogLevel::Normal, QString("Loading %1. Name: %3. Author: %4. Version: %5.")
						.arg(dirEntry.filePath()).arg(factory->getName()).arg(factory->getAuthor()).arg(factory->getVersion()));

					mLibraries << library;

					// Загрузка локализации
					QDir translations(dirEntry.fileInfo().absolutePath(), QString("%1_*.qm").arg(dirEntry.fileInfo().baseName()));

					if (translations.count())
					{
						QString translation = translations.entryInfoList().first().absoluteFilePath();
						QScopedPointer<QTranslator> translator(new QTranslator(qApp));

						if (translator->load(translation))
						{
							qApp->installTranslator(translator.take());

							mKernel->getLog()->write(LogLevel::Normal, QString("Translation %1 for %2 loaded.").arg(translation).arg(factory->getName()));
						}
					}

					factory->translateParameters();

					// Загрузка информации о плагинах
					foreach (QString path, factory->getPluginList())
					{
						if (mPlugins.contains(path))
						{
							mKernel->getLog()->write(LogLevel::Warning, QString("Plugin %1 is already registred in %2, ignoring this one...")
								.arg(path).arg(mPlugins[path]->getName()));
						}
						else
						{
							mPlugins[path] = factory;
						}
					}
				}
				else
				{
					mKernel->getLog()->write(LogLevel::Warning, QString("Failed to initialize plugin factory in %1.").arg(dirEntry.filePath()));
					library->unload();
				}
			}
			else
			{
				mKernel->getLog()->write(LogLevel::Warning, QString("%1 doesn't support base plugin interface.").arg(dirEntry.filePath()));
				library->unload();
			}
		}
		else
		{
			mKernel->getLog()->write(LogLevel::Warning, QString("Skipping %1: %2").arg(dirEntry.filePath()).arg(library->errorString()));
		}
	}

#ifdef Q_OS_WIN
	// Сбрасываем локальные пути для неявной загрузки сторонних dll, которые лежат не в корне дистрибутива
	SetDllDirectory(0);
#else
	#error Handling path for implicit dll loading is not implemented for this platform!
#endif

	return mPlugins.count();
}

//------------------------------------------------------------------------------
TParameterList PluginLoader::getPluginParametersDescription(const QString & aPath) const
{
	QMutexLocker lock(&mAccessMutex);

	if (mPlugins.contains(aPath))
	{
		return mPlugins.value(aPath)->getPluginParametersDescription(aPath);
	}

	return TParameterList();
}

//------------------------------------------------------------------------------
}} // namespace SDK::Plugin
