/* @file Реализация интерфейса SysUtils. */

// windows
#include <windows.h>
#include <winspool.h>

// Модули
#include "Hardware/Common/DeviceDataConstants.h"
#include "SysUtils/ISysUtils.h"
#include "SystemPrinterStatusCodes.h"

//---------------------------------------------------------------------------
typedef QList<ulong> TJobsStatus;

QString getLastErrorMessage();

//---------------------------------------------------------------------------
bool getPrinterStatusData(const QString & aPrinterName, TJobsStatus & aJobsStatus, ulong & aStatus, ulong & aAttributes)
{
	DEVMODE devMode = { 0 };
	PRINTER_DEFAULTS defaults = { 0, &devMode, PRINTER_ACCESS_USE };

	HANDLE printer;

	if (!OpenPrinter((LPWSTR)aPrinterName.toStdWString().data(), &printer, &defaults))
	{
		return false;
	}

	DWORD byteNeeded, returned, byteUsed;
	if (!GetPrinter(printer, 2, NULL, 0, &byteNeeded) && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		ClosePrinter(printer);

		return false;
	}

	QByteArray printerInfoBuffer;
	printerInfoBuffer.fill(0, byteNeeded);
	PRINTER_INFO_2 * printerInfo = reinterpret_cast<PRINTER_INFO_2 *>(printerInfoBuffer.data());

	if (!GetPrinter(printer, 2, (LPBYTE)printerInfo, byteNeeded, &byteUsed))
	{
		ClosePrinter(printer);

		return false;
	}

	aStatus = printerInfo->Status;
	aAttributes = printerInfo->Attributes;

	if (!EnumJobs(printer, 0, printerInfo->cJobs, 2, NULL, 0, (LPDWORD)&byteNeeded, (LPDWORD)&returned) && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		ClosePrinter(printer);

		return false;
	}

	QByteArray jobsInfoBuffer;
	jobsInfoBuffer.reserve(byteNeeded);
	JOB_INFO_2 * jobStorage = reinterpret_cast<JOB_INFO_2 *>(jobsInfoBuffer.data());

	bool enumJobsOK = EnumJobs(printer, 0, printerInfo->cJobs, 2, (LPBYTE)jobStorage, byteNeeded, (LPDWORD)&byteUsed, (LPDWORD)&returned);

	if (enumJobsOK)
	{
		for (DWORD i = 0; i < returned; ++i)
		{
			aJobsStatus.push_back(jobStorage[i].Status);
		}
	}

	ClosePrinter(printer);

	return enumJobsOK;
}

//---------------------------------------------------------------------------
QVariantMap ISysUtils::getPrinterData(const QString & aPrinterName)
{
	DEVMODE devMode = { 0 };
	PRINTER_DEFAULTS defaults = { 0, &devMode, PRINTER_ACCESS_USE };

	HANDLE printer;
	QVariantMap result;

	if (!OpenPrinter((LPWSTR)aPrinterName.toStdWString().data(), &printer, &defaults))
	{
		return result;
	}

	DWORD byteNeeded, byteUsed;

	if (!GetPrinter(printer, 2, NULL, 0, &byteNeeded) && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		ClosePrinter(printer);

		return result;
	}

	QByteArray printerInfoBuffer;
	printerInfoBuffer.fill(0, byteNeeded);
	PRINTER_INFO_2 * printerInfo = reinterpret_cast<PRINTER_INFO_2 *>(printerInfoBuffer.data());

	if (!GetPrinter(printer, 2, (LPBYTE)printerInfo, byteNeeded, &byteUsed))
	{
		ClosePrinter(printer);

		return result;
	}

	result.insert(CDeviceData::Name,   QString::fromWCharArray(printerInfo->pPrinterName));
	result.insert(CDeviceData::Port,   QString::fromWCharArray(printerInfo->pPortName));
	result.insert(CDeviceData::Driver, QString::fromWCharArray(printerInfo->pDriverName));
	result.insert(CDeviceData::Printers::Location, QString::fromWCharArray(printerInfo->pLocation));
	result.insert(CDeviceData::Printers::Comment,  QString::fromWCharArray(printerInfo->pComment));
	result.insert(CDeviceData::Printers::Server,   QString::fromWCharArray(printerInfo->pServerName));
	result.insert(CDeviceData::Printers::Share,    QString::fromWCharArray(printerInfo->pShareName));

	ClosePrinter(printer);

	return result;
}

