#include "LptPort.h"
#include "stdio.h"

CLptPort::CLptPort(int PortNum)
{
	bOpenLptDriver = false;
    static bool bInitialized = false;	// Инициализируем 1 раз при вызове CLptPort::CLptPort
    if(!bInitialized)
    {
    	hDLL = NULL;
    	LptPortNum = PortNum;
        if( CopyTVicPortSys() )	Init();
    }
}
CLptPort::~CLptPort(void)
{
	if(bOpenLptDriver)
    {
        CloseTVicPort();
        FreeLibrary(hDLL);
        hDLL = NULL;
        DeleteTVicPortSys();
    }
}
//------------------------------------------
bool __stdcall CLptPort::Init()
{
    hDLL = LoadLibrary("TVicPort.dll");
    if(hDLL)
    {
        //------------------------------------------
        OpenTVicPort		= (TOpenTVicPort) GetProcAddress(hDLL, "OpenTVicPort");
        IsDriverOpened		= (TIsDriverOpened)GetProcAddress(hDLL, "IsDriverOpened");
        CloseTVicPort		= (TCloseTVicPort)GetProcAddress(hDLL, "CloseTVicPort");

        SetLPTNumber		= (TSetLPTNumber)GetProcAddress(hDLL, "SetLPTNumber");

        SetPin				= (TSetPin)GetProcAddress(hDLL, "SetPin");
        GetPin				= (TGetPin)GetProcAddress(hDLL, "GetPin");

        GetLPTNumPorts		= (TGetLPTNumPorts)GetProcAddress(hDLL, "GetLPTNumPorts");
        GetLPTNumber		= (TGetLPTNumber)GetProcAddress(hDLL, "GetLPTNumber");
        GetLPTBasePort		= (TGetLPTBasePort)GetProcAddress(hDLL, "GetLPTBasePort");
        //------------------------------------------

        ChangeLptPortNumber();
        OpenLptDriver();
        return true;
    }
    return false;
}
bool __stdcall CLptPort::ChangeLptPortNumber()
{
  if ( (LptPortNum>0) && (LptPortNum <= GetLPTNumPorts()) )
  {
	  SetLPTNumber((UCHAR)LptPortNum);
	  return true;
  }
  return false;
}
bool __stdcall CLptPort::OpenLptDriver()
{	
	OpenTVicPort();
	if (IsDriverOpened())
	{
    	bOpenLptDriver = true;
	 	for (int n=1; n<=17; n++)
        {
        	if(n != 10 && n != 11 && n != 12 && n != 13 && n != 15)
            	SetPin(n, 0);
        }
		GetLptInfo();
		return true;
	}
	return false;
}
void __stdcall CLptPort::GetLptInfo()
{  
    LptNumPorts = GetLPTNumPorts();
    CurrentLptPort = GetLPTNumber();
    sprintf(CurrentLptBasePort, "%03Xh", GetLPTBasePort());
}
bool CLptPort::DllIsLoaded()
{
	return (hDLL != NULL);
}
bool CLptPort::CopyTVicPortSys()
{
     OSVERSIONINFO Ver;
     Ver.dwOSVersionInfoSize= sizeof(OSVERSIONINFO);

	if(GetVersionEx(&Ver))
	{
		if(Ver.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
            switch(Ver.dwMinorVersion)
            {
                case 0:	 	//	Windows 2000
                {
					return CopyFile("TVicPort.sys", "C:\\WINNT\\system32\\drivers\\TVicPort.sys", FALSE);
                }
                case 1: 	//	Windows XP
                {
					return CopyFile("TVicPort.sys", "C:\\WINDOWS\\system32\\drivers\\TVicPort.sys", FALSE);
                }
            }
		}
	}
    return false;
}
bool CLptPort::DeleteTVicPortSys()
{
     OSVERSIONINFO Ver;
     Ver.dwOSVersionInfoSize= sizeof(OSVERSIONINFO);

	if(GetVersionEx(&Ver))
	{
		if(Ver.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
            switch(Ver.dwMinorVersion)
            {
                case 0:	 	//	Windows 2000
                {
					return DeleteFile("C:\\WINNT\\system32\\drivers\\TVicPort.sys");
                }
                case 1: 	//	Windows XP
                {
					return DeleteFile("C:\\WINDOWS\\system32\\drivers\\TVicPort.sys");
                }
            }
		}
	}
    return false;
}
