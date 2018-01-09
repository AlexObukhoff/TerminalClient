/* @file Реализация криптодвижока на libipriv. */

//boost
#include <boost/optional.hpp>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtGlobal>
#include <QtCore/QTime>
#include <QtCore/QString>
#include <QtCore/QDir>
#include <QtCore/QCoreApplication>
#include <QtCore/QTextCodec>
#include <QtCore/QMutexLocker>
#include <Common/QtHeadersEnd.h>

// Project
#include "CryptEngine.h"

//---------------------------------------------------------------------------
namespace CCryptEngine
{
#ifdef TC_USE_MD5
	#pragma message("CryptEngine use IPRIV_ALG_MD5!") 
	const int IprivHashAlg = IPRIV_ALG_MD5;
#else
	const int IprivHashAlg = IPRIV_ALG_SHA256;
#endif

	const int DecryptMaxBufferSize = 4096; // Максимальный размер для расшифрования данных.
	const int KeyExportBufferSize = 4096; // Размер памяти для экспорта рубличного ключа.
	const int KeySize = 2048;

	ICryptEngine & instance()
	{
		static CryptEngine cryptEngine;
		return cryptEngine;
	}
}

//---------------------------------------------------------------------------
CryptEngine::CryptEngine()
	: mMutex(QMutex::Recursive),
	  mInitialized(false),
	  mEngine(CCrypt::ETypeEngine::File)
{
}

//---------------------------------------------------------------------------
CryptEngine::~CryptEngine()
{
}

//---------------------------------------------------------------------------
bool CryptEngine::initialize()
{
	if (!mInitialized)
	{
		if (CRYPT_IS_SUCCESS(Crypt_Initialize()))
		{
			Crypt_SetHashAlg(CCryptEngine::IprivHashAlg);
			mInitialized = true;

			return true;
		}
		else
		{
			return false;
		}
	}

	return true;
}

//---------------------------------------------------------------------------
void CryptEngine::shutdown()
{
	releaseKeyPairs();

	if (mInitialized)
	{
		Crypt_Done();
		mInitialized = false;
	}
}

//---------------------------------------------------------------------------
QSet<CCrypt::ETypeEngine> CryptEngine::availableEngines()
{
	QMutexLocker locker(&mMutex);

	QSet<CCrypt::ETypeEngine> result;

	if (CRYPT_IS_SUCCESS(Crypt_Ctrl_Null(IPRIV_ENGINE_RSAREF, IPRIV_ENGCMD_IS_READY)))
	{
		result << CCrypt::ETypeEngine::File;
	}

#ifdef TC_USE_TOKEN
	// Движок инициализирован и в наличии хотя бы один ключ
	if (CRYPT_IS_SUCCESS(Crypt_Ctrl_Null(IPRIV_ENGINE_PKCS11_RUTOKEN, IPRIV_ENGCMD_IS_READY)) &&
		Crypt_Ctrl_Null(IPRIV_ENGINE_PKCS11_RUTOKEN, IPRIV_ENGCMD_GET_PKCS11_SLOTS_NUM) > 0)
	{
		// Сразу инициализируем ключ
		Crypt_Ctrl_Int(IPRIV_ENGINE_PKCS11_RUTOKEN, IPRIV_ENGCMD_SET_PKCS11_SLOT, 0);
		Crypt_Ctrl_String(IPRIV_ENGINE_PKCS11_RUTOKEN, IPRIV_ENGCMD_SET_PIN, getRootPassword()[0].data());

		result << CCrypt::ETypeEngine::RuToken;
	}
#endif

	return result;
}

