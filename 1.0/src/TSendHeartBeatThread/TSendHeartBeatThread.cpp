//---------------------------------------------------------------------------

#include <vcl.h>
#include <algorith.h>
#pragma hdrstop

#include "TSendHeartBeatThread.h"
#include "globals.h"
#include <boost/format.hpp>
#pragma package(smart_init)
//---------------------------------------------------------------------------
//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall TSendHeartBeatThread::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

__fastcall TSendHeartBeatThread::TSendHeartBeatThread(TFileMap *fileMap, TWConfig *cfg, TLogClass *log)
    : TThread(true)
{
    try
    {
        CoInitializeEx(NULL, COINIT_MULTITHREADED);
        Finished=false;

        m_pFileMap = fileMap;
        m_pCfg = cfg;

        if(log == 0)
        {
            m_pLog = new TLogClass("TSendHeartBeatThread");
            m_isInnerLog = true;
        }
        else
        {
            m_pLog = log;
            m_isInnerLog = false;
        }

        if(cfg->StatInfo.ProcessorType == cnCyberPlatServer)
        {
            Interval = max(15, cfg->StatInfo.StatServerPollInterval) * 60000;
            PS = new TCSPacketSender("", cfg, log, fileMap);
        }
        else
        {
            Interval = max(1, cfg->StatInfo.StatServerPollInterval) * 60000;
            PS = new TSSPacketSender("", cfg, log, fileMap);
        }

        EMailSender = new TEMailSender("", cfg, log, fileMap);
        CS = new TCriticalSection();
        m_timeMark = TDateTime::CurrentDateTime();

        log->Write((boost::format("SendHeartBeat thread initialized, interval: %1% s.") % (Interval/1000)).str().c_str());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, log);
    }
}

//---------------------------------------------------------------------------

__fastcall TSendHeartBeatThread::~TSendHeartBeatThread(void)
{
    try
    {
        if (PS != 0)
        {
            PS->Terminated=true;
            Sleep(50);
            delete PS;
        }

        if (EMailSender != 0)
        {
            delete EMailSender;
            EMailSender=NULL;
        }

        if (CS != 0)
            delete CS;

        if (m_isInnerLog)
            delete m_pLog;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
    }
}

//---------------------------------------------------------------------------

void __fastcall TSendHeartBeatThread::Execute()
{
    bool bFirstTry = false;
    try
    {
        m_pLog->WriteInLine("TSendHeartBeatThread::Execute Started.");
        /**/
        if(m_pFileMap)
        {
            int startTimeOut = 1200;
            while(--startTimeOut > 0)
            {
                bool ex1 =     (!m_pFileMap->FirstConnected)
                            || (m_pFileMap->WCState == -1)
                            || (m_pFileMap->BillsCount == -1)
                            || (m_pFileMap->BillsSum == -1);
                bool ex2 =      m_pCfg->Peripherals.Modem.AutoDial
                            && (m_pCfg->Peripherals.Modem.GetServiceInfoInterval > 0)
                            && (m_pCfg->Peripherals.Modem.Port != 0);
                bool ex3 =  (m_pFileMap->SIMBalance == -1) || (m_pFileMap->GSMSignalQuality == -1);
                if(! (ex1 || (ex2 && ex3)))
                    break;

                Sleep(100);
                m_timeMark = TDateTime::CurrentDateTime();
            }
        }
        /**/
        while (!Terminated)
        {
            m_timeMark=TDateTime::CurrentDateTime();
            if((m_pCfg->GetStatServerHost().LowerCase() != "none") && (m_pCfg->GetStatServerHost().LowerCase() != ""))
            {
                if (m_pCfg->GetStatServerHost().LowerCase()!="external_sender")
                {
                    m_pFileMap->MonSrvrConnectOK = PS->SendHeartBeat(m_pCfg->InternalVersion, m_pFileMap->WCState, m_pFileMap->BillsCount, m_pFileMap->BillsSum, m_pFileMap->SIMBalance, m_pFileMap->GSMSignalQuality, bFirstTry, m_pFileMap->ChequeCounter/*m_pCfg->Terminal.ChequeCounter*/);
                }
            }

            if((AnsiString(m_pCfg->EMailInfo.SMailHost.c_str()).LowerCase() != "none") && (m_pCfg->EMailInfo.SMailHost != ""))
            {
                AnsiString temp = AnsiString(TDateTime::CurrentDateTime()) + "\r\n";
                temp += "Кол-во купюр: " + AnsiString(m_pFileMap->BillsCount) + "\r\n";
                temp += "Сумма: " + AnsiString(m_pFileMap->BillsSum) + "\r\n";
                temp += "Статус: " + AnsiString(m_pFileMap->WCState) + "\r\n";
                temp += "Купюропр.: " + AnsiString(m_pFileMap->ValidatorOutState) + "\r\n";
                temp += "Принтер: " + AnsiString(m_pFileMap->PrinterOutState) + "\r\n";
                temp += "Посл. принятый: " + AnsiString(m_pFileMap->LastPaymentReceived) + "\r\n";
                temp += "Посл. проведенный: " + AnsiString(m_pFileMap->LastPaymentProcessed) + "\r\n";
                temp += "Платежей в очереди: " + AnsiString(m_pFileMap->UnprocPaymentsCount) + "\r\n";
                temp += "Пакетов в очереди: " + AnsiString(m_pFileMap->UnprocStatPacketsCount) + "\r\n";
                m_pLog->Write(temp.c_str());

                if((m_pCfg->EMailInfo.Ext) || (AnsiString(m_pCfg->EMailInfo.SMailHost.c_str()).LowerCase() == "external_sender"))
                    EMailSender->StoreHeartBeatMessage(TDateTime::CurrentDateTime(),"HeartBeat", temp);
            }

            int i = Interval / 100;
            while(!Terminated && (i-- > 0))
            {
                m_timeMark = TDateTime::CurrentDateTime();
                Sleep(100);
            }
        }
        PS->Terminated = true;
        Sleep(50);
        Finished = true;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
    }
}

//---------------------------------------------------------------------------

TDateTime TSendHeartBeatThread::ReadThreadTimeMark()
{
    TDateTime DT;
    CS->Acquire();
    m_pLog->Write((boost::format("CS not acquired in ReadThreadTimeMark. %1%") % AnsiString(m_timeMark).c_str()).str().c_str());
    try
    {
        m_pLog->DeleteLastLine();
        DT = m_timeMark;
    }
    __finally
    {
        CS->Release();
    }
    return DT;
}

//---------------------------------------------------------------------------

void TSendHeartBeatThread::WriteThreadTimeMark(TDateTime _Src)
{
    m_pLog->Write((boost::format("CS not acquired in WriteThreadTimeMark. %1%") % AnsiString(_Src).c_str()).str().c_str());
    CS->Acquire();
    try
    {
        m_pLog->DeleteLastLine();
        m_timeMark=_Src;
    }
    __finally
    {
        CS->Release();
    }
}

//---------------------------------------------------------------------------


