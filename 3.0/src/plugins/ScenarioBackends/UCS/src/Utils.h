#pragma once

#include <windows.h>
#include <psapi.h> // For access to GetModuleFileNameEx
#include <tchar.h>
#include <tlhelp32.h>
#include <iostream>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <Common/QtHeadersEnd.h>

namespace CUtils
{
	TCHAR UnitellerHostName[] = L"Uniteller.SoftwareManager.ServiceHost.exe";
	TCHAR UnitellerConsoleHostName[] = L"Uniteller.Framework.Kernel.ConsoleHost.exe";
	char UnitellerServiceConfig[] = "service.config";
	char UnitellerIdentificationKey[] = "Identification";
}

namespace Ucs
{

//---------------------------------------------------------------------------
inline QString getUnitellerHostPath()
{
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	QString path;

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry))
	{
		while (Process32Next(snapshot, &entry))
		{
			if (_wcsicmp(entry.szExeFile, CUtils::UnitellerHostName) == 0 ||
				_wcsicmp(entry.szExeFile, CUtils::UnitellerConsoleHostName) == 0)
			{
				HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);

				HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, entry.th32ProcessID);

				TCHAR fullPath[MAX_PATH] = { 0 };
				
				if (processHandle != NULL)
				{
					if (GetModuleFileNameEx(processHandle, NULL, fullPath, MAX_PATH) != 0)
					{
						CloseHandle(processHandle);
						CloseHandle(hProcess);
						CloseHandle(snapshot);

						return QFileInfo(QString::fromWCharArray(fullPath)).dir().path();
					}

					CloseHandle(processHandle);
				}

				CloseHandle(hProcess);
			}
		}
	}

	CloseHandle(snapshot);

	return QString();
}

//---------------------------------------------------------------------------
inline QString getUPID()
{
	QFile config(QString("%1%2%3").arg(getUnitellerHostPath()).arg(QDir::separator()).arg(CUtils::UnitellerServiceConfig));

	if (config.open(QIODevice::ReadOnly))
	{
		QStringList content = QString(config.readAll()).split("\n");

		foreach (QString line, content)
		{
			if (!line.contains(CUtils::UnitellerIdentificationKey))
			{
				continue;
			}

			QRegExp rx("\"(.*)\"");
			return rx.indexIn(line) != -1 ? rx.capturedTexts().last().rightJustified(10, '0').right(10) : QString().fill('0', 10);
		}
	}

	return QString();
}

} //namespace Uniteller
