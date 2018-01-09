#include "globals.h"
#include "windows.h"
#include <vcl.h>
#include <string>
#include "boost\format.hpp"
#include "time.h"
#include "Tlhelp32.h"
std::string FileVersion;

void ReadVersion()
{
    unsigned long visize = GetFileVersionInfoSize(Application->ExeName.c_str(), &visize);
    if(visize > 0) {
        unsigned short *data;
        unsigned int datasize;
        char *pBuf = new char[visize];
        if(GetFileVersionInfo(Application->ExeName.c_str(), 0, visize, pBuf)) {
            if(VerQueryValue(pBuf, "\\VarFileInfo\\Translation", (void **)(&data), &datasize)) {
                std::string version = (boost::format("StringFileInfo\\%04x%04x\\FileVersion") % data[0] % data[1]).str();
                VerQueryValue(pBuf, (char *)version.c_str(), (void **)(&data), &datasize);
                FileVersion = (char *)data;
            }
        }
        delete[] pBuf;
    }
}

std::string GetCurrentDateTime(const char *Format)
{
    struct tm *time_now;
    ::time_t secs_now;
    char curtimestr[64];
    memset(curtimestr, 0, sizeof(curtimestr));
    time(&secs_now);
    time_now = localtime(&secs_now);
    strftime(curtimestr, sizeof(curtimestr), Format, time_now);
    return std::string(curtimestr);
}

void ExceptionFilter(const char* File, const char* Function, int Line, TLogClass* log)
{
    try
    {
        std::string strtmp=File;
        strtmp=strtmp.substr(strtmp.rfind("\\")+1);
        bool dellog = false;
        if(!log)
        {
            dellog = true;
            try
            {
                log = new TLogClass("exception");
            }
            catch(...)
            {
                return;
            }
        }

        try
        {
            throw;
        }
        catch(std::exception& ex)
        {
            log->Write(" ");
            log->Write((boost::format("std::exception in file: %1% ,from function: %2%, catch line: %3%, message: %4%") % strtmp % Function % Line % ex.what()).str().c_str());
            log->Write(" ");
        }
        catch(Exception& ex)
        {
            log->Write(" ");
            log->Write((boost::format("borland Exception in file: %1% ,from function: %2%, catch line: %3%, message: %4%") % strtmp % Function % Line % ex.Message.c_str()).str().c_str());
            log->Write(" ");

            std::string strtmp = ex.Message.c_str();

            if(std::string::npos != strtmp.find("Разрушительный сбой"))
            {
                log->Write("Critical exception, exit");
                exit(0);
            }

            #ifdef __CONN__
              if(std::string::npos != strtmp.find("EAccessViolation"))
              {
                  log->Write("Critical exception, exit");
                  exit(0);
              }
            #endif
        }
        catch(...)
        {
            log->Write(" ");
            log->Write((boost::format("Unknown Exception in file: %1% ,from function: %2%, catch line: %3%, ") % strtmp % Function % Line).str().c_str());
            log->Write(" ");
        }

        if(dellog)
        {
            try
            {
                delete log;
            }
            catch(...)
            {
            }
        }
    }
    catch(...)
    {
    }
}

//---------------------------------------------------------------------------

bool killProgramm(char* programmName, TLogClass* Log)
{
    enableDebugPrivilege(true);
    HANDLE hProgramm = findProgramm(programmName);
    bool result = false;

    if(hProgramm)
        result = TerminateProcess(hProgramm, -1);

    if (Log)
    {
        Log->Write((boost::format("kill %1%... %2%!") % programmName % (result ? "OK" : "fail")).str().c_str());
    }

    return result;
}

//---------------------------------------------------------------------------

HANDLE findProgramm(char* programmName)
{
    HANDLE hProcessSnap;

    hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
    if( hProcessSnap == INVALID_HANDLE_VALUE )
        return NULL;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof( PROCESSENTRY32 );
    if( !Process32First( hProcessSnap, &pe32 ) )
    {
      CloseHandle( hProcessSnap );
      return NULL;
    }

    enableDebugPrivilege(true);

    do
    {
      DWORD dwPriorityClass = 0;
      HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID );
      if( hProcess != NULL )
      {
        std::string peName(pe32.szExeFile);
        if(peName.find(programmName) != std::string::npos)
        {
            HANDLE hResultProcess = hProcess;
            CloseHandle( hProcessSnap );
            return hResultProcess;
        }
      }
    } while( Process32Next( hProcessSnap, &pe32 ) );

    CloseHandle( hProcessSnap );
    return NULL;
}

//---------------------------------------------------------------------------

void enableDebugPrivilege(bool fEnable)
{
    HANDLE hToken;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
    {
        TOKEN_PRIVILEGES tp;
        tp.PrivilegeCount = 1;
        
        LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
        tp.Privileges[0].Attributes = fEnable ? SE_PRIVILEGE_ENABLED : 0;
        AdjustTokenPrivileges(hToken, false, &tp, sizeof(tp), NULL, NULL);
        CloseHandle(hToken);
    }
}
