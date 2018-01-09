/* @file Реализация перечислителя процессов в системе. */

// Windows
#include <windows.h>
#include <psapi.h>

// stl
#include <algorithm>

// Project
#include "processenumerator.h"

#pragma comment (lib, "Psapi.lib")

//----------------------------------------------------------------------------
namespace CProcessEnumerator
{
	const DWORD WM_EXITEXPLORER = 0x5B4;
	const int WaitExplorerExit = 3000;

	static QMap<QString, QString> dosDriveMap;
}

//----------------------------------------------------------------------------
void loadDosDriveMap(QMap<QString, QString> & aResult)
{
	DWORD size = MAX_PATH;
	char logicalDrives[MAX_PATH] = { 0 };
	DWORD result = GetLogicalDriveStringsA(size, logicalDrives);

	if (result > 0 && result <= MAX_PATH)
	{
		char * szSingleDrive = logicalDrives;
		while (*szSingleDrive)
		{
			char str[3] = { 0 };
			strncpy(str, szSingleDrive, 2);

			char path[MAX_PATH] = { 0 };
			if (QueryDosDeviceA(str, path, MAX_PATH))
			{
				aResult.insert(QString::fromLatin1(path), QString::fromLatin1(str));
			}

			// get the next drive
			szSingleDrive += strlen(szSingleDrive) + 1;
		}
	}
}

//----------------------------------------------------------------------------
bool waitForClose(ProcessEnumerator::PID aPid, int aTimeout)
{
	bool result = false;
	HANDLE process = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE, aPid);

	if (process)
	{
		result = WaitForSingleObject(process, aTimeout) == WAIT_OBJECT_0;
		CloseHandle(process);
	}
	else
	{
		// Проверка на наличие процесса, возможно слишком быстро умер
		result = GetLastError() == ERROR_INVALID_PARAMETER;
	}


	return result;
}

//----------------------------------------------------------------------------
ProcessEnumerator::ProcessEnumerator()
{
	if (CProcessEnumerator::dosDriveMap.isEmpty())
	{
		loadDosDriveMap(CProcessEnumerator::dosDriveMap);
	}

	enumerate();
}

//----------------------------------------------------------------------------
ProcessEnumerator::const_iterator ProcessEnumerator::begin() const
{
	return mProcesses.begin();
}

//----------------------------------------------------------------------------
ProcessEnumerator::const_iterator ProcessEnumerator::end() const
{
	return mProcesses.end();
}

//----------------------------------------------------------------------------
struct handle_data
{
	ProcessEnumerator::PID process_id;
	HWND best_handle;
};

//----------------------------------------------------------------------------
BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam)
{
	handle_data & data = *(handle_data*)lParam;
	unsigned long process_id = 0;

	GetWindowThreadProcessId(handle, &process_id);
	if (data.process_id == process_id) 
	{
		data.best_handle = handle;
		PostMessage(handle, WM_CLOSE, 0, 0);
	}
	
	return TRUE;
}

//----------------------------------------------------------------------------
bool ProcessEnumerator::kill(PID aPid, quint32 & aErrorCode) const
{
	bool result = false;

	if (mProcesses.contains(aPid))
	{
		if (mProcesses.value(aPid).path.contains("explorer.exe", Qt::CaseInsensitive))
		{
			OSVERSIONINFOEX osVersion = { sizeof(OSVERSIONINFOEX), 0 };
			GetVersionEx((OSVERSIONINFO *)&osVersion);

			// http://stackoverflow.com/a/6246182/1073032
			if (osVersion.dwMajorVersion < 6)
			{
				// Windows XP
				result = PostMessage(FindWindow(L"Progman", NULL), WM_QUIT, 0, FALSE);   // <=  lParam == FALSE !
				aErrorCode = GetLastError();

				if (!waitForClose(aPid, CProcessEnumerator::WaitExplorerExit))
				{
					return killInternal(aPid, aErrorCode);
				}
			}
			else
			{
				// Windows 7
				HWND trayWnd = FindWindow(L"Shell_TrayWnd", NULL);
				if (trayWnd)
				{
					result = PostMessage(trayWnd, CProcessEnumerator::WM_EXITEXPLORER, 0, 0);
					aErrorCode = GetLastError();

					if (!waitForClose(aPid, CProcessEnumerator::WaitExplorerExit))
					{
						return killInternal(aPid, aErrorCode);
					}
				}
				else
				{
					return killInternal(aPid, aErrorCode);
				}
			}
		}
		else
		{
			return killInternal(aPid, aErrorCode);
		}
	}

	return result;
}

//----------------------------------------------------------------------------
bool ProcessEnumerator::killInternal(PID aPid, quint32 & aErrorCode) const
{
	bool result = false;

	// http://support.microsoft.com/kb/178893
	handle_data data = { aPid, 0 };
	EnumWindows(enum_windows_callback, (LPARAM)&data);

	HANDLE process = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE, aPid);

	// Ждем немного что бы приложение закрылось
	if (data.best_handle && process)
	{
		if (WaitForSingleObject(process, 1000) == WAIT_OBJECT_0)
		{
			CloseHandle(process);
			process = NULL;
			return true;
		}
	}

	if (process)
	{
		result = TerminateProcess(process, 0);
		aErrorCode = GetLastError();
		CloseHandle(process);
	}
	
	return result;
}

//----------------------------------------------------------------------------
bool getModuleName(HANDLE aProcess, QString & aPath)
{
	wchar_t path[MAX_PATH] = { 0 };
	if (GetProcessImageFileName(aProcess, path, MAX_PATH))
	{
		aPath = QString::fromWCharArray(path);

		QMapIterator<QString, QString> i(CProcessEnumerator::dosDriveMap);
		while (i.hasNext()) 
		{
			i.next();
			aPath.replace(i.key(), i.value());
		}

		return true;
	}

	return false;
}

//----------------------------------------------------------------------------
void ProcessEnumerator::enumerate()
{
	DWORD processes[1024] = { 0 }, cbNeeded = 0, cProcesses = 0;

	if (EnumProcesses(processes, sizeof(processes), &cbNeeded))
	{
		// Calculate how many process identifiers were returned.
		cProcesses = cbNeeded / sizeof(DWORD);

		// Print the name and process identifier for each process.
		for (unsigned int i = 1; i < cProcesses && processes[i] != 0; i++)
		{
			// Get a handle to the process.
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);

			// Get the process name.
			if (NULL != hProcess)
			{
				QString path;

				if (getModuleName(hProcess, path))
				{
					ProcessInfo pInfo = { processes[i], path };
					
					mProcesses.insert(processes[i], pInfo);
				}

				CloseHandle(hProcess);
			}
		}
	}
}



//----------------------------------------------------------------------------
