/* @file Набор функций для создания и регистрации ключей на сервере. */

// Qt headers
#include "Common/QtHeadersBegin.h"
#include <QtCore/QUrlQuery>
#include "Common/QtHeadersEnd.h"

// Modules
#include <Common/ScopedPointerLaterDeleter.h>

#include <Crypt/ICryptEngine.h>

#include <NetworkTaskManager/NetworkTaskManager.h>
#include <NetworkTaskManager/NetworkTask.h>
#include <NetworkTaskManager/MemoryDataStream.h>

// Project
#include "KeysUtils.h"

//---------------------------------------------------------------------------
namespace CKeysFactory
{
	const int KeySize = 2048;

	namespace ClientFields
	{
		const char User[]       = "UserID";
		const char Password[]   = "Password";
		const char Key[]        = "key";
		const char Phrase[]     = "phrase";
		const char Query[]      = "query";
		const char AcceptKeys[] = "accept_keys";
	}

	namespace ServerFields
	{
		const char AP[]        = "AP";
		const char SD[]        = "SD";
		const char OP[]        = "OP";
		const char KeyCard[]   = "KeyCard";
		const char Error[]     = "Error";
		const char PublicKey[] = "PublicKey";
	}

	namespace ServerErrors
	{
		const char NoError[]       = "0 OK";
		const char WrongPassword[] = "2 Authentification error";
	}

	namespace Queries
	{
		const char GetCard[]     = "getcard";
		const char RegisterKey[] = "putpublickey";
	}
}

//---------------------------------------------------------------------------
EKeysUtilsError::Enum createKeyPair(ICryptEngine * aCrypt, int aKeyNumber, NetworkTaskManager * aNetwork, const QUrl & aUrl,
                  const QString & aLogin, const QString & aPassword, SKeyPair & aPair)
{
	QUrl url(aUrl);
	QUrlQuery urlQuery;

	urlQuery.addQueryItem(CKeysFactory::ClientFields::User, aLogin);
	urlQuery.addQueryItem(CKeysFactory::ClientFields::Password, aPassword);
	urlQuery.addQueryItem(CKeysFactory::ClientFields::Query, CKeysFactory::Queries::GetCard);
	urlQuery.addQueryItem(CKeysFactory::ClientFields::AcceptKeys, QString::number(1));
	url.setQuery(urlQuery);

	QScopedPointer<NetworkTask, ScopedPointerLaterDeleter<NetworkTask>> task(new NetworkTask());

	task->setUrl(url);
	task->setType(NetworkTask::Get);
	task->setDataStream(new MemoryDataStream());

	aNetwork->addTask(task.data());

	task->waitForFinished();

	if (task->getError() != NetworkTask::NoError)
	{
		// Получили сетевую ошибку.
		task->deleteLater();

		return EKeysUtilsError::NetworkError;
	}

	// Если разница локального времени с серверным больше 10 мин., то ключи созданы не будут.
	QDateTime serverDate = task->getServerDate();
	if (serverDate.isValid())
	{
		int maxDeltaSec = 600;
		QDateTime localDate = QDateTime::currentDateTime();

		if (qAbs(serverDate.secsTo(localDate)) > maxDeltaSec)
		{
			qDebug() << QString("Local date and server date don't match. Sytem date: %1. Server date: %2.")
				.arg(localDate.toLocalTime().toString("yyyy.MM.dd hh:mm:ss"))
				.arg(serverDate.toLocalTime().toString("yyyy.MM.dd hh:mm:ss"));

			return EKeysUtilsError::WrongLocalTime;
		}
	}

	// Карточка для генерации ключа
	QByteArray keyCard;

	// Информация о ключе
	SKeyPair pair;

	// Парсим ответ сервера
	QByteArray response = task->getDataStream()->takeAll();

	QList<QByteArray> dataList = response.split('&');
	foreach (const QByteArray & data, dataList)
	{
		QList<QByteArray> parameter = data.split('=');
		if (parameter.size() < 2)
		{
			continue;
		}

		QByteArray name = parameter.first();
		QByteArray value = QByteArray::fromPercentEncoding(parameter.last());

		if (name == CKeysFactory::ServerFields::AP)
		{
			pair.ap = QByteArray::fromBase64(value);
		}
		else if (name == CKeysFactory::ServerFields::SD)
		{
			pair.sd = QByteArray::fromBase64(value);
		}
		else if (name == CKeysFactory::ServerFields::OP)
		{
			pair.op = QByteArray::fromBase64(value);
		}
		else if (name == CKeysFactory::ServerFields::KeyCard)
		{
			keyCard = QByteArray::fromBase64(value);
		}
		else if (name == CKeysFactory::ServerFields::Error)
		{
			if (value == CKeysFactory::ServerErrors::WrongPassword)
			{
				return EKeysUtilsError::WrongPassword;
			}
			else if (value != CKeysFactory::ServerErrors::NoError)
			{
				return EKeysUtilsError::UnknownServerError;
			}
		}
	}

	// Проверим всё ли получили
	if (pair.ap.isEmpty() || pair.sd.isEmpty() || pair.op.isEmpty() || keyCard.isEmpty())
	{
		return EKeysUtilsError::WrongServerAnswer;
	}

	// Выбираем подходящий движок ключа
#ifdef TC_USE_TOKEN
	pair.engine = CCrypt::ETypeEngine::RuToken;
#else
	pair.engine = CCrypt::ETypeEngine::File;
#endif

	QString error;

	if (!aCrypt->createKeyPair(aKeyNumber, static_cast<CCrypt::ETypeEngine>(pair.engine), keyCard, CKeysFactory::KeySize, error))
	{
		return EKeysUtilsError::KeyPairCreateError;
	}

	pair.id = aKeyNumber;

	aPair = pair;

	return EKeysUtilsError::Ok;
}