//---------------------------------------------------------------------------
CCrypt::TokenStatus CryptEngine::getTokenStatus(CCrypt::ETypeEngine aEngine)
{
	QMutexLocker locker(&mMutex);

	CCrypt::TokenStatus result;

	result.available = false;
	result.initialized = false;

	if (aEngine != CCrypt::ETypeEngine::RuToken)
	{
		return result;
	}

	result.available = availableEngines().contains(aEngine);
	result.initialized = false;

	if (result.available)
	{
		char tmp[80] = { 0 };
		int len = Crypt_Ctrl(aEngine, IPRIV_ENGCMD_GET_PKCS11_SLOT_NAME, 0, tmp, sizeof(tmp) - 1);
		if (len > 0)
		{
			result.name = QString::fromLocal8Bit(tmp);
		}

		IPRIV_KEY keys[16];

		result.initialized = CRYPT_IS_SUCCESS(Crypt_Ctrl(aEngine, IPRIV_ENGCMD_ENUM_PKCS11_KEYS, keys, sizeof(keys) / sizeof(keys[0])));
	}

	return result;
}

//---------------------------------------------------------------------------
bool CryptEngine::initializeToken(CCrypt::ETypeEngine aEngine)
{
#ifdef TC_USE_TOKEN
	QMutexLocker locker(&mMutex);

	Crypt_Ctrl_Int(aEngine, IPRIV_ENGCMD_SET_PKCS11_SLOT, 0);
	Crypt_Ctrl_String(aEngine, IPRIV_ENGCMD_SET_PIN, getRootPassword()[0].data());

	return CRYPT_IS_SUCCESS(Crypt_Ctrl_Int(aEngine, IPRIV_ENGCMD_PKCS11_FORMAT_SLOT, 0));
#else
	return false;
#endif
}

//---------------------------------------------------------------------------
bool CryptEngine::createKeyPair(const QString & aPath, const QString & aKeyPairName, const QString & aUserId, const QString & aPassword,
	const ulong aSerialNumber, QString & aErrorDescription)
{
	QMutexLocker locker(&mMutex);

	IPRIV_KEY secretKey, publicKey;

	QString secretPath = aPath + "/" + aKeyPairName + "s.key";
	QString publicPath = aPath + "/" + aKeyPairName + "p.key";

	int res = Crypt_GenKey2(CCrypt::ETypeEngine::File, aSerialNumber, aUserId.toLatin1().data(), &secretKey, &publicKey, CCryptEngine::KeySize);

	if (res)
	{
		aErrorDescription = errorToString(res);

		return false;
	}

	res = Crypt_ExportSecretKeyToFile(secretPath.toLocal8Bit(), aPassword.toLatin1(), &secretKey);
	if (res <= 0)
	{
		aErrorDescription = errorToString(res);

		return false;
	}

	res = Crypt_ExportPublicKeyToFile(publicPath.toLocal8Bit(), &publicKey, 0);
	if (res <= 0)
	{
		aErrorDescription = errorToString(res);

		return false;
	}

	Crypt_CloseKey(&secretKey);
	Crypt_CloseKey(&publicKey);

	return true;
}

//---------------------------------------------------------------------------
QByteArray CryptEngine::generatePassword() const
{
	QTime time(QTime::currentTime());
	qsrand(unsigned(time.hour() + time.minute() + time.second() + time.msec()));
	
	QByteArray phrase;

	while (phrase.size() < 32)
	{
		phrase.append(QByteArray::number(qrand()));
	}

	phrase.resize(32);

	return phrase;
}

//---------------------------------------------------------------------------
bool CryptEngine::exportSecretKey(int aKeyPair, QByteArray & aSecretKey, const QByteArray & aPassword)
{
	QMutexLocker locker(&mMutex);

	if (!mKeyPairs.contains(aKeyPair))
	{
		return false;
	}

	aSecretKey.resize(CCryptEngine::KeyExportBufferSize);

	TKeyPair pair = mKeyPairs[aKeyPair];

	int result = Crypt_ExportSecretKey(aSecretKey.data(), aSecretKey.size(), aPassword.data(), &pair.first);
	if (result <= 0)
	{
		return false;
	}

	aSecretKey.resize(result);

	return true;
}

//---------------------------------------------------------------------------
bool CryptEngine::exportSecretKeyToFile(int aKeyPair, const QString & aFilePath, const QByteArray & aPassword)
{
	QByteArray keyData;

	if (!exportSecretKey(aKeyPair, keyData, aPassword))
	{
		return false;
	}

	QFile keyFile(aFilePath);

	if (!keyFile.open(QIODevice::ReadWrite | QIODevice::Append))
	{
		return false;
	}

	keyFile.write(keyData);
	keyFile.close();

	return true;
}

