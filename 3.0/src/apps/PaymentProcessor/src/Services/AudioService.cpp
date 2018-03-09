/* @file Менеджер для работы со звуком. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <Common/QtHeadersEnd.h>

// Проект
#include "System/IApplication.h"
#include "System/SettingsConstants.h"
#include "Services/ServiceNames.h"
#include "Services/AudioService.h"

//---------------------------------------------------------------------------
namespace CAudioService
{
	/// Название лога.
	const char LogName[] = "Interface";
}

//---------------------------------------------------------------------------
AudioService * AudioService::instance(IApplication * aApplication)
{
	return static_cast<AudioService *>(aApplication->getCore()->getService(CServices::AudioService));
}

//---------------------------------------------------------------------------
AudioService::AudioService(IApplication * aApplication)
	: mApplication(aApplication),
	  ILogable(CAudioService::LogName)
{
	// Получаем директорию с файлами интерфейса из настроек.
	mInterfacePath = IApplication::toAbsolutePath(mApplication->getSettings().value(CSettings::InterfacePath).toString());
}

//---------------------------------------------------------------------------
AudioService::~AudioService()
{
}

//---------------------------------------------------------------------------
bool AudioService::initialize()
{
	mPlayer = QSharedPointer<QMediaPlayer>(new QMediaPlayer());
	connect(mPlayer.data(), SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(stateChanged(QMediaPlayer::State)));

	return true;
}

//------------------------------------------------------------------------------
void AudioService::finishInitialize()
{
}

//---------------------------------------------------------------------------
bool AudioService::canShutdown()
{
	return true;
}

//---------------------------------------------------------------------------
bool AudioService::shutdown()
{
	return true;
}

//---------------------------------------------------------------------------
QString AudioService::getName() const
{
	return CServices::AudioService;
}

//---------------------------------------------------------------------------
const QSet<QString> & AudioService::getRequiredServices() const
{
	static QSet<QString> requiredResources;
	return requiredResources;
}

//---------------------------------------------------------------------------
QVariantMap AudioService::getParameters() const
{
	return QVariantMap();
}

//---------------------------------------------------------------------------
void AudioService::resetParameters(const QSet<QString> &)
{
}

//---------------------------------------------------------------------------
void AudioService::play(const QString & aFileName)
{
	QString filePath = mInterfacePath + "/" + aFileName;

	if (QFile::exists(filePath))
	{
		if (mPlayer->state() !=  QMediaPlayer::StoppedState)
		{
			stop();
		}

		mPlayer->setMedia(QUrl::fromLocalFile(filePath));
		mPlayer->play();
	}
	else
	{
		stop();

		toLog(LogLevel::Warning, QString("Audio file %1 not found.").arg(aFileName));
	}
}

//---------------------------------------------------------------------------
void AudioService::stop()
{
	if (mPlayer)
	{
		mPlayer->stop();
	}
}

//---------------------------------------------------------------------------
void AudioService::stateChanged(QMediaPlayer::State aState)
{
	if (aState == QMediaPlayer::StoppedState)
	{
	}
}

//---------------------------------------------------------------------------