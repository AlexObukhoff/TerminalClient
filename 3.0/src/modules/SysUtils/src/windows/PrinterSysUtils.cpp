/* @file Реализация интерфейса SysUtils. */

// windows
#include <windows.h>

// Модули
#include "Hardware/Common/DeviceDataConstants.h"
#include "SysUtils/ISysUtils.h"

struct SPrinterData
{
	HANDLE printer;
	QByteArray info;
	bool allowInfoError;

	PRINTER_INFO_2 * getInfo() { return reinterpret_cast<PRINTER_INFO_2 *>(info.data()); }

	SPrinterData(bool aAllowInfoError = false): printer(0), info(nullptr), allowInfoError(aAllowInfoError) {}
};

//---------------------------------------------------------------------------
void ISysUtils::makeLog(ILog * aLog, const QString & aFunctionName)
{
	if (aLog)
	{
		aLog->write(LogLevel::Error, QString("Failed to perform %1 due to %2").arg(aFunctionName).arg(getLastErrorMessage()));
	}
}

//---------------------------------------------------------------------------
bool ISysUtils::makeError(ILog * aLog, const QString & aFunctionName, void * aPrinter)
{
	makeLog(aLog, aFunctionName);
	
	if (aPrinter)
	{
		ClosePrinter(aPrinter);
	}

	return false;
}

//---------------------------------------------------------------------------
bool ISysUtils::getPrinterInfo(ILog * aLog, const QString & aPrinterName, SPrinterData & aPrinterData)
{
	DEVMODE devMode = { 0 };
	PRINTER_DEFAULTS defaults = { 0, &devMode, PRINTER_ACCESS_USE };

	if (!OpenPrinter((LPWSTR)aPrinterName.toStdWString().data(), &aPrinterData.printer, &defaults))
	{
		return makeError(aLog, "OpenPrinter");
	}

	DWORD byteNeeded, byteUsed;

	if (!GetPrinter(aPrinterData.printer, 2, NULL, 0, &byteNeeded) && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		return makeError(aLog, "GetPrinter", aPrinterData.printer);
	}

	aPrinterData.info.clear();
	aPrinterData.info.fill(0, byteNeeded);
	PRINTER_INFO_2 * info = reinterpret_cast<PRINTER_INFO_2 *>(aPrinterData.info.data());

	if (!GetPrinter(aPrinterData.printer, 2, (LPBYTE)info, byteNeeded, &byteUsed) && !aPrinterData.allowInfoError)
	{
		return makeError(aLog, "GetPrinter with printer info", aPrinterData.printer);
	}

	return true;
}

//---------------------------------------------------------------------------
bool ISysUtils::getPrinterStatusData(ILog * aLog, const QString & aPrinterName, TJobStatus & aJobStatus, ulong & aStatus, ulong & aAttributes)
{
	SPrinterData printerData;

	if (!getPrinterInfo(aLog, aPrinterName, printerData))
	{
		return false;
	}

	aStatus = printerData.getInfo()->Status;
	aAttributes = printerData.getInfo()->Attributes;

	DWORD byteNeeded = 0;
	DWORD returned = 0;
	DWORD byteUsed = 0;

	if (!EnumJobs(printerData.printer, 0, printerData.getInfo()->cJobs, 2, NULL, 0, (LPDWORD)&byteNeeded, (LPDWORD)&returned) && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		return makeError(aLog, "EnumJobs", printerData.printer);
	}

	QByteArray jobsInfoBuffer;
	jobsInfoBuffer.reserve(byteNeeded);
	JOB_INFO_2 * jobStorage = reinterpret_cast<JOB_INFO_2 *>(jobsInfoBuffer.data());

	bool enumJobsOK = EnumJobs(printerData.printer, 0, printerData.getInfo()->cJobs, 2, (LPBYTE)jobStorage, byteNeeded, (LPDWORD)&byteUsed, (LPDWORD)&returned);

	if (enumJobsOK)
	{
		for (DWORD i = 0; i < returned; ++i)
		{
			aJobStatus.push_back(jobStorage[i].Status);
		}
	}

	ClosePrinter(printerData.printer);

	return enumJobsOK;
}

//---------------------------------------------------------------------------
QVariantMap ISysUtils::getPrinterData(ILog * aLog, const QString & aPrinterName)
{
	SPrinterData printerData;

	if (!getPrinterInfo(aLog, aPrinterName, printerData))
	{
		return QVariantMap();
	}

	QVariantMap result;

	result.insert(CDeviceData::Name,               QString::fromUtf16(printerData.getInfo()->pPrinterName));
	result.insert(CDeviceData::Port,               QString::fromUtf16(printerData.getInfo()->pPortName));
	result.insert(CDeviceData::Driver,             QString::fromUtf16(printerData.getInfo()->pDriverName));
	result.insert(CDeviceData::Printers::Location, QString::fromUtf16(printerData.getInfo()->pLocation));
	result.insert(CDeviceData::Printers::Comment,  QString::fromUtf16(printerData.getInfo()->pComment));
	result.insert(CDeviceData::Printers::Server,   QString::fromUtf16(printerData.getInfo()->pServerName));
	result.insert(CDeviceData::Printers::Share,    QString::fromUtf16(printerData.getInfo()->pShareName));

	ClosePrinter(printerData.printer);

	return result;
}

//--------------------------------------------------------------------------------
bool ISysUtils::setPrintingQueuedMode(ILog * aLog, const QString & aPrinterName)
{
	bool result = false;

	bool directPrinting = false;
	SPrinterData printerData(true);

	if (!getPrinterInfo(aLog, aPrinterName, printerData))
	{
		return false;
	}
	else if (printerData.getInfo())
	{
		directPrinting = (printerData.getInfo()->Attributes & PRINTER_ATTRIBUTE_DIRECT);

		if (printerData.getInfo()->Status & PRINTER_STATUS_PAUSED)
		{
			// запускаем печать, если печать на принтер была приостановлена
			result = SetPrinter(printerData.printer, 0, NULL, PRINTER_CONTROL_RESUME);
		}
	}

	ClosePrinter(printerData.printer);

	if (!directPrinting)
	{
		return true;
	}

	DEVMODE devMode = { 0 };
	PRINTER_DEFAULTS defaultsAdmin = { 0, &devMode, PRINTER_ACCESS_ADMINISTER };

	// Открываем с правами админа
	if (!OpenPrinter((LPWSTR)aPrinterName.toStdWString().data(), &printerData.printer, &defaultsAdmin))
	{
		return makeError(aLog, "OpenPrinter with admin rules");
	}

	// очищаем очередь заданий
	result = SetPrinter(printerData.printer, 0, NULL, PRINTER_CONTROL_PURGE);

	printerData.getInfo()->Attributes &= ~(DWORD)(PRINTER_ATTRIBUTE_DIRECT);
	printerData.getInfo()->Attributes |= PRINTER_ATTRIBUTE_QUEUED;

	// переключаем режим печати на печать через очередь
	result = SetPrinter(printerData.printer, 2, reinterpret_cast<LPBYTE>(printerData.getInfo()), 0);

	if (!result)
	{
		makeLog(aLog, "SetPrinter");
	}

	if (printerData.getInfo()->Status & PRINTER_STATUS_PAUSED)
	{
		// запускаем печать, если печать на принтер была приостановлена
		result = SetPrinter(printerData.printer, 0, NULL, PRINTER_CONTROL_RESUME);
	}

	ClosePrinter(printerData.printer);

	return result;
}

//--------------------------------------------------------------------------------
