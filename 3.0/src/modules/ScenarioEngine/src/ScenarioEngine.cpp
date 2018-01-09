/* @file Движок сценариев. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDirIterator>
#include <QtCore/QSettings>
#include <Common/QtHeadersEnd.h>

// Проект
#include "JSScenario.h"
#include "ScenarioEngine.h"

namespace GUI {

//---------------------------------------------------------------------------
namespace CScenarioEngine
{
	/// Название лога.
	const char LogName[] = "Interface";

	/// Маска поиска файлов сценариев.
	const char ScenarioFileMask[] = "*.ini";

	/// Название раздела с названием сценария в файле.
	const char ScenarioGroup[] = "scenario";

	/// Ключи конфигурационного файла.
	const char NameKey[]   = "name";   /// Название сценария/состояния.
	const char ScriptKey[] = "script"; /// Пусть к скрипту.
	const char BaseScenarioKey[] = "base"; /// Базовый сценарий.
}

//---------------------------------------------------------------------------
ScenarioEngine::ScenarioEngine()
	: ILogable(CScenarioEngine::LogName), mLogPadding(0)
{
}

//---------------------------------------------------------------------------
ScenarioEngine::~ScenarioEngine()
{
}

//---------------------------------------------------------------------------
void ScenarioEngine::addDirectory(const QString & aDirectory)
{
	// Ищем в данном каталоге сценарии рекурсивно
	QDirIterator dirEntry(aDirectory, QStringList(CScenarioEngine::ScenarioFileMask), QDir::Files, QDirIterator::Subdirectories);

	QMap<QString, QPair<QString, QString> > scenarios;

	while (dirEntry.hasNext())
	{
		dirEntry.next();

		QSettings file(dirEntry.fileInfo().absoluteFilePath(), QSettings::IniFormat);
		file.setIniCodec("UTF-8");

		// не путаем с другими файлами
		if (file.childGroups().contains(CScenarioEngine::ScenarioGroup))
		{
			file.beginGroup(CScenarioEngine::ScenarioGroup);
			scenarios[file.value(CScenarioEngine::NameKey).toString()] = qMakePair(
				dirEntry.fileInfo().absolutePath() + "/" + file.value(CScenarioEngine::ScriptKey).toString(),
				file.value(CScenarioEngine::BaseScenarioKey).toString());
		}
	}

	foreach (QString key, scenarios.keys())
	{
		QPair<QString, QString> params;
		QString basePath;

		if (!params.second.isEmpty())
		{
			if (scenarios.contains(params.second))
			{
				basePath = scenarios[params.second].first;
			}
			else
			{
				toLog(LogLevel::Error, QString("Failed to find base scenario for %1.").arg(key));
				continue;
			}
		}

		mScenarios.insert(key, SScenarioDescriptor(key, scenarios[key].first, basePath));
	}
}

//---------------------------------------------------------------------------
void ScenarioEngine::addScenario(Scenario * aScenario)
{
	connect(aScenario, SIGNAL(finished(const QVariantMap &)), SLOT(finished(const QVariantMap &)), Qt::UniqueConnection);

	// Устанавливаем лог, если он не был установлен отдельно.
	aScenario->setLog(getLog());

	mScenarios.insert(aScenario->getName(), SScenarioDescriptor(aScenario->getName(), aScenario->getName(), QString(), aScenario));
	mScenarioStorage.insertMulti(aScenario->getName(), QSharedPointer<Scenario>(aScenario));

	toLog(LogLevel::Normal, QString("Added scenario: %1.").arg(aScenario->getName()));
}

//---------------------------------------------------------------------------
bool ScenarioEngine::initialize() const
{
	return !mScenarioStorage.isEmpty();
}

//---------------------------------------------------------------------------
void ScenarioEngine::finalize()
{
	mScenarioStorage.clear();
	mScenarioStack.clear();
	mScenarios.clear();
	mScriptObjects.clear();
}

//---------------------------------------------------------------------------
void ScenarioEngine::resetTimeout()
{
	if (!mScenarioStack.isEmpty())
	{
		mScenarioStack.top()->resetTimeout();
	}
}

//---------------------------------------------------------------------------
void ScenarioEngine::signalTriggered(const QString & aSignal, const QVariantMap & aArguments)
{
	if (mScenarioStack.isEmpty())
	{
		toLog(LogLevel::Warning, QString("Cannot handle '%1' signal, no scenario is running."));
	}
	else
	{
		mScenarioStack.top()->signalTriggered(aSignal, aArguments);
	}
}

//---------------------------------------------------------------------------
bool ScenarioEngine::startScenario(const QString & aScenario, const QVariantMap & aParameters)
{
	TScenarioDescriptorMap::Iterator sc = mScenarios.find(aScenario);
	
	if (sc != mScenarios.end())
	{
		if (!mScenarioStack.isEmpty())
		{
			auto prevScenario = mScenarioStack.top();

			if (prevScenario->getName() == aScenario)
			{
				return true;
			}

			// Приостанавливаем текущий сценарий.
			toLog(LogLevel::Normal, QString("SUSPEND %1 scenario.").arg(prevScenario->getName()));
			prevScenario->pause();
		}

		Scenario * scenario;

		// В стеке уже есть такой запущенный сценарий или 
		// Сценарий еще не создан
		if (!sc->instance || qFind(mScenarioStack.begin(), mScenarioStack.end(), sc->instance) != mScenarioStack.end())
		{
			// Запускаем новый экземпляр сценария.
			scenario = new JSScenario(sc->name, sc->path, sc->basePath, getLog());

			mScenarioStorage.insertMulti(aScenario, QSharedPointer<Scenario>(scenario));

			if (scenario->initialize(mScriptObjects))
			{
				connect(scenario, SIGNAL(finished(const QVariantMap &)), SLOT(finished(const QVariantMap &)), Qt::UniqueConnection);
				scenario->setLog(getLog());

				sc->instance = scenario;
				mScenarios.insert(aScenario, *sc);

				foreach(QSharedPointer<Scenario> s, mScenarioStorage)
				{
					if (s->getStateHook().isEmpty())
					{
						continue;
					}

					QList<Scenario::SExternalStateHook> hooks;

					foreach(Scenario::SExternalStateHook hook, s->getStateHook())
					{
						if (hook.targetScenario == scenario->getName())
						{
							hooks << hook;
						}
					}

					scenario->setStateHook(hooks);
				}
			}
			else
			{
				toLog(LogLevel::Error, QString("Skipping invalid scenario %1.").arg(sc->name));
				return false;
			}
		}
		else
		{
			scenario = sc->instance;
		}

		mScenarioStack.push(scenario);
		emit scenarioChanged(aScenario);
		
		toLog(LogLevel::Normal, QString("START %1 scenario.").arg(aScenario));
		getLog()->adjustPadding(1);

		scenario->start(aParameters);
	}
	else
	{
		toLog(LogLevel::Error, QString("Failed to start '%1' scenario: not found").arg(aScenario));
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
void ScenarioEngine::finished(const QVariantMap & aResult)
{
	if (mScenarioStack.isEmpty())
	{
		toLog(LogLevel::Error, "Some scenario has finished but the stack is empty.");
		return;
	}

	getLog()->adjustPadding(-1);
	
	auto sourceScenario = dynamic_cast<GUI::Scenario *>(sender());
	if (sourceScenario && sourceScenario->getName() != mScenarioStack.top()->getName())
	{
		toLog(LogLevel::Warning, QString("Received a signal from the scenario '%1', but '%2' is active.")
			.arg(sourceScenario->getName()).arg(mScenarioStack.top()->getName()));
	}
	else
	{
		toLog(LogLevel::Normal, QString("STOP %1 scenario.").arg(mScenarioStack.top()->getName()));
		mScenarioStack.top()->stop();

		mScenarioStack.pop();

		if (!mScenarioStack.isEmpty())
		{
			toLog(LogLevel::Normal, QString("RESUME %1 scenario.").arg(mScenarioStack.top()->getName()));
			mScenarioStack.top()->resume(aResult);
			emit scenarioChanged(mScenarioStack.top()->getName());
		}
	}
}

//---------------------------------------------------------------------------
bool ScenarioEngine::canStop() const
{
	bool result = true;

	foreach(auto scenario, mScenarioStack)
	{
		if (!scenario->canStop())
		{
			result = false;
		}
	}

	return result;
}

//---------------------------------------------------------------------------

} // namespace GUI
