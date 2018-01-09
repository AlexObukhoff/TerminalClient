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
	mMusic = QSharedPointer<Phonon::MediaObject>(Phonon::createPlayer(Phonon::MusicCategory));
	connect(mMusic.data(), SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(stateChanged(Phonon::State, Phonon::State)));

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
		if (mMusic->state() ==  Phonon::PlayingState || mMusic->state() == Phonon::LoadingState)
		{
			mMusic->enqueue(Phonon::MediaSource(filePath));
		}
		else
		{
			mMusic->setCurrentSource(Phonon::MediaSource(filePath));
		}
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
	if (mMusic)
	{
		mMusic->clear();
	}
}

//---------------------------------------------------------------------------
void AudioService::stateChanged(Phonon::State aNewstate, Phonon::State)
{
	if (aNewstate == Phonon::StoppedState) // this state will be set when media has loaded clip
	{
		Phonon::MediaSource::Type type = mMusic->currentSource().type();

		if (mMusic->queue().size() > 0 || 
			(type != Phonon::MediaSource::Invalid && type != Phonon::MediaSource::Empty))
		{
			mMusic->play();
		}
	}
}

//---------------------------------------------------------------------------