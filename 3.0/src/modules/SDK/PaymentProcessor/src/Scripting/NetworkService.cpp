/* @file Прокси класс для работы с сетью. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFuture>
#include <QtCore/QUrl>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Crypt/ICryptEngine.h>
#include <NetworkTaskManager/NetworkTaskManager.h>
#include <NetworkTaskManager/NetworkTask.h>
#include <NetworkTaskManager/MemoryDataStream.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/INetworkService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Core/ICryptService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/PaymentProcessor/Scripting/NetworkService.h>
#include <SDK/PaymentProcessor/Core/IPrinterService.h>
#include <SDK/PaymentProcessor/Core/IPaymentService.h>
#include <SDK/PaymentProcessor/CyberPlat/RequestSender.h>

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
namespace CRequest
{
	const QString SD = "SD";
	const QString AP = "AP";
	const QString OP = "OP";
	const QString SESSION = "SESSION";
}

//------------------------------------------------------------------------------
Request::Request(const QVariantMap & aRequestParameters)
{
	foreach(QString key, aRequestParameters.keys())
	{
		addParameter(key, aRequestParameters.value(key).toString());
	}
}

//------------------------------------------------------------------------------
Response::Response(const SDK::PaymentProcessor::CyberPlat::Request & aRequest, const QString & aResponseString)
	: SDK::PaymentProcessor::CyberPlat::Response(aRequest, aResponseString)
{
}

//------------------------------------------------------------------------------
NetworkService::NetworkService(ICore * aCore)
	: mCore(aCore),
	  mCurrentTask(0),
	  mNetworkService(aCore->getNetworkService())
{
	if (mNetworkService)
	{
		mTaskManager = mNetworkService->getNetworkTaskManager();
		mRequestSender = QSharedPointer<PPSDK::CyberPlat::RequestSender>(new PPSDK::CyberPlat::RequestSender(mTaskManager, mCore->getCryptService()->getCryptEngine()));
		mRequestSender->setResponseCreator(std::bind(&NetworkService::createResponse, this, std::placeholders::_1, std::placeholders::_2));

		connect(&mResponseWatcher, SIGNAL(finished()), SLOT(onResponseFinished()));
		connect(&mResponseSendReceiptWatcher, SIGNAL(finished()), SLOT(onResponseSendReceiptFinished()));
	}

	mCore->getEventService()->subscribe(this, SLOT(onEvent(const SDK::PaymentProcessor::Event &)));

	mSD = static_cast<PPSDK::TerminalSettings *>
		(mCore->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter))->getKeys()[0].sd;

	mAP = static_cast<PPSDK::TerminalSettings *>
		(mCore->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter))->getKeys()[0].ap;

	mOP = static_cast<PPSDK::TerminalSettings *>
		(mCore->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter))->getKeys()[0].op;
}

//------------------------------------------------------------------------------
NetworkService::~NetworkService()
{
	if (mTaskManager && mCurrentTask)
	{
		disconnect(mCurrentTask, SIGNAL(onComplete()), this, SLOT(taskComplete()));

		mTaskManager->removeTask(mCurrentTask);
		mCurrentTask->deleteLater();

		mCurrentTask = 0;
	}
}

//------------------------------------------------------------------------------
void NetworkService::onEvent(const SDK::PaymentProcessor::Event & aEvent)
{
	switch(aEvent.getType())
	{
	case SDK::PaymentProcessor::EEventType::ConnectionEstablished:
	case SDK::PaymentProcessor::EEventType::ConnectionLost:
		emit connectionStatus();
		break;
	}
}

//------------------------------------------------------------------------------
bool NetworkService::isConnected()
{
	return mNetworkService->isConnected(true);
}

//------------------------------------------------------------------------------
int NetworkService::get(const QString & /*aUrl*/, const QString & /*aData*/)
{
	// TODO:

	return 0;
}

//------------------------------------------------------------------------------
bool NetworkService::post(const QString & aUrl, const QString & aData)
{
	if (mTaskManager)
	{
		if (mCurrentTask)
		{
			disconnect(mCurrentTask, SIGNAL(onComplete()), this, SLOT(taskComplete()));

			mTaskManager->removeTask(mCurrentTask);
			mCurrentTask->deleteLater();

			mCurrentTask = 0;
		}

		QScopedPointer<NetworkTask> task(new NetworkTask());

		task->setUrl(aUrl);
		task->setType(NetworkTask::Post);
		task->setDataStream(new MemoryDataStream());
		task->getDataStream()->write(aData.toUtf8());

		connect(task.data(), SIGNAL(onComplete()), SLOT(taskComplete()));

		mTaskManager->addTask(mCurrentTask = task.take());

		return true;
	}

	return false;
}

