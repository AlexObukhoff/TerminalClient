/* @file Прокси класс для работы с принтерами. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <QtCore/QFutureSynchronizer>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace PaymentProcessor {

class ICore;
class IPrinterService;
class IPaymentService;

namespace Scripting {

//------------------------------------------------------------------------------
class PrinterService : public QObject
{
	Q_OBJECT

public:
	PrinterService(ICore * aCore);

public slots:
	/// Проверка принтера на возможность печати чека. Если aRealCheck - true, то результат придёт в сигнале printerChecked.
	bool checkPrinter(bool aRealCheck);

	/// Печать типизированного чека с параметрами aParameters.
	void printReceipt(const QString & aReceiptType, const QVariantMap & aParameters, const QString & aTemplate, bool aContinuousMode = false);

	/// Сохранить на диск чек
	void saveReceipt(const QVariantMap & aParameters, const QString & aTemplate);

	QString loadReceipt(qint64 aPaymentId = -1);

	/// Проверить, указан ли урл, по которому отправлять данные для копии чека
	bool checkReceiptMail();

private slots:
	/// Обработчик сигнала печати от сервиса печати ядра
	void onReceiptPrinted(int aJobIndex, bool aError);

signals:
	/// Срабатывает после печати произвольного чека. Успешность операции передаётся в поле aError.
	void receiptPrinted(bool aError);

	/// Срабатывает после проверки принтеров.
	void printerChecked(bool aReady);

protected:
	void privateCheckPrinter();

private:
	ICore * mCore;
	IPrinterService * mPrinterService;
	IPaymentService * mPaymentService;
	typedef QPair<qint64, QString> TJobInfo;
	QMap<int, TJobInfo> mPrintedJobs;
	QFutureSynchronizer<void> mCheckSynchronizer;
};

//------------------------------------------------------------------------------
}}} // Scripting::PaymentProcessor::SDK
