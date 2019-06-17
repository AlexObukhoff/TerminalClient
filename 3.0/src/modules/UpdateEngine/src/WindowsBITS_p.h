/* @file Обертка над подсистемой Windows BITS. Приватный класс. */

#pragma once

// Windows 
#include <Windows.h>
#include <atlbase.h>
#include <bits2_5.h>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSet>
#include <QtCore/QMap>
#include <QtCore/QThread>
#include <QtCore/QUrl>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILogable.h>

// Project
#include "WindowsBITS_i.h"


namespace CBITS
{


namespace AddTaskResult
{
	enum Enum
	{
		OK,
		Error,
		JobIsFull
	};
}


//---------------------------------------------------------------------------
class CopyManager_p : public ILogable
{
public:
	CopyManager_p(ILog * aLog);
	virtual ~CopyManager_p();

	/// Проверка работоспособности подсистемы
	bool isReady() const;

	/// Получить список существующих заданий на скачивание
	QMap<QString, SJob> getJobs();

	/// Создать задание
	bool createJob(const QString & aName, SJob & aJob, int aPriority);

	/// Сконфигурировать задачу для запуска приложения по окончанию скачивания
	bool setJobNotify(const QString & aApplicationPath, const QString & aParamaters);

	/// Добавить в задачу файл для скачивания
	AddTaskResult::Enum addTask(const QUrl & aUrl, const QString & aFileName);

	/// Открыть существующую задачу
	bool openJob(const SJob & aJob);

	/// Запустить текущую задачу в обработку
	bool resume();

	/// Сбросить текущую задачу
	bool cancel();

	/// Завершить обработку текущей задачи
	bool complete();

private:
	QString getJobError();

private:
	static QSet<Qt::HANDLE> mThreadInitialized;

private:
	CComPtr<IBackgroundCopyManager> mQueueMgr;
	CComPtr<IBackgroundCopyJob> mCurrentJob;
};


//---------------------------------------------------------------------------
} // namespace CBITS
