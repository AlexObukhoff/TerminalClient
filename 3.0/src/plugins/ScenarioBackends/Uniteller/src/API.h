/* Реализация протокола с EFTPOS 3.0 компании Uniteller. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QSharedPointer>
#include <QtNetwork/QTcpSocket>
#include <QtCore/QTextCodec>
#include <QtCore/QTimer>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

// Modules
#include <Common/ILogable.h>

// Project headers
#include "Uniteller.h"


class NetworkTaskManager;

//---------------------------------------------------------------------------
namespace Uniteller
{
	const quint16 DefaultPort = 10400;
	const int SocketTimeout = 5000;
	const char LogName[] = "Uniteller";
}

//---------------------------------------------------------------------------
namespace Uniteller
{
class BaseResponse;
typedef QSharedPointer<BaseResponse> BaseResponsePtr;

//---------------------------------------------------------------------------
/// API для работы с сервисом 
class API : public QObject, public ILogable
{
	Q_OBJECT

	API(ILog * aLog, SDK::PaymentProcessor::ICore * aCore, quint16 aPort = Uniteller::DefaultPort);
public:

	static QSharedPointer<API> getInstance(ILog * aLog, SDK::PaymentProcessor::ICore * aCore, quint16 aPort = Uniteller::DefaultPort);

	void setTerminalID(const QString & aTerminalID) { mTerminalID = aTerminalID; }

	bool isReady() const;

	/// Возвращает последнюю ошибку
	int getLastError() const { return mLastError; }

	void setupRuntimePath(const QString & aRuntimePath);

public slots:
	/// Подсоединиться к EFTPOS
	void enable();

	/// Отсоединиться
	void disable();

	/// EFTPOS запросы
	void login();
	void sell(double aAmount, const QString & aOrderID, const QString & aCategory);
	void breakSell();
	void getState();

signals:
	void ready();
	void readyToCard();
	void error(const QString & aMessage);
	void breakComplete();
	void pinRequired();
	void onlineRequired();
	void showMessage(const QString & aMessage);
	void print(const QStringList & aReceipt);
	void state(int aState, const QString & aDeviceName, bool aLast);
	void deviceEvent(Uniteller::DeviceEvent::Enum aEvent, Uniteller::KeyCode::Enum aKeyCode);
	void sellComplete(double aAmount, int aCurrency, const QString & aRRN, const QString & aConfirmationCode);
	void changeState(bool aReady);

private slots:
	void doConnect();
	void onConnected();
	void onDisconnected();
	void onError(QAbstractSocket::SocketError aSocketError);
	void onReadyRead();
	void onCheckStateTimeout();

private:
	QByteArray makeRequest(char aClass, char aCode, const QByteArray & aData = QByteArray());
	void send(const QByteArray & aRequest);
	QString translateErrorMessage(const QString & aError, const QString & aMessage) const;

private:
	/// Обработчики ответов сервера
	bool isErrorResponse(BaseResponsePtr aResponse);
	bool isLoginResponse(BaseResponsePtr aResponse);
	bool isPrintLineResponse(BaseResponsePtr aResponse);
	bool isGetStateResponse(BaseResponsePtr aResponse);
	bool isInitialResponse(BaseResponsePtr aResponse);
	bool isDeviceEventResponse(BaseResponsePtr aResponse);
	bool isBreakResponse(BaseResponsePtr aResponse);
	bool isPINRequiredResponse(BaseResponsePtr aResponse);
	bool isOnlineRequiredResponse(BaseResponsePtr aResponse);
	bool isAuthResponse(BaseResponsePtr aResponse);

	virtual void timerEvent(QTimerEvent * aEvent);

private:
	quint16 mPort;
	QTcpSocket mSocket;
	SDK::PaymentProcessor::TerminalSettings * mTerminalSettings;
	QString mTerminalID;
	bool mLoggedIn;
	
	QStringList mCurrentReceipt;

private:
	bool mEnabled;
	bool mHaveContactlessReader;
	int mLastError;
	QString mLastErrorString;
	int mGetStateTimerID;
	QMap<QString, int> mDeviceState; // Объединенное состояние для всех устройств
	QTimer mCheckStateTimer;
	int mLoginCheckTimer;
	bool mLastReadyState;
	QString mRuntimePath;
};

} // PlatezhRu
