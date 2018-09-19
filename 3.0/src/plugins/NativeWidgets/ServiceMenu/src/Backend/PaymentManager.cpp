/* @file Менеджер для работы с платежами */

// std
#include <algorithm>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QList>
#include <QtCore/QDebug>
#include <QtCore/QCache>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/IPaymentService.h>
#include <SDK/PaymentProcessor/Core/IDeviceService.h>
#include <SDK/PaymentProcessor/Core/IPrinterService.h>
#include <SDK/PaymentProcessor/Core/ICryptService.h>
#include <SDK/PaymentProcessor/Core/ReceiptTypes.h>
#include <SDK/PaymentProcessor/Core/ServiceParameters.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/PaymentProcessor/Payment/Step.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Payment/Security.h>
#include <SDK/Drivers/Components.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/Event.h>

// Modules
#include <Crypt/ICryptEngine.h>
#include <PaymentProcessor/PrintConstants.h>

// Project
#include "GUI/ServiceTags.h"
#include "PaymentManager.h"
#include "../GUI/PaymentInfo.h" //FIXME

namespace PPSDK = SDK::PaymentProcessor;
namespace CPayment = PPSDK::CPayment::Parameters;

//------------------------------------------------------------------------
namespace CPaymentManager
{
	const char UnprintedReest[] = "rup";
	const char UnprintedPaymentList[] = "[UNPRINTED_PAYMENT_LIST]";
}

//------------------------------------------------------------------------
PaymentManager::PaymentManager(PPSDK::ICore * aCore) :
	mCore(aCore),
	mUseFiscalPrinter(false),
	mPaymentsRegistryPrintJob(0)
{
	mPrinterService = mCore->getPrinterService();
	connect(mPrinterService, SIGNAL(receiptPrinted(int, bool)), this, SLOT(onReceiptPrinted(int, bool)));

	mPaymentService = mCore->getPaymentService();
	connect(mPaymentService, SIGNAL(stepCompleted(qint64, int, bool)), this, SIGNAL(paymentChanged(qint64)));

	mDealerSettings = static_cast<PPSDK::DealerSettings *>(mCore->getSettingsService()->
		getAdapter(PPSDK::CAdapterNames::DealerAdapter));
}

//------------------------------------------------------------------------
PaymentManager::~PaymentManager()
{
}

//------------------------------------------------------------------------
QVariantMap PaymentManager::balanceParameters(const SDK::PaymentProcessor::SBalance & aBalance) const
{
	QVariantMap cashInfo;
	if (aBalance.isValid)
	{
		cashInfo[CServiceTags::LastEncashmentDate] = aBalance.lastEncashmentDate;
		cashInfo[CServiceTags::CashAmount] = aBalance.amount;

		auto fields = aBalance.getFields();

		cashInfo[CServiceTags::NoteCount] = fields["BILL_COUNT"];
		cashInfo[CServiceTags::CoinCount] = fields["COIN_COUNT"];
	}

	return cashInfo;
}

//------------------------------------------------------------------------
QVariantMap PaymentManager::getBalanceInfo() const
{
	return balanceParameters(mPaymentService->getBalance());
}

//------------------------------------------------------------------------
QVariantMap PaymentManager::getEncashmentInfo(int aIndex) const
{
	if (mEncashmentList.size() > aIndex && aIndex >= 0)
	{
		auto encashment = mEncashmentList[aIndex];
		auto info = balanceParameters(encashment.balance);

		return info.unite(encashment.getFields());
	}

	return QVariantMap();
}

//------------------------------------------------------------------------
SDK::PaymentProcessor::EncashmentResult::Enum PaymentManager::perform(const QVariantMap & aParameters)
{
	mCore->getEventService()->sendEvent(PPSDK::Event(PPSDK::EEventType::ProcessEncashment));

	if (mPaymentService->getBalance().isEmpty())
	{
		mEncashment = mPaymentService->getLastEncashment();

		return mEncashment.isValid() ? PPSDK::EncashmentResult::OK : PPSDK::EncashmentResult::Error;
	}

	return mPaymentService->performEncashment(aParameters, mEncashment);
}

//------------------------------------------------------------------------
bool PaymentManager::canPrint(const QString & aReceiptType) const
{
	return mPrinterService->canPrintReceipt(aReceiptType, false);
}

