/* @file Базовый класс устройств приема денег. */

// STL
#include <numeric>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtAlgorithms>
#include <Common/QtHeadersEnd.h>

// Project
#include "Hardware/CashAcceptors/CashAcceptorStatusesDescriptions.h"
#include "CashAcceptorBase.h"

using namespace SDK::Driver;

//---------------------------------------------------------------------------
template <class T>
CashAcceptorBase<T>::CashAcceptorBase()
{
	// данные устройства
	mDeviceName = "Base cash acceptor";
	mCurrencyError = ECurrencyError::OK;
	mReady = false;

	// описания для кодов статусов
	mStatusCodesSpecification = DeviceStatusCode::PSpecifications(new BillAcceptorStatusCode::CSpecifications());
	mDeviceType = CHardware::Types::CashAcceptor;

	// параметры истории статусов
	mStatusHistory.setSize(5);

	// восстановимые ошибки
	mRecoverableErrors.insert(BillAcceptorStatusCode::Error::ParTableLoading);

	// Неустойчивые пограничные состояния
	for (auto it = mStatusCodesSpecification->data().begin(); it != mStatusCodesSpecification->data().end(); ++it)
	{
		if ((it->status == ECashAcceptorStatus::Cheated) ||
		    (it->status == ECashAcceptorStatus::Busy) ||
		    (it->status == ECashAcceptorStatus::BillOperation) ||
		    (it->status == ECashAcceptorStatus::Rejected))
		{
			mUnsafeStatusCodes.insert(it.key());
		}
	}
}

//--------------------------------------------------------------------------------
template <class T>
bool CashAcceptorBase<T>::release()
{
	bool result = T::release();

	MutexLocker locker(&mResourceMutex);

	mStatusHistory.clear();
	mStatusHistory.append(CCashAcceptor::SStatusSpecification());
	mStatusHistory.updateLevel();

	mStatuses.clear();
	mCurrencyError = ECurrencyError::OK;

	return result;
}

//---------------------------------------------------------------------------
template <class T>
bool CashAcceptorBase<T>::isDeviceReady()
{
	return mReady;
}

//---------------------------------------------------------------------------
template <class T>
CCashAcceptor::TStatuses CashAcceptorBase<T>::getLastStatuses(int aLevel) const
{
	if (mStatusCollectionHistory.isEmpty())
	{
		return CCashAcceptor::TStatuses();
	}

	HystoryList<TStatusCollection>::const_iterator lastStatusCollectionIt = mStatusCollectionHistory.end() - qMin(mStatusCollectionHistory.size(), aLevel);
	TStatusCodes lastStatusCodes;

	foreach (const TStatusCodes & statusCodes, *lastStatusCollectionIt)
	{
		lastStatusCodes += statusCodes;
	}

	CCashAcceptor::TStatuses statuses;

	foreach (int statusCode, lastStatusCodes)
	{
		ECashAcceptorStatus::Enum status = static_cast<ECashAcceptorStatus::Enum>(mStatusCodesSpecification->value(statusCode).status);
		statuses[status].insert(statusCode);
	}

	return statuses;
}

//---------------------------------------------------------------------------
template <class T>
TStatusCodes CashAcceptorBase<T>::getLongStatusCodes() const
{
	TStatusCodes result;

	for (auto it = mStatusCodesSpecification->data().begin(); it != mStatusCodesSpecification->data().end(); ++it)
	{
		ECashAcceptorStatus::Enum status = ECashAcceptorStatus::Enum(it->status);

		if (CCashAcceptor::Set::LongStatuses.contains(status))
		{
			result << it.key();
		}
	}

	result -= TStatusCodes()
		<< BillAcceptorStatusCode::BillOperation::Unloaded
		<< BillAcceptorStatusCode::BillOperation::Dispensed
		<< BillAcceptorStatusCode::Busy::SetStackerType
		<< BillAcceptorStatusCode::Busy::Returned;

	return result;
}

//---------------------------------------------------------------------------
template <class T>
bool CashAcceptorBase<T>::canDisable() const
{
	CCashAcceptor::TStatuses lastStatuses = getLastStatuses();

	return lastStatuses[ECashAcceptorStatus::BillOperation].isEmpty() &&
	       lastStatuses[ECashAcceptorStatus::Escrow].isEmpty()        &&
	       lastStatuses[ECashAcceptorStatus::Stacked].isEmpty()       &&
	       lastStatuses[ECashAcceptorStatus::Busy].isEmpty()          &&
	      !lastStatuses[ECashAcceptorStatus::Warning].contains(BillAcceptorStatusCode::Warning::Cheated);
}

//---------------------------------------------------------------------------
template <class T>
bool CashAcceptorBase<T>::isNotEnabled() const
{
	CCashAcceptor::TStatuses lastStatuses = getLastStatuses();

	return !isEnabled() || isDisabled() || lastStatuses[ECashAcceptorStatus::OK].contains(DeviceStatusCode::OK::OK);
}

