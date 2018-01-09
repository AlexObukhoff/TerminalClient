//---------------------------------------------------------------------------


#include <time.h>
#pragma hdrstop

#include "TFileMap.h"
#include "common.h"
#include "globals.h"
#include "boost/format.hpp"

//---------------------------------------------------------------------------

TFileMap::TFileMap(TLogClass *_Log)
{
    try
    {
        InnerLog=false;
        Connected=false;
        hFileMapping = NULL;
        lpFileMap = NULL;
        Map=NULL;
        if (_Log==NULL)
        {
            Log = new TLogClass("TFileMap");
            InnerLog=true;
        }
        else
        {
            Log=_Log;
        }
        CS = NULL;
        CS = new TCriticalSection();
        //Log->Write("TFileMap initialized...");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Connected=false;
    }
}

//---------------------------------------------------------------------------

void TFileMap::Create(AnsiString FileMapName)
{
    try
    {
        Map = NULL;
        Connected=false;
        int MapSize = 2048;
        hFileMapping = NULL;
        hFileMapping = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, MapSize, FileMapName.c_str());
        if (hFileMapping==NULL)
        {
            Log->Write(ShowError("File-mapping object creating error").c_str());
            return;
        }
        else
        {
            lpFileMap = NULL;
            lpFileMap = MapViewOfFile(hFileMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
            if (lpFileMap==NULL)
            {
                Log->Write(ShowError("Mapping view of the file-mapping object error").c_str());
                return;
            }
        }
        Map = (TSFileMap*)lpFileMap;
        Clear();
        Connected=true;
        Log->Write((boost::format("File mapping created, size: %1% B, %2% B used.") % MapSize % sizeof(TSFileMap)).str().c_str());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Map=NULL;
        Connected=false;
    }
}

//---------------------------------------------------------------------------

