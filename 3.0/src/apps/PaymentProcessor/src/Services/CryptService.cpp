/* @file Сервис, владеющий крипто-движком. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QByteArray>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/INetworkService.h>

// Modules
#include <Crypt/ICryptEngine.h>
#include <System/IApplication.h>
#include <Common/ILog.h>

// Project
#include "Services/ServiceNames.h"
#include "Services/SettingsService.h"
#include "Services/CryptService.h"

namespace PP = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
namespace CServiceCore
{
	const char DefaultPublicKey[] = "pubkeys.key";
	const char DefaultSecretKey[] = "secret.key";
}

//---------------------------------------------------------------------------
CryptService * CryptService::instance(IApplication * aApplication)
{
	return static_cast<CryptService *>(aApplication->getCore()->getService(CServices::CryptService));
}

//---------------------------------------------------------------------------
CryptService::CryptService(IApplication * aApplication)
	: mApplication(aApplication),
	  mLog(aApplication->getLog())
{
}

//---------------------------------------------------------------------------
CryptService::~CryptService()
{
}

//---------------------------------------------------------------------------
bool CryptService::initialize()
{
	ICryptEngine & crypt = CCryptEngine::instance();

	if (!crypt.initialize())
	{
		LOG(mLog, LogLevel::Error, "Failed to initialize CryptEngine.");
		return false;
	}

	// Подготавливаем к работе пары ключей.
	PP::TerminalSettings * settings = SettingsService::instance(mApplication)->getAdapter<PP::TerminalSettings>();

	QList<QByteArray> passwords = crypt.getRootPassword();

	if (passwords.isEmpty())
	{
		LOG(mLog, LogLevel::Error, "Failed to generate root key password.");
		return false;
	}

	// Создаем ключ терминала.
	PP::SKeySettings terminalKey;
	terminalKey.serialNumber = 12345678;
	terminalKey.publicKeyPath = mApplication->getUserDataPath().append("/keys/rootp.key");
	terminalKey.secretKeyPath = mApplication->getUserDataPath().append("/keys/roots.key");

	bool result = false;

	// Перебираем все возможные пароли для ключей терминала.
	foreach (const QByteArray & password, passwords)
	{
		terminalKey.secretPassword = QString::fromLatin1(password.data(), password.size());

		QString error;
		result = crypt.loadKeyPair(-1, CCrypt::ETypeEngine::File, terminalKey.secretKeyPath, terminalKey.secretPassword,
			terminalKey.publicKeyPath, terminalKey.serialNumber, terminalKey.serialNumber, error);

		if (result)
		{
			break;
		}
	}

	if (!result || !QFile::exists(terminalKey.publicKeyPath) || !QFile::exists(terminalKey.secretKeyPath))
	{
		// Ключ не подошёл или отсутвует, надо создать.
		QString keyPath = mApplication->getUserDataPath() + "/keys";

		QDir keysDir;
		QString error;

		if (!keysDir.mkpath(keyPath))
		{
			LOG(mLog, LogLevel::Error, QString("Failed to create key path: %1.").arg(keyPath));
			return false;
		}

		QString backupExt = QDateTime::currentDateTime().toString(".yyyy-MM-dd_hh-mm-ss") + "_backup";

		QFile::rename(terminalKey.secretKeyPath, terminalKey.secretKeyPath + backupExt);
		QFile::rename(terminalKey.publicKeyPath, terminalKey.publicKeyPath + backupExt);

		if (!crypt.createKeyPair(keyPath, "root", "CyberPlat", terminalKey.secretPassword, terminalKey.serialNumber, error))
		{
			LOG(mLog, LogLevel::Error, QString("Failed to create terminal key pair. Error: %1.").arg(error));
			return false;
		}

		LOG(mLog, LogLevel::Normal, "Terminal key pair created.");

		// Пробуем загрузить созданный ключ
		if (!crypt.loadKeyPair(-1, CCrypt::ETypeEngine::File, terminalKey.secretKeyPath, terminalKey.secretPassword,
			terminalKey.publicKeyPath, terminalKey.serialNumber, terminalKey.serialNumber, error))
		{
			LOG(mLog, LogLevel::Error, QString("Failed to load terminal key pair. Error: %1.").arg(error));
			return false;
		}
	}

	terminalKey.isValid = true;
	mKeys.insert(-1, terminalKey);

	// Загружаем остальные ключи.
	foreach (PP::SKeySettings key, settings->getKeys().values())
	{
		loadKey(key);
	}

	// Проверяем, что первый ключ был корректно загружен.
	if (mKeys.value(0).isValid)
	{
		// Пишем в лог информацию ключе по умолчанию.
		LOG(mLog, LogLevel::Normal, QString("Default terminal key: SD %1, AP %2, OP %3.")
			.arg(mKeys.value(0).sd).arg(mKeys.value(0).ap).arg(mKeys.value(0).op));
	}
	else
	{
		LOG(mLog, LogLevel::Error, "Terminal default key is not valid.");
	}

	return true;
}

//------------------------------------------------------------------------------
void CryptService::loadKey(SDK::PaymentProcessor::SKeySettings & aKey)
{
	QString error;
	ICryptEngine & crypt = CCryptEngine::instance();

	aKey.isValid = false;

	// Преобразовываем пути к ключам.
	QString secretKeyPath;

	if (!QDir::isAbsolutePath(aKey.secretKeyPath))
	{
		secretKeyPath = QDir::cleanPath(mApplication->getUserDataPath() + "/" + aKey.secretKeyPath);
	}
	else
	{
		secretKeyPath = QDir::cleanPath(aKey.secretKeyPath);
	}

	QString publicKeyPath;

	if (!QDir::isAbsolutePath(aKey.publicKeyPath))
	{
		publicKeyPath = QDir::cleanPath(mApplication->getUserDataPath() + "/" + aKey.publicKeyPath);
	}
	else
	{
		publicKeyPath = QDir::cleanPath(aKey.publicKeyPath);
	}

	// Пытаемся расшифровать кодовую фразу.
	QByteArray secretPassword = aKey.secretPassword.toLatin1();
	QByteArray decryptedSecretPassword;

	// Для Token не нужен шифрованный пароль
	if (aKey.engine == CCrypt::ETypeEngine::RuToken ||
		crypt.decrypt(-1, secretPassword, decryptedSecretPassword, error))
	{
		aKey.secretPassword = QString::fromLatin1(decryptedSecretPassword.data());

		if (!crypt.loadKeyPair(aKey.id, static_cast<CCrypt::ETypeEngine>(aKey.engine),
			secretKeyPath, aKey.secretPassword, publicKeyPath, aKey.serialNumber, aKey.bankSerialNumber, error))
		{
			LOG(mLog, LogLevel::Error, QString("Failed to load key pair %1 with encrypted password. Error: %2.").arg(aKey.id).arg(error));
		}
		else
		{
			aKey.isValid = true;
		}
	}
	else
	{
		// Пытаемся загрузить пару ключей без шифрования
		if (!crypt.loadKeyPair(aKey.id, static_cast<CCrypt::ETypeEngine>(aKey.engine),
			secretKeyPath, QString::fromLatin1(secretPassword.data()), publicKeyPath, aKey.serialNumber, aKey.bankSerialNumber, error))
		{
			LOG(mLog, LogLevel::Error, QString("Failed to load key pair %1 with unencrypted password. Error: %2.").arg(aKey.id).arg(error));
		}
		else
		{
			aKey.isValid = true;
			LOG(mLog, LogLevel::Warning, QString("Key pair %1 has unencrypted secret password.").arg(aKey.id));
		}
	}

	mKeys.insert(aKey.id, aKey);
}

//------------------------------------------------------------------------------
void CryptService::finishInitialize()
{
}

//---------------------------------------------------------------------------
bool CryptService::canShutdown()
{
	return true;
}

//---------------------------------------------------------------------------
bool CryptService::shutdown()
{
	CCryptEngine::instance().releaseKeyPairs();
	CCryptEngine::instance().shutdown();

	return true;
}

//---------------------------------------------------------------------------
QString CryptService::getName() const
{
	return CServices::CryptService;
}

//---------------------------------------------------------------------------
const QSet<QString> & CryptService::getRequiredServices() const
{
	static QSet<QString> requiredServices = QSet<QString>()
		<< CServices::SettingsService;

	return requiredServices;
}

//---------------------------------------------------------------------------
QVariantMap CryptService::getParameters() const
{
	return QVariantMap();
}

//---------------------------------------------------------------------------
void CryptService::resetParameters(const QSet<QString> &)
{
}

//---------------------------------------------------------------------------
PP::SKeySettings CryptService::getKey(int aId) const
{
	if (mKeys.contains(aId))
	{
		return mKeys[aId];
	}
	else
	{
		return PP::SKeySettings();
	}
}

//---------------------------------------------------------------------------
bool CryptService::addKey(const PP::SKeySettings & aKey)
{
	if (aKey.isValid)
	{
		// добавляем в список ключей
		mKeys.insert(aKey.id, aKey);

		PP::SKeySettings key = aKey;

		// Шифруем кодовую фразу
		QByteArray encryptedPhrase;

		QString error;

		if (aKey.engine == CCrypt::ETypeEngine::File)
		{
			if (!CCryptEngine::instance().encrypt(-1, aKey.secretPassword.toLatin1(), encryptedPhrase, CCrypt::ETypeKey::Public, error))
			{
				LOG(mLog, LogLevel::Error, QString("Failed to encrypt password phrase for key pair %1, serial = %2. Error: %3.")
					.arg(key.id).arg(key.bankSerialNumber).arg(error));
				return false;
			}

			key.secretPassword = QString::fromLatin1(encryptedPhrase);
		}

		// Добавляем запись в дерево настроек.
		PP::TerminalSettings * settings = SettingsService::instance(mApplication)->getAdapter<PP::TerminalSettings>();

		// Сериализуем настройки.
		settings->setKey(key);

		return true;
	}

	return false;
}

//---------------------------------------------------------------------------
int CryptService::generateKey(int aKeyId, const QString & aLogin, const QString & aPassword, const QString & aURL, QString & aSD, QString & aAP, QString & aOP)
{
	PP::INetworkService * networkService = mApplication->getCore()->getNetworkService();

	if (!networkService->isConnected() && !networkService->openConnection(true))
	{
		LOG(mLog, LogLevel::Error, QString("Key %1 generation failed. Unable to establish connection.").arg(aKeyId));
		return EKeysUtilsError::NetworkError;
	}

	SKeyPair keyPair;

	EKeysUtilsError::Enum result;
	if ((result = createKeyPair(getCryptEngine(), aKeyId, networkService->getNetworkTaskManager(), aURL, aLogin, aPassword, keyPair)) != EKeysUtilsError::Ok)
	{
		LOG(mLog, LogLevel::Error, QString("Failed to create key pair %1. Error is %2").arg(aKeyId).arg(EKeysUtilsError::errorToString(result)));
		return result;
	}

	mKeyPair = keyPair;

	if ((result = registerKeyPair(getCryptEngine(), aKeyId, networkService->getNetworkTaskManager(), aURL, aLogin, aPassword, mKeyPair)) != EKeysUtilsError::Ok)
	{
		LOG(mLog, LogLevel::Error, QString("Failed to register key pair %1. Error is %2").arg(aKeyId).arg(EKeysUtilsError::errorToString(result)));
		return result;
	}

	LOG(mLog, LogLevel::Normal, QString("Key pair %1 with SD %2, AP %3, OP %4 is generated and registered successfully.")
		.arg(aKeyId).arg(QString(keyPair.sd)).arg(QString(keyPair.ap)).arg(QString(keyPair.op)));

	aAP = keyPair.ap;
	aSD = keyPair.sd;
	aOP = keyPair.op;

	return EKeysUtilsError::Ok;
}

//---------------------------------------------------------------------------
bool CryptService::replaceKeys(int aKeyIdSrc, int aKeyIdDst)
{
	bool result = false;

	if (mKeys.contains(aKeyIdSrc))
	{
		PP::SKeySettings keySrc = getKey(aKeyIdSrc);

		if (keySrc.isValid)
		{
			getCryptEngine()->releaseKeyPair(aKeyIdSrc);
			getCryptEngine()->releaseKeyPair(aKeyIdDst);

			PP::SKeySettings dstKey = keySrc;
			
			dstKey.id = aKeyIdDst;

			mKeys.remove(aKeyIdSrc);
			mKeys[aKeyIdDst] = dstKey;

			// загружаем новый ключ в память
			loadKey(dstKey);

			PP::TerminalSettings * settings = SettingsService::instance(mApplication)->getAdapter<PP::TerminalSettings>();

			// чистим список ключей
			settings->cleanKeys();

			// сохраняем заново список действующих ключей
			foreach (auto key, mKeys)
			{
				if (key.id >= 0)
				{
					addKey(key);
				}
			}

			result = true;
		}
	}

	return result;
}


//---------------------------------------------------------------------------
bool CryptService::saveKey()
{
	LOG(mApplication->getLog(), LogLevel::Normal, QString("Saving key pair %1.").arg(mKeyPair.id));

	// Формируем и сохраяняем ключ.
	ICryptEngine * crypt = &CCryptEngine::instance();

	PP::SKeySettings key;

	key.id = mKeyPair.id;
	key.ap = mKeyPair.ap;
	key.op = mKeyPair.op;
	key.sd = mKeyPair.sd;
	key.engine = mKeyPair.engine;

	QByteArray serverPublicKey = mKeyPair.serverPublicKey;

	QString shortKeyPath = QString("keys/") + key.ap;
	QString fullKeyPath = mApplication->getUserDataPath() + "/" + shortKeyPath;

	if (QDir(fullKeyPath).exists())
	{
		// если такая папка с ключами существует, то создаем новую папку
		QDateTime currentDateTime = QDateTime::currentDateTime();

		shortKeyPath += currentDateTime.toString(".yyyyMMdd_hhmmss");
		fullKeyPath += currentDateTime.toString(".yyyyMMdd_hhmmss");
	}

	QString shortPublicKeyPath = shortKeyPath + "/" + CServiceCore::DefaultPublicKey;
	QString shortSecretKeyPath = shortKeyPath + "/" + CServiceCore::DefaultSecretKey;
	QString fullPublicKeyPath = fullKeyPath + "/" + CServiceCore::DefaultPublicKey;
	QString fullSecretKeyPath = fullKeyPath + "/" + CServiceCore::DefaultSecretKey;

	QDir keyDir;

	if (!keyDir.mkpath(fullKeyPath))
	{
		LOG(mApplication->getLog(), LogLevel::Error, QString("Failed to create directory: %1.").arg(fullKeyPath));
		return false;
	}

	// Сначала дампим свой открытый ключ
	if (!crypt->exportPublicKeyToFile(key.id, fullPublicKeyPath, key.serialNumber))
	{
		LOG(mApplication->getLog(), LogLevel::Error, "Failed to export public key.");
		return false;
	}

	// Потом открытый серверный
	if (!crypt->replacePublicKey(key.id, serverPublicKey))
	{
		LOG(mApplication->getLog(), LogLevel::Error, "Failed to replace public key.");
		return false;
	}

	if (!crypt->exportPublicKeyToFile(key.id, fullPublicKeyPath, key.bankSerialNumber))
	{
		LOG(mApplication->getLog(), LogLevel::Error, "Failed to export server public key.");
		return false;
	}

	key.publicKeyPath = shortPublicKeyPath;

	if (key.engine == CCrypt::ETypeEngine::File)
	{
		key.secretPassword = QString::fromLatin1(crypt->generatePassword());
		key.secretKeyPath = shortSecretKeyPath;

		if (!crypt->exportSecretKeyToFile(key.id, fullSecretKeyPath, key.secretPassword.toLatin1()))
		{
			LOG(mApplication->getLog(), LogLevel::Error, "Failed to export private key.");
			return false;
		}
	}

	// Обновляем конфигурационный файл с ключами.
	key.isValid = true;

	return addKey(key);
}

//---------------------------------------------------------------------------
ICryptEngine * CryptService::getCryptEngine()
{
	return &CCryptEngine::instance();
}

//---------------------------------------------------------------------------
