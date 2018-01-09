#pragma hdrstop
#include "ExpressPaydevice.h"
#pragma package(smart_init)
#include "ExpressPayThread.h"
#include "globals.h"
#include "boost/format.hpp"

TExpressPaydevice::TExpressPaydevice(int ComPort,TLogClass* _Log) : TExpressPayDeviceClass(ComPort,_Log)
{
  DataLength = 4;
  LoggingErrors = false;
  Port->timeout = 250;
  DeviceName = "ExpressPay";
}

TExpressPaydevice::~TExpressPaydevice()
{
  try
  {
    StopTimer();
    Sleep(500);
    StopPooling();
    Log->Write("~TExpressPaydevice()");
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
}

void TExpressPaydevice::StartTimer()
{
  if (!Port->PortInit)
  {
      Log->Write("COM port init error! Can't start device");
      return;
  }
  if (DeviceThread == NULL)
    StartDevice();
}

void TExpressPaydevice::StartDevice()
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
      DeviceThread->_PollingInterval = 2*1000;
      DeviceThread->OnlyPnP = OnlyPnP;
    }
    //запускаем поток
    Resume();
  }
  __finally
  {
  }
}

void TExpressPaydevice::SetCommand(BYTE command)
{
  if (DeviceThread == NULL)
    return;
  if (((TExpressPayDeviceThread*)DeviceThread)->ExtCommand != EXTC_Free)
    for(int i = 1; i<=1000; i++)
    {
      Sleep(10);
      Application->ProcessMessages();
      if (((TExpressPayDeviceThread*)DeviceThread)->ExtCommand == EXTC_Free)
        break;
    }
  DeviceThread->SetCommand(command);
}

void TExpressPaydevice::ClearGSM()
{
    SetExtCommand((BYTE)CLEARGSM);
}

void TExpressPaydevice::ResetPC()
{
    SetExtCommand((BYTE)RESET);
}

void TExpressPaydevice::StopTimer()
{
    SetExtCommand((BYTE)STOP);
}

bool TExpressPaydevice::IsItYou()
{
/*
    bool result = false;
    if (DeviceThread)
      ((TExpressPayDeviceThread*)DeviceThread)->Version = "";
    SetExtCommand((BYTE)VERSION);
    Sleep(500);
    if (DeviceThread)
    {
        AnsiString version = ((TExpressPayDeviceThread*)DeviceThread)->Version.UpperCase();
        if (version.Pos("WDT V2.00") > 0)
            result = true;
    }
    */
    return true;
}

clock_t TExpressPaydevice::GetCreationTime() //метка времени создания потока
{
    if (DeviceThread)
        return DeviceThread->CreationTime;
    else
        return -1;
}

