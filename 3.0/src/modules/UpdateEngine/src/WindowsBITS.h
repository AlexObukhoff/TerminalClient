/* @file Обертка над подсистемой Windows BITS. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QSharedPointer>
#include <QtCore/QUrl>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILogable.h>

// Project
#include "WindowsBITS_i.h"


namespace CBITS
{

//---------------------------------------------------------------------------
class CopyManager_p;

//---------------------------------------------------------------------------
class CopyManager : public ILogable
{
public:
	CopyManager(ILog * aLog);
	virtual ~CopyManager();

	/// Прекратить мучения BITS
	void shutdown();

	/// Проверка работоспособности подсистемы
	bool isReady() const;

	/// Получить список существующих заданий на скачивание
	QMap<QString, SJob> getJobs(const QString & aFilter);

	/// Создать задание
	bool createJob(const QString & aName, SJob & aJob, int aPriority);

	/// Сконфигурировать задачу для запуска приложения по окончанию скачивания
	bool setNotify(const QString & aApplicationPath, const QString & aParamaters);

	/// Добавить в задачу файл для скачивания
	bool addTask(const QUrl & aUrl, const QString & aFileName);

	/// Открыть существующую задачу
	bool openJob(const SJob & aJob);

	/// Запустить текущую задачу в обработку
	bool resume();

	/// Сбросить текущую задачу
	bool cancel();

	/// Завершить обработку текущей задачи
	bool complete();

private:
	bool internalResume();
	QString makeJobName(const QString & aName = QString());

private:
	int mJobsCount;
	int mPriority;
	QString mJobName;
	QString mNotifyApplication;
	QString mNotifyParameters;
	QSharedPointer<CopyManager_p> mCopyManager;
};

//---------------------------------------------------------------------------
} // namespace CBITS