//---------------------------------------------------------------------------
bool CryptEngine::exportPublicKey(int aKeyPair, QByteArray & aPublicKey, ulong & aSerialNumber)
{
	QMutexLocker locker(&mMutex);

	if (!mKeyPairs.contains(aKeyPair))
	{
		return false;
	}

	aPublicKey.resize(CCryptEngine::KeyExportBufferSize);

	TKeyPair pair = mKeyPairs[aKeyPair];

	int result = Crypt_ExportPublicKey(aPublicKey.data(), aPublicKey.size(), &pair.second, &pair.first);
	if (result <= 0)
	{
		return false;
	}

	aPublicKey.resize(result);
	aSerialNumber = pair.second.keyserial;

	return true;
}

//---------------------------------------------------------------------------
bool CryptEngine::exportPublicKeyToFile(int aKeyPair, const QString & aFilePath, ulong & aSerialNumber)
{
	QByteArray keyData;

	if (!exportPublicKey(aKeyPair, keyData, aSerialNumber))
	{
		return false;
	}

	QFile keyFile(aFilePath);

	if (!keyFile.open(QIODevice::ReadWrite | QIODevice::Append))
	{
		return false;
	}

	if (keyFile.size())
	{
		keyFile.write("\r\n");
	}

	keyFile.write(keyData);
	keyFile.close();

	return true;
}

//---------------------------------------------------------------------------
bool CryptEngine::replacePublicKey(int aKeyPair, const QByteArray & aPublicKey)
{
	QMutexLocker locker(&mMutex);

	// Проверяем есть ли такая пара.
	if (!mKeyPairs.contains(aKeyPair))
	{
		return false;
	}

	IPRIV_KEY key;

	int result = Crypt_OpenPublicKey(mEngine, aPublicKey.data(), aPublicKey.size(), 0, &key, 0);
	if (result != 0)
	{
		return false;
	}

	Crypt_CloseKey(&mKeyPairs[aKeyPair].second);

	mKeyPairs[aKeyPair].second = key;

	return true;
}

//---------------------------------------------------------------------------
bool CryptEngine::createKeyPair(int aKeyPair, CCrypt::ETypeEngine aEngine, const QByteArray & aKeyCard, int aKeySize, QString & aErrorDescription)
{
	CCrypt::ETypeEngine primaryEngine = CCrypt::ETypeEngine::File;

#ifdef TC_USE_TOKEN
	primaryEngine = CCrypt::ETypeEngine::RuToken;
#endif

	if (aKeyPair >= 0 && aEngine != primaryEngine)
	{
		aErrorDescription = errorToString(CRYPT_ERR_INVALID_ENG);

		return false;
	}

	QMutexLocker locker(&mMutex);

	boost::optional<TKeyPair> oldPair;

	// Если уже загружена эта пара, то выгружаем и перетираем.
	if (mKeyPairs.contains(aKeyPair))
	{
		oldPair = mKeyPairs.take(aKeyPair);
	}

	if (aEngine == CCrypt::ETypeEngine::RuToken)
	{
		Crypt_Ctrl_Int(aEngine, IPRIV_ENGCMD_SET_PKCS11_SLOT, 0);
		Crypt_Ctrl_String(aEngine, IPRIV_ENGCMD_SET_PIN, getRootPassword()[0].data());
	}

	TKeyPair pair;

	int result = Crypt_GenKey(aEngine, aKeyCard.data(), aKeyCard.size(), &pair.first, &pair.second, aKeySize);
	if (result != 0)
	{
		aErrorDescription = errorToString(result);

		if (oldPair.is_initialized())
		{
			mKeyPairs.insert(aKeyPair, oldPair.get());
		}

		return false;
	}
	else
	{
		if (oldPair.is_initialized())
		{
			Crypt_CloseKey(&oldPair->first);
			Crypt_CloseKey(&oldPair->second);
		}
	}

	mKeyPairs[aKeyPair] = pair;

	return true;
}

