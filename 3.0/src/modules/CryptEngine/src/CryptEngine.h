/* @file Реализация криптодвижока на libipriv. */

#pragma once

// Libipriv
#include <libipriv/libipriv.h>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QPair>
#include <QtCore/QSet>
#include <QtCore/QMap>
#include <QtCore/QMutex>
#include <Common/QtHeadersEnd.h>

// Project
#include "Crypt/ICryptEngine.h"

//---------------------------------------------------------------------------
/// Пара ключей: закрытый, открытый
typedef QPair<IPRIV_KEY, IPRIV_KEY> TKeyPair;

/// Номер пары ключей + ключи
typedef QMap<int, TKeyPair> TKeyPairList;

//---------------------------------------------------------------------------
/// Реализация криптодвижка, основанная на библиотеке iprivg.
class CryptEngine : public ICryptEngine
{
public:
	CryptEngine();

	virtual ~CryptEngine();

	/// Функции init и shutdown предназначены для инициализации внутренних компонент шифрования.
	/// Их необходимо вызывать в начале и в конце работы приложения.
	virtual bool initialize();
	virtual void shutdown();

	/// Список доступных для использования движков
	virtual QSet<CCrypt::ETypeEngine> availableEngines();

	/// Получить статус аппаратного ключа
	virtual CCrypt::TokenStatus getTokenStatus(CCrypt::ETypeEngine aEngine);

	/// Инициализировать токен для работы с CryptEngine
	virtual bool initializeToken(CCrypt::ETypeEngine aEngine);

	/// Генерирует произвольный пароль.
	virtual QByteArray generatePassword() const;

	/// Возвращает список возможных паролей для секретных ключей приложения (rootp, roots).
	virtual QList<QByteArray> getRootPassword() const;

	/// Все нижеследующие public-функции потокобезопасны.

	/// Создаёт пару ключей с именами aKeyPairNames.key и aKeyPairNamep.key.
	virtual bool createKeyPair(const QString & aPath, const QString & aKeyPairName, const QString & aUserId, 
		const QString & aPassword, const ulong aSerialNumber, QString & aErrorDescription);

	/// Создаёт пару ключей по карточке ключа.
	virtual bool createKeyPair(int aKeyPair, CCrypt::ETypeEngine aEngine, const QByteArray & aKeyCard, int aKeySize, QString & aErrorDescription);

	/// Загружает пару ключей в крипто-движок.
	virtual bool loadKeyPair(int aKeyPair, CCrypt::ETypeEngine aEngine, const QString & aSecretKeyPath, const QString & aPassword,
		const QString & aPublicKeyPath, const ulong aSerialNumber, const ulong aBankSerialNumber, QString & aErrorDescription);

	/// Возвращает серийный номер открытого ключа для указанной пары.
	virtual QString getKeyPairSerialNumber(int aKeyPair);

	/// Выгружает пару ключей с номером aKeyPair из движка.
	virtual bool releaseKeyPair(int aKeyPair);

	/// Выгружает все загруженные ключи. 
	virtual void releaseKeyPairs();

	/// Заменяет открытый ключ в указанной паре.
	virtual bool replacePublicKey(int aKeyPair, const QByteArray & aPublicKey);

	/// Выгружает закрытый ключ.
	virtual bool exportSecretKey(int aKeyPair, QByteArray & aSecretKey, const QByteArray & aPassword);

	/// Выгружает закрытый ключ в файл.
	virtual bool exportSecretKeyToFile(int aKeyPair, const QString & aFilePath, const QByteArray & aPassword);

	/// Выгружает открытый ключ.
	virtual bool exportPublicKey(int aKeyPair, QByteArray & aPublicKey, ulong & aSerialNumber);

	/// Выгружает открытый ключ в файл.
	virtual bool exportPublicKeyToFile(int aKeyPair, const QString & aFilePath, ulong & aSerialNumber);

	/// Функции подписывания данных ЭЦП

