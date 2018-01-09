/* @file Обработчик команд работы с устройствами/сервисами получения денег. */

#include <numeric>

// PaymentProcessor SDK
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/EventTypes.h>
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/IChargeProvider.h>
#include <SDK/PaymentProcessor/Core/ServiceParameters.h>
#include <SDK/PaymentProcessor/Core/IPaymentService.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

// Driver SDK
#include <SDK/Drivers/WarningLevel.h>
#include <SDK/Drivers/HardwareConstants.h>
#include <SDK/Drivers/Components.h>

// Проект
#include "System/IApplication.h"
#include "Services/ServiceNames.h"
#include "Services/DeviceService.h"
#include "Services/DatabaseService.h"
#include "Services/SettingsService.h"
#include "Services/PluginService.h"
#include "Services/PaymentService.h"
#include "DatabaseUtils/IHardwareDatabaseUtils.h"
#include "FundsService.h"
#include "Services/CashAcceptorManager.h"

namespace PPSDK = SDK::PaymentProcessor;

namespace CCashAcceptor
{
	const char * CashPaymentMethod = "cash";
}

//---------------------------------------------------------------------------
bool CashAcceptorManager::SPaymentData::maxAmountReached() const
{
	return !qFuzzyIsNull(maxAmount) && (currentAmount > maxAmount || qFuzzyCompare(currentAmount, maxAmount));
}

//---------------------------------------------------------------------------
bool CashAcceptorManager::SPaymentData::chargeSourceEmpty() const
{
	return validators.empty() && chargeProviders.empty();
}

//---------------------------------------------------------------------------
CashAcceptorManager::CashAcceptorManager(IApplication * aApplication)
	: ILogable(CFundsService::LogName),
	mApplication(aApplication),
	mDatabase(nullptr),
	mDeviceService(nullptr),
	mDisableAmountOverflow(false)
{
}

//---------------------------------------------------------------------------
CashAcceptorManager::~CashAcceptorManager()
{
}

//---------------------------------------------------------------------------
bool CashAcceptorManager::initialize(IPaymentDatabaseUtils * aDatabase)
{
	mDatabase = aDatabase;

	auto pluginLoader = PluginService::instance(mApplication)->getPluginLoader();
	QStringList providers = pluginLoader->getPluginList(QRegExp(QString("%1\\.%2\\..*").arg(PPSDK::Application, PPSDK::CComponents::ChargeProvider)));

	foreach (const QString & path, providers)
	{
		SDK::Plugin::IPlugin * plugin = pluginLoader->createPlugin(path);
		if (plugin)
		{
			PPSDK::IChargeProvider * provider = dynamic_cast<PPSDK::IChargeProvider *>(plugin);
			if (provider)
			{
				mChargeProviders << provider;

				provider->subscribe(SDK::PaymentProcessor::CChargeProvider::StackedSignal,
					this, SLOT(onChargeProviderStacked(SDK::PaymentProcessor::SNote)));
			}
			else
			{
				pluginLoader->destroyPlugin(plugin);
			}
		}
	}

	mDeviceService = DeviceService::instance(mApplication);

	PPSDK::TerminalSettings * settings = SettingsService::instance(mApplication)->getAdapter<PPSDK::TerminalSettings>();

	mDisableAmountOverflow = settings->getCommonSettings().disableAmountOverflow;

	if (settings->getCurrencySettings().id == -1)
	{
		toLog(LogLevel::Error, "Currency is not set for funds service!");

		return false;
	}

	// Передаем ид валюты - параметр, нобходимый для инициации купюрников.
	QVariantMap params;
	params[CHardwareSDK::CashAcceptor::SystemCurrencyId] = settings->getCurrencySettings().id;
	mDeviceService->setInitParameters(DSDK::CComponents::BillAcceptor, params);
	mDeviceService->setInitParameters(DSDK::CComponents::CoinAcceptor, params);

	initWorkingParList();

	updateHardwareConfiguration();

	connect(mDeviceService, SIGNAL(configurationUpdated()), SLOT(updateHardwareConfiguration()));

	return true;
}