//---------------------------------------------------------------------------
bool CryptEngine::loadKeyPair(int aKeyPair, CCrypt::ETypeEngine aEngine, const QString & aSecretKeyPath, const QString & aPassword,
		const QString & aPublicKeyPath, const ulong aSerialNumber, const ulong aBankSerialNumber, QString & aErrorDescription)
{
#ifdef TC_USE_TOKEN
	if (aKeyPair >= 0 && aEngine != CCrypt::ETypeEngine::RuToken)
	{
		aErrorDescription = errorToString(CRYPT_ERR_INVALID_ENG);

		return false;
	}
#endif

	QMutexLocker locker(&mMutex);

	if (mKeyPairs.contains(aKeyPair))
	{
		// Пара ключей с таким номером уже загружена, выгружаем.
		releaseKeyPair(aKeyPair);
	}

	TKeyPair keyPair;
	int res = 0;

	switch (aEngine)
	{
	case CCrypt::ETypeEngine::File:
		res = ::Crypt_OpenSecretKeyFromFile(aEngine, aSecretKeyPath.toLocal8Bit().data(), aPassword.toLatin1().data(), &keyPair.first);
		break;

	case CCrypt::ETypeEngine::RuToken:
		res = ::Crypt_Ctrl(aEngine, IPRIV_ENGCMD_SET_PIN, getRootPassword()[0].data());

		if (CRYPT_IS_SUCCESS(res)) 
		{
			res = ::Crypt_OpenSecretKeyFromStore(aEngine, aSerialNumber, &keyPair.first);
		}
		break;
	}

	if (CRYPT_IS_ERROR(res))
	{
		aErrorDescription = errorToString(res);

		return false;
	}

	res = ::Crypt_OpenPublicKeyFromFile(CCrypt::ETypeEngine::File, aPublicKeyPath.toLocal8Bit(), aBankSerialNumber, &keyPair.second, 0);

	if (res != 0)
	{
		aErrorDescription = errorToString(res);

		// Уже успели загрузить закрытый ключ, освобождаем
		::Crypt_CloseKey(&keyPair.first);

		return false;
	}

	mKeyPairs[aKeyPair] = keyPair;

	return true;
}

//---------------------------------------------------------------------------
QString CryptEngine::getKeyPairSerialNumber(int aKeyPair)
{
	QMutexLocker locker(&mMutex);

	TKeyPairList::iterator keyPair = mKeyPairs.find(aKeyPair);
	if (keyPair == mKeyPairs.end())
	{
		return QString();
	}

#ifdef TC_USE_MD5
	return QString::number(keyPair.value().second.keyserial);
#else
	return QString::number(keyPair.value().second.keyserial) + "-sha256";
#endif
}

//---------------------------------------------------------------------------
bool CryptEngine::releaseKeyPair(int aKeyPair)
{
	QMutexLocker locker(&mMutex);

	TKeyPairList::iterator it = mKeyPairs.find(aKeyPair);

	if (it != mKeyPairs.end())
	{
		Crypt_CloseKey(&it.value().first);
		Crypt_CloseKey(&it.value().second);

		mKeyPairs.erase(it);

		return true;
	}
	else
	{
		return false;
	}
}

//---------------------------------------------------------------------------
void CryptEngine::releaseKeyPairs()
{
	QMutexLocker locker(&mMutex);

	while (!mKeyPairs.isEmpty())
	{
		releaseKeyPair(mKeyPairs.keys()[0]);
	}
}

//---------------------------------------------------------------------------
bool CryptEngine::sign(int aKeyPair, const QByteArray & aRequest , QByteArray & aSignature, QString & aErrorDescription)
{
	QMutexLocker locker(&mMutex);

	TKeyPairList::iterator keyPair = mKeyPairs.find(aKeyPair);
	if (keyPair == mKeyPairs.end())
	{
		aErrorDescription = "key pair not found";

		return false;
	}

	aSignature.resize(aRequest.size() + 2048);

	int res = ::Crypt_SignEx(aRequest.data(), aRequest.size(), aSignature.data(), aSignature.size(), &keyPair.value().first, CCryptEngine::IprivHashAlg);

	if (res <= 0)
	{
		aErrorDescription = errorToString(res);

		return false;
	}
	else
	{
		aSignature.resize(res);
		
		return true;
	}
}