//---------------------------------------------------------------------------
template <class T>
bool CashAcceptorBase<T>::isEnabled(const CCashAcceptor::TStatuses & aStatuses) const
{
	CCashAcceptor::TStatuses lastStatuses = !aStatuses.isEmpty() ? aStatuses : getLastStatuses();

	if (!std::accumulate(lastStatuses.begin(), lastStatuses.end(), 0, [] (int aStatusAmount, const TStatusCodes & aStatusCodes) -> int
		{ return aStatusAmount + aStatusCodes.size(); }))
	{
		toLog(LogLevel::Normal, QString("No actual last statuses, it is impossible to know the device is turned on"));
		return false;
	}

	return !lastStatuses[ECashAcceptorStatus::Enabled].isEmpty()        ||
	       !lastStatuses[ECashAcceptorStatus::BillOperation].isEmpty()  ||
	       !lastStatuses[ECashAcceptorStatus::Escrow].isEmpty()         ||
	       !lastStatuses[ECashAcceptorStatus::Stacked].isEmpty()        ||
	       !lastStatuses[ECashAcceptorStatus::Cheated].isEmpty()        ||
	        lastStatuses[ECashAcceptorStatus::OperationError].contains(BillAcceptorStatusCode::OperationError::Accept) ||
	        lastStatuses[ECashAcceptorStatus::OperationError].contains(BillAcceptorStatusCode::OperationError::Escrow) ||
	        lastStatuses[ECashAcceptorStatus::OperationError].contains(BillAcceptorStatusCode::OperationError::Stack)  ||
	        lastStatuses[ECashAcceptorStatus::Warning].contains(BillAcceptorStatusCode::Warning::Cheated)              ||
	        lastStatuses[ECashAcceptorStatus::Busy].contains(BillAcceptorStatusCode::Busy::Pause);
}

//---------------------------------------------------------------------------
template <class T>
bool CashAcceptorBase<T>::isNotDisabled() const
{
	CCashAcceptor::TStatuses lastStatuses = getLastStatuses();

	return !isDisabled() || isEnabled() || lastStatuses[ECashAcceptorStatus::OK].contains(DeviceStatusCode::OK::OK);
}

//---------------------------------------------------------------------------
template <class T>
bool CashAcceptorBase<T>::isDisabled(const CCashAcceptor::TStatuses & aStatuses) const
{
	CCashAcceptor::TStatuses lastStatuses = !aStatuses.isEmpty() ? aStatuses : getLastStatuses();

	if (!std::accumulate(lastStatuses.begin(), lastStatuses.end(), 0, [] (int aStatusAmount, const TStatusCodes & aStatusCodes) -> int
		{ return aStatusAmount + aStatusCodes.size(); }))
	{
		toLog(LogLevel::Normal, QString("No actual last statuses, it is impossible to know the device is turned off"));
		return false;
	}

	return !isEnabled() &&
	       (isInitialize()                                                ||
	       !lastStatuses[ECashAcceptorStatus::Disabled].isEmpty()         ||
	       !lastStatuses[ECashAcceptorStatus::Inhibit].isEmpty()          ||
	       !lastStatuses[ECashAcceptorStatus::MechanicFailure].isEmpty()  ||
	       !lastStatuses[ECashAcceptorStatus::StackerOpen].isEmpty()      ||
	       !lastStatuses[ECashAcceptorStatus::StackerFull].isEmpty()      ||
	       !lastStatuses[ECashAcceptorStatus::Error].isEmpty());
}

//---------------------------------------------------------------------------
template <class T>
bool CashAcceptorBase<T>::isInitialize() const
{
	TStatusCodes lastStatusCodes = getLastStatuses()[ECashAcceptorStatus::Busy];

	return lastStatusCodes.contains(DeviceStatusCode::OK::Initialization) ||
	       lastStatusCodes.contains(DeviceStatusCode::OK::Busy) ||
	       lastStatusCodes.contains(BillAcceptorStatusCode::Busy::Calibration);
}

//---------------------------------------------------------------------------
template <class T>
bool CashAcceptorBase<T>::isAvailable()
{
	if (mStatusCollection.isEmpty())
	{
		simplePoll();
	}

	return !mStatusCollection.contains(DeviceStatusCode::Error::NotAvailable);
}

//---------------------------------------------------------------------------
template <class T>
bool CashAcceptorBase<T>::canReturning(bool aOnline)
{
	if (mDeviceType != CHardware::Types::BillAcceptor)
	{
		return false;
	}

	simplePoll();

	TStatusCodes lastStatusCodes = mStatusCollection.value(EWarningLevel::OK);
	bool result = lastStatusCodes.contains(BillAcceptorStatusCode::BillOperation::Escrow) ||
	             (lastStatusCodes.contains(BillAcceptorStatusCode::BillOperation::Accepting) && !aOnline);

	return result;
}

