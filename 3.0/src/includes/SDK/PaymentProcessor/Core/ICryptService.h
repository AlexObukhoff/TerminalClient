/* @file Криптографический сервис. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

class ICryptEngine;

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
class ICryptService
{
public:
	/// Генерирует и регистрирует ключ на сервере. Возвращает параметры сгенерированного ключа.
	virtual int generateKey(int aKeyId, const QString & aLogin, const QString & aPassword, const QString & aURL,
		QString & aSD, QString & aAP, QString & aOP) = 0;

	/// Сохраняет сгенерированный ключ.
	virtual bool saveKey() = 0;

	/// Меняем ключи местами
	virtual bool replaceKeys(int aKeyIdSrc, int aKeyIdDst) = 0;

	/// Возвращает интерфейс криптодвижка.
	virtual ICryptEngine * getCryptEngine() = 0;

protected:
	virtual ~ICryptService() {}
};

//------------------------------------------------------------------------------
}} // SDK::PaymentProcessor
