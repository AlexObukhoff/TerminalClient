/* @file Плагин сценария меню платежной книжки */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCryptographicHash>
#include <QtCore/QDir>
#include <Common/QtHeadersEnd.h>

// PaymentProcessor SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>

// Driver SDK
#include <SDK/Drivers/IDevice.h>
#include <SDK/Drivers/HardwareConstants.h>

// Modules
#include "UpdateEngine/ReportBuilder.h"

// Project
#include "System/IApplication.h"
#include "TerminalService.h"
#include "GUIService.h"
#include "DeviceService.h"
#include "EventService.h"
#include "FirmwareUploadScenario.h"

namespace PPSDK = SDK::PaymentProcessor;

//--------------------------------------------------------------------------
namespace CFirmwareUploadScenario
{
	const QString Name = "FirmwareUpload";

	const int UploadRetryTimeout = 10 * 1000;
	const int DeviceInitializedTimeout = 10 * 60 * 1000;
}

//---------------------------------------------------------------------------
FirmwareUploadScenario::FirmwareUploadScenario(IApplication * aApplication) :
	Scenario(CFirmwareUploadScenario::Name, ILog::getInstance(CFirmwareUploadScenario::Name)),
	mApplication(aApplication),
	mRetryCount(2),
	mDevice(nullptr)
{
	mReportBuilder = new ReportBuilder(mApplication->getWorkingDirectory());
}

//---------------------------------------------------------------------------
FirmwareUploadScenario::~FirmwareUploadScenario()
{
	delete mReportBuilder;
}

//---------------------------------------------------------------------------
bool FirmwareUploadScenario::initialize(const QList<GUI::SScriptObject> & /*aScriptObjects*/)
{
	return true;
}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::start(const QVariantMap & aContext)
{
	Q_UNUSED(aContext);

	mTimeoutTimer.stop(); 

	lockGUI();

	mCommand = RemoteService::instance(mApplication)->findUpdateCommand(PPSDK::IRemoteService::FirmwareUpload);

	if (!mCommand.isValid())
	{
		toLog(LogLevel::Error, "Not found active FirmwareUpload command.");

		GUIService::instance(mApplication)->disable(false);

		QVariantMap parameters;
		parameters.insert("result", "abort");
		emit finished(parameters);

		return;
	}

	toLog(LogLevel::Normal, QString("Command %1 loaded.").arg(mCommand.ID));

	mReportBuilder->open(QString::number(mCommand.ID), mCommand.configUrl.toString(), mCommand.parameters.at(2));
	mReportBuilder->setStatus(PPSDK::IRemoteService::Executing);

	QFile firmwareFile(mApplication->getWorkingDirectory() + "/update/" + mCommand.parameters.at(1) + "/firmware.bin");
	if (firmwareFile.open(QIODevice::ReadOnly))
	{
		mFirmware = firmwareFile.readAll();
		firmwareFile.close();

		toLog(LogLevel::Normal, QString("Firmware readed from file. size=%1.").arg(mFirmware.size()));
	}
	else
	{
		mRetryCount = 0;
		abortScenario(QString("Error open file %1.").arg(firmwareFile.fileName()));
		return;
	}

	if (QCryptographicHash::hash(mFirmware, QCryptographicHash::Md5).toHex().toLower() != mCommand.parameters.at(2).toLower())
	{
		mRetryCount = 0;
		abortScenario("Firmware MD5 verify failed.");
		return;
	}

	toLog(LogLevel::Normal, QString("Firmware MD5 verify OK. (%1).").arg(mCommand.parameters.at(2)));

	QMetaObject::invokeMethod(this, "acquireDevice", Qt::QueuedConnection);
	return;
}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::stop() 
{
	if (mReportBuilder)
	{
		mReportBuilder->close();
	}
}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::pause()
{
}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::resume(const QVariantMap & /*aContext*/)
{
}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::signalTriggered(const QString & /*aSignal*/, const QVariantMap & /*aArguments*/)
{
}

//---------------------------------------------------------------------------
QString FirmwareUploadScenario::getState() const
{
	return QString("main");
}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::onTimeout()
{
}

//--------------------------------------------------------------------------
bool FirmwareUploadScenario::canStop()
{
	return false;
}

