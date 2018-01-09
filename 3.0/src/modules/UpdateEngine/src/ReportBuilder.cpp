/* @file Генератор отчётов обновления. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFileInfo>
#include <QtCore/QDateTime>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/Application.h>

// Project
#include "ReportBuilder.h"

//------------------------------------------------------------------------
ReportBuilder::ReportBuilder(QString aWorkDirectory /*= ""*/) :
	ILogable(CReportBuilder::LogName),
	mWorkDirectory(aWorkDirectory)
{
}

//------------------------------------------------------------------------
void ReportBuilder::open(const QString & aCommand, const QString & aUrl, const QString & aMd5)
{
	QString fileName = (mWorkDirectory.isEmpty() ? QDir::currentPath() : mWorkDirectory)  + "/update/" + QString("update_%1.rpt").arg(aCommand);

	bool writeCreateDate = QFile::exists(fileName) ? false : true;

	QFileInfo fileInfo(fileName);

	QDir dir;
	dir.mkpath(fileInfo.absolutePath());

	toLog(LogLevel::Normal, QString("Opening report file %1.").arg(fileName));

	mReport = QSharedPointer<QSettings>(new QSettings(fileName, QSettings::IniFormat));
	
	mReport->setValue("id", aCommand);
	mReport->setValue("url", aUrl);
	mReport->setValue("md5", aMd5);

	if (writeCreateDate)
	{
		mReport->setValue("create_date", QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
	}

	updateTimestamp();
}

//------------------------------------------------------------------------
void ReportBuilder::close()
{
	mReport.clear();
}

//------------------------------------------------------------------------
void ReportBuilder::setStatus(SDK::PaymentProcessor::IRemoteService::EStatus aStatus)
{
	if (mReport)
	{
		mReport->setValue("status", aStatus);
		updateTimestamp();
	}
}

//------------------------------------------------------------------------
void ReportBuilder::setStatusDescription(const QString & aStatusMessage)
{	
	if (mReport)
	{
		mReport->setValue("status_desc", aStatusMessage);
		updateTimestamp();
	}
}

//------------------------------------------------------------------------
void ReportBuilder::setProgress(int aProgress)
{
	if (mReport)
	{
		mReport->setValue("progress", aProgress);
		updateTimestamp();
	}
}

//------------------------------------------------------------------------
void ReportBuilder::updateTimestamp()
{
	if (mReport)
	{
		mReport->setValue("last_update", QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
		mReport->sync();
	}
}

//------------------------------------------------------------------------
