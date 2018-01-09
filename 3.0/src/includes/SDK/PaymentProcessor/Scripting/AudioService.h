/* @file Прокси-класс для работы со звуком в скриптах. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>


namespace SDK {
namespace PaymentProcessor {

class ICore;
class IAudioService;

namespace Scripting {

//------------------------------------------------------------------------------
class AudioService : public QObject
{
	Q_OBJECT

public:
	AudioService(ICore * aCore);

public slots:
	/// Воспроизводит wav-файл.
	void play(const QString & aFileName);

	/// Остановить вопроизведение.
	void stop();

private:
	ICore * mCore;
	IAudioService * mAudioService;
};

//------------------------------------------------------------------------------
}}} // Scripting::PaymentProcessor::SDK