//--------------------------------------------------------------------------
void FirmwareUploadScenario::lockGUI()
{
	auto guiService = GUIService::instance(mApplication);

	// Показываем экран блокировки и определяем, что делать дальше.
	guiService->show("SplashScreen", QVariantMap());

	QVariantMap parameters;
	parameters.insert("firmware_upload", true);

	guiService->notify("SplashScreen", parameters);
}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::abortScenario(const QString & aErrorMessage)
{
	toLog(LogLevel::Error, aErrorMessage);

	if (--mRetryCount > 0)
	{
		toLog(LogLevel::Normal, QString("Retry upload throw %1 sec").arg(CFirmwareUploadScenario::UploadRetryTimeout / 1000));

		if (mDevice)
		{
			QTimer::singleShot(CFirmwareUploadScenario::UploadRetryTimeout, this, SLOT(onDeviceInitialized()));
		}
		else
		{
			QTimer::singleShot(CFirmwareUploadScenario::UploadRetryTimeout, this, SLOT(acquireDevice()));
		}

		return;
	}

	killTimer(mDeviceInitializedTimer);

	if (mDevice)
	{
		disconnect(dynamic_cast<QObject *>(mDevice), SDK::Driver::IDevice::InitializedSignal, this, SLOT(onDeviceInitialized()));
		disconnect(dynamic_cast<QObject *>(mDevice), SDK::Driver::IDevice::UpdatedSignal, this, SLOT(onUpdated(bool)));

		DeviceService::instance(mApplication)->releaseDevice(mDevice);
		mDevice = nullptr;
	}

	mReportBuilder->setStatusDescription(aErrorMessage);
	mReportBuilder->setStatus(PPSDK::IRemoteService::Error);

	cleanFirmwareArtifacts();

	EventService::instance(mApplication)->sendEvent(PPSDK::Event(PPSDK::EEventType::Restart));
	
	QVariantMap parameters;
	parameters.insert("result", "abort");
	emit finished(parameters);
}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::acquireDevice()
{
	SDK::Driver::IDevice * device = nullptr;
	QString deviceConfig;

	foreach (auto config, DeviceService::instance(mApplication)->getConfigurations())
	{
		if (config.contains(mCommand.parameters.at(1)))
		{
			deviceConfig = config;
			device = DeviceService::instance(mApplication)->acquireDevice(config);
			break;
		}
	}

	if (device == nullptr)
	{
		abortScenario(QString("Not found device with GUID:%1.").arg(mCommand.parameters.at(1)));
		return;
	}

	toLog(LogLevel::Normal, QString("Device '%1' acquire OK.").arg(device->getName()));

	if (!connect(dynamic_cast<QObject *>(device), SDK::Driver::IDevice::UpdatedSignal, this, SLOT(onUpdated(bool))))
	{
		abortScenario("Fail connect to device UpdatedSignal.");
		return;
	}

	toLog(LogLevel::Normal, "Device UpdatedSignal connected.");

	// Проверяем в каком состоянии находится устройство
	auto status = DeviceService::instance(mApplication)->getDeviceStatus(deviceConfig);
	if (status && status->isMatched(SDK::Driver::EWarningLevel::Warning))
	{
		toLog(LogLevel::Normal, QString("Device '%1' have status: %2.").arg(device->getName()).arg(status->description()));

		mDevice = device;

		QMetaObject::invokeMethod(this, "onDeviceInitialized", Qt::QueuedConnection);
	}
	else
	{

		if (!connect(dynamic_cast<QObject *>(device), SDK::Driver::IDevice::InitializedSignal, this, SLOT(onDeviceInitialized())))
		{
			abortScenario("Fail connect to device Initialized signal.");
			return;
		}

		toLog(LogLevel::Normal, "Device Initialized signal connected.");

		mDevice = device;

		auto configuration = device->getDeviceConfiguration();
		int waitInitializedTimeout = configuration.contains(CHardwareSDK::WaitUpdatingTimeout) ? 
			configuration.value(CHardwareSDK::WaitUpdatingTimeout).toInt() : CFirmwareUploadScenario::DeviceInitializedTimeout;

		// Здесь ждём инициализации устройства -> onDeviceInitialized()
		// Если не инициализировались в течении DeviceInitializedTimeout, то обрываем сценарий
		mDeviceInitializedTimer = startTimer(waitInitializedTimeout);

		toLog(LogLevel::Normal, "Wait for device initialize...");
	}
}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::timerEvent(QTimerEvent * aEvent)
{
	if (aEvent->timerId() == mDeviceInitializedTimer)
	{
		killTimer(mDeviceInitializedTimer);

		abortScenario("Device initialization timeout.");
	}
}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::onDeviceInitialized()
{
	toLog(LogLevel::Normal, "Device initialized.");

	killTimer(mDeviceInitializedTimer);
	disconnect(dynamic_cast<QObject *>(mDevice), SDK::Driver::IDevice::InitializedSignal, this, SLOT(onDeviceInitialized()));

	if (!mDevice)
	{
		abortScenario("onDeviceInitialized but mDevice isNULL.");
		return;
	}

	if (!mDevice->canUpdateFirmware())
	{
		abortScenario("Device can't update.");
		return;
	}

	toLog(LogLevel::Normal, "START upload firmware.");

	mDevice->updateFirmware(mFirmware);
}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::onUpdated(bool aSuccess)
{
	if (aSuccess)
	{
		toLog(LogLevel::Normal, "Firmware upload OK.");

		mReportBuilder->setStatusDescription("OK");
		mReportBuilder->setStatus(PPSDK::IRemoteService::OK);
	}
	else
	{
		toLog(LogLevel::Error, "Firmware upload failed. See the log for details device errors.");

		mReportBuilder->setStatusDescription("Fail");
		mReportBuilder->setStatus(PPSDK::IRemoteService::Error);
	}

	cleanFirmwareArtifacts();

	EventService::instance(mApplication)->sendEvent(PPSDK::Event(PPSDK::EEventType::Restart));
	
	QVariantMap parameters;
	parameters.insert("result", aSuccess ? "ok" : "error");
	emit finished(parameters);
}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::cleanFirmwareArtifacts()
{
	killTimer(mDeviceInitializedTimer);

	if (mCommand.isValid())
	{
		QDir updateDir(mApplication->getWorkingDirectory() + "/update/" + mCommand.parameters.at(1));

		updateDir.remove("firmware.bin");
		updateDir.cdUp();
		updateDir.rmdir(mCommand.parameters.at(1));
	}
}

//---------------------------------------------------------------------------
