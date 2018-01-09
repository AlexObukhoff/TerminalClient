/* @file Мьютекс-локер с возможностью ограничения синхронизации определенного потока. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QMap>
#include <QtCore/QPair>
#include <Common/QtHeadersEnd.h>

//--------------------------------------------------------------------------------
class MutexLocker
{
	typedef QMap<QThread *, QThread *> TMatchedThreads;
	typedef QPair<QThread *, int> TLocksCounter;
	typedef QMap<QMutex *, TLocksCounter> TThreadsLocked;

public:
	MutexLocker(QMutex * aMutex);
	~MutexLocker();

	/// Замапить потока с ограниченной синхронизацией на вызвавший его поток.
	static void setMatchedThread(QThread * aOwner, QThread * aMatched);

	/// Удалить данные потоке с ограниченной синхронизацией.
	static void clearMatchedThread(QThread * aOwner);

private:
	/// Рабочий мьютекс.
	QMutex * mMutex;

	/// Таблица соответствия потока, вызвавшего локер, и замещающего его потока с ограниченной синхронизацией.
	static TMatchedThreads mMatchedThreads;

	/// Потоки, залочившие мьютексы + количество локов (для рекурсивных мьютексов).
	static TThreadsLocked mThreadsLocked;

	// Мьютекс для защиты ресурсов.
	static QMutex mResourceMutex;
};

//--------------------------------------------------------------------------------
