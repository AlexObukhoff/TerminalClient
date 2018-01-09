/* @file Сценарий на основе javascript. */

#include <Common/QtHeadersBegin.h>
#include <QtCore/QFinalState>
#include <QtCore/QAbstractTransition>
#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <Common/QtHeadersEnd.h>

#include "JSScenario.h"

namespace GUI {

//---------------------------------------------------------------------------
namespace CJSScenario
{
	/// Название сервиса для Core API.
	const char ServiceName[] = "ScenarioEngine";

	/// Функция импорта другого файла из скрипта.
	const char ScriptIncludeFunction[] = "include";

	/// Javascript-функции сценария.
	const char ScriptInitFunction[]     = "initialize";   /// Инициализация.
	const char ScriptStartFunction[]    = "onStart";      /// Запуск сценария.
	const char ScriptStopFunction[]     = "onStop";       /// Завершение.
	const char ScriptTimeoutFunction[]  = "onTimeout";    /// Таймаут.
	const char ScriptPauseFunction[]    = "onPause";      /// Прерывание выполнения сценария.
	const char ScriptResumeFunction[]   = "onResume";     /// Возобновление прерванного сценария.
	const char ScriptFinalHandler[]     = "Handler";      /// Суффикс обработчика конечного состояния.
	const char ScriptEnterHandler[]     = "EnterHandler"; /// Суффикс обработчика входа в состояние.
	const char ScriptExitHandler[]      = "ExitHandler";  /// Суффикс обработчика выхода в состояние.
	const char ScriptCanStopFunction[]  = "canStop";      /// Возвращает false, если сценарий не может быть остановлен в текущий момент.

	/// Параметры состояний.
	const char ParamInitial[]      = "initial";            /// Начальное состояние.
	const char ParamFinal[]        = "final";              /// Конечное состояние.
	const char ParamTimeout[]      = "timeout";            /// Таймаут состояния.
	const char ParamUserActivity[] = "ignoreUserActivity"; /// Игнорировать активность пользователя.
	const char ParamSignalName[]   = "signal";             /// Имя сигнала, при которому мы поишли в данное состояние.
	const char ParamResult[]       = "result";             /// Результат работы сценария.
	const char ParamResultError[]  = "resultError";        /// Ошибка, возвращаемая сценарием.
}

//---------------------------------------------------------------------------
/// Событие сценария.
class ScenarioEvent: public QEvent
{
public:
	/// Пользовательский тип.
	static const int Type = QEvent::User + 1;

	ScenarioEvent(const QString & aSignal): QEvent(QEvent::Type(Type)), mSignal(aSignal) {}
	QString getSignal() const { return mSignal; }

private:
	QString mSignal;
};

//---------------------------------------------------------------------------
/// Переход между состояниями сценария.
class ScenarioTransition: public QAbstractTransition
{
public:
	ScenarioTransition(const QString & aSignal): mSignal(aSignal) {}

	virtual void onTransition(QEvent *) {}
	virtual bool eventTest(QEvent * aEvent)
	{
		if (aEvent->type() == ScenarioEvent::Type)
		{
			ScenarioEvent * se = static_cast<ScenarioEvent *>(aEvent);
			return mSignal == se->getSignal();
		}
	
		return false;
	}

