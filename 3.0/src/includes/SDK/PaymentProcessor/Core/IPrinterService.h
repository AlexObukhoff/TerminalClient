/* @file Интерфейс обеспечивающий взаимодействие с системой печати. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/WarningLevel.h>

namespace SDK {
namespace Driver { class IPrinter; }
namespace PaymentProcessor {

//------------------------------------------------------------------------------
class IPrinterService : public QObject
{
	Q_OBJECT

public:
	/// Результат фискального отчета для следующей версии.
	typedef QPair<Driver::EWarningLevel::Enum, QString> TReportResult;

	/// Проверка принтера на возможность печати чека.
	virtual bool canPrintReceipt(const QString & aReceiptType, bool aRealCheck) = 0;

	/// Печать типизированного чека с параметрами aParameters. Возвращает индекс задания, поставленного в очередь.
	/// Результат придёт в сигнале receiptPrinted.
	virtual int printReceipt(const QString & aReceiptType, const QVariantMap & aParameters, const QString & aReceiptTemplate, bool aContinuousMode, bool aServiceOperation = false) = 0;

	/// Сохранение электронной версии типизированного чека с параметрами aParameters.
	virtual void saveReceipt(const QVariantMap & aParameters, const QString & aReceiptTemplate) = 0;

	/// Загрузить и вернуть содержимое чека по номеру платежа
	virtual QString loadReceipt(qint64 aPaymentId) = 0;

	/// Печать тестового чека.
	virtual bool printReceiptDirected(SDK::Driver::IPrinter * aPrinter, const QString & aReceiptTemplate, const QVariantMap & aParameters) = 0;

	/// Печать отчета.
	virtual int printReport(const QString & aReceiptType, const QVariantMap & aParameters) = 0;

signals:
	/// Срабатывает после печати произвольного чека. Успешность операции передаётся в поле aError.
	void receiptPrinted(int aJobIndex, bool aErrorHappened);

protected:
	virtual ~IPrinterService() {}
};

//------------------------------------------------------------------------------
}} // SDK::PaymentProcessor