//------------------------------------------------------------------------
QString PaymentManager::decryptParameter(const QString & aValue)
{
	ICryptEngine * cryptEngine = mCore->getCryptService()->getCryptEngine();

	QByteArray decryptedValue;

	QString error;

	if (cryptEngine->decryptLong(-1, aValue.toLatin1(), decryptedValue, error))
	{
		return QString::fromUtf8(decryptedValue);
	}

	return "**DECRYPT ERROR**";
}

//------------------------------------------------------------------------
bool PaymentManager::printReceipt(qint64 aPaymentId, bool aContinuousMode)
{
	QList<PPSDK::IPayment::SParameter> paymentParams = mPaymentService->getPaymentFields(aPaymentId);
	qint64 providerId = PPSDK::IPayment::parameterByName(CPayment::Provider, paymentParams).value.toLongLong();
	PPSDK::DealerSettings * dealerSettings = static_cast<PPSDK::DealerSettings *>(mCore->getSettingsService()->
		getAdapter(PPSDK::CAdapterNames::DealerAdapter));
	PPSDK::SProvider provider = dealerSettings->getProvider(providerId);

	QString receiptTemplate = provider.receipts.contains("default") ? provider.receipts["default"].value<QString>().replace(".xml", "") : PPSDK::CReceiptType::Payment;

	QVariantMap receiptParameters;
	foreach (PPSDK::IPayment::SParameter parameter, paymentParams)
	{
		receiptParameters[parameter.name] = parameter.crypted ? decryptParameter(parameter.value.toString()) : parameter.value;
	}

	receiptParameters[CPrintConstants::OpBrand] = provider.name;

	foreach (QString parameter, provider.receiptParameters.keys())
	{
		receiptParameters[parameter] = provider.receiptParameters[parameter];
	}

	int jonIndex = mPrinterService->printReceipt(PPSDK::CReceiptType::Payment, receiptParameters, receiptTemplate, aContinuousMode, true);

	mPaymentPrintJobs.insert(jonIndex, aPaymentId);

	return jonIndex != 0;
}

//------------------------------------------------------------------------
bool PaymentManager::printUnprintedReceiptsRegistry(const QSet<qint64> & aPayments)
{
	struct PaymentAmounts
	{
		QVariantList summAmounts;
		double summAmountAll;
		double summDealerFee;
		double summProcessingFee;
		QStringList registry;
		QStringList paymentTitles;
		QVariantList paymentsVAT;
		QStringList paymentInn;

		PaymentAmounts()
		{
			summAmountAll = 0.0;
			summDealerFee = 0.0;
			summProcessingFee = 0.0;
		}
	};

	// группируем суммы по типу платежных средств
	QMap<int, PaymentAmounts> amounts;

	auto formatPaymentTitle = [](const PPSDK::SProvider & aProvider) -> QString {
		return QString("%1 (%2)")
			.arg(aProvider.receiptParameters[CPrintConstants::ServiceType].toString())
			.arg(aProvider.name);
	};

	auto payments = aPayments.toList();
	qSort(payments);

	foreach (qint64 id, payments)
	{
		QString session, amountAll;
		double amount = 0.0;
		double dealerFee = 0.0;
		double processingFee = 0.0;
		qint64 providerId = -1;
		qint64 getewayIn = 0;
		qint64 getewayOut = 0;
		int vat = 0;
		int payTool = 0;

		foreach (auto parameter, mPaymentService->getPaymentFields(id))
		{
			     if (parameter.name == CPayment::Amount)         amount        = parameter.value.toDouble();
			else if (parameter.name == CPayment::DealerFee)      dealerFee     = parameter.value.toDouble();
			else if (parameter.name == CPayment::ProcessingFee)  processingFee = parameter.value.toDouble();
			else if (parameter.name == CPayment::AmountAll)      amountAll     = parameter.value.toString();
			else if (parameter.name == CPayment::InitialSession) session       = parameter.value.toString();
			else if (parameter.name == CPayment::Provider)       providerId    = parameter.value.toLongLong();
			else if (parameter.name == CPayment::MNPGetewayIn)   getewayIn     = parameter.value.toLongLong();
			else if (parameter.name == CPayment::MNPGetewayOut)  getewayOut    = parameter.value.toLongLong();
			else if (parameter.name == CPayment::Vat)            vat           = parameter.value.toInt();
			else if (parameter.name == CPayment::PayTool)        payTool       = parameter.value.toInt();
		}

		if (!qFuzzyIsNull(amountAll.toDouble()))
		{
			auto provider = mDealerSettings->getMNPProvider(providerId, getewayIn, getewayOut);

			amounts[payTool].summAmountAll += amountAll.toDouble();
			amounts[payTool].summAmounts << amount;
			amounts[payTool].paymentTitles << formatPaymentTitle(provider);
			amounts[payTool].paymentsVAT << vat;
			amounts[payTool].paymentInn << provider.receiptParameters.value("OPERATOR_INN").toString();
			amounts[payTool].summDealerFee += dealerFee;
			amounts[payTool].summProcessingFee += processingFee;

			amounts[payTool].registry << QString("%1 %2 %3").arg(id).arg(session).arg(amountAll);
		}
	}

	bool ok = false;

	foreach (int payTool, amounts.keys())
	{
		if (!amounts[payTool].registry.isEmpty())
		{
			QVariantMap receiptParameters;
			receiptParameters[CPaymentManager::UnprintedPaymentList] = amounts[payTool].registry;
			receiptParameters[CPayment::AmountAll] = amounts[payTool].summAmountAll;
			receiptParameters[QString("[%1]").arg(CPayment::Amount)] = amounts[payTool].summAmounts;
			receiptParameters["[AMOUNT_TITLE]"] = amounts[payTool].paymentTitles;
			receiptParameters["[AMOUNT_VAT]"] = amounts[payTool].paymentsVAT;
			receiptParameters["[OPERATOR_INN]"] = amounts[payTool].paymentInn;
			receiptParameters[CPayment::PayTool] = payTool;
			receiptParameters[CPayment::DealerFee] = amounts[payTool].summDealerFee;
			receiptParameters[CPayment::Fee] = amounts[payTool].summDealerFee + amounts[payTool].summProcessingFee;
			receiptParameters[CPayment::ProcessingFee] = amounts[payTool].summProcessingFee;

			mPaymentsRegistryPrintJob = mPrinterService->printReceipt(PPSDK::CReceiptType::Payment, receiptParameters, CPaymentManager::UnprintedReest, true, true);
			
			ok = ok || (mPaymentsRegistryPrintJob != 0);
		}
	}

	return ok;
}

