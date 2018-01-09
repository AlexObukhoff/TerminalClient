/* @file Базовый класс для сценариев. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVariantMap>
#include <QtCore/QTimer>
#include <Common/QtHeadersEnd.h>

#include <functional>

#include <Common/ILogable.h>

namespace GUI {

class ScenarioEngine;

//---------------------------------------------------------------------------
/// Описание апи объекта для сценария.
struct SScriptObject
{
	SScriptObject(const QString & aName, QObject * aObject): name(aName), isType(false), metaObject(0), object(aObject) {}
	SScriptObject(const QString & aName, const QMetaObject * aMetaObject): name(aName), isType(true), metaObject(aMetaObject), object(0) {}

	QString name;
	bool isType;
	const QMetaObject * metaObject;
	QObject * object;
};

//---------------------------------------------------------------------------
/// Базовый класс сценария.
class Scenario : public QObject, protected ILogable
{
	Q_OBJECT

public:
	Scenario(const QString & aName, ILog * aLog = 0);
	virtual ~Scenario();

	/// Запуск сценария.
	virtual void start(const QVariantMap & aContext) = 0;

	/// Остановка сценария.
	virtual void stop() = 0;

	/// Приостановить сценарий с возможностью последующего возобновления.
	virtual void pause() = 0;

	/// Продолжение после паузы.
	virtual void resume(const QVariantMap & aContext) = 0;

	/// Инициализация сценария.
	virtual bool initialize(const QList<SScriptObject> & aScriptObjects) = 0;

	struct SExternalStateHook
	{
		typedef std::function<QVariantMap(const QVariantMap &, const QVariantMap &)> THookFunction;
		
		QString targetScenario;
		QString targetState;
		THookFunction hook; //scenario context, state arguments

		SExternalStateHook() {}
		SExternalStateHook(const QString & aTargetScenario, const QString & aTargetState, THookFunction aHook) :
			targetScenario(aTargetScenario), targetState(aTargetState), hook(aHook) {}

		void clear()
		{
			targetScenario.clear();
			targetState.clear();
			hook = nullptr;
		}
	};
	
	virtual void setStateHook(const QList<SExternalStateHook> & aHook) { Q_UNUSED(aHook) }

	virtual QList<SExternalStateHook> getStateHook() { return QList<SExternalStateHook>(); }

	/// Обработка сигнала из активного состояния с дополнительными аргументами.
	virtual void signalTriggered(const QString & aSignal, const QVariantMap & aArguments = QVariantMap()) = 0;

	/// Переустановка лога.
	void setLog(ILog * aLog);

	/// Возвращает false, если сценарий не может быть остановлен в текущий момент.
	virtual bool canStop() = 0;

public slots:
	/// Текущее состояние.
	virtual QString getState() const = 0;

	/// Имя сценария.
	QString getName() const;

	/// Сброс таймаута сценария.
	virtual void resetTimeout();

	/// Установка таймаута.
	virtual void setStateTimeout(int aSec);

		/// Обработчик таймаута
	virtual void onTimeout() = 0;

signals:
	/// Вошли в состояние
	void enteredState(const QString & aScenario, const QString & aState, const QVariantMap & aArguments);

	/// Сценарий завершился.
	void finished(const QVariantMap & aResult);

protected:
	QString mName;        /// Название сценария.
	QString mPath;        /// Пусть к файлу сценария.
	int mDefaultTimeout;  /// Таймаут состояния по умолчанию.
	QTimer mTimeoutTimer; /// Таймер состояния.
};

} // namespace GUI

//---------------------------------------------------------------------------
