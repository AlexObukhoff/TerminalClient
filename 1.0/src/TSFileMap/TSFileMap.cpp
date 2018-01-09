//---------------------------------------------------------------------------


#pragma hdrstop

#include "TSFileMap.h"

//---------------------------------------------------------------------------

TSFileMap::TSFileMap(TLogClass *_Log)
{
try
    {
    InnerLog=false;
    Connected=false;
    Map=NULL;
    if (_Log==NULL) {
        Log = new TLogClass("TSFileMap");
        InnerLog=true;
        }
        else
        Log=_Log;
//    Log->Write("TFileMap initialized...");
    }
catch (Exception &exception)
    {
    Log->WriteInLine("Exception in TFileMap.TFileMap: "+exception.Message);
    Connected=false;
    }
}

//---------------------------------------------------------------------------

void TSFileMap::Create(AnsiString FileMapName)
{
try
    {
    Connected=false;
    hFileMapping = NULL;
    hFileMapping = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, 100, FileMapName.c_str());
    if (hFileMapping==NULL) {
        ShowError("File-mapping object creating error");
        return;
        }
        else {
        lpFileMap = NULL;
        lpFileMap = MapViewOfFile(hFileMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
        if (lpFileMap==NULL) {
            ShowError("Mapping view of the file-mapping object error");
            return;
            }
        }
    Map = (TStrFileMap*)lpFileMap;
    Connected=true;
    Log->Write("File mapping connected...");
    }
catch (Exception &exception)
    {
    Map=NULL;
    Log->WriteInLine("Exception in TFileMap.Create: "+exception.Message);
    Connected=false;
    }
}
//---------------------------------------------------------------------------

void TSFileMap::Open(AnsiString FileMapName)
{
try
    {
    Connected=false;
    hFileMapping = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, FileMapName.c_str());
    if (hFileMapping==NULL) {
        ShowError("File-mapping object '"+FileMapName+"' opening error");
        return;
        }
        else {
        lpFileMap = MapViewOfFile(hFileMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
        if (hFileMapping==NULL) {
            ShowError("Mapping view of the file-mapping object error");
            return;
            }
        }
    Map = (TStrFileMap*)lpFileMap;
    Connected=true;
    Log->Write("File mapping connected...");
    }
catch (Exception &exception)
    {
    Map=NULL;
    Log->WriteInLine("Exception in TFileMap.Open: "+exception.Message);
    Connected=false;
    }
}

//---------------------------------------------------------------------------

void TSFileMap::Close()
{
try
    {
    if (lpFileMap)
        if (!UnmapViewOfFile(lpFileMap))
             ShowError("Unmapping view of the file-mapping object error");
    if (hFileMapping)
        if (!CloseHandle(hFileMapping))
             ShowError("File-mapping object closing error");
    Map=NULL;
    Connected=false;
    }
catch (Exception &exception)
    {
    Map=NULL;
    Log->WriteInLine("Exception in TFileMap.Close: "+exception.Message);
    Connected=false;
    }
}

//---------------------------------------------------------------------------

TSFileMap::~TSFileMap()
{
if (InnerLog)
    delete Log;
Close();
//Log->Write("TFileMap done.");
}

//---------------------------------------------------------------------------

void TSFileMap::ShowError(AnsiString Header)
{
LPVOID lpMsgBuf;
try
    {
    try
        {
        int ErrorCode = GetLastError();
        if (ErrorCode!=0) {
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,NULL,ErrorCode,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0,NULL);
            Log->Write(Header+": "+AnsiString(ErrorCode)+" "+AnsiString((char*)lpMsgBuf));
            }
        }
    catch (Exception &exception)
        {
        Log->WriteInLine("Exception in TFileMap.ShowError: "+exception.Message);
        }
    }
__finally
    {
    LocalFree(lpMsgBuf);
    }
}

//---------------------------------------------------------------------------

TDateTime TSFileMap::ReadProgramTimeMark()
{
if (Map)
    return Map->ProgramDateTime;
    else
    return 0;
}

//---------------------------------------------------------------------------

void TSFileMap::WriteProgramTimeMark(TDateTime DT)
{
//time_t tim = time(NULL);
if (Map)
    Map->ProgramDateTime=DT;
}

//---------------------------------------------------------------------------
/*
    ULONG ReadProgramTimeMark();
    void WriteProgramTimeMark(ULONG);
    bool ReadClosedNormally();
    void WriteClosedNormally(bool);
    bool ReadClosedNormally();
    void WriteClosedNormally(bool);
    bool ReadProgramIsIdle();
    void WriteProgramIsIdle(bool);
*/

//---------------------------------------------------------------------------

TDateTime TSFileMap::GetCurrentTime()
{
TDateTime DT;
return DT.CurrentDateTime();
//time_t tim = time(NULL);
//return tim;
}

//---------------------------------------------------------------------------

bool TSFileMap::ReadClosedNormally()
{
if (Map)
    return Map->ClosedNormally;
    else
    return 0;
}

//---------------------------------------------------------------------------

void TSFileMap::WriteClosedNormally(bool _ClosedNormally)
{
if (Map)
    Map->ClosedNormally=_ClosedNormally;
}

//---------------------------------------------------------------------------

bool TSFileMap::ReadProgramIsIdle()
{
if (Map)
    return Map->ProgramIsIdle;
    else
    return 0;
}

//---------------------------------------------------------------------------

void TSFileMap::WriteProgramIsIdle(bool _ProgramIsIdle)
{
if (Map)
    Map->ProgramIsIdle=_ProgramIsIdle;
}

#pragma package(smart_init)