//------------------------------------------------------------------------
void PaymentManager::onReceiptPrinted(int aJobIndex, bool aErrorHappened)
{
	if (mPaymentsRegistryPrintJob == aJobIndex)
	{
		if (!aErrorHappened)
		{
			// Обновить платёжи - чек напечатан
			foreach(auto paymentId, mEncashment.balance.notPrintedPayments)
			{
				mPaymentService->updatePaymentField(paymentId, PPSDK::IPayment::SParameter(CPayment::ReceiptPrinted, true, true), true);
			}

			mEncashment.balance.notPrintedPayments.clear();
		}

		mPaymentsRegistryPrintJob = 0;

		// Если была задача снять Z-отчёт и мы напечатали суммарный список платежей - выполняем сам отчёт.
		if (!mNeedPrintZReport.isEmpty() && aErrorHappened)
		{
			mPrinterService->printReport(mNeedPrintZReport, QVariantMap());

			mNeedPrintZReport.clear();
		}
	}
	else if (mPaymentPrintJobs.contains(aJobIndex))
	{
		qint64 paymentId = mPaymentPrintJobs.value(aJobIndex);

		if (!aErrorHappened)
		{
			// Обновить платёж - чек напечатан
			mPaymentService->updatePaymentField(paymentId, PPSDK::IPayment::SParameter(CPayment::ReceiptPrinted, true, true), true);

			mEncashment.balance.notPrintedPayments.remove(paymentId);

			emit paymentChanged(paymentId);
		}

		mPaymentPrintJobs.remove(aJobIndex);
		emit receiptPrinted(paymentId, aErrorHappened);
	}
	else
	{
		emit receiptPrinted(0, aErrorHappened);
	}
}

