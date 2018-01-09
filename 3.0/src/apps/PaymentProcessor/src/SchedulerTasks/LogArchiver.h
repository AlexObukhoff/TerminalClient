/* @file Реализация задачи архивации журнальных файлов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDir>
#include <QtCore/QDate>
#include <QtCore/QList>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ITask.h>

// Модули
#include <Common/ILogable.h>
#include <Packer/Packer.h>

//---------------------------------------------------------------------------
class LogArchiver : public QObject, public SDK::PaymentProcessor::ITask, public ILogable
{
	Q_OBJECT

public:
	LogArchiver(const QString & aName, const QString & aLogName, const QString & aParams);
	virtual ~LogArchiver();

	/// выполнить задачу
	virtual void execute();

	/// остановить выполнение задачи
	virtual bool cancel();

	/// подписаться на сигнал окончания задания
	virtual bool subscribeOnComplete(QObject * aReceiver, const char * aSlot);

private:
	bool mCanceled;
	int mMaxSize; // максимальный размер логов из настроек
	QDir mLogDir; // папка с логами
	QString mKernelPath; 
	Packer mPacker;

private:
	/// Получить список дат, логи которых подлежат упаковке
	QList<QDate> getDatesForPack() const;

	/// запаковать логи в архив
	void packLogs(QDate aDate);

	/// Удалить логи за данную дату
	void removeLogs(QDate aDate);

	/// Удаление старых архивов, в случае превышения размера архива
	void checkArchiveSize();

	QString logArchiveFileName(QDate aDate);

	bool removeFile(const QFileInfo & aFile);

signals:
	void finished(const QString & aName, bool aComplete);
};

