/* @file Базовый ФР с портовой реализацией протокола. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTextCodec>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Printers/SerialPrinterBase.h"
#include "Hardware/FR/ProtoFR.h"

// Project
#include "Hardware/FR/FRBase.h"

//--------------------------------------------------------------------------------
template <class T>
class PortFRBase : public FRBase<T>
{
public:
	PortFRBase();

	/// Печать фискального чека.
	virtual bool printFiscal(const QStringList & aReceipt, const SDK::Driver::SPaymentData & aPaymentData, SDK::Driver::TFiscalPaymentData & aFPData, SDK::Driver::TComplexFiscalPaymentData & aPSData);

	/// Выполнить Z-отчет [и распечатать отложенные Z-отчеты].
	virtual bool printZReport(bool aPrintDeferredReports);

	/// Выполнить X-отчет [и распечатать нефискальный чек - баланс].
	virtual bool printXReport(const QStringList & aReceipt);

protected:
	/// Идентификация.	
	virtual bool checkExistence();

	/// Установить начальные параметры.
	virtual void setInitialData();

	/// Сформировать массив байтов для печаит из массива строк.
	typedef QList<QByteArray> TReceiptBuffer;
	void makeReceipt(const QStringList & aReceipt, QStringList & aBuffer);
	void makeReceipt(const QStringList & aReceipt, TReceiptBuffer & aBuffer);

	/// Буфер обрабатываемых ошибок.
	class ErrorBuffer : public QList<char>
	{
	public:
		void removeLast() { if (!isEmpty()) QList::removeLast(); }
		void   pop_back() { if (!isEmpty()) QList::pop_back();   }
	};

	ErrorBuffer mProcessingErrors;

	/// Ошибка на последний запрос.
	char mLastError;

	/// Результат последней выполненной протокольной команды.
	TResult mLastCommandResult;
};

typedef PortFRBase<SerialPrinterBase<PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoFR>>>>> TSerialFRBase;
typedef PortFRBase<PortPrinterBase<PrinterBase<TCPDeviceBase<PortPollingDeviceBase<ProtoFR>>>>> TTCPFRBase;

//--------------------------------------------------------------------------------
