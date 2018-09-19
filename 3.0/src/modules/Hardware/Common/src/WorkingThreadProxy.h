/* @file Прослойка для вызова функционала в рабочем потоке. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <Common/QtHeadersEnd.h>

// Project
#include "Hardware/Common/FunctionTypes.h"

//--------------------------------------------------------------------------------
class WorkingThreadProxy : public QObject
{
	Q_OBJECT

public:
	WorkingThreadProxy(QThread * aWorkingThread);

	/// Вызывает метод в рабочем потоке и возвращает результат.
	template <class T>
	T invokeMethod(std::function<T()> aMethod);

signals:
	/// Сигналы для обработки указанного метода в нити объекта.
	void invoke(TVoidMethod aMethod);
	void invoke(TBoolMethod aMethod, bool * aResult);
	void invoke(TIntMethod aMethod, int * aResult);
	void invoke(TDoubleMethod aMethod, double * aResult);
	void invoke(TStringMethod aMethod, QString * aResult);

protected slots:
	/// Обработчики сигнала invoke.
	void onInvoke(TVoidMethod aMethod);
	void onInvoke(TBoolMethod aMethod, bool * aResult);
	void onInvoke(TIntMethod aMethod, int * aResult);
	void onInvoke(TDoubleMethod aMethod, double * aResult);
	void onInvoke(TStringMethod aMethod, QString * aResult);

	void checkThreadStarted();

protected:
	WorkingThreadProxy() {}

	/// Из рабочего ли потока происходит вызов.
	bool isWorkingThread();

	/// Поток, который будет считаться рабочим.
	QThread * mWorkingThread;

	/// Мьютекс для операций при старте потока.
	QMutex mStartMutex;

	/// Wait-condition для операций при старте потока.
	QWaitCondition mStartCondition;
};

//---------------------------------------------------------------------------