//------------------------------------------------------------------------
bool PaymentManager::printEncashment(int aIndex /*= -1*/)
{
	PPSDK::SEncashment emptyEncashment;
	PPSDK::SEncashment & encashment = mEncashment;

	if (aIndex >= 0)
	{
		encashment = (mEncashmentList.size() > aIndex) ? mEncashmentList[aIndex] : emptyEncashment;
	}

	if (!encashment.isValid())
	{
		return false;
	}

	if (mUseFiscalPrinter && aIndex < 0)
	{
		PPSDK::TerminalSettings * terminalSettings = static_cast<PPSDK::TerminalSettings *>(mCore->getSettingsService()->
			getAdapter(PPSDK::CAdapterNames::TerminalAdapter));

		if (terminalSettings->getCommonSettings().printFailedReceipts)
		{
			// Перед отчётом печатаем все не напечатанные чеки
			printUnprintedReceiptsRegistry(encashment.balance.notPrintedPayments);
		}
		else
		{
			encashment.balance.notPrintedPayments.clear();
		}
	}

	auto fields = encashment.getFields();
	bool result = false;
	
	if (aIndex >= 0)
	{
		// печатаем копию инкассации
		fields.insert("NO_FISCAL", QVariant());
	}

	result = (mPrinterService->printReport(PPSDK::CReceiptType::Encashment, fields) != 0);

	// Если есть устройство диспенсер
	if (!mCore->getDeviceService()->getConfigurations().filter(QRegExp(DSDK::CComponents::Dispenser)).isEmpty())
	{
		mPrinterService->printReceipt(PPSDK::CReceiptType::DispenserEncashment, fields, PPSDK::CReceiptType::DispenserEncashment, false, true);
	}

	return result;
}

//------------------------------------------------------------------------
bool PaymentManager::printTestPage()
{
	return (mPrinterService->printReceipt(PPSDK::CReceiptType::Test, QVariantMap(), PPSDK::CReceiptType::Test, false, true) != 0);
}

//------------------------------------------------------------------------
bool PaymentManager::printBalance() const
{
	auto fields = mPaymentService->getBalance().getFields();
	bool result = (mPrinterService->printReport(PPSDK::CReceiptType::Balance, fields) != 0);

	// Если есть устройство диспенсер
	if (!mCore->getDeviceService()->getConfigurations().filter(QRegExp(DSDK::CComponents::Dispenser)).isEmpty())
	{
		mPrinterService->printReceipt(PPSDK::CReceiptType::DispenserBalance, fields, PPSDK::CReceiptType::DispenserBalance, false, true);
	}

	return result;
}

//------------------------------------------------------------------------
bool PaymentManager::printZReport(bool aFullZReport)
{
	if (mEncashment.balance.notPrintedPayments.isEmpty() || !mUseFiscalPrinter)
	{
		return mPrinterService->printReport(aFullZReport ? PPSDK::CReceiptType::ZReportFull : PPSDK::CReceiptType::ZReport, QVariantMap());
	}
	else
	{
		if (mPaymentsRegistryPrintJob == 0)
		{
			// Перед отчётом печатаем все не напечатанные чеки
			printUnprintedReceiptsRegistry(mEncashment.balance.notPrintedPayments);
		}

		// Оставляем пометку что мы хотели напечатать Z-отчет, но у нас есть не напечатанные чеки
		mNeedPrintZReport = aFullZReport ? PPSDK::CReceiptType::ZReportFull : PPSDK::CReceiptType::ZReport;

		return false;
	}
}

//------------------------------------------------------------------------
bool PaymentManager::getPaymentsInfo(QVariantMap & /*aPaymentsInfo*/) const
{
	QVariantMap result;

	foreach(PPSDK::IService * service, mCore->getServices())
	{
		result.unite(service->getParameters());
	}

	// TODO Заполнить поля
	/*aPaymentsInfo[]
	lbRejectedBills->setText(QString::number(result[SDK::PaymentProcessor::CServiceParameters::Funds::RejectCount].toInt()));
	lbUnprocessedPayments->setText(QString::number(result[PPSDK::CServiceParameters::Payment::UnprocessedPaymentCount].toInt()));
	lbPrintedReceipts->setText(QString::number(result[SDK::PaymentProcessor::CServiceParameters::Printing::ReceiptCount].toInt()));
	lbRestartPerDay->setText(QString::number(result[SDK::PaymentProcessor::CServiceParameters::Terminal::RestartCount].toInt()));
	lbPaymentsPerDay->setText(QString::number(result[SDK::PaymentProcessor::CServiceParameters::Payment::PaymentsPerDay].toInt()));*/

	return true;
}