//---------------------------------------------------------------------------
template <class T>
void CashAcceptorBase<T>::logEnabledPars()
{
	MutexLocker locker(&mResourceMutex);

	bool onlyBillAcceptor = true;
	bool enable = false;

	for (auto it = mEscrowParTable.data().begin(); it != mEscrowParTable.data().end(); ++it)
	{
		SPar & par = it.value();

		if (!par.inhibit && par.enabled && par.nominal)
		{
			enable = true;
			onlyBillAcceptor = onlyBillAcceptor && (it.value().cashReceiver == ECashReceiver::BillAcceptor);
		}
	}

	if (enable)
	{
		QString log = mDeviceName + ": successfully enable nominals - ";

		for (auto it = mEscrowParTable.data().begin(); it != mEscrowParTable.data().end(); ++it)
		{
			if (!it.value().inhibit && it.value().enabled && it.value().nominal)
			{
				log += QString("\nnominal %1: currency %2(\"%3\")%4")
					.arg(it->nominal, 5)
					.arg(it->currencyId)
					.arg(it->currency)
					.arg(onlyBillAcceptor ? "" : ((it->cashReceiver == ECashReceiver::BillAcceptor) ? " in bill acceptor" : " in coin acceptor"));
			}
		}

		toLog(LogLevel::Normal, log);
	}
	else
	{
		toLog(LogLevel::Warning, mDeviceName + ": no nominals for enabling!");
	}
}

//---------------------------------------------------------------------------
template <class T>
void CashAcceptorBase<T>::setParList(const TParList & aParList)
{
	MutexLocker locker(&mParListMutex);

	mParList = aParList;

	START_IN_WORKING_THREAD(employParList)
}

//---------------------------------------------------------------------------
template <class T>
TParList CashAcceptorBase<T>::getParList()
{
	MutexLocker parListLocker(&mParListMutex);
	MutexLocker resourceLocker(&mResourceMutex);

	TParList parList = mEscrowParTable.data().values();

	foreach (const SPar & par, mParList)
	{
		if (!parList.contains(par))
		{
			parList << par;
		}
	}

	return parList;
}

//---------------------------------------------------------------------------
template <class T>
void CashAcceptorBase<T>::employParList()
{
	bool noValidParTable = (mCurrencyError == ECurrencyError::Loading) ||
	                       (mCurrencyError == ECurrencyError::Config)  ||
	                       (mCurrencyError == ECurrencyError::Billset) ||
	                       (mCurrencyError == ECurrencyError::Compatibility);

	if (!mConnected || (mInitialized == ERequestStatus::Fail) || noValidParTable)
	{
		return;
	}

	MutexLocker resourceLocker(&mResourceMutex);

	CCashAcceptor::TStatuses lastStatuses = mStatusHistory.lastValue().statuses;
	TStatusCollection lastStatusCollection = mStatusCollectionHistory.lastValue();

	if (!lastStatuses.isEmpty(ECashAcceptorStatus::Rejected) || !lastStatusCollection.isEmpty(EWarningLevel::Error))
	{
		return;
	}

	TParTable oldTable = mEscrowParTable.data();

	{
		MutexLocker parListLocker(&mParListMutex);

		if (mParList.isEmpty())
		{
			return;
		}

		for (auto it = mEscrowParTable.data().begin(); it != mEscrowParTable.data().end(); ++it)
		{
			int index = mParList.indexOf(*it);
			it.value().enabled = (index != -1) && mParList[index].enabled;
		}

		if (std::find_if(mEscrowParTable.data().begin(), mEscrowParTable.data().end(), [&] (const SPar & aPar) -> bool
			{ return aPar.enabled; }) == mEscrowParTable.data().end())
		{
			mCurrencyError = ECurrencyError::NoAvailable;

			return;
		}
	}

	QList<int> positions = mEscrowParTable.data().keys();

	if (std::find_if(positions.begin(), positions.end(), [&](int aPosition) -> bool { return mEscrowParTable[aPosition].isEqual(oldTable[aPosition]); }) != positions.end())
	{
		ECashReceiver::Enum cashReceiver;
		bool complexDevice = 
			(std::find_if(mEscrowParTable.data().begin(), mEscrowParTable.data().end(), [&] (const SPar & aPar) -> bool
				{ bool result = bool(aPar.nominal); cashReceiver = aPar.cashReceiver; return result; }) != mEscrowParTable.data().end()) &&
			(std::find_if(mEscrowParTable.data().begin(), mEscrowParTable.data().end(), [&] (const SPar & aPar) -> bool
				{ return cashReceiver != aPar.cashReceiver; }) != mEscrowParTable.data().end());

		if (std::find_if(mEscrowParTable.data().begin(), mEscrowParTable.data().end(), [&] (const SPar & aPar) -> bool
			{ bool result = aPar.nominal && !aPar.enabled; return result; }) != mEscrowParTable.data().end())
		{
			QString log = mDeviceName + ": Nominal(s) disabled:";

			for (auto it = mEscrowParTable.data().begin(); it != mEscrowParTable.data().end(); ++it)
			{
				if (it->nominal && !it->enabled)
				{
					log += QString("\nnominal %1: currency %2(\"%3\")%4")
						.arg(it->nominal, 5)
						.arg(it->currencyId)
						.arg(it->currency)
						.arg(!complexDevice ? "" : ((it->cashReceiver == ECashReceiver::BillAcceptor) ? " in bill acceptor" : " in coin acceptor"));
				}
			}

			toLog(LogLevel::Normal, log);
		}
	}

	mCurrencyError = ECurrencyError::OK;
}

