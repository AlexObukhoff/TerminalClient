#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#ifndef DeviceClassH
#include "ComPortParameters.h"
#pragma package(smart_init)

TComPortInitParameters::TComPortInitParameters(TCriticalSection* CS)
{
  COMParametersCriticalSection = CS;
  BaudRate = CBR_9600;
  PortNumber = 1;
  StopBits = ONESTOPBIT;
  Parity = NOPARITY;
  ByteSize = 8;
  fParity = false;
  //22-02-2007
  //DtrControl = DTR_CONTROL_HANDSHAKE;
  DtrControl = DTR_CONTROL_ENABLE;
  RtsControl  = RTS_CONTROL_ENABLE;
  timeout = 200;
  //Xon = 0;
  //Xoff = 0;
  //fNull = 0;
}

TComPortInitParameters::~TComPortInitParameters()
{
}

AnsiString TComPortInitParameters::GetPortNumberString()
{
  return PortNumberString;
}

void TComPortInitParameters::SetPortNumber(BYTE value)
{
  PortNumber = value;
  char Port[5];
  itoa(PortNumber,(char*)Port,10);
  //PortNumberString = "\\.\COM";
  PortNumberString = "\\\\.\\COM";
  PortNumberString += Port;
}

void TComPortInitParameters::SetParameters(BYTE PortNumber,DWORD BaudRate,BYTE StopBits,BYTE Parity,BYTE ByteSize,DWORD fParity,DWORD DtrControl,DWORD timeout)
{
  COMParametersCriticalSection->Acquire();
  try
  {
    SetPortNumber(PortNumber);
    this->BaudRate = BaudRate;
    this->StopBits = StopBits;
    this->Parity = Parity;
    this->ByteSize = ByteSize;
    this->fParity = fParity;
    this->DtrControl = DtrControl;
    this->timeout = timeout;
  }
  __finally
  {
    COMParametersCriticalSection->Release();
  }
}

void TComPortInitParameters::GetParameters(BYTE& PortNumber,DWORD& BaudRate,BYTE& StopBits,BYTE& Parity,BYTE& ByteSize,DWORD& fParity,DWORD& DtrControl,DWORD& timeout)
{
  COMParametersCriticalSection->Acquire();
  try
  {
    PortNumber = this->PortNumber;
    BaudRate = this->BaudRate;
    StopBits = this->StopBits;
    Parity = this->Parity;
    ByteSize = this->ByteSize;
    fParity = this->fParity;
    DtrControl = this->DtrControl;
    timeout = this->timeout;
  }
  __finally
  {
    COMParametersCriticalSection->Release();
  }
}

#endif
