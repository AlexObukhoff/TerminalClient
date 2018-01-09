/* @file Прокси-класс для работы с оборудованием. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>


namespace SDK {
namespace PaymentProcessor {

class ICore;
class IDeviceService;

namespace Scripting {

//------------------------------------------------------------------------------
class DeviceService: public QObject
{
	Q_OBJECT

public:
	DeviceService(ICore * aCore);

public slots:
	void detect();
	void onDeviceDetected(const QString & aConfigName);
	void onDetectionStopped();

signals:
	void deviceDetected(const QString & aConfigName);
	void detectionStopped();

private:
	ICore * mCore;
	IDeviceService * mDeviceService;
};

//------------------------------------------------------------------------------
}}} // Scripting::PaymentProcessor::SDK
