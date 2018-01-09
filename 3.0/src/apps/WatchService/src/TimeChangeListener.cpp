/* @file Класс, отслеживающий активность пользователя. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QThread>
#include <QtCore/QMutexLocker>
#include <QtCore/QDateTime>
#include <QtCore/QMetaObject>
#include <QtCore/QTimer>
#include <Common/QtHeadersEnd.h>

// Project
#include "TimeChangeListener.h"

//------------------------------------------------------------------------
TimeChangeListener * gListener;

namespace CTimeChangeListener
{
	/// время периода проверки изменения времени
	const int CheckTimerTimeout = 100; // ms 
}

#ifdef Q_OS_WIN
HHOOK  TimeChangeListener::mHook;
QMutex TimeChangeListener::mHookMutex;
#endif

//------------------------------------------------------------------------
TimeChangeListener::TimeChangeListener(QObject * aParent) : QObject(aParent), mTimeOffset(0)
{
	gListener = this;

#ifdef Q_OS_WIN
	mHook = ::SetWindowsHookEx(WH_GETMESSAGE, &TimeChangeListener::MsgProc, 0, ::GetCurrentThreadId());
#endif

	startTimer(CTimeChangeListener::CheckTimerTimeout);
}

//------------------------------------------------------------------------
TimeChangeListener::~TimeChangeListener()
{
#ifdef Q_OS_WIN
	::UnhookWindowsHookEx(mHook);
#endif
}

//------------------------------------------------------------------------
void TimeChangeListener::timerEvent(QTimerEvent * aEvent)
{
	QMutexLocker locker(&mHookMutex);

#ifndef Q_OS_WIN
	mLastCheckTime = checkTimeOffset();
#else 
	mLastCheckTime = QDateTime::currentDateTime();
#endif
}

//------------------------------------------------------------------------
QDateTime TimeChangeListener::checkTimeOffset()
{
	QMutexLocker locker(&mHookMutex);

	QDateTime currentTime = QDateTime::currentDateTime();

	if (mTimeOffset == 0)
	{
		qint64 offset = mLastCheckTime.msecsTo(currentTime);

		if (qAbs(offset) > CTimeChangeListener::CheckTimerTimeout)
		{
			// время поменяли!
			mTimeOffset = offset;

			QMetaObject::invokeMethod(this, "emitTimeChanged", Qt::QueuedConnection);
		}
	}

	return currentTime;
}

//------------------------------------------------------------------------
void TimeChangeListener::emitTimeChanged()
{
	if (mTimeOffset)
	{
		QMutexLocker locker(&mHookMutex);

		emit timeChanged(mTimeOffset);

		QTimer::singleShot(2000, this, SLOT(cleanTimeOffset()));
	}
}

//------------------------------------------------------------------------
void TimeChangeListener::cleanTimeOffset()
{
	mTimeOffset = 0;
}

#ifdef Q_OS_WIN
//------------------------------------------------------------------------
LRESULT CALLBACK TimeChangeListener::MsgProc(int aCode, WPARAM aWParam, LPARAM aLParam)
{
	if (aCode == HC_ACTION && ((MSG*)aLParam)->message == WM_TIMECHANGE)
	{
		gListener->checkTimeOffset();
	}

	return ::CallNextHookEx(mHook, aCode, aWParam, aLParam);
}
#endif // Q_OS_WIN

