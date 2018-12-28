/* @file Окно инкассации. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSettings>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ReceiptTypes.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/UserSettings.h>
#include <SDK/PaymentProcessor/Core/ServiceParameters.h>
#include <SDK/PaymentProcessor/Core/ITerminalService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/PaymentProcessor/Core/IFundsService.h>

//
#include <SysUtils/ISysUtils.h>

// Проект
#include "MessageBox/MessageBox.h"
#include "Backend/PaymentManager.h"
#include "Backend/HardwareManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "ServiceTags.h"
#include "EncashmentServiceWindow.h"
#include "EncashmentHistoryWindow.h"
#include "InputBox.h"

namespace
{
	const QString PayloadSettings = "payload.ini";
}

namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------
EncashmentServiceWindow::EncashmentServiceWindow(ServiceMenuBackend * aBackend, QWidget * aParent)
	: EncashmentWindow(aBackend, aParent),
	mBackend(aBackend)
{
	ui.setupUi(this);

	// TODO Заполнять значениями
	ui.twNotes->hide();

	ui.btnPayload->setVisible(false);

	///Найдем файл настроек для диспенсеров
	PPSDK::TerminalSettings * s = static_cast<PPSDK::TerminalSettings *>(mBackend->getCore()->getSettingsService()->
		getAdapter(SDK::PaymentProcessor::CAdapterNames::TerminalAdapter));
	
	mPayloadSettingsPath = QString("%1/%2").arg(s->getAppEnvironment().userDataPath).arg(QString("%1_%2").arg(s->getKeys().value(0).ap).arg(PayloadSettings));
	
	if (QFile::exists(mPayloadSettingsPath))
	{
		QSettings settings(ISysUtils::rmBOM(mPayloadSettingsPath), QSettings::IniFormat);

		foreach(QString deviceGuid, settings.childGroups())
		{
			mPayloadSettings.insert(deviceGuid, settings.value(QString("%1/%2").arg(deviceGuid).arg("payload")));
		}

		/// Кнопка активна, если есть что загрузить в диспенсеры
		ui.btnPayload->setVisible(!mPayloadSettings.isEmpty());
		connect(ui.btnPayload, SIGNAL(clicked()), this, SLOT(doPayload()));
	}
		
	connect(ui.btnEncash, SIGNAL(clicked()), this, SLOT(doEncashment()));
	connect(ui.btnPrintBalance, SIGNAL(clicked()), this, SLOT(onPrintBalance()));
	connect(ui.btnPrintZReport, SIGNAL(clicked()), this, SLOT(onPrintZReport()));

	mHistoryWindow = new EncashmentHistoryWindow(aBackend, this);
	ui.gridLayoutEncashment->addWidget(mHistoryWindow, 2, 0);
}

//------------------------------------------------------------------------
bool EncashmentServiceWindow::initialize()
{
	updateUI();

	return true;
}

//------------------------------------------------------------------------
bool EncashmentServiceWindow::shutdown()
{
	return true;
}

//----------------------------------------------------------------------------
void EncashmentServiceWindow::updateUI()
{
	// Указываем сервису платежей, есть ли у нас аппаратный ФР
	mBackend->getPaymentManager()->useHardwareFiscalPrinter(mBackend->getHardwareManager()->isFiscalPrinterPresent(false));

	// Настраиваем интерфейс в зависимости от типа принтера
	bool isFiscalPrinter = mBackend->getHardwareManager()->isFiscalPrinterPresent(true);

	ui.btnPrintZReport->setEnabled(isFiscalPrinter && mBackend->getPaymentManager()->canPrint(PPSDK::CReceiptType::ZReport));
	ui.btnPrintBalance->setEnabled(mBackend->getPaymentManager()->canPrint(PPSDK::CReceiptType::Balance));

	updateInfo();
}

//------------------------------------------------------------------------
bool EncashmentServiceWindow::activate()
{
	connect(mBackend->getHardwareManager(), SIGNAL(deviceStatusChanged(const QString &, const QString &, const QString &, SDK::Driver::EWarningLevel::Enum)), 
		this, SLOT(onDeviceStatusChanged(const QString &, const QString &, const QString &, SDK::Driver::EWarningLevel::Enum)));

	updateUI();

	mHistoryWindow->updateHistory();

	return true;
}

//------------------------------------------------------------------------
bool EncashmentServiceWindow::deactivate()
{
	disconnect(mBackend->getHardwareManager(), SIGNAL(deviceStatusChanged(const QString &, const QString &, const QString &, SDK::Driver::EWarningLevel::Enum)), 
		this, SLOT(onDeviceStatusChanged(const QString &, const QString &, const QString &, SDK::Driver::EWarningLevel::Enum)));

	return EncashmentWindow::deactivate();
}

//------------------------------------------------------------------------
void EncashmentServiceWindow::onDeviceStatusChanged(const QString & aConfigName, const QString & aStatusString, const QString & aStatusColor, SDK::Driver::EWarningLevel::Enum aLevel)
{
	Q_UNUSED(aConfigName);
	Q_UNUSED(aStatusString);
	Q_UNUSED(aStatusColor);
	Q_UNUSED(aLevel);

	updateUI();
}

//------------------------------------------------------------------------
void EncashmentServiceWindow::updateInfo()
{
	QVariantMap cashInfo = mBackend->getPaymentManager()->getBalanceInfo();

	int noteCount = cashInfo[CServiceTags::NoteCount].toInt();
	int coinCount = cashInfo[CServiceTags::CoinCount].toInt();

	ui.lbLastEncashmentDate->setText(cashInfo[CServiceTags::LastEncashmentDate].toDateTime().toString("yyyy.MM.dd hh:mm"));
	ui.lbCashAmount->setText(cashInfo[CServiceTags::CashAmount].toString());
	ui.lbNoteCount->setText(QString("%1").arg(noteCount));
	ui.lbCoinCount->setText(QString("%1").arg(coinCount));

	ui.btnEncash->setEnabled(true);

	mHistoryWindow->updateHistory();
}

//------------------------------------------------------------------------
void EncashmentServiceWindow::onPrintBalance()
{
	mMessageError = tr("#balance_print_failed");

	if (mBackend->getPaymentManager()->canPrint(PPSDK::CReceiptType::Balance))
	{
		mMessageSuccess = tr("#balance_printed");
		mBackend->getPaymentManager()->printBalance();
	}
	else
	{
		// TODO Дополнять статусом принтера
		GUI::MessageBox::critical(mMessageError);
	}
}

//---------------------------------------------------------------------------
void EncashmentServiceWindow::doPayload()
{
	PPSDK::TCashUnitsState cashUnitState = mBackend->getCore()->getFundsService()->getDispenser()->getCashUnitsState();

	bool unitUpdateOK = false;
	
	QStringList devices = cashUnitState.keys();
	foreach (QString device, devices)
	{
		PPSDK::TCashUnitList cashUnits = cashUnitState.value(device);

		if (cashUnits.isEmpty())
		{
			continue;
		}

		// номер кассеты:номинал:количество|номер кассеты:номинал:количество
		// обнулить все кассеты payload=0
		// обнулить первую кассету payload=1:0
		for(QString & unitPayload : mPayloadSettings.value(device.split("_").last()).toString().split("|"))
		{
			if (unitPayload.isEmpty())
			{
				continue;
			}
			
			QStringList payload = unitPayload.split(":");

			if (payload.count() == 1 && payload.first().toInt() == 0)
			{
				cashUnits.fill(PPSDK::SCashUnit());
			}
			else if(payload.count() == 2 && payload.first().toInt() < cashUnits.count() && payload.last().toInt() == 0)
			{
				cashUnits[payload.first().toInt()] = PPSDK::SCashUnit();
			}
			else if (payload.count() == 3)
			{
				PPSDK::SCashUnit & unit = cashUnits[payload.takeFirst().toInt()];
				unit.nominal = payload.takeFirst().toInt();
				unit.count = payload.takeFirst().toInt();
			}
			else
			{
				GUI::MessageBox::critical(tr("#check_update_payload_settings"));
			}
		}

		unitUpdateOK = mBackend->getCore()->getFundsService()->getDispenser()->setCashUnitsState(device, cashUnits);
	}

	// Все кассеты обновились успешно
	if (unitUpdateOK)
	{
		ui.btnPayload->setVisible(false);
		QFile::remove(mPayloadSettingsPath);
	}
}

//----------------------------------------------------------------------------
