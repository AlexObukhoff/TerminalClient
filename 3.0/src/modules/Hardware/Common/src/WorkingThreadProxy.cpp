/* @file Прослойка для вызова функционала в рабочем потоке. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QThread>
#include <QtCore/QMetaType>
#include <Common/QtHeadersEnd.h>

// Project
#include "WorkingThreadProxy.h"

//-------------------------------------------------------------------------------
template int WorkingThreadProxy::invokeMethod<int>(std::function<int()> aMethod);
template double WorkingThreadProxy::invokeMethod<double>(std::function<double()> aMethod);
template bool WorkingThreadProxy::invokeMethod<bool>(std::function<bool()> aMethod);
template QString WorkingThreadProxy::invokeMethod<QString>(std::function<QString()> aMethod);

//-------------------------------------------------------------------------------
WorkingThreadProxy::WorkingThreadProxy(QThread * aWorkingThread): mWorkingThread(aWorkingThread)
{
	if (mWorkingThread)
	{
		if (!mWorkingThread->isRunning())
		{
			mWorkingThread->start();
		}

		moveToThread(mWorkingThread);
	}

	connect(this, SIGNAL(invoke(TVoidMethod)), SLOT(onInvoke(TVoidMethod)), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(invoke(TBoolMethod, bool *)), SLOT(onInvoke(TBoolMethod, bool *)), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(invoke(TIntMethod, int *)), SLOT(onInvoke(TIntMethod, int *)), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(invoke(TDoubleMethod, double *)), SLOT(onInvoke(TDoubleMethod, double *)), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(invoke(TStringMethod, QString *)), SLOT(onInvoke(TStringMethod, QString *)), Qt::BlockingQueuedConnection);
}

//--------------------------------------------------------------------------------
template <class T>
T WorkingThreadProxy::invokeMethod(std::function<T()> aMethod)
{
	T result;

	isWorkingThread() ? onInvoke(aMethod, &result) : emit invoke(aMethod, &result);

	return result;
}

//--------------------------------------------------------------------------------
template <>
void WorkingThreadProxy::invokeMethod(TVoidMethod aMethod)
{
	isWorkingThread() ? onInvoke(aMethod) : emit invoke(aMethod);
}

//--------------------------------------------------------------------------------
void WorkingThreadProxy::onInvoke(TVoidMethod aMethod)
{
	aMethod();
}

//--------------------------------------------------------------------------------
void WorkingThreadProxy::onInvoke(TBoolMethod aMethod, bool * aResult)
{
	*aResult = aMethod();
}

//--------------------------------------------------------------------------------
void WorkingThreadProxy::onInvoke(TIntMethod aMethod, int * aResult)
{
	*aResult = aMethod();
}

//--------------------------------------------------------------------------------
void WorkingThreadProxy::onInvoke(TDoubleMethod aMethod, double * aResult)
{
	*aResult = aMethod();
}

//--------------------------------------------------------------------------------
void WorkingThreadProxy::onInvoke(TStringMethod aMethod, QString * aResult)
{
	*aResult = aMethod();
}

//--------------------------------------------------------------------------------
bool WorkingThreadProxy::isWorkingThread()
{
	return !mWorkingThread || (mWorkingThread == QThread::currentThread());
}

//--------------------------------------------------------------------------------
