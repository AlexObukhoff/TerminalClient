//---------------------------------------------------------------------------

#include <vcl.h>
#include <algorith.h>
#include <Classes.hpp>
#include <ExtCtrls.hpp>
#include <ras.h>
#include <raserror.h>
#include <idicmpclient.hpp>
#pragma hdrstop

#include "TModem.h"
#include "globals.h"
#include "boost/format.hpp"

#pragma package(smart_init)
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------

TModemClass::TModemClass(TLogClass *log, TWConfig *cfg, CWatchDog *watchDog, TFileMap *fileMap)
{
    m_pCfg = cfg;
    m_pWatchDog = watchDog;
    m_pFileMap = fileMap;
    
    if(log == 0)
    {
        m_pLog = new TLogClass("Modem");
        m_isInnerLog = true;
    }
    else
    {
        m_pLog = log;
        m_isInnerLog = false;
    }

    m_pCfg->ActiveConnection = 0;
    m_linkFailures = 0;
    m_rebootFailures = 0;

    m_lastDialTime.Val = 0;

    OSVERSIONINFO osVerInfo;
    ZeroMemory(&osVerInfo, sizeof(OSVERSIONINFO));
    osVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osVerInfo);

    //m_rasConnSize = offsetof(RASCONN, szDeviceType);
    m_rasConnSize = sizeof(RASCONN);
    if(osVerInfo.dwMajorVersion >= 0x04 && osVerInfo.dwMinorVersion >= 0x00)
        m_rasConnSize = offsetof(RASCONN, szPhonebook);
    if(osVerInfo.dwMajorVersion >= 0x04 && osVerInfo.dwMinorVersion >= 0x01)
        m_rasConnSize = offsetof(RASCONN, guidEntry);
    if(osVerInfo.dwMajorVersion >= 0x05 && osVerInfo.dwMinorVersion >= 0x00)
        m_rasConnSize = offsetof(RASCONN, dwFlags);

    m_pLog->Write((boost::format("osVersion = 0x%02X%02X. RASCON size set to: %d") % osVerInfo.dwMajorVersion % osVerInfo.dwMinorVersion % m_rasConnSize).str().c_str());
}

//---------------------------------------------------------------------------

TModemClass::~TModemClass()
{
    if(m_isInnerLog)
        delete m_pLog;
}

//---------------------------------------------------------------------------
HRASCONN TModemClass::getConnection()
{
    HRASCONN result = 0;
    const int rasConnectionsCount = 32;
    RASCONN rasConnections[rasConnectionsCount];
    DWORD dwBufferSize = m_rasConnSize * rasConnectionsCount;
    DWORD dwActualConnectionsCount = 0;

    ZeroMemory(rasConnections, dwBufferSize);

    rasConnections[0].dwSize = m_rasConnSize;

    DWORD dwError = RasEnumConnections(rasConnections, &dwBufferSize, &dwActualConnectionsCount);
    if(dwError == 0)
    {
        std::string connName = m_pCfg->RASConnections[0].Name;

        for(DWORD i = 0; i < dwActualConnectionsCount; i++)
        {
            if(connName == rasConnections[i].szEntryName)
            {
                m_pLog->Write("getConnection() - connection found");
                result = rasConnections[i].hrasconn;
                break;
            }
        }

        if(result == 0)
            m_pLog->Write("connection not found");
    }
    else
    {
        m_pLog->Write((boost::format("RasEnumConnections() failed. error: %1%") % dwError).str().c_str());
        if(dwError == 1722 || 1723)
        {
            // какая-то полная херня, думаем что отваливается RPC.. пожтому только ребут
            m_pLog->Write("trying to reboot terminal due critical error");
            m_pFileMap->WriteCommand(1);
            Sleep(60000);
        }
    }

    return result;
}

//RASCONNSTATE TModemClass::getConnectionState(RASCONN rasConn)
RASCONNSTATE TModemClass::getConnectionState(HRASCONN hRasConn)
{
    RASCONNSTATE result = RASCS_Disconnected;
    RASCONNSTATUS status;

    ZeroMemory(&status, sizeof(RASCONNSTATUS));
    status.dwSize = sizeof(RASCONNSTATUS);
    if(hRasConn != 0)
    {
        DWORD stateError = RasGetConnectStatus(hRasConn, &status);

        if(stateError == 0)
        {
            result = status.rasconnstate;
            m_pLog->Write((boost::format("connection status: %1%") % getStatusDescription(result)).str().c_str());
        }
        else
        {
            m_pLog->Write((boost::format("RasGetConnectionStatus() failed. error: %1%") % stateError).str().c_str());
        }
    }
    else
    {
        m_pLog->Write("connection status unavailable - hRasConn == 0");
    }

    return result;
}

bool TModemClass::isConnected()
{
    return (getConnectionState(getConnection()) == RASCS_Connected);
    /*
    bool result = false;
    if(getConnectionState(getConnection()) == RASCS_Connected)
        result = true;

    return result;
    */
}