	/// Подписывает переданный запрос парой ключей aKeyPair. 
	virtual bool sign(int aKeyPair, const QByteArray & aRequest , QByteArray & aSignature, QString & aErrorDescription);

	/// Подписывает данные из запроса aRequest парой ключей aKeyPair и возвращает подпись в aSignature. 
	virtual bool signDetach(int aKeyPair, const QByteArray & aRequest, QByteArray & aSignature, QString & aErrorDescription);

	/// Функции проверки ЭЦП

	/// Проверяет подпись в строке aResponseString парой ключей aKeyPair, оригинальное сообщение складывает в aOriginalResponse. 
	virtual bool verify(int aKeyPair, const QByteArray & aResponseString, QByteArray & aOriginalResponse, QString & aErrorDescription);

	/// Проверяет подпись aSignature для сроки aResponseString с помощью пары ключей aKeyPair. 
	virtual bool verifyDetach(int aKeyPair, const QByteArray & aResponseString, const QByteArray & aSignature, QString & aErrorDescription);

	/// Функции шифрования данных

	/// Шифрует aData и складывает результат в aResult. Используется пара ключей aKeyPair, тип ключа - aKey. 
	virtual bool encrypt(int aKeyPair, const QByteArray & aData, QByteArray & aResult, CCrypt::ETypeKey aKey, QString & aErrorDescription);

	/// Шифрует aData и складывает результат в aResult. Используется пара ключей aKeyPair, выбирается открытый ключ. 
	virtual bool encrypt(int aKeyPair, const QByteArray & aData, QByteArray & aResult, QString & aErrorDescription);

	/// Шифрует длинные данные aData и складывает результат в aResult. Используется пара ключей aKeyPair, тип ключа - aKey. 
	virtual bool encryptLong(int aKeyPair, const QByteArray & aData, QByteArray & aResult, CCrypt::ETypeKey aKey, QString & aErrorDescription);

	/// Шифрует длинные данные aData и складывает результат в aResult. Используется пара ключей aKeyPair, выбирается открытый ключ. 
	virtual bool encryptLong(int aKeyPair, const QByteArray & aData, QByteArray & aResult, QString & aErrorDescription);

	/// Функции расшифровывания данных

	/// Расшифровывает aData и складывает результат в aResult. Используется пара ключей aKeyPair, тип ключа - aKey. 
	virtual bool decrypt(int aKeyPair, const QByteArray & aData, QByteArray & aResult, CCrypt::ETypeKey aKey, QString & aErrorDescription);

	/// Расшифровывает aData и складывает результат в aResult. Используется пара ключей aKeyPair, выбирается закрытый ключ. 
	virtual bool decrypt(int aKeyPair, const QByteArray & aData, QByteArray & aResult, QString & aErrorDescription);

	/// Расшифровывает длинные данные aData и складывает результат в aResult. Используется пара ключей aKeyPair, тип ключа - aKey. 
	virtual bool decryptLong(int aKeyPair, const QByteArray & aData, QByteArray & aResult, CCrypt::ETypeKey aKey, QString & aErrorDescription);

	/// Расшифровывает длинные данные aData и складывает результат в aResult. Используется пара ключей aKeyPair, выбирается закрытый ключ. 
	virtual bool decryptLong(int aKeyPair, const QByteArray & aData, QByteArray & aResult, QString & aErrorDescription);

	/// Функции хранения защищенного файла на токене

	/// Сохранить даные в токен
	virtual bool setData(const QString & aName, const QByteArray & aData);

	/// Прочитать данные из токена
	virtual bool getData(const QString & aName, QByteArray & aData);

private:
	/// Возвращает описание ошибки библиотеки libipriv.
	QString errorToString(int aError) const;

private:
	bool mInitialized;
	CCrypt::ETypeEngine mEngine;

	QMutex mMutex;

	TKeyPairList mKeyPairs;
};

//---------------------------------------------------------------------------
