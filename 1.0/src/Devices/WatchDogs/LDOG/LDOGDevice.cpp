
#pragma hdrstop

#include "LDOGdevice.h"
#include "LDOGThread.h"
#include "globals.h"
#include "boost/format.hpp"

#pragma package(smart_init)

TLDOGdevice::TLDOGdevice(int ComPort,TLogClass* _Log) : TLDOGDeviceClass(ComPort,_Log)
{
  BeginByte = 0x90;
  EndByte   = 0x0D;
  CRCLength = 0;
  DataLengthIndex = -1;
  LoggingErrors = false;
  DeviceName = "LDOG";
}

TLDOGdevice::~TLDOGdevice()
{
  try
  {
    StopTimer();
    Sleep(500);
    StopPooling();
    Log->Write("~TLDOGdevice()");
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TLDOGdevice::StartTimer()
{
  if (!Port->PortInit)
  {
      Log->Write("Port init error! Can't start device");
      return;
  }
  if (DeviceThread == NULL)
    StartDevice();
}

void TLDOGdevice::StartDevice()
{
  try
  {
    if (!GlobalStop)
      return;
    ClearAnswer();
    Stop();
    //создаём поток
    Start();
    if (DeviceThread)
    {
      DeviceThread->PollingMode = true;
      DeviceThread->OnlyPnP = OnlyPnP;
    }
    //запускаем поток
    Resume();
  }
  __finally
  {
  }
}

void TLDOGdevice::ClearGSM()
{
    SetExtCommand((BYTE)WD2_CLEARGSM);
}

void TLDOGdevice::ResetPC()
{
    SetExtCommand((BYTE)WD2_RESET);
}

void TLDOGdevice::GetSensors()
{
    SetExtCommand((BYTE)WD2_GETSENSORS);
}

bool TLDOGdevice::IsItYou()
{
    bool result = false;
    if (DeviceThread)
    {
        if (((TLDOGDeviceThread*)DeviceThread)->version == "1111")
            result = true;
    }
    return result;
}

clock_t TLDOGdevice::GetCreationTime() //метка времени создания потока
{
    if (DeviceThread)
        return DeviceThread->CreationTime;
    else
        return -1;
}

