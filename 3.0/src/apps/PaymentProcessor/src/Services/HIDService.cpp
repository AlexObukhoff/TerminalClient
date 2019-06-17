/* @file Cервис для работы с HID-устройствами. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTextCodec>
#include <Common/QtHeadersEnd.h>

// PaymentProcessor SDK
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

// Driver SDK
#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/ICardReader.h>

// Project
#include "System/IApplication.h"
#include "Services/HIDService.h"
#include "Services/ServiceNames.h"
#include "Services/DeviceService.h"
#include "Services/SettingsService.h"

namespace PPSDK = SDK::PaymentProcessor;

namespace CHIDService
{
	const QString Name = "HID";
};

//---------------------------------------------------------------------------
HIDService * HIDService::instance(IApplication * aApplication)
{
	return static_cast<HIDService *>(aApplication->getCore()->getService(CServices::HIDService));
}

//---------------------------------------------------------------------------
HIDService::HIDService(IApplication * aApplication) :
	mApplication(aApplication),
	mDeviceService(0),
	ILogable(CHIDService::Name)
{
}

//---------------------------------------------------------------------------
HIDService::~HIDService()
{
}

//---------------------------------------------------------------------------
bool HIDService::initialize()
{
	mDeviceService = DeviceService::instance(mApplication);

	updateHardwareConfiguration();

	connect(mDeviceService, SIGNAL(configurationUpdated()), SLOT(updateHardwareConfiguration()));

	return true;
}

//------------------------------------------------------------------------------
void HIDService::finishInitialize()
{
}

//---------------------------------------------------------------------------
void HIDService::updateHardwareConfiguration()
{
	mHIDs.clear();
	mCardReaders.clear();

	toLog(LogLevel::Normal, "Initialize HIDs...");

	// Получаем список всех доступных HID-устройств.
	PPSDK::TerminalSettings * settings = SettingsService::instance(mApplication)->getAdapter<PPSDK::TerminalSettings>();

	foreach (const QString & configurationName, settings->getDeviceList().filter(QRegExp(SDK::Driver::CComponents::Scanner)))
	{
		SDK::Driver::IHID * device = dynamic_cast<SDK::Driver::IHID *>(mDeviceService->acquireDevice(configurationName));

		if (device)
		{
			mHIDs.append(device);

			// Подписываемся на сигнал от устройства.
			device->subscribe(SDK::Driver::IHID::DataSignal, this, SIGNAL(data(const QVariantMap &)));
			toLog(LogLevel::Normal, QString("Device '%1' added...").arg(device->getName()));
		}
		else
		{
			toLog(LogLevel::Error, QString("Failed to acquire HID device %1.").arg(configurationName));
		}
	}

	toLog(LogLevel::Normal, "Initialize CardReaders...");

	// Добавляем картридеры
	foreach (const QString & configurationName, settings->getDeviceList().filter(QRegExp(SDK::Driver::CComponents::CardReader)))
	{
		SDK::Driver::ICardReader * device = dynamic_cast<SDK::Driver::ICardReader *>(mDeviceService->acquireDevice(configurationName));

		if (device)
		{
			mCardReaders.append(device);

			// Подписываемся на сигнал от устройства.
			device->subscribe(SDK::Driver::ICardReader::InsertedSignal, this, SLOT(onCardInserted(SDK::Driver::ECardType::Enum, const QVariantMap &)));
			device->subscribe(SDK::Driver::ICardReader::EjectedSignal, this, SLOT(onCardEjected()));
			device->subscribe(SDK::Driver::IDevice::StatusSignal, this, SLOT(onStatusChanged(SDK::Driver::EWarningLevel::Enum, const QString &, int)));

			toLog(LogLevel::Normal, QString("Device '%1' added...").arg(device->getName()));
		}
		else
		{
			toLog(LogLevel::Error, QString("Failed to acquire CardReader device %1.").arg(configurationName));
		}
	}
}

//---------------------------------------------------------------------------
void HIDService::onStatusChanged(DSDK::EWarningLevel::Enum /*aLevel*/, const QString & /*aTranslation*/, int aStatus)
{
	if (aStatus == DSDK::ECardReaderStatus::NeedReloading)
	{
		emit error();
	}
}