//--------------------------------------------------------------------------------
template <class T>
ECurrencyError::Enum CashAcceptorBase<T>::processParTable()
{
	if (!containsConfigParameter(CHardwareSDK::CashAcceptor::SystemCurrencyId))
	{
		toLog(LogLevel::Error, mDeviceName + ": No system currency id in parameters!");
		return ECurrencyError::Config;
	}

	int systemCurrencyId = getConfigParameter(CHardwareSDK::CashAcceptor::SystemCurrencyId).toInt();

	if (!CurrencyCodes.data().values().contains(systemCurrencyId))
	{
		toLog(LogLevel::Error, mDeviceName + ": Unknown system currency id = " + QString::number(systemCurrencyId));
		return ECurrencyError::Config;
	}

	QString log = mDeviceName + ": successfully loaded nominal table -";

	MutexLocker locker(&mResourceMutex);

	mEscrowParTable.data().clear();

	if (!loadParTable())
	{
		return ECurrencyError::Loading;
	}

	if (mEscrowParTable.data().isEmpty())
	{
		toLog(LogLevel::Error, mDeviceName + ": par table is empty!");
		return ECurrencyError::Loading;
	}

	bool billset = false;
	bool compability = false;

	for (auto it = mEscrowParTable.data().begin(); it != mEscrowParTable.data().end(); ++it)
	{
		ECashReceiver::Enum cashReceiver = it.value().cashReceiver;

		if (mDeviceType == CHardware::Types::CashAcceptor)
		{
			mDeviceType = (cashReceiver == ECashReceiver::BillAcceptor) ? CHardware::Types::BillAcceptor : CHardware::Types::CoinAcceptor;
		}
		else if (((mDeviceType == CHardware::Types::BillAcceptor) && (cashReceiver == ECashReceiver::CoinAcceptor)) ||
			((mDeviceType == CHardware::Types::CoinAcceptor) && (cashReceiver == ECashReceiver::BillAcceptor)))
		{
			mDeviceType = CHardware::Types::DualCashAcceptor;
		}

		it.value().inhibit = true;
		int currencyId = it.value().currencyId;

		if (it.value().currency.isEmpty() && currencyId && (currencyId != Currency::NoCurrency))
		{
			it.value().currency = CurrencyCodes.key(currencyId);
		}

		if (CurrencyCodes.data().keys().contains(it.value().currency))
		{
			billset = true;

			if (CurrencyCodes.isAccorded(it.value().currency, systemCurrencyId))
			{
				compability = true;

				if (it.value().nominal > 0)
				{
					it.value().inhibit = false;
					it.value().currencyId = CurrencyCodes[it.value().currency];
				}
			}
			else
			{
				toLog(LogLevel::Error, QString("%1: nominal %2 - currency \"%3\" is not accorded with system currency id %4")
					.arg(mDeviceName)
					.arg(it.value().nominal, 5)
					.arg(it.value().currency)
					.arg(systemCurrencyId));
			}
		}
		else if (it.value().nominal)
		{
			toLog(LogLevel::Error, QString("%1: nominal %2 - unknown currency \"%3\"")
				.arg(mDeviceName)
				.arg(it.value().nominal, 5)
				.arg(it.value().currency));
		}
	}

	if (!billset)
	{
		toLog(LogLevel::Error, mDeviceName + ": Unknown billset");
		return ECurrencyError::Billset;
	}

	if (!compability)
	{
		toLog(LogLevel::Error, mDeviceName + ": Wrong compability billset currency and config currency");
		return ECurrencyError::Config;
	}

	for (auto it = mEscrowParTable.data().begin(); it != mEscrowParTable.data().end(); ++it)
	{
		if (!it.value().inhibit)
		{
			log += QString("\nnominal %1: currency %2(\"%3\")%4%5")
				.arg(it->nominal, 5)
				.arg(it->currencyId)
				.arg(it->currency)
				.arg((mDeviceType != CHardware::Types::DualCashAcceptor) ? "" : ((it->cashReceiver == ECashReceiver::BillAcceptor) ? " in bill acceptor" : " in coin acceptor"))
				.arg(it->inhibit ? ", inhibited" : "");
		}
	}

	toLog(LogLevel::Normal, log);

	if (getConfigParameter(CHardware::CallingType).toString() == CHardware::CallingTypes::Internal)
	{
		employParList();

		return mCurrencyError;
	}

	return ECurrencyError::OK;
}

