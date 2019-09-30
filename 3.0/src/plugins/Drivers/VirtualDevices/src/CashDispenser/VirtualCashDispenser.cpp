/* @file Виртуальный диспенсер. */

// STL
#include <algorithm>

// Modules
#include "Hardware/Dispensers/DispenserStatusCodes.h"
#include "Hardware/Dispensers/DispenserData.h"

// Project
#include "VirtualCashDispenser.h"

//---------------------------------------------------------------------------
VirtualDispenser::VirtualDispenser() : mJammedItem(0), mNearEndCount(0)
{
	mDeviceName = "Virtual cash dispenser";
}

//---------------------------------------------------------------------------------
void VirtualDispenser::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	TVirtualDispenser::setDeviceConfiguration(aConfiguration);

	mJammedItem = aConfiguration.value(CHardware::Dispenser::JammedItem, mJammedItem).toInt();
	mNearEndCount = aConfiguration.value(CHardware::Dispenser::NearEndCount, mNearEndCount).toInt();
	mUnits = aConfiguration.value(CHardware::Dispenser::Units, mUnits).toInt();
}

//---------------------------------------------------------------------------
void VirtualDispenser::applyUnitList()
{
	moveToThread(&mThread);

	START_IN_WORKING_THREAD(applyUnitList)

	if (!mUnitConfigData.isEmpty())
	{
		adjustUnitList(true);
	}

	moveToThread(qApp->thread());
}


//--------------------------------------------------------------------------------
void VirtualDispenser::performDispense(int aUnit, int aItems)
{
	moveToThread(&mThread);

	if (!isWorkingThread())
	{
		QMetaObject::invokeMethod(this, "performDispense", Qt::QueuedConnection, Q_ARG(int, aUnit), Q_ARG(int, aItems));

		return;
	}

	moveToThread(qApp->thread());

	int dispensedItems = 0;

	if (!mStatusCodes.contains(DispenserStatusCode::Error::Jammed))
	{
		dispensedItems = qMin(aItems, mUnitData[aUnit]);

		if (mJammedItem && (dispensedItems >= mJammedItem))
		{
			mStatusCodes.insert(DispenserStatusCode::Error::Jammed);
			dispensedItems = mJammedItem - 1;
		}

		mUnitData[aUnit] -= dispensedItems;
	}

	SleepHelper::msleep(CVirtualDispenser::ItemDispenseDelay * dispensedItems);

	if (mUnitData[aUnit])
	{
		mUnitData[aUnit]--;
		toLog(LogLevel::Warning, QString("%1: Send rejected 1 item from %2 unit").arg(mDeviceName).arg(aUnit));

		emit rejected(aUnit, 1);
	}

	if (!mUnitData[aUnit])
	{
		toLog(LogLevel::Warning, QString("%1: Send emptied unit %2").arg(mDeviceName).arg(aUnit));

		emit unitEmpty(aUnit);
	}

	toLog(LogLevel::Normal, QString("%1: Send dispensed %2 item(s) from %3 unit").arg(mDeviceName).arg(dispensedItems).arg(aUnit));

	emit dispensed(aUnit, dispensedItems);

	onPoll();
}

//--------------------------------------------------------------------------------
void VirtualDispenser::checkUnitStatus(TStatusCodes & aStatusCodes, int aUnit)
{
	if ((mUnitData.size() > aUnit) && (mUnitData[aUnit] <= mNearEndCount))
	{
		aStatusCodes.insert(CDispenser::StatusCodes::Data[aUnit].nearEmpty);
	}

	TVirtualDispenser::checkUnitStatus(aStatusCodes, aUnit);
}

//--------------------------------------------------------------------------------
void VirtualDispenser::filterKeyEvent(int aKey, const Qt::KeyboardModifiers & aModifiers)
{
	if (aModifiers & Qt::AltModifier)
	{
		switch (aKey)
		{
			case Qt::Key_F1: { changeStatusCode(DispenserStatusCode::Error::Unit0Opened); break; }
			case Qt::Key_F2: { changeStatusCode(DispenserStatusCode::Error::Unit1Opened); break; }
			case Qt::Key_F3: { changeStatusCode(DispenserStatusCode::Error::Unit2Opened); break; }
			case Qt::Key_F4: { changeStatusCode(DispenserStatusCode::Error::Unit3Opened); break; }

			case Qt::Key_F7: { changeStatusCode(DispenserStatusCode::Error::RejectingOpened); break; }
			case Qt::Key_F8: { changeStatusCode(DispenserStatusCode::Error::Jammed); break; }
			case Qt::Key_F9: { changeStatusCode(DeviceStatusCode::Error::NotAvailable); break; }
		}
	}
	else if (aModifiers & Qt::ShiftModifier)
	{
		switch (aKey)
		{
			case Qt::Key_F1: { changeStatusCode(DispenserStatusCode::Warning::Unit0Empty); break; }
			case Qt::Key_F2: { changeStatusCode(DispenserStatusCode::Warning::Unit1Empty); break; }
			case Qt::Key_F3: { changeStatusCode(DispenserStatusCode::Warning::Unit2Empty); break; }
			case Qt::Key_F4: { changeStatusCode(DispenserStatusCode::Warning::Unit3Empty); break; }
		}
	}
}

//--------------------------------------------------------------------------------
