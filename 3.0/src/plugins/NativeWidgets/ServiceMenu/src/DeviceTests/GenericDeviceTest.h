/* @file Обобщенный тест устройств. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtConcurrentRun>
#include <QtCore/QFutureWatcher>
#include <Common/QtHeadersEnd.h>

// SDK
#include <Common/ObjectPointer.h>
#include <SDK/PaymentProcessor/IDeviceTest.h>

namespace SDK {
namespace Driver {
	class IDevice;
}}

//------------------------------------------------------------------------------
class GenericDeviceTest : public SDK::PaymentProcessor::IDeviceTest
{
	Q_OBJECT

public:
	GenericDeviceTest(SDK::Driver::IDevice * aDevice);
	virtual ~GenericDeviceTest();

	/// Возвращает имена и описания тестов.
	virtual QList<QPair<QString, QString>> getTestNames() const;

	virtual bool run(const QString & aTestName);
	
	virtual void stop();

	virtual bool isReady();

	virtual bool hasResult();

private slots:
	void onTestFinished();

private:
	QFutureWatcher<void> mResult;
	ObjectPointer<SDK::Driver::IDevice> mDevice;

	const QString mGenericTest;
};

//------------------------------------------------------------------------------
