/* @file Менеджер для работы с сключами */

#pragma once

// Qt
#include "Common/QtHeadersBegin.h"
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QVariantMap>
#include "Common/QtHeadersEnd.h"

// Modules
#include <KeysUtils/KeysUtils.h>
#include <Crypt/ICryptEngine.h>

// Project
#include "IConfigManager.h"

namespace SDK {
namespace PaymentProcessor {
	class ICore;
	class ICryptService;
	class TerminalSettings;
}}

//---------------------------------------------------------------------------
class KeysManager : public QObject, public IConfigManager
{
	Q_OBJECT

public:
	KeysManager(SDK::PaymentProcessor::ICore * aCore);
	~KeysManager();

public:
	/// Ключи создались?
	virtual bool isConfigurationChanged() const;

	/// Делаем текущую конфигурацию начальной
	virtual void resetConfiguration();

public:
	/// Получение информации о eToken
	CCrypt::TokenStatus tokenStatus() const;

	/// Форматирование ключа eToken
	bool formatToken();
	
public:
	/// Генерирует и регистрирует ключ на сервере.
	bool generateKey(QVariantMap & aKeysParam);

	/// Сохраняет сгенерированный ключ.
	bool saveKey();

	/// Параметры, получаемые от ICryptService в случае успешной регистрации ключей
	QString getSD() const;
	QString getAP() const;
	QString getOP() const;

private:
	QString errorToString(EKeysUtilsError::Enum aCode) const;

private:
	SDK::PaymentProcessor::ICore * mCore;
	SDK::PaymentProcessor::ICryptService * mCryptService;
	SDK::PaymentProcessor::TerminalSettings * mTerminalSettings;

	QString mSD;
	QString mAP;
	QString mOP;

	bool mIsGenerated;
};

//---------------------------------------------------------------------------
