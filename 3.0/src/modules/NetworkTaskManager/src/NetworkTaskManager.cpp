/* @file Реализация менеджера загрузки файлов с возможностью докачки. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtNetwork/QNetworkRequest>
#include <Common/QtHeadersEnd.h>

// Project
#include "DataStream.h"
#include "NetworkTask.h"
#include "NetworkTaskManager.h"

//------------------------------------------------------------------------
NetworkTaskManager::NetworkTaskManager()
{
	qRegisterMetaType<QNetworkProxy>("QNetworkProxy");
	qRegisterMetaType<NetworkTask *>("NetworkTask");

	moveToThread(this);

	setObjectName("NetworkTaskManager");

	start();
}

//------------------------------------------------------------------------
NetworkTaskManager::~NetworkTaskManager()
{
	if (isRunning())
	{
		quit();
		if (!wait(3000))
		{
			toLog(LogLevel::Error, "Terminate NetworkTaskManager thread.");
			terminate();
		}
	}
}

//------------------------------------------------------------------------
void NetworkTaskManager::setProxy(const QNetworkProxy & aProxy)
{
	metaObject()->invokeMethod(this, "onSetProxy", Qt::QueuedConnection, Q_ARG(QNetworkProxy, aProxy));
}

//------------------------------------------------------------------------
void NetworkTaskManager::setDownloadSpeedLimit(int aPercent)
{
	metaObject()->invokeMethod(this, "onSetDownloadSpeedLimit", Qt::QueuedConnection, Q_ARG(int, aPercent));
}

//------------------------------------------------------------------------
void NetworkTaskManager::addTask(NetworkTask * aTask)
{
	aTask->setProcessing(this, true);
	metaObject()->invokeMethod(this, "onAddTask", Qt::AutoConnection, Q_ARG(NetworkTask *, aTask));
}

//------------------------------------------------------------------------
void NetworkTaskManager::removeTask(NetworkTask * aTask)
{
	metaObject()->invokeMethod(this, "onRemoveTask", Qt::AutoConnection, Q_ARG(NetworkTask *, aTask));
}

//------------------------------------------------------------------------
void NetworkTaskManager::onSetProxy(QNetworkProxy aProxy)
{
	mNetwork->setProxy(aProxy);
}

//------------------------------------------------------------------------
void NetworkTaskManager::onSetDownloadSpeedLimit(int /*aPercent*/)
{
	// TODO:
}

//------------------------------------------------------------------------
void NetworkTaskManager::onAddTask(NetworkTask * aTask)
{
	toLog(LogLevel::Debug, QString("> url:%1").arg(aTask->getUrl().toString()));

	QNetworkRequest request;

	request.setUrl(aTask->getUrl());

	if (aTask->getFlags() & NetworkTask::Continue)
	{
		qint64 offset = aTask->getDataStream()->size();

		if (offset > 0)
		{
			request.setRawHeader("Range", QString("bytes=%1-").arg(offset).toLatin1());

			toLog(LogLevel::Normal, QString("Downloading data. Url: %1. Offset: %2.")
				.arg(aTask->getUrl().toString(QUrl::RemoveUserInfo))
				.arg(offset));
		}
	}

	// Добавляем установленные хидеры в запрос
	const NetworkTask::TByteMap & headers = aTask->getRequestHeader();
	for (auto it = headers.begin(); it != headers.end(); ++it)
	{
		request.setRawHeader(it.key(), it.value());
	}

	// Добавляем User-Agent, если он ранее не был добавлен
	if (!headers.contains("User-Agent")) 
	{
		request.setRawHeader("User-Agent", getUserAgent().toLatin1());
	}

	aTask->getResponseHeader().clear();

	QNetworkReply * reply = 0;

	switch (aTask->getType())
	{
		case NetworkTask::Head:
		{
			aTask->getDataStream()->clear();

			reply = mNetwork->head(request);

			break;
		}

		case NetworkTask::Get:
		{
			reply = mNetwork->get(request);

			break;
		}

		case NetworkTask::Post:
		{
			QByteArray postData = aTask->getDataStream()->takeAll();

			toLog(LogLevel::Debug, QString("> POST %1 bytes").arg(postData.size()));

			reply = mNetwork->post(request, postData);

			break;
		}

		default:
		{
			toLog(LogLevel::Error, QString("Failed to process task. Unknown operation type: %1.").arg(aTask->getType()));

			aTask->setError(NetworkTask::UnknownOperation);

			removeTask(aTask);

			return;
		}
	}

	mTasks[reply] = aTask;

	connect(reply, SIGNAL(downloadProgress(qint64, qint64)), SLOT(onTaskProgress(qint64, qint64)));
	connect(reply, SIGNAL(uploadProgress(qint64, qint64)), SLOT(onTaskUploadProgress(qint64, qint64)));
	connect(reply, SIGNAL(readyRead()), SLOT(onTaskReadyRead()));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(onTaskError(QNetworkReply::NetworkError)));
	connect(reply, SIGNAL(sslErrors(const QList<QSslError> &)), SLOT(onTaskSslErrors(const QList<QSslError> &)));
	connect(reply, SIGNAL(finished()), SLOT(onTaskComplete()));
}

