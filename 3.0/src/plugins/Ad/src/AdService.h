/* @file Сервис для работы с рекламой. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSharedPointer>
#include <QtCore/QDateTime>
#include <QtCore/QUrl>
#include <Common/QtHeadersEnd.h>

// Modules
#include <System/IApplication.h>
#include <Common/ILogable.h>

// SDK
#include <SDK/PaymentProcessor/Core/IAdService.h>
#include <SDK/PaymentProcessor/Core/IService.h>

//---------------------------------------------------------------------------
namespace Ad
{
	struct Campaign; 
	class DatabaseUtils;
	class Client;
}

//---------------------------------------------------------------------------
class AdService :
	public QObject,
	public SDK::PaymentProcessor::IAdService,
	public SDK::PaymentProcessor::IService,
	private ILogable
{
	Q_OBJECT

public:
	/// Получение AdService'а.
	static AdService * instance(IApplication * aApplication);

	AdService(IApplication * aApplication);
	virtual ~AdService();

	/// IService: Инициализация сервиса.
	virtual bool initialize();

	/// IService: Закончена инициализация всех сервисов.
	virtual void finishInitialize();

	/// Возвращает false, если сервис не может быть остановлен в текущий момент.
	virtual bool canShutdown();

	/// IService: Завершение работы сервиса.
	virtual bool shutdown();

	/// IService: Возвращает имя сервиса.
	virtual QString getName() const;

	/// IService: Список необходимых сервисов.
	virtual const QSet<QString> & getRequiredServices() const;

	/// IService: Получить параметры сервиса.
	virtual QVariantMap getParameters() const;

	/// IService: Сброс служебной информации.
	virtual void resetParameters(const QSet<QString> & aParameters);

	#pragma region SDK::PaymentProcessor::IAdService interface

	/// Получить содержимое рекламного контента
	virtual QVariant getContent(const QString & aName) const;

	/// Зарегистрировать событие в статистике
	virtual void addEvent(const QString & aName);

	#pragma endregion

private:
	IApplication * mApplication;
	QSettings * mSettings;
	QMap<int, QString> mChannelsMap;

	QSharedPointer<Ad::DatabaseUtils> mDatabase;
	QSharedPointer<Ad::Client> mClient;
};


//---------------------------------------------------------------------------
