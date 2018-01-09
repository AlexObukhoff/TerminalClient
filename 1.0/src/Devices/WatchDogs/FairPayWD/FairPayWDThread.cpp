#include <math.h>
//---------------------------------------------------------------------------


#pragma hdrstop

#include "FairPayWDThread.h"
#include "globals.h"
#include "boost/format.hpp"

//---------------------------------------------------------------------------

#pragma package(smart_init)

#define TIMEOUT 250

__fastcall TFairPayWDDeviceThread::TFairPayWDDeviceThread() : TDeviceThread(true)
{
    FreeOnTerminate = false;
    StopOperation = true;
    ExtCommand = EXTC_Free;
    _PollingInterval = 30*1000;

    ProcInfo = new _info();

    InitDevice     = NULL;
    DeInit         = NULL;
    StartWork      = NULL;
    SetValues      = NULL;
    ResetModem     = NULL;
    WriteIdleReport= NULL;
}

__fastcall TFairPayWDDeviceThread::~TFairPayWDDeviceThread()
{
    if (DeInit)
    {
        Log->Write("DeInit()");
        DeInit();
    }
    if (ProcInfo)
        delete ProcInfo;
    Log->Write("~TFairPayWDDeviceThread()");
}

void TFairPayWDDeviceThread::PollingLoop()
{
  if (InitDevice == NULL)
  {
      Log->Write("Device not mount.");
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      ChangeDeviceState();
      return;
  }

  int result = 0;

  if (InitDevice)
  {
      ProcInfo->Status = ProcInfo->cs_Processing;
      result = InitDevice();
      Log->Write((boost::format("InitDevice() = %1%") % result).str().c_str());
      ProcInfo->Status = ProcInfo->cs_Done;
      if (result == 1)
          SetInitialized();
  }
  if ((StartWork)&&(OnlyPnP == false))
  {
      ProcInfo->Status = ProcInfo->cs_Processing;
      result = StartWork();
      Log->Write((boost::format("StartWork() = %1%") % result).str().c_str());
      ProcInfo->Status = ProcInfo->cs_Done;
  }

  if (SetValues)
  {
      ProcInfo->Status = ProcInfo->cs_Processing;
      result = SetValues(5000);
      Log->Write((boost::format("SetValues() = %1%") % result).str().c_str());
      ProcInfo->Status = ProcInfo->cs_Done;
  }

  while(!Terminated)
  {
      SetExtCommand(FP_WriteIdleReport);

      DWORD interval = _PollingInterval;
      int ticks = (int)ceill(interval/10);
      for(int i = 1; i<=ticks; i++)
      {
        Sleep(10);
        ProcessOutCommand();
        if (Terminated)
          break;
      }

      ProcessOutCommand();
  }
  Log->Write("Exit from PollingLoop().");
}

void __fastcall TFairPayWDDeviceThread::ProcessLoopCommand()
{
  PollingLoop();
}

void TFairPayWDDeviceThread::ProcessOutCommand()
{
    //обновим метку времени, типа ещё жив поток
    CreationTime = clock();
    int Command = GetExtCommand();
    int result = 0;
    switch (Command)
    {
        case FP_InitDevice:
          try
          {
              if (InitDevice)
              {
                  ProcInfo->Status = ProcInfo->cs_Processing;
                  result = InitDevice();
                  ProcInfo->Status = ProcInfo->cs_Done;
                  Log->Write((boost::format("InitDevice() = %1%") % result).str().c_str());
                  if (result == 1)
                      SetInitialized();
              }
          }
          catch(...)
          {
              ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
          }
          break;

        case FP_DeInit:
          try
          {
              if (DeInit)
              {
                  ProcInfo->Status = ProcInfo->cs_Processing;
                  DeInit();
                  Log->Write((boost::format("DeInit() = %1%") % result).str().c_str());
                  ProcInfo->Status = ProcInfo->cs_Done;
              }
          }
          catch(...)
          {
              ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
          }
          break;

        case FP_StartWork:
          try
          {
              if (StartWork)
              {
                  ProcInfo->Status = ProcInfo->cs_Processing;
                  result = StartWork();
                  Log->Write((boost::format("StartWork() = %1%") % result).str().c_str());
                  ProcInfo->Status = ProcInfo->cs_Done;
              }
          }
          catch(...)
          {
              ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
          }
          break;

        case FP_SetValues:
          try
          {
              if (SetValues)
              {
                  ProcInfo->Status = ProcInfo->cs_Processing;
                  result = SetValues(5000);
                  Log->Write((boost::format("SetValues() = %1%") % result).str().c_str());
                  ProcInfo->Status = ProcInfo->cs_Done;
              }
          }
          catch(...)
          {
              ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
          }
          break;

        case FP_ResetModem:
          try
          {
              if (ResetModem)
              {
                  ProcInfo->Status = ProcInfo->cs_Processing;
                  result = ResetModem();
                  Log->Write((boost::format("ResetModem() = %1%") % result).str().c_str());
                  ProcInfo->Status = ProcInfo->cs_Done;
              }
          }
          catch(...)
          {
              ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
          }
          break;

        case FP_WriteIdleReport:
          try
          {
              if (WriteIdleReport)
              {
                  ProcInfo->Status = ProcInfo->cs_Processing;
                  result = WriteIdleReport();
                  Log->Write((boost::format("WriteIdleReport() = %1%") % result).str().c_str());
                  ProcInfo->Status = ProcInfo->cs_Done;
              }
          }
          catch(...)
          {
              ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
          }
          break;
    }

    /*if (!DeviceState) return;

    switch (result)
    {
        case 1:
            ServerConnected = true;
            DeviceState->OutStateCode = DSE_OK;
            ChangeDeviceState();
            break;

        case 0:
            ServerConnected = false;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            ChangeDeviceState();
            break;
    }*/
}
