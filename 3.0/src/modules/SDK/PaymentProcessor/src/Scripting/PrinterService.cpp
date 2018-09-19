/* @file Прокси класс для работы с принтерами. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtConcurrentRun>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IPrinterService.h>
#include <SDK/PaymentProcessor/Core/IPaymentService.h>
#include <SDK/PaymentProcessor/Scripting/PrinterService.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>

namespace CPrintingService
{
	const int SaveReceiptJobID = 1234567;
}

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------------
PrinterService::PrinterService(ICore * aCore) :
	mCore(aCore),
	mPrinterService(mCore->getPrinterService()),
	mPaymentService(mCore->getPaymentService())
{
	connect(mPrinterService, SIGNAL(receiptPrinted(int, bool)), SLOT(onReceiptPrinted(int, bool)));
	connect(mPrinterService, SIGNAL(printerStatus(bool)), SIGNAL(printerChecked(bool)));
}

//------------------------------------------------------------------------------
bool PrinterService::checkPrinter(bool aRealCheck)
{
	if (aRealCheck)
	{
		// эта проверка должна вызываться крайне редко, т.к. в случае не ответа принтера подвешивает интерфейс
		if (mCheckSynchronizer.futures().size() == 0 || mCheckSynchronizer.futures().last().isFinished())
			mCheckSynchronizer.addFuture(QtConcurrent::run(this, &PrinterService::privateCheckPrinter));

		return true;
	}
	else
	{
		return mPrinterService->canPrintReceipt("payment", false);
	}
}

//------------------------------------------------------------------------------
void PrinterService::privateCheckPrinter()
{
	emit printerChecked(mPrinterService->canPrintReceipt("payment", true));
}

//------------------------------------------------------------------------------
void PrinterService::printReceipt(const QString & aReceiptType, const QVariantMap & aParameters, const QString & aTemplate, bool aContinuousMode)
{
	qint64 paymentID = mPaymentService->getActivePayment();
		
	if (paymentID > 0 && !aParameters.contains(SDK::PaymentProcessor::CPayment::Parameters::ID))
	{
		QVariantMap aNewParameters(aParameters);
		aNewParameters.insert(SDK::PaymentProcessor::CPayment::Parameters::ID, paymentID);

		// Отправляем на печать с добавлением ID платежа
		mPrintedJobs.insert(
			mPrinterService->printReceipt(aReceiptType, aNewParameters, QString(aTemplate).replace(".xml", ""), aContinuousMode),
			TJobInfo(paymentID, aReceiptType));
	}
	else
	{
		// Отправляем на печать
		mPrintedJobs.insert(
			mPrinterService->printReceipt(aReceiptType, aParameters, QString(aTemplate).replace(".xml", ""), aContinuousMode),
			TJobInfo(paymentID, aReceiptType));
	}
}

//------------------------------------------------------------------------------
void PrinterService::saveReceipt(const QVariantMap & aParameters, const QString & aTemplate)
{
	mPrinterService->saveReceipt(aParameters, QString(aTemplate).replace(".xml", ""));
}

//------------------------------------------------------------------------------
QString PrinterService::loadReceipt(qint64 aPaymentId)
{
	return mPrinterService->loadReceipt(aPaymentId == -1 ? mPaymentService->getActivePayment() : aPaymentId);
}

//------------------------------------------------------------------------------
bool PrinterService::checkReceiptMail()
{
	return !(static_cast<PPSDK::TerminalSettings *>(mCore->getSettingsService()->
		getAdapter(PPSDK::CAdapterNames::TerminalAdapter))->getReceiptMailURL().isEmpty());
}

//------------------------------------------------------------------------------
void PrinterService::onReceiptPrinted(int aJobIndex, bool aError)
{
	if (mPrintedJobs.contains(aJobIndex))
	{
		emit receiptPrinted(aError);
		
		TJobInfo job = mPrintedJobs.value(aJobIndex);

		// В случае успеха отмечаем платеж как напечатанный
		if (!aError && job.first != 0 && !job.second.isEmpty())
		{
			mPaymentService->updatePaymentField(job.first, IPayment::SParameter(SDK::PaymentProcessor::CPayment::Parameters::ReceiptPrinted, true, true), true);
		}

		mPrintedJobs.remove(aJobIndex);
	}
}

//------------------------------------------------------------------------------
}}} // Scripting::PaymentProcessor::SDK