//---------------------------------------------------------------------------
void ISysUtils::getPrinterStatus(const QString & aPrinterName, TStatusCodes & aStatusCodes, TStatusGroupNames & aGroupNames)
{
	TJobsStatus jobs;
	ulong status;
	ulong attributes;
	QString errorMessage;

	if (!getPrinterStatusData(aPrinterName, jobs, status, attributes))
	{
		aStatusCodes.insert(DeviceStatusCode::Warning::ThirdPartyDriver);

		return;
	}

	for (int i = 0; i < sizeof(ulong) * 8; ++i)
	{
		ulong key = 1 << i;

		if (WindowsPrinterStatuses.data().contains(key) && (status & key))
		{
			const SDeviceCodeSpecification & data = WindowsPrinterStatuses[key];
			aStatusCodes.insert(data.statusCode);
			aGroupNames["Statuses"].insert(data.description);
		}

		foreach(auto jobStatus, jobs)
		{
			if (WindowsPrinterJobStatuses.data().contains(key) && (jobStatus & key))
			{
				const SDeviceCodeSpecification & data = WindowsPrinterJobStatuses[key];
				aStatusCodes.insert(data.statusCode);
				aGroupNames["Job statuses"].insert(data.description);
			}
		}
	}

	if (!ISysUtils::setPrinterQueuedMode(aPrinterName, errorMessage))
	{
		aStatusCodes.insert(DeviceStatusCode::Warning::ThirdPartyDriver);
	}

	if (attributes & PRINTER_STATUS_NOT_AVAILABLE ||
		attributes & PRINTER_ATTRIBUTE_WORK_OFFLINE ||
		status & PRINTER_STATUS_PAUSED)
	{
		aStatusCodes.insert(DeviceStatusCode::Error::NotAvailable);
		aGroupNames["Attributes"].insert("work_offline");
	}
}

//--------------------------------------------------------------------------------
bool ISysUtils::setPrinterQueuedMode(const QString & aPrinterName, QString & aErrorMessage)
{
	bool result = false;

	DEVMODE devMode = { 0 };
	PRINTER_DEFAULTS defaults = { 0, &devMode, PRINTER_ACCESS_USE };
	PRINTER_DEFAULTS defaultsAdmin = { 0, &devMode, PRINTER_ACCESS_ADMINISTER };

	bool directPrinting = false;
	PRINTER_INFO_2 * printerInfo = nullptr;
	QByteArray printerInfoBuffer;
	HANDLE printer;

	if (OpenPrinter((LPWSTR)aPrinterName.toStdWString().data(), &printer, &defaults))
	{
		DWORD byteNeeded = 0, byteUsed = 0;

		// Get the buffer size needed.
		if (!GetPrinter(printer, 2, NULL, 0, &byteNeeded) && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		{
			aErrorMessage = QString("GetPrinter error: %1").arg(getLastErrorMessage());
			ClosePrinter(printer);
			return false;
		}

		printerInfoBuffer.fill(0, byteNeeded);
		printerInfo = reinterpret_cast<PRINTER_INFO_2 *>(printerInfoBuffer.data());

		/* Get the printer information. */
		if (GetPrinter(printer, 2, reinterpret_cast<LPBYTE>(printerInfo), byteNeeded, &byteUsed))
		{
			directPrinting = (printerInfo->Attributes & PRINTER_ATTRIBUTE_DIRECT);

			if (printerInfo->Status & PRINTER_STATUS_PAUSED)
			{
				// запускаем печать, если печать на принтер была приостановлена
				result = SetPrinter(printer, 0, NULL, PRINTER_CONTROL_RESUME);
			}
		}

		ClosePrinter(printer);
	}
	else
	{
		aErrorMessage = getLastErrorMessage();

		return false;
	}

	if (!directPrinting)
	{
		return true;
	}

	// Открываем с правами админа
	if (OpenPrinter((LPWSTR)aPrinterName.toStdWString().data(), &printer, &defaultsAdmin))
	{
		// очищаем очередь заданий
		result = SetPrinter(printer, 0, NULL, PRINTER_CONTROL_PURGE);

		printerInfo->Attributes &= ~(DWORD)(PRINTER_ATTRIBUTE_DIRECT);
		printerInfo->Attributes |= PRINTER_ATTRIBUTE_QUEUED;

		// переключаем режим печати на печать через очередь
		result = SetPrinter(printer, 2, reinterpret_cast<LPBYTE>(printerInfo), 0);

		if (!result)
		{
			aErrorMessage = getLastErrorMessage();
		}

		if (printerInfo->Status & PRINTER_STATUS_PAUSED)
		{
			// запускаем печать, если печать на принтер была приостановлена
			result = SetPrinter(printer, 0, NULL, PRINTER_CONTROL_RESUME);
		}

		ClosePrinter(printer);
	}
	else
	{
		aErrorMessage = getLastErrorMessage();

		return false;
	}

	return result;
}

//--------------------------------------------------------------------------------
