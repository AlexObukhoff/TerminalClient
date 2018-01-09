/* @file Класс для тестирования принтеров. */

// SDK
#include <SDK/Drivers/IPrinter.h>
#include <SDK/Drivers/IFiscalPrinter.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/ReceiptTypes.h>
#include <SDK/PaymentProcessor/Core/IPrinterService.h>

// Project
#include "PrinterTest.h"

//------------------------------------------------------------------------------
namespace CPrinterTest
{
	const QString PrintTestReceipt = QT_TRANSLATE_NOOP("PrinterTest", "#print_test_receipt");
}

namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------------
PrinterTest::PrinterTest(SDK::Driver::IDevice * aDevice, SDK::PaymentProcessor::ICore * aCore) :
	mPrinter(dynamic_cast<SDK::Driver::IPrinter *>(aDevice)),
	mCore(aCore)
{
}

//------------------------------------------------------------------------------
QList<QPair<QString, QString>> PrinterTest::getTestNames() const
{
	return QList<QPair<QString, QString>>() << qMakePair(CPrinterTest::PrintTestReceipt, QString());
}

//------------------------------------------------------------------------------
bool PrinterTest::run(const QString & aName)
{
	if (aName == CPrinterTest::PrintTestReceipt)
	{
		mTestResult = QtConcurrent::run(mCore->getPrinterService(),
			static_cast<bool (PPSDK::IPrinterService::*)(SDK::Driver::IPrinter *, const QString &, const QVariantMap &)>(&PPSDK::IPrinterService::printReceiptDirected),
				 mPrinter.data(), QString(PPSDK::CReceiptType::Test), QVariantMap());
	}

	return true;
}

//------------------------------------------------------------------------------
void PrinterTest::stop()
{
	mTestResult.waitForFinished();
}

//------------------------------------------------------------------------------
bool PrinterTest::isReady()
{
	if (mPrinter)
	{
		auto fr = dynamic_cast<SDK::Driver::IFiscalPrinter *>(mPrinter.data());

		return fr ? fr->isFiscalReady(false, SDK::Driver::EFiscalPrinterCommand::Print) : mPrinter->isDeviceReady(true);
	}

	return false;
}

//------------------------------------------------------------------------------
bool PrinterTest::hasResult()
{
	return false;
}

//------------------------------------------------------------------------------
void PrinterTest::onPrinted(bool aError)
{
	emit result(CPrinterTest::PrintTestReceipt, aError ? tr("#failed") : tr("#ok"));
}

//------------------------------------------------------------------------------