void TModemClass::hangUp()
{
    m_pLog->Write("HangUp");
    HRASCONN hRasConn = getConnection();
    
    if(getConnectionState(hRasConn) != 0)
        hangUpThis(hRasConn);
    else
        m_pLog->Write("nothing todo: connection not available");
}

void TModemClass::hangUpThis(HRASCONN hRasConn)
{
    if(hRasConn == 0)
    {
        m_pLog->Write("hangUpThis() - nothing todo - no connection available");
        return;
    }
    
    DWORD hangUpResult = RasHangUp(hRasConn);
    m_pLog->Write((boost::format("RasHangUp() - result: %1%") % hangUpResult).str().c_str());

    RASCONNSTATUS rasStatus;
    ZeroMemory(&rasStatus, sizeof(RASCONNSTATUS));
    rasStatus.dwSize = sizeof(RASCONNSTATUS);

    while(RasGetConnectStatus(hRasConn, &rasStatus) != ERROR_INVALID_HANDLE)
    {
        Sleep(10);
        // просто так оно почему-то не хочет умирать...
        // приходится повторять для особо тугодумных
        RasHangUp(hRasConn);
    }
    m_pLog->Write("hangUp complete");
}

void TModemClass::dial()
{
    m_pLog->Write("dial");

    if(m_pCfg->Connection().Name == "")
    {
        m_pLog->Write("cant dial - connection name not specified");
        return;
    }

    // проверим, не соединенны ли мы
    m_pLog->Write("before dial - check for active connections");
    HRASCONN hRasConn = getConnection();
    if(getConnectionState(hRasConn) == RASCS_Connected)
        hangUpThis(hRasConn);

    m_pLog->Write("continue dial");
    // получаем параметры
    RASDIALPARAMS rasDialParams;
    rasDialParams.dwSize = sizeof(RASDIALPARAMS);
    strcpy(rasDialParams.szEntryName, m_pCfg->Connection().Name.c_str());
    int pass;

    if(RasGetEntryDialParams(0, &rasDialParams, &pass) == 0)
    {
        // звоним
        HRASCONN hrc = 0;
        DWORD dialError = 0;
        dialError = RasDial(0, 0, &rasDialParams, 0, 0, &hrc);
        if(dialError != 0)
        {
            m_pLog->Write((boost::format("RasDial() failed. error = %1%") % dialError).str().c_str());
            // попробуем повесить трубку для соединения
            // иногда отваливаемся по 756 ошибке - соединение уже в процессе установки
            hangUpThis(hrc);
            processFailure();
        }
        else
        {
            m_pLog->Write("dial successfull");
            m_lastDialTime = TDateTime::CurrentDateTime();
            m_pFileMap->LastConnectionDT = 0;
            m_pFileMap->LastSuccessfullConnectionDT = 0;
            m_pFileMap->LastCPConnectionDT = 0;
            m_pFileMap->LastSuccessfullCPConnectionDT = 0;
        }
    }
    else
    {
        m_pLog->Write((boost::format("connection not found: %1%") % m_pCfg->Connection().Name.c_str()).str().c_str());
    }
}
//---------------------------------------------------------------------------

void TModemClass::checkConnection()
{
    if(m_pCfg->Peripherals.Modem.AutoDial != 1)
        return;

    HRASCONN hRasConn = getConnection();
    if(getConnectionState(hRasConn) == RASCS_Connected)
    {
        if(isPingOK())
        {
            // нужно ли обрывать?
            if(m_pCfg->Connection().DisconnectTime != 0)
            {
                if((double)TDateTime::CurrentDateTime() - (double)m_lastDialTime > double(m_pCfg->Connection().DisconnectTime) / 24 / 60)
                {
                    m_pLog->Write("time to disconnect");
                    hangUp();
                }
            }
        }
        else
        {
            processFailure();
        }
    }
    else
    {
        dial();
    }
}

bool TModemClass::isPingOK()
{
    bool result = true;

    if(m_pCfg->Peripherals.Modem.Hosts[0] == "" && m_pCfg->Peripherals.Modem.Hosts[1] == "")
    {
        TDateTime cpDiffDT;
        cpDiffDT = m_pCfg->Peripherals.Modem.ConnectionCheckTimeOutDT;

        if(TDateTime::CurrentDateTime() - m_pFileMap->LastCPConnectionDT < cpDiffDT)
        {
            m_pLog->Write("CP timeout");
            result = false;
        }
        else
        {
            TDateTime monitoringDiffDT;
            monitoringDiffDT = max(cpDiffDT.Val,2.5*float(m_pCfg->StatInfo.StatServerPollInterval)/24/60);
            if(m_pFileMap->LastConnectionDT - m_pFileMap->LastSuccessfullConnectionDT > monitoringDiffDT)
            {
                m_pLog->Write("monitoring timeout");
                result = false;
            }
        }

        m_pLog->Write("timeout's OK");
    }
    else
    {
        std::auto_ptr <TIdIcmpClient> IdPing ( new TIdIcmpClient(Application) );
        for (int i=0; i<2; i++)
        {
            IdPing->Host = m_pCfg->Peripherals.Modem.Hosts[i];
            m_pLog->Write((boost::format("ping: %1%") % IdPing->Host.c_str()).str().c_str());
            try
            {
                IdPing->Ping();
                if(IdPing->ReplyStatus.ReplyStatusType == rsEcho)
                {
                    m_pLog->Write((boost::format("Ok. %1% ms.") % AnsiString(IdPing->ReplyStatus.MsRoundTripTime).c_str()).str().c_str());
                    break;
                }
                else
                {
                    switch (IdPing->ReplyStatus.ReplyStatusType)
                    {
                        case rsError:
                            m_pLog->Write("can't ping: rsError");
                            break;
                        case rsTimeOut:
                            m_pLog->Write("rsTimeout");
                            break;
                        case rsErrorUnreachable:
                            m_pLog->Write("rsErrorUnreachable");
                            break;
                        case rsErrorTTLExceeded:
                            m_pLog->Write("rsErrorTTLExceeded");
                            break;
                    }
                }
            }
            catch(...)
            {
                result = false;
                ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
            }
        }
    }
    
    return result;
}

