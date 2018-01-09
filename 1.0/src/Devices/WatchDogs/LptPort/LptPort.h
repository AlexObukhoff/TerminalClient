#ifndef LptPortH
#define LptPortH
#include <windows.h>
#pragma once

class CLptPort
{
private:
	HINSTANCE hDLL;
    bool bOpenLptDriver;
    int LptPortNum;

    //------------------------------------------
    typedef	void	__stdcall (*TOpenTVicPort)();
    typedef	bool	__stdcall (*TIsDriverOpened)();
	typedef	void	__stdcall (*TCloseTVicPort)();
	typedef void  	__stdcall (*TSetLPTNumber)(UCHAR);
	typedef BOOL 	__stdcall (*TGetLPTNumPorts)();
    typedef UCHAR  	__stdcall (*TGetLPTNumber)();
    typedef BOOL  	__stdcall (*TGetLPTBasePort)();

    TOpenTVicPort	OpenTVicPort;
    TIsDriverOpened IsDriverOpened;
    TCloseTVicPort	CloseTVicPort;
    TSetLPTNumber  	SetLPTNumber;
	TGetLPTNumPorts	GetLPTNumPorts;
    TGetLPTNumber	GetLPTNumber;
    TGetLPTBasePort	GetLPTBasePort;
	//------------------------------------------

    bool __stdcall Init();
    bool __stdcall ChangeLptPortNumber();
	bool __stdcall OpenLptDriver();
	void __stdcall GetLptInfo();
    bool 		   CopyTVicPortSys();
    bool  		   DeleteTVicPortSys();
public:
	USHORT	LptNumPorts;
	USHORT	CurrentLptPort;
	char	CurrentLptBasePort[255];

	//------------------------------------------
	CLptPort(int LptPortNum = 1);
    ~CLptPort(void);    
	//------------------------------------------
	typedef BOOL 	__stdcall (*TSetPin)(UCHAR PinNumber, BOOL PinLevel);
	typedef BOOL 	__stdcall (*TGetPin)(UCHAR PinNumber);
    
	TSetPin		   	SetPin;
    TGetPin		   	GetPin;
	//------------------------------------------
    bool DllIsLoaded();
};
#endif
