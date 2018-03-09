/* @file Реализация базового класса для сетевого запроса. */

#include <Common/QtHeadersBegin.h>
#include <QtCore/QMutexLocker>
#include <Common/QtHeadersEnd.h>

#include "IVerifier.h"
#include "DataStream.h"
#include "NetworkTaskManager.h"
#include "NetworkTask.h"

//------------------------------------------------------------------------
NetworkTask::NetworkTask()
	: mType(Get), mTimeout(0), mError(NotReady), mHttpError(0), mProcessing(false), mParentThread(QThread::currentThread()), mFlags(None), mSize(0), mCurrentSize(0)
{
	mTimer.setParent(this);
	mTimer.setSingleShot(true);

	connect(&mTimer, SIGNAL(timeout()), SLOT(onTimeout()));
}

//------------------------------------------------------------------------
NetworkTask::~NetworkTask()
{
}

//------------------------------------------------------------------------
NetworkTaskManager * NetworkTask::getManager() const
{
	return mManager;
}

QString qtNetworkError(QNetworkReply::NetworkError aError)
{
	switch(aError)
	{
	case QNetworkReply::ConnectionRefusedError:       return "ConnectionRefusedError";
	case QNetworkReply::RemoteHostClosedError:        return "RemoteHostClosedError";
	case QNetworkReply::HostNotFoundError:            return "HostNotFoundError";
	case QNetworkReply::TimeoutError:                 return "TimeoutError";
	case QNetworkReply::OperationCanceledError:       return "OperationCanceledError";
	case QNetworkReply::SslHandshakeFailedError:      return "SslHandshakeFailedError";
	case QNetworkReply::TemporaryNetworkFailureError: return "TemporaryNetworkFailureError";
	case QNetworkReply::UnknownNetworkError:          return "UnknownNetworkError";

	// proxy errors (101-199):
	case QNetworkReply::ProxyConnectionRefusedError:      return "ProxyConnectionRefusedError";
	case QNetworkReply::ProxyConnectionClosedError:       return "ProxyConnectionClosedError";
	case QNetworkReply::ProxyNotFoundError:               return "ProxyNotFoundError";
	case QNetworkReply::ProxyTimeoutError:                return "ProxyTimeoutError";
	case QNetworkReply::ProxyAuthenticationRequiredError: return "ProxyAuthenticationRequiredError";
	case QNetworkReply::UnknownProxyError:                return "UnknownProxyError";

	// content errors (201-299):
	case QNetworkReply::ContentAccessDenied:               return "ContentAccessDenied";
	case QNetworkReply::ContentOperationNotPermittedError: return "ContentOperationNotPermittedError";
	case QNetworkReply::ContentNotFoundError:              return "ContentNotFoundError";
	case QNetworkReply::AuthenticationRequiredError:       return "AuthenticationRequiredError";
	case QNetworkReply::ContentReSendError:                return "ContentReSendError";
	case QNetworkReply::UnknownContentError:               return "UnknownContentError";

	// protocol errors
	case QNetworkReply::ProtocolUnknownError:          return "ProtocolUnknownError";
	case QNetworkReply::ProtocolInvalidOperationError: return "ProtocolInvalidOperationError";
	case QNetworkReply::ProtocolFailure:               return "ProtocolFailure";

	default: return "";
	}
}

//------------------------------------------------------------------------
QString NetworkTask::errorString()
{
	switch (getError())
	{
		case NoError: return "no error";
		case StreamWriteError: return "stream write error";
		case UnknownOperation: return "unknown operation specified";
		case Timeout: return "request timeout exceeded";
		case BadTask: return "bad task";
		case VerifyFailed: return "verify failed";
		case TaskFailedButVerified: return "network error but content is verified";
		case NotReady: return "task is in progress";

		default:
			if (mNetworkReplyError.isEmpty())
			{
				return QString("(%1) %2").arg(getError()).arg(qtNetworkError((QNetworkReply::NetworkError)getError()));
			}
			else
			{
				return QString("(%1) %2").arg(getError()).arg(mNetworkReplyError);
			}
	}
}

//------------------------------------------------------------------------
void NetworkTask::setProcessing(NetworkTaskManager * aManager, bool aProcessing)
{
	mManager = aManager;
	mProcessing = aProcessing;

	if (aProcessing)
	{
		if (mTimer.interval())
		{
			mTimer.start();
		}

		this->moveToThread(getManager()->thread());

		mProcessingMutex.lock();
	}
	else
	{
		mTimer.stop();

		if (mParentThread != nullptr && mParentThread->isRunning())
		{
			this->moveToThread(mParentThread);
		}

		if (mVerifier)
		{
			if (!mVerifier->verify(this, getDataStream()->readAll()))
			{
				if (getError() == NoError)
				{
					setError(VerifyFailed);
				}
			}
			else
			{
				if (getError() != NoError)
				{
					setError(TaskFailedButVerified);
				}
			}
		}

		emit onComplete();

		mProcessingCondition.wakeAll();
	}
}

