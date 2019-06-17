/* @file Реализация клиента, взаимодействующего с сервером рекламы. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTimer>
#include <QtCore/QTimerEvent>
#include <QtCore/QCryptographicHash>
#include <QtXml/QDomDocument>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/INetworkService.h>
#include <SDK/PaymentProcessor/Core/ICryptService.h>
#include <SDK/PaymentProcessor/Core/IRemoteService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

// Modules
#include <NetworkTaskManager/DataStream.h>
#include <NetworkTaskManager/FileDownloadTask.h>
#include <SysUtils/ISysUtils.h>

// Project
#include <AdBackend/IDatabaseUtils.h>
#include <AdBackend/DatabaseUtils.h>

#include "Client.h"
#include "AdRequests.h"
#include "AdResponses.h"

namespace PPSDK = SDK::PaymentProcessor;

using namespace std::placeholders;

namespace Ad
{
	/// Имя файла настроек
	const QString SettingsName = "ad.ini";

	/// Имена параметров в ini, описывающие рекламную кампанию
	namespace Settings
	{
		const QString Url = "ad/url";

		const QString Types = "ad/types";

		const QString ID = "ID";
		const QString Source = "source";
		const QString Expired = "expired";
		const QString Text = "text";
		const QString MD5 = "md5";
	}

//------------------------------------------------------------------------
namespace CClient
{
	const int ReinitInterval           =      10 * 60 * 1000;
	const int ContentCheckInterval     =      20 * 60 * 1000;
	const int ContentRecheckInterval   =       3 * 60 * 1000;
	const int StatisticsSendInterval   = 12 * 60 * 60 * 1000;
	const int StatisticsResendInterval =      30 * 60 * 1000;

	const QString ThreadName = "AdClient";
}

//------------------------------------------------------------------------
Client::Client(SDK::PaymentProcessor::ICore * aCore, ILog * aLog, int aKeyPair) :
	ILogable(aLog),
	mCore(aCore),
	mSavedTypes(0),
	mExpirationTimer(-1),
	mCurrentDownloadCommand(-999)
{
	mThread.setObjectName(CClient::ThreadName);
	moveToThread(&mThread);

	auto terminalSettings = static_cast<PPSDK::TerminalSettings *>(mCore->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter));
	mContentPath = terminalSettings->getAppEnvironment().adPath;

	QDir dir;
	if (!dir.mkpath(mContentPath))
	{
		toLog(LogLevel::Error, QString("Cannot create path %1.").arg(mContentPath));
	}

	
	mSettings = QSharedPointer<QSettings>(new QSettings(ISysUtils::rmBOM(mContentPath + QDir::separator() + Ad::SettingsName), QSettings::IniFormat));
	mSettings->setIniCodec("utf-8");
	mSettings->moveToThread(&mThread);

	mServerUrl = mSettings->value(Settings::Url).toUrl();
	mDatabaseUtils = QSharedPointer<DatabaseUtils>(new DatabaseUtils(mContentPath, getLog()));
	mHttp = QSharedPointer<RequestSender>(new RequestSender(mCore->getNetworkService()->getNetworkTaskManager(),
		static_cast<ICryptEngine *>(mCore->getCryptService()->getCryptEngine())));

	mHttp->setCryptKeyPair(aKeyPair);
	mHttp->setResponseCreator(std::bind(&Client::createResponse, this, _1, _2));

	connect(mCore->getRemoteService(), SIGNAL(commandStatusChanged(int, int, QVariantMap)), SLOT(onCommandStatusChanged(int, int, QVariantMap)));
	
	mThread.start();
}

//------------------------------------------------------------------------
Client::~Client()
{
	if (mThread.isRunning())
	{
		mThread.quit();
		mThread.wait();
	}
}

//------------------------------------------------------------------------
QSharedPointer<QSettings> Client::getSettings()
{
	return mSettings;
}

//------------------------------------------------------------------------
void Client::reinitialize()
{
	toLog(LogLevel::Normal, "Initializing advertising system client.");

	toLog(LogLevel::Normal, QString("Server: %1.").arg(mServerUrl.toString()));
	toLog(LogLevel::Normal, QString("Content path: %1.").arg(mContentPath));

	QDir dir;
	if (!dir.mkpath(mContentPath))
	{
		toLog(LogLevel::Error, QString("Cannot create path %1.").arg(mContentPath));

		QTimer::singleShot(CClient::ReinitInterval, this, SLOT(reinitialize()));

		return;
	}

	foreach (auto channel, mSettings->value(Settings::Types).toStringList())
	{
		checkExpiration(getAdInternal(channel));
	}
}

//------------------------------------------------------------------------------
void Client::update()
{
	// Проинициализировались, теперь можно проверить обновление
	QTimer::singleShot(0, this, SLOT(doUpdate()));

	// Пробуем отправить статистику
	QTimer::singleShot(0, this, SLOT(sendStatistics()));
}

//------------------------------------------------------------------------------
Response * Client::sendRequest(const QUrl & aUrl, Request & aRequest)
{
	if (mHttp)
	{
		toLog(LogLevel::Normal, QString("> %1.").arg(aRequest.toLogString()));

		RequestSender::ESendError error;

		QScopedPointer<Response> response(mHttp->post(aUrl, aRequest, RequestSender::Solid, error));

		if (response)
		{
			toLog(LogLevel::Normal, QString("< %1.").arg(response->toLogString()));
		}
		else
		{
			toLog(LogLevel::Error, QString("Failed to send: %1.").arg(RequestSender::translateError(error)));
		}

		return response.take();
	}

	return 0;
}

//------------------------------------------------------------------------------
Response * Client::createResponse(const Request & aRequest, const QString & aResponseString)
{
	QString requestType = aRequest.getParameter(Ad::Parameters::RequestType).toString();

	if (requestType == Ad::Requests::ChannelList)
	{
		return new AdGetChannelsResponse(aRequest, aResponseString);
	}
	else if (requestType == Ad::Requests::Channel)
	{
		return new AdGetChannelResponse(aRequest, aResponseString);
	}

	return new AdResponse(aRequest, aResponseString);
}

//------------------------------------------------------------------------
void Client::doUpdate()
{
	toLog(LogLevel::Normal, "Getting channels list...");

	mUpdateStamp = QDateTime::currentDateTime();

	AdGetChannelsRequest request(mCore);

	QScopedPointer<Response> response(sendRequest(mServerUrl, request));

	if (!response || !response->isOk())
	{
		toLog(LogLevel::Error, QString("Failed to get channels list: (%1) %2").arg(response ? response->getError() : 0).arg(response ? response->getErrorMessage() : ""));

		QTimer::singleShot(CClient::ReinitInterval, this, SLOT(doUpdate()));
	}
	else
	{
		mTypeList = static_cast<AdGetChannelsResponse *>(response.data())->channels();

		QTimer::singleShot(0, this, SLOT(updateTypes()));
	}
}

//------------------------------------------------------------------------
void Client::updateTypes()
{
	foreach (auto channel, mTypeList)
	{
		AdGetChannelRequest request(mCore, channel);

		QScopedPointer<Response> response(sendRequest(mServerUrl, request));

		if (!response || !response->isOk())
		{
			toLog(LogLevel::Error, QString("Failed to get channel '%1': (%2) %3").arg(channel).arg(response ? response->getError() : 0).arg(response ? response->getErrorMessage() : ""));

			QTimer::singleShot(CClient::ReinitInterval, this, SLOT(updateTypes()));
			break;
		}
		else
		{
			foreach (auto campaign, static_cast<AdGetChannelResponse *>(response.data())->getCampaigns())
			{
				toLog(LogLevel::Debug, QString("Receive campaign: %1").arg(campaign.toString()));

				mCampaigns.insert(campaign.type, campaign);
			}
		}
	}

	QTimer::singleShot(0, this, SLOT(checkContent()));
}

//------------------------------------------------------------------------
void Client::checkContent()
{
	mTypeDownloadList.clear();

	foreach (auto campaign, mCampaigns.values())
	{
		Campaign c = getAdInternal(campaign.type);

		if (!c.isEqual(campaign))
		{
			if (campaign.isDownloaded())
			{
				mTypeDownloadList << campaign.type;
			}
			else
			{
				// сохраняем содержимое канала
				saveChannel(campaign);
			}
		}
	}

	download();
}

//------------------------------------------------------------------------
void Client::download()
{
	if (mTypeDownloadList.isEmpty())
	{
		toLog(LogLevel::Normal, "Download nothing. All up to date.");

		if (mSavedTypes)
		{
			emit contentUpdated();
		}

		return;
	}

	QString campaignName = mTypeDownloadList.at(0);

	if (mCampaigns[campaignName].isValid() && mCampaigns[campaignName].isDownloaded())
	{
		toLog(LogLevel::Normal, QString("Download campaign [%1]:%2...").arg(mCampaigns[campaignName].id).arg(mCampaigns[campaignName].type));

		mCurrentDownloadCommand = mCore->getRemoteService()->registerUpdateCommand(PPSDK::IRemoteService::AdUpdate, mCampaigns[campaignName].url, QUrl(), mCampaigns[campaignName].md5);

		if (!mCurrentDownloadCommand)
		{
			toLog(LogLevel::Warning, "Failed try to start update ad. Try later.");

			QTimer::singleShot(CClient::ReinitInterval, this, SLOT(download()));
		}
	}
	else
	{
		mTypeDownloadList.removeAt(0);

		QTimer::singleShot(10, this, SLOT(download()));
	}
}

//------------------------------------------------------------------------
void Client::sendStatistics()
{
	QList<SStatisticRecord> statisticList;

	int retryTimeout = CClient::StatisticsResendInterval;

	if (!mDatabaseUtils->getUnsentStatisticRecords(statisticList))
	{
		toLog(LogLevel::Error, "Cannot get ad statistics from DB.");
	}
	else
	{
		if (!statisticList.isEmpty())
		{
			AdStatisticRequest request(mCore, statisticList);

			QScopedPointer<Response> response(sendRequest(mServerUrl, request));

			if (!response || !response->isOk())
			{
				toLog(LogLevel::Error, QString("Failed to sent statistic: (%1) %2").arg(response ? response->getError() : 0).arg(response ? response->getErrorMessage() : ""));
			}
			else
			{
				toLog(LogLevel::Normal, "Mark statistic as sent.");

				mDatabaseUtils->deleteStatisticRecords(statisticList);

				retryTimeout = CClient::StatisticsSendInterval;
			}
		}
		else
		{
			toLog(LogLevel::Normal, "Statistic: nothing to send.");

			retryTimeout = CClient::StatisticsSendInterval;
		}
	}

	QTimer::singleShot(retryTimeout, this, SLOT(sendStatistics()));
}

//---------------------------------------------------------------------------
QList<Campaign> Client::getAds() const
{
	QList<Campaign> result;

	foreach (auto ch, mSettings->value(Settings::Types).toStringList())
	{
		result << getAdInternal(ch);
	}

	return result;
}

//---------------------------------------------------------------------------
Ad::Campaign Client::getAdInternal(const QString & aType) const
{
	Ad::Campaign camp;

	mSettings->beginGroup("ad_" + aType);

	camp.id = mSettings->value(Settings::ID, -1).toLongLong();
	camp.type = aType;
	camp.url = mSettings->value(Settings::Source).toUrl();
	camp.md5 = mSettings->value(Settings::MD5).toString();
	camp.expired = QDateTime::fromString(mSettings->value(Settings::Expired).toString(), Ad::Parameters::DateTimeFormat);
	camp.text = mSettings->value(Settings::Text).toString();

	mSettings->endGroup();

	return camp;
}

//------------------------------------------------------------------------
Ad::Campaign Client::getAd(const QString & aType) const
{
	auto channel = getAdInternal(aType);

	QString bannerPath = mContentPath + QDir::separator() + channel.type + QDir::separator() + "banner.swf";

	if (!channel.isValid() || channel.isExpired())
	{
		return getAdInternal(aType + Ad::DefaultChannelPostfix);
	}

	if (!channel.isDefault() && channel.isBanner() && !QFile::exists(bannerPath))
	{
		toLog(LogLevel::Error, QString("Failed to get ad content: path '%1' not found. Use default channel.").arg(bannerPath));

		return getAdInternal(aType + Ad::DefaultChannelPostfix);
	}

	return channel;
}

//------------------------------------------------------------------------
void Client::saveChannel(const Campaign & aCampaign)
{
	mSettings->beginGroup("ad_" + aCampaign.type);

	mSettings->setValue(Settings::ID, aCampaign.id);
	mSettings->setValue(Settings::Source, aCampaign.url.toString());
	mSettings->setValue(Settings::MD5, aCampaign.md5);
	mSettings->setValue(Settings::Expired, aCampaign.expired.toString(Ad::Parameters::DateTimeFormat));
	mSettings->setValue(Settings::Text, aCampaign.text);

	mSettings->endGroup();

	QStringList types = mSettings->value(Settings::Types).toStringList();
	if (!types.contains(aCampaign.type))
	{
		types << aCampaign.type;
		mSettings->setValue(Settings::Types, types);
	}

	mSettings->sync();

	mSavedTypes++;

	checkExpiration(aCampaign);
}

//------------------------------------------------------------------------
void Client::onCommandStatusChanged(int aID, int aStatus, QVariantMap aParameters)
{
	if (mCurrentDownloadCommand != aID)
	{
		return;
	}

	// Нас интересуют только финальные состояния
	switch (aStatus)
	{
	case PPSDK::IRemoteService::OK:
		{
			if (!mTypeDownloadList.isEmpty())
			{
				QString type = mTypeDownloadList.at(0);
			
				saveChannel(mCampaigns[type]);

				mCurrentDownloadCommand = 0;
				mTypeDownloadList.removeAt(0);

				toLog(LogLevel::Normal, QString("Download '%1' complete.").arg(type));

				QTimer::singleShot(0, this, SLOT(download()));
			}
		}
		break;

	case PPSDK::IRemoteService::Error:
		{
			toLog(LogLevel::Error, "Failed to download ad content. Try later.");

			// Перезапускаем загрузку компаний
			QTimer::singleShot(CClient::ContentCheckInterval, this, SLOT(download()));
		}
		break;
	}
}

//------------------------------------------------------------------------
QString Client::getContent(const QString & aType)
{
	auto channel = getAd(aType);

	return channel.isBanner() ? 
		mContentPath + QDir::separator() + channel.type :
		channel.text;
}

//------------------------------------------------------------------------
void Client::addEvent(const QString & aType)
{
	auto channel = getAd(aType);

	if (channel.isValid())
	{
		if (channel.isBanner())
		{
			// Баннер регистрируем только 1 в день
			mDatabaseUtils->setStatisticRecord(channel.id, channel.type, 1);
		}
		else
		{
			mDatabaseUtils->addStatisticRecord(channel.id, channel.type);
		}
	}
	else
	{
		toLog(LogLevel::Warning, QString("Skip not valid channel '%1' event.").arg(aType));
	}
}

//------------------------------------------------------------------------
bool Client::updateExpired() const
{
	return mUpdateStamp.isNull() || (mUpdateStamp.addSecs(6 * 60 * 60) < QDateTime::currentDateTime());
}

//------------------------------------------------------------------------
void Client::timerEvent(QTimerEvent * aEvent)
{
	if (aEvent->timerId() == mExpirationTimer)
	{
		emit contentExpired();
	}
}

//------------------------------------------------------------------------
void Client::checkExpiration(const Campaign & aCampaign)
{
	// Планируем перезагрузку ТК по истечении рекламы
	if (aCampaign.isValid() && !aCampaign.isExpired() && !aCampaign.expired.isNull())
	{
		toLog(LogLevel::Debug, QString("expTime: %1  camp exp time: %2").arg(mExpirationTime.toString()).arg(aCampaign.expired.toString()));

		if (mExpirationTime.isNull() || mExpirationTime > aCampaign.expired)
		{
			if (mExpirationTimer >= 0)
			{
				killTimer(mExpirationTimer);
			}

			int interval = static_cast<int>(QDateTime::currentDateTime().msecsTo(aCampaign.expired));

			if (interval > 0)
			{
				mExpirationTime = aCampaign.expired;
				toLog(LogLevel::Normal, QString("Start expiration timer on '%1'").arg(mExpirationTime.toString("yyyy-MM-dd hh:mm:ss")));

				mExpirationTimer = startTimer(interval);
			}

		}
	}
}

//------------------------------------------------------------------------
} // Ad
