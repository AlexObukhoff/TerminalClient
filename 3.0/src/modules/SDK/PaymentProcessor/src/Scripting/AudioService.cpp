/* @file Прокси-класс для работы со звуком в скриптах. */

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IAudioService.h>
#include <SDK/PaymentProcessor/Scripting/AudioService.h>

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

//------------------------------------------------------------------------------
AudioService::AudioService(ICore * aCore) :
	mCore(aCore),
	mAudioService(nullptr /*mCore->getAudioService()*/)
{
}

//------------------------------------------------------------------------------
void AudioService::play(const QString & aFileName)
{
	mAudioService->play(aFileName);
}

//------------------------------------------------------------------------------
void AudioService::stop()
{
	mAudioService->stop();
}

//------------------------------------------------------------------------------

}}} // Scripting::PaymentProcessor::SDK