//---------------------------------------------------------------------------
bool CryptEngine::signDetach(int aKeyPair, const QByteArray & aRequest, QByteArray & aSignature, QString & aErrorDescription)
{
	QMutexLocker locker(&mMutex);

	TKeyPairList::iterator keyPair = mKeyPairs.find(aKeyPair);
	if (keyPair == mKeyPairs.end())
	{
		aErrorDescription = "key pair not found";

		return false;
	}

	aSignature.resize(4096);

	int res = ::Crypt_Sign2Ex(aRequest.data(), aRequest.size(), aSignature.data(), aSignature.size(), &keyPair.value().first, CCryptEngine::IprivHashAlg);

	if (res <= 0)
	{
		aErrorDescription = errorToString(res);

		return false;
	}
	else
	{
		aSignature.resize(res);
		
		return true;
	}
}

//---------------------------------------------------------------------------
bool CryptEngine::verify(int aKeyPair, const QByteArray & aResponseString, QByteArray & aOriginalResponse, QString & aErrorDescription)
{
	QMutexLocker locker(&mMutex);

	TKeyPairList::iterator keyPair = mKeyPairs.find(aKeyPair);
	if (keyPair == mKeyPairs.end())
	{
		aErrorDescription = "key pair not found";

		return false;
	}

	const char * originalResponseData = 0;
	int originalResponseDataSize = 0;

	int res = ::Crypt_Verify(aResponseString.data(), aResponseString.size(), &originalResponseData, &originalResponseDataSize, &keyPair.value().second);

	if (res == 0)
	{
		aOriginalResponse = QByteArray(originalResponseData, originalResponseDataSize);

		return true;
	}
	else
	{
		aErrorDescription = errorToString(res);

		return false;
	}
}

//---------------------------------------------------------------------------
bool CryptEngine::verifyDetach(int aKeyPair, const QByteArray & aResponseString, const QByteArray & aSignature, QString & aErrorDescription)
{
	QMutexLocker locker(&mMutex);

	TKeyPairList::iterator keyPair = mKeyPairs.find(aKeyPair);
	if (keyPair == mKeyPairs.end())
	{
		aErrorDescription = "key pair not found";

		return false;
	}

	int res = ::Crypt_Verify3(aResponseString.data(), aResponseString.size(), aSignature.data(), aSignature.size(), &keyPair.value().second);

	if (res == 0)
	{
		return true;
	}
	else
	{
		aErrorDescription = errorToString(res);

		return false;
	}
}

//---------------------------------------------------------------------------
bool CryptEngine::encrypt(int aKeyPair, const QByteArray & aData, QByteArray & aResult, CCrypt::ETypeKey aKey, QString & aErrorDescription)
{
	QMutexLocker locker(&mMutex);

	TKeyPairList::iterator keyPair = mKeyPairs.find(aKeyPair);
	if (keyPair == mKeyPairs.end())
	{
		aErrorDescription = "key pair not found";

		return false;
	}

	aResult.resize(CCryptEngine::DecryptMaxBufferSize);

	IPRIV_KEY key = (aKey == CCrypt::ETypeKey::Public) ? keyPair.value().second : keyPair.value().first;

	int res = ::Crypt_Encrypt(aData.data(), aData.size(), aResult.data(), aResult.size(), &key);

	if (res <= 0)
	{
		aErrorDescription = errorToString(res);

		return false;
	}
	else
	{
		aResult.resize(res);

		return true;
	}
}

//---------------------------------------------------------------------------
bool CryptEngine::encrypt(int aKeyPair, const QByteArray & aData, QByteArray & aResult, QString & aErrorDescription)
{
	return encrypt(aKeyPair, aData, aResult, CCrypt::ETypeKey::Public, aErrorDescription);
}

