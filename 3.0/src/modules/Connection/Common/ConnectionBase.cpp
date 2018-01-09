/* @file Реализация базового функционала сетевого соединения. */

// Qt headers
#include "Common/QtHeadersBegin.h"
#include <QtCore/QStringList>
#include "Common/QtHeadersEnd.h"

// Модули
#include <Common/ScopedPointerLaterDeleter.h>

#include <NetworkTaskManager/NetworkTaskManager.h>
#include <NetworkTaskManager/NetworkTask.h>
#include <NetworkTaskManager/MemoryDataStream.h>

// Проект
#include "ConnectionBase.h"

//--------------------------------------------------------------------------------
/// Константы
namespace CConnection
{
	/// Период проверки статуса соединения.
	const int DefaultCheckPeriod = 60 * 1000; // 1 минутa

	/// Период пинга соединения.
	const int DefaultPingPeriod = 15 * 60 * 1000; // 15 минут

	/// Таймаут запроса проверки соединения.
	const int PingTimeout = 30 * 1000; // 30 секунд

	/// Хост по умолчанию для проверки соединения.
	const QString DefaultCheckHost = "http://mon.cyberplat.com:80/ping";

	/// Строка ответа по умолчанию для проверки соединения.
	const QString DefaultCheckResponse = "";
}

//--------------------------------------------------------------------------------
ConnectionBase::ConnectionBase(const QString & aName, NetworkTaskManager * aNetwork, ILog * aLog)
	: mNetwork(aNetwork),
	  mName(aName),
	  mConnected(false),
	  mCheckCount(0),
	  mWatch(false),
	  mLog(aLog)
{
	// Таймер будет взводится заново после каждой удачной проверки
	mCheckTimer.setSingleShot(true);
	mCheckTimer.setInterval(CConnection::DefaultCheckPeriod);
	QObject::connect(&mCheckTimer, SIGNAL(timeout()), this, SLOT(onCheckTimeout()));
	
	mCheckHosts << IConnection::CheckUrl(QUrl(CConnection::DefaultCheckHost), CConnection::DefaultCheckResponse);
}

//--------------------------------------------------------------------------------
void ConnectionBase::open(bool aWatch) throw(...)
{
	toLog(LogLevel::Normal, QString("*").repeated(40));
	toLog(LogLevel::Normal, QString("Connection:     %1").arg(getName()));
	toLog(LogLevel::Normal, QString("Type:           %1").arg(EConnectionTypes::getConnectionTypeName(getType())));

	if (aWatch)
	{
		toLog(LogLevel::Normal, QString("Check interval: %1 sec").arg(mCheckTimer.interval() / 1000));
		toLog(LogLevel::Normal, QString("Ping interval: %1 sec").arg(mCheckTimer.interval() * mPingPeriod / 1000));
	}
	else
	{
		toLog(LogLevel::Normal, "Check interval: uncontrolled connection");
	}

	toLog(LogLevel::Normal, QString("*").repeated(40));

	if (!isConnected(false))
	{
		doConnect();
	}
	else
	{
		toLog(LogLevel::Normal, "Already connected.");
	}

	mWatch = aWatch;
	mCheckCount = 0;
	mConnected = true;

	// Если нужно наблюдение за соединением
	if (mWatch)
	{
		toLog(LogLevel::Normal, "Check timer START.");
		mCheckTimer.start();

		// пропингуем сервер на следующем шаге проверки
		mCheckCount = mPingPeriod - 1;
	}
}

//--------------------------------------------------------------------------------
void ConnectionBase::close() throw(...)
{
	toLog(LogLevel::Debug, "Check timer STOP.");
	mCheckTimer.stop();

	if (isConnected(true))
	{
		doDisconnect();
	}

	mConnected = false;
}

//--------------------------------------------------------------------------------
QString ConnectionBase::getName() const
{
	return mName;
}

//--------------------------------------------------------------------------------
void ConnectionBase::setCheckPeriod(int aMinutes)
{
	mPingPeriod = aMinutes;
}

//--------------------------------------------------------------------------------
bool ConnectionBase::isConnected(bool aUseCache) throw(...)
{
	if (!aUseCache)
	{
		mConnected = doIsConnected();
	}

	return mConnected;
}

