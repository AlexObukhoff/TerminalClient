/* @file Прокси-класс для работы с оборудованием. */

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IDeviceService.h>
#include <SDK/PaymentProcessor/Scripting/DeviceService.h>

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

//------------------------------------------------------------------------------
DeviceService::DeviceService(ICore * aCore)
	: mCore(aCore),
	  mDeviceService(mCore->getDeviceService())
{
	connect(mDeviceService, SIGNAL(deviceDetected(const QString &)), this, SLOT(onDeviceDetected(const QString &)));
	connect(mDeviceService, SIGNAL(detectionStopped()), this, SLOT(onDetectionStopped()));
}

//------------------------------------------------------------------------------
void DeviceService::detect()
{
	//TODO: 1-й параметр - галка быстрого поиска
	mDeviceService->detect(false);
}

//------------------------------------------------------------------------------
void DeviceService::onDeviceDetected(const QString & aConfigName)
{
	emit deviceDetected(aConfigName);
}

//------------------------------------------------------------------------------
void DeviceService::onDetectionStopped()
{
	emit detectionStopped();
}

//------------------------------------------------------------------------------
}}} // Scripting::PaymentProcessor::SDK