//---------------------------------------------------------------------------
EKeysUtilsError::Enum registerKeyPair(ICryptEngine * aCrypt, int aKeyNumber, NetworkTaskManager * aNetwork, const QUrl & aUrl,
                    const QString & aLogin, const QString & aPassword, SKeyPair & aPair)
{
	QByteArray publicKey;
	ulong serialNumber;

	if (!aCrypt->exportPublicKey(aKeyNumber, publicKey, serialNumber))
	{
		return EKeysUtilsError::KeyPairCreateError;
	}

	QByteArray request = QByteArray(CKeysFactory::ClientFields::User) + "=" + aLogin.toUtf8() + "&" +
		CKeysFactory::ClientFields::Password + "=" + aPassword.toUtf8() + "&" +
		CKeysFactory::ClientFields::Query + "=" + CKeysFactory::Queries::RegisterKey + "&" +
		CKeysFactory::ClientFields::Phrase + "=" + QString::number(serialNumber).toLatin1() + "&" +
		CKeysFactory::ClientFields::AcceptKeys + "=" + QString::number(CKeysFactory::KeySize).toLatin1() + "&" +
		CKeysFactory::ClientFields::Key + "=" + publicKey.toPercentEncoding();

	QScopedPointer<NetworkTask, ScopedPointerLaterDeleter<NetworkTask>> task(new NetworkTask());

	task->setUrl(aUrl);
	task->setType(NetworkTask::Post);
	task->setDataStream(new MemoryDataStream());
	task->getDataStream()->write(request);
	task->getRequestHeader().insert("content-type", "application/x-www-form-urlencoded");

	aNetwork->addTask(task.data());

	task->waitForFinished();

	if (task->getError() != NetworkTask::NoError)
	{
		// Получили сетевую ошибку.
		task->deleteLater();

		return EKeysUtilsError::NetworkError;
	}

	// Парсим ответ сервера
	QByteArray response = task->getDataStream()->takeAll();

	bool publicKeyLoaded(false);

	QList<QByteArray> dataList = response.split('&');
	foreach (const QByteArray & data, dataList)
	{
		QList<QByteArray> parameter = data.split('=');
		if (parameter.size() < 2)
		{
			continue;
		}

		QByteArray name = parameter.first();
		QByteArray value = QByteArray::fromPercentEncoding(parameter.last());

		if (name == CKeysFactory::ServerFields::Error)
		{
			if (value == CKeysFactory::ServerErrors::WrongPassword)
			{
				return EKeysUtilsError::WrongPassword;
			}
			else if (value == CKeysFactory::ServerErrors::NoError)
			{
				continue;
			}
		}
		else if (name == CKeysFactory::ServerFields::PublicKey)
		{
			aPair.serverPublicKey = QByteArray::fromPercentEncoding(value);

			publicKeyLoaded = true;
		}
	}

	return publicKeyLoaded ? EKeysUtilsError::Ok : EKeysUtilsError::UnknownServerError;
}

//---------------------------------------------------------------------------
QString EKeysUtilsError::errorToString(Enum aError)
{
	switch(aError)
	{
		case Ok:                 return "Ok";
		case NetworkError:       return "NetworkError";
		case WrongPassword:      return "WrongPassword";
		case WrongServerAnswer:  return "WrongServerAnswer";
		case WrongLocalTime:     return "WrongLocalTime";
		case UnknownServerError: return "UnknownServerError";
		case KeyPairCreateError: return "KeyPairCreateError";
		case KeyExportError:     return "KeyExportError";
		default:                 return "UnknownError";
	}
}

//---------------------------------------------------------------------------