//------------------------------------------------------------------------
void NetworkTask::resetTimer()
{
	if (mProcessing)
	{
		if (mTimer.interval())
		{
			mTimer.start();
		}
	}
}

//------------------------------------------------------------------------
void NetworkTask::setType(Type aType)
{
	mType = aType;
}

//------------------------------------------------------------------------
NetworkTask::Type NetworkTask::getType() const
{
	return mType;
}

//------------------------------------------------------------------------
void NetworkTask::setUrl(const QUrl & aUrl)
{
	mUrl = aUrl;
}

//------------------------------------------------------------------------
const QUrl & NetworkTask::getUrl() const
{
	return mUrl;
}

//------------------------------------------------------------------------
void NetworkTask::setTimeout(int aMsec)
{
	mTimer.setInterval(aMsec);
}

//------------------------------------------------------------------------
int NetworkTask::getTimeout() const
{
	return mTimeout;
}

//------------------------------------------------------------------------
void NetworkTask::setFlags(Flags aFlags)
{
	mFlags = aFlags;
}

//------------------------------------------------------------------------
NetworkTask::Flags NetworkTask::getFlags() const
{
	return mFlags;
}

//------------------------------------------------------------------------
void NetworkTask::setError(int aError, const QString & aMessage)
{
	if ((getError() != QNetworkReply::NoError) && (aError == QNetworkReply::OperationCanceledError))
	{
		return;
	}

	mError = aError;
	mNetworkReplyError = aMessage;
}

//------------------------------------------------------------------------
int NetworkTask::getError() const
{
	return mError;
}

//------------------------------------------------------------------------
void NetworkTask::setHttpError(int aError)
{
	mHttpError = aError;
}

//------------------------------------------------------------------------
int NetworkTask::getHttpError() const
{
	return mHttpError;
}

//------------------------------------------------------------------------
void NetworkTask::setVerifier(IVerifier * aVerifier)
{
	mVerifier = QSharedPointer<IVerifier>(aVerifier);
}

//------------------------------------------------------------------------
IVerifier * NetworkTask::getVerifier() const
{
	return mVerifier.data();
}

//------------------------------------------------------------------------
void NetworkTask::setDataStream(DataStream * aDataStream)
{
	mDataStream = QSharedPointer<DataStream>(aDataStream);
}

//------------------------------------------------------------------------
DataStream * NetworkTask::getDataStream() const
{
	return mDataStream.data();
}

//------------------------------------------------------------------------
void NetworkTask::onTimeout()
{
	setError(NetworkTask::Timeout);

	getManager()->removeTask(this);
}

//------------------------------------------------------------------------
NetworkTask::TByteMap & NetworkTask::getRequestHeader()
{
	return mRequestHeader;
}

//------------------------------------------------------------------------
NetworkTask::TByteMap & NetworkTask::getResponseHeader()
{
	return mResponseHeader;
}

//------------------------------------------------------------------------
void NetworkTask::waitForFinished()
{
	if (mProcessing)
	{
		mProcessingCondition.wait(&mProcessingMutex);
	}

	// �������������� ������������ ������, ��� ������� � ��������� ��������������
	mProcessingMutex.tryLock();
	mProcessingMutex.unlock();
}

//------------------------------------------------------------------------
void NetworkTask::setSize(qint64 aCurrent, qint64 aTotal)
{
	mCurrentSize = aCurrent;
	mSize = aTotal;

	emit onProgress(aCurrent, aTotal);
}

//------------------------------------------------------------------------
qint64 NetworkTask::getSize() const
{
	return mSize;
}

//------------------------------------------------------------------------
qint64 NetworkTask::getCurrentSize() const
{
	return mCurrentSize;
}

//------------------------------------------------------------------------
void NetworkTask::setTag(const QVariant & aTag)
{
	mTag = aTag;
}

//------------------------------------------------------------------------
const QVariant & NetworkTask::getTag() const
{
	return mTag;
}

//------------------------------------------------------------------------
QDateTime NetworkTask::getServerDate() const
{
	QDateTime date;

	// Проверим на наличие серверной даты в ответе
	if (mResponseHeader.contains("Date"))
	{
		// Пример: Mon, 05 Sep 2011 10:43:11 GMT
		QString dateString = QString::fromLatin1(mResponseHeader["Date"]);
		dateString.replace(0, dateString.indexOf(",") + 2, "");
		dateString.chop(4);

		dateString
				.replace("Jan", "01")
				.replace("Feb", "02")
				.replace("Mar", "03")
				.replace("Apr", "04")
				.replace("May", "05")
				.replace("Jun", "06")
				.replace("Jul", "07")
				.replace("Aug", "08")
				.replace("Sep", "09")
				.replace("Oct", "10")
				.replace("Nov", "11")
				.replace("Dec", "12");

		date = QDateTime::fromString(dateString, "dd MM yyyy hh:mm:ss");
		date.setTimeSpec(Qt::UTC);
	}

	return date;
}

//------------------------------------------------------------------------
