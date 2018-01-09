/* @file Интерфейс, обеспечивающий взаимодействие со звуком. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
class IAudioService
{
public:
	/// Воспроизвести звуковой файл.
	virtual void play(const QString & aFileName) = 0;

	/// Остановить вопроизведение.
	virtual void stop() = 0;

protected:
	virtual ~IAudioService() {}
};

//------------------------------------------------------------------------------
}} // SDK::PaymentProcessor