//---------------------------------------------------------------------------
bool CashAcceptorManager::shutdown()
{
	foreach (DSDK::ICashAcceptor * acceptor, mDeviceList)
	{
		mDeviceService->releaseDevice(acceptor);
	}

	mDeviceList.clear();

	foreach (SDK::PaymentProcessor::IChargeProvider * provider, mChargeProviders)
	{
		provider->unsubscribe(SDK::PaymentProcessor::CChargeProvider::StackedSignal, this);

		PluginService::instance(mApplication)->getPluginLoader()->destroyPlugin(dynamic_cast<SDK::Plugin::IPlugin *>(provider));
	}

	mChargeProviders.clear();
	mPaymentData.clear();

	return true;
}

//---------------------------------------------------------------------------
QStringList CashAcceptorManager::getPaymentMethods()
{
	QSet<QString> result;

	if (!mDeviceList.isEmpty())
	{
		result.insert(CCashAcceptor::CashPaymentMethod);
	}

	foreach (SDK::PaymentProcessor::IChargeProvider * provider, mChargeProviders)
	{
		QString method = provider->getMethod();

		if (!method.isEmpty())
		{
			result.insert(method);
		}
	}

	return result.toList();
}

//---------------------------------------------------------------------------
void CashAcceptorManager::updateHardwareConfiguration()
{
	// Получаем список всех доступных устройств.
	PPSDK::TerminalSettings * settings = SettingsService::instance(mApplication)->getAdapter<PPSDK::TerminalSettings>();
	QStringList deviceList = settings->getDeviceList().filter(QRegExp(QString("(%1|%2)")
		.arg(DSDK::CComponents::BillAcceptor)
		.arg(DSDK::CComponents::CoinAcceptor)));

	mDeviceList.clear();

	foreach (const QString & configurationName, deviceList)
	{
		DSDK::ICashAcceptor * device = dynamic_cast<DSDK::ICashAcceptor *>(mDeviceService->acquireDevice(configurationName));

		if (device)
		{
			mDeviceList.append(device);

			// Подписываемся на сигналы.
			if (mDisableAmountOverflow)
			{
				device->subscribe(SDK::Driver::ICashAcceptor::EscrowSignal, this, SLOT(onEscrowChangeControl(SDK::Driver::SPar)));
			}
			else 
			{
				device->subscribe(SDK::Driver::ICashAcceptor::EscrowSignal, this, SLOT(onEscrow(SDK::Driver::SPar)));
			}

			device->subscribe(SDK::Driver::IDevice::StatusSignal, this, SLOT(onStatusChanged(SDK::Driver::EWarningLevel::Enum, const QString &, int)));
			device->subscribe(SDK::Driver::ICashAcceptor::StackedSignal, this, SLOT(onStacked(SDK::Driver::TParList)));

			device->setParList(mWorkingParList);
		}
		else
		{
			toLog(LogLevel::Error, QString("Failed to acquire cash acceptor %1.").arg(configurationName));
		}
	}
}

