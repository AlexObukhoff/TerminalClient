/* @file Обработчик команд работы с устройствами выдачи наличных. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QSet>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/IDispenser.h>
#include <SDK/PaymentProcessor/Core/IFundsService.h>

// Modules
#include <Common/ILogable.h>

// PP
#include "DatabaseUtils/IPaymentDatabaseUtils.h"

class IApplication;
class IHardwareDatabaseUtils;

namespace SDK { namespace PaymentProcessor {
	class IDeviceService;
}}

//---------------------------------------------------------------------------
class CashDispenserManager : public SDK::PaymentProcessor::ICashDispenserManager, public ILogable
{
	Q_OBJECT

	/// Данные выдаваемых объектов.
	struct SItemData
	{
		SDK::Driver::IDispenser * dispenser;
		int unit;
		int count;

		SItemData(): dispenser(nullptr), unit(0), count(0) {}
		SItemData(SDK::Driver::IDispenser * aDispenser, int aUnit, int aCount): dispenser(aDispenser), unit(aUnit), count(aCount) {}
	};

	typedef QList<SItemData> TItemData;
	typedef QMap<int, TItemData> TItemDataSet;
	typedef TItemDataSet::iterator TItemDataSetIt;

	struct SAmounts
	{
		SDK::PaymentProcessor::TPaymentAmount toDispensing;    /// к выдаче
		SDK::PaymentProcessor::TPaymentAmount dispensed;       /// выданное

		SAmounts(): toDispensing(0), dispensed(0) {}
		SAmounts(SDK::PaymentProcessor::TPaymentAmount aToDispensing, SDK::PaymentProcessor::TPaymentAmount aDispensed):
			toDispensing(aToDispensing), dispensed(aDispensed) {}

		SAmounts & operator+=(const SAmounts & aAmounts)
		{
			toDispensing  += aAmounts.toDispensing;
			dispensed     += aAmounts.dispensed;

			return *this;
		}
	};

public:
	CashDispenserManager(IApplication * aApplication);

	/// Инициализация
	virtual bool initialize(IPaymentDatabaseUtils * aDatabase);

	/// Проверка, возможна ли выдача наличных средств
	virtual SDK::PaymentProcessor::TPaymentAmount canDispense(SDK::PaymentProcessor::TPaymentAmount aRequiredAmount);

	/// Выдать указанную сумму (асинхронная операция)
	virtual void dispense(SDK::PaymentProcessor::TPaymentAmount aAmount);

	/// Получить список номиналов и их кол-ва для всех устройств выдачи денег
	virtual SDK::PaymentProcessor::TCashUnitsState getCashUnitsState();

	/// Сохранить список номиналов и их кол-ва для всех устройств выдачи денег
	virtual bool setCashUnitsState(const QString & aDeviceConfigurationName, const SDK::PaymentProcessor::TCashUnitList & aCashUnitList);

public slots:
	void shutdown();

private slots:
	void updateHardwareConfiguration();

	void onUnitsDefined();
	void onDispensed(int aUnit, int aItems);
	void onRejected(int aUnit, int aItems);
	void onUnitEmpty(int aUnit);

	/// Изменение статуса диспенсера
	void onStatusChanged(SDK::Driver::EWarningLevel::Enum aWarningLevel, const QString & aTranslation, int aStatus);

private:
	/// Загрузить содержимое диспенсеров из БД
	void loadCashList();

	/// Сохранить содержимое диспенсеров в БД
	void saveCashCount();

	/// Получить данные объектов для выдачи
	TItemDataSet getItemDataSet(SDK::PaymentProcessor::TPaymentAmount aAmount);

	/// Получить данные объектa для выдачи
	bool getItemData(SDK::PaymentProcessor::TPaymentAmount aAmount, TItemDataSet & aItemData, TItemDataSetIt & aItemDataSetIt);

	/// Проверить сигнал о результате выдачи денег
	SDK::PaymentProcessor::SCashUnit * checkSignal(QObject * aSender, const QString & aSignalName, int aUnit);

	/// Сохранить в базе информацию о выданных купюрах
	bool storeNotes(QObject * aSender, int aUnit, int aItems);

	/// Проверить и обработать сигнал о результате выдачи денег
	bool handleSignal(QObject * aSender, const QString & aSignalName, int aUnit, int aItemCount, SDK::PaymentProcessor::TPaymentAmount & aAmount);

	/// Установить конфигурацию кассет
	void setCashList(SDK::Driver::IDispenser * aDispenser);

	/// Валюта
	QString mCurrencyName;

	/// Приложение
	IApplication * mApplication;

	/// Девайс-сервис
	SDK::PaymentProcessor::IDeviceService * mDeviceService;

	/// Экземпляр девайс-сервиса
	IHardwareDatabaseUtils * mDatabase;
	IPaymentDatabaseUtils * mPaymentDatabase;

	/// Сумма для выдачи/выданная
	SAmounts mAmounts;

	/// Список всех устройств с их GUID
	QMap<SDK::Driver::IDispenser *, QString> mDispensers;

	/// Список сломанных устройств
	typedef QSet<SDK::Driver::IDispenser *> TDispensers;
	TDispensers mFailedDispensers;

	/// Актуальный список номиналов в каждом диспенсере
	SDK::PaymentProcessor::TCashUnitsState mCurrencyCashList;
};

//---------------------------------------------------------------------------
