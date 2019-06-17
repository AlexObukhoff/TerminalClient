/* @file Набор функций для создания и регистрации ключей на сервере. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QUrl>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <Common/QtHeadersEnd.h>

class ICryptEngine;
class NetworkTaskManager;

//---------------------------------------------------------------------------
namespace EKeysUtilsError
{
	enum Enum
	{
		/// Ошибок нет
		Ok = 0,
		/// Во время обмена с сервером произошла сетевая ошибка
		NetworkError,
		/// Неправильный логин или пароль для генерации карточки ключа
		WrongPassword,
		/// Сервер отдал нам не всю необходимую информацию
		WrongServerAnswer,
		/// Время на сервере киберплата сильно отличается от локального времени терминала
		WrongLocalTime,
		/// Неизвестная ошибка скрипта генерации карточки ключа
		UnknownServerError,
		/// Ошибка при создании пары ключей из карточки
		KeyPairCreateError,
		/// Невозможно экспортировать ключ из хранилища
		KeyExportError
	};

	QString errorToString(Enum aError);
};

//---------------------------------------------------------------------------
struct SKeyPair
{
	SKeyPair()
	{
		id = 0;
		engine = 0;
	}

	int id;

	QByteArray ap;
	QByteArray sd;
	QByteArray op;
	QString description;
	int engine;


	QByteArray serverPublicKey;
};

//---------------------------------------------------------------------------
/// Создание карточки ключа по логину и паролю.
///		aCrypt - криптодвижок, в котором будет создана пара ключей.
///		aKeyNumber - номер, под которым в криптодвижке будут созданы ключи.
///		aNetwork - сетевой интерфейс.
///		aUrl - адрес крипта для генерации ключей.
///		aLogin, aPassword - реквизиты для аутентификации.
///		aPair - результат генерации ключей.
///
EKeysUtilsError::Enum createKeyPair(ICryptEngine * aCrypt, int aKeyNumber, NetworkTaskManager * aNetwork, const QUrl & aUrl,
                  const QString & aLogin, const QString & aPassword, SKeyPair & aPair);

/// Регистрирует открытый ключ на сервере.
///		aCrypt - криптодвижок, в котором будет создана пара ключей.
///		aKeyNumber - номер, под которым в криптодвижке будут созданы ключи.
///		aNetwork - сетевой интерфейс.
///		aUrl - адрес крипта для генерации ключей.
///		aLogin, aPassword - реквизиты для аутентификации.
///		aPair - сюда запишется открытый ключ сервера после регистрации.
///
EKeysUtilsError::Enum registerKeyPair(ICryptEngine * aCrypt, int aKeyNumber, NetworkTaskManager * aNetwork, const QUrl & aUrl,
                    const QString & aLogin, const QString & aPassword, SKeyPair & aPair);

//---------------------------------------------------------------------------
