/* @file Реализация клиента, взаимодействующего с сервером рекламы. */

#pragma once

// Stl
#include <memory>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QUrl>
#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QSettings>
#include <QtCore/QPointer>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/IPluginEnvironment.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/CyberPlat/RequestSender.h>
#include <SDK/PaymentProcessor/CyberPlat/Response.h>

// Modules
#include <Common/ILogable.h>
#include <Crypt/ICryptEngine.h>
#include <NetworkTaskManager/NetworkTaskManager.h>

using SDK::PaymentProcessor::CyberPlat::Request;
using SDK::PaymentProcessor::CyberPlat::Response;
using SDK::PaymentProcessor::CyberPlat::RequestSender;

class FileDownloadTask;

namespace Ad
{

class DatabaseUtils;
struct Campaign;

//------------------------------------------------------------------------
namespace CClient
{
	const QString LogName = "Ad";
}

//------------------------------------------------------------------------
class Client : public QObject, protected ILogable
{
	Q_OBJECT

public:
	Client(SDK::PaymentProcessor::ICore * aCore, ILog * aLog, int aKeyPair);

	virtual ~Client();

public:
	/// Получить содержимое рекламного контента
	virtual QString getContent(const QString & aType);

	/// Зарегистрировать событие в статистике
	virtual void addEvent(const QString & aType);

	/// Получить объект настроек клиента
	QSharedPointer<QSettings> getSettings();

	virtual void timerEvent(QTimerEvent * aEvent);

public:
	Campaign getAd(const QString & aType) const;
	QList<Campaign> getAds() const;
	void saveChannel(const Campaign & aCampaign);

private:
	/// Прочитать из конфигурации описание канала с рекламной кампанией
	Campaign getAdInternal(const QString & aType) const;

protected:
	bool checkFile(const QUrl & aUrl, const QString & aPath, const QString & aMd5);
	void downloadFile(const QUrl & aUrl, const QString & aPath);
	void checkExpiration(const Campaign & aCampaign);

signals:
	void contentUpdated();
	void contentExpired();

public slots:
	void reinitialize();

	/// Выполнить процедуру обновления контента
	void update();

private slots:
	void doUpdate();
	void updateTypes();
	/// Запускаем updater для закачивания рекламы
	void download();
	void checkContent();
	void sendStatistics();

private slots:
	void onCommandStatusChanged(int aID, int aStatusInt, QVariantMap aParameters);

private:
	/// Создаёт класс ответа по классу запроса.
	virtual Response * createResponse(const Request & aRequest, const QString & aResponseString);

	/// Отправка запроса на указанный адрес.
	Response * sendRequest(const QUrl & aUrl, Request & aRequest);

	/// Проверка, устарели ли обновлённые данные
	bool updateExpired() const;

private:
	QThread                          mThread;
	SDK::PaymentProcessor::ICore   * mCore;
	QSharedPointer<RequestSender>    mHttp;
	QUrl                             mServerUrl;
	QString                          mContentPath;
	QSharedPointer<DatabaseUtils>    mDatabaseUtils;
	QSharedPointer<QSettings>        mSettings;

	int mExpirationTimer;
	QDateTime mExpirationTime;

private:
	/// Время запуска обновления
	QDateTime mUpdateStamp;

	/// Список актуальных каналов
	QStringList mTypeList;
	
	/// Список новых каналов, полученных от сервера
	QMap<QString, Ad::Campaign> mCampaigns;
	
	/// Список каналов, подлежащих обновлению
	QStringList mTypeDownloadList;

	/// Кол-во сохранённых в настройки каналов
	int mSavedTypes;

	/// Текущая команда загрузки архива
	int mCurrentDownloadCommand;
};

//------------------------------------------------------------------------
} // Ad

//------------------------------------------------------------------------
