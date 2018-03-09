/* @file Класс для обмена сообщениями по http. */

// STL
#include <functional>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTextCodec>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ScopedPointerLaterDeleter.h>

#include <Crypt/ICryptEngine.h>

#include <NetworkTaskManager/NetworkTaskManager.h>
#include <NetworkTaskManager/MemoryDataStream.h>

// Project
#include "Request.h"
#include "Response.h"
#include "RequestSender.h"

using namespace std::placeholders;

namespace SDK {
namespace PaymentProcessor {
namespace CyberPlat {

//---------------------------------------------------------------------------
namespace CRequestSender
{
	const int DefaultKeyPair = 0;
}


//---------------------------------------------------------------------------
RequestSender::RequestSender(NetworkTaskManager * aNetwork, ICryptEngine * aCryptEngine)
	: mNetwork(aNetwork),
	  mCryptEngine(aCryptEngine),
	  mKeyPair(CRequestSender::DefaultKeyPair),
	  mOnlySecureConnection(true)
{
#if defined(_DEBUG) || defined(DEBUG_INFO)
	mOnlySecureConnection = false;
#endif // _DEBUG || DEBUG_INFO

	setResponseCreator(std::bind(&RequestSender::defaultResponseCreator, this, _1, _2));
	setRequestEncoder(std::bind(&RequestSender::defaultRequestEncoder, this, _1, _2));
	setResponseDecoder(std::bind(&RequestSender::defaultResponseDecoder, this, _1, _2));
	setRequestSigner(std::bind(&RequestSender::defaultRequestSigner, this, _1, _2, _3, _4));
	setResponseVerifier(std::bind(&RequestSender::defaultResposneVerifier, this, _1, _2, _3, _4));
	setRequestModifier(std::bind(&RequestSender::defaultRequestModifier, this, _1, _2, _3));
}

//---------------------------------------------------------------------------
RequestSender::~RequestSender()
{
}

//---------------------------------------------------------------------------
void RequestSender::setNetworkTaskManager(NetworkTaskManager * aNetwork)
{
	mNetwork = aNetwork;
}

//---------------------------------------------------------------------------
void RequestSender::setResponseCreator(const TResponseCreator & aResponseCreator)
{
	mResponseCreator = aResponseCreator;
}

//---------------------------------------------------------------------------
void RequestSender::setRequestEncoder(const TRequestEncoder & aRequestEncoder)
{
	mRequestEncoder = aRequestEncoder;
}

//---------------------------------------------------------------------------
void RequestSender::setResponseDecoder(const TResponseDecoder & aResponseDecoder)
{
	mResponseDecoder = aResponseDecoder;
}

//---------------------------------------------------------------------------
void RequestSender::setRequestSigner(const TRequestSigner & aRequestSigner)
{
	mRequestSigner = aRequestSigner;
}

//---------------------------------------------------------------------------
void RequestSender::setResponseVerifier(const TResponseVerifier & aResponseVerifier)
{
	mResponseVerifier = aResponseVerifier;
}

//---------------------------------------------------------------------------
void RequestSender::setRequestModifier(const TRequestModifier & aRequestModifier)
{
	mRequestModifier = aRequestModifier;
}

//---------------------------------------------------------------------------
void RequestSender::setCryptKeyPair(int aKeyPair)
{
	mKeyPair = aKeyPair;
}

//---------------------------------------------------------------------------
void RequestSender::setOnlySecureConnectionEnabled(bool aOnlySecure)
{
	mOnlySecureConnection = aOnlySecure;
}

//---------------------------------------------------------------------------
Response * RequestSender::request(NetworkTask::Type aType, const QUrl & aUrl, Request & aRequest, ESignatureType aSignatureType, ESendError & aError, int aTimeout)
{
	if (aUrl.scheme().toLower() == "http" && mOnlySecureConnection)
	{
		aError = HttpIsNotSupported;

		return 0;
	}

	aError = Ok;

	if (mNetwork.isNull())
	{
		aError = NoNetworkInterfaceSpecified;

		return 0;
	}

	// Подставляем серийный номер пары ключа в каждый запрос.
	QString keyPairSerial = mCryptEngine->getKeyPairSerialNumber(mKeyPair);

	if (keyPairSerial.isEmpty())
	{
		aError = ClientCryptError;

		return 0;
	}

	if (aSignatureType == Solid)
	{
		aRequest.addParameter("ACCEPT_KEYS", keyPairSerial);
	}

	QScopedPointer<NetworkTask, ScopedPointerLaterDeleter<NetworkTask>> task(new NetworkTask());

	task->setUrl(aUrl);
	task->setType(aType);
	task->setTimeout(aTimeout);
	task->setDataStream(new MemoryDataStream());
	task->getRequestHeader().insert("Content-Type", "application/x-www-form-urlencoded");

	QByteArray encodedRequest;
	QByteArray signedRequest;
	QByteArray detachedSignature;

	if (!mRequestEncoder(aRequest.toString(), std::ref(encodedRequest)))
	{
		aError = EncodeError;

		return 0;
	}

	if (!mRequestSigner(encodedRequest, std::ref(signedRequest), aSignatureType, std::ref(detachedSignature)))
	{
		aError = ClientCryptError;

		return 0;
	}

	if (aSignatureType == Solid)
	{
		signedRequest = "inputmessage=" + signedRequest;

		auto rawParameters = aRequest.getParameters(true);
		foreach (auto name, rawParameters.keys())
		{
			signedRequest += "\n" + name + "=" + rawParameters.value(name).toString();
		}
	}
	else if (aSignatureType == Detached)
	{
		task->getRequestHeader().insert("X-signature", detachedSignature);
		task->getRequestHeader().insert("X-cyberplat-accepted-keys", keyPairSerial.toLatin1());
	}
	else
	{
		aError = UnknownSignatureType;

		return 0;
	}

	if (!mRequestModifier(aRequest, std::ref(task->getRequestHeader()), std::ref(signedRequest)))
	{
		aError = RequestModifyError;

		return 0;
	}

	task->getDataStream()->write(signedRequest);

	mNetwork->addTask(task.data());

	task->waitForFinished();

	if (task->getError() != NetworkTask::NoError)
	{
		aError = NetworkError;

		return 0;
	}

	QByteArray signedResponseData = task->getDataStream()->takeAll();

	// Проверим на запакованные данные
	if (task->getResponseHeader()["Content-Type"] == "application/x-gzip")
	{
		signedResponseData = qUncompress((const uchar *)signedResponseData.data(), signedResponseData.size());
	}

	QByteArray encodedResponseData;
	QByteArray responseSignature;
	QString responseData;

	if (aSignatureType == Detached)
	{
		responseSignature = QByteArray::fromPercentEncoding(task->getResponseHeader()["X-signature"]);
	}

	if (!mResponseVerifier(signedResponseData, std::ref(encodedResponseData), aSignatureType, responseSignature))
	{
		aError = ServerCryptError;

		return 0;
	}

	if (!mResponseDecoder(encodedResponseData, std::ref(responseData)))
	{
		aError = DecodeError;

		return 0;
	}

	return mResponseCreator(aRequest, responseData);
}

//---------------------------------------------------------------------------
Response * RequestSender::get(const QUrl & aUrl, Request & aRequest, ESignatureType aSignatureType, ESendError & aError, int aTimeout)
{
	return request(NetworkTask::Get, aUrl, aRequest, aSignatureType, aError, aTimeout);
}

//---------------------------------------------------------------------------
Response * RequestSender::post(const QUrl & aUrl, Request & aRequest, ESignatureType aSignatureType, ESendError & aError, int aTimeout)
{
	return request(NetworkTask::Post, aUrl, aRequest, aSignatureType, aError, aTimeout);
}

//---------------------------------------------------------------------------
Response * RequestSender::defaultResponseCreator(const Request & aRequest, const QString & aData)
{
	return new Response(aRequest, aData);
}

//---------------------------------------------------------------------------
bool RequestSender::defaultRequestEncoder(const QString & aRequest, QByteArray & aEncodedRequest)
{
	QTextCodec * codec = QTextCodec::codecForName("Windows-1251");
	if (!codec)
	{
		return false;
	}

	aEncodedRequest = codec->fromUnicode(aRequest);

	return true;
}

//---------------------------------------------------------------------------
bool RequestSender::defaultResponseDecoder(const QByteArray & aResponse, QString & aDecodedResponse)
{
	QTextCodec * codec = QTextCodec::codecForName("Windows-1251");
	if (!codec)
	{
		return false;
	}

	aDecodedResponse = codec->toUnicode(aResponse);

	return true;
}

//---------------------------------------------------------------------------
bool RequestSender::defaultRequestSigner(const QByteArray & aRequest, QByteArray & aSignedRequest, ESignatureType aSignatureType, QByteArray & aSignature)
{
	QString error;

	switch (aSignatureType)
	{
		case Solid:
		{
			if (mCryptEngine->sign(mKeyPair, aRequest, aSignedRequest, error))
			{
				aSignedRequest = aSignedRequest.toPercentEncoding();

				return true;
			}
			else
			{
				return false;
			}
		}

		case Detached:
		{
			if (mCryptEngine->signDetach(mKeyPair, aRequest, aSignature, error))
			{
				aSignedRequest = aRequest;
				aSignature = aSignature.toPercentEncoding();

				return true;
			}
			else
			{
				return false;
			}
		}

		default: return false;
	}
}

//---------------------------------------------------------------------------
bool RequestSender::defaultResposneVerifier(const QByteArray & aSignedResponse, QByteArray & aResponse, ESignatureType aSignatureType, const QByteArray & aSignature)
{
	QString error;

	switch (aSignatureType)
	{
		case Solid: return mCryptEngine->verify(mKeyPair, aSignedResponse, aResponse, error);
		case Detached:
		{
			if (mCryptEngine->verifyDetach(mKeyPair, aSignedResponse, aSignature, error))
			{
				aResponse = aSignedResponse;

				return true;
			}
			else
			{
				return false;
			}
		}

		default: return false;
	}
}

//---------------------------------------------------------------------------
bool RequestSender::defaultRequestModifier(Request & /*aRequest*/, NetworkTask::TByteMap & /*aHeaders*/, QByteArray & /*aEncodedAndSignedData*/)
{
	return true;
}

//---------------------------------------------------------------------------
QString RequestSender::translateError(ESendError aError)
{
	switch (aError)
	{
		case Ok: return "ok";
		case NetworkError: return "network error";
		case EncodeError: return "request encode error";
		case DecodeError: return "response decode error";
		case ClientCryptError: return "request sign error";
		case ServerCryptError: return "response verify error";
		case UnknownSignatureType: return "signature type is not supported";
		case RequestModifyError: return "request modify error";
		case NoNetworkInterfaceSpecified: return "no network interface specified";
		case HttpIsNotSupported: return "only secure connection supported";

		default: return "unknown error";
	}
}

//---------------------------------------------------------------------------
}}} // SDK::PaymentProcessor::CyberPlat