//---------------------------------------------------------------------------
bool CryptEngine::encryptLong(int aKeyPair, const QByteArray & aData, QByteArray & aResult, CCrypt::ETypeKey aKey, QString & aErrorDescription)
{
	QMutexLocker locker(&mMutex);

	TKeyPairList::iterator keyPair = mKeyPairs.find(aKeyPair);
	if (keyPair == mKeyPairs.end())
	{
		aErrorDescription = "key pair not found";

		return false;
	}

	aResult.resize((aData.size() + 1024) * 2);

	IPRIV_KEY key = (aKey == CCrypt::ETypeKey::Public) ? keyPair.value().second : keyPair.value().first;

	int res = ::Crypt_EncryptLong(aData.data(), aData.size(), aResult.data(), aResult.size(), &key);

	if (res <= 0)
	{
		aErrorDescription = errorToString(res);

		return false;
	}
	else
	{
		aResult.resize(res);

		return true;
	}
}

//---------------------------------------------------------------------------
bool CryptEngine::encryptLong(int aKeyPair, const QByteArray & aData, QByteArray & aResult, QString & aErrorDescription)
{
	return encryptLong(aKeyPair, aData, aResult, CCrypt::ETypeKey::Public, aErrorDescription);
}

//---------------------------------------------------------------------------
bool CryptEngine::decrypt(int aKeyPair, const QByteArray & aData, QByteArray & aResult, CCrypt::ETypeKey aKey, QString & aErrorDescription)
{
	QMutexLocker locker(&mMutex);

	TKeyPairList::iterator keyPair = mKeyPairs.find(aKeyPair);
	if (keyPair == mKeyPairs.end())
	{
		aErrorDescription = "key pair not found";

		return false;
	}

	aResult.resize(CCryptEngine::DecryptMaxBufferSize);

	IPRIV_KEY key = (aKey == CCrypt::ETypeKey::Public) ? keyPair.value().second : keyPair.value().first;

	int res = ::Crypt_Decrypt(aData.data(), aData.size(), aResult.data(), aResult.size(), &key);

	if (res <= 0)
	{
		aErrorDescription = errorToString(res);

		return false;
	}
	else
	{
		aResult.resize(res);

		return true;
	}
}

//---------------------------------------------------------------------------
bool CryptEngine::decrypt(int aKeyPair, const QByteArray & aData, QByteArray & aResult, QString & aErrorDescription)
{
	return decrypt(aKeyPair, aData, aResult, CCrypt::ETypeKey::Private, aErrorDescription);
}

//---------------------------------------------------------------------------
bool CryptEngine::decryptLong(int aKeyPair, const QByteArray & aData, QByteArray & aResult, CCrypt::ETypeKey aKey, QString & aErrorDescription)
{
	QMutexLocker locker(&mMutex);

	TKeyPairList::iterator keyPair = mKeyPairs.find(aKeyPair);
	if (keyPair == mKeyPairs.end())
	{
		aErrorDescription = "key pair not found";

		return false;
	}

	aResult.resize((aData.size() + 1024) * 2);

	IPRIV_KEY key = (aKey == CCrypt::ETypeKey::Public) ? keyPair.value().second : keyPair.value().first;

	int res = ::Crypt_DecryptLong(aData.data(), aData.size(), aResult.data(), aResult.size(), &key);

	if (res <= 0)
	{
		aErrorDescription = errorToString(res);

		return false;
	}
	else
	{
		aResult.resize(res);

		return true;
	}
}

//---------------------------------------------------------------------------
bool CryptEngine::decryptLong(int aKeyPair, const QByteArray & aData, QByteArray & aResult, QString & aErrorDescription)
{
	return decryptLong(aKeyPair, aData, aResult, CCrypt::ETypeKey::Private, aErrorDescription);
}

//---------------------------------------------------------------------------
bool CryptEngine::setData(const QString & aName, const QByteArray & aData)
{
	if (getTokenStatus(CCrypt::ETypeEngine::RuToken).isOK())
	{
		return CRYPT_IS_SUCCESS(Crypt_Ctrl(IPRIV_ENGINE_PKCS11_RUTOKEN, IPRIV_ENGCMD_PKCS11_SET_DATA, 
			aName.toLatin1().data(), aData.data(), aData.size()));
	}

	return false;
}

