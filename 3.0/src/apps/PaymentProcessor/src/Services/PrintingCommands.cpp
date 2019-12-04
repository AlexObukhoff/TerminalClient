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

#if 0
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
#endif

//---------------------------------------------------------------------------
DSDK::SPaymentData PrintFiscalCommand::getPaymentData(const QVariantMap & aParameters)
{
	DSDK::TUnitDataList unitDataList;

	bool dealerIsBank = aParameters.value(CPrintConstants::DealerIsBank, false).toBool();
	DSDK::TVAT dealerVAT = aParameters.value(CPrintConstants::DealerVAT, 0).toInt();

	double amount = aParameters.value("AMOUNT").toDouble();
	QVariant amountList = aParameters.value("[AMOUNT]");
	double processingFee = aParameters.value("PROCESSING_FEE").toDouble();
	double fee = aParameters.value("DEALER_FEE").toDouble();
	int vat = aParameters.value("VAT").toInt();

	if (amountList.isNull())
	{
		QString operatorINN = aParameters.value(CPrintConstants::OpINN).toString();
		QString paymentTitle = QString("%1 (%2)")
			.arg(aParameters[CPrintConstants::ServiceType].toString())
			.arg(aParameters[CPrintConstants::OpBrand].toString());

		unitDataList << DSDK::SUnitData(amount, vat, paymentTitle, operatorINN, DSDK::EPayOffSubjectTypes::Payment);
	}
	else
	{
		QVariantList amounts = amountList.toList();
		QVariantList amountTitles = aParameters.value("[AMOUNT_TITLE]").toList();
		QVariantList amountsVAT = aParameters.value("[AMOUNT_VAT]").toList();
		QVariantList operatorINNs = aParameters.value("[OPERATOR_INN]").toList();

		// amount содержит список сумм для печати реестра нераспечатанных чеков
		for (int i = 0; i < amounts.size(); i++)
		{
			unitDataList << DSDK::SUnitData(
				amounts.value(i).toDouble(), 
				amountsVAT.value(i).toInt(), 
				amountTitles.value(i).toString(), 
				operatorINNs.value(i).toString(), 
				DSDK::EPayOffSubjectTypes::Payment);
		}
	}

	bool combineFeeWithZeroVAT = aParameters.value("COMBINE_FEE_WITH_ZERO_VAT", false).toBool();
	QString feeName = combineFeeWithZeroVAT ? tr("#dealer_bpa_fee") : tr("#dealer_fee");

	if (dealerVAT == 0 && combineFeeWithZeroVAT)
	{
		fee = aParameters.value("FEE").toDouble();
		if (!qFuzzyIsNull(fee))
		{
			QString dealerINN = aParameters.value(CPrintConstants::DealerInn).toString();
			unitDataList << DSDK::SUnitData(fee, dealerVAT, feeName, dealerINN, DSDK::EPayOffSubjectTypes::AgentFee);
		}
	}
	else
	{
		if (!qFuzzyIsNull(fee))
		{
			QString dealerINN = aParameters.value(CPrintConstants::DealerInn).toString();
			unitDataList << (dealerIsBank ?
				DSDK::SUnitData(fee, dealerVAT, tr("#bank_fee"), dealerINN, DSDK::EPayOffSubjectTypes::Payment) :
				DSDK::SUnitData(fee, dealerVAT, feeName, dealerINN, DSDK::EPayOffSubjectTypes::AgentFee));
		}

		if (!qFuzzyIsNull(processingFee))
		{
			QString bankINN = aParameters.value(CPrintConstants::BankInn).toString();
			unitDataList << DSDK::SUnitData(processingFee, 0, tr("#processing_fee"), bankINN, DSDK::EPayOffSubjectTypes::Payment);
		}
	}

	bool EMoney = aParameters.value(PPSDK::CPayment::Parameters::PayTool).toInt() > 0;
	auto payType = EMoney ? DSDK::EPayTypes::EMoney : DSDK::EPayTypes::Cash;
	auto taxSystem = aParameters.contains(CPrintConstants::DealerTaxSystem) ? static_cast<DSDK::ETaxSystems::Enum>(aParameters.value(CPrintConstants::DealerTaxSystem).toInt()) : DSDK::ETaxSystems::None;
	auto agentFlag = aParameters.contains(CPrintConstants::DealerAgentFlag) ? static_cast<DSDK::EAgentFlags::Enum>(aParameters.value(CPrintConstants::DealerAgentFlag).toInt()) : DSDK::EAgentFlags::None;

	DSDK::SPaymentData result(unitDataList, DSDK::EPayOffTypes::Debit, payType, taxSystem, agentFlag);

	result.fiscalParameters[CHardwareSDK::FR::UserPhone] = QString();
	result.fiscalParameters[CHardwareSDK::FR::UserMail] = QString();

	foreach (auto FFData, CPrintCommands::FFDataList)
	{
		result.fiscalParameters[FFData] = aParameters[FFData];
	}

#if 0
	//TODO - решить как на верхнем уровне в интерфейсе мы будем давать возможность 
	//       вводить телефон/email для отправки чека ПЕРЕД печатью этого чека.
	//       Отключено по жалобе дилеров 1 sms = 2руб в счете от ОФД
	//       #60325 
	QVariantMap upperKeyParameters = toUpperCaseKeys(aParameters);
	QRegExp phoneRegexp("^9\\d{9}$");

	foreach (auto fieldName, QStringList() << "100" << "PHONE" << "CONTACT" << "101" << "102" << "103" << "104")
	{
		if (upperKeyParameters.contains(fieldName) && phoneRegexp.exactMatch(upperKeyParameters.value(fieldName).toString()))
		{
			result.fiscalParameters[CHardwareSDK::FR::UserPhone] = "7" + upperKeyParameters.value(fieldName).toString();
			break;
		}
	}

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
#endif

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

	QStringList receipt = mService->getReceipt(mReceiptTemplate, actualParameters);

	QVariantMap parameters = aParameters;
	QStringList fiscalPart;
	bool hasFiscalInfo = getFiscalInfo(parameters, fiscalPart);
	if (hasFiscalInfo)
	{
		receipt.append(fiscalPart);
	}

	bool result = false;

	// Повторно мы печатаем только нефискальные чеки
	if (!fiscalPrinting)
	{
		// Если есть фискальный сервис, но нет фискальной информации и запрещено печатать чеки без ФП
		if (!mService->enableBlankFiscalData() && mService->getFiscalRegister() && !hasFiscalInfo)
		{
			mService->toLog(LogLevel::Error, QString("[%1] Not print receipt with blank fiscal data.")
				.arg(aParameters[PPSDK::CPayment::Parameters::ID].toLongLong()));
		}
		else
		{
			result = canPrint(aPrinter, false) && aPrinter->print(receipt);
		}
	}
	else if (canFiscalPrint(aPrinter, false))
	{
		DSDK::SPaymentData paymentData = getPaymentData(actualParameters);
		DSDK::IFiscalPrinter * fiscalPrinter = static_cast<DSDK::IFiscalPrinter *>(aPrinter);

		quint32 FDNumber = 0;
		result = fiscalPrinter->printFiscal(receipt, paymentData, &FDNumber);

		if (result)
		{
			if (mFiscalFieldData.isEmpty())
			{
				mFiscalFieldData = aPrinter->getDeviceConfiguration().value(CHardwareSDK::FR::FiscalFieldData).value<DSDK::TFiscalFieldData>();
			}

			DSDK::TFiscalPaymentData fiscalPaymentData;
			DSDK::TComplexFiscalPaymentData payOffSubjectData;
			fiscalPrinter->checkFiscalFields(FDNumber, fiscalPaymentData, payOffSubjectData);

			addFiscalPaymentData(fiscalPaymentData, receipt);

			for (int i = 0; i < payOffSubjectData.size(); ++i)
			{
				addFiscalPaymentData(payOffSubjectData[i], receipt);
			}
		}
	}

	// Сохраняем чек на диске.
	QString receiptName = QTime::currentTime().toString(CPrintCommands::ReceiptNameTemplate) + "_" + aParameters["ID"].toString() +
		(result ? "" : CPrintCommands::NotPrintedPostfix);
	mService->saveReceiptContent(receiptName, receipt);

	return result;
}

