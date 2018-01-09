/* @file Реализация интерфейса SysUtils. */

// windows
#include <windows.h>
#include <psapi.h>
#include <TlHelp32.h>

// STL
#include <algorithm>

// Модули
#include "SysUtils/ISysUtils.h"
#include "SysUtils/windows/PrivilegeElevator.h"

#pragma comment (lib, "User32")
#pragma comment (lib, "Psapi")

//--------------------------------------------------------------------------------
int ISysUtils::systemReboot()
{
	PrivilegeElevator privilege(SE_SHUTDOWN_NAME);

	//Shut down the system and force all applications to close.
	if (!::ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0)) 
	{
		return (int)::GetLastError();
	}

	return 0;
}

//--------------------------------------------------------------------------------
int ISysUtils::systemShutdown()
{
	PrivilegeElevator privilege(SE_SHUTDOWN_NAME);

	//Shut down the system and force all applications to close.
	if (!::ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0)) 
	{
		return (int)::GetLastError();
	}

	return 0;
}

//--------------------------------------------------------------------------------
QString ISysUtils::getOSVersionInfo()
{
	QString version = ("Unknown");

	// Если используется Windows
	switch (QSysInfo::WindowsVersion)
	{
		case QSysInfo::WV_32s:        version = "Windows 3.1";   break;
		case QSysInfo::WV_95:         version = "Windows 95";    break;
		case QSysInfo::WV_98:         version = "Windows 98";    break;
		case QSysInfo::WV_Me:         version = "Windows Me";    break;
		case QSysInfo::WV_NT:         version = "Windows NT";    break;
		case QSysInfo::WV_2000:       version = "Windows 2000";  break;
		case QSysInfo::WV_XP:         version = "Windows XP";    break;
		case QSysInfo::WV_2003:       version = "Windows 2003";  break;
		case QSysInfo::WV_VISTA:      version = "Windows Vista"; break;
		case QSysInfo::WV_WINDOWS7:   version = "Windows 7";     break;
		case QSysInfo::WV_WINDOWS8:   version = "Windows 8";     break;
		case QSysInfo::WV_WINDOWS8_1: version = "Windows 8.1";   break;
		case QSysInfo::WV_WINDOWS10:  version = "Windows 10";    break;
		default:
		{
			if (QSysInfo::WV_NT_based & QSysInfo::WindowsVersion)
			{
				version = "Windows NT based";
				break;
			}
		}
	}

	OSVERSIONINFOEXW info;
	ZeroMemory(&info, sizeof(info));
	info.dwOSVersionInfoSize = sizeof(info);

	::GetVersionExW((LPOSVERSIONINFOW)&info);

	return version + QString("%1 %2.%3.%4 sp%5.%6 (0x%7)")
		.arg((info.wSuiteMask & VER_SUITE_EMBEDDEDNT) ? " Embedded" : "")
		.arg(info.dwMajorVersion).arg(info.dwMinorVersion).arg(info.dwBuildNumber)
		.arg(info.wServicePackMajor).arg(info.wServicePackMinor)
		.arg(info.wSuiteMask, 8, 16, QChar('0'));
}

//--------------------------------------------------------------------------------
void ISysUtils::disableScreenSaver()
{
	::SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, FALSE, NULL, 0);
}

//--------------------------------------------------------------------------------
void ISysUtils::displayOn(bool aOn)
{
	enum {
		DISPLAY_ON = -1,
		DISPLAY_OFF = 2
	};

	if (aOn)
	{
		PostMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, DISPLAY_ON);
		
		// имитируем дёргание мышки для включения монитора в Windows 8
		INPUT input;
		SecureZeroMemory(&input, sizeof(input));
		input.type = INPUT_MOUSE;
		input.mi.dy = 1;
		input.mi.dwFlags = MOUSEEVENTF_MOVE;

		SendInput(1, &input, sizeof(input));
		SleepEx(40, TRUE);

		input.mi.dy = -1;
		SendInput(1, &input, sizeof(input));
	}
	else
	{
		PostMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, DISPLAY_OFF);
	}
}

//--------------------------------------------------------------------------------
QString getLastErrorMessage()
{
	HLOCAL hlocal = NULL;
	FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 
		MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPTSTR) &hlocal, 0, NULL);

	QString msg;

	if (hlocal != NULL)
	{
		msg = QString::fromWCharArray((const wchar_t *)LocalLock(hlocal));
		LocalFree(hlocal);
	}

	return QString("%1 (%2)").arg(GetLastError()).arg(msg.trimmed());
}