//---------------------------------------------------------------------------
bool CryptEngine::getData(const QString & aName, QByteArray & aData)
{
	if (getTokenStatus(CCrypt::ETypeEngine::RuToken).isOK())
	{
		int size = 0, count = 1;
		aData.clear();

		do
		{
			aData.resize(CCryptEngine::DecryptMaxBufferSize * (count++));

			size = Crypt_Ctrl(IPRIV_ENGINE_PKCS11_RUTOKEN, IPRIV_ENGCMD_PKCS11_GET_DATA, aName.toLatin1().data(), aData.data(), aData.size());
		} while (size == CRYPT_ERR_OUT_OF_MEMORY);

		aData.resize(size > 0 ? size : 0);

		return CRYPT_IS_SUCCESS(size);
	}

	return false;
}

//---------------------------------------------------------------------------
QString CryptEngine::errorToString(int aError) const
{
	switch (aError)
	{
		case CRYPT_ERR_BAD_ARGS: return QString("wrong arguments (%1)").arg(aError);
		case CRYPT_ERR_OUT_OF_MEMORY: return QString("bad memory allocation (%1)").arg(aError);
		case CRYPT_ERR_INVALID_FORMAT: return QString("bad document format (%1)").arg(aError);
		case CRYPT_ERR_NO_DATA_FOUND: return QString("unexpected end of document (%1)").arg(aError);
		case CRYPT_ERR_INVALID_PACKET_FORMAT: return QString("wrong document structure (%1)").arg(aError);
		case CRYPT_ERR_UNKNOWN_ALG: return QString("unknown algorithm (%1)").arg(aError);
		case CRYPT_ERR_INVALID_KEYLEN: return QString("key length and document length are different (%1)").arg(aError);
		case CRYPT_ERR_INVALID_PASSWD: return QString("wrong secret password (%1)").arg(aError);
		case CRYPT_ERR_DOCTYPE: return QString("wrong document type (%1)").arg(aError);
		case CRYPT_ERR_RADIX_DECODE: return QString("failed to ASCII decode (%1)").arg(aError);
		case CRYPT_ERR_RADIX_ENCODE: return QString("failed to ASCII encode (%1)").arg(aError);
		case CRYPT_ERR_INVALID_ENG: return QString("unknown provider (%1)").arg(aError);
		case CRYPT_ERR_ENG_NOT_READY: return QString("provider is not ready (%1)").arg(aError);
		case CRYPT_ERR_NOT_SUPPORT: return QString("function is not suppoerted (%1)").arg(aError);
		case CRYPT_ERR_FILE_NOT_FOUND: return QString("file not found (%1)").arg(aError);
		case CRYPT_ERR_CANT_READ_FILE: return QString("failed to read file (%1)").arg(aError);
		case CRYPT_ERR_INVALID_KEY: return QString("invalid key (%1)").arg(aError);
		case CRYPT_ERR_SEC_ENC: return QString("failed to build sign (%1)").arg(aError);
		case CRYPT_ERR_PUB_KEY_NOT_FOUND: return QString("public key not found (%1)").arg(aError);
		case CRYPT_ERR_VERIFY: return QString("failed to verify document (%1)").arg(aError);
		case CRYPT_ERR_CREATE_FILE: return QString("failed to create faile (%1)").arg(aError);
		case CRYPT_ERR_CANT_WRITE_FILE: return QString("failed to write file (%1)").arg(aError);
		case CRYPT_ERR_INVALID_KEYCARD: return QString("wrong keycard format (%1)").arg(aError);
		case CRYPT_ERR_GENKEY: return QString("failed to generate key (%1)").arg(aError);
		case CRYPT_ERR_PUB_ENC: return QString("encode error (%1)").arg(aError);
		case CRYPT_ERR_SEC_DEC: return QString("decode error (%1)").arg(aError);
		case CRYPT_ERR_UNKNOWN_SENDER: return QString("unknown sender (%1)").arg(aError);

		default: return QString("unknown error (%1)").arg(aError);
	}
}

//---------------------------------------------------------------------------
