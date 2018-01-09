/* @file Прокси класс для работы с сетью. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QDebug>
#include <QtCore/QSharedPointer>
#include <QtCore/QFutureWatcher>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/CyberPlat/Request.h>
#include <SDK/PaymentProcessor/CyberPlat/Response.h>
#include <SDK/PaymentProcessor/CyberPlat/RequestSender.h>

class NetworkTaskManager;
class NetworkTask;

namespace SDK {
namespace PaymentProcessor {

class ICore;
class INetworkService;

namespace Scripting {

//---------------------------------------------------------------------------
class Request : public SDK::PaymentProcessor::CyberPlat::Request
{
public:
	Request(const QVariantMap & aRequestParameters);
};

//---------------------------------------------------------------------------
class Response : public SDK::PaymentProcessor::CyberPlat::Response
{
public:
	Response(const SDK::PaymentProcessor::CyberPlat::Request & aRequest, const QString & aResponseString);
};

//------------------------------------------------------------------------------
class NetworkService : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool connected READ isConnected NOTIFY connectionStatus)

public:
	NetworkService(ICore * aCore);
	~NetworkService();

public slots:
	/// Выполнение запроса GET на адрес aUrl с данными aData.
	int get(const QString & aUrl, const QString & aData);

	/// Выполнение запроса POST на адрес aUrl с данными aData.
	bool post(const QString & aUrl, const QString & aData);

	/// Получение статуса соединения.
	bool isConnected();

public slots:
	void sendRequest(const QString & aUrl, QVariantMap aParameters);
	void sendReceipt(const QString & aEmail, const QString & aContact);

private slots:
	/// Сетевой запрос завершился.
	void taskComplete();
	
	/// Получения событий ядра
	void onEvent(const SDK::PaymentProcessor::Event & aEvent);

	void onResponseFinished();

	void onResponseSendReceiptFinished();

signals:
	/// Срабатывает при завершении запроса aRequest.
	void complete(bool aError, QString aResult);

	/// Срабатывает при получении сигнала о состоянии соединения.
	void connectionStatus();

	void requestCompleted(const QVariantMap & aResult);

	void sendReceiptComplete(bool aResult);

private:
	/// Создаёт ответ по классу запроса и данным.
	SDK::PaymentProcessor::CyberPlat::Response * createResponse(const SDK::PaymentProcessor::CyberPlat::Request & aRequest, const QString & aData);

	SDK::PaymentProcessor::CyberPlat::Response * postRequest(const QString & aUrl, QVariantMap & aRequestParameters);

	Response * sendRequestInternal(Request * aRequest, const QString & aUrl);

private:
	ICore * mCore;
	INetworkService * mNetworkService;
	NetworkTaskManager * mTaskManager;
	NetworkTask * mCurrentTask;

private:
	QString mSD;
	QString mAP;
	QString mOP;
	QSharedPointer<SDK::PaymentProcessor::CyberPlat::RequestSender> mRequestSender;
	QFutureWatcher<SDK::PaymentProcessor::CyberPlat::Response *> mResponseWatcher;
	QFutureWatcher<Response *> mResponseSendReceiptWatcher;
};

//------------------------------------------------------------------------------
}}} // Scripting::PaymentProcessor::SDK
