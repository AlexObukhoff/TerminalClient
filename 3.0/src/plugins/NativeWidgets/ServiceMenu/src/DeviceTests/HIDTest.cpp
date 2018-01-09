/* @file Класс для тестирования сканеров. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTextCodec>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IHIDService.h>
#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/HardwareConstants.h>

// Project
#include "HIDTest.h"

//------------------------------------------------------------------------------
namespace CHIDTest
{
	const QString TestRead = QT_TRANSLATE_NOOP("HIDTest", "#test_read");
}

//------------------------------------------------------------------------------
HIDTest::HIDTest(SDK::Driver::IDevice * aDevice, const QString & aInstancePath)
{
	mHID = dynamic_cast<SDK::Driver::IHID *>(aDevice);

	mTestNames << qMakePair(CHIDTest::TestRead, aInstancePath.contains(SDK::Driver::CComponents::Camera) ? tr("#wait_photo") : tr("#read_barcode"));
}

//------------------------------------------------------------------------------
QList<QPair<QString, QString>> HIDTest::getTestNames() const
{
	return mTestNames;
}

//------------------------------------------------------------------------------
bool HIDTest::run(const QString & aName)
{
	if (aName == CHIDTest::TestRead)
	{
		if (mHID->enable(true))
		{
			mHID->subscribe(SDK::Driver::IHID::DataSignal, this, SLOT(onData(const QVariantMap &)));
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
void HIDTest::stop()
{
	mHID->unsubscribe(SDK::Driver::IHID::DataSignal, this);
	mHID->enable(false);
}

//------------------------------------------------------------------------------
bool HIDTest::isReady()
{
	return mHID && mHID->isDeviceReady();
}

//------------------------------------------------------------------------------
bool HIDTest::hasResult()
{
	return true;
}

//------------------------------------------------------------------------------
void HIDTest::onData(const QVariantMap & aDataMap)
{
	QString value;

	if (aDataMap.contains(CHardwareSDK::HID::Text))
	{
		const QVariant data = aDataMap.value(CHardwareSDK::HID::Text);

		switch (data.type())
		{
		case QVariant::String:
			value = data.value<QString>();
			break;

		case QVariant::ByteArray:
		{
			QTextCodec::ConverterState stateUtf8;
			QString utf8 = QTextCodec::codecForName("UTF-8")->toUnicode(data.toString().toLatin1(), data.toByteArray().size(), &stateUtf8);
			value = stateUtf8.invalidChars == 0 ? utf8 : QTextCodec::codecForName("windows-1251")->toUnicode(data.toString().toLatin1());
		}
			break;
		default:
			value = data.toString();
		}

		if (!value.isEmpty())
		{
			emit result("", QString("%1 %2").arg(tr("#readed")).arg(value));
		}
	}
	
	if (aDataMap.contains(CHardwareSDK::HID::Image))
	{
		bool faceDetected = aDataMap.value(CHardwareSDK::HID::FaceDetected, false).toBool();

		emit result("", aDataMap.value(faceDetected ? CHardwareSDK::HID::ImageWithFaceArea : CHardwareSDK::HID::Image));
	}
}

//------------------------------------------------------------------------------
