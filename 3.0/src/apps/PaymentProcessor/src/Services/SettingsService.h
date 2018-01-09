/* @file Сервис для работы с настройками. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/ISettingsAdapter.h>

class SettingsManager;
class IApplication;

//---------------------------------------------------------------------------
class SettingsService : public SDK::PaymentProcessor::IService, public SDK::PaymentProcessor::ISettingsService
{
public:
	SettingsService(IApplication * aApplication);
	virtual ~SettingsService();

	/// IService: инициализация сервиса.
	virtual bool initialize();

	/// IService: Закончена инициализация всех сервисов.
	virtual void finishInitialize();

	/// IService: Возвращает false, если сервис не может быть остановлен в текущий момент.
	virtual bool canShutdown();

	/// IService: остановка сервиса.
	virtual bool shutdown();

	/// IService: имя сервиса.
	virtual QString getName() const;

	/// IService: список зависимостей.
	virtual const QSet<QString> & getRequiredServices() const;

	/// Получить параметры сервиса.
	virtual QVariantMap getParameters() const;

	/// Сброс служебной информации.
	virtual void resetParameters(const QSet<QString> & aParameters);

	/// ISettingsService: получить настроки.
	virtual SDK::PaymentProcessor::ISettingsAdapter * getAdapter(const QString & aAdapterName);

	/// ISettingsService: сохранить настройки.
	virtual bool saveConfiguration();

	SettingsManager * getSettingsManager() const;

	/// Получить адаптер настроек по его имени.
	template <typename T> T * getAdapter()
	{
		QString adapterName = T::getAdapterName();
		return static_cast<T *>(mSettingsAdapters.contains(adapterName) ? mSettingsAdapters[adapterName] : 0);
	}

	/// Возвращает список адаптеров.
	QList<SDK::PaymentProcessor::ISettingsAdapter *> enumerateAdapters() const;

	/// Получить инстанс сервиса.
	static SettingsService * instance(IApplication * aApplication);

private:
	SettingsManager * mSettingsManager;
	bool mRestoreConfiguration;
	QMap<QString, SDK::PaymentProcessor::ISettingsAdapter *> mSettingsAdapters;

	IApplication * mApplication;
};

//---------------------------------------------------------------------------

