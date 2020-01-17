/* Реализация протокола с EFTPOS 3.0 компании Uniteller. */

#pragma once

#include <numeric>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTimer>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ISettingsService.h>

// Project headers
#include "Uniteller.h"
#include "API.h"
#include "Responses.h"
#include "Utils.h"
#include "SDK/PaymentProcessor/Settings/ISettingsAdapter.h"

#pragma comment(lib, "psapi.lib")

namespace PPSDK = SDK::PaymentProcessor;

namespace CUniteller
{
	const char * DefaultTerminalID = "0000000000";
	const int ReconnectTimeout = 5000;
	const int PollTimeout = 1000;

	static QStringList PinPad = QStringList() << "Cryptera" << "Virtual" << "Zt" << "PinPad" << "BluePad 50";
	static QStringList CardReader = QStringList() << "CreatorCrt310" << "Neuron" << "Omron" << "Sankyo3k7";
	static QStringList ContactlessReader = QStringList() << "VivoPay Kiosk 2";
	static QStringList Host = QStringList() << "Host";
}

namespace Uniteller
{

//---------------------------------------------------------------------------
API::API(ILog * aLog, SDK::PaymentProcessor::ICore * aCore, quint16 aPort) :
	mPort(aPort),
	mTerminalSettings(static_cast<PPSDK::TerminalSettings *>(aCore->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter))),
	mLoggedIn(false),
	mEnabled(false),
	mHaveContactlessReader(true),
	mLastError(0),
	mGetStateTimerID(0),
	mLastReadyState(false)
{
	setLog(aLog);

	//TODO connect to mSocket signals
	connect(&mSocket, SIGNAL(connected()), this, SLOT(onConnected()));
	connect(&mSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	connect(&mSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
	connect(&mSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

	connect(&mCheckStateTimer, SIGNAL(timeout()), this, SLOT(onCheckStateTimeout()));
	mCheckStateTimer.setInterval(1000);
	mCheckStateTimer.start();
}

//---------------------------------------------------------------------------
void API::enable()
{
	mEnabled = true;
	doConnect();
}

//---------------------------------------------------------------------------
QSharedPointer<API> API::getInstance(ILog * aLog, SDK::PaymentProcessor::ICore * aCore, quint16 aPort /*= CUniteller::DefaultPort*/)
{
	static QSharedPointer<API> gApi = QSharedPointer<API>(new API(aLog, aCore, aPort));

	return gApi;
}

//---------------------------------------------------------------------------
bool API::isReady() const
{
	return mLoggedIn;
}

//---------------------------------------------------------------------------
void API::setupRuntimePath(const QString & aRuntimePath)
{
	mRuntimePath = aRuntimePath;
}

//---------------------------------------------------------------------------
void API::disable()
{
	mEnabled = false;
	mSocket.close();
	mSocket.waitForDisconnected(Uniteller::SocketTimeout);
}

//--------------------------------------------------------------------------------
void API::onCheckStateTimeout()
{
	if (mLastReadyState != isReady())
	{
		mLastReadyState = isReady();

		emit state(mLastReadyState ? StatusCode::OK : StatusCode::Error, "Lost connection", true);
	}
}

//---------------------------------------------------------------------------
void API::login()
{
	mTerminalID = getUPID(mRuntimePath);

	if (!mTerminalID.isEmpty())
	{
		toLog(LogLevel::Normal, QString("> Login. TerminalID: '%1'.").arg(mTerminalID));

		killTimer(mGetStateTimerID);
		killTimer(mLoginCheckTimer);

		mLoggedIn = false;

		send(makeRequest(Uniteller::Class::Session, Uniteller::Login::CodeRequest));
	}
	else
	{
		toLog(LogLevel::Normal, QString("> Login failed. Try again..."));
	}
}

//---------------------------------------------------------------------------
void API::sell(double aAmount, const QString & aOrderID, const QString & aCategory)
{
	if (!isReady())
	{
		// Сервис не готов к оплате, уходим
		emit error("0900");

		return;
	}

	toLog(LogLevel::Normal, QString("> Sell: amount=%1, order=%2, category=%3.")
		.arg(aAmount, 0, 'f', 2).arg(aOrderID).arg(aCategory));

	QByteArray buffer;

	buffer.append(QString("%1").arg(qFloor((aAmount * 1000 + 0.001) / 10.0), 12, 10, QChar('0')).toLatin1());

	if (!aOrderID.isEmpty())
	{
		QByteArray order = aOrderID.toLatin1().right(20);
		order.insert(0, "00000000000000000000", 20 - order.size());
		buffer.append(order);
	}
	
	if (!aCategory.isEmpty())
	{
		buffer.append('\x1b');
		buffer.append(aCategory.toLatin1());
		buffer.append('\x1b');
	}

	send(makeRequest(Uniteller::Class::AuthRequest, Uniteller::Sell::Code, buffer));
}

//---------------------------------------------------------------------------
void API::breakSell()
{
	toLog(LogLevel::Normal, "> Break sell.");

	send(makeRequest(Uniteller::Class::Session, Uniteller::Break::CodeRequest));
}

//---------------------------------------------------------------------------
void API::getState()
{
	toLog(LogLevel::Debug, "> Get state.");

	send(makeRequest(Uniteller::Class::Diagnostic, Uniteller::State::CodeRequest));
}

//---------------------------------------------------------------------------
void API::doConnect()
{
	if (mEnabled)
	{
		mSocket.connectToHost(QHostAddress::LocalHost, mPort);
	}
}

//---------------------------------------------------------------------------
void API::onConnected()
{
	toLog(LogLevel::Normal, "Connected.");

	// Проверка на залогиненность
	mLoginCheckTimer = startTimer(1000);
}

//---------------------------------------------------------------------------
void API::onDisconnected()
{
	toLog(LogLevel::Normal, "Disconnected.");

	killTimer(mGetStateTimerID);
	killTimer(mLoginCheckTimer);
	
	mLoggedIn = false;
}

//---------------------------------------------------------------------------
void API::timerEvent(QTimerEvent *)
{
	if (mEnabled && !mLoggedIn)
	{
		login();
	}
}

//---------------------------------------------------------------------------
void API::onError(QAbstractSocket::SocketError)
{
	QString e = mSocket.errorString();
	if (mLastErrorString != e)
	{
		mLastErrorString = e;
		toLog(LogLevel::Error, QString("Connection error: %1.").arg(mLastErrorString));
	}

	mLastError = -1;

	emit error(mSocket.errorString());

	// переподсоединяемся через 5 сек.
	if (mEnabled)
	{
		QTimer::singleShot(CUniteller::ReconnectTimeout, this, SLOT(doConnect()));
	}
}

//---------------------------------------------------------------------------
bool API::isErrorResponse(BaseResponsePtr aResponse)
{
	auto errorResponse = qSharedPointerDynamicCast<ErrorResponse, BaseResponse>(aResponse);
	if (errorResponse)
	{
		toLog(LogLevel::Error, QString("< Error: 0x%1 (%2)")
			.arg(errorResponse->getError()).arg(errorResponse->getErrorMessage()));

		emit error(translateErrorMessage(errorResponse->getError(), errorResponse->getErrorMessage()));
	}

	return errorResponse;
}

//---------------------------------------------------------------------------
bool API::isLoginResponse(BaseResponsePtr aResponse)
{
	auto loginResponse = qSharedPointerDynamicCast<LoginResponse, BaseResponse>(aResponse);
	if (loginResponse)
	{
		toLog(LogLevel::Normal, QString("< Login OK. TID: %1.").arg(loginResponse->mTerminalID));

		//корректно подсоединились к терминалу
		emit ready();
	
		mLoggedIn = true;
		mHaveContactlessReader = haveContactlessReader();

		mGetStateTimerID = startTimer(CUniteller::PollTimeout);
		QMetaObject::invokeMethod(this, "getState", Qt::QueuedConnection);
	}

	return loginResponse;
}

//---------------------------------------------------------------------------
bool API::isPrintLineResponse(BaseResponsePtr aResponse)
{
	auto printLineResponse = qSharedPointerDynamicCast<PrintLineResponse, BaseResponse>(aResponse);
	if (printLineResponse)
	{
		toLog(LogLevel::Normal, QString("< Print line: %1.").arg(printLineResponse->getText()));

		mCurrentReceipt << printLineResponse->getText();

		if (printLineResponse->isLast())
		{
			emit print(mCurrentReceipt);
			mCurrentReceipt.clear();
		}
	}

	return printLineResponse;
}

//---------------------------------------------------------------------------
bool API::isGetStateResponse(BaseResponsePtr aResponse)
{
	auto stateResponse = qSharedPointerDynamicCast<GetStateResponse, BaseResponse>(aResponse);

	if (stateResponse)
	{
		toLog(LogLevel::Debug, QString("< State: %1 - %2.")
			.arg(stateResponse->state(), 2, 16, QChar('0')).arg(stateResponse->getName()));

		QString name = stateResponse->getName();

		bool sendStatus = true;

		if (CUniteller::Host.contains(name, Qt::CaseInsensitive))
		{
			// Проверяем только железки
		}
		else if(CUniteller::CardReader.contains(name, Qt::CaseInsensitive))
		{
			mDeviceState.insert("cardreader", stateResponse->state());
		}
		else if (CUniteller::PinPad.contains(name, Qt::CaseInsensitive))
		{
			mDeviceState.insert("pinpad", stateResponse->state());
		}
		else if (CUniteller::ContactlessReader.contains(name, Qt::CaseInsensitive))
		{
			if (mHaveContactlessReader)
			{
				mDeviceState.insert("contactlessreader", stateResponse->state());
			}
			else
			{
				sendStatus = false;
			}
		}
		else
		{
			mDeviceState.insert(name.toLower(), stateResponse->state());
		}

		if (sendStatus)
		{
			emit state(stateResponse->state(), stateResponse->getName(), stateResponse->isLast());
		}
		else if (stateResponse->isLast())
		{
			// если статус последний нужно перепослать любой
			emit state(mDeviceState.begin().value(), mDeviceState.begin().key(), true);
		}
	}

	return stateResponse;
}

//---------------------------------------------------------------------------
bool API::isInitialResponse(BaseResponsePtr aResponse)
{
	auto initialResponse = qSharedPointerDynamicCast<InitialResponse, BaseResponse>(aResponse);
	if (initialResponse)
	{
		toLog(LogLevel::Normal, "< Initial response: OK.");

		emit readyToCard();
	}

	return initialResponse;
}

//---------------------------------------------------------------------------
bool API::isDeviceEventResponse(BaseResponsePtr aResponse)
{
	auto deviceEventResponse = qSharedPointerDynamicCast<DeviceEventResponse, BaseResponse>(aResponse);
	if (deviceEventResponse)
	{
		toLog(LogLevel::Normal, QString("< Device event: %1 '%2'.")
			.arg(toString(deviceEventResponse->event())).arg(deviceEventResponse->keyCode()));

		emit deviceEvent(deviceEventResponse->event(), deviceEventResponse->key());
	}

	return deviceEventResponse;
}

//---------------------------------------------------------------------------
bool API::isBreakResponse(BaseResponsePtr aResponse)
{
	auto breakResponse = qSharedPointerDynamicCast<BreakResponse, BaseResponse>(aResponse);
	if (breakResponse)
	{
		toLog(breakResponse->isComplete() ? LogLevel::Normal : LogLevel::Error, 
			QString("< Break response: %1.").arg(breakResponse->isComplete() ? "OK" : "DENY"));

		// TODO - надо как-то реагировать на невозможность отмены транзакции!
		if (breakResponse->isComplete())
		{
			emit breakComplete();
		}
	}

	return breakResponse;
}

//---------------------------------------------------------------------------
bool API::isPINRequiredResponse(BaseResponsePtr aResponse)
{
	auto pinRequiredResponse = qSharedPointerDynamicCast<PINRequiredResponse, BaseResponse>(aResponse);
	if (pinRequiredResponse)
	{
		toLog(LogLevel::Normal, "< PIN required.");

		emit pinRequired();
	}

	return pinRequiredResponse;
}

//---------------------------------------------------------------------------
bool API::isOnlineRequiredResponse(BaseResponsePtr aResponse)
{
	auto onlineRequiredResponse = qSharedPointerDynamicCast<OnlineRequiredResponse, BaseResponse>(aResponse);
	if (onlineRequiredResponse)
	{
		toLog(LogLevel::Normal, "< Online required.");

		emit onlineRequired();
	}

	return onlineRequiredResponse;
}

//---------------------------------------------------------------------------
bool API::isAuthResponse(BaseResponsePtr aResponse)
{
	auto authResponse = qSharedPointerDynamicCast<AuthResponse, BaseResponse>(aResponse);
	if (authResponse)
	{
		toLog(LogLevel::Normal, QString("< Auth response. %1.").arg(authResponse->toString()));

		if (authResponse->isOK())
		{
			emit sellComplete(authResponse->mTransactionSumm / 100.0, 
				authResponse->mCurrency, 
				authResponse->mRRN, 
				authResponse->mConfirmation);
		}
		else
		{
			emit error(authResponse->mMessage);
		}
	}

	return authResponse;
}

//---------------------------------------------------------------------------
void API::onReadyRead()
{
	QByteArray responseBuffer = mSocket.readAll();
	
	QList<std::function<bool(API &, BaseResponsePtr)>> responseHandlers;
	responseHandlers
		<< std::mem_fun_ref(&API::isErrorResponse)
		<< std::mem_fun_ref(&API::isLoginResponse)
		<< std::mem_fun_ref(&API::isPrintLineResponse)
		<< std::mem_fun_ref(&API::isGetStateResponse)
		<< std::mem_fun_ref(&API::isInitialResponse)
		<< std::mem_fun_ref(&API::isDeviceEventResponse)
		<< std::mem_fun_ref(&API::isBreakResponse)
		<< std::mem_fun_ref(&API::isPINRequiredResponse)
		<< std::mem_fun_ref(&API::isOnlineRequiredResponse)
		<< std::mem_fun_ref(&API::isAuthResponse);

	while (!responseBuffer.isEmpty())
	{
		BaseResponsePtr response = BaseResponse::createResponse(responseBuffer);

		if (!response->isValid())
		{
			toLog(LogLevel::Error, QString("Receive unknown packet from EFTPOS. Class='%1' Code='%2'").arg(response->mClass).arg(response->mCode));
		}
		else
		{
			foreach (auto handler, responseHandlers)
			{
				if (handler(*this, response))
				{
					break;
				}
			}
		}
	}
}

//---------------------------------------------------------------------------
QByteArray API::makeRequest(char aClass, char aCode, const QByteArray & aData)
{
	QByteArray request;

	request.append(aClass);
	request.append(aCode);
	request.append(mTerminalID.toLatin1());
	QString dataLength = QString::number(aData.size(), 16);
	if (dataLength.size() == 1)
	{
		dataLength.insert(0, '0');
	}
	request.append(dataLength.toLatin1());
	request.append(aData);

	return request;
}

//---------------------------------------------------------------------------
void API::send(const QByteArray & aRequest)
{
	if (!mSocket.isOpen())
	{
		toLog(LogLevel::Error, "Error write. Socket not connected.");
	}
	else
	{
		toLog(LogLevel::Debug, QString("Send command: %1").arg(QString::fromLatin1(aRequest)));

		if (mSocket.write(aRequest) != aRequest.size())
		{
			toLog(LogLevel::Error, QString("Error write data to socket. Error: %1").arg(mSocket.errorString()));
		}
	}
}

} // Uniteller

//---------------------------------------------------------------------------
