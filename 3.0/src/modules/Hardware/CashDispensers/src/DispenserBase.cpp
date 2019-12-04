/* @file Диспенсер. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/qmath.h>
#include <QtCore/QWriteLocker>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Dispensers/DispenserStatusesDescriptions.h"
#include "Hardware/CashAcceptors/CashAcceptorStatusesDescriptions.h"

// Project
#include "DispenserBase.h"

using namespace SDK::Driver;

//---------------------------------------------------------------------------
template <class T>
DispenserBase<T>::DispenserBase() : mUnits(0), mNeedGetUnits(false), mUnitError(false)
{
	// описатель статус-кодов
	mStatusCodesSpecification = DeviceStatusCode::PSpecifications(new DispenserStatusCode::CSpecifications());
	DeviceStatusCode::PSpecifications billAcceptorStatusCodeSpecification = DeviceStatusCode::PSpecifications(new BillAcceptorStatusCode::CSpecifications());
	QMap<int, SStatusCodeSpecification> billAcceptorStatusCodeData = billAcceptorStatusCodeSpecification->data();

	for (auto it = billAcceptorStatusCodeData.begin(); it != billAcceptorStatusCodeData.end(); ++it)
	{
		mStatusCodesSpecification->data().insert(it.key(), it.value());
	}

	// восстановимые ошибки
	mRecoverableErrors.insert(DispenserStatusCode::Error::AllUnitsEmpty);
}

//--------------------------------------------------------------------------------
template <class T>
bool DispenserBase<T>::updateParameters()
{
	processDeviceData();
	adjustUnitList(mUnitError || mUnitData.isEmpty());

	if (!mUnits)
	{
		toLog(LogLevel::Error, mDeviceName + ": No units.");
		return false;
	}

	if (!reset())
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to reset.");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool DispenserBase<T>::reset()
{
	return true;
}

//---------------------------------------------------------------------------
template <class T>
void DispenserBase<T>::setUnitList(const TUnitData & aUnitData)
{
	mUnitConfigData = aUnitData;

	applyUnitList();
}

//---------------------------------------------------------------------------
template <class T>
void DispenserBase<T>::applyUnitList()
{
	START_IN_WORKING_THREAD(applyUnitList)

	if (!mUnitConfigData.isEmpty())
	{
		adjustUnitList(true);
	}
}

//---------------------------------------------------------------------------
template <class T>
void DispenserBase<T>::adjustUnitList(bool aConfigData)
{
	mUnitError = !mUnits;

	if (!mUnitError)
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
				emitUnitEmpty(i, " during status filtration");
			}
		}
	}
}

//---------------------------------------------------------------------------
template <class T>
void DispenserBase<T>::emitUnitEmpty(int aUnit, const QString & aLog)
{
	mUnitData[aUnit] = 0;
	toLog(LogLevel::Warning, mDeviceName + QString(": emit emptied unit %1%2").arg(aUnit).arg(aLog));

	emit unitEmpty(aUnit);
}

//---------------------------------------------------------------------------
template <class T>
void DispenserBase<T>::emitDispensed(int aUnit, int aItems, const QString & aLog)
{
	toLog(LogLevel::Normal, mDeviceName + QString(": emit dispensed %1 items from %2 unit%3").arg(aItems).arg(aUnit).arg(aLog));

	emit dispensed(aUnit, aItems);
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

	if ((mInitialized != ERequestStatus::InProcess) && (mUnitData.size() > aUnit) && !mUnitData[aUnit])
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

	aStatusCodes.remove(DispenserStatusCode::OK::SingleMode);
	aStatusCodes.remove(DispenserStatusCode::OK::Locked);

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
		toLog(LogLevel::Warning, mDeviceName + ": emit units defined");

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
	if (!aItems)
	{
		toLog(LogLevel::Error, mDeviceName + ": Nothing for dispense");

		return;
	}

	if (mUnitData.size() <= aUnit)
	{
		toLog(LogLevel::Warning, mDeviceName + QString(": emit emptied unit %1 due to no such unit, need max %2").arg(aUnit).arg(mUnits - 1));

		emit unitEmpty(aUnit);

		return;
	}
	else if (!mUnitData[aUnit])
	{
		emitDispensed(aUnit, 0, " due to unit is empty already");

		return;
	}

	performDispense(aUnit, aItems);
}

//--------------------------------------------------------------------------------