//--------------------------------------------------------------------------------
void ISysUtils::setSystemTime(QDateTime aDateTime) throw (...)
{
	PrivilegeElevator privilege(SE_SYSTEMTIME_NAME);

	// Синхронизируем
	SYSTEMTIME localTime;

	localTime.wYear = static_cast<WORD>(aDateTime.date().year());
	localTime.wMonth = static_cast<WORD>(aDateTime.date().month());
	localTime.wDay = static_cast<WORD>(aDateTime.date().day());
	localTime.wHour = static_cast<WORD>(aDateTime.time().hour());
	localTime.wMinute = static_cast<WORD>(aDateTime.time().minute());
	localTime.wSecond = static_cast<WORD>(aDateTime.time().second());
	localTime.wMilliseconds = static_cast<WORD>(aDateTime.time().msec());

	BOOL result = FALSE;

	switch (aDateTime.timeSpec())
	{
		case Qt::UTC:
		{
			result = ::SetSystemTime(&localTime);
			break;
		}
		default:
		{
			result = ::SetLocalTime(&localTime);
			break;
		}
	}

	if (!result)
	{
		throw Exception(ECategory::System, ESeverity::Major, GetLastError(), QString("Windows error: %1.").arg(getLastErrorMessage()));
	}
}

//---------------------------------------------------------------------------
void ISysUtils::sleep(int aMs)
{
	Q_ASSERT(aMs > 0);

	::SleepEx(uint(aMs), FALSE);
}


//--------------------------------------------------------------------------------
bool ISysUtils::getProcessMemoryUsage(MemoryInfo & aMemoryInfo, const QProcess * aProcess /*= nullptr*/)
{
	ZeroMemory(&aMemoryInfo, sizeof(MemoryInfo));

	DWORD processId = GetCurrentProcessId();

	if (aProcess)
	{
		processId = aProcess->pid()->dwProcessId;
	}

	// http://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);

	aMemoryInfo.total = memInfo.ullTotalPhys;
	aMemoryInfo.totalUsed =  memInfo.ullTotalPhys - memInfo.ullAvailPhys;

	PROCESS_MEMORY_COUNTERS_EX pmc;
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);

	if (hProcess) 
	{
		if (GetProcessMemoryInfo(hProcess, reinterpret_cast<PROCESS_MEMORY_COUNTERS *>(&pmc), sizeof(pmc)))
		{
			CloseHandle(hProcess);
			aMemoryInfo.processUsed = pmc.WorkingSetSize;
			return true;
		}

		CloseHandle(hProcess);
	}

	return false;
}

//---------------------------------------------------------------------------------
bool ISysUtils::bringWindowToFront(WId aWindow)
{
	if (!::IsWindow(aWindow))
	{
		return false;
	}

	DWORD dwThreadID = GetWindowThreadProcessId(aWindow, NULL);
	if (dwThreadID)
	{
		AttachThreadInput(dwThreadID, GetCurrentThreadId(), true);
	}

	// Прописываем окну флаг HWND_TOPMOST
	SetWindowPos(aWindow, HWND_TOPMOST, NULL, NULL, NULL, NULL, SWP_NOMOVE | SWP_NOSIZE);

	SetForegroundWindow(aWindow);
	SetActiveWindow(aWindow);
	SetFocus(aWindow);

	if (dwThreadID)
	{
		AttachThreadInput(dwThreadID, GetCurrentThreadId(), false);
	}

	return true;
}

//---------------------------------------------------------------------------------
bool ISysUtils::bringWindowToFront(const QString & aWindowTitle)
{
	wchar_t * array = new wchar_t[aWindowTitle.size() + 1];
	ZeroMemory(array, (aWindowTitle.size() + 1) * sizeof(wchar_t));
	aWindowTitle.toWCharArray(array);
	HWND hwnd = ::FindWindowW(NULL, array);
	delete[] array;

	return bringWindowToFront(hwnd);
}

//---------------------------------------------------------------------------------
bool ISysUtils::getAllProcessHandleCount(quint32 & aCountOfHandles)
{
	aCountOfHandles = 0;

	PROCESSENTRY32 pe32;
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);
		return false;
	}

	do
	{
		DWORD handleCount = 0;
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);

		if (hProcess != INVALID_HANDLE_VALUE && hProcess != NULL)
		{
			if (GetProcessHandleCount(hProcess, &handleCount))
			{
				aCountOfHandles += handleCount;
			}

			CloseHandle(hProcess);
		}
	} while (Process32Next(hProcessSnap, &pe32));

	return true;
}

//--------------------------------------------------------------------------------
void ISysUtils::runScreenSaver()
{
	SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_SCREENSAVE, 0);
}

//--------------------------------------------------------------------------------