//---------------------------------------------------------------------------
void PrintPayment::addFiscalPaymentData(const DSDK::TFiscalPaymentData & aFPData, QStringList & aData)
{
	for (auto it = aFPData.begin(); it != aFPData.end(); ++it)
	{
		DSDK::SFiscalFieldData FFData = mFiscalFieldData.value(it.key());

		QString key = FFData.translationPF;
		QString value = FFData.isMoney ? QString("%1").arg(it->toInt() / 100.0, 0, 'f', 2) : it->toString();
		QString text;

		     if (!key.isEmpty() && !value.isEmpty()) text = key + " = " + value;
		else if (!key.isEmpty())                     text = key;
		else if (!value.isEmpty())                   text = value;

		if (!text.isEmpty())
		{
			aData << text.simplified();
		}
	}
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
	if (aPrinter == nullptr)
	{
		return false;
	}

	QVariantMap configuration = aPrinter->getDeviceConfiguration();
	QString KKMSerialNumber = configuration[CHardwareSDK::SerialNumber].toString();

	if (KKMSerialNumber.isEmpty())
	{
		KKMSerialNumber = "0";
	}

	QVariantMap actualParameters = aParameters;
	actualParameters.insert(CPrintConstants::KKM::SerialNumber, KKMSerialNumber);
	QStringList receipt = mService->getReceipt(mReceiptTemplate, actualParameters);

	QString receiptFileName = QString("%1_%2").arg(QTime::currentTime().toString("hhmmsszzz")).arg(mReceiptTemplate);

	if (aParameters.contains(SDK::PaymentProcessor::CPayment::Parameters::ID))
	{
		receiptFileName += QString("_%1").arg(aParameters[SDK::PaymentProcessor::CPayment::Parameters::ID].toLongLong());
	}

	mService->saveReceiptContent(receiptFileName, receipt);

	return aPrinter->print(receipt);
}

//---------------------------------------------------------------------------
