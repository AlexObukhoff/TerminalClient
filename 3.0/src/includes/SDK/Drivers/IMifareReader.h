/* @file Интерфейс драйвера PC/SC ридера. */
#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QByteArray>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/ICardReader.h>

//------------------------------------------------------------------------------
namespace SDK {
namespace Driver {

class IMifareReader : public ICardReader
{
public:
	/// Сброс карты по питанию.
	virtual bool reset(QByteArray & aAnswer) = 0;

	/// Произвести обмен данными с картой
	virtual bool communicate(const QByteArray & aSendMessage, QByteArray & aReceiveMessage) = 0;

protected:
	virtual ~IMifareReader() {}
};

}} // namespace SDK::Driver