void TModemClass::processFailure()
{
    m_pLog->Write((boost::format("processFailure() - linkFailures = %1%, rebootFailures = %2%") % m_linkFailures % m_rebootFailures).str().c_str());

    if(++m_linkFailures > m_pCfg->Peripherals.Modem.FailuresBeforeModemReset)
    {
        m_linkFailures = 0;

        // надо ребутнуть модем
        if(m_pWatchDog)
        {
            m_pLog->Write("link failed. reseting modem power via watchdog");
            m_pWatchDog->ClearGSM();
            Sleep(30000);
        }
        else
        {
            // если не можем - ребутим терминал
            if(++m_rebootFailures > m_pCfg->Peripherals.Modem.FailuresBeforeTerminalReboot)
            {
                m_rebootFailures = 0;
                m_pLog->Write("trying to reboot terminal");
                if(m_pCfg->Terminal.RebootAllowed)
                {
                    m_pFileMap->WriteCommand(1);
                    Sleep(60000);                    }
                else
                {
                    m_pLog->Write("terminal reboot not allowed");
                }
            }
        }
    }
}

std::string TModemClass::getStatusDescription(RASCONNSTATE rasconnstate)
{
    std::string result = "";

    switch (rasconnstate)
    {
    case RASCS_OpenPort:
        result = "The communication port is about to be opened.";
        break;
    case RASCS_PortOpened:
        result = "The communication port has been opened successfully.";
        break;
    case RASCS_ConnectDevice:
        result = "A device is about to be connected.";
        break;
    case RASCS_DeviceConnected:
        result = "A device has connected successfully.";
        break;
    case RASCS_AllDevicesConnected:
        result = "All devices in the device chain have successfully connected.";
        break;
    case RASCS_Authenticate:
        result = "The authentication process is starting.";
        break;
    case RASCS_AuthNotify:
        result = "An authentication event has occurred.";
        break;
    case RASCS_AuthRetry:
        result = "The client has requested another validation attempt with a new user name/password/domain.";
        break;
    case RASCS_AuthCallback:
        result = "The remote access server has requested a callback number.";
        break;
    case RASCS_AuthChangePassword:
        result = "The client has requested to change the password on the account.";
        break;
    case RASCS_AuthProject:
        result = "The projection phase is starting.";
        break;
    case RASCS_AuthLinkSpeed:
        result = "The link-speed calculation phase is starting.";
        break;
    case RASCS_AuthAck:
        result = "An authentication request is being acknowledged.";
        break;
    case RASCS_ReAuthenticate:
        result = "Reauthentication (after callback) is starting.";
        break;
    case RASCS_Authenticated:
        result = "The client has successfully completed authentication.";
        break;
    case RASCS_PrepareForCallback:
        result = "The line is about to disconnect in preparation for callback.";
        break;
    case RASCS_WaitForModemReset:
        result = "The client is delaying in order to give the modem time to reset itself in preparation for callback.";
        break;
    case RASCS_WaitForCallback:
        result = "The client is waiting for an incoming call from the remote access server.";
        break;
    case RASCS_Projected:
        result = "This state occurs after the RASCS_AuthProject state. It indicates that projection result information is available. You can access the projection result information by calling RasGetProjectionInfo.";
        break;
    case RASCS_SubEntryConnected:
        result = "When dialing a multilink phone-book entry, this state indicates that a subentry has been connected during the dialing process. The dwSubEntry parameter of a RasDialFunc2 callback function indicates the index of the subentry.";
        //When the final state of all subentries in the phone-book entry has been determined, the connection state is RASCS_Connected if one or more subentries have been connected successfully.";
        break;
    case RASCS_SubEntryDisconnected:
        result = "When dialing a multilink phone-book entry, this state indicates that a subentry has been disconnected during the dialing process. The dwSubEntry parameter of a RasDialFunc2 callback function indicates the index of the subentry.";
        break;
    case RASCS_Connected:
        result = "Connected.";
        break;
    case RASCS_Disconnected:
        result = "Disconnected or failed.";
        break;
    default:
        result = (boost::format("Unknown connection state: %1%") % rasconnstate).str();
        break;
    }
    return result;
}
