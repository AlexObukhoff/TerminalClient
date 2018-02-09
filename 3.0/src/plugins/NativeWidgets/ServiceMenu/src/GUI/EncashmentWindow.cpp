/* @file Базовый виджет для инкасации */

#include <QtCore/QDebug>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>

// SDK
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/ReceiptTypes.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/UserSettings.h>
#include <SDK/PaymentProcessor/Core/ServiceParameters.h>

// Project
#include "Backend/MessageBox.h"
#include "Backend/PaymentManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "ServiceTags.h"
#include "InputBox.h"
#include "EncashmentWindow.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
template<class T>
void safeDelete(T * & aPointer)
{
	if (aPointer)
	{
		aPointer->deleteLater();
		aPointer = nullptr;
	}
}

//---------------------------------------------------------------------------
EncashmentWindow::EncashmentWindow(ServiceMenuBackend * aBackend, QWidget * aParent) :
	QWidget(aParent),
	ServiceWindowBase(aBackend),
	mEncashmentWithZReport(false),
	mInputBox(nullptr),
	mHistoryWindow(nullptr)
{
}

//---------------------------------------------------------------------------
EncashmentWindow::~EncashmentWindow()
{
}

//---------------------------------------------------------------------------
void EncashmentWindow::doEncashment()
{
	auto paymentManager = mBackend->getPaymentManager();
	bool isPrinterOK = paymentManager->canPrint(PPSDK::CReceiptType::Encashment);
	QString text = isPrinterOK ? tr("#question_encash") : tr("#question_encash_without_receipt");

	safeDelete(mInputBox);

	if (MessageBox::question(text))
	{
		// Если баланс не пустой и нужно ввести номер кассеты
		if (paymentManager->getBalanceInfo()[CServiceTags::CashAmount].toDouble() > 0.0 &&
			dynamic_cast<PPSDK::UserSettings *>(mBackend->getCore()->getSettingsService()->getAdapter(PPSDK::CAdapterNames::UserAdapter))->useStackerID())
		{
			InputBox::ValidatorFunction validator = [](const QString & aText) -> bool { return !aText.trimmed().isEmpty(); };
			mInputBox = new InputBox(this, validator);
			mInputBox->setLabelText(tr("#enter_stacker_id"));

			connect(mInputBox, SIGNAL(accepted()), this, SLOT(doEncashmentProcess()));

			mInputBox->show();
		}
		else
		{
			doEncashmentProcess();
		}
	}
}

//---------------------------------------------------------------------------
bool EncashmentWindow::doEncashmentProcess()
{
	bool result = false;
	auto paymentManager = mBackend->getPaymentManager();
	bool printerOK = paymentManager->canPrint(PPSDK::CReceiptType::Encashment);

	mIdleTimer.stop();

	QVariantMap parameters;

	if (mInputBox)
	{
		parameters[PPSDK::EncashmentParameter::StackerID] = mInputBox->textValue().trimmed();
		safeDelete(mInputBox);
	}

	switch (paymentManager->perform(parameters))
	{
	case PPSDK::EncashmentResult::OK:
		result = true;

		MessageBox::info(tr("#encashment_complete"));
		
		if (!printerOK)
		{
			mMessageError = tr("#encashment_print_failed");
		}
		else
		{
			mMessageSuccess = tr("#encashment_complete_and_printed");
			MessageBox::wait(tr("#printing"));
		}

		// Даже если принтер недоступен сохраним электронную копию чека инкассации
		paymentManager->printEncashment();

		// Сбросим счетчики отбракованных купюр/монет
		mBackend->getCore()->getService("FundsService")->resetParameters(QSet<QString>() << PPSDK::CServiceParameters::Funds::RejectCount);
		break;

	case PPSDK::EncashmentResult::TryLater:
		MessageBox::critical(tr("#encashment_error_try_later"));
		break;

	default:
		MessageBox::critical(tr("#encashment_error"));
		break;
	}

	updateUI();

	return result;
}

//---------------------------------------------------------------------------
void EncashmentWindow::onPrintZReport()
{
	QPushButton * zReportButton = dynamic_cast<QPushButton *>(sender());

	MessageBox::hide();

	mMessageError = tr("#zreport_failed");
	if (mBackend->getPaymentManager()->canPrint(PPSDK::CReceiptType::ZReport))
	{
		bool fullZReport = MessageBox::question(tr("#print_full_zreport"));

		mIdleTimer.stop();

		mMessageSuccess = tr("#zreport_printed");

		MessageBox::wait(tr("#printing"));
		if (!mBackend->getPaymentManager()->printZReport(fullZReport) && zReportButton)
		{
			zReportButton->setEnabled(false);
		}
	}
	else
	{
		// TODO Дополнять статусом принтера
		MessageBox::critical(mMessageError);
	}
}

//------------------------------------------------------------------------
void EncashmentWindow::onPeceiptPrinted(qint64 aPaymentId, bool aErrorHappened)
{
	Q_UNUSED(aPaymentId);

	if (!mMessageError.isEmpty() && aErrorHappened)
	{
		MessageBox::hide();
		MessageBox::critical(mMessageError);
	}
	else if (!mMessageSuccess.isEmpty() && !aErrorHappened)
	{
		MessageBox::hide();
		MessageBox::info(mMessageSuccess);
	}

	mMessageError.clear();
	mMessageSuccess.clear();

	if (mEncashmentWithZReport)
	{
		QTimer::singleShot(1000, this, SLOT(onPrintZReport()));

		mEncashmentWithZReport = false;
	}

	mIdleTimer.start();;
}

//---------------------------------------------------------------------------
bool EncashmentWindow::deactivate()
{
	safeDelete(mInputBox);

	return true;
}

//---------------------------------------------------------------------------

