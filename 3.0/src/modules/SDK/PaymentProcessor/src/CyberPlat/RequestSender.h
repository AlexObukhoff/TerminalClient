/* @file Класс для обмена сообщениями по http. */

#pragma once

// STL
#include <functional>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QDateTime>
#include <QtCore/QWeakPointer>
#include <Common/QtHeadersEnd.h>

// Modules
#include <NetworkTaskManager/NetworkTask.h>

class ICryptEngine;

namespace SDK {
namespace PaymentProcessor {
namespace CyberPlat {

class Request;
class Response;

//---------------------------------------------------------------------------
class RequestSender : public QObject
{
	Q_OBJECT

public:
	/// Способ запроса к серверу.
	enum ERequestType
	{
		Get = 0,
		Post
	};

	/// Описание результатов отправки.
	enum ESendError
	{
		/// Ошибок нет.
		Ok = 0,
		/// Сетевая ошибка.
		NetworkError,
		/// Ошибка кодирования запроса.
		EncodeError,
		/// Ошибка декодирования ответа.
		DecodeError,
		/// Ошибка подписи на стороне клиента.
		ClientCryptError,
		/// Ошибка подписи на стороне сервера.
		ServerCryptError,
		/// Указан неизвестный способ подписи запроса.
		UnknownSignatureType,
		/// Ошибка при построении заголовка запроса.
		RequestModifyError,
		/// Не был указан сетевой интерфейс для передачи запроса.
		NoNetworkInterfaceSpecified,
		/// Поддерживается только HTTPS.
		HttpIsNotSupported
	};

	/// Способ подписывания запроса.
	enum ESignatureType
	{
		/// Запрос "оборачивается" подписью.
		Solid = 0,
		/// Подпись передаётся в http заголовке.
		Detached
	};

	/// Конструктор для создания специфичных классов ответа по запросу и данным ответа.
	typedef std::function<Response * (const Request &, const QString &)> TResponseCreator;

	/// Функция кодирующая данные запроса в подходящий формат.
	typedef std::function<bool (const QString &, QByteArray &)> TRequestEncoder;

	/// Функция декодирующая ответ сервера.
	typedef std::function<bool (const QByteArray &, QString &)> TResponseDecoder;

	/// Метод, подписывающий запрос.
	typedef std::function<bool (const QByteArray &, QByteArray &, ESignatureType, QByteArray &)> TRequestSigner;

	/// Метод, проверяющий подпись ответа и возвращающий данные, освобождённые от подписи.
	typedef std::function<bool (const QByteArray &, QByteArray &, ESignatureType, const QByteArray &)> TResponseVerifier;

	/// Метод, корректирующий значения из HTTP заголовка и тело запроса.
	typedef std::function<bool (Request &, NetworkTask::TByteMap &, QByteArray &)> TRequestModifier;

	/// Возвращает текстовое описание ошибки.
	static QString translateError(ESendError aError);

	RequestSender(NetworkTaskManager * aNetwork, ICryptEngine * aCryptEngine);
	~RequestSender();

	/// Устанавливает сетевой интерфейс для выполнения запросов.
	void setNetworkTaskManager(NetworkTaskManager * aNetwork);

	/// Устанавливает конструктор для специфических ответов.
	void setResponseCreator(const TResponseCreator & aResponseCreator);

	/// Устанавливает кодировщик запросов.
	void setRequestEncoder(const TRequestEncoder & aRequestEncoder);

	/// Устанавливает декодер овтетов.
	void setResponseDecoder(const TResponseDecoder & aResponseDecoder);

	/// Устанавливает подписчика запросов.
	void setRequestSigner(const TRequestSigner & aRequestSigner);

	/// Устанавливает верифаера ответов.
	void setResponseVerifier(const TResponseVerifier & aResponseVerifier);

	/// Устанавливает модфикатор заголовков запроса.
	void setRequestModifier(const TRequestModifier & aRequestModifier);

	/// Устанавливает номер пары ключей, которой будет подписан запрос.
	void setCryptKeyPair(int aKeyPair);

	/// Активирует поддержку HTTP запросов. По умолчанию доступны только HTTPS.
	void setOnlySecureConnectionEnabled(bool aOnlySecure);

	/// Отправляет запрос методом GET на адрес aUrl.
	Response * get(const QUrl & aUrl, Request & aRequest, ESignatureType aSignatureType, ESendError & aError, int aTimeout = 60 * 1000);

	/// Отправляет запрос методом POST на адрес aUrl.
	Response * post(const QUrl & aUrl, Request & aRequest, ESignatureType aSignatureType, ESendError & aError, int aTimeout = 60 * 1000);

protected:
	/// Отправляет запрос методом aType на адрес aUrl.
	Response * request(NetworkTask::Type aType, const QUrl & aUrl, Request & aRequest, ESignatureType aSignatureType, ESendError & aError, int aTimeout);

	/// Конструктор ответов по умолчанию.
	Response * defaultResponseCreator(const Request & aRequest, const QString & aData);

	/// Кодировщик запросов по умолчанию.
	bool defaultRequestEncoder(const QString & aRequest, QByteArray & aEncodedRequest);

	/// Кодировщик запросов по умолчанию.
	bool defaultResponseDecoder(const QByteArray & aResponse, QString & aDecodedResponse);

	/// Метод, подписывающий запросы по умолчанию.
	bool defaultRequestSigner(const QByteArray & aRequest, QByteArray & aSignedRequest, ESignatureType aSignatureType, QByteArray & aSiganture);

	/// Метод, производящий последние преобразования над заголовками и телом HTTP запроса перед отправкой.
	bool defaultRequestModifier(Request & aRequest, NetworkTask::TByteMap & aHeaders, QByteArray & aEncodedAndSignedData);

	/// Метод, проверяющий запросы по умолчанию.
	bool defaultResposneVerifier(const QByteArray & aSignedResponse, QByteArray & aResponse, ESignatureType aSignatureType, const QByteArray & aSignature = QByteArray());

private:
	TResponseCreator mResponseCreator;
	TRequestEncoder mRequestEncoder;
	TResponseDecoder mResponseDecoder;
	TRequestSigner mRequestSigner;
	TResponseVerifier mResponseVerifier;
	TRequestModifier mRequestModifier;

	int mKeyPair;

	QWeakPointer<NetworkTaskManager> mNetwork;
	ICryptEngine * mCryptEngine;

	bool mOnlySecureConnection;
};

//------------------------------------------------------------------------------
}}} // SDK::PaymentProcessor::CyberPlat