//---------------------------------------------------------------------------
bool HIDService::canShutdown()
{
	return true;
}

//---------------------------------------------------------------------------
bool HIDService::shutdown()
{
	toLog(LogLevel::Normal, "Shutting down HIDs...");

	foreach (SDK::Driver::IHID * HID, mHIDs)
	{
		mDeviceService->releaseDevice(HID);
	}

	mHIDs.clear();

	foreach (auto cardReader, mCardReaders)
	{
		mDeviceService->releaseDevice(cardReader);
	}

	mCardReaders.clear();

	return true;
}

//---------------------------------------------------------------------------
QString HIDService::getName() const
{
	return CServices::HIDService;
}

//---------------------------------------------------------------------------
const QSet<QString> & HIDService::getRequiredServices() const
{
	static QSet<QString> requiredServices = QSet<QString>()
		<< CServices::SettingsService
		<< CServices::DeviceService;

	return requiredServices;
}

//---------------------------------------------------------------------------
QVariantMap HIDService::getParameters() const
{
	return QVariantMap();
}

//---------------------------------------------------------------------------
void HIDService::resetParameters(const QSet<QString> &)
{
}

//---------------------------------------------------------------------------
bool HIDService::setEnable(bool aEnable, const QString & aDevice)
{
	bool result = false;

	if (!aDevice.isEmpty())
	{
		// Получаем список всех доступных HID-устройств.
		PPSDK::TerminalSettings * settings = SettingsService::instance(mApplication)->getAdapter<PPSDK::TerminalSettings>();

		foreach(const QString & configurationName, settings->getDeviceList().filter(QRegExp(aDevice)))
		{
			SDK::Driver::IHID * device = dynamic_cast<SDK::Driver::IHID *>(mDeviceService->acquireDevice(configurationName));

			if (device)
			{
				// Подписываемся на сигнал от устройства.
				device->subscribe(SDK::Driver::IHID::DataSignal, this, SIGNAL(data(const QVariantMap &)));

				if (device->enable(aEnable))
				{
					result = true;
				}
			}
		}
	}
	else
	{
		// TODO когда появится контекст - конкретизируем
		foreach (SDK::Driver::IHID * HID, mHIDs)
		{
			if (HID->enable(aEnable))
			{
				result = true;
			}
		}
	}

	toLog(LogLevel::Debug, QString("Update HID device state: name [%1], state [%2], result: [%3]")
		.arg(aDevice)
		.arg(aEnable ? "ENABLE" : "DISABLE")
		.arg(result ? "OK" : "ERROR"));

	return result;
}

//------------------------------------------------------------------------------
QString HIDService::valueToString(const QVariant & aData)
{
	if (aData.type() == QVariant::ByteArray)
	{
		QTextCodec::ConverterState stateUtf8;
		QString utf8 = QTextCodec::codecForName("UTF-8")->toUnicode(aData.toString().toLatin1(), aData.toByteArray().size(), &stateUtf8);
		return stateUtf8.invalidChars == 0 ? utf8 : QTextCodec::codecForName("windows-1251")->toUnicode(aData.toString().toLatin1());
	}

	return aData.toString();
}

//------------------------------------------------------------------------------
void HIDService::onCardInserted(SDK::Driver::ECardType::Enum aCardType, const QVariantMap & aData)
{
	SDK::Driver::ICardReader * cardReader = dynamic_cast<SDK::Driver::ICardReader *>(sender());

	if (cardReader)
	{
		toLog(LogLevel::Normal, QString("Card type %1 inserted into '%2'.").arg(aCardType).arg(cardReader->getName()));

		if (!aData.isEmpty())
		{
#ifdef _DEBUG
			foreach (auto track, aData.keys())
			{
				toLog(LogLevel::Debug, QString("Card data %1 : '%2'.").arg(track).arg(aData.value(track).toString()));
			}
#endif
			emit inserted(aData);
		}
	}
}

//------------------------------------------------------------------------------
void HIDService::onCardEjected()
{
	SDK::Driver::ICardReader * cardReader = dynamic_cast<SDK::Driver::ICardReader *>(sender());

	if (cardReader)
	{
		toLog(LogLevel::Normal, QString("Card ejected from '%1'.").arg(cardReader->getName()));

		emit ejected();
	}
}

//---------------------------------------------------------------------------
