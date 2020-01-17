/* @file TCP-порт. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <QtCore/QMetaType>
#include <Common/QtHeadersEnd.h>

// Project
#include "TCPPort.h"

//--------------------------------------------------------------------------------
TCPPort::TCPPort() : mState(QAbstractSocket::UnconnectedState), mError(QAbstractSocket::UnknownSocketError), mSocketGuard(QMutex::Recursive)
{
	mType = SDK::Driver::EPortTypes::TCP;
	setOpeningTimeout(CTCPPort::OpeningTimeout);

	moveToThread(&mThread);
	mThread.start();

	qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");
	qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");

	connect(this, SIGNAL(invoke(TBoolMethod, bool *)), SLOT(onInvoke(TBoolMethod, bool *)), Qt::BlockingQueuedConnection);
}

//--------------------------------------------------------------------------------
bool TCPPort::invokeMethod(TBoolMethod aMethod)
{
	bool result;

	isWorkingThread() ? onInvoke(aMethod, &result) : emit invoke(aMethod, &result);

	return result;
}

//--------------------------------------------------------------------------------
void TCPPort::onInvoke(TBoolMethod aMethod, bool * aResult)
{
	*aResult = aMethod();
}

//--------------------------------------------------------------------------------
void TCPPort::initialize()
{
}

//--------------------------------------------------------------------------------
bool TCPPort::opened()
{
	return PERFORM_IN_THREAD(performOpened);
}

//--------------------------------------------------------------------------------
bool TCPPort::performOpened()
{
	return mSocket && (mSocket->state() == QAbstractSocket::ConnectedState);
}

//--------------------------------------------------------------------------------
bool TCPPort::open()
{
	return PERFORM_IN_THREAD(performOpen);
}

//--------------------------------------------------------------------------------
bool TCPPort::performOpen()
{
	if (!mSocket)
	{
		mSocket = PSocket(new QTcpSocket());

		connect(mSocket.data(), SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(onStateChanged(QAbstractSocket::SocketState)));
		connect(mSocket.data(), SIGNAL(error(QAbstractSocket::SocketError)), SLOT(onErrorChanged(QAbstractSocket::SocketError)));
		connect(mSocket.data(), SIGNAL(readyRead()), SLOT(onReadyRead()));
	}

	if (mSocket->state() == QAbstractSocket::ConnectedState)
	{
		return true;
	}

	QString IP = getConfigParameter(CHardwareSDK::Port::TCP::IP).toString();

	auto writeLog = [&] (const QString & aLog) { toLog((aLog == mLastErrorLog) ? LogLevel::Debug : LogLevel::Error, aLog); mLastErrorLog = aLog; };

	if (QRegExp(CTCPPort::AddressMask).indexIn(IP) == -1)
	{
		writeLog(QString("Failed to open the TCP socket with IP = %1, need like %2").arg(IP).arg(CTCPPort::AddressMaskLog));
		return false;
	}

	mSocket->abort();

	uint portNumber = getConfigParameter(CHardwareSDK::Port::TCP::Number).toUInt();
	mSocket->connectToHost(IP, portNumber);
	QString portLogName = QString("TCP socket %1:%2").arg(IP).arg(portNumber);

	if (!mSocket->waitForConnected(mOpeningTimeout))
	{
		writeLog(QString("Failed to open the %1 due to timeout = %2 is expired").arg(portLogName).arg(mOpeningTimeout));
		return false;
	}

	QAbstractSocket::SocketState state = mSocket->state();

	if (state != QAbstractSocket::ConnectedState)
	{
		writeLog(QString("Failed to open the %1 due to wrong state = %2").arg(portLogName).arg(int(state)));
		return false;
	}

	mSocket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
	mSocket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);

	toLog(LogLevel::Normal, portLogName + " is opened");

	return true;
}

//--------------------------------------------------------------------------------
bool TCPPort::close()
{
	QMutexLocker locker(&mSocketGuard);

	mLastErrorLog.clear();
	bool result = PERFORM_IN_THREAD(performClose);

	SleepHelper::msleep(CTCPPort::CloseningPause);

	return result;
}

//--------------------------------------------------------------------------------
bool TCPPort::performClose()
{
	if (!mSocket || (mSocket->state() == QAbstractSocket::UnconnectedState))
	{
		return true;
	}

	mSocket->close();

	if (mSocket->state() == QAbstractSocket::UnconnectedState)
	{
		toLog(LogLevel::Normal, "TCP socket is closed");
		return true;
	}

	if (!mSocket->waitForDisconnected(CTCPPort::CloseningTimeout))
	{
		toLog(LogLevel::Error, QString("Failed to close the TCP socket due to timeout = %1 is expired").arg(CTCPPort::CloseningTimeout));
		return false;
	}

	QAbstractSocket::SocketState state = mSocket->state();

	if (state != QAbstractSocket::UnconnectedState)
	{
		toLog(LogLevel::Error, QString("Failed to close the TCP socket due to wrong state = %1").arg(int(state)));
		return false;
	}

	toLog(LogLevel::Normal, QString("TCP socket is closed, state = %1").arg(int(state)));

	return true;
}

//--------------------------------------------------------------------------------
void TCPPort::onStateChanged(QAbstractSocket::SocketState aState)
{
	toLog(LogLevel::Debug, QString("Socket state has changed = %1").arg(int(aState)));

	mState = aState;
}

//--------------------------------------------------------------------------------
void TCPPort::onErrorChanged(QAbstractSocket::SocketError aError)
{
	if (aError != QAbstractSocket::SocketTimeoutError)
	{
		toLog(LogLevel::Debug, QString("Socket error has changed = %1").arg(int(aError)));

		mError = aError;
	}
}

//--------------------------------------------------------------------------------
void TCPPort::onReadyRead()
{
	QMutexLocker locker(&mDataFromGuard);

	mDataFrom += mSocket->readAll();
}

//--------------------------------------------------------------------------------
bool TCPPort::checkReady()
{
	return PERFORM_IN_THREAD(performCheckReady);
}

//--------------------------------------------------------------------------------
bool TCPPort::performCheckReady()
{
	return mSocket && ((mSocket->state() == QAbstractSocket::ConnectedState) || open());
}

//--------------------------------------------------------------------------------
bool TCPPort::read(QByteArray & aData, int aTimeout, int aMinSize)
{
	QMutexLocker locker(&mSocketGuard);

	return PERFORM_IN_THREAD(performRead, std::ref(aData), aTimeout, aMinSize);
}

//--------------------------------------------------------------------------------
bool TCPPort::performRead(QByteArray & aData, int aTimeout, int aMinSize)
{
	aData.clear();

	if (!checkReady())
	{
		return false;
	}

	QTime waitingTimer;
	waitingTimer.start();

	while ((waitingTimer.elapsed() < aTimeout) && (aData.size() < aMinSize))
	{
		mSocket->waitForReadyRead(CTCPPort::ReadingTimeout);

		QMutexLocker locker(&mDataFromGuard);
		{
			aData += mDataFrom;

			mDataFrom.clear();
		}

		if (aData == CTCPPort::AntiNaglePing)
		{
			aData.clear();
		}
	}

	if (mDeviceIOLoging == ELoggingType::ReadWrite)
	{
		toLog(LogLevel::Normal, QString("%1: << {%2}").arg(mConnectedDeviceName).arg(aData.toHex().constData()));
	}

	return true;
}

//--------------------------------------------------------------------------------
bool TCPPort::write(const QByteArray & aData)
{
	QMutexLocker locker(&mSocketGuard);

	return PERFORM_IN_THREAD(performWrite, std::ref(aData));
}

//--------------------------------------------------------------------------------
bool TCPPort::performWrite(const QByteArray & aData)
{
	if (aData.isEmpty())
	{
		toLog(LogLevel::Normal, mConnectedDeviceName + ": written data is empty.");
		return false;
	}

	if (!checkReady())
	{
		return false;
	}

	if (mDeviceIOLoging != ELoggingType::None)
	{
		toLog(LogLevel::Normal, QString("%1: >> {%2}").arg(mConnectedDeviceName).arg(aData.toHex().constData()));
	}

	if ((mSocket->state() != QAbstractSocket::ConnectedState) && !open())
	{
		return false;
	}

	int bytesWritten = int(mSocket->write(aData));
	int actualSize = aData.size();

	if (bytesWritten != actualSize)
	{
		toLog(LogLevel::Normal, mConnectedDeviceName + QString(": %1 bytes instead of %2 bytes have been written.").arg(bytesWritten).arg(actualSize));
		return false;
	}

	if (!mSocket->waitForBytesWritten())
	{
		toLog(LogLevel::Debug, "Failed to wait writing bytes");

		if (!mSocket->waitForBytesWritten())
		{
			toLog(LogLevel::Error, "Failed twice to wait writing bytes");
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool TCPPort::deviceConnected()
{
	return open();
};

//--------------------------------------------------------------------------------
bool TCPPort::isExist()
{
	return mState != QAbstractSocket::UnconnectedState;
}

//--------------------------------------------------------------------------------