//------------------------------------------------------------------------------
void NetworkService::taskComplete()
{
	if (mCurrentTask && (mCurrentTask->getError() == NetworkTask::NoError))
	{
		emit complete(false, QString::fromUtf8(mCurrentTask->getDataStream()->takeAll()));

		mCurrentTask->deleteLater();
	}
	else
	{
		emit complete(true, QString());
	}

	mCurrentTask = 0;
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::CyberPlat::Response * NetworkService::createResponse(const SDK::PaymentProcessor::CyberPlat::Request & aRequest, const QString & aData)
{
	return new Response(aRequest, aData);
}

//------------------------------------------------------------------------------
void NetworkService::sendRequest(const QString & aUrl, QVariantMap aParameters)
{
	if (mRequestSender.isNull())
	{
		//TODO
		return;
	}

	mResponseWatcher.setFuture(QtConcurrent::run(
		this, &SDK::PaymentProcessor::Scripting::NetworkService::postRequest, aUrl, aParameters));
}

//------------------------------------------------------------------------------
Response * NetworkService::sendRequestInternal(Request * aRequest, const QString & aUrl)
{
	PPSDK::CyberPlat::RequestSender::ESendError error;

	QScopedPointer<SDK::PaymentProcessor::CyberPlat::Response> response(mRequestSender->post(aUrl, *aRequest, PPSDK::CyberPlat::RequestSender::Solid, error));

	delete aRequest;
	aRequest = nullptr;

	if (response.isNull())
	{
		return nullptr;
	}

	if (!dynamic_cast<Response *>(response.data()))
	{
		return nullptr;
	}

	return dynamic_cast<Response *>(response.take());
}

//------------------------------------------------------------------------------
void NetworkService::sendReceipt(const QString & aEmail, const QString & aContact)
{
	QVariantMap params;
	params.insert("PAYER_EMAIL", aEmail);
	params.insert("CONTACT", aContact);
	params.insert("PAYER_EMAIL_MESSAGE", mCore->getPrinterService()->loadReceipt(mCore->getPaymentService()->getActivePayment()).toLocal8Bit().toPercentEncoding());

	PPSDK::TerminalSettings * terminalSettings = static_cast<PPSDK::TerminalSettings *>(mCore->getSettingsService()->
		getAdapter(PPSDK::CAdapterNames::TerminalAdapter));

	Request * request = new Request(params);

	mResponseSendReceiptWatcher.setFuture(QtConcurrent::run(
		this, &SDK::PaymentProcessor::Scripting::NetworkService::sendRequestInternal, request, terminalSettings->getReceiptMailURL()));
}

//------------------------------------------------------------------------------
SDK::PaymentProcessor::CyberPlat::Response * NetworkService::postRequest(const QString & aUrl, QVariantMap & aRequestParameters)
{
	//Предохранимся
	foreach(QString key, aRequestParameters.keys())
	{
		if (key.toLower() == "amount" || key.toLower() == "amount_all")
		{
			aRequestParameters.remove(key);
		}
	}

	aRequestParameters.insert(CRequest::SD, mSD);
	aRequestParameters.insert(CRequest::AP, mAP);
	aRequestParameters.insert(CRequest::OP, mOP);
	aRequestParameters.insert(CRequest::SESSION, QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz")
		+ QString("%1").arg(qrand() % 1000, 3, 10, QChar('0')));

	Request request(aRequestParameters);
	
	PPSDK::CyberPlat::RequestSender::ESendError e;

	return mRequestSender->post(QUrl(aUrl), request, PPSDK::CyberPlat::RequestSender::Solid, e);
}

//------------------------------------------------------------------------------
void NetworkService::onResponseFinished()
{
	QScopedPointer<SDK::PaymentProcessor::CyberPlat::Response> response(mResponseWatcher.result());
	emit requestCompleted(response.isNull() ? QVariantMap() : dynamic_cast<Response *>(response.take())->getParameters());
}

//------------------------------------------------------------------------------
void NetworkService::onResponseSendReceiptFinished()
{
	QScopedPointer<SDK::PaymentProcessor::CyberPlat::Response> response(mResponseSendReceiptWatcher.result());
	emit sendReceiptComplete(!response.isNull() && response->isOk());
}

//------------------------------------------------------------------------------

}}} // Scripting::PaymentProcessor::SDK
