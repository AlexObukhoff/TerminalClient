/* @file Реализация задачи синхронизации системного времени терминала. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QElapsedTimer>
#include <QtNetwork/QHostInfo>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Core/INetworkService.h>

// Модули
#include <System/IApplication.h>
#include <Common/Application.h>
#include <SysUtils/ISysUtils.h>
#include <NetworkTaskManager/NetworkTaskManager.h>
#include <NetworkTaskManager/MemoryDataStream.h>
#include <NetworkTaskManager/NetworkTask.h>

// Thirdparty
#include "qntp/NtpClient.h"
#include "qntp/NtpReply.h"

// Проект
#include "Services/TerminalService.h"
#include "TimeSync.h"

namespace PPSDK = SDK::PaymentProcessor;

namespace CTimeSync
{
	const int DesyncLimit = 60;  /// Максимальный предел рассинхронизации времени с сервером в секундах.
	const int NtpPort     = 123;
	const int NtpTimeout  = 10;  /// Время в секундах, сколько мы ждем ответа NTP сервера
	const int HttpTimeout = 10;  /// Таймаут запроса времени по http.

	const QString TimeFormat = "yyyy.MM.dd hh:mm:ss";
}

//---------------------------------------------------------------------------
TimeSync::TimeSync(const QString & aName, const QString & aLogName, const QString & aParams) :
	ITask(aName, aLogName, aParams),
	ILogable(aLogName),
	mNetwork(nullptr),
	mTimeReceived(false)
{
	mClient = new NtpClient(this);

	connect (mClient, SIGNAL(replyReceived(const QHostAddress &, quint16, const NtpReply &)), 
		this, SLOT(ntpReplyReceived(const QHostAddress &, quint16, const NtpReply &)), Qt::DirectConnection);

	foreach (auto param, aParams.split(",", QString::SkipEmptyParts))
	{
		mTimeSyncHosts << QUrl(QString("ntp://%1").arg(param.trimmed()));
	}

	IApplication * app = dynamic_cast<IApplication *>(BasicApplication::getInstance());

	PPSDK::ICore * core = app->getCore();
	PPSDK::TerminalSettings * terminalSettings = static_cast<PPSDK::TerminalSettings *>(core->getSettingsService()->
		getAdapter(PPSDK::CAdapterNames::TerminalAdapter));

	if (core->getNetworkService())
	{
		mNetwork = core->getNetworkService()->getNetworkTaskManager();
	}

	if (terminalSettings->isValid())
	{
		foreach (auto host, terminalSettings->getCheckHosts())
		{
			mTimeSyncHosts << host.first;
		}
	}
}

//---------------------------------------------------------------------------
void TimeSync::checkNextUrl()
{
	if (mTimeReceived || mCanceled)
	{
		emit finished(mName, mTimeReceived);

		return;
	}
}

//---------------------------------------------------------------------------
void TimeSync::execute()
{
	if (mTimeSyncHosts.isEmpty())
	{
		emit finished(mName, false);

		return;
	}

	// запускаем запросы ко всем серверам сразу
	QSet<QUrl> ntpUrls;

	foreach (auto url, mTimeSyncHosts)
	{
		if (url.scheme() == "ntp")
		{
			ntpUrls << url;

			ntpCheckMethod(url);
		}
		else
		{
			httpCheckMethod(url);
		}
	}

	mTimeSyncHosts.subtract(ntpUrls);

	if (!ntpUrls.isEmpty())
	{
		QTimer::singleShot(CTimeSync::NtpTimeout * 1000, this, SLOT(ntpRequestTimeout()));
	}
}

//---------------------------------------------------------------------------
bool TimeSync::cancel()
{
	emit finished(mName, mTimeReceived);

	return (mCanceled = true);
}

//---------------------------------------------------------------------------
bool TimeSync::subscribeOnComplete(QObject * aReceiver, const char * aSlot)
{
	return connect(this, SIGNAL(finished(const QString &, bool)), aReceiver, aSlot);
}

//---------------------------------------------------------------------------
void TimeSync::ntpCheckMethod(const QUrl & aHost)
{
	toLog(LogLevel::Normal, QString("Get time from %1...").arg(aHost.toString()));

	QHostInfo hostInfo = QHostInfo::fromName(aHost.host());

	if (hostInfo.addresses().isEmpty())
	{
		toLog(LogLevel::Error, QString("Error IP lookup for '%1'. Skip this server.").arg(aHost.host()));
	}
	else
	{
		if (!mClient->sendRequest(hostInfo.addresses().first(), CTimeSync::NtpPort))
		{
			toLog(LogLevel::Error, QString("Failed send NTP sequest to '%1'.").arg(aHost.host()));
		}
	}
}

//---------------------------------------------------------------------------
void TimeSync::ntpRequestTimeout()
{
	if (mTimeSyncHosts.isEmpty() || mTimeReceived)
	{
		emit finished(mName, mTimeReceived);

		return;
	}
}

//---------------------------------------------------------------------------
void TimeSync::ntpReplyReceived(const QHostAddress & aAddress, quint16 aPort, const NtpReply & aReply)
{
	Q_UNUSED(aAddress)
	Q_UNUSED(aPort)

	if (!aReply.isNull())
	{
		toLog(LogLevel::Normal, QString("Receive NTP reply: server time: %1, local clock offset: %2 ms")
			.arg(aReply.transmitTime().toLocalTime().toString(CTimeSync::TimeFormat))
			.arg(aReply.localClockOffset()));

		timeOffsetReceived(aReply.localClockOffset());
	}

	checkNextUrl();
}

//---------------------------------------------------------------------------
void TimeSync::timeReceived(QDateTime aServerDateTime)
{
	if (aServerDateTime.isValid())
	{
		qint64 clockOffset = QDateTime::currentDateTime().msecsTo(aServerDateTime);

		toLog(LogLevel::Normal, QString("Receive HTTP reply: server time: %1, local clock offset: %2 ms")
			.arg(aServerDateTime.toLocalTime().toString(CTimeSync::TimeFormat))
			.arg(clockOffset));

		timeOffsetReceived(clockOffset);
	}

	checkNextUrl();
}

//--------------------------------------------------------------------------------
void TimeSync::timeOffsetReceived(qint64 aLocalTimeOffset)
{
	if (!mTimeReceived)
	{
		mTimeReceived = true;

		if (qAbs(aLocalTimeOffset / 1000) > CTimeSync::DesyncLimit)
		{
			toLog(LogLevel::Normal, QString("Required time synchronization. Offset %1 sec.").arg(aLocalTimeOffset / 1000., 0, 'f', 3));

			try
			{
				ISysUtils::setSystemTime(QDateTime::currentDateTime().addMSecs(aLocalTimeOffset));

				toLog(LogLevel::Normal, "Time is syncronized.");

				ILog::logRotateAll();
			}
			catch (Exception & e)
			{
				toLog(LogLevel::Error, QString("Failed to set new system date. %1").arg(e.getMessage()));

				mTimeReceived = false;
			}
		}
		else
		{
			toLog(LogLevel::Normal, QString("Time synchronization is not required.  Offset %1 sec.").arg(aLocalTimeOffset / 1000., 0, 'f', 3));
		}
	}
}

//--------------------------------------------------------------------------------
void TimeSync::httpCheckMethod(const QUrl & aHost)
{
	toLog(LogLevel::Normal, QString("Get time from %1...").arg(aHost.toString()));

	if (!mNetwork)
	{
		toLog(LogLevel::Error, "Failed to check connection. Network interface is not specified.");

		emit finished(mName, mTimeReceived);
	}
	else
	{
		NetworkTask * task(new NetworkTask());

		task->setTimeout(CTimeSync::HttpTimeout * 1000);
		task->setUrl(aHost);
		// По-хорошему, тут должен быть HEAD-запрос, но он почему-то не проходит аутентификацию на прокси-сервере.
		task->setType(NetworkTask::Get);
		task->setDataStream(new MemoryDataStream());

		connect(task, SIGNAL(onComplete()), this, SLOT(httpReplyReceived()));

		mNetwork->addTask(task);
	}
}

//--------------------------------------------------------------------------------
void TimeSync::httpReplyReceived()
{
	NetworkTask * task = dynamic_cast<NetworkTask *>(sender());

	mTimeSyncHosts.remove(task->getUrl());

	// Проверка серверной даты и активация сигнала о её получении.
	QDateTime serverDate = task->getServerDate();
	if (serverDate.isValid())
	{
		timeReceived(serverDate);
	}
	else
	{
		toLog(LogLevel::Error, QString("Get time failed. Error %1.").arg(task->errorString()));
	}
}

//---------------------------------------------------------------------------

