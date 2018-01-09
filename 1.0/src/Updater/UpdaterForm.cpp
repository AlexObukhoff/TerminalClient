//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "UpdaterForm.h"
#include <boost/format.hpp>
#include "globals.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
    : TForm(Owner)
{
    ReadVersion();
    UpdateProcessId=NULL;
    Log = NULL;
    Log = new TLogClass("Updater");
    Log->Write("----------------------------------------------------------------------------------------");
    Log->WriteInLine((boost::format("Version %1%") % FileVersion).str().c_str());
    UpdateStarted=false;
    Log->Write("WebClient updater started.");
    Path = GetPath();
    Drive = GetDrive();
    PrRoot = GetPrRoot();
}

//---------------------------------------------------------------------------

bool TForm1::IsProcessRunning(DWORD ProcessId)
{
    UNREFERENCED_PARAMETER(ProcessId);
    try
    {
        ULONG ExitCode;
        if (!GetExitCodeProcess(UpdateHandle,&ExitCode))
        {
        //Log->Write(ShowError("GetExitCodeProcess error"));
        }
        else
        {
        //Log->Write("ExitCode = "+AnsiString(ExitCode));
            if (ExitCode!=STILL_ACTIVE)
              return false;
        }
        return true;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

//---------------------------------------------------------------------------

bool TForm1::IsProgramWindowExists(AnsiString ApplicationWindowCaption)
{
    try
    {
        return FindWindow(NULL, (LPCTSTR)ApplicationWindowCaption.c_str());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

//---------------------------------------------------------------------------

bool TForm1::StartUpdate(void)
{
    if (FileExists(PrRoot+"WebUpdate\\update.exe"))
    {
        Log->Write("Starting update...");
        if (!BackUp())
        {
            Log->Write("Can't store backup files - aborting update.");
            return false;
        }
        ChDir(PrRoot+"WebClient");
        StartProgram(PrRoot+"WebUpdate\\update.exe");
        ChDir(PrRoot+"WebUpdate");
        DTStartUpdate=DTStartUpdate.CurrentDateTime();
        return true;
    }
    else
    {
        Log->Write((boost::format("Can't start update - %1% WebUpdate\\update.exe not found") % PrRoot.c_str()).str().c_str());
    }
    return false;
}

//---------------------------------------------------------------------------

bool TForm1::FinishUpdate(void)
{
    if (StartWebClient())
    {
        return true;
    }
    else
    {
        Log->Write("Can't start new WebClient - rolling back...");
        RollBack();
        if (StartWebClient())
            return true;
        else
            Log->Write("Can't start old WebClient.");
    }
    return false;
}
//---------------------------------------------------------------------------

bool TForm1::StartWebClient(void)
{
    int TimeOut;
    for (int i=0;i<1;i++)
    {
        ChDir(PrRoot+"WebClient");
        if (StartProgram(PrRoot+"WebClient\\WebClient.exe"))
        {
            Log->Write("WebClient started. Waiting for the window to appear...");
            TimeOut=100;
            while ((TimeOut>0)&&(!IsProgramWindowExists("WebClientForm")))
            {
                TimeOut--;
                Application->ProcessMessages();
                Sleep(300);
            }
            if (IsProgramWindowExists("WebClientForm"))
            {
                Log->Write("WebClient window found. Sleeping 30 sec...");
                TimeOut=100;
                while (TimeOut>0)
                {
                    TimeOut--;
                    Application->ProcessMessages();
                    Sleep(300);
                }
                if (IsProgramWindowExists("WebClientForm"))
                {
                    Log->Write("WebClient window found. FinishUpdate completed.");
                    return true;
                }
            }
            TerminateProcess(UpdateHandle, NO_ERROR);
        }
    }
    return false;
}


//---------------------------------------------------------------------------

bool TForm1::BackUp(void)
{
    bool bResult=false;
    bResult=CopyDir(PrRoot+"WebClient",PrRoot+"WebUpdate\\BackUp");
//DeleteDir("c:\\WebUpdate\\Backup");
    if (DirectoryExists(PrRoot+"WebUpdate\\BackUp\\outbound"))
        if (!DeleteDir(PrRoot+"WebUpdate\\BackUp\\outbound"))
            Log->Append(" Error deleting outbound dir.");
    if (DirectoryExists(PrRoot+"WebUpdate\\BackUp\\inbound"))
        if (!DeleteDir(PrRoot+"WebUpdate\\BackUp\\inbound"))
            Log->Append(" Error deleting inbound dir.");
    if (DirectoryExists(PrRoot+"WebUpdate\\BackUp\\logs"))
        if (!DeleteDir(PrRoot+"WebUpdate\\BackUp\\logs"))
            Log->Append(" Error deleting logs dir.");
    if (FileExists(PrRoot+"WebUpdate\\BackUp\\config\\Details.xml"))
        if (!DeleteFile(PrRoot+"WebUpdate\\BackUp\\config\\Details.xml"))
            Log->Write("Error deleting Details.xml.");
        else
            Log->Write("Details.xml deleted.");
    return bResult;
}

//---------------------------------------------------------------------------

bool TForm1::CopyDir(AnsiString aFrom, AnsiString aTo, bool bClearDir)
{
    try
    {
        TDateTime DT;
        if ((bClearDir)&&(DirectoryExists(aTo)))
        {
            if (!DeleteDir(aTo))
            {
                Log->Append((boost::format(" Error deleting directory %1%") % aTo.c_str()).str().c_str());
                return false;
            }
        }
        if (!ForceDirectories(aTo))
        {
            Log->Append((boost::format(" Error creating %1% directory") % aTo.c_str()).str().c_str());
            return false;
        }

        Log->Write((boost::format("Copying dir %1% to %2%") % aFrom.c_str() % aTo.c_str()).str().c_str());
        aFrom+="\\*.*";
        char From[MAX_PATH];
        ZeroMemory(From, sizeof(From));
        strcat(From,aFrom.c_str());
        strcat(From,"\0\0");

        char To[MAX_PATH];
        ZeroMemory(To, sizeof(To));
        strcat(To,aTo.c_str());
        strcat(To,"\0\0");

        SHFILEOPSTRUCT op;
        ZeroMemory(&op, sizeof(op));
        op.hwnd = Handle;
        op.wFunc = FO_COPY;
        op.pFrom = From;
        op.pTo = To;
        op.fFlags = FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_NOERRORUI|FOF_SILENT;
        if (!SHFileOperation( &op))
        {
            Log->Append(" OK.");
            return true;
        }
        else
        {
            Log->Append(ShowError(" Error copying files: ").c_str());
            return false;
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

//---------------------------------------------------------------------------

bool TForm1::RollBack(void)
{
    if (DirectoryExists(PrRoot+"WebClient\\outbound"))
      CopyDir(PrRoot+"WebClient\\outbound", PrRoot+"WebUpdate\\BackUp\\outbound");
    if (DirectoryExists(PrRoot+"WebClient\\inbound"))
      CopyDir(PrRoot+"WebClient\\inbound", PrRoot+"WebUpdate\\BackUp\\inbound");
    if (DirectoryExists(PrRoot+"WebClient\\logs"))
      CopyDir(PrRoot+"WebClient\\logs", PrRoot+"WebUpdate\\BackUp\\logs");
    if (FileExists(PrRoot+"WebClient\\config\\Details.xml"))
      if (!CopyFile((PrRoot+"WebClient\\config\\Details.xml").c_str(), (PrRoot+"WebUpdate\\BackUp\\config\\Details.xml").c_str(), false))
          Log->Write("Error copying Details.xml.");
      else
          Log->Write("Details.xml copied.");
    return CopyDir(PrRoot+"WebUpdate\\BackUp",PrRoot+"WebClient");
}

//---------------------------------------------------------------------------

bool TForm1::DeleteDir(AnsiString DirName)
{
    Log->Write((boost::format("Deleting %1%") % DirName.c_str()).str().c_str());
    char From[MAX_PATH];
    ZeroMemory(From, sizeof(From));
    strcat(From,DirName.c_str());
    strcat(From,"\0\0");
    SHFILEOPSTRUCT op;
    ZeroMemory(&op, sizeof(op));
    op.hwnd = Handle;
    op.wFunc = FO_DELETE;
    op.pFrom = From;
    op.fFlags = FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_NOERRORUI|FOF_SILENT;
    if (!SHFileOperation(&op))
    {
        Log->Append(" OK.");
        return true;
    }
    else {
        Log->Append(ShowError(" Error! ").c_str());
        return false;
    }
}

//---------------------------------------------------------------------------

bool TForm1::StartProgram(AnsiString ApplicationName)
{
    try
    {
        Log->Write((boost::format("Starting %1%") % ApplicationName.c_str()).str().c_str());
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.lpDesktop="WinSta0\\Default";

 //       DWORD lpExitCode;
        if (!CreateProcess((LPCTSTR)ApplicationName.c_str(), NULL,
                NULL,             // Process handle not inheritable.
                NULL,             // Thread handle not inheritable.
                true,            // Set handle inheritance to FALSE.
                0,//CREATE_NEW_CONSOLE|CREATE_DEFAULT_ERROR_MODE|CREATE_NEW_PROCESS_GROUP,                // No creation flags.
                NULL,             // Use parent's environment block.
                NULL,             // Use parent's starting directory.
                &si,              // Pointer to STARTUPINFO structure.
                &pi))
        {
            Log->Append(ShowError("Starting program error").c_str());
            return false;
        }
        else
        {
            Log->Append("OK.");
            UpdateHandle=pi.hProcess;
            UpdateProcessId=pi.dwProcessId;
            return true;
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

//---------------------------------------------------------------------------

void TForm1::TerminateUpdateProgram()
{
    try
    {
        Log->Write("Terminating program...");
        if (!TerminateProcess(UpdateHandle, NO_ERROR))
            Log->Append(ShowError("error").c_str());
        else
            Log->Append("OK.");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

AnsiString TForm1::ShowError(AnsiString Header)
{
    LPVOID lpMsgBuf;
    AnsiString Temp;
    try
    {
        try
        {
            int ErrorCode = GetLastError();
            if (ErrorCode!=0)
            {
                FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,NULL,ErrorCode,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0,NULL);
                Temp=Header+": "+AnsiString(ErrorCode)+" "+AnsiString((char*)lpMsgBuf);
            }
        }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        }
    }
    __finally
    {
        LocalFree(lpMsgBuf);
        return Temp;
    }
}

void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
    UNREFERENCED_PARAMETER(Sender);
    UNREFERENCED_PARAMETER(Action);
    Log->Write("WebClient updater finished.");
}

//---------------------------------------------------------------------------

void __fastcall TForm1::FormDestroy(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);
    if (Log!=NULL)
    {
        delete Log;
        Log=NULL;
    }
}

//---------------------------------------------------------------------------

AnsiString TForm1::GetPath(void)
{
    char path_buf[_MAX_PATH];
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];

    ::GetModuleFileName(NULL,path_buf,sizeof path_buf);
    _splitpath(path_buf,drive,dir,fname,0);
    _makepath (path_buf,drive,dir,0,0);
    return AnsiString(path_buf);
}

//---------------------------------------------------------------------------

AnsiString TForm1::GetDrive(void)
{
    char path_buf[_MAX_PATH];
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];

    ::GetModuleFileName(NULL,path_buf,sizeof path_buf);
    _splitpath(path_buf,drive,dir,fname,0);
    _makepath (path_buf,drive,0,0,0);
    return AnsiString(path_buf);
}

//---------------------------------------------------------------------------

AnsiString TForm1::GetPrRoot(void)
{
    char path_buf[_MAX_PATH];
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];

    ::GetModuleFileName(NULL,path_buf,sizeof path_buf);
    _splitpath(path_buf,drive,dir,fname,0);
    _makepath (path_buf,drive,dir,0,0);
    AnsiString Temp = AnsiString(path_buf);
    Temp=Temp.SubString(0,Temp.Pos("WebUpdate\\")-1);
    return Temp;
}

void __fastcall TForm1::StartTimerTimer(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);
    StartTimer->Enabled=false;
    TDateTime DT;

    while (IsProgramWindowExists("WebClientForm"))
    {
        Sleep(1000);
        Application->ProcessMessages();
    }

/*while (IsProgramWindowExists("ConnMainForm"))
	{
	Sleep(1000);
	Application->ProcessMessages();
	}*/

    if (FileExists(PrRoot+"WebUpdate\\update.exe"))
    {
        Log->Write((boost::format("Using %1%WebUpdate\\update.exe...") % PrRoot.c_str()).str().c_str());
        if (StartUpdate())
        {
            Log->Write("Waiting for update to finish...");
            while ((DTStartUpdate+double(10)/24/60>=DT.CurrentDateTime())&&(IsProcessRunning(UpdateProcessId)))
            {
                Sleep(1000);
                Application->ProcessMessages();
            }

            if (IsProcessRunning(UpdateProcessId))
            {
                Log->Write("Update timeout.");
                TerminateUpdateProgram();
            }
            else
            {
                Log->Write("Update finished.");
            }
        }
        else
        {
            if (StartWebClient())
            {
                Close();
                return;
            }
            else
            {
                Log->Write("StartUpdate failed, unable to start WebClient, freezing...");
                while (true)
                {
                    Sleep(1000);
                    Application->ProcessMessages();
                }
            }
        }
        Sleep(5000);
    }
    else
    {
        Log->Write((boost::format("Using %1%WebUpdate\\update\\*.*...") % PrRoot.c_str()).str().c_str());
        if (!BackUp())
        {
            Log->Write("Can't store backup files - aborting update.");
        }
        else
        {
            if (!CopyDir(PrRoot+"WebUpdate\\Update",PrRoot+"WebClient",false))          // Копируем файлы из каталога \webupdate\update
            {                                                                         // в \webclient
/*			if (StartWebClient())
				{
				Close();
				return;
				}
				else {
				Log->Write("Copy Update failed, unable to start WebClient, freezing...");
				while (true)
					{
					Sleep(1000);
					Application->ProcessMessages();
					}
}*/
            }
        }
    }

    if (FileExists(PrRoot+"WebUpdate\\update.exe"))
        if (DeleteFile (PrRoot+"WebUpdate\\update.exe"))
            Log->Write((boost::format("%1%WebUpdate\\update.exe deleted.") % PrRoot.c_str()).str().c_str());
    else
    {
        Log->Write((boost::format("Can't delete %1%WebUpdate\\update.exe file.") % PrRoot.c_str()).str().c_str());
        if (FileExists(PrRoot+"WebUpdate\\update.exe.old"))
            if (DeleteFile (PrRoot+"WebUpdate\\update.exe.old"))
                if (RenameFile(PrRoot+"WebUpdate\\update.exe",PrRoot+"WebUpdate\\update.exe.old"))
                    Log->Write((boost::format("%1%WebUpdate\\update.exe renamed to %1%WebUpdate\\update.exe.old") % PrRoot.c_str()).str().c_str());
                else
                {
                    Log->Write((boost::format("Can't rename %1%WebUpdate\\update.exe to %1%WebUpdate\\update.exe.old.") % PrRoot.c_str()).str().c_str());
                }
    }

    if (DirectoryExists(PrRoot+"WebUpdate\\update"))
        if (DeleteDir(PrRoot+"WebUpdate\\update"))
            Log->Write((boost::format("%1%WebUpdate\\update directory deleted.") % PrRoot.c_str()).str().c_str());
        else
            Log->Write((boost::format("Error deleting %1%WebUpdate\\update directory!") % PrRoot.c_str()).str().c_str());

    if (!FinishUpdate())
    {
        Log->Write("FinishUpdate failed, unable to start WebClient, freezing...");
        while (true)
        {
            Sleep(1000);
            Application->ProcessMessages();
        }
    }
    Close();
    Application->Terminate();
}
//---------------------------------------------------------------------------