//------------------------------------------------------------------------
void NetworkTaskManager::onRemoveTask(NetworkTask * aTask)
{
	for (TTaskMap::iterator it = mTasks.begin(); it != mTasks.end(); ++it)
	{
		if (it.value().data() == aTask && !it.value().isNull())
		{
			disconnect(it.key(), 0, this, 0);

			it.key()->close();
			it.key()->abort();

			if (aTask->getError())
			{
				toLog(LogLevel::Error, QString("< Error: %1. HttpError: %2. Request URL: %3.")
					.arg(aTask->errorString()).arg(aTask->getHttpError()).arg(aTask->getUrl().toString()));
			}

			it.value().data()->setProcessing(this, false);

			it.key()->deleteLater();

			mTasks.erase(it);

			return;
		}
	}
}

//------------------------------------------------------------------------
void NetworkTaskManager::onTaskProgress(qint64 aReceived, qint64 aTotal)
{
	TTaskMap::iterator it = mTasks.find(dynamic_cast<QNetworkReply *>(sender()));

	if (it != mTasks.end() && !it.value().isNull())
	{
		auto task = it.value().data();

		task->setSize(aReceived, aTotal);
		task->resetTimer();
	}
}

//------------------------------------------------------------------------
void NetworkTaskManager::onTaskUploadProgress(qint64, qint64)
{
	TTaskMap::iterator it = mTasks.find(dynamic_cast<QNetworkReply *>(sender()));

	if (it != mTasks.end() && !it.value().isNull())
	{
		it.value().data()->resetTimer();
	}
}

//------------------------------------------------------------------------
void NetworkTaskManager::onTaskReadyRead()
{
	QNetworkReply * reply = dynamic_cast<QNetworkReply *>(sender());

	if (reply && mTasks.contains(reply) && !mTasks.value(reply).isNull())
	{
		auto task = mTasks.value(reply).data();

		if (task)
		{
			QVariant httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
			if (httpStatusCode.isValid())
			{
				int statusCode = httpStatusCode.toInt();
				task->setHttpError(statusCode);

				switch (statusCode)
				{
					// Ошибка в заголовоке Range
					case 416:
					{
						toLog(LogLevel::Error, "Request range header is wrong, cannot download content.");

						task->setError(NetworkTask::BadTask);
						reply->abort();
						break;
					}

					case 200: // Успех.
					case 206: // Успех частичнного скачивания.
					{
						if (task->getSize() == 0)
						{
							if (statusCode == 200)
							{
								// Если запрашивали кусок данных, а пришел файл целиком - нужно начинать писать поток с 0-го байта
								task->getDataStream()->clear();
							}
							else 
							{
								// Если запрашивали кусок данных позиционируем на начало передаваемого диапазона
								// http://tools.ietf.org/html/rfc2616#section-14.16
								QRegExp rx("(\\d+)\\-\\d+/\\d+");

								if (rx.indexIn(QString::fromLatin1(reply->rawHeader("Content-Range"))) > 0)
								{
									task->getDataStream()->seek(rx.cap(1).toLongLong());
								}
								else
								{
									task->getDataStream()->clear();
								}
							}
						}

						QByteArray replyData = reply->readAll();

						toLog(LogLevel::Debug, QString("< receive %1%2 bytes.").arg(statusCode == 206 ? "(partial) " : "").arg(replyData.size()));

						if (!task->getDataStream()->write(replyData))
						{
							toLog(LogLevel::Error, "Cannot save received data to the stream.");
							task->setError(NetworkTask::StreamWriteError);
							reply->abort();
						}

						break;
					}

					default:
						toLog(LogLevel::Error, QString("Data is ready for read, but responce code is incorrect: %1").arg(httpStatusCode.toString()));
						reply->abort();
				}
			}
		}
	}
}