//--------------------------------------------------------------------------------
template <class T>
bool CashAcceptorBase<T>::isStatusCollectionConformed(const TStatusCodesHistory & aHistory)
{
	if (mStatusCollectionHistory.size() < aHistory.size())
	{
		return false;
	}

	for (int i = 0; i < aHistory.size(); ++i)
	{
		TStatusCodes historyStatusCodes = getStatusCodes(mStatusCollectionHistory.lastValue(aHistory.size() - i));
		TStatusCodes testStatusCodes = TStatusCodes() << aHistory[i];

		if (!historyStatusCodes.contains(testStatusCodes))
		{
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
void CashAcceptorBase<T>::replaceConformedStatusCodes(TStatusCodes & aStatusCodes, int aStatusCodeFrom, int aStatusCodeTo)
{
	TStatusCodesHistory history = TStatusCodesHistory() << aStatusCodeFrom;

	if (aStatusCodes.contains(aStatusCodeFrom) && (isStatusCollectionConformed(history) || isStatusCollectionConformed(history << aStatusCodeTo)))
	{
		aStatusCodes.remove(aStatusCodeFrom);
		aStatusCodes.insert(aStatusCodeTo);
	}
}

//--------------------------------------------------------------------------------
template <class T>
void CashAcceptorBase<T>::cleanStatusCodes(TStatusCodes & aStatusCodes)
{
	if (mCurrencyError != ECurrencyError::OK)
	{
		aStatusCodes.insert(ECurrencyError::Specification[mCurrencyError]);
	}

	T::cleanStatusCodes(aStatusCodes);

	TStatusCodes ordinaryStatusCodes = TStatusCodes()
		<< BillAcceptorStatusCode::Normal::Enabled
		<< BillAcceptorStatusCode::Normal::Disabled
		<< BillAcceptorStatusCode::Normal::Inhibit;

	if ((aStatusCodes - ordinaryStatusCodes).isEmpty())
	{
		bool  enabled = aStatusCodes.contains(BillAcceptorStatusCode::Normal::Enabled);
		bool disabled = aStatusCodes.contains(BillAcceptorStatusCode::Normal::Disabled);
		bool  inhibit = aStatusCodes.contains(BillAcceptorStatusCode::Normal::Inhibit);

		// купюрник одновременно говорит, что он и включен, и выключен на прием
		if (enabled && (disabled || inhibit))
		{
			if (getConfigParameter(CHardware::CashAcceptor::Enabled).toBool())
			{
				aStatusCodes.remove(BillAcceptorStatusCode::Normal::Disabled);
				aStatusCodes.remove(BillAcceptorStatusCode::Normal::Inhibit);
				aStatusCodes.insert(BillAcceptorStatusCode::Normal::Enabled);
			}
			else
			{
				aStatusCodes.remove(BillAcceptorStatusCode::Normal::Enabled);
				aStatusCodes.insert(BillAcceptorStatusCode::Normal::Disabled);

				if (disabled && inhibit)
				{
					aStatusCodes.remove(BillAcceptorStatusCode::Normal::Disabled);
				}
			}
		}
	}

	TStatusCodes notOrdinaryStatusCodes = aStatusCodes - ordinaryStatusCodes;

	if (!notOrdinaryStatusCodes.isEmpty() && (notOrdinaryStatusCodes.size() < aStatusCodes.size()))
	{
		aStatusCodes = notOrdinaryStatusCodes;
	}

	cleanSpecificStatusCodes(aStatusCodes);

	if (aStatusCodes.contains(BillAcceptorStatusCode::MechanicFailure::StackerFull))
	{
		aStatusCodes.remove(BillAcceptorStatusCode::Warning::StackerNearFull);
	}

	if (aStatusCodes.contains(BillAcceptorStatusCode::OperationError::Accept))
	{
		aStatusCodes.remove(BillAcceptorStatusCode::BillOperation::Accepting);
	}

	if (aStatusCodes.contains(BillAcceptorStatusCode::OperationError::Escrow))
	{
		aStatusCodes.remove(BillAcceptorStatusCode::BillOperation::Escrow);
	}

	if (aStatusCodes.contains(BillAcceptorStatusCode::OperationError::Stack))
	{
		aStatusCodes.remove(BillAcceptorStatusCode::BillOperation::Stacked);
	}

	if (aStatusCodes.contains(BillAcceptorStatusCode::OperationError::Return))
	{
		aStatusCodes.remove(BillAcceptorStatusCode::Busy::Returned);
	}

	CCashAcceptor::TStatuses statuses;

	foreach (int statusCode, aStatusCodes)
	{
		ECashAcceptorStatus::Enum status = static_cast<ECashAcceptorStatus::Enum>(mStatusCodesSpecification->value(statusCode).status);
		statuses[status].insert(statusCode);
	}

	TStatusCodes rejects = statuses.value(ECashAcceptorStatus::Rejected);

	if (rejects.size() > 1)
	{
		aStatusCodes.remove(BillAcceptorStatusCode::Reject::Unknown);
	}

	bool warningNotCheated = !statuses.isEmpty(ECashAcceptorStatus::Warning) && !statuses.contains(BillAcceptorStatusCode::Warning::Cheated);

	TStatusCodes unknownErrors = TStatusCodes()
		<< BillAcceptorStatusCode::Error::Clock
		<< BillAcceptorStatusCode::Error::NoParsAvailable
		<< BillAcceptorStatusCode::Error::Firmware
		<< DeviceStatusCode::Error::Initialization
		<< DeviceStatusCode::Error::MemoryStorage;
	TStatusCodes actualUnknownErrors = statuses.value(ECashAcceptorStatus::Error) & unknownErrors;

	if ((aStatusCodes.size() > 1) && !warningNotCheated && (!statuses.contains(ECashAcceptorStatus::Error) || actualUnknownErrors.isEmpty()))
	{
		aStatusCodes.remove(BillAcceptorStatusCode::BillOperation::Unknown);
	}

	if (aStatusCodes.contains(BillAcceptorStatusCode::Busy::Returning) || aStatusCodes.contains(BillAcceptorStatusCode::Busy::Returned))
	{
		foreach (int statusCode, aStatusCodes)
		{
			ECashAcceptorStatus::Enum status = static_cast<ECashAcceptorStatus::Enum>(mStatusCodesSpecification->value(statusCode).status);

			if (status == ECashAcceptorStatus::Rejected)
			{
				aStatusCodes.remove(statusCode);
			}
		}
	}

	if (aStatusCodes.size() > 1)
	{
		aStatusCodes.remove(DeviceStatusCode::OK::OK);
	}
}

//--------------------------------------------------------------------------------
template <class T>
void CashAcceptorBase<T>::saveStatuses(const CCashAcceptor::TStatuses & aStatuses, ECashAcceptorStatus::Enum aTargetStatus, const CCashAcceptor::TStatusSet aSourceStatuses)
{
	CCashAcceptor::SStatusSpecification & lastStatusHistory = mStatusHistory.last();

	CCashAcceptor::TStatusSet sourceStatuses = aSourceStatuses + (CCashAcceptor::TStatusSet() << aTargetStatus);

	foreach(ECashAcceptorStatus::Enum status, sourceStatuses)
	{
		if (!aStatuses[status].isEmpty())
		{
			lastStatusHistory.statuses[aTargetStatus].unite(aStatuses[status]);
		}
	}

	lastStatusHistory.warningLevel = qMax(lastStatusHistory.warningLevel, mStatusCodesSpecification->warningLevelByStatus(aTargetStatus));
}

//--------------------------------------------------------------------------------
template <class T>
void CashAcceptorBase<T>::emitStatuses(CCashAcceptor::SStatusSpecification & aSpecification, const CCashAcceptor::TStatusSet & aSet)
{
	if (!mPostPollingAction)
	{
		mStatusHistory.saveLevel();
		return;
	}

	foreach(ECashAcceptorStatus::Enum currentStatus, aSet)
	{
		if (aSpecification.statuses.contains(currentStatus) && (currentStatus != ECashAcceptorStatus::Escrow) && (currentStatus != ECashAcceptorStatus::Stacked))
		{
			TStatusCollection statusCollection = getStatusCollection(aSpecification.statuses[currentStatus]);
			emitStatusCodes(statusCollection, currentStatus);
		}
	}

	mStatusHistory.updateLevel();
}

//--------------------------------------------------------------------------------
template <class T>
void CashAcceptorBase<T>::sendStatuses(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection)
{
	// Предполагается, что дублирующие и недостоверные статусы уже отфильтрованы на предыдущем шаге.
	// Конвертим ворнинг-уровни в статус-уровни.
	CCashAcceptor::TStatuses statuses = getLastStatuses();

	mStatusHistory.append(CCashAcceptor::SStatusSpecification());
	CCashAcceptor::SStatusSpecification & lastStatusHistory = mStatusHistory.last();
	CCashAcceptor::TStatuses & lastStatuses = lastStatusHistory.statuses;

	// 1. Сначала Stacked, т.к. по нему начисляется сумма.
	if (!statuses.isEmpty(ECashAcceptorStatus::Stacked) &&
	   (mStatuses.isEmpty(ECashAcceptorStatus::Stacked) || getConfigParameter(CHardware::CashAcceptor::StackedFilter).toBool()))
	{
		saveStatuses(statuses, ECashAcceptorStatus::Stacked);

		foreach(auto par, mEscrowPars)
		{
			toLog(LogLevel::Normal, QString("Send statuses: Stacked, note = %1, currency = %2")
				.arg(par.nominal)
				.arg(par.currencyId));

			emit stacked(TParList() << par);
		}

		mEscrowPars.clear();
	}

	// 2. Потом - Escrow.
	if (statuses.size(ECashAcceptorStatus::Escrow) > mStatuses.size(ECashAcceptorStatus::Escrow))
	{
		saveStatuses(statuses, ECashAcceptorStatus::Escrow);

		if (mPostPollingAction)
		{
			SPar par = mEscrowPars[0];
			toLog(LogLevel::Normal, QString("Send statuses: Escrow, note = %1, currency = %2")
				.arg(par.nominal)
				.arg(par.currencyId));

			emit escrow(par);
		}
	}

	// 3. Спец. статусы и статистика
	TStatusCodes badSpecialStatusCodes;
	ECashAcceptorStatus::Enum specialStatus = ECashAcceptorStatus::OK;

	foreach(ECashAcceptorStatus::Enum status, CCashAcceptor::Set::SpecialStatuses)
	{
		if (!statuses.isEmpty(status))
		{
			if (CCashAcceptor::Set::BadSpecialStatuses.contains(status))
			{
				badSpecialStatusCodes.unite(statuses.value(status));
				specialStatus = qMax(specialStatus, CCashAcceptor::SpecialStatus::Specification[status]);
			}

			if (statuses.value(status) != mStatuses.value(status))
			{
				saveStatuses(statuses, status);
				emitStatuses(lastStatusHistory, CCashAcceptor::TStatusSet() << status);
			}
		}
	}

	// 4. Остальные статусы.
	// Если есть ErrorStatus, все warning-и подтягиваем к error-ам, а если валидатор либо в Busy, либо в Rejected, либо в Cheated, при этом ошибок нет, то ждем, чем дело кончится.
	// Соответственно, если сменился error на warning, то эмитим warning с WarningLevel-ом Error; если сменился error на OK, но есть либо Reject, либо Busy, то  ничего не эмитим.
	// Если есть и MechanicFailure-ы, и Error-ы, то Error-ы сливаем в MechanicFailure-ы, последние хуже, из-за подозрения, что купюра застряла в терминале.

	QList<ECashAcceptorStatus::Enum> badStatusList = CCashAcceptor::Set::BadStatuses.toList();
	qSort(badStatusList);

	auto saveBadStatuses = [&] (ECashAcceptorStatus::Enum aStatus)
	{
		ECashAcceptorStatus::Enum totalStatus = qMax(specialStatus, aStatus);
		statuses[totalStatus].unite(badSpecialStatusCodes);
		QList<ECashAcceptorStatus::Enum> statusList = badStatusList.mid(0, badStatusList.indexOf(totalStatus));

		saveStatuses(statuses, totalStatus, statusList.toSet());
	};

	// 4.1. Сохраняем значимые статусы
	if (!statuses.isEmpty(ECashAcceptorStatus::MechanicFailure))
	{
		saveBadStatuses(ECashAcceptorStatus::MechanicFailure);
	}
	else if (!statuses.isEmpty(ECashAcceptorStatus::Error))
	{
		saveBadStatuses(ECashAcceptorStatus::Error);
	}
	else if (!statuses.isEmpty(ECashAcceptorStatus::Warning))
	{
		saveBadStatuses(ECashAcceptorStatus::Warning);
	}
	else
	{
		saveStatuses(statuses, ECashAcceptorStatus::OK, CCashAcceptor::Set::NormalStatuses);

		if (!lastStatuses.keys().toSet().intersect(CCashAcceptor::Set::NormalStatuses).isEmpty())
		{
			foreach(ECashAcceptorStatus::Enum status, CCashAcceptor::Set::NormalStatuses)
			{
				lastStatuses.remove(status);
			}

			lastStatuses[ECashAcceptorStatus::OK].insert(DeviceStatusCode::OK::OK);
		}
	}

	// 4.2. Принимаем решение об эмите
	CCashAcceptor::SStatusSpecification beforeLastStatusSpec = mStatusHistory.lastValue(2);
	CCashAcceptor::TStatuses & beforeLastStatuses = beforeLastStatusSpec.statuses;

	TStatusCodes beforeStatusCodes;
	TStatusCodes newStatusCodes;

	foreach(ECashAcceptorStatus::Enum status, CCashAcceptor::Set::GeneralStatuses)
	{
		if (beforeLastStatuses.contains(status) && !beforeLastStatuses.isEmpty(status))
		{
			beforeStatusCodes.unite(beforeLastStatuses.value(status));
		}

		newStatusCodes.unite(lastStatuses.value(status));
	}

	bool emitSignal = !newStatusCodes.isEmpty();

	// если выходим из ошибки, но купюрник чем-то занят - ничего не делаем, ждем, пока закончит
	if ((beforeLastStatusSpec.warningLevel == EWarningLevel::Error) && !beforeLastStatuses.isEmpty() &&
	    (lastStatusHistory.warningLevel != EWarningLevel::Error))
	{
		TStatusCodes statusCodes;

		foreach(ECashAcceptorStatus::Enum status, CCashAcceptor::Set::BusyStatuses)
		{
			statusCodes.unite(statuses.value(status));
		}

		if (!statusCodes.isEmpty())
		{
			toLog(LogLevel::Warning, mDeviceName + " is busy, waiting for change status...");
			lastStatusHistory.warningLevel = EWarningLevel::Error;

			emitSignal = false;
		}
	}

	if (emitSignal && !beforeLastStatuses.isEmpty() && (beforeLastStatusSpec.warningLevel == lastStatusHistory.warningLevel))
	{
		emitSignal = (beforeStatusCodes != newStatusCodes) || aOldStatusCollection.isEmpty();
	}

	emitSignal = emitSignal || environmentChanged();
	bool statusChanged = aNewStatusCollection != aOldStatusCollection;

	if (statusChanged)
	{
		toLog(LogLevel::Normal, QString("Signal emitting is %1allowed, post polling action is %2enabled")
			.arg(emitSignal ? "" : "not ")
			.arg(mPostPollingAction ? "" : "not "));

		QString debugLog = "Status history :";

		for(int i = 0; i < mStatusHistory.size(); ++i)
		{
			CCashAcceptor::SStatusSpecification statusSpecification = mStatusHistory[i];
			QString warningLevel = (statusSpecification.warningLevel == EWarningLevel::Error) ? "Error" :
				((statusSpecification.warningLevel == EWarningLevel::Warning) ? "Warning" : "OK");

			QString statusLog;

			#define DEBUG_DECLARE_CA_STATUS(aStatus) \
				QString debug##aStatus = getStatusTranslations(statusSpecification.statuses.value(ECashAcceptorStatus::aStatus), false);      \
				QString name##aStatus = #aStatus; name##aStatus += QString(15 - name##aStatus.size(), QChar(' '));                         \
				if (!debug##aStatus.isEmpty()) statusLog += QString("\n%1 : %2").arg(name##aStatus).arg(debug##aStatus);

			DEBUG_DECLARE_CA_STATUS(OK);
			DEBUG_DECLARE_CA_STATUS(Escrow);
			DEBUG_DECLARE_CA_STATUS(Stacked);

			DEBUG_DECLARE_CA_STATUS(Warning);

			DEBUG_DECLARE_CA_STATUS(Error);
			DEBUG_DECLARE_CA_STATUS(MechanicFailure);
			DEBUG_DECLARE_CA_STATUS(StackerFull);
			DEBUG_DECLARE_CA_STATUS(StackerOpen);

			DEBUG_DECLARE_CA_STATUS(Cheated);
			DEBUG_DECLARE_CA_STATUS(Rejected);

			DEBUG_DECLARE_CA_STATUS(Inhibit);
			DEBUG_DECLARE_CA_STATUS(Disabled);
			DEBUG_DECLARE_CA_STATUS(Enabled);
			DEBUG_DECLARE_CA_STATUS(BillOperation);
			DEBUG_DECLARE_CA_STATUS(Busy);
			DEBUG_DECLARE_CA_STATUS(OperationError);
			DEBUG_DECLARE_CA_STATUS(Unknown);

			debugLog += QString("\n [%1] : warning level = %2%3").arg(i).arg(warningLevel).arg(statusLog);
		}

		toLog(LogLevel::Normal, debugLog);
	}

	// 4.3. Если надо - эмитим статус
	if (emitSignal)
	{
		emitStatuses(lastStatusHistory, CCashAcceptor::Set::GeneralStatuses);
	}
	else if ((lastStatusHistory == beforeLastStatusSpec) || lastStatusHistory.statuses.keys().toSet().intersect(CCashAcceptor::Set::MainStatuses).isEmpty())
	{
		mStatusHistory.removeLast();
	}

	mStatusHistory.updateLevel(true);

	if (statusChanged)
	{
		toLog(LogLevel::Normal, QString("Status history: level = %1, size = %2")
			.arg(mStatusHistory.getLevel())
			.arg(mStatusHistory.size()));
	}

	mStatuses = statuses;

	if (mStatusHistory.isEmpty())
	{
		mReady = false;
	}
	else
	{
		CCashAcceptor::TStatuses statusBuffer = mStatusHistory.lastValue().statuses;
		CCashAcceptor::TStatusSet statusSet = statusBuffer.keys().toSet();

		mReady = (mInitialized == ERequestStatus::Success) && !statusSet.isEmpty() && statusSet.intersect(CCashAcceptor::Set::ErrorStatuses).isEmpty() &&
			!statusBuffer.contains(BillAcceptorStatusCode::Busy::Pause);
	}
}

//--------------------------------------------------------------------------------
