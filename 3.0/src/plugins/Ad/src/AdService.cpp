/* @file Сервис для работы с рекламой. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICryptService.h>

// Modules
#include <AdBackend/DatabaseUtils.h>
#include <AdBackend/Client.h>
#include <AdBackend/Campaign.h>

// Project
#include "System/IApplication.h"
#include "System/SettingsConstants.h"
#include "Services/ServiceNames.h"
#include "Services/AdService.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
namespace CAdService
{
	/// Название лога.
	const QString LogName = "Ad";

	/// Путь к данным рекламы по умолчанию
	const QString DefaultContentPath = "user/ad";

	/// Путь к данным рекламы по умолчанию
	const QString DefaultUrl = "https://service.cyberplat.ru/cgi-bin/ad/ad.cgi";

	/// Имя файла настроек
	const QString SettingsName = "ad.ini";

	/// Имена параметров в ini, описывающие рекламную кампанию
	namespace Settings
	{
		const QString Url = "ad/Url";
		const QString ContendPath = "ad/path";
	}
}

//---------------------------------------------------------------------------
AdService * AdService::instance(IApplication * aApplication)
{
	return static_cast<AdService *>(aApplication->getCore()->getService(CServices::AdService));
}

//---------------------------------------------------------------------------
AdService::AdService(IApplication * aApplication)
	: mApplication(aApplication),
	  ILogable(CAdService::LogName),
	  mSettings(nullptr)
{
	QString userPath = IApplication::toAbsolutePath(mApplication->getSettings().value(CSettings::UserDataPath).toString());
	mSettings = new QSettings(userPath + QDir::separator() + CAdService::SettingsName, QSettings::IniFormat, this);
	mSettings->setIniCodec("utf-8");
}

//---------------------------------------------------------------------------
AdService::~AdService()
{
}

//---------------------------------------------------------------------------
bool AdService::initialize()
{
	QString userPath = IApplication::toAbsolutePath(mApplication->getSettings().value(CSettings::UserDataPath).toString());
	mDatabase = QSharedPointer<Ad::DatabaseUtils>(new Ad::DatabaseUtils(userPath));

	mClient = QSharedPointer<Ad::Client>(new Ad::Client(mSettings->value(CAdService::Settings::Url, CAdService::DefaultUrl).toUrl(),
		mApplication->getWorkingDirectory() + QDir::separator() + mSettings->value(CAdService::Settings::ContendPath, CAdService::DefaultContentPath).toString(),
		mDatabase.data(), mApplication->getCore(), mSettings, 0));

	mDatabase->addStatisticRecord("terminal_started");

	return true;
}

//------------------------------------------------------------------------------
void AdService::finishInitialize()
{
}

//---------------------------------------------------------------------------
bool AdService::canShutdown()
{
	return true;
}

//---------------------------------------------------------------------------
bool AdService::shutdown()
{
	mClient.clear();
	mDatabase.clear();

	return true;
}

//---------------------------------------------------------------------------
QString AdService::getName() const
{
	return CServices::AdService;
}

//---------------------------------------------------------------------------
const QSet<QString> & AdService::getRequiredServices() const
{
	static QSet<QString> requiredServices = QSet<QString>()
		<< CServices::SettingsService;
	
	return requiredServices;
}

//---------------------------------------------------------------------------
QVariantMap AdService::getParameters() const
{
	return QVariantMap();
}

//---------------------------------------------------------------------------
void AdService::resetParameters(const QSet<QString> &)
{
}

//---------------------------------------------------------------------------
QVariant AdService::getContent(const QString & aName) const
{
	auto channel = mClient->getChannel(aName);

	if (channel.isExpired())

	return QVariant();
}

//---------------------------------------------------------------------------
void AdService::addEvent(const QString & aName)
{
	//TODO
}

//---------------------------------------------------------------------------
