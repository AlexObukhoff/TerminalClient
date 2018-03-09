/* @file Команды печати и формирования чеков. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ReceiptTypes.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/Drivers/FR/FiscalFields.h>
#include <SDK/Drivers/HardwareConstants.h>

// Common
#include <System/IApplication.h>
#include <Common/Application.h>

// Project
#include "PrintingCommands.h"
#include "PrintingService.h"
#include "PrintConstants.h"
#include "PaymentService.h"

namespace FiscalCommand = SDK::Driver::EFiscalPrinterCommand;
namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
void PrintCommand::setReceiptTemplate(const QString & aTemplateName)
{
	mReceiptTemplate = aTemplateName;
}

//---------------------------------------------------------------------------
PrintFiscalCommand::PrintFiscalCommand(const QString & aReceiptType, FiscalCommand::Enum aFiscalCommand, PrintingService * aService) :
	PrintCommand(aReceiptType), 
	mFiscalCommand(aFiscalCommand), 
	mService(aService)
{
}

//---------------------------------------------------------------------------
QVariantMap toUpperCaseKeys(const QVariantMap & aParameters)
{
	QVariantMap result;

	foreach (auto key, aParameters.keys())
	{
		result.insert(key.toUpper(), aParameters.value(key));
	}

	return result;
}

//---------------------------------------------------------------------------
SDK::Driver::SPaymentData PrintFiscalCommand::getPaymentData(const QVariantMap & aParameters)
{
	DSDK::TAmountDataList fiscalAmountList;

	bool dealerIsBank = aParameters.value(CPrintConstants::DealerIsBank, false).toBool();
	DSDK::TVAT dealerVAT = aParameters.value(CPrintConstants::DealerVAT, 0).toInt();

	double amount = aParameters.value("AMOUNT").toDouble();
	QVariant amountList = aParameters.value("[AMOUNT]");
	double processingFee = aParameters.value("PROCESSING_FEE").toDouble();
	double fee = aParameters.value("DEALER_FEE").toDouble();
	int vat = aParameters.value("VAT").toInt();

	if (amountList.isNull())
	{
		QString paymentTitle = QString("%1 (%2)")
			.arg(aParameters[CPrintConstants::ServiceType].toString())
			.arg(aParameters[CPrintConstants::OpBrand].toString());

		fiscalAmountList << DSDK::SAmountData(amount, vat, paymentTitle, DSDK::EPayOffSubjectTypes::Payment);
	}
	else
	{
		QVariantList amounts = amountList.toList();
		QVariantList amountTitles = aParameters.value("[AMOUNT_TITLE]").toList();
		QVariantList amountsVAT = aParameters.value("[AMOUNT_VAT]").toList();

		// amount содержит список сумм для печати реестра нераспечатанных чеков
		for (int i = 0; i < amounts.size(); i++)
		{
			fiscalAmountList << DSDK::SAmountData(amounts[i].toDouble(), amountsVAT[i].toInt(), amountTitles[i].toString(), DSDK::EPayOffSubjectTypes::Payment);
		}
	}

	if (!qFuzzyIsNull(fee))
	{
		fiscalAmountList << (dealerIsBank ?
			DSDK::SAmountData(fee, dealerVAT, tr("#bank_fee"), DSDK::EPayOffSubjectTypes::Payment) :
			DSDK::SAmountData(fee, dealerVAT, tr("#dealer_fee"), DSDK::EPayOffSubjectTypes::AgentFee));
	}

	if (!qFuzzyIsNull(processingFee))
	{
		fiscalAmountList << DSDK::SAmountData(processingFee, 0, tr("#processing_fee"), DSDK::EPayOffSubjectTypes::Payment);
	}

	bool EMoney = aParameters.value(PPSDK::CPayment::Parameters::PayTool).toInt() > 0;
	auto payType = EMoney ? DSDK::EPayTypes::EMoney : DSDK::EPayTypes::Cash;
	auto taxation  = aParameters.contains(CPrintConstants::DealerTaxation)  ? static_cast<DSDK::ETaxations::Enum> (aParameters.value(CPrintConstants::DealerTaxation).toInt()) : DSDK::ETaxations::None;
	auto agentFlag = aParameters.contains(CPrintConstants::DealerAgentFlag) ? static_cast<DSDK::EAgentFlags::Enum>(aParameters.value(CPrintConstants::DealerAgentFlag).toInt()) : DSDK::EAgentFlags::None;

	DSDK::SPaymentData result(fiscalAmountList, false, payType, taxation, agentFlag);

	QVariantMap upperKeyParameters = toUpperCaseKeys(aParameters);
	QRegExp phoneRegexp("^9\\d{9}$");

	result.fiscalParameters[CHardwareSDK::FR::UserPhone] = QString();

	foreach (auto fieldName, QStringList() << "100" << "PHONE" << "CONTACT" << "101" << "102" << "103" << "104")
	{
		if (upperKeyParameters.contains(fieldName) && phoneRegexp.exactMatch(upperKeyParameters.value(fieldName).toString()))
		{
			result.fiscalParameters[CHardwareSDK::FR::UserPhone] = "7" + upperKeyParameters.value(fieldName).toString();
			break;
		}
	}

	result.fiscalParameters[CHardwareSDK::FR::UserMail] = QString();

	foreach (auto fieldName, QStringList() << "PAYER_EMAIL")
	{
		if (upperKeyParameters.contains(fieldName))
		{
			QString email = upperKeyParameters.value(fieldName).toString();
			if (!email.isEmpty() && email.contains("@") && email.contains("."))
			{
				result.fiscalParameters[CHardwareSDK::FR::UserMail] = upperKeyParameters.value(fieldName).toString();
				break;
			}
		}
	}

	return result;
}

//---------------------------------------------------------------------------
bool PrintFiscalCommand::canFiscalPrint(DSDK::IPrinter * aPrinter, bool aRealCheck)
{
	auto fr = dynamic_cast<DSDK::IFiscalPrinter *>(aPrinter);

	return (mService->getFiscalRegister() && PrintCommand::canPrint(aPrinter, aRealCheck)) ||
		(fr && fr->isFiscalReady(aRealCheck, mFiscalCommand));
}

//---------------------------------------------------------------------------
bool PrintFiscalCommand::getFiscalInfo(QVariantMap & aParameters, QStringList & aReceiptLines)
{
	PPSDK::IFiscalRegister * fr = mService->getFiscalRegister();

	if (fr && fr->haveCapability(PPSDK::IFiscalRegister::Receipt))
	{
		qint64 paymentId = aParameters.value(PPSDK::CPayment::Parameters::ID).toLongLong();
		QStringList parameterNames = fr->getParameterNames();

		// Если в параметрах платежа ещё нет информации о фискальном номере
		bool OK = !parameterNames.toSet().intersect(aParameters.keys().toSet()).isEmpty();
	
		if (!OK)
		{
			auto fiscalParameters = fr->createFiscalTicket(paymentId, aParameters, getPaymentData(aParameters));
			OK = !fiscalParameters.isEmpty();
		
			aParameters.unite(fiscalParameters);

			mService->setFiscalNumber(paymentId, fiscalParameters);
		}

		if (OK)
		{
			aReceiptLines = fr->getReceipt(paymentId, aParameters);
		}

		return OK;
	}

	return false;
}

//---------------------------------------------------------------------------
bool PrintPayment::canPrint(DSDK::IPrinter * aPrinter, bool aRealCheck)
{
	if (!aPrinter)
	{
		return false;
	}

	auto fr = dynamic_cast<DSDK::IFiscalPrinter *>(aPrinter);

	return isFiscal(aPrinter) ? fr->isFiscalReady(aRealCheck, FiscalCommand::Sale) : aPrinter->isDeviceReady(aRealCheck);
}

//---------------------------------------------------------------------------
bool PrintPayment::print(DSDK::IPrinter * aPrinter, const QVariantMap & aParameters)
{
	QVariantMap parameters = aParameters;
	QStringList receipt;
	getFiscalInfo(parameters, receipt);

	// Добавляем строки основного чека
	QVariantMap configuration = aPrinter->getDeviceConfiguration();
	bool onlineKKM = configuration[CHardwareSDK::CanOnline].toBool();
	bool fiscalPrinting = isFiscal(aPrinter) && !aParameters[PPSDK::CPayment::Parameters::ReceiptPrinted].toInt();

	QString KKMSerialNumber = "0";

	if (!fiscalPrinting)
	{
		KKMSerialNumber = configuration[CHardwareSDK::SerialNumber].toString();
	}

	QVariantMap actualParameters = aParameters;
	actualParameters.insert(CPrintConstants::KKM::SerialNumber, KKMSerialNumber);
	actualParameters.insert("ONLINE_KKM", onlineKKM ? 1 : 0);

	receipt.append(mService->getReceipt(mReceiptTemplate, actualParameters));

	bool result = false;

	// Повторно мы печатаем только нефискальные чеки
	if (!fiscalPrinting)
	{
		result = canPrint(aPrinter, false) && aPrinter->print(receipt);
	}
	else if (canFiscalPrint(aPrinter, false))
	{
		DSDK::SPaymentData paymentData = getPaymentData(actualParameters);

		static_cast<DSDK::IFiscalPrinter *>(aPrinter)->setDeviceConfiguration(paymentData.fiscalParameters);
		result = static_cast<DSDK::IFiscalPrinter *>(aPrinter)->printFiscal(receipt, paymentData, mFiscalPaymentData, mPayOffSubjectData);

		if (!mFiscalPaymentData.isEmpty())
		{
			#define ADD_FISCAL_TAG(aTranstation, aFiscalTag) receipt << aTranstation + ":  " + mFiscalPaymentData[DSDK::FiscalFields::aFiscalTag].toString();

			ADD_FISCAL_TAG(tr("#taxation"),      TaxSystem);
			ADD_FISCAL_TAG(tr("#kkt_timestamp"), FDDateTime);
			ADD_FISCAL_TAG(tr("#kkt_znm"),       SerialFRNumber);
			ADD_FISCAL_TAG(tr("#kkt_rnm"),       RNM);
			ADD_FISCAL_TAG(tr("#kkt_session"),   SessionNumber);
			ADD_FISCAL_TAG(tr("#kkt_fd_serial"), DocumentNumber);
			ADD_FISCAL_TAG(tr("#kkt_fn"),        SerialFSNumber);
			ADD_FISCAL_TAG(tr("#kkt_fd"),        FDNumber);
			ADD_FISCAL_TAG(tr("#kkt_fp"),        FDSign);

			ADD_FISCAL_TAG(tr("#tax_amount_02"), TaxAmount02);
			ADD_FISCAL_TAG(tr("#tax_amount_03"), TaxAmount03);
			ADD_FISCAL_TAG(tr("#tax_amount_04"), TaxAmount04);
			ADD_FISCAL_TAG(tr("#tax_amount_05"), TaxAmount05);
		}
	}

	// Сохраняем чек на диске.
	QString receiptName = QTime::currentTime().toString(CPrintCommands::ReceiptNameTemplate) + "_" + aParameters["ID"].toString() +
		(result ? "" : CPrintCommands::NotPrintedPostfix);
	mService->saveReceiptContent(receiptName, receipt);

	return result;
}

//---------------------------------------------------------------------------
const DSDK::TFiscalPaymentData & PrintPayment::getFiscalData() const
{
	return mFiscalPaymentData;
}

//---------------------------------------------------------------------------
bool PrintPayment::isFiscal(DSDK::IPrinter * aPrinter)
{
	DSDK::IFiscalPrinter * fiscalPrinter = dynamic_cast<DSDK::IFiscalPrinter *>(aPrinter);

	return fiscalPrinter && fiscalPrinter->isFiscal();
}

//---------------------------------------------------------------------------
bool PrintBalance::print(DSDK::IPrinter * aPrinter, const QVariantMap & aParameters)
{
	QStringList receipt = mService->getReceipt(mReceiptType, expandFields(aParameters));
	auto fr = dynamic_cast<DSDK::IFiscalPrinter *>(aPrinter);

	auto virtualFR = mService->getFiscalRegister();

	if (mFiscalMode && virtualFR && virtualFR->haveCapability(PPSDK::IFiscalRegister::Balance))
	{
		receipt.append(virtualFR->balance());
	}

	mService->saveReceiptContent(QString("%1_balance").arg(QTime::currentTime().toString("hhmmsszzz")), receipt);

	return mFiscalMode && fr && fr->isFiscalReady(false, mFiscalCommand) ? fr->printXReport(receipt) : aPrinter->print(receipt);
}

//---------------------------------------------------------------------------
QVariantMap PrintBalance::expandFields(const QVariantMap & aParameters)
{
	QVariantMap parameters;

	auto currencySettings = mService->getCurrencySettings();

	auto fillParameters = [&](const QString & aPrefix)
	{
		//монеты
		foreach (auto coin, currencySettings.coins)
		{
			parameters[aPrefix + QString("%1_COIN_COUNT").arg(coin.toString())] = 0;
			parameters[aPrefix + QString("%1_COIN_SUM").arg(coin.toString())] = 0;
		}

		parameters[aPrefix + "COIN_COUNT"] = 0;
		parameters[aPrefix + "COIN_SUM"] = 0;

		//купюры
		foreach (auto note, currencySettings.notes)
		{
			parameters[aPrefix + QString("%1_BILL_COUNT").arg(note.toString())] = 0;
			parameters[aPrefix + QString("%1_BILL_SUM").arg(note.toString())] = 0;
		}

		parameters[aPrefix + "BILL_COUNT"] = 0;
		parameters[aPrefix + "BILL_SUM"] = 0;
	};

	fillParameters("");
	fillParameters("DISPENSED_");

	for (auto i = aParameters.begin(); i != aParameters.end(); ++i)
	{
		parameters.insert(i.key(), i.value());
	}

	return parameters;
}

//---------------------------------------------------------------------------
PrintEncashment::PrintEncashment(const QString & aReceiptType, PrintingService * aService) :
	PrintBalance(aReceiptType, aService)
{
	mFiscalCommand = FiscalCommand::Encashment;
}

//---------------------------------------------------------------------------
bool PrintEncashment::print(DSDK::IPrinter * aPrinter, const QVariantMap & aParameters)
{
	// Сохраняем чек перед печатью.
	QStringList receipt = mService->getReceipt(mReceiptType, expandFields(aParameters));

	auto virtualFR = mService->getFiscalRegister();

	// Производим выплату фискального регистратора.
	if (mFiscalMode && virtualFR && virtualFR->haveCapability(PPSDK::IFiscalRegister::Encashment))
	{
		receipt.append(virtualFR->encashment());
	}

	mService->saveReceiptContent(QString("%1_%2_encashment").arg(QTime::currentTime().toString("hhmmsszzz")).arg(aParameters["ENCASHMENT_NUMBER"].toString()), receipt);

	auto fr = dynamic_cast<DSDK::IFiscalPrinter *>(aPrinter);

	if (!canPrint(aPrinter, false))
	{
		return false;
	}

	return (mFiscalMode && fr && fr->isFiscalReady(false, mFiscalCommand)) ? fr->printEncashment(receipt) : (aPrinter && aPrinter->print(receipt));
}

//---------------------------------------------------------------------------
bool PrintZReport::canPrint(DSDK::IPrinter * aPrinter, bool aRealCheck)
{
	auto fr = dynamic_cast<DSDK::IFiscalPrinter *>(aPrinter);

	auto virtualFR = mService->getFiscalRegister();

	return (virtualFR && virtualFR->haveCapability(PPSDK::IFiscalRegister::ZReport) && PrintCommand::canPrint(aPrinter, aRealCheck)) ||
		(fr && fr->isFiscalReady(aRealCheck, mFiscalCommand));
}

//---------------------------------------------------------------------------
bool PrintZReport::print(DSDK::IPrinter * aPrinter, const QVariantMap & /*aParameters*/)
{
	bool result = false;

	if (!canFiscalPrint(aPrinter, false))
	{
		return false;
	}

	auto virtualFR = mService->getFiscalRegister();

	if (virtualFR && virtualFR->haveCapability(PPSDK::IFiscalRegister::ZReport))
	{
		QStringList report = virtualFR->getZreport();
		
		if (!report.isEmpty())
		{
			result = aPrinter && aPrinter->print(report);

			mService->saveReceiptContent(QString("%1_Z_report%2")
				.arg(QTime::currentTime().toString("hhmmsszzz"))
				.arg(result ? "" : CPrintCommands::NotPrintedPostfix), report);
		}
	}

	auto fr = dynamic_cast<DSDK::IFiscalPrinter *>(aPrinter);

	return (fr && fr->isFiscalReady(false, mFiscalCommand) && fr->printZReport(mFull)) || result;
}

//---------------------------------------------------------------------------
bool PrintReceipt::print(DSDK::IPrinter * aPrinter, const QVariantMap & aParameters)
{
	QVariantMap configuration = aPrinter->getDeviceConfiguration();
	QString KKMSerialNumber = configuration[CHardwareSDK::SerialNumber].toString();

	if (KKMSerialNumber.isEmpty())
	{
		KKMSerialNumber = "0";
	}

	QVariantMap actualParameters = aParameters;
	actualParameters.insert(CPrintConstants::KKM::SerialNumber, KKMSerialNumber);
	QStringList receipt = mService->getReceipt(mReceiptTemplate, actualParameters);

	mService->saveReceiptContent(QString("%1_%2").arg(QTime::currentTime().toString("hhmmsszzz")).arg(mReceiptTemplate), receipt);

	return aPrinter && aPrinter->print(receipt);
}

//---------------------------------------------------------------------------
