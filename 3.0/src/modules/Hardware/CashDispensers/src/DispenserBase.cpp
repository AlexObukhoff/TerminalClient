/* @file Диспенсер. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/qmath.h>
#include <QtCore/QWriteLocker>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Dispensers/DispenserData.h"
#include "Hardware/Dispensers/DispenserStatusesDescriptions.h"

#include "Hardware/Dispensers/ProtoDispenser.h"
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/SerialDeviceBase.h"

// Project
#include "DispenserBase.h"

using namespace SDK::Driver;

//-------------------------------------------------------------------------------
template class DispenserBase<SerialDeviceBase<PortPollingDeviceBase<ProtoDispenser>>>;
template class DispenserBase<DeviceBase<ProtoDispenser>>;

//---------------------------------------------------------------------------
template <class T>
DispenserBase<T>::DispenserBase() : mUnits(0), mNeedGetUnits(false), mCashError(false)
{
	mStatusCodesSpecification = DeviceStatusCode::PSpecifications(new DispenserStatusCode::CSpecifications());

	// восстановимые ошибки
	mRecoverableErrors.insert(DispenserStatusCode::Error::AllUnitsEmpty);
}

//--------------------------------------------------------------------------------
template <class T>
bool DispenserBase<T>::updateParameters()
{
	adjustCashList(mCashError || mUnitData.isEmpty());

	return mUnits;
}

//---------------------------------------------------------------------------
template <class T>
void DispenserBase<T>::setCashList(const TUnitData & aUnitData)
{
	mUnitConfigData = aUnitData;

	applyCashList();
}

//---------------------------------------------------------------------------
template <class T>
void DispenserBase<T>::applyCashList()
{
	START_IN_WORKING_THREAD(applyCashList)

	if (!mUnitConfigData.isEmpty())
	{
		adjustCashList(true);
	}
}

//---------------------------------------------------------------------------
template <class T>
void DispenserBase<T>::adjustCashList(bool aConfigData)
{
	mCashError = !mUnits;

	if (!mCashError)
	{
		TUnitData & unitData = aConfigData ? mUnitConfigData : mUnitData;
		mUnitData = unitData.mid(0, mUnits);
		int newUnits = mUnitData.size();

		if (newUnits < mUnits)
		{
			mUnitData << TUnitData(mUnits - newUnits, 0);
		}

		onPoll();

		for (int i = 0; i < unitData.size(); ++i)
		{
			if ((mStatusCollection.contains(CDispenser::StatusCodes::Data[i].empty) ||
			     mStatusCollection.contains(DispenserStatusCode::Error::AllUnitsEmpty)) && unitData[i])
			{
				toLog(LogLevel::Warning, QString("%1: Send emptied unit %2 during status filtration").arg(mDeviceName).arg(i));

				emit unitEmpty(i);
			}
		}
	}
}

//---------------------------------------------------------------------------
template <class T>
bool DispenserBase<T>::isDeviceReady(int aUnit)
{
	MutexLocker locker(&mExternalMutex);

	if (!mPostPollingAction || (mInitialized != ERequestStatus::Success) || (aUnit >= mUnits) || !mStatusCollection.isEmpty(EWarningLevel::Error))
	{
		return false;
	}

	using namespace DispenserStatusCode::Warning;

	auto isEmpty = [&] (int aCDStatusCode, int aCDUnit) -> bool { return mStatusCollection.contains(aCDStatusCode) && (aUnit == aCDUnit); };

	return !((aUnit != -1) && (isEmpty(Unit0Empty, 0) || isEmpty(Unit1Empty, 1) || isEmpty(Unit2Empty, 2) || isEmpty(Unit3Empty, 3)));
}

//--------------------------------------------------------------------------------
template <class T>
void DispenserBase<T>::checkUnitStatus(TStatusCodes & aStatusCodes, int aUnit)
{
	CDispenser::StatusCodes::SData & data = CDispenser::StatusCodes::Data.data()[aUnit];

	if (aStatusCodes.contains(data.empty))
	{
		aStatusCodes.remove(data.nearEmpty);

		if (mUnitData.size() > aUnit)
		{
			mUnitData[aUnit] = 0;
		}
	}

	if ((mUnitData.size() > aUnit) && !mUnitData[aUnit])
	{
		aStatusCodes.insert(data.empty);
		aStatusCodes.remove(data.nearEmpty);
	}
}

//--------------------------------------------------------------------------------
template <class T>
void DispenserBase<T>::cleanStatusCodes(TStatusCodes & aStatusCodes)
{
	using namespace DispenserStatusCode::Warning;
	using namespace DispenserStatusCode::Error;

	for (int i = 0; i < mUnits; ++i)
	{
		checkUnitStatus(aStatusCodes, i);
	}

	TStatusCodes allEmpty = CDispenser::StatusCodes::AllEmpty.mid(0, mUnits).toSet();
	TStatusCodes allNearEmpty = CDispenser::StatusCodes::AllNearEmpty.mid(0, mUnits).toSet();

	if ((aStatusCodes & allEmpty) == allEmpty)
	{
		aStatusCodes -= allEmpty + allNearEmpty;
		aStatusCodes.insert(DispenserStatusCode::Error::AllUnitsEmpty);
	}
	else if ((aStatusCodes & allNearEmpty) == allNearEmpty)
	{
		aStatusCodes -= allNearEmpty;
		aStatusCodes.insert(DispenserStatusCode::Warning::AllUnitsNearEmpty);
	}

	T::cleanStatusCodes(aStatusCodes);
}

//--------------------------------------------------------------------------------
template <class T>
void DispenserBase<T>::postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection)
{
	if (mNeedGetUnits && mUnits)
	{
		mNeedGetUnits = false;
		toLog(LogLevel::Warning, mDeviceName + ": Send units defined");

		emit unitsDefined();
	}

	T::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

//--------------------------------------------------------------------------------
template <class T>
void DispenserBase<T>::emitStatusCodes(TStatusCollection & aStatusCollection, int /*aExtendedStatus*/)
{
	TStatusCodes opened = aStatusCollection.value(EWarningLevel::Error) & CDispenser::StatusCodes::AllOpened;
	int extendedStatus = opened.isEmpty() ? EStatus::Actual : EDispenserStatus::CassetteOpened;

	T::emitStatusCodes(aStatusCollection, extendedStatus);
}

//--------------------------------------------------------------------------------
template <class T>
int DispenserBase<T>::units()
{
	mNeedGetUnits = !mUnits;

	return mUnits;
}

//--------------------------------------------------------------------------------
template <class T>
void DispenserBase<T>::dispense(int aUnit, int aItems)
{
	bool empty = !mUnitData[aUnit];
	
	if ((mUnitData.size() > aUnit) && empty)
	{
		toLog(LogLevel::Warning, QString("%1: Send emptied unit %2, cannot dispense").arg(mDeviceName).arg(aUnit));

		emit unitEmpty(aUnit);
	}

	if ((mUnitData.size() <= aUnit) || empty)
	{
		if (empty)
		{
			toLog(LogLevel::Error, QString("%1: Send dispensed 0 notes due to %2 unit is empty already").arg(mDeviceName).arg(aUnit)); 
		}
		else
		{
			toLog(LogLevel::Error, QString("%1: Send dispensed 0 notes due to wrong unit number = %2, need max %3").arg(mDeviceName).arg(aUnit).arg(mUnits - 1));
		}

		emit dispensed(aUnit, 0);

		return;
	}

	performDispense(aUnit, aItems);
}

//--------------------------------------------------------------------------------
