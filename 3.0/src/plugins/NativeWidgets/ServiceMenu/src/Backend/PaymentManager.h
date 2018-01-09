/* @file Менеджер для работы с сплатежами */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Payment/Step.h>
#include <SDK/PaymentProcessor/Core/Encashment.h>
#include <SDK/PaymentProcessor/Settings/DealerSettings.h>
#include <SDK/PaymentProcessor/Payment/IPayment.h>
#include <SDK/PaymentProcessor/Core/IPaymentService.h>
#include <SDK/Drivers/FR/FiscalDataTypes.h>

#include "../GUI/PaymentInfo.h"

namespace SDK { 
namespace PaymentProcessor {
	class ICore;
	class IPrinterService;
}}

//---------------------------------------------------------------------------
class PaymentManager : public QObject
{
	Q_OBJECT

public:
	PaymentManager(SDK::PaymentProcessor::ICore * aCore);
	~PaymentManager();

public:
	/// Указать менеджеру, что используется аппаратный ФР
	void useHardwareFiscalPrinter(bool aUseFiscalPrinter);

public:
	/// Возвращает текущий баланс, сконвертированную в QVariantMap
	QVariantMap getBalanceInfo() const;

	/// Возвращает SEncashment из истории, сконвертированный в QVariantMap
	QVariantMap getEncashmentInfo(int aIndex) const;

	/// Получить кол-во инкассаций в истории
	int getEncashmentsHistoryCount();

	/// Провести инкассацию
	SDK::PaymentProcessor::EncashmentResult::Enum perform(const QVariantMap & aParameters);

	/// Указанный тип печати доступен?
	bool canPrint(const QString & aReceiptType) const;

	/// Ставит задание печати в очередь и возвращает jobIndex
	bool printReceipt(qint64 aPaymentId, bool aContinuousMode);

	bool printTestPage();
	bool printEncashment(int aIndex = -1);
	bool printBalance() const;
	bool printZReport(bool aFullZReport);

	/// Получить информацию о платежах
	bool getPaymentsInfo(QVariantMap & aPaymentsInfo) const;

	/// Получить список платежей
	QList<PaymentInfo> getPayments(bool aNeedUpdate = true);

	/// Получить информацию по одному платежу
	PaymentInfo getPayment(qint64 id);

	QString translatePaymentStatus(SDK::PaymentProcessor::EPaymentStatus::Enum aStatus) const;

	void processPayment(qint64 id);

signals:
	/// Срабатывает после печати чека платежа. Успешность операции передаётся в поле aError.
	void receiptPrinted(qint64 aPaymentId, bool aErrorHappened);

	/// Срабатывает при изменении состояния конкретного платежа
	void paymentChanged(qint64 aPaymentId);

private slots:
	/// Срабатывает после печати произвольного чека. Успешность операции передаётся в поле aError.
	void onReceiptPrinted(int aJobIndex, bool aErrorHappened);

private:
	/// Обновить список платежей
	void updatePaymentList();

	/// Печать суммарного чека о всех не напечатанных платежах
	bool printUnprintedReceiptsRegistry(const QSet<qint64> & aPayments);

	/// Преобразовать список параметров в структуру PaymentInfo
	PaymentInfo loadPayment(const QList<SDK::PaymentProcessor::IPayment::SParameter> & aPaymentParams);

	/// Конвертер баланса в параметры
	QVariantMap balanceParameters(const SDK::PaymentProcessor::SBalance & aBalance) const;

	/// Расшифровать значение параметра
	QString decryptParameter(const QString & aValue);

private:
	QList<PaymentInfo> mPaymentList;
	SDK::PaymentProcessor::ICore * mCore;
	SDK::PaymentProcessor::IPaymentService * mPaymentService;
	SDK::PaymentProcessor::IPrinterService * mPrinterService;
	SDK::PaymentProcessor::SEncashment mEncashment;
	QList<SDK::PaymentProcessor::SEncashment> mEncashmentList;
	SDK::PaymentProcessor::DealerSettings * mDealerSettings;

	/// Список заданий в очереди печати
	QMap<int, qint64> mPaymentPrintJobs;

	/// Задание печати реестра нераспечатанных платежей
	int mPaymentsRegistryPrintJob;

	/// Флаг, выставляемый при необходимости печати Z-отчета если имеются не напечатанные чеки
	bool mUseFiscalPrinter;
	QString mNeedPrintZReport;
};

//------------------------------------------------------------------------