//---------------------------------------------------------------------------
bool CashAcceptorManager::enable(qint64 aPayment, const QString & aPaymentMethod, PPSDK::TPaymentAmount aMaxAmount)
{
	if (mPaymentData && mPaymentData->paymentId == aPayment)
	{
		// Пересчитаем максимальную сумму платежа
		if (mPaymentData->currentAmount == 0)
		{
			mPaymentData->maxAmount = aMaxAmount;
		}

		if (mPaymentData->maxAmountReached())
		{
			return false;
		}

		// Продолжаем прием средств в контексте предыдущего платежа.
	}
	else
	{
		// Создаем новый контекст приема средств.
		SPaymentData paymentData(aPayment);

		paymentData.maxAmount = aMaxAmount;

		mPaymentData = QSharedPointer<SPaymentData>(new SPaymentData(paymentData));
	}

	const QString method = aPaymentMethod.isEmpty() ? CCashAcceptor::CashPaymentMethod : aPaymentMethod;

	if (method == CCashAcceptor::CashPaymentMethod)
	{
		// Взять валидатор, который поддерживает нужный набор номиналов.
		foreach (DSDK::ICashAcceptor * acceptor, mDeviceList)
		{
			if (!acceptor->isDeviceReady())
			{
				toLog(LogLevel::Warning, QString("Payment %1. Cash acceptor %2 is not ready.").arg(aPayment).arg(acceptor->getName()));

				continue;
			}

			if (acceptor->setEnable(true))
			{
				toLog(LogLevel::Debug, QString("Payment %2. %1 was added.").arg(acceptor->getName()).arg(mPaymentData->paymentId));

				mPaymentData->validators.insert(acceptor);
			}
		}
	}

	// Готовы принимать "деньги" от остальных поставщиков
	foreach (PPSDK::IChargeProvider * provider, mChargeProviders)
	{
		if (provider->getMethod() == method && provider->enable(aMaxAmount))
		{
			mPaymentData->chargeProviders.insert(provider);
		}
	}

	if (mPaymentData->chargeSourceEmpty())
	{
		toLog(LogLevel::Error, QString("Payment %1. No funds sources available.").arg(aPayment));
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
bool CashAcceptorManager::disable(qint64 aPayment)
{
	toLog(LogLevel::Debug, "Disable all cash acceptors.");

	if (!mPaymentData || mPaymentData->paymentId != aPayment)
	{
		// Валидатор уже отключен.
		toLog(LogLevel::Debug, "No need to disable the cash acceptors due to data absent.");

		emit disabled(aPayment);

		return false;
	}

	// Отключаем источники средств.
	foreach (PPSDK::IChargeProvider * provider, mChargeProviders)
	{
		if (provider->disable())
		{
			mPaymentData->chargeProviders.remove(provider);
		}
	}

	// Даем команду на отключение устройств.
	foreach (DSDK::ICashAcceptor * acceptor, mDeviceList)
	{
		if (!acceptor->setEnable(false))
		{
			toLog(LogLevel::Error, QString("Failed to disable cash acceptor %1.").arg(mDeviceService->getDeviceConfigName(acceptor)));
		}
	}

	if (mPaymentData->chargeSourceEmpty())
	{
		toLog(LogLevel::Debug, "All cash acceptors were disabled already.");

		// Отключение устройств уже было произведено.
		emit disabled(aPayment);
	}

	return true;
}

//---------------------------------------------------------------------------
void CashAcceptorManager::onEscrowChangeControl(SDK::Driver::SPar aPar)
{
	DSDK::ICashAcceptor * acceptor = dynamic_cast<DSDK::ICashAcceptor *>(sender());

	if (mPaymentData && mPaymentData->validators.contains(acceptor) &&
		!isFixedAmountPayment(mPaymentData->paymentId) && !allowMoreMoney(aPar.nominal))
	{
		toLog(LogLevel::Error, QString("Escrow will overflow max amount. Reject %1.").arg(aPar.nominal));

		if (!acceptor->reject())
		{
			toLog(LogLevel::Error, QString("Return command failed. Nominal : %1.").arg(aPar.currencyId));
		}

		emit warning(mPaymentData->paymentId, "#overflow_amount");

		return;
	}

	onEscrow(aPar);
}

//---------------------------------------------------------------------------
void CashAcceptorManager::onEscrow(DSDK::SPar aPar)
{
	DSDK::ICashAcceptor * acceptor = dynamic_cast<DSDK::ICashAcceptor *>(sender());

	if (mPaymentData && mPaymentData->validators.contains(acceptor))
	{
		//TODO по флажку в настройках проверять будущую сумму платежа и выбрасывать, если результирующая сумма больше maxAmount.

		if (mPaymentData->maxAmountReached())
		{
			if (!acceptor->reject())
			{
				toLog(LogLevel::Error, QString("Return command failed. Nominal : %1.").arg(aPar.currencyId));
			}
		}
		else if (!acceptor->stack())
		{
			toLog(LogLevel::Error, QString("Stack command failed. Nominal : %1.").arg(aPar.currencyId));
		}
	}
	else
	{
		toLog(LogLevel::Error, QString("Escrow to unknown payment. Nominal %1 will be rejected.").arg(aPar.currencyId));

		if (acceptor->reject())
		{
			toLog(LogLevel::Error, QString("Return command failed. Nominal : %1.").arg(aPar.currencyId));
		}
	}
}

//---------------------------------------------------------------------------
void CashAcceptorManager::onStacked(DSDK::TParList aNotes)
{
	DSDK::ICashAcceptor * acceptor = dynamic_cast<DSDK::ICashAcceptor *>(sender());

	if (mPaymentData && mPaymentData->validators.contains(acceptor))
	{
		double amount = std::accumulate(aNotes.begin(), aNotes.end(), 0.0, [](double acc, const DSDK::SPar & par) -> double { return acc + par.nominal;});
		auto paymentId = mPaymentData->paymentId;

		// Записываем в БД информацию о принятых купюрах.
		if (!mDatabase->addPaymentNote(paymentId, std::accumulate(aNotes.begin(), aNotes.end(), QList<SDK::PaymentProcessor::SNote>(),
			[](QList<SDK::PaymentProcessor::SNote> & list, const DSDK::SPar & note) -> QList<SDK::PaymentProcessor::SNote>
		{
			return list << SDK::PaymentProcessor::SNote(
				note.cashReceiver == DSDK::ECashReceiver::CoinAcceptor ? PPSDK::EAmountType::Coin : PPSDK::EAmountType::Bill,
				note.nominal, note.currencyId, note.serialNumber);
		})))
		{
			toLog(LogLevel::Error, QString("Payment %1. Failed to update payment amount. Total sum: %2.").arg(paymentId).arg(amount));
		}

		mPaymentData->currentAmount += amount;

		emit PPSDK::ICashAcceptorManager::amountUpdated(paymentId, mPaymentData->currentAmount, amount);

		if (mPaymentData->maxAmountReached())
		{
			disable(paymentId);
		}
	}
	else
	{
		// Сообщим о средствах, которые не попали в платеж
		QStringList lostMoney;
		foreach(SDK::Driver::SPar par, aNotes)
		{
			lostMoney.append(QString("%1: %2").arg(par.cashReceiver == SDK::Driver::ECashReceiver::CoinAcceptor ? "Coin" : "Bill").arg(par.nominal));
		}

		toLog(LogLevel::Error, QString("Funds was stacked but not add to payment. %1").arg(lostMoney.join("; ")));
	}
}

//---------------------------------------------------------------------------
void CashAcceptorManager::onChargeProviderStacked(SDK::PaymentProcessor::SNote aNote)
{
	if (mPaymentData)
	{
		double amount = aNote.nominal;
		auto paymentId = mPaymentData->paymentId;

		// Записываем в БД информацию о принятых купюрах.
		if (!mDatabase->addPaymentNote(paymentId, aNote))
		{
			toLog(LogLevel::Error, QString("Payment %1. Failed to update payment amount. Total sum: %2.").arg(paymentId).arg(amount));
		}

		mPaymentData->currentAmount += amount;

		emit PPSDK::ICashAcceptorManager::amountUpdated(paymentId, mPaymentData->currentAmount, amount);

		if (mPaymentData->maxAmountReached())
		{
			disable(paymentId);
		}
	}
}

//---------------------------------------------------------------------------
void CashAcceptorManager::onStatusChanged(DSDK::EWarningLevel::Enum aLevel, const QString & aTranslation, int aStatus)
{
	if (aLevel == DSDK::EWarningLevel::Error)
	{
		if (mPaymentData)
		{
			// Прервать прием денег.
			disable(mPaymentData->paymentId);

			emit error(mPaymentData->paymentId, aTranslation);
		}
	}
	else
	{
		DSDK::ICashAcceptor * acceptor = dynamic_cast<DSDK::ICashAcceptor *>(sender());

		// Нормальные статусы.
		if (aStatus == DSDK::ECashAcceptorStatus::Rejected)
		{
			// TODO: не обрабатывать непринятые купюры по запрещенным номиналам.
			incrementRejectCount();

			emit activity();
		}
		else if (aStatus == DSDK::ECashAcceptorStatus::Cheated)
		{
			emit cheated(mPaymentData ? mPaymentData->paymentId : -1);
		}
		else if (acceptor && aStatus == DSDK::ECashAcceptorStatus::Disabled)
		{
			// Отправляем сигнал об отключении купюроприемника.
			toLog(LogLevel::Debug, acceptor->getName() + " is disabled.");

			// Удаляем купюроприемник из списка включенных.
			if (mPaymentData)
			{
				toLog(LogLevel::Debug, QString("Payment %2. %1 was removed.").arg(acceptor->getName()).arg(mPaymentData->paymentId));

				mPaymentData->validators.remove(acceptor);

				// Если список пуст, сообщаем об окончании приема средств.
				if (mPaymentData->chargeSourceEmpty())
				{
					toLog(LogLevel::Debug, "All cash acceptors are disabled.");

					emit disabled(mPaymentData->paymentId);
				}
			}
		}
	}
}

//---------------------------------------------------------------------------
int CashAcceptorManager::getRejectCount() const
{
	auto dbUtils = DatabaseService::instance(mApplication)->getDatabaseUtils<IHardwareDatabaseUtils>();
	return dbUtils->getDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal, PPSDK::CDatabaseConstants::Parameters::RejectCount).toInt();
}

//---------------------------------------------------------------------------
void CashAcceptorManager::incrementRejectCount()
{
	auto dbUtils = DatabaseService::instance(mApplication)->getDatabaseUtils<IHardwareDatabaseUtils>();
	dbUtils->setDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal, PPSDK::CDatabaseConstants::Parameters::RejectCount, getRejectCount() + 1);
}

