/* @file Базовый ФР с портовой реализацией протокола. */

#pragma once

// Modules
#include "Hardware/Printers/PortPrintersBase.h"
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
	virtual bool printFiscal(const QStringList & aReceipt, const SDK::Driver::SPaymentData & aPaymentData, quint32 * aFDNumber = nullptr);

	/// Выполнить Z-отчет [и распечатать отложенные Z-отчеты].
	virtual bool printZReport(bool aPrintDeferredReports);

	/// Выполнить X-отчет [и распечатать нефискальный чек - баланс].
	virtual bool printXReport(const QStringList & aReceipt);

protected:
	/// Установить начальные параметры.
	virtual void setInitialData();

	/// Идентификация.	
	virtual bool checkExistence();

	/// Получить статус. Возвращает Fail, Error (константы) или правильный ответ.
	template <class T2>
	QByteArray performStatus(TStatusCodes & aStatusCodes, T2 aCommand, int aIndex = -1);

	/// Сформировать массив байтов для печаит из массива строк.
	typedef QList<QByteArray> TReceiptBuffer;
	void makeReceipt(const QStringList & aReceipt, QStringList & aBuffer);
	void makeReceipt(const QStringList & aReceipt, TReceiptBuffer & aBuffer);

	typedef std::function<TResult(QByteArray & aData)> TGetFiscalTLVData;
	typedef std::function<bool(const CFR::STLV & aTLV)> TProcessTLVAction;
	bool processFiscalTLVData(const TGetFiscalTLVData & aGetFiscalTLVData, SDK::Driver::TFiscalPaymentData * aFPData, SDK::Driver::TComplexFiscalPaymentData * aPSData);
	bool processTLVData(const TGetFiscalTLVData & aGetFiscalTLVData, TProcessTLVAction aAction = TProcessTLVAction());

	/// Загрузить названия отделов.
	typedef std::function<bool(int aIndex, QByteArray & aValue)> TLoadSectionName;
	bool loadSectionNames(const TLoadSectionName & aLoadSectionName);

	/// Является ли ошибка необрабатываемой?
	bool isErrorUnprocessed(char aCommand, char aError);
	virtual bool isErrorUnprocessed(const QByteArray & aCommand, char aError);

	/// Буфер обрабатываемых ошибок.
	class ErrorBuffer : public QList<char>
	{
	public:
		void removeLast() { if (!isEmpty()) QList::removeLast(); }
		void   pop_back() { if (!isEmpty()) QList::pop_back();   }
	};

	ErrorBuffer mProcessingErrors;

	/// Команда последнего запроса.
	QByteArray mLastCommand;

	/// Ошибка на последний запрос.
	char mLastError;

	/// Результат последней выполненной протокольной команды.
	TResult mLastCommandResult;

	/// Данные ошибок.
	typedef QSharedPointer<FRError::Data> PErrorData;
	PErrorData mErrorData;

	/// Данные необрабатываемых ошибок.
	typedef QSet<char> TErrors;

	class UnprocessedErrorData : public CSpecification<QByteArray, TErrors>
	{
	public:
		void add(char aCommand, char aError) { data()[QByteArray(1, aCommand)].insert(aError); }
		void add(const QByteArray & aCommand, char aError) { data()[aCommand].insert(aError); }
		void add(char aCommand, const TErrors & aErrors) { append(QByteArray(1, aCommand), aErrors); }
		void add(const QByteArray & aCommand, const TErrors & aErrors) { append(aCommand, aErrors); }
	};

	UnprocessedErrorData mUnprocessedErrorData;
};

typedef PortFRBase<SerialPrinterBase<PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoFR>>>>> TSerialFRBase;
typedef PortFRBase<PortPrinterBase<PrinterBase<TCPDeviceBase<PortPollingDeviceBase<ProtoFR>>>>> TTCPFRBase;

//--------------------------------------------------------------------------------
