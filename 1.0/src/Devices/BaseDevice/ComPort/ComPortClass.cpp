#include <vcl.h>
#include <stdio.h>
#include <sstream>
#pragma hdrstop

#include "ComPortClass.h"
#include "boost/format.hpp"

#pragma package(smart_init)

TComPort::TComPort(TComPortInitParameters* _COMParameters,TLogClass* Log, bool logging, PortType::Enum portType)
{
    UNREFERENCED_PARAMETER(logging);
    this->Log = Log;
    COMParameters = _COMParameters;
    PortInit = false;
    Port = NULL;
    m_portType = portType;
    Init();
}

TComPort::~TComPort()
{
  Release();
}

bool TComPort::Init()
{
  PortInit = false;
  LPVOID lpMsgBuf = NULL;
      try
      {
            std::string fileName;
            if(m_portType == PortType::lpt)
            {
                Log->Write("!!!LPT!!! все последующие записи COM следует читать как LPT");
                std::stringstream ss;
                ss << "LPT" << (int)COMParameters->PortNumber;
                fileName = ss.str();
            }
            else
            {
                fileName = COMParameters->GetPortNumberString().c_str();
            }

             Port = CreateFile(/*COMParameters->GetPortNumberString().c_str()*/fileName.c_str(),
                                GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);


              // если ошибка - выходим
              if (Port == INVALID_HANDLE_VALUE)
              {
                Port = NULL;
                FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    GetLastError(),
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL
                );
                if (Log != NULL)
                  Log->Write((boost::format("COM port open error! #%1% Error:%2%") % COMParameters->GetPortNumberString().c_str() % ((char*)lpMsgBuf)).str().c_str());
                LocalFree(lpMsgBuf);
                Port = NULL;
                return false;
              }

              // для LPT больше ничего не делаем
              if(m_portType == PortType::lpt)
              {
                  PortInit = true;
                  return true;
              }
              // получаем состояние порта
              if ( !GetCommState(Port, &dcb) )
              {
                FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    GetLastError(),
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL
                );
                if (Log != NULL)
                  Log->Write((boost::format("Get port status error! %1%") % ((char*)lpMsgBuf)).str().c_str());
                LocalFree(lpMsgBuf);
                CloseHandle(Port);
                Port = NULL;
                return false;
              }
              // заполняем структуру для инициализации порта
              dcb.DCBlength = sizeof(DCB);

              dcb.BaudRate = (DWORD)COMParameters->BaudRate;
              dcb.StopBits = (BYTE)COMParameters->StopBits;
              dcb.Parity = (BYTE)COMParameters->Parity;
              dcb.ByteSize = (BYTE)COMParameters->ByteSize;
              dcb.fParity = (DWORD)COMParameters->fParity;

              if (Log != NULL)
                Log->Write((boost::format("Port speed = %1%")  % COMParameters->BaudRate).str().c_str());

              //new 22-02-2007
              dcb.fDtrControl = (DWORD)COMParameters->DtrControl;
              dcb.fRtsControl = (DWORD)COMParameters->RtsControl;
              //dcb.fRtsControl = RTS_CONTROL_ENABLE;
              //dcb.fDtrControl = DTR_CONTROL_ENABLE;

              dcb.fBinary = true;
              dcb.fOutxCtsFlow=false;
              dcb.fOutxDsrFlow=false;
              dcb.fDsrSensitivity=false;
              dcb.fTXContinueOnXoff=false;
              dcb.fOutX=false;
              dcb.fInX=false;
              dcb.fNull=false;
              dcb.fAbortOnError=false;

              if ( !SetCommState(Port, &dcb) )
              {
                FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    GetLastError(),
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL
                );
                if (Log != NULL)
                  Log->Write((boost::format("Error writing port settings! %1%") % ((char*)lpMsgBuf)).str().c_str());
                LocalFree(lpMsgBuf);
                CloseHandle(Port);
                Port = NULL;
                return false;
              }

              //get current timeouts
              GetCommTimeouts(Port, &to);
              to.ReadIntervalTimeout = MAXDWORD;
              //to.ReadIntervalTimeout = 10;
              to.ReadTotalTimeoutMultiplier = 0;
              to.ReadTotalTimeoutConstant = 0;
              to.WriteTotalTimeoutConstant = 0;
              to.WriteTotalTimeoutMultiplier = 0;

              if ( !SetCommTimeouts(Port, &to) )
              {
                FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    GetLastError(),
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL
                );
                if (Log != NULL)
                  Log->Write((boost::format("Error writing port timeouts! %1%") % ((char*)lpMsgBuf)).str().c_str());
                LocalFree(lpMsgBuf);
                CloseHandle(Port);
                Port = NULL;
                return false;
              }

              // очищаем порт
              if (!ClearCOMPort())
                return false;

              timeout =  COMParameters->timeout;
              if (!SetupComm(Port,256,256))
              {
                FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    GetLastError(),
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL
                );
                if (Log != NULL)
                  Log->Write((boost::format("Error port buffer setup! %1%") % ((char*)lpMsgBuf)).str().c_str());
                LocalFree(lpMsgBuf);
                CloseHandle(Port);
                Port = NULL;
                return false;
              }

              if (!ClearCommBreak(Port))
              {
                FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    GetLastError(),
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL
                );
                if (Log != NULL)
                  Log->Write((boost::format("ClearCommBreak Error: %1%") % ((char*)lpMsgBuf)).str().c_str());
                LocalFree(lpMsgBuf);
                CloseHandle(Port);
                Port = NULL;
                return false;
              }

              PortInit = true;
              Log->Write((boost::format("Open port %1%") % COMParameters->GetPortNumberString().c_str()).str().c_str());
              return true;
      }
      __finally
      {
      }

  return true;
}