//---------------------------------------------------------------------------
void CashAcceptorManager::initWorkingParList()
{
	// Получаем список поддерживаемых номиналов.
	PPSDK::TerminalSettings * settings = SettingsService::instance(mApplication)->getAdapter<PPSDK::TerminalSettings>();
	PPSDK::SCommonSettings commonSettings = settings->getCommonSettings();

	foreach (auto nominal, commonSettings.enabledParNotesList)
	{
		mWorkingParList.append(DSDK::SPar(nominal, settings->getCurrencySettings().id, DSDK::ECashReceiver::BillAcceptor, commonSettings.isValid));
	}

	foreach (auto nominal, commonSettings.enabledParCoinsList)
	{
		mWorkingParList.append(DSDK::SPar(nominal, settings->getCurrencySettings().id, DSDK::ECashReceiver::CoinAcceptor, commonSettings.isValid));
	}
}

//---------------------------------------------------------------------------
bool CashAcceptorManager::isFixedAmountPayment(qint64 aPayment)
{
	PPSDK::IPaymentService * paymentService = PaymentService::instance(mApplication);

	auto providerID = paymentService->getPaymentField(aPayment, PPSDK::CPayment::Parameters::Provider);

	if (!providerID.isNull())
	{
		auto limits = paymentService->getProvider(providerID.value.toLongLong()).limits;

		if (limits.min == limits.max)
		{
			return true;
		}

		auto minAmount = paymentService->getPaymentField(aPayment, PPSDK::CPayment::Parameters::MinAmount);
		auto maxAmount = paymentService->getPaymentField(aPayment, PPSDK::CPayment::Parameters::MaxAmount);

		if (!minAmount.isNull() && !maxAmount.isNull() &&
			qFuzzyCompare(minAmount.value.toDouble(), maxAmount.value.toDouble()))
		{
			return true;
		}
	}

	return false;
}

//---------------------------------------------------------------------------
bool CashAcceptorManager::allowMoreMoney(SDK::PaymentProcessor::TPaymentAmount aAmount)
{
	QList<PPSDK::IPayment::SParameter> parameters;

	PPSDK::IPaymentService * paymentService = PaymentService::instance(mApplication);

	parameters = paymentService->getPaymentFields(mPaymentData->paymentId);

	auto amountAll = std::find_if(parameters.begin(), parameters.end(), [](const PPSDK::IPayment::SParameter & aParameter) -> bool
		{ return aParameter.name == PPSDK::CPayment::Parameters::AmountAll; });
	Q_ASSERT(amountAll != parameters.end());

	double newAmountAll = amountAll->value.toDouble() + aAmount;

	parameters.clear();
	parameters << PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::AmountAll, newAmountAll);
	parameters = paymentService->calculateCommission(parameters);

	auto change = std::find_if(parameters.begin(), parameters.end(), [](const PPSDK::IPayment::SParameter & aParameter) -> bool
		{ return aParameter.name == PPSDK::CPayment::Parameters::Change; });
	Q_ASSERT(change != parameters.end());

	return qFuzzyIsNull(change->value.toDouble());
}

//---------------------------------------------------------------------------