 private:
	 QString mSignal;
};

//---------------------------------------------------------------------------
JSScenario::JSScenario(const QString & aName, const QString & aPath, const QString & aBasePath, ILog * aLog)
	: Scenario(aName, aLog),
	mBasePath(aBasePath),
	mPath(aPath),
	mIsPaused(true)
{
	connect(&mEnterSignalMapper, SIGNAL(mapped(const QString &)), SLOT(onEnterState(const QString &)));
	connect(&mExitSignalMapper, SIGNAL(mapped(const QString &)), SLOT(onExitState(const QString &)));
}

//---------------------------------------------------------------------------
JSScenario::~JSScenario()
{
}

//---------------------------------------------------------------------------
void JSScenario::start(const QVariantMap & aContext)
{
	mContext = aContext;
	mCurrentState = mInitialState;

	mIsPaused = false;

	functionCall(CJSScenario::ScriptStartFunction, QVariantMap(), mName + ":" + CJSScenario::ScriptStartFunction);

	mSignalArguments.clear();
	mSignalArguments[CJSScenario::ParamSignalName] = "start";

	mStateMachine->setInitialState(mStates[mCurrentState].qstate);
	mStateMachine->start();
}

//---------------------------------------------------------------------------
void JSScenario::pause()
{
	mIsPaused = true;
	
	mTimeoutTimer.stop();
	mStateMachine->stop();

	functionCall(CJSScenario::ScriptPauseFunction, QVariantMap(), mName + ":" + CJSScenario::ScriptPauseFunction);
}

//---------------------------------------------------------------------------
void JSScenario::resume(const QVariantMap & aContext)
{
	// Очищаем контекст от совпадающих ключей
	foreach (QString key, aContext.keys())
	{
		mContext.remove(key);
	}
	
	mContext.unite(aContext);

	mIsPaused = false;

	functionCall(CJSScenario::ScriptResumeFunction, QVariantMap(), mName + ":" + CJSScenario::ScriptResumeFunction);

	mSignalArguments.clear();
	mSignalArguments[CJSScenario::ParamSignalName] = "resume";

	mStateMachine->setInitialState(mStates[mCurrentState].qstate);
	mStateMachine->start();
}

//---------------------------------------------------------------------------
bool JSScenario::initialize(const QList<SScriptObject> & aScriptObjects)
{
	mStateMachine = QSharedPointer<QStateMachine>(new QStateMachine);
	mScriptEngine = QSharedPointer<QScriptEngine>(new QScriptEngine);

	connect(mStateMachine.data(), SIGNAL(finished()), this, SLOT(onFinish()));
	connect(mScriptEngine.data(), SIGNAL(signalHandlerException(const QScriptValue &)), this, SLOT(onException(const QScriptValue &)));

	// Добавляем в скрипты внешние объекты.
	foreach (const SScriptObject & object, aScriptObjects)
	{
		// Движок сохраняет указатель на object.metaObject, временные переменные не передавать!
		mScriptEngine->globalObject().setProperty(object.name,
			object.isType ? mScriptEngine->newQMetaObject(object.metaObject) : mScriptEngine->newQObject(object.object));
	}

	mScriptEngine->globalObject().setProperty(CJSScenario::ServiceName, mScriptEngine->newQObject(this));
	mScriptEngine->globalObject().setProperty(CJSScenario::ScriptIncludeFunction, mScriptEngine->newFunction(&JSScenario::includeScript, this));

	mScriptEngine->installTranslatorFunctions();

	// Загружаем базовый сценарий, если такой имеется.
	if (!mBasePath.isEmpty())
	{
		if (!loadScript(mScriptEngine.data(), mName, mBasePath))
		{
			toLog(LogLevel::Error, QString("Failed to load base scenario script %1 for scenario %2.").arg(mBasePath).arg(mName));
			return false;
		}

		// Делаем вызов функции initialize в скрипте.
		functionCall(CJSScenario::ScriptInitFunction, QVariantMap(), mName + ":" + CJSScenario::ScriptInitFunction);
	}

	if (!loadScript(mScriptEngine.data(), mName, mPath))
	{
		return false;
	}

	// Инициализируем скрипт сценария.
	QScriptValue result = functionCall(CJSScenario::ScriptInitFunction, QVariantMap(),
		QString("%1:%2").arg(mName).arg(CJSScenario::ScriptInitFunction));

	if (result.isError())
	{
		toLog(LogLevel::Error, QString("Failed to initialize '%1' scenario: %2").arg(mName).arg(result.toString()));
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
bool JSScenario::canStop()
{
	QScriptValue result = functionCall(CJSScenario::ScriptCanStopFunction, QVariantMap(),
		QString("%1:%2").arg(mName).arg(CJSScenario::ScriptCanStopFunction));

	if (result.isError())
	{
		toLog(LogLevel::Error, QString("Failed to call '%1' scenario: %2").arg(mName).arg(result.toString()));
		return true;
	}

	return result.toBool();
}

//---------------------------------------------------------------------------
void JSScenario::signalTriggered(const QString & aSignal, const QVariantMap & aArguments)
{
	// Если переход не зарегистрирован, не обрабатываем его.
	if (!mTransitions.contains(mCurrentState, aSignal))
	{
		toLog(LogLevel::Debug, QString("Transition from state '%1' by signal '%2' not found.")
			.arg(mCurrentState)
			.arg(aSignal));
		
		return;
	}

	// Стейт-машина была активирована, но еще не вошла в рабочее состояние (срабатывание приведет к утечке памяти).
	if (!mStateMachine->isRunning())
	{
		return;
	}

	// Переход не был выполнен.
	if (!mSignalArguments.isEmpty())
	{
		toLog(LogLevel::Debug, QString("Transition from state '%1' was not completed but new signal '%2' was happened.")
			.arg(mCurrentState)
			.arg(aSignal));

		return;
	}

	mSignalArguments = aArguments;
	mSignalArguments[CJSScenario::ParamSignalName] = aSignal;

	mStateMachine->postEvent(new ScenarioEvent(aSignal));
}

//---------------------------------------------------------------------------
void JSScenario::addState(const QString & aStateName, const QVariantMap & aParameters)
{
	Q_ASSERT(!aStateName.isEmpty());

	SState state;
	state.name = aStateName;
	state.parameters = aParameters;

	if (state.name.isEmpty())
	{
		toLog(LogLevel::Error, QString("Failed to add a state to '%1' scenario: name not specified.").arg(mName));
		return;
	}

	// Начальное, конечное или обычное состояние.
	if (aParameters.contains(CJSScenario::ParamFinal))
	{
		state.qstate = new QFinalState();
	}
	else
	{
		if (aParameters.contains(CJSScenario::ParamInitial))
		{
			mInitialState = state.name;
		}

		state.qstate = new QState();
	}

	mEnterSignalMapper.connect(state.qstate, SIGNAL(entered()), SLOT(map()));
	mEnterSignalMapper.setMapping(state.qstate, aStateName);

	mExitSignalMapper.connect(state.qstate, SIGNAL(exited()), SLOT(map()));
	mExitSignalMapper.setMapping(state.qstate, aStateName);

	mStates[aStateName] = state;
	mStateMachine->addState(state.qstate);
}

//---------------------------------------------------------------------------
void JSScenario::addTransition(const QString & aSource, const QString & aTarget, const QString & aSignal)
{
	Q_ASSERT(!aSignal.isEmpty());

	TStateList::iterator src = mStates.find(aSource);
	TStateList::iterator dst = mStates.find(aTarget);

	if (src == mStates.end() || dst == mStates.end())
	{
		toLog(LogLevel::Error, QString("Failed to add '%1->%2' transition to '%3' scenario: either source or target state doesn't exist.")
			.arg(aSource).arg(aTarget).arg(mName));
		return;
	}

	QScopedPointer<ScenarioTransition> transition(new ScenarioTransition(aSignal));
	transition->setTargetState(dst->qstate);
	
	QState * state = dynamic_cast<QState *>(src->qstate);
	if (!state)
	{
		toLog(LogLevel::Error, QString("Failed to add '%1->%2' transition to '%3' scenario: source state cannot have transitions.")
				.arg(aSource).arg(aTarget).arg(mName));
		return;
	}
	
	state->addTransition(transition.take());
	mTransitions.insert(aSource, aSignal);
}

//---------------------------------------------------------------------------
void JSScenario::setDefaultTimeout(int aSeconds, const QScriptValue & aHandler)
{
	mDefaultTimeout = aSeconds;
	mTimeoutHandler = aHandler;
}

//---------------------------------------------------------------------------
void JSScenario::onEnterState(const QString & aState)
{
	if (mIsPaused)
	{
		toLog(LogLevel::Warning, QString("Scenario %1 in da pause. Skip state %2.").arg(mName).arg(aState));
		return;
	}
	
	TStateList::iterator s = mStates.find(aState);
	bool final = s->parameters.contains(CJSScenario::ParamFinal);

	toLog(LogLevel::Normal, QString("ENTER %1 %2state.").arg(aState).arg(final ? "final " : ""));

	QVariantMap::iterator t = s->parameters.find("timeout");
	int timeout = mDefaultTimeout;

	if (t != s->parameters.end())
	{
		timeout = t->toInt();
	}
	
	if (timeout > 0)
	{
		setStateTimeout(timeout);
	}

	mCurrentState = aState;

	// Если достигли конечного состояния - копируем результат.
	if (final)
	{
		mContext[CJSScenario::ParamResult] = s->parameters[CJSScenario::ParamResult];
	}

	foreach(Scenario::SExternalStateHook hook, mHooks)
	{
		if (hook.targetScenario == getName() && hook.targetState == mCurrentState)
		{
			mSignalArguments = hook.hook(getContext(), mSignalArguments);
		}
	}

	QString handler = aState + (final ? CJSScenario::ScriptFinalHandler : CJSScenario::ScriptEnterHandler);
	functionCall(handler, mSignalArguments, QString("%1:%2:%3").arg(mName).arg(aState).arg(handler));

	mSignalArguments.clear();
}

//---------------------------------------------------------------------------
void JSScenario::onExitState(const QString & aState)
{
	if (mIsPaused)
	{
		toLog(LogLevel::Warning, QString("Scenario %1 in da pause. Skip state %2.").arg(mName).arg(aState));
		return;
	}
	
	QString handler = aState + CJSScenario::ScriptExitHandler;
	functionCall(handler, mSignalArguments, QString("%1:%2:%3").arg(mName).arg(aState).arg(handler));

	mTimeoutTimer.stop();
}

//---------------------------------------------------------------------------
void JSScenario::onTimeout()
{
	bool handled = false;

	if (mTimeoutHandler.isFunction())
	{
		QScriptValue result = mTimeoutHandler.call(QScriptValue(), QScriptValueList() << mCurrentState);
		if (result.isError())
		{
			toLog(LogLevel::Error, QString("An exception occured during executing '%1' timeout handler: %2.").arg(mName).arg(result.toString()));
		} 
		else
		{
			handled = result.toBool();
		}
	}

	if (!handled) 
	{
		mSignalArguments.clear();
		signalTriggered("timeout");
	}

	resetTimeout();
}

//---------------------------------------------------------------------------
void JSScenario::onFinish()
{
	mTimeoutTimer.stop();

	QScriptValue resultError = functionCall(CJSScenario::ScriptStopFunction, QVariantMap(),
		QString("%1:%2").arg(mName).arg(CJSScenario::ScriptStopFunction));

	// Помещаем в контекст возвращаемое сценарием значение
	mContext[CJSScenario::ParamResultError] = resultError.toVariant();

	emit finished(mContext);
}

//---------------------------------------------------------------------------
QString JSScenario::getState() const
{
	return mCurrentState;
}

//---------------------------------------------------------------------------
QVariantMap JSScenario::getContext() const
{
	return mContext;
}

//---------------------------------------------------------------------------
QScriptValue JSScenario::functionCall(const QString & aFunction, const QVariantMap & aArguments, const QString & aNameForLog)
{
	QScriptValue function = mScriptEngine->globalObject().property(aFunction);

	QScriptValue result;

	if (function.isValid())
	{
		toLog(LogLevel::Normal, QString("CALL %1 function.").arg(aNameForLog));

		if (aArguments.isEmpty())
		{
			result = function.call();
		}
		else
		{
			QScriptValueList arguments;
			arguments << function.engine()->toScriptValue<QVariantMap>(aArguments);

			result = function.call(QScriptValue(), arguments);
		}

		if (function.engine()->hasUncaughtException())
		{
			toLog(LogLevel::Error, QString("An exception occured while calling %1(line %2): %3\nBacktrace:\n%4.")
				.arg(aNameForLog)
				.arg(function.engine()->uncaughtExceptionLineNumber())
				.arg(function.engine()->uncaughtException().toString())
				.arg(function.engine()->uncaughtExceptionBacktrace().join("\n")));
		}
		else if (!result.isValid())
		{
			toLog(LogLevel::Error, QString("%1 is not a function.").arg(aNameForLog));
		}
	}
	else 
	{
		toLog(LogLevel::Debug, QString("Failed to call %1, not a valid script object.").arg(aNameForLog));
	}

	return result;
}

//---------------------------------------------------------------------------
bool JSScenario::loadScript(QScriptEngine * aScriptEngine, const QString & aScenarioName, const QString & aScriptPath)
{
	// Загружаем скрипт сценария.
	QFile script(aScriptPath);
	if (!script.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		toLog(LogLevel::Error, QString("Failed to open '%1' scenario script %2: %3").arg(aScenarioName).arg(aScriptPath).arg(script.errorString()));
		return false;
	}

	QTextStream stream(&script);
	QString program = stream.readAll();

	// Проверка синтаксиса скрипта.
	QScriptSyntaxCheckResult syntax = QScriptEngine::checkSyntax(program);
	if (syntax.state() == QScriptSyntaxCheckResult::Error)
	{
		toLog(LogLevel::Error, QString("Failed to execute '%1'. Syntax error at line %2 column %3. %4.").arg(aScriptPath).arg(syntax.errorLineNumber()).
			arg(syntax.errorColumnNumber()).arg(syntax.errorMessage()));

		return false;
	}

	QScriptValue result = aScriptEngine->evaluate(program);
	if (result.isError())
	{
		toLog(LogLevel::Error, QString("Failed to execute '%1' scenario script: %2").arg(aScenarioName).arg(result.toString()));
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
QScriptValue JSScenario::includeScript(QScriptContext * aContext, QScriptEngine * aEngine, void * aScenario)
{
	JSScenario * self = static_cast<JSScenario *>(aScenario);

	QString includeFilePath = self->mPath.section("/", 0, -2) + "/" + aContext->argument(0).toString();
	QString namespaceName = aContext->argument(1).toString();

	if (!namespaceName.isEmpty())
	{
		aEngine->pushContext();

		// Добавляем другой файл в текущий контекст.
		if (!self->loadScript(aEngine, self->mName, includeFilePath))
		{
			aEngine->popContext();
			return QScriptValue(false);
		}

		// Сохраняем контекст активации.
		QScriptValue vars = aEngine->currentContext()->activationObject();
		aEngine->popContext();

		// И добавляем его под другим именем.
		aEngine->globalObject().setProperty(namespaceName, vars);

		return QScriptValue(true);
	}
	else
	{
		// Устанавливаем объект активации от внешнего контекста.
		aContext->setActivationObject(aContext->parentContext()->activationObject());

		// Добавляем другой скрипт в текущий контекст.
		if (self->loadScript(aEngine, self->mName, includeFilePath))
		{
			return QScriptValue(true);
		}

		return QScriptValue(false);
	}
}

//---------------------------------------------------------------------------
void JSScenario::onException(const QScriptValue & aException)
{
	toLog(LogLevel::Error, QString("An exception occured in scenario %1: %2").arg(getName()).arg(aException.toString()));
}

} // namespace GUI

//---------------------------------------------------------------------------