//--------------------------------------------------------------------------------
bool ConnectionBase::checkConnection(const IConnection::CheckUrl & aHost) throw(...)
{
	QList<CheckUrl> hosts;

	if (aHost.first.isValid() && !aHost.first.isEmpty())
	{
		hosts << aHost;
	}
	else
	{
		hosts = mCheckHosts;
	}

	if (hosts.isEmpty())
	{
		toLog(LogLevel::Error, QString("Cannot check connection, no hosts specified."));
		return true;
	}

	// Для каждого хоста из списка или до первой удачной проверки
	foreach (auto host, hosts)
	{
		if (doCheckConnection(host))
		{
			emit connectionAlive();
			
			return true;
		}
	}

	return false;
}

//--------------------------------------------------------------------------------
void ConnectionBase::setCheckHosts(const QList<IConnection::CheckUrl> & aHosts)
{
	if (aHosts.isEmpty())
	{
		toLog(LogLevel::Warning, QString("No check hosts specified, using previous/default ones."));
	}
	else
	{
		mCheckHosts = aHosts;
	}
}

//--------------------------------------------------------------------------------
void ConnectionBase::onCheckTimeout()
{
	++mCheckCount;

	toLog(LogLevel::Debug, QString("Check timeout. count: %1.").arg(mCheckCount));

	try
	{
		// Проверяем состояние соединения через апи ОС и делаем http запрос если подошло время
		if (isConnected(false) && mCheckCount >= mPingPeriod)
		{
			mCheckCount = 0;
			mConnected = checkConnection();
		}

		if (isConnected(true))
		{
			toLog(LogLevel::Debug, "Check timer START.");
			mCheckTimer.start();
		}
		else
		{
			emit connectionLost();
		}	
	}
	catch (const NetworkError & e)
	{
		toLog(LogLevel::Error, e.getMessage());

		emit connectionLost();
	}
}

//--------------------------------------------------------------------------------
bool ConnectionBase::httpCheckMethod(const IConnection::CheckUrl & aHost)
{
	toLog(LogLevel::Normal, QString("Checking connection on %1...").arg(aHost.first.toString()));

	if (!mNetwork)
	{
		toLog(LogLevel::Error, "Failed to check connection. Network interface is not specified.");

		return false;
	}

	QScopedPointer<NetworkTask, ScopedPointerLaterDeleter<NetworkTask>> task(new NetworkTask());

	task->setTimeout(CConnection::PingTimeout);
	task->setUrl(aHost.first);
	// По-хорошему, тут должен быть HEAD-запрос, но он почему-то не проходит аутентификацию на прокси-сервере.
	task->setType(NetworkTask::Get);
	task->setDataStream(new MemoryDataStream());

	mNetwork->addTask(task.data());

	task->waitForFinished();

	QByteArray answer = task->getDataStream()->takeAll();

	auto traceLog = [&]() {
		toLog(LogLevel::Trace, QString("error:%1 http_code:%2").arg(task->getError()).arg(task->getHttpError()));

		QStringList response;
		QMapIterator<QByteArray, QByteArray> i(task->getResponseHeader());
		while (i.hasNext())
		{
			i.next();
			response << QString("%1: %2").arg(QString::fromLatin1(i.key())).arg(QString::fromLatin1(i.value()));
		}

		toLog(LogLevel::Trace, QString("HEADER:\n%1\nBODY:\n%2")
			.arg(response.join("\n"))
			.arg(QString::fromLatin1(answer.left(80))));
	};

	if (task->getError() != NetworkTask::NoError)
	{
		toLog(LogLevel::Error, QString("Connection check failed. Error %1.").arg(task->errorString()));

		traceLog();

		return false;
	}

	if (!aHost.second.isEmpty() && !answer.contains(aHost.second.toLatin1()))
	{
		toLog(LogLevel::Error, QString("Connection check failed. Server answer verify failed '%1'.").arg(aHost.second));

		traceLog();

		return false;
	}

	toLog(LogLevel::Normal, "Connection check ok.");

	return true;
}

//----------------------------------------------------------------------------
void ConnectionBase::toLog(LogLevel::Enum aLevel, const QString & aMessage) const
{
	mLog->write(aLevel, aMessage);
}

//----------------------------------------------------------------------------