//------------------------------------------------------------------------
PaymentInfo PaymentManager::loadPayment(const QList<PPSDK::IPayment::SParameter> & aPaymentParams)
{
	PaymentInfo paymentInfo;

	int status = PPSDK::IPayment::parameterByName(CPayment::Status, aPaymentParams).value.toInt();
	paymentInfo.setStatus(static_cast<PPSDK::EPaymentStatus::Enum>(status));

	paymentInfo.setPrinted(PPSDK::IPayment::parameterByName(CPayment::ReceiptPrinted, aPaymentParams).value.toBool());

	paymentInfo.setId(PPSDK::IPayment::parameterByName(CPayment::ID, aPaymentParams).value.toLongLong());
	paymentInfo.setAmout(PPSDK::IPayment::parameterByName(CPayment::Amount, aPaymentParams).value.toFloat());
	paymentInfo.setAmountAll(PPSDK::IPayment::parameterByName(CPayment::AmountAll, aPaymentParams).value.toFloat());
	paymentInfo.setCreationDate(PPSDK::IPayment::parameterByName(CPayment::CreationDate, aPaymentParams).value.toDateTime());
	paymentInfo.setLastUpdate(PPSDK::IPayment::parameterByName(CPayment::LastUpdateDate, aPaymentParams).value.toDateTime());

	paymentInfo.setSession(PPSDK::IPayment::parameterByName(CPayment::Session, aPaymentParams).value.toString());
	paymentInfo.setInitialSession(PPSDK::IPayment::parameterByName(CPayment::InitialSession, aPaymentParams).value.toString());
	paymentInfo.setTransId(PPSDK::IPayment::parameterByName(CPayment::TransactionId, aPaymentParams).value.toString());

	qint64 providerId = PPSDK::IPayment::parameterByName(CPayment::Provider, aPaymentParams).value.toLongLong();

	PPSDK::SProvider provider = mDealerSettings->getProvider(providerId);
	if (provider.name.isEmpty())
	{
		paymentInfo.setProvider(paymentInfo.getStatus() == PPSDK::EPaymentStatus::LostChange ? tr("#lost_change") : QString::number(providerId));
	}
	else
	{
		paymentInfo.setProvider(QString("%1 (%2)").arg(provider.name).arg(providerId));
	}

	PPSDK::SecurityFilter filter(provider, PPSDK::SProviderField::SecuritySubsystem::Display);

	QStringList providerFields;
	foreach (PPSDK::SProviderField pf, provider.fields)
	{
		auto p = PPSDK::IPayment::parameterByName(pf.id, aPaymentParams);
		providerFields << pf.title + ": " + filter.apply(pf.id, p.crypted ? decryptParameter(p.value.toString()) : p.value.toString());
	}

	paymentInfo.setProviderFields(providerFields.join("\n"));

	return paymentInfo;
}

//------------------------------------------------------------------------
void PaymentManager::updatePaymentList()
{
	mPaymentList.clear();

	QList<qint64> payments = mPaymentService->getPayments(QSet<PPSDK::EPaymentStatus::Enum>());

	auto parameters = mPaymentService->getPaymentsFields(payments);

	for (auto it = parameters.constBegin(); it != parameters.constEnd(); ++it)
	{
		mPaymentList << loadPayment(it.value());
	}
}

//------------------------------------------------------------------------
QList<PaymentInfo> PaymentManager::getPayments(bool aNeedUpdate)
{
	if (aNeedUpdate)
	{
		updatePaymentList();
	}

	return mPaymentList;
}

//------------------------------------------------------------------------
void PaymentManager::processPayment(qint64 id)
{
	if (mPaymentService->canProcessPaymentOffline(id))
	{
		mPaymentService->processPayment(id, false);
	}
}

//------------------------------------------------------------------------
PaymentInfo PaymentManager::getPayment(qint64 id)
{
	return loadPayment(mPaymentService->getPaymentFields(id));
}

//------------------------------------------------------------------------
void PaymentManager::useHardwareFiscalPrinter(bool aUseFiscalPrinter)
{
	mUseFiscalPrinter = aUseFiscalPrinter;
}

//------------------------------------------------------------------------
int PaymentManager::getEncashmentsHistoryCount()
{
	mEncashmentList = mPaymentService->getEncashmentList(10);

	// Удаляем первую "техническую" инкасацию
	mEncashmentList.erase(std::remove_if(mEncashmentList.begin(), mEncashmentList.end(),
		[](const PPSDK::SEncashment & aEncashment) -> bool { return aEncashment.id == 1; }), mEncashmentList.end());

	return mEncashmentList.size();
}

//------------------------------------------------------------------------