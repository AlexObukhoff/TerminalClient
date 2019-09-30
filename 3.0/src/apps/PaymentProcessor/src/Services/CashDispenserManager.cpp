/* @file Обработчик команд работы с устройствами выдачи наличных. */

// STL
#include <algorithm>
#include <numeric>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QVector>
#include <QtCore/QtAlgorithms>
#include <Common/QtHeadersEnd.h>

// PaymentProcessor SDK
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

// Driver SDK
#include <SDK/Drivers/WarningLevel.h>
#include <SDK/Drivers/HardwareConstants.h>
#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/IDispenser.h>

// Project
#include "System/IApplication.h"
#include "Services/SettingsService.h"
#include "Services/DatabaseService.h"
#include "Services/DeviceService.h"
#include "Services/PaymentService.h"
#include "DatabaseUtils/IHardwareDatabaseUtils.h"
#include "FundsService.h"
#include "CashDispenserManager.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
CashDispenserManager::CashDispenserManager(IApplication * aApplication) :
	ILogable(CFundsService::LogName),
	mApplication(aApplication),
	mDeviceService(nullptr),
	mDatabase(nullptr),
	mPaymentDatabase(nullptr)
{
}

//---------------------------------------------------------------------------
bool CashDispenserManager::initialize(IPaymentDatabaseUtils * aDatabase)
{
	mDeviceService = DeviceService::instance(mApplication);
	mDatabase = DatabaseService::instance(mApplication)->getDatabaseUtils<IHardwareDatabaseUtils>();
	mPaymentDatabase = aDatabase;

	mDispensers.clear();

	// Получаем настройки терминала
	PPSDK::TerminalSettings * settings = SettingsService::instance(mApplication)->getAdapter<PPSDK::TerminalSettings>();

	mCurrencyName = settings->getCurrencySettings().name;

	if (mCurrencyName.isEmpty())
	{
		toLog(LogLevel::Error, "Currency is not set for funds service!");

		return false;
	}

	updateHardwareConfiguration();

	connect(mDeviceService, SIGNAL(configurationUpdated()), this, SLOT(updateHardwareConfiguration()));

	return true;
}

//---------------------------------------------------------------------------
void CashDispenserManager::updateHardwareConfiguration()
{
	mDispensers.clear();
	
	// Получаем настройки терминала
	PPSDK::TerminalSettings * settings = SettingsService::instance(mApplication)->getAdapter<PPSDK::TerminalSettings>();

	// Получаем список всех доступных устройств.
	QStringList deviceList = settings->getDeviceList().filter(QRegExp(QString("(%1)").arg(DSDK::CComponents::Dispenser)));

	foreach (const QString & configurationName, deviceList)
	{
		DSDK::IDispenser * device = dynamic_cast<DSDK::IDispenser *>(mDeviceService->acquireDevice(configurationName));

		if (device)
		{
			// Подписываемся на сигналы.
			device->subscribe(SDK::Driver::IDispenser::UnitsDefinedSignal, this, SLOT(onUnitsDefined()));
			device->subscribe(SDK::Driver::IDispenser::DispensedSignal, this, SLOT(onDispensed(int, int)));
			device->subscribe(SDK::Driver::IDispenser::RejectedSignal, this, SLOT(onRejected(int, int)));
			device->subscribe(SDK::Driver::IDispenser::UnitEmptySignal, this, SLOT(onUnitEmpty(int)));

			device->subscribe(SDK::Driver::IDevice::StatusSignal, this, SLOT(onStatusChanged(SDK::Driver::EWarningLevel::Enum, const QString &, int)));

			mDispensers.insert(device, configurationName);
		}
		else
		{
			toLog(LogLevel::Error, QString("Failed to acquire cash dispenser %1.").arg(configurationName));
		}
	}

	// Грузим список загруженных купюр
	loadCashList();

	onUnitsDefined();
}

//---------------------------------------------------------------------------
void CashDispenserManager::shutdown()
{
	// Сохраняем состояние по купюрам
	saveCashCount();

	foreach (DSDK::IDispenser * dispenser, mDispensers.keys())
	{
		mDeviceService->releaseDevice(dispenser);
	}

	mDispensers.clear();
}