void TFileMap::Clear()
{
    try
    {
        if (Map)
        {
            Map->WCHandle=NULL;
            memset(Map->Session, 0, strlen(Map->Session));
            Map->WCTimeMark.Val=0;
            Map->ConnTimeMark.Val=0;
            Map->LastConnectionDT.Val=0;
            Map->LastSuccessfullConnectionDT.Val=0;
            Map->LastCPConnectionDT.Val=0;
            Map->LastSuccessfullCPConnectionDT.Val=0;
            //Map->ConnSendHeartBeatThreadTimeMark.Val=0;
            Map->Command=0;
            Map->Idle=true;
            Map->CheckStatDir=false;
            Map->WCState=0;
            Map->DebugState = 0;
            Map->LastPaymentReceived.Val=0;
            Map->LastPaymentProcessed.Val=0;
            Map->BillsCount=-1;
            Map->BillsSum=-1;
            Map->SIMBalance=-1;
            Map->GSMSignalQuality=-1;
            memset(Map->ValidatorOutState, 0, strlen(Map->ValidatorOutState));
            //Map->ValidatorOutState[255];
            memset(Map->PrinterOutState, 0, strlen(Map->PrinterOutState));
            //Map->PrinterOutState[255];
            Map->ValidatorState=-1;
            Map->PrinterState=-1;
            Map->UnprocPaymentsCount=0;
            Map->UnprocStatPacketsCount=0;
            Map->SW0=-1;
            Map->SW1=-1;
            Map->SW2=-1;
            Map->GSMOperatorID=-1;
            Map->CID=-1;
            memset(Map->CIDFileName, 0, strlen(Map->CIDFileName));
            Map->NumDBUpdateDone = false;
            Map->NumDBLastUpdatedDT.Val = 0;
            Map->WriteErrorFound = false;
            Map->FirstConnected = false;
            Map->ForceBlock = false;
            Map->MonSrvrConnectOK = false;
            Map->WCRestartReq = false;
            Map->m_incrementMoneyTransferToken = false;
            Map->m_MoneyTransferToken = 0;
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TFileMap::Open(AnsiString FileMapName)
{
    try
    {
        Connected=false;
        hFileMapping = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, FileMapName.c_str());
        if (hFileMapping==NULL)
        {
            Log->Write(ShowError("File-mapping object '"+FileMapName+"' opening error").c_str());
            //Create(FileMapName);
            return;
        }
        else
        {
            lpFileMap = MapViewOfFile(hFileMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
            if (hFileMapping==NULL)
            {
                Log->Write(ShowError("Mapping view of the file-mapping object error").c_str());
                //Close();
                //Create(FileMapName);
                return;
            }
        }
        Map = (TSFileMap*)lpFileMap;
        Connected=true;
        Log->Write("File mapping connected...");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Map=NULL;
        Connected=false;
    }
}

//---------------------------------------------------------------------------

void TFileMap::Close()
{
    try
    {
        if (lpFileMap)
            if (!UnmapViewOfFile(lpFileMap))
                Log->Write(ShowError("Unmapping view of the file-mapping object error").c_str());
        //else
        //CloseHandle(lpFileMap);
        if (hFileMapping)
            if (!CloseHandle(hFileMapping))
                Log->Write(ShowError("File-mapping object closing error").c_str());
        Map=NULL;
        Connected=false;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Map=NULL;
        Connected=false;
    }
}

//---------------------------------------------------------------------------

TFileMap::~TFileMap()
{
    Close();
    if (CS!=NULL)
    {
        CS->Acquire();
        CS->Release();
        delete CS;
        CS = NULL;
    }
    if (InnerLog)
        delete Log;
}

//---------------------------------------------------------------------------

/*void TFileMap::ShowError(AnsiString Header)
{
try
		{
		LPVOID lpMsgBuf;
		int ErrorCode = GetLastError();
		if (ErrorCode!=0) {
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,NULL,ErrorCode,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0,NULL);
				Log->Write(Header+": "+AnsiString(ErrorCode)+" "+AnsiString((char*)lpMsgBuf));
				LocalFree(lpMsgBuf);
				}
		}
catch (Exception &exception)
		{
		Log->WriteInLine("Exception in TFileMap.ShowError: "+exception.Message);
		}
}*/

//---------------------------------------------------------------------------
/*
void TFileMap::WriteInt(int Source, int Offset)
{
LPSTR DstPtr = (LPSTR)lpFileMap + Offset;
memcpy(DstPtr, &Source, sizeof(int));
}

//---------------------------------------------------------------------------

int TFileMap::ReadInt(int Offset)
{
LPSTR DstPtr = (LPSTR)lpFileMap + Offset;
int Tgt=0;
memcpy(&Tgt, DstPtr, sizeof(int));
return Tgt;
}

//---------------------------------------------------------------------------

void TFileMap::WriteLong(long Source, int Offset)
{
LPSTR DstPtr = (LPSTR)lpFileMap + Offset;
memcpy(DstPtr, &Source, sizeof(long));
}

//---------------------------------------------------------------------------

long TFileMap::ReadLong(int Offset)
{
LPSTR DstPtr = (LPSTR)lpFileMap + Offset;
int Tgt=0;
memcpy(&Tgt, DstPtr, sizeof(long));
return Tgt;
} */

//---------------------------------------------------------------------------

HANDLE TFileMap::ReadWCHandle()
{
    if (Map)
    {
//    Log->Write("Handle: "+AnsiString(Map->WCHandle));
        return (void*)Map->WCHandle;
    }
    else
        return 0;
}

//---------------------------------------------------------------------------

void TFileMap::WriteWCHandle(int Src)
{
    if (Map)
        Map->WCHandle=(UINT)Src;
}

//---------------------------------------------------------------------------

/*ULONG TFileMap::ReadConnTimeMark()
{
if (Map)
    return Map->ConnDateTime;
    else
    return 0;
}

//---------------------------------------------------------------------------

void TFileMap::WriteConnTimeMark()
{
time_t tim = time(NULL);
if (Map)
    Map->ConnDateTime=tim;
}

//---------------------------------------------------------------------------

ULONG TFileMap::ReadWCTimeMark()
{
if (Map)
    return Map->WCDateTime;
    else
    return 0;
}

//---------------------------------------------------------------------------

void TFileMap::WriteWCTimeMark()
{
time_t tim = time(NULL);
if (Map)
    Map->WCDateTime=tim;
}*/

//---------------------------------------------------------------------------

int TFileMap::ReadCommand()
{
    return (Map) ? Map->Command : 0;
}

//---------------------------------------------------------------------------

void TFileMap::WriteCommand(int _Command)
{
    if (Map)
        Map->Command=_Command;
}

//---------------------------------------------------------------------------

TDateTime TFileMap::GetCurrentTime()
{
    return TDateTime::CurrentDateTime();
}

//---------------------------------------------------------------------------

bool TFileMap::ReadWCIdle()
{
    return (Map) ? Map->Idle : 0;
}

//---------------------------------------------------------------------------

void TFileMap::WriteWCIdle(bool _Idle)
{
    if (Map)
        Map->Idle=_Idle;
}

//---------------------------------------------------------------------------

short TFileMap::ReadWCState()
{
    return (Map) ? Map->WCState : (short)0;
}

//---------------------------------------------------------------------------

void TFileMap::WriteWCState(short _WCState)
{
    if (Map)
        Map->WCState=_WCState;
}

//---------------------------------------------------------------------------

short TFileMap::ReadDebugState()
{
    return (Map) ? Map->DebugState : (short)0;
}

//---------------------------------------------------------------------------

void TFileMap::WriteDebugState(short _DebugState)
{
    if (Map)
        Map->DebugState = _DebugState;
}

//---------------------------------------------------------------------------

void TFileMap::SetDebugState(short Bit)
{
  if ((Bit & DebugState) == 0)
        DebugState = DebugState | Bit;
}

//---------------------------------------------------------------------------

void TFileMap::ClearDebugState(short Bit)
{
    if ((Bit & DebugState) == Bit)
        DebugState = DebugState ^ Bit;
}

//---------------------------------------------------------------------------

bool TFileMap::CheckDebugState(short Bit)
{
    return Bit & DebugState;
}

//---------------------------------------------------------------------------

void TFileMap::WriteValidatorOutState(AnsiString _Src)
{
    if (Map)
        strcpy(Map->ValidatorOutState,(_Src.SubString(0,255)).c_str());
}

//---------------------------------------------------------------------------

void TFileMap::WritePrinterOutState(AnsiString _Src)
{
    if (Map)
        strcpy(Map->PrinterOutState,(_Src.SubString(0,255)).c_str());
}

//---------------------------------------------------------------------------

void TFileMap::WriteCoinAcceptorOutState(AnsiString _Src)
{
    if (Map)
        strcpy(Map->PrinterOutState,(_Src.SubString(0,255)).c_str());
}

//---------------------------------------------------------------------------

void TFileMap::WriteCIDFileName(AnsiString _Src)
{
    if (Map)
        strcpy(Map->CIDFileName,(_Src.SubString(0,255)).c_str());
}

//---------------------------------------------------------------------------

void TFileMap::SetWCState(short Bit)
{
    if (((Bit & WCState) && CheckDebugState(Bit)) == 0)
        WCState = WCState | Bit;
}

//---------------------------------------------------------------------------

void TFileMap::ClearWCState(short Bit)
{
    if ((Bit & WCState) == Bit)
        WCState = WCState ^ Bit;
}

//---------------------------------------------------------------------------

bool TFileMap::CheckWCState(short Bit)
{
    return (Bit & WCState) && (!CheckDebugState(Bit));
}

#pragma package(smart_init)


