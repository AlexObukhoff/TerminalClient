//---------------------------------------------------------------------------

#include <Classes.hpp>
#include <ExtCtrls.hpp>
#pragma hdrstop

#include "TModemThread.h"
#include "EMailSender.h"
#include "PacketSender.h"
#include "CSPacketSender.h"
#include "SSPacketSender.h"
#include "SMSSender.h"
#include <globals.h>
#include "boost/format.hpp"
#include "MemoryLeaks.h"

#pragma package(smart_init)

//---------------------------------------------------------------------------

__fastcall TModemThread::TModemThread(TWConfig *pCfg, CWatchDog* pWatchDog, TFileMap *pFileMap)
    : TThread(true)
{
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    Finished = false;
    FreeOnTerminate = false;
    m_timeMark = TDateTime::CurrentDateTime();

    m_pCfg = pCfg;
    m_pWatchDog = pWatchDog;
    m_pFileMap = pFileMap;

    m_pLog = new TLogClass("Modem");
    m_pOutbound = new TSMSOutbound(m_pCfg, m_pLog, m_pFileMap);
    m_pModem = new TModemClass(m_pLog, m_pCfg, m_pWatchDog, m_pFileMap);

    m_pLog->Write((boost::format("Modem Interval ms= %1%") % (m_pCfg->Peripherals.Modem.Interval*1000)).str().c_str());
}

__fastcall TModemThread::~TModemThread()
{
    m_pLog->Write("Turning off modem...");
    delete m_pModem;
    m_pLog->Append("done");

    delete m_pOutbound;
    delete m_pLog;
}

//---------------------------------------------------------------------------
void TModemThread::processSMS()
{
    // если отправка смс настроенна, то отправляем
    if(m_pCfg->SMSInfo.PhoneNumber != "")
    {
        if(m_pOutbound->FilesCount() > 0)
        {
            m_pLog->Write("processing sms in outbound folder");
            m_pOutbound->Process();
        }
    }
}

void TModemThread::processInfo()
{
    if(m_pCfg->Peripherals.Modem.GetServiceInfoInterval > 0)
    {
        m_pLog->Write("processing balance and signal level");
        CModemSiemensMC35i* modem = new CModemSiemensMC35i(m_pCfg->Peripherals.Modem.Port, 0);
        float signal = modem->SignalQuality();

        //
        m_timeMark = TDateTime::CurrentDateTime();
        //
        
        double balance = modem->GetBalance(m_pCfg->Peripherals.Modem.ServiceNumber.c_str()) * 100.0;

        m_pLog->Write((boost::format("signal: %1%, balance: %2%") % signal % balance).str().c_str());

        m_pFileMap->GSMSignalQuality = (int)signal;
        m_pFileMap->SIMBalance = (int)balance;

        delete modem;
    }
}

//---------------------------------------------------------------------------
const int THREAD_SLEEPTIME = 100;   // время сна потока (миллисекунды)
const int MS_IN_HOUR = 3600000;     // миллисекунд в часе
const int MS_IN_MINUTE = 60000;     // миллисекунд в минуте
const int MS_IN_SECOND = 1000;      // миллисекунд в секунде

void __fastcall TModemThread::Execute()
{
    Finished = false;
    
    if(m_pCfg->Peripherals.Modem.AutoDial != 1 || m_pCfg->Peripherals.Modem.Port == 0)
    {
        m_pLog->Write("TModemThread::Execute() - nothing ToDo: autoDial = 0 or modem not configured");
    }
    else
    {
        if(m_pFileMap)
            m_pFileMap->FirstConnected = true;
        m_pLog->Write("TModemThread::Execute() - start");

        bool canSendSMS = m_pCfg->SMSInfo.PhoneNumber != "";
        if(!canSendSMS)
            m_pLog->Write("SMS sender omited - no phonenumber specified");

        // первая проверка баланса и отправки смс через час
        int smsTimeout = MS_IN_HOUR / THREAD_SLEEPTIME;
        int infoTimeout = MS_IN_HOUR / THREAD_SLEEPTIME;
        // первая проверка связи немедленно
        int onlineTimeout = 0;

        // и понеслась
        while(!Terminated)
        {
            m_timeMark = TDateTime::CurrentDateTime();

            // быстрая отправка смс
            if(m_message != "")
            {
                TSMSSender* sms = new TSMSSender("", m_pCfg, m_pLog, m_pFileMap);
                sms->SendMessage(m_message);
                m_message = "";
                delete sms;
            }

            // проверям время операций для которых надо убивать соединение
            if((smsTimeout == 0 && canSendSMS) || infoTimeout == 0)
            {
                // сбрасываем соединение
                bool isWasConnected = m_pModem->isConnected();
                if(isWasConnected)
                    m_pModem->hangUp();

                if(smsTimeout == 0)
                {
                    processSMS();
                    smsTimeout = m_pCfg->SMSInfo.Interval * MS_IN_MINUTE / THREAD_SLEEPTIME;
                }
                if(infoTimeout == 0)
                {
                    processInfo();
                    infoTimeout = m_pCfg->Peripherals.Modem.GetServiceInfoInterval * MS_IN_MINUTE / THREAD_SLEEPTIME;
                }

                // поднимаем соединение
                if(isWasConnected)
                    m_pModem->dial();
            }

            // проверяем есть у нас вообще соединение
            if(onlineTimeout == 0)
            {
                m_pModem->checkConnection();
                onlineTimeout = m_pCfg->Peripherals.Modem.Interval * MS_IN_SECOND / THREAD_SLEEPTIME;
            }

            smsTimeout--;
            infoTimeout--;
            onlineTimeout--;

            Sleep(THREAD_SLEEPTIME);
        }
    }

    Finished = true;
}
//---------------------------------------------------------------------------

TDateTime TModemThread::ReadThreadTimeMark()
{
    return m_timeMark;
}

void TModemThread::WriteThreadTimeMark(TDateTime timeMark)
{
    m_timeMark = timeMark;
}

//---------------------------------------------------------------------------

AnsiString TModemThread::ReadMessage()
{
    return m_message;
}

void TModemThread::WriteMessage(AnsiString msg)
{
    m_message = msg;
}

//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
/*
void TModemThread::SendNotification(float _SignalQuality, float _Balance)
{
    TPacketSender* packetSender = 0;
    TEMailSender* mailSender = 0;

    try
    {
        try
        {
            if (Cfg->StatInfo.ProcessorType==cnCyberPlatServer)
                packetSender = new TCSPacketSender("", Cfg, Log, FileMap);
            else
                packetSender = new TSSPacketSender("", Cfg, Log, FileMap);

            packetSender->StoreError(TDateTime::CurrentDateTime(), "Modem Info", int(_SignalQuality), AnsiString(_Balance),0,"");
            mailSender = new TEMailSender("", Cfg, Log, FileMap);
            mailSender->StoreMessage(cnETMdmMsg, TDateTime::CurrentDateTime(),("Terminal #"+Cfg->Terminal.Number+" Modem info message: Balance: ").c_str()+AnsiString(_Balance)+", Signal quality: "+AnsiString(_SignalQuality)+".", "");
        }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        }
    }
    __finally
    {
        delete packetSender;
        delete mailSender;
    }
}
*/