bool TComPort::Release()
{
  PortInit = false;
  if ((Port != NULL)&&(Port != INVALID_HANDLE_VALUE))
    CloseHandle(Port);
  Port = NULL;
  if (Log != NULL)
    Log->Write((boost::format("Release port %1%") % COMParameters->GetPortNumberString().c_str()).str().c_str());
  return true;
}

bool TComPort::ClearCOMPort()
{
    if(m_portType == PortType::lpt)
        return true;
        
  //if ( PurgeComm(Port,PURGE_TXCLEAR) && PurgeComm(Port,PURGE_RXCLEAR) )
  if ( PurgeComm(Port,PURGE_TXCLEAR || PURGE_RXCLEAR))// || PURGE_TXABORT || PURGE_RXABORT) )
    return true;
  else
  {
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0,
        NULL
    );
    if (Log != NULL)
      Log->Write((boost::format("Port cleanup error! %1%") %((char*)lpMsgBuf)).str().c_str());
    LocalFree(lpMsgBuf);
    Release();
    return false;
  }
}

int TComPort::DSR_CTS()
{
    if(m_portType == PortType::lpt)
        return -1;
        
    ModemStat = 0;
    if (!GetCommModemStatus(Port,&ModemStat))
    {
        LPVOID lpMsgBuf;
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0,
            NULL
        );
        if (Log != NULL)
          Log->Write((boost::format("GetCommModemStatus(): Ошибка! %1%") % ((char*)lpMsgBuf)).str().c_str());
        LocalFree(lpMsgBuf);
        Release();
        return DsrCtsOff;
    }
    if ((ModemStat&MS_CTS_ON)&&(ModemStat&MS_DSR_ON))
        return DsrCtsOn;
    if (ModemStat&MS_CTS_ON)
        return CtsOn;
    if (ModemStat&MS_DSR_ON)
        return DsrOn;
    return DsrCtsOff;
}

void TComPort::ReopenPort()
{
    Release();
    Init();
}

