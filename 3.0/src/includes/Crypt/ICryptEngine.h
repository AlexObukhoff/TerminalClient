/* @file Интерфейс криптодвижка. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <Common/QtHeadersEnd.h>

//---------------------------------------------------------------------------
namespace CCrypt
{
	// Константы соответствуют значениям в libipriv.h
	enum ETypeEngine
	{
		File = 0,
		RuToken = 5
	};

	enum ETypeKey
	{
		Private = 0,
		Public
	};

	/// Состояние аппаратного ключа
	struct TokenStatus
	{
		bool available;
		QString name;
		bool initialized;

		TokenStatus() : available(false), initialized(false) {}
		bool isOK() const { return available && initialized; }
	};
}

//---------------------------------------------------------------------------
/// Интерфейс криптодвижка, производящего подписывание и верификацию сообщений, летящих
///	к серверу и от него.
class ICryptEngine
{
public:
	/// Функции init и shutdown предназначены для инициализации внутренних компонент шифрования.
	/// Их необходимо вызывать в начале и в конце работы приложения.
	virtual bool initialize() = 0;
	virtual void shutdown() = 0;

	/// Список доступных для использования движков
	virtual QSet<CCrypt::ETypeEngine> availableEngines() = 0;

	/// Получить статус аппаратного ключа
	virtual CCrypt::TokenStatus getTokenStatus(CCrypt::ETypeEngine aEngine) = 0;

	/// Инициализировать токен для работы с CryptEngine
	virtual bool initializeToken(CCrypt::ETypeEngine aEngine) = 0;

	/// Генерирует произвольный пароль.
	virtual QByteArray generatePassword() const = 0;

	/// Возвращает список возможных паролей для секретных ключей приложения (rootp, roots). 
	virtual QList<QByteArray> getRootPassword() const = 0;

	/// Все нижеследующие public-функции потокобезопасны.

	/// Создаёт пару ключей с именами aKeyPairNames.key и aKeyPairNamep.key.
	virtual bool createKeyPair(const QString & aPath, const QString & aKeyPairName, const QString & aUserId, 
		const QString & aPassword, const ulong aSerialNumber, QString & aErrorDescription) = 0;

	/// Создаёт пару ключей по карточке ключа.
	virtual bool createKeyPair(int aKeyPair, CCrypt::ETypeEngine aEngine, const QByteArray & aKeyCard, int aKeySize, QString & aErrorDescription) = 0;

	/// Загружает пару ключей в крипто-движок.
	virtual bool loadKeyPair(int aKeyPair, CCrypt::ETypeEngine aEngine, const QString & aSecretKeyPath, const QString & aPassword,
		const QString & aPublicKeyPath, const ulong aSerialNumber, const ulong aBankSerialNumber, QString & aErrorDescription) = 0;

	/// Возвращает серийный номер открытого ключа для указанной пары.
	virtual QString getKeyPairSerialNumber(int aKeyPair) = 0;

	/// Выгружает пару ключей с номером aKeyPair из движка.
	virtual bool releaseKeyPair(int aKeyPair) = 0;

	/// Выгружает все загруженные ключи. 
	virtual void releaseKeyPairs() = 0;

	/// Заменяет открытый ключ в указанной паре.
	virtual bool replacePublicKey(int aKeyPair, const QByteArray & aPublicKey) = 0;

	/// Выгружает закрытый ключ.
	virtual bool exportSecretKey(int aKeyPair, QByteArray & aSecretKey, const QByteArray & aPassword) = 0;

	/// Выгружает закрытый ключ в файл.
	virtual bool exportSecretKeyToFile(int aKeyPair, const QString & aFilePath, const QByteArray & aPassword) = 0;

	/// Выгружает открытый ключ.
	virtual bool exportPublicKey(int aKeyPair, QByteArray & aPublicKey, ulong & aSerialNumber) = 0;

	/// Выгружает открытый ключ в файл.
	virtual bool exportPublicKeyToFile(int aKeyPair, const QString & aFilePath, ulong & aSerialNumber) = 0;

	/// Функции подписывания данных ЭЦП

	/// Подписывает переданный запрос парой ключей aKeyPair. 
	virtual bool sign(int aKeyPair, const QByteArray & aRequest , QByteArray & aSignature, QString & aErrorDescription) = 0;

	/// Подписывает данные из запроса aRequest парой ключей aKeyPair и возвращает подпись в aSignature. 
	virtual bool signDetach(int aKeyPair, const QByteArray & aRequest, QByteArray & aSignature, QString & aErrorDescription) = 0;

	/// Функции проверки ЭЦП

	/// Проверяет подпись в строке aResponseString парой ключей aKeyPair, оригинальное сообщение складывает в aOriginalResponse. 
	virtual bool verify(int aKeyPair, const QByteArray & aResponseString, QByteArray & aOriginalResponse, QString & aErrorDescription) = 0;

	/// Проверяет подпись aSignature для сроки aResponseString с помощью пары ключей aKeyPair. 
	virtual bool verifyDetach(int aKeyPair, const QByteArray & aResponseString, const QByteArray & aSignature, QString & aErrorDescription) = 0;

	/// Функции шифрования данных

	/// Шифрует aData и складывает результат в aResult. Используется пара ключей aKeyPair, тип ключа - aKey. 
	virtual bool encrypt(int aKeyPair, const QByteArray & aData, QByteArray & aResult, CCrypt::ETypeKey aKey, QString & aErrorDescription) = 0;

	/// Шифрует aData и складывает результат в aResult. Используется пара ключей aKeyPair, выбирается открытый ключ. 
	virtual bool encrypt(int aKeyPair, const QByteArray & aData, QByteArray & aResult, QString & aErrorDescription) = 0;

	/// Шифрует длинные данные aData и складывает результат в aResult. Используется пара ключей aKeyPair, тип ключа - aKey. 
	virtual bool encryptLong(int aKeyPair, const QByteArray & aData, QByteArray & aResult, CCrypt::ETypeKey aKey, QString & aErrorDescription) = 0;

	/// Шифрует длинные данные aData и складывает результат в aResult. Используется пара ключей aKeyPair, выбирается открытый ключ. 
	virtual bool encryptLong(int aKeyPair, const QByteArray & aData, QByteArray & aResult, QString & aErrorDescription) = 0;

	/// Функции расшифровывания данных

	/// Расшифровывает aData и складывает результат в aResult. Используется пара ключей aKeyPair, тип ключа - aKey. 
	virtual bool decrypt(int aKeyPair, const QByteArray & aData, QByteArray & aResult, CCrypt::ETypeKey aKey, QString & aErrorDescription) = 0;

	/// Расшифровывает aData и складывает результат в aResult. Используется пара ключей aKeyPair, выбирается закрытый ключ. 
	virtual bool decrypt(int aKeyPair, const QByteArray & aData, QByteArray & aResult, QString & aErrorDescription) = 0;

	/// Расшифровывает длинные данные aData и складывает результат в aResult. Используется пара ключей aKeyPair, тип ключа - aKey. 
	virtual bool decryptLong(int aKeyPair, const QByteArray & aData, QByteArray & aResult, CCrypt::ETypeKey aKey, QString & aErrorDescription) = 0;

	/// Расшифровывает длинные данные aData и складывает результат в aResult. Используется пара ключей aKeyPair, выбирается закрытый ключ. 
	virtual bool decryptLong(int aKeyPair, const QByteArray & aData, QByteArray & aResult, QString & aErrorDescription) = 0;

	/// Функции хранения защищенного файла на токене

	/// Сохранить даные в токен
	virtual bool setData(const QString & aName, const QByteArray & aData) = 0;

	/// Прочитать данные из токена
	virtual bool getData(const QString & aName, QByteArray & aData) = 0;

protected:
	virtual ~ICryptEngine() {};
};

//---------------------------------------------------------------------------
namespace CCryptEngine
{
	ICryptEngine & instance();
}

//---------------------------------------------------------------------------