//------------------------------------------------------------------------
void NetworkTaskManager::onTaskError(QNetworkReply::NetworkError aError)
{
	for (TTaskMap::iterator it = mTasks.begin(); it != mTasks.end(); ++it)
	{
		if (it.key() == sender() && !it.value().isNull())
		{
			it.value().data()->setError(aError, it.key()->errorString());

			switch (aError)
			{
			case QNetworkReply::ConnectionRefusedError:
			case QNetworkReply::RemoteHostClosedError:
			case QNetworkReply::HostNotFoundError:
			case QNetworkReply::TimeoutError:
			case QNetworkReply::TemporaryNetworkFailureError:
			case QNetworkReply::ProxyNotFoundError:
			case QNetworkReply::ProxyTimeoutError:
				emit networkTaskStatus(true);
				break;
			}

			return;
		}
	}
}

//------------------------------------------------------------------------
void NetworkTaskManager::onTaskSslErrors(const QList<QSslError> & aErrors)
{
	QString errorString;

	foreach (const QSslError & error, aErrors)
	{
		if (!errorString.isEmpty())
			errorString += ", ";

		errorString += error.errorString();
	}

	toLog(LogLevel::Warning, QString("One or more SSL errors has occurred: %1").arg(errorString));

#if defined(_DEBUG) || defined(DEBUG_INFO)
	dynamic_cast<QNetworkReply *>(sender())->ignoreSslErrors();
#endif // _DEBUG || DEBUG_INFO
}

//------------------------------------------------------------------------
void NetworkTaskManager::onTaskComplete()
{
	QNetworkReply * reply = dynamic_cast<QNetworkReply *>(sender());

	for (TTaskMap::iterator it = mTasks.begin(); it != mTasks.end(); ++it)
	{
		if (it.key() == reply && !it.value().isNull())
		{
			auto task = it.value().data();

			int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

			QList<QByteArray> headers = reply->rawHeaderList();
			foreach (QByteArray header, headers)
			{
				task->getResponseHeader().insert(header, reply->rawHeader(header));
			}

			task->getResponseHeader().insert("Content-Type", reply->header(QNetworkRequest::ContentTypeHeader).toByteArray());

			if (reply->error() != QNetworkReply::OperationCanceledError)
			{
				task->setError(reply->error(), reply->errorString());
				if (statusCode && ((statusCode == 301) || (statusCode == 416) || (statusCode == 404)))
				{
					toLog(LogLevel::Warning, QString("Set bad task error, because statusCode=%1.").arg(statusCode));

					task->setHttpError(statusCode);
					task->setError(NetworkTask::BadTask);
				}
				else if (task->getCurrentSize() != task->getSize())
				{
					toLog(LogLevel::Warning, QString("Set bad task error, because taskSize != size: (%1 != %2).").arg(task->getCurrentSize()).arg(task->getSize()));

					// Qt error work around.
					task->setError(NetworkTask::BadTask);
				}
			}

			if (!task->getError())
			{
				// сообщаем об успешном статусе задачи
				emit networkTaskStatus(false);
			}

			removeTask(task);

			return;
		}
	}
}

//------------------------------------------------------------------------
void NetworkTaskManager::run()
{
	mNetwork = QSharedPointer<QNetworkAccessManager>(new QNetworkAccessManager());

	QThread::exec();

	onClearTasks();

	mNetwork.clear();
}

//------------------------------------------------------------------------
void NetworkTaskManager::setUserAgent(const QString & aUserAgent)
{
	mUserAgent = aUserAgent;
}

//------------------------------------------------------------------------
QString NetworkTaskManager::getUserAgent() const
{
	return mUserAgent;
}

//------------------------------------------------------------------------
void NetworkTaskManager::clearTasks()
{
	metaObject()->invokeMethod(this, "onClearTasks", Qt::AutoConnection);
}

//------------------------------------------------------------------------
void NetworkTaskManager::onClearTasks()
{
	while (!mTasks.isEmpty())
	{
		onRemoveTask(mTasks.begin().value().data());
	}
}

//------------------------------------------------------------------------