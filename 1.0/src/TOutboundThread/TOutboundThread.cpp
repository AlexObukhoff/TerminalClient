//---------------------------------------------------------------------------
#include <vcl.h>
#include <algorith.h>
#pragma hdrstop
#include "TOutboundThread.h"
#include "globals.h"
#include "boost/format.hpp"
#pragma package(smart_init)
//---------------------------------------------------------------------------
//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall TSendThread::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------
__fastcall TOutboundThread::TOutboundThread(int _DirType, TWConfig *_Cfg, TLogClass *_Log, TFileMap *_FileMap,TXMLInfo* infoFile)
: TThread(true)
{
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    Command = -1;
    Finished = false;
    CS = new TCriticalSection();
    DirType = _DirType;
    FileMap = _FileMap;
    m_infoFile = infoFile;

    InnerLog = false;
    if(_Log == NULL)
        InnerLog = true;
    else
    {
        InnerLog = false;
        Log = _Log;
    }

    Cfg =_Cfg;
    Outbound = NULL;
    UnprocessedFilesCount = 0;
    DTLastFileProcessed = 0;
    switch (DirType)
    {
        case cnPaymentsOut:
            if (InnerLog)
                Log = new TLogClass("Conn-PaymentsThread");
            Outbound = new TPaymentsOutbound(Cfg, Log, FileMap, infoFile);
            Interval = 60000;  // 1 minute
            break;
        case cnStatOut:
            if (InnerLog)
                Log = new TLogClass("Conn-StatThread");
            if (Cfg->GetStatServerHost().LowerCase() != "none")
                    Outbound = new TStatisticsOutbound(Cfg, Log, FileMap);
            Interval = 300000;  // 5 minutes
//            Interval = 60000;  // 5 minutes
            break;
        case cnEMailOut:
            if (InnerLog)
                Log = new TLogClass("Conn-EMailThread");
            if (AnsiString(Cfg->EMailInfo.SMailHost.c_str()).LowerCase() != "none")
                Outbound = new TEMailOutbound(Cfg, Log, FileMap);
            Interval = Cfg->EMailInfo.SendInterval*60000;
            break;
        case cnCommandsIn:
            if (InnerLog)
                Log = new TLogClass("Conn-CommandsThread");
            Outbound = new TCommandsInbound(Cfg, Log, FileMap);
            Interval = 15000;  // 15 seconds
            break;
    }

    IndyError = false;
    Log->Write((boost::format("Outbound thread initialized, interval = %1% s.") % (Interval / 1000)).str().c_str());
    TimeMark=TDateTime::CurrentDateTime();
}
//---------------------------------------------------------------------------
__fastcall TOutboundThread::~TOutboundThread(void)
{
    Log->Write("~TOutboundThread started...");
    if(Outbound)
    {
        Log->Append("Outbound...");
        delete Outbound;
        Outbound = NULL;
    }

    if(CS)
    {
        delete CS;
        CS = NULL;
    }

    if(InnerLog)
        delete Log;
}
//---------------------------------------------------------------------------
void __fastcall TOutboundThread::Execute()
{
    try
    {
        int StartTimeOut = 1200;
        if(FileMap)
        {
            while((!FileMap->FirstConnected) && (--StartTimeOut > 0) && (!Terminated))
            {
                Sleep(100);
                TimeMark = TDateTime::CurrentDateTime();
            }
        }

        while(!Terminated)
        {
            TimeMark = TDateTime::CurrentDateTime();
            if (!((DirType == cnStatOut) && Cfg->StatInfo.Inhibit))
                Process();
            else
                Process(true);

            if(IndyError && !Outbound->IndyError)
                Log->Write("Thread: IndyError detected...");

            IndyError = Outbound->IndyError;
            if((Outbound != NULL) && (Outbound->Command != 0))
                Command = Outbound->Command;

            int i = Interval / 1000;
            while((!Terminated) && (--i > 0))
            {
                Sleep(1000);
                TimeMark = TDateTime::CurrentDateTime();
                if((DirType == cnStatOut) && FileMap->CheckStatDir)
                {
                    FileMap->CheckStatDir=false;
                    break;
                }
                if((DirType == cnPaymentsOut) && FileMap->CheckPaymentDir)
                {
                    FileMap->CheckPaymentDir = false;
                    Log->Write("CheckPaymentDir flag detected...");
                    break;
                }
            }
        }
        Finished = true;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Finished = true;
    }
}
//---------------------------------------------------------------------------
void TOutboundThread::Stop()
{
    Outbound->terminating = true;
    Finished=true;
}
//---------------------------------------------------------------------------
void TOutboundThread::Process(bool bFilesCountOnly)
{
    try
    {
        if(Outbound)
        {
            if (bFilesCountOnly)
                Outbound->FilesCount();
            else
                Outbound->Process();

            UnprocessedFilesCount = Outbound->UnprocessedFilesCount;
            DTLastFileProcessed = Outbound->DTLastFileProcessed;
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}
//---------------------------------------------------------------------------
TDateTime TOutboundThread::ReadThreadTimeMark()
{
    TDateTime DT;
    try
    {
        if(CS)
            CS->Acquire();
        DT = _TimeMark;
    }
    __finally
    {
        if (CS)
            CS->Release();
    }
    return DT;
}
//---------------------------------------------------------------------------
void TOutboundThread::WriteThreadTimeMark(TDateTime _Src)
{
    try
    {
        if(CS)
          CS->Acquire();

        Log->Write((boost::format("CS not acquired in WriteThreadTimeMark. %1%") % AnsiString(_Src).c_str()).str().c_str());
        Log->DeleteLastLine();
        _TimeMark=_Src;
    }
    __finally
    {
        if(CS)
            CS->Release();
    }
}
//---------------------------------------------------------------------------