//---------------------------------------------------------------------------
void CashDispenserManager::onStatusChanged(DSDK::EWarningLevel::Enum aLevel, const QString & aTranslation, int /*aStatus*/)
{
	DSDK::IDispenser * dispenser = dynamic_cast<DSDK::IDispenser *>(sender());

	if (!dispenser)
	{
		return;
	}

	if (aLevel == DSDK::EWarningLevel::Error)
	{
		mFailedDispensers.insert(dispenser);

		emit error(aTranslation);
	}
	else
	{
		mFailedDispensers.remove(dispenser);

		emit activity();
	}
}

//---------------------------------------------------------------------------
PPSDK::SCashUnit * CashDispenserManager::checkSignal(QObject * aSender, const QString & aSignalName, int aUnit)
{
	DSDK::IDispenser * dispenser = dynamic_cast<DSDK::IDispenser *>(aSender);

	if (!dispenser)
	{
		toLog(LogLevel::Error, QString("Receive %1 signal, but sender not have DSDK::IDispenser interface.").arg(aSignalName));
		return nullptr;
	}

	QString configurationName = mDispensers.value(dispenser);
	int unitCount = mCurrencyCashList[configurationName].size();

	if (unitCount <= aUnit)
	{
		toLog(LogLevel::Error, QString("Wrong unit number = %1, need max %2.").arg(aUnit).arg(unitCount - 1));
		return nullptr;
	}

	return &mCurrencyCashList[configurationName][aUnit];
}

//---------------------------------------------------------------------------
bool CashDispenserManager::handleSignal(QObject * aSender, const QString & aSignalName, int aUnit, int aItems, PPSDK::TPaymentAmount & aAmount)
{
	PPSDK::SCashUnit * cashUnit = checkSignal(aSender, aSignalName, aUnit);

	if (!cashUnit)
	{
		return false;
	}

	cashUnit->count -= qMin(aItems, cashUnit->count);
	int nominal = cashUnit->nominal;
	aAmount = nominal * aItems;

	toLog(LogLevel::Normal, QString("%1 %2 notes (nominal %3) AMOUNT = %4").arg(aSignalName).arg(aItems).arg(nominal).arg(aAmount, 0, 'f', 2));

	if (!cashUnit->count)
	{
		DSDK::IDispenser * dispenser = dynamic_cast<DSDK::IDispenser *>(aSender);
		setCashList(dispenser);
	}

	saveCashCount();

	return true;
}

//---------------------------------------------------------------------------
void CashDispenserManager::setCashList(DSDK::IDispenser * aDispenser)
{
	DSDK::TUnitData unitData;

	foreach(auto cashUnit, mCurrencyCashList[mDispensers[aDispenser]])
	{
		unitData << cashUnit.count;
	}

	aDispenser->setUnitList(unitData);
}

//---------------------------------------------------------------------------
void CashDispenserManager::onUnitsDefined()
{
	DSDK::IDispenser * dispenser = dynamic_cast<DSDK::IDispenser *>(sender());
	TDispensers dispensers;

	if (dispenser)
	{
		dispensers << dispenser;
	}
	else
	{
		dispensers = mDispensers.keys().toSet();
	}

	/// Проверяем и обновляем список доступных купюр
	/// если загруженных купюр нет - создаем и сохраняем в БД пустой список

	foreach(DSDK::IDispenser * dispenser, dispensers)
	{
		QString configPath = mDispensers.value(dispenser);
		int units = dispenser->units();

		if (units)
		{
			int currentUnits = mCurrencyCashList.contains(configPath) ? mCurrencyCashList[configPath].size() : 0;

			if (currentUnits < units)
			{
				PPSDK::TCashUnitList cashUnitList = PPSDK::TCashUnitList(units - currentUnits, PPSDK::SCashUnit(mCurrencyName, 0, 0));
				mCurrencyCashList[configPath] << cashUnitList;
			}
			else if (currentUnits > units)
			{
				PPSDK::TCashUnitList & cashUnitList = mCurrencyCashList[configPath];
				cashUnitList = cashUnitList.mid(0, units);
			}

			if (currentUnits != units)
			{
				saveCashCount();
			}
		}
	}
}

