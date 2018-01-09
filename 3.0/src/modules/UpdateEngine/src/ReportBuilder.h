/* @file Генератор отчётов обновления. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QSettings>
#include <QtCore/QSharedPointer>
#include <QtCore/QDir>
#include <Common/QtHeadersEnd.h>

#include <Common/ILogable.h>

// SDK
#include <SDK/PaymentProcessor/Core/IRemoteService.h>

//------------------------------------------------------------------------
namespace CReportBuilder
{
	const QString LogName = "ReportBuilder";
}

//------------------------------------------------------------------------
class ReportBuilder : public QObject, protected ILogable
{
	Q_OBJECT

public:
	ReportBuilder(QString aWorkDirectory = "");

	/// Создаёт или открывает уже созданный отчёт для команды обновления aCommand.
	void open(const QString & aCommand, const QString & aUrl = QString(), const QString & aMd5 = QString());

	/// Закрыть отчёт
	void close();

	/// Устанавливает команде статус aStatus.
	void setStatus(SDK::PaymentProcessor::IRemoteService::EStatus aStatus);

	/// Устанавливает команде описание статуса aStatusMessage.
	void setStatusDescription(const QString & aStatusMessage);

public slots:
	/// Устанавливает прогресс выполнения команды.
	void setProgress(int aProgress);

private:
	void updateTimestamp();

private:
	QString mWorkDirectory;
	QSharedPointer<QSettings> mReport;
};

//------------------------------------------------------------------------
