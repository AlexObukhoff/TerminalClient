/* @file Сервис, владеющий крипто-движком. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QMap>
#include <Common/QtHeadersEnd.h>

#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/ICryptService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include <KeysUtils/KeysUtils.h>

class ICryptEngine;
class IApplication;

//---------------------------------------------------------------------------
class CryptService : public SDK::PaymentProcessor::IService, public SDK::PaymentProcessor::ICryptService
{
public:
	static CryptService * instance(IApplication * aApplication);

	CryptService(IApplication * aApplication);
	virtual ~CryptService();

	/// IService: инициализация сервиса.
	virtual bool initialize();

	/// IService: Закончена инициализация всех сервисов.
	virtual void finishInitialize();

	/// Возвращает false, если сервис не может быть остановлен в текущий момент.
	virtual bool canShutdown();

	/// IService: завершение работы сервиса.
	virtual bool shutdown();

	/// IService: возвращает имя сервиса.
	virtual QString getName() const;

	/// IService : список зависимостей.
	virtual const QSet<QString> & getRequiredServices() const;

	/// Получить параметры сервиса.
	virtual QVariantMap getParameters() const;

	/// Сброс служебной информации.
	virtual void resetParameters(const QSet<QString> & aParameters);

	#pragma region SDK::PaymentProcessor::ICryptService interface

	/// Сгенерировать и зарегистрировать ключ на сервере. Возвращает EKeysUtilsError::Enum.
	virtual int generateKey(int aKeyId, const QString & aLogin, const QString & aPassword, const QString & aURL, QString & aSD, QString & aAP, QString & aOP, const QString & aDescription = QString());

	/// Сохранить сгенерированный ключ.
	virtual bool saveKey();

	/// Меняем ключи местами
	virtual bool replaceKeys(int aKeyIdSrc, int aKeyIdDst);

	/// Получить информацию о ключе по номеру пары
	virtual SDK::PaymentProcessor::ICryptService::SKeyInfo getKeyInfo(int aKeyId);

	/// Возвращает загруженные номера пар ключей
	virtual QList<int> getLoadedKeys() const;

	/// Возвращает интерфейс криптодвижка.
	virtual ICryptEngine * getCryptEngine();

	#pragma endregion

	/// Получить ключ aId.
	SDK::PaymentProcessor::SKeySettings getKey(int aId) const;

	/// Добавить ключ.
	bool addKey(const SDK::PaymentProcessor::SKeySettings & aKey);

private:
	/// Загружаем ключ в криптодвижок
	void loadKey(SDK::PaymentProcessor::SKeySettings & aKey);

private:
	IApplication * mApplication;
	ILog * mLog;
	QMap<int, SDK::PaymentProcessor::SKeySettings> mKeys;

	SKeyPair mKeyPair;
};

//---------------------------------------------------------------------------