//---------------------------------------------------------------------------
void CashDispenserManager::onDispensed(int aUnit, int aItems)
{
	PPSDK::TPaymentAmount amount = 0;

	if (handleSignal(sender(), "Dispensed", aUnit, aItems, amount))
	{
		storeNotes(sender(), aUnit, aItems);

		mAmounts += SAmounts(-amount, amount);

		if (canDispense(mAmounts.toDispensing))
		{
			emit activity();

			dispense(mAmounts.toDispensing);
		}
		else
		{
			toLog(LogLevel::Normal, QString("Send dispensed total amount = %1").arg(mAmounts.dispensed, 0, 'f', 2));

			emit dispensed(mAmounts.dispensed);

			mAmounts = SAmounts(0, 0);
		}
	}
	else
	{
		toLog(LogLevel::Warning, "Send dispensed total amount = 0 due to error in handling notes info");

		emit dispensed(0);

		mAmounts = SAmounts(0, 0);
	}
}

//---------------------------------------------------------------------------
void CashDispenserManager::onRejected(int aUnit, int aItems)
{
	PPSDK::TPaymentAmount amount;
	handleSignal(sender(), "Rejected", aUnit, aItems, amount);
}

//---------------------------------------------------------------------------
void CashDispenserManager::onUnitEmpty(int aUnit)
{
	PPSDK::SCashUnit * cashUnit = checkSignal(sender(), "unitEmpty", aUnit);

	if (cashUnit)
	{
		cashUnit->count = 0;
	}
}

//---------------------------------------------------------------------------
CashDispenserManager::TItemDataSet CashDispenserManager::getItemDataSet(PPSDK::TPaymentAmount aAmount)
{
	CashDispenserManager::TItemDataSet result;

	foreach (auto dispenser, mDispensers.keys())
	{
		if (!mFailedDispensers.contains(dispenser))
		{
			auto cashList = mCurrencyCashList[mDispensers[dispenser]];

			for (int i = 0; i < cashList.size(); i++)
			{
				if (dispenser->isDeviceReady(i) && cashList[i].count && cashList[i].nominal && (cashList[i].nominal <= aAmount))
				{
					int count = (cashList[i].count > 0) ? cashList[i].count : 0;
					result[cashList[i].nominal] << SItemData(dispenser, i, count);
				}
			}
		}
	}

	return result;
}

//---------------------------------------------------------------------------
void CashDispenserManager::saveCashCount()
{
	foreach (auto configName, mCurrencyCashList.keys())
	{
		QStringList parameterValueList;

		foreach (auto cash, mCurrencyCashList.value(configName))
		{
			parameterValueList << QString("%1:%2:%3").arg(cash.currencyName).arg(cash.nominal).arg(cash.count);
		}

		mDatabase->setDeviceParam(configName, PPSDK::CDatabaseConstants::Parameters::CashUnits, parameterValueList.join(";"));
	}
}

//---------------------------------------------------------------------------
void CashDispenserManager::loadCashList()
{
	mCurrencyCashList.clear();

	for (auto it = mDispensers.begin(); it != mDispensers.end(); ++it)
	{
		QString configPath = *it;
		QString cashUnitData = mDatabase->getDeviceParam(configPath, PPSDK::CDatabaseConstants::Parameters::CashUnits).toString();
		QStringList cashUnits = cashUnitData.split(";", QString::SkipEmptyParts);

		for (int i = 0; i < cashUnits.size(); ++i)
		{
			QStringList unit = cashUnits[i].split(":");
			int count = unit[2].toInt();
			PPSDK::SCashUnit cashUnit(unit[0], unit[1].toInt(), count);
			mCurrencyCashList[configPath] << cashUnit;
		}

		setCashList(it.key());
	}
}

//---------------------------------------------------------------------------
bool CashDispenserManager::getItemData(PPSDK::TPaymentAmount aAmount, const TItemDataSet & aItemDataSet, TItemDataSetIt & aItemDataSetIt)
{
	if (aItemDataSet.isEmpty())
	{
		return false;
	}

	QList<int> nominals = aItemDataSet.keys();

	if (*std::min_element(nominals.begin(), nominals.end()) > aAmount)
	{
		return false;
	}

	int nominal = *std::max_element(nominals.begin(), nominals.end());

	if (nominal > aAmount)
	{
		qSort(nominals);
		QList<int>::iterator it = qLowerBound(nominals.begin(), nominals.end(), aAmount);

		nominal = *it;

		if (nominal > aAmount && it != nominals.begin())
		{
			it--;
			nominal = *it;
		}
	}

	aItemDataSetIt = TItemDataSetIt(aItemDataSet.find(nominal));

	return true;
}

