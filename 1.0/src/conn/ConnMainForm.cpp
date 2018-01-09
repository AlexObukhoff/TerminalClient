//---------------------------------------------------------------------------

#include <vcl.h>
#include <algorith.h>
#pragma hdrstop

#include "ConnMainForm.h"
#include "globals.h"
#include "TFileMap.h"
#include <time.h>
#include "PacketSender.h"
#include "common.h"
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <IdTCPClient.hpp>
#include "CryptLib2.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//#pragma link "SevenZipVCL"
#pragma resource "*.dfm"
TForm1 *Form1;

//---------------------------------------------------------------------------
//const double cnThreadTimeOut = double(1)/24/60/6;
const double cnThreadTimeOut = double(10)/24/60;
const double cnSendHeartBeatThreadTimeOut = double(20)/24/60;
const double cnPaymentsProcessingTimeOut = double(30)/24/60;
const double cnConnStartTimeOut = double(60)/24/60;
const double cnCommandThreadStartTime = double(1)/24/60;
std::string LastEntryURL = "";
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
		: TForm(Owner)
        , hMutexOneInstance(0)
{
    try
    {
        ReadVersion();
        CoInitializeEx(NULL, COINIT_MULTITHREADED);
        SendHeartBeatThread = NULL;
        PaymentsThread = NULL;
        CommandsThread = NULL;
        hMutexOneInstance = NULL;
        StatThread = NULL;
        Log = NULL;
        Cfg = NULL;
        FileMap = NULL;
        Log = new TLogClass("Conn2");
        Log->Write("----------------------------------------------------------------------------------------");
        Log->WriteInLine((boost::format("Version %1%") % FileVersion).str().c_str());
        if (Log)
        {
            if(AlreadyRunning())
            {
                Log->Write("Another Conn instance is already running, exiting...");
                delete Log;
                Log=NULL;
                Application->Terminate();
                return;
            }
        }

        Cfg = new TWConfig(".\\config\\config.xml", ".\\config\\operators.xml", Log);
        Cfg->SetDefaultValues();
        Cfg->ProcessConfigFile(false,true,true);
        Cfg->GetOperatorsInfo();

        //Log->Write("WorkDir: "+Cfg->Dirs->WorkDir);

        if (!Cfg->Prepared)
        {
            Log->Write("Config not initialized - exiting.");
            delete Cfg;
            Cfg=NULL;
            delete Log;
            Log=NULL;
            Application->Terminate();
            return;
        }

        FileMap = new TFileMap(Log);
        //FileMap->Create("$share$");

        if (FileMap)
        {
            FileMap->Open("$share$");
            FileMap->PutConnTimeMark();
        }
        ConnStartDT = new TDateTime(TDateTime::CurrentDateTime());

        crypt::init(Log);
        for(size_t i = 0; i < Cfg->Keys.size(); i++)
            crypt::addKeys(Cfg->Keys[i].SecKeyPath, Cfg->Keys[i].PubKeyPath, Cfg->Keys[i].SecKeyPassword, Cfg->Keys[i].PubKeySerial);

        InfoFile = new TXMLInfo(Cfg->Dirs.StatusFileName.c_str(), Log);
        //Cfg->Terminal.ChequeCounter = ::GetInt(InfoFile->Read("Program","ChequeCounter"));
        //Sleep(30000);
        PurgeLogs();
        CheckThreads();
        CheckTimeTimer->Enabled=true;
        CheckThreadsTimer->Enabled=true;

        //FileMap->incrementMoneyTransferToken = true;
        //if(FileMap && InfoFile)
        //    FileMap->moneyTransferToken = boost::lexical_cast<long>(InfoFile->Read("MoneyTransfer","token").c_str());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }

    m_timerTicksCount = 0;
}

//---------------------------------------------------------------------------


void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
    UNREFERENCED_PARAMETER(Sender);
    UNREFERENCED_PARAMETER(Action);
    try	{
		Log->Write("FormClose started.");
		CheckThreadsTimer->Enabled=false;
		CheckTimeTimer->Enabled=false;
		if(CommandsThread)
			CommandsThread->Terminate();
		if(SendHeartBeatThread)
			SendHeartBeatThread->Terminate();
		if(PaymentsThread) {
			if (PaymentsThread->Outbound)
					PaymentsThread->Outbound->terminating=true;
			PaymentsThread->Terminate();
		}
		if(StatThread) {
			if (StatThread->Outbound)
					StatThread->Outbound->terminating=true;
			StatThread->Terminate();
        }
		if(EMailThread) {
			if (EMailThread->Outbound)
					EMailThread->Outbound->terminating=true;
			EMailThread->Terminate();
		}
		Log->Write("Waiting for threads to finish...");
		bool check = true;
		while(check) {
			Sleep(100);
			check=false;
			if (CommandsThread)
				if (!CommandsThread->Finished)
					check=true;
			if (SendHeartBeatThread)
				if (!SendHeartBeatThread->Finished)
					check=true;
			if (PaymentsThread)
				if (!PaymentsThread->Finished)
					check=true;
			if (StatThread)
				if (!StatThread->Finished)
					check=true;
			if (EMailThread)
				if (!EMailThread->Finished)
					check=true;
			//check=((!CommandsThread->Finished)||(!SendHeartBeatThread->Finished)||(!PaymentsThread->Finished)||(!StatThread->Finished)||(!EMailThread->Finished));
//			TimeOut--;
		}
		if (CommandsThread) {
			Log->Write("Commands thread...");
			if (!CommandsThread->Finished) {
				Log->Append("terminating...");
				ULONG ExitCode=0;
				TerminateThread((HANDLE)CommandsThread->Handle,ExitCode);
			}
			delete CommandsThread;
			Log->Append("done.");
		}
		if (SendHeartBeatThread) {
			Log->Write("SendHeartBeat thread...");
			if (!SendHeartBeatThread->Finished) {
				Log->Append("terminating...");
				ULONG ExitCode=0;
				TerminateThread((HANDLE)SendHeartBeatThread->Handle,ExitCode);
			}
			delete SendHeartBeatThread;
			Log->Append("done.");
		}
		if (PaymentsThread) {
			Log->Write("Payments thread...");
			if (!PaymentsThread->Finished) {
				Log->Append("terminating...");
				ULONG ExitCode=0;
				TerminateThread((HANDLE)PaymentsThread->Handle,ExitCode);
			}
			delete PaymentsThread;
			Log->Append("done.");
		}
		if (StatThread) {
			Log->Write("Stat thread...");
			if (!StatThread->Finished) {
				Log->Append("terminating...");
				ULONG ExitCode=0;
				TerminateThread((HANDLE)StatThread->Handle,ExitCode);
			}
			delete StatThread;
			Log->Append("done.");
		}
		if (EMailThread) {
			Log->Write("EMail thread...");
			if(!EMailThread->Finished) {
				Log->Append("terminating...");
				ULONG ExitCode=0;
				TerminateThread((HANDLE)EMailThread->Handle,ExitCode);
			}
			delete EMailThread;
			Log->Append("done.");
		}
		if (FileMap) {
			Log->Write("FileMap...");
			delete FileMap;
			FileMap = NULL;
			Log->Append("done.");
        }
		if (hMutexOneInstance) {
			Log->Write("Mutex...");
       	    ::ReleaseMutex(hMutexOneInstance);
			CloseHandle(hMutexOneInstance);
			hMutexOneInstance = NULL;
			Log->Append("done.");
        }

        crypt::close();

		if (Cfg) {
			Log->Write("Cfg...");
			delete Cfg;
			Cfg = NULL;
			Log->Append("done.");
		}
		if(Log) {
			Log->Write("Log...");
			delete Log;
            Log=NULL;
        }
		CoUninitialize();
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void __fastcall TForm1::CheckTimeTimerTimer(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);
    try
    {
        m_timerTicksCount++;
        if(m_timerTicksCount > 60 / 5 * 60) // (60 / 5) - сколько тиков в минуту, нужен интервал 5 минут
        {
            m_timerTicksCount = 0;

            Log->Write("----====!!!!!!TIME TO WIPE OUT ALL OF THE DEAD FILES!!!!!====----");
            int iAttrib = 0;
            TSearchRec sr;
            AnsiString dirs[2] = { Cfg->Dirs.StatOutboundTemp.c_str(), Cfg->Dirs.StatOutbound.c_str() } ;


            for(int i = 0 ; i < 2; i++)
            {
                AnsiString curDir = dirs[i];
                Log->Write((boost::format("cleanup in %1%") % curDir.c_str()).str().c_str());
                if(FindFirst(curDir + "\\*.file", iAttrib, sr) == 0)
                {
                    do
                    {
                        // удалаяем файл + пакет
                        AnsiString f1 = curDir + "\\" + sr.Name;
                        AnsiString f2 = f1.SubString(0, f1.Length() - 4) + "pkt";

                        Log->Write((boost::format("deleting: %1% and %2%") % f1.c_str() % f2.c_str()).str().c_str());
                        DeleteFile(f1);
                        DeleteFile(f2);
                    }while(FindNext(sr) == 0);

                    FindClose(sr);
                }
                Log->Write("----====!!!!!!DONE AND DONE!!!!!====----");
            }
        }

        FileMap->PutConnTimeMark();
        if((TDateTime::CurrentDateTime() > FileMap->LastStartNavigate+double(1)/24/60 * 2)&&(FileMap->LastStartNavigate > FileMap->LastEndNavigate))
        {
            if(TDateTime::CurrentDateTime() > (*ConnStartDT + double(30)/24/60))
            {
                Log->Write((boost::format("WebClient interface timeout: %1%;%2%") % AnsiString(FileMap->LastStartNavigate).c_str() % AnsiString(FileMap->LastEndNavigate).c_str()).str().c_str());
/* */
                if (Cfg->Terminal.RebootAllowed) {
                    Log->Append(", trying to reboot...");
                    Reboot();
                } else {
                    Log->Append(", reboot is not allowed, restarting...");
                    RestartWebClient();
                }
/**/
            }
        }
        else
        {
            if(TDateTime::CurrentDateTime() > FileMap->WCTimeMark+double(2)/24/60)
            {
/**/
                Log->Write((boost::format("WebClient timeout: %1%, trying to restart...") % AnsiString(FileMap->WCTimeMark).c_str()).str().c_str());
                RestartWebClient();
/**/
            }
        }
        if(FileMap)
        {
            if(!IsProcessRunning(FileMap->ReadWCHandle()))
            {
/* */
                Log->Write("WebClient process is not running - restarting...");
                bool bStarted = StartWebClient();
                if(bStarted)
                {
                    Log->Write("Terminating conn...");
                    Close();
                }
/**/
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TForm1::Reboot()
{
/*  Log->Write("Reboot sequence started...");
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken); // Get a token for this process.
    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); // Get the LUID for the shutdown privilege.
    tkp.PrivilegeCount = 1;  // one privilege to set
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); // Get the shutdown privilege for this process.
    // Cannot test the return value of AdjustTokenPrivileges.
    ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0); //Shut down the system and force all applications to close.
    Log->Write("ExitWindows done...");
    Application->Terminate();
    */
}

//---------------------------------------------------------------------------

void TForm1::ShutDown()
{
    Log->Write("Shutdown sequence started...");
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken); // Get a token for this process.
    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); // Get the LUID for the shutdown privilege.
    tkp.PrivilegeCount = 1;  // one privilege to set
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); // Get the shutdown privilege for this process.
    ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0); //Shut down the system and force all applications to close.
    Log->Write("ExitWindows done...");
    Application->Terminate();
}

//---------------------------------------------------------------------------

bool TForm1::IsProgramWindowExists(AnsiString ApplicationWindowCaption)
{
    try {
        return FindWindow(NULL, (LPCTSTR)ApplicationWindowCaption.c_str());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

//---------------------------------------------------------------------------

bool TForm1::StartUpdate()
{
try
	{
	for (int i=0;i<10;i++) {
		ChDir("\\WebUpdate");
		StartProgram("\\WebUpdate\\updater.exe");
		Log->Write("Updater started. Waiting for the window to appear...");
		int TimeOut=100;
		while ((TimeOut>0)&&(!IsProgramWindowExists("WebClientUpdaterForm"))) {
			TimeOut--;
			Sleep(300);
			FileMap->PutConnTimeMark();
			}
		if (IsProgramWindowExists("WebClientUpdaterForm")) {
			Log->Write("Updater window found. Sleeping 30 sec...");
			Sleep(30000);
			if (IsProgramWindowExists("WebClientUpdaterForm")) {
				Log->Write("Updater window found. StartUpdate completed.");
				return true;
				}
			}
		}
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return false;
}

//---------------------------------------------------------------------------

void TForm1::CheckThreads()
{
    try
    {
        if (CommandsThread == NULL)
        {
            if (TDateTime::CurrentDateTime() > (*ConnStartDT + cnCommandThreadStartTime))
            {
                Log->Write("Starting CommandsThread.");
                CommandsThread = new TOutboundThread(cnCommandsIn,Cfg,NULL,FileMap,InfoFile);
                CommandsThread->Resume();
            }
        }
        else
        {
            if (CommandsThread->Finished)
            {
                Log->Write("Restarting CommandsThread.");
                CommandsThread->Resume();
            }
            if (TDateTime::CurrentDateTime()>CommandsThread->TimeMark+cnThreadTimeOut)
            {
                Log->Write("Terminating and restarting CommandsThread...");
                Log->Append((boost::format("Last thread timemark: %1%.") % AnsiString(CommandsThread->TimeMark).c_str()).str().c_str());
                ULONG ExitCode=0;
                TerminateThread((HANDLE)CommandsThread->Handle,ExitCode);
                delete CommandsThread;
                CommandsThread = NULL;
                //Sleep(5000);
                CommandsThread = new TOutboundThread(cnCommandsIn,Cfg,NULL,FileMap,InfoFile);
                CommandsThread->Resume();
                Log->Append("OK");
            }
            int Command = CommandsThread->Command;

            if ((FileMap->WCRestartReq)||(Command==cnCmdReboot)||(Command==cnCmdReceiveFileOld)||(Command==cnCmdReceiveFile)||(Command==cnHTTPFileRequest)||(Command==cnCmdShutDown)||(Command==cnCmdBlock)||(Command==cnCmdUnblock))
            {
                //Log->Write("Command : "+AnsiString(Command));
                FileMap->WriteCommand(Command);

                int i=600;  // waiting up to 60 seconds

                if (FileMap->NumDBUpdateDone)
                {
                    Log->Write("NumDB update received.");
                    return;
                }
                while ((i>0)&&(FileMap->NumDBUpdateDone))
                {
                    Sleep(100);
                    i--;
                    FileMap->PutConnTimeMark();
                    Application->ProcessMessages();
                }

                i=6000;  // waiting up to 10 minutes
                if (!FileMap->WCIdle)
                    Log->Write("Waiting for WebClient to be in Idle state to process command...");
                while ((i>0)&&(!FileMap->WCIdle))
                {
                    Sleep(100);
                    i--;
                    FileMap->PutConnTimeMark();
                    Application->ProcessMessages();
                }
                if (FileMap->WCRestartReq)
                {
                    Log->Write("WebClient restart requested...");
                    RestartWebClient();
                }
                switch (Command)
                {
                    case cnCmdReboot:
                        Reboot();
                        break;
                    case cnCmdShutDown:
                        ShutDown();
                        break;
                    case cnCmdBlock:
                        if (!FileMap->CheckWCState(cnTerminalForceBlock))
                        {
                            Log->Write("Terminal blocked.");
                        }
                        FileMap->ForceBlock=true;
                        break;
                    case cnCmdUnblock:
                        if (FileMap->CheckWCState(cnTerminalForceBlock))
                        {
                            Log->Write("Terminal unblocked.");
                        }
                        FileMap->ForceBlock=false;
                        break;
                    case cnHTTPFileRequest:
                    case cnCmdReceiveFileOld:
                    case cnCmdReceiveFile:
                        Log->Write("File received, restarting WebClient...");
                        CheckTimeTimer->Enabled=false;
                        CheckThreadsTimer->Enabled=false;
                        if ((FileExists("\\WebUpdate\\updater.exe"))&&((FileExists("\\WebUpdate\\update.exe"))||(DirectoryExists("\\WebUpdate\\update"))))
                        {
                            if (StartUpdate())
                            {
                                TerminateWebClient();
                                Close();
                                return;
                            }
                        }
                        else
                        {
                            RestartWebClient();
                        }
                        break;
                }
            }
        }
        //if ((_Cfg->GetStatServerHost().LowerCase()=="none")||(_Cfg->GetStatServerHost().LowerCase()==""))

        if ((Cfg->GetStatServerHost().LowerCase()!="none")&&(Cfg->GetStatServerHost().LowerCase()!="")&&(Cfg->GetStatServerHost().LowerCase()!="external_sender"))
        {
            if (StatThread == NULL)
            {
                Log->Write("Starting StatThread.");
                StatThread = new TOutboundThread(cnStatOut,Cfg,NULL,FileMap,InfoFile);
                StatThread->Resume();
            }
            else
            {
                FileMap->UnprocStatPacketsCount=StatThread->UnprocessedFilesCount;
                if (StatThread->Finished)
                {
                    Log->Write("Restarting StatThread.");
                    StatThread->Resume();
                }
                if (TDateTime::CurrentDateTime()>StatThread->TimeMark+cnThreadTimeOut)
                { //   /240
                    Log->Write("Terminating and restarting StatThread...");
                    Log->Append((boost::format("Last thread timemark: %1%") % AnsiString(StatThread->TimeMark).c_str()).str().c_str());
                    ULONG ExitCode=0;
                    TerminateThread((HANDLE)StatThread->Handle,ExitCode);
                    delete StatThread;
                    StatThread = NULL;
                    //Sleep(5000);
                    StatThread = new TOutboundThread(cnStatOut,Cfg,NULL,FileMap,InfoFile);
                    StatThread->Resume();
                    Log->Append("OK");
                }
            }
        }

        AnsiString SMailHost=AnsiString(Cfg->EMailInfo.SMailHost.c_str()).LowerCase();
        if
        (
            Cfg->GetStatServerHost().LowerCase() != "none"
            && Cfg->GetStatServerHost().LowerCase() != ""
            && Cfg->GetStatServerHost().LowerCase()!="external_sender"
        )
        {
            if (SendHeartBeatThread == NULL)
            {
                Log->Write("Starting SendHeartBeatThread.");
                SendHeartBeatThread = new TSendHeartBeatThread(FileMap,Cfg,Log);
                SendHeartBeatThread->Resume();
            }
            else
            {
                if (SendHeartBeatThread->Finished)
                {
                    Log->Write("Restarting SendHeartBeatThread.");
                    SendHeartBeatThread->Resume();
                }
                if (TDateTime::CurrentDateTime()>SendHeartBeatThread->TimeMark+cnSendHeartBeatThreadTimeOut)
                {
                    Log->Write("Terminating and restarting SendHeartBeatThread...");
                    Log->Append((boost::format("Last thread timemark: %1%.") % AnsiString(SendHeartBeatThread->TimeMark).c_str()).str().c_str());
                    ULONG ExitCode=0;
                    TerminateThread((HANDLE)SendHeartBeatThread->Handle,ExitCode);
                    delete SendHeartBeatThread;
                    SendHeartBeatThread = NULL;
                    //Sleep(2000);
                    SendHeartBeatThread = new TSendHeartBeatThread(FileMap,Cfg,Log);
                    SendHeartBeatThread->Resume();
                    Log->Append("OK");
                }
            }
        }

        SMailHost=AnsiString(Cfg->EMailInfo.SMailHost.c_str()).LowerCase();
        if ((SMailHost!="none")&&(SMailHost!="")&&(SMailHost!="external_sender"))
        {
            if (EMailThread == NULL)
            {
                Log->Write("Starting EMailThread.");
                EMailThread = new TOutboundThread(cnEMailOut,Cfg,NULL,FileMap,InfoFile);
                EMailThread->Resume();
            }
            else
            {
                if (EMailThread->Finished)
                {
                    Log->Write("Restarting EMailThread.");
                    EMailThread->Resume();
                }
                if (TDateTime::CurrentDateTime()>EMailThread->TimeMark+cnThreadTimeOut)
                {
                    Log->Write("Terminating and restarting EMailThread...");
                    Log->Append((boost::format("Last thread timemark: %1%.") % AnsiString(EMailThread->TimeMark).c_str()).str().c_str());
                    ULONG ExitCode=0;
                    TerminateThread((HANDLE)EMailThread->Handle,ExitCode);
                    delete EMailThread;
                    EMailThread = NULL;
                    //Sleep(2000);
                    EMailThread = new TOutboundThread(cnEMailOut,Cfg,NULL,FileMap,InfoFile);
                    EMailThread->Resume();
                    Log->Append("OK");
                }
            }
        }
        if (PaymentsThread == NULL)
        {
            Log->Write("Starting PaymentsThread.");
            PaymentsThread = new TOutboundThread(cnPaymentsOut,Cfg,NULL,FileMap,InfoFile);
            PaymentsThread->Resume();
        }
        else
        {
            FileMap->UnprocPaymentsCount=PaymentsThread->UnprocessedFilesCount;
            if (PaymentsThread->DTLastFileProcessed.Val!=0)
                FileMap->LastPaymentProcessed=PaymentsThread->DTLastFileProcessed;
            if ((TDateTime::CurrentDateTime() > (*ConnStartDT + cnConnStartTimeOut))&&(TDateTime::CurrentDateTime()>FileMap->LastPaymentReceived+cnPaymentsProcessingTimeOut)&&(FileMap->LastPaymentReceived>FileMap->LastPaymentProcessed)&&(FileMap->UnprocPaymentsCount>10))
            {
                Log->Write((boost::format("There was no payments processed since last payment was received for + %1% minutes and %2% payments are in the payment thread - terminating conn...") % (cnPaymentsProcessingTimeOut * 60 * 24) % FileMap->UnprocPaymentsCount).str().c_str());
                Close();
            }
            if ((TDateTime::CurrentDateTime() > (*ConnStartDT + cnConnStartTimeOut))&&(PaymentsThread->IndyError))
            {
                Log->Write("Indy Library error detected, terminating conn.exe...");
                Application->Terminate();
            }
            if (PaymentsThread->Finished)
            {
                Log->Write("Restarting PaymentsThread.");
                PaymentsThread->Resume();
            }
            if (TDateTime::CurrentDateTime()>PaymentsThread->TimeMark+cnThreadTimeOut)
            {
                Log->Write("Terminating and restarting PaymentsThread...");
                Log->Append((boost::format("Last thread timemark: %1%") % AnsiString(PaymentsThread->TimeMark).c_str()).str().c_str());
                ULONG ExitCode=0;
                TerminateThread((HANDLE)PaymentsThread->Handle,ExitCode);
                delete PaymentsThread;
                PaymentsThread = NULL;
                //Sleep(5000);
                PaymentsThread = new TOutboundThread(cnPaymentsOut,Cfg,NULL,FileMap,InfoFile);
                PaymentsThread->Resume();
                Log->Append("OK");
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void __fastcall TForm1::CheckThreadsTimerTimer(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);
    CheckThreads();

    if (TDateTime::CurrentTime() > TDateTime("23:58:00"))
    {
        if (TDateTime::CurrentDateTime() > FileMap->WCTimeMark+double(2)/24/60)
        {
            Log->Write((boost::format("WebClient TimeOut: %1%, trying to restart...") % AnsiString(FileMap->WCTimeMark).c_str()).str().c_str());
            RestartWebClient();
        }
        else
        {
            Log->Write("Closing...");
            Close();
        }
    }
}

//---------------------------------------------------------------------------

void TForm1::PurgeLogs()
{
    try
    {
        Log->Write("Deleting old logs and not processed payments...");
        TSearchRec sr;
        int iAttributes = 0;
        AnsiString Path = (Cfg->Dirs.WorkDir+"logs\\").c_str();
        if (FindFirst(Path+"????-??-??-*.log", iAttributes, sr) == 0)
        {
            do
            {
                AnsiString ModuleName=AnsiString(sr.Name).LowerCase();
                ModuleName=ModuleName.SubString(12,ModuleName.Length());
                ModuleName=ModuleName.SubString(0,ModuleName.Length()-4);
                //int delta = DT - FileDateToDateTime(sr.Time);
                //Log->Write(ModuleName+"  "+sr.Name+"  "+AnsiString(delta));
                if (((ModuleName=="payments")&&(Cfg->LogsDelete.Payments>0)&&(int(TDateTime::CurrentDateTime() - FileDateToDateTime(sr.Time)) > Cfg->LogsDelete.Payments))||((ModuleName=="webclient")&&(Cfg->LogsDelete.Main>0)&&(int(TDateTime::CurrentDateTime() - FileDateToDateTime(sr.Time)) > Cfg->LogsDelete.Main))||(((ModuleName!="webclient")&&(ModuleName!="payments"))&&(Cfg->LogsDelete.Other>0)&&(int(TDateTime::CurrentDateTime() - FileDateToDateTime(sr.Time)) > Cfg->LogsDelete.Other)))
                    if (DeleteFile(Path+sr.Name))
                        Log->Write((boost::format("File %1%%2% deleted.") % Path.c_str() % sr.Name.c_str()).str().c_str());
                    else
                        Log->Write((boost::format("Error deleting file %1%%2%!") % Path.c_str() % sr.Name.c_str()).str().c_str());
            }
            while (FindNext(sr) == 0);
            FindClose(sr);
        }
        else
        {
            Log->Write("No logs found.");
        }

        Path = (Cfg->Dirs.WorkDir+"outbound\\payments\\unprocessed\\").c_str();
        if (FindFirst(Path+"*.pkt", iAttributes, sr) == 0)
        {
            do
            {
                //Log->Write(Path+sr.Name+" "+FileDateToDateTime(sr.Time));
                if ((Cfg->LogsDelete.UnprocessedPayments>0)&&(int(TDateTime::CurrentDateTime() - FileDateToDateTime(sr.Time)) > Cfg->LogsDelete.UnprocessedPayments))
                    if (DeleteFile(Path+sr.Name))
                        Log->Write((boost::format("File %1%%2% deleted.") % Path.c_str() % sr.Name.c_str()).str().c_str());
                    else
                        Log->Write((boost::format("Error deleting file %1%%2%!") % Path.c_str() % sr.Name.c_str()).str().c_str());
            }
            while (FindNext(sr) == 0);
            FindClose(sr);
        }
        else
        {
            Log->Write("No payment packets found.");
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}
//---------------------------------------------------------------------------
bool TForm1::AlreadyRunning(void)
{
	hMutexOneInstance = ::CreateMutex(NULL, TRUE, "CONNAPPLICATION-088FA840-B10D-11D3-BC36-006067709674");
	return (GetLastError() == ERROR_ALREADY_EXISTS);
}

__fastcall TForm1::~TForm1()
{
        delete ConnStartDT;
}
//---------------------------------------------------------------------------
bool TForm1::IsProcessRunning(HANDLE PrHandle)
{
	ULONG ExitCode;
    if(PrHandle) {
        if(GetExitCodeProcess(PrHandle,&ExitCode))
            return ExitCode == STILL_ACTIVE;
        else
            return false;    
    } else {
        return false;
    }
}

//---------------------------------------------------------------------------

HANDLE TForm1::StartProgram(AnsiString ApplicationName)
{
try
		{
		Log->Write((boost::format("Starting %1%...") % ApplicationName.c_str()).str().c_str());
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		si.lpDesktop="WinSta0\\Default";

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
			return NULL;
			}
			else
			{
			Log->Append("OK.");
			return pi.hProcess;
//			UpdateProcessId=pi.dwProcessId;
			}
	 }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
       return NULL;
   }
}

//---------------------------------------------------------------------------

bool TForm1::TerminateProgram(HANDLE PrHandle)
{
try
	{
	Log->Write("Terminating program...");
	if (!TerminateProcess(PrHandle, NO_ERROR))
		Log->Append(ShowError("error").c_str());
		else {
    		Log->Append("OK.");
	    	return true;
		}
	return false;
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

//---------------------------------------------------------------------------

bool TForm1::StartWebClient()
{
HANDLE WCHandle = NULL;
bool bResult = false;
	try
		{
		//Log->Write("Starting WebClient...");
		for (int i=0;i<10;i++) {
			WCHandle = StartProgram((Cfg->Dirs.WorkDir+"WebClient.exe").c_str());
			if (WCHandle!=NULL)
				break;
			}
		if ((WCHandle!=NULL)&&(IsProcessRunning(WCHandle))) {
			//Log->Append("OK.");
			bResult = true;
			}
			else {
			if (Cfg->Terminal.RebootAllowed) {
				Log->Write("Can't start WebClient. Reboot allowed - trying to reboot...");
				Reboot();
				}
				else
				Log->Append("Can't start WebClient. Reboot not allowed.");
			}
		}
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
	}
    	return bResult;
}

//---------------------------------------------------------------------------

void TForm1::TerminateWebClient()
{
try
	{
	Log->Write("Terminating WebClient...");
	HANDLE WCHandle=FileMap->ReadWCHandle();
	TerminateProgram(WCHandle);
	Sleep(500);
	if (IsProcessRunning(WCHandle))
		if(Cfg->Terminal.RebootAllowed) {
			Log->Write("Can't terminate WebClient. Reboot allowed - trying to reboot...");
			Reboot();
		} else
			Log->Append("Can't terminate WebClient. Reboot not allowed.");
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TForm1::RestartWebClient()
{
try
	{
	Log->Write("Restarting WebClient...");
	TerminateWebClient();
	bool bStarted = StartWebClient();
	if (bStarted) {
		Log->Write("Terminating conn...");
		Close();
		}
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

