/* @file Класс для тестирования диспенсеров. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IHIDService.h>

// Project
#include "DispenserTest.h"

//------------------------------------------------------------------------------
namespace CDispenserTest
{
	const QString TestDispense = QT_TRANSLATE_NOOP("DispenserTest", "#dispense");
}

//------------------------------------------------------------------------------
DispenserTest::DispenserTest(SDK::Driver::IDevice * aDevice, const QString & aConfigurationName, SDK::PaymentProcessor::ICore * aCore) :
	mConfigurationName(aConfigurationName),
	mCore(aCore)
{
	mDispenser = dynamic_cast<SDK::Driver::IDispenser *>(aDevice);
}

//------------------------------------------------------------------------------
QList<QPair<QString, QString>> DispenserTest::getTestNames() const
{
	return QList<QPair<QString, QString>>() << qMakePair(CDispenserTest::TestDispense, tr("#dispense"));
}

//------------------------------------------------------------------------------
bool DispenserTest::run(const QString & aName)
{
	mResults.clear();

	if (aName == CDispenserTest::TestDispense)
	{
		if (mDispenser->isDeviceReady())
		{
			mDispenser->subscribe(SDK::Driver::IDispenser::DispensedSignal, this, SLOT(onDispensed(int, int)));
			mDispenser->subscribe(SDK::Driver::IDispenser::RejectedSignal, this, SLOT(onRejected(int, int)));

			// Выдаём из диспенсера по одной купюре из каждого ящика.
			for (int i = 0; i < mDispenser->units(); ++i)
			{
				mDispenser->dispense(i, 1);
			}

			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
void DispenserTest::stop()
{
	// mHID->unsubscribe(SDK::Driver::IHID::DataSignal, this);
}

//------------------------------------------------------------------------------
bool DispenserTest::isReady()
{
	return mDispenser && mDispenser->isDeviceReady();
}

//------------------------------------------------------------------------------
bool DispenserTest::hasResult()
{
	return true;
}

//------------------------------------------------------------------------------
void DispenserTest::onDispensed(int aCashUnit, int aCount)
{
	mResults << QString("%1 from slot %2 count: %3").arg(tr("#dispensed")).arg(aCashUnit).arg(aCount);

	emit result("", mResults.join("\n"));
}

//------------------------------------------------------------------------------
void DispenserTest::onRejected(int aCashUnit, int aCount)
{
	mResults << QString("%1 from slot %2 count: %3").arg(tr("#rejected")).arg(aCashUnit).arg(aCount);

	emit result("", mResults.join("\n"));
}

//------------------------------------------------------------------------------