//---------------------------------------------------------------------------
PPSDK::TPaymentAmount CashDispenserManager::canDispense(PPSDK::TPaymentAmount aRequiredAmount)
{
	TItemDataSet itemDataSet = getItemDataSet(aRequiredAmount);
	TItemDataSetIt itemDataSetIt;
	PPSDK::TPaymentAmount dispensingAmount = 0;

	while (getItemData(aRequiredAmount - dispensingAmount, itemDataSet, itemDataSetIt))
	{
		int nominal = itemDataSetIt.key();
		int requiredCount = int(aRequiredAmount - dispensingAmount) / nominal;
		int availableCount = std::accumulate(itemDataSetIt->begin(), itemDataSetIt->end(), 0, [] (int aCount, const SItemData & data) -> int { return aCount + data.count; });
		int count = qMin(requiredCount, availableCount);
		dispensingAmount += count * nominal;

		if (count == availableCount)
		{
			itemDataSet.erase(itemDataSetIt);
		}
	}

	return dispensingAmount;
}

//---------------------------------------------------------------------------
void CashDispenserManager::dispense(PPSDK::TPaymentAmount aAmount)
{
	if (qFuzzyIsNull(mAmounts.toDispensing) && qFuzzyIsNull(mAmounts.dispensed))
	{
		mAmounts.toDispensing = aAmount;
		toLog(LogLevel::Normal, QString("Amount to dispensing = %1").arg(aAmount, 0, 'f', 2));
	}

	TItemDataSet itemDataSet = getItemDataSet(aAmount);
	TItemDataSetIt itemDataSetIt;

	if (!getItemData(aAmount, itemDataSet, itemDataSetIt))
	{
		mAmounts = SAmounts(0, 0);

		toLog(LogLevel::Warning, "Send dispensed total amount = 0 due to absence of available dispensing resources");

		emit dispensed(0);
	}
	else
	{
		int requiredCount = int(aAmount) / itemDataSetIt.key();
		SItemData & data = *itemDataSetIt->begin();
		int count = qMin(data.count, requiredCount);

		data.dispenser->dispense(data.unit, count);
	}
}

//---------------------------------------------------------------------------
PPSDK::TCashUnitsState CashDispenserManager::getCashUnitsState()
{
	return mCurrencyCashList;
}

//---------------------------------------------------------------------------
bool CashDispenserManager::setCashUnitsState(const QString & aDeviceConfigurationName, const PPSDK::TCashUnitList & aCashUnitList)
{
	if (!mCurrencyCashList.contains(aDeviceConfigurationName))
	{
		toLog(LogLevel::Error, QString("Unknown cash dispenser device %1.").arg(aDeviceConfigurationName));

		return false;
	}

	if (mCurrencyCashList.value(aDeviceConfigurationName).size() != aCashUnitList.size())
	{
		toLog(LogLevel::Error, QString("Incorrect cash unit count for device %1.").arg(aDeviceConfigurationName));

		return false;
	}

	mCurrencyCashList[aDeviceConfigurationName] = aCashUnitList;
	setCashList(mDispensers.key(aDeviceConfigurationName));

	saveCashCount();

	return true;
}

//---------------------------------------------------------------------------
bool CashDispenserManager::storeNotes(QObject * aSender, int aUnit, int aItems)
{
	DSDK::IDispenser * dispenser = dynamic_cast<DSDK::IDispenser *>(aSender);

	if (!dispenser)
	{
		toLog(LogLevel::Error, "Receive dispense signal, but sender not have DSDK::IDispenser interface.");
		return false;
	}

	QString configurationName = mDispensers.value(dispenser);

	PPSDK::SCashUnit * cashUnit = &mCurrencyCashList[configurationName][aUnit];

	PPSDK::TerminalSettings * settings = SettingsService::instance(mApplication)->getAdapter<PPSDK::TerminalSettings>();

	QList<PPSDK::SNote> notes;

	for (int i = 0; i < aItems; i++)
	{
		notes.push_back(PPSDK::SNote(PPSDK::EAmountType::Bill, cashUnit->nominal, settings->getCurrencySettings().id, ""));
	}

	return mPaymentDatabase->addChangeNote(PaymentService::instance(mApplication)->getChangeSessionRef(), notes);
}

//---------------------------------------------------------------------------
