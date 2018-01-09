/* @file Обобщенный тест устройств. */

// boost
#include <boost/bind.hpp>

// SDK
#include <SDK/Drivers/IDevice.h>

// Project
#include "GenericDeviceTest.h"

//------------------------------------------------------------------------------
namespace CGenericDeviceTest
{
	const QString GenericTest = QT_TRANSLATE_NOOP("GenericDeviceTest", "#generic_test");
}

//------------------------------------------------------------------------------
GenericDeviceTest::GenericDeviceTest(SDK::Driver::IDevice * aDevice)
	: mDevice(aDevice)
{
	connect(&mResult, SIGNAL(finished()), this, SLOT(onTestFinished()));
}

//------------------------------------------------------------------------------
GenericDeviceTest::~GenericDeviceTest()
{
	mResult.waitForFinished();
}

//------------------------------------------------------------------------------
bool GenericDeviceTest::run(const QString & aTestName)
{
	if (aTestName != CGenericDeviceTest::GenericTest)
	{
		return false;
	}

	QFuture<void> future = QtConcurrent::run(boost::bind(&SDK::Driver::IDevice::initialize, mDevice.data()));
	mResult.setFuture(future);

	return true;
}

//------------------------------------------------------------------------------
void GenericDeviceTest::stop()
{
}

//------------------------------------------------------------------------------
bool GenericDeviceTest::isReady()
{
	return false;
}

//------------------------------------------------------------------------------
bool GenericDeviceTest::hasResult()
{
	return false;
}

//------------------------------------------------------------------------------
void GenericDeviceTest::onTestFinished()
{
	emit result(mGenericTest, tr("#ok"));
}

//------------------------------------------------------------------------------
QList<QPair<QString, QString>> GenericDeviceTest::getTestNames() const
{
	return QList<QPair<QString, QString>>() << qMakePair(CGenericDeviceTest::GenericTest, QString());
}

//------------------------------------------------------------------------------
