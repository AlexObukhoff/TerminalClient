/* @file Класс, отслеживающий смену системного времени. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QDateTime>
#include <QtCore/QMutex>
#include <Common/QtHeadersEnd.h>

#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
#endif

//------------------------------------------------------------------------
class TimeChangeListener : public QObject
{
	Q_OBJECT

public:
	TimeChangeListener(QObject * aParent);
	virtual ~TimeChangeListener();

signals:
	/// сигнал об изменении времени (смещение в мс. примерное)
	void timeChanged(qint64 aOffset);

protected:
	void timerEvent(QTimerEvent * aEvent);

	/// Проверка и попытка примерного вычисления смещения нового времени.
	QDateTime checkTimeOffset();

protected slots:
	/// слот для развязывания хука с signal\slot Qt
	void emitTimeChanged();

	/// очистка смещения времени
	void cleanTimeOffset();

#ifdef Q_OS_WIN
protected:
	static LRESULT CALLBACK MsgProc(int aCode, WPARAM aWParam, LPARAM aLParam);
	
private:
	static HHOOK  mHook;
	static QMutex mHookMutex;
#endif // Q_OS_WIN

private:
	QDateTime mLastCheckTime;
	qint64 mTimeOffset;
};

//------------------------------------------------------------------------
