#include <math.h>
//---------------------------------------------------------------------------


#pragma hdrstop

#include "SBK2Thread.h"
#include "globals.h"
#include "boost/format.hpp"

//---------------------------------------------------------------------------

#pragma package(smart_init)

#define TIMEOUT 250

#define CorrectResult 0

__fastcall TSBK2DeviceThread::TSBK2DeviceThread() : TDeviceThread(true)
{
    FreeOnTerminate = false;
    StopOperation = true;
    ExtCommand = EXTC_Free;
    _PollingInterval = 20*1000;

    ProcInfo = new _info();

    WDTGetDoorSwitch = NULL;
    WDTSetTimer = NULL;
    WDTClearTimer = NULL;
    WDTStopTimer = NULL;
    WDTResetModem = NULL;
    WDTResetComputer = NULL;

    Sensor1 = Sensor2 = Sensor3 = 0;
}

__fastcall TSBK2DeviceThread::~TSBK2DeviceThread()
{
    if (WDTStopTimer)
    {
        Log->Write("WDTStopTimer()");
        WDTStopTimer();
    }
    if (ProcInfo)
        delete ProcInfo;
    Log->Write("~TSBK2DeviceThread()");
}

void TSBK2DeviceThread::PollingLoop()
{
  if (WDTSetTimer == NULL)
  {
      Log->Write("Device not mount.");
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      ChangeDeviceState();
      return;
  }

  int result = 0;

  if (WDTSetTimer)
  {
      ProcInfo->Status = ProcInfo->cs_Processing;
      result = WDTSetTimer(60*3);
      Log->Write((boost::format("WDTSetTimer() = %1%") % result).str().c_str());
      ProcInfo->Status = ProcInfo->cs_Done;
      if (result == CorrectResult)
      {
          SetInitialized();
          DeviceState->OutStateCode = DSE_OK;
          ChangeDeviceState();
      }
  }

  while(!Terminated)
  {
      SetExtCommand(EC_WDTClearTimer);

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

void __fastcall TSBK2DeviceThread::ProcessLoopCommand()
{
  PollingLoop();
}

void TSBK2DeviceThread::CheckDoorSwitch(int value)
{
    //cheking sensor1
    int sensor = value & 0x01;
    if(sensor > 0)
    {
        if (Sensor1 == 0)
        {
            Log->Write("Sensor1 is ON");
            DeviceState->OutStateCode = DSE_SENSOR_1_ON;
            Sensor1 = 1;
            ChangeDeviceState();
            return;
        }
    }
    else
    {
        if (Sensor1 == 1)
        {
            Log->Write("Sensor1 is OFF");
            DeviceState->OutStateCode = DSE_SENSOR_1_OFF;
            Sensor1 = 0;
            ChangeDeviceState();
            return;
        }
    }

    //cheking sensor2
    sensor = value & 0x02;
    if(sensor > 0)
    {
        if (Sensor2 == 0)
        {
            Log->Write("Sensor2 is ON");
            DeviceState->OutStateCode = DSE_SENSOR_2_ON;
            Sensor2 = 1;
            ChangeDeviceState();
            return;
        }
    }
    else
    {
        if (Sensor2 == 1)
        {
            Log->Write("Sensor2 is OFF");
            DeviceState->OutStateCode = DSE_SENSOR_2_OFF;
            Sensor2 = 0;
            ChangeDeviceState();
            return;
        }
    }

    //cheking sensor3
    sensor = value & 0x04;
    if(sensor > 0)
    {
        if (Sensor3 == 0)
        {
            Log->Write("Sensor3 is ON");
            DeviceState->OutStateCode = DSE_SENSOR_3_ON;
            Sensor3 = 1;
            ChangeDeviceState();
            return;
        }
    }
    else
    {
        if (Sensor3 == 1)
        {
            Log->Write("Sensor3 is OFF");
            DeviceState->OutStateCode = DSE_SENSOR_3_OFF;
            Sensor3 = 0;
            ChangeDeviceState();
            return;
        }
    }
}


void TSBK2DeviceThread::ProcessOutCommand()
{
    //обновим метку времени, типа ещё жив поток
    CreationTime = clock();
    int Command = GetExtCommand();
    int result = 0;

    switch (Command)
    {
        case EC_WDTGetDoorSwitch:
          try
          {
              if (WDTGetDoorSwitch)
              {
                  ProcInfo->Status = ProcInfo->cs_Processing;
                  switches = 0;
                  result = WDTGetDoorSwitch(switches);
                  if (result != CorrectResult)
                  {
                      DeviceState->OutStateCode = DSE_NOTMOUNT;
                      ChangeDeviceState();
                      return;
                  }
                  else
                  {
                      DeviceState->OutStateCode = DSE_OK;
                      ChangeDeviceState();
                  }
                  ProcInfo->Status = ProcInfo->cs_Done;
                  Log->Write((boost::format("WDTGetDoorSwitch() = %1%") % result).str().c_str());
                  CheckDoorSwitch(switches);
              }
          }
          catch(...)
          {
              ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
              Log->Write("WDTGetDoorSwitch() exception");
          }
          break;

        case EC_WDTSetTimer:
          try
          {
              if (WDTSetTimer)
              {
                  ProcInfo->Status = ProcInfo->cs_Processing;
                  result = WDTSetTimer(60*3);
                  Log->Write((boost::format("WDTSetTimer() = %1%") % result).str().c_str());
                  ProcInfo->Status = ProcInfo->cs_Done;
                  Log->Write((boost::format("WDTSetTimer() = %1%") % result).str().c_str());
                  if (result == CorrectResult)
                      SetInitialized();
                  if (result != CorrectResult)
                  {
                      DeviceState->OutStateCode = DSE_NOTMOUNT;
                      ChangeDeviceState();
                      return;
                  }
                  else
                  {
                      DeviceState->OutStateCode = DSE_OK;
                      ChangeDeviceState();
                  }
              }
          }
          catch(...)
          {
              ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
              Log->Write("WDTSetTimer() exception");
          }
          break;

        case EC_WDTClearTimer:
          try
          {
              if (WDTClearTimer)
              {
                  ProcInfo->Status = ProcInfo->cs_Processing;
                  result = WDTClearTimer();
                  Log->Write((boost::format("WDTClearTimer() = %1%") % result).str().c_str());
                  ProcInfo->Status = ProcInfo->cs_Done;
                  if (result != CorrectResult)
                  {
                      DeviceState->OutStateCode = DSE_NOTMOUNT;
                      ChangeDeviceState();
                      return;
                  }
                  else
                  {
                      DeviceState->OutStateCode = DSE_OK;
                      ChangeDeviceState();
                  }
              }
          }
          catch(...)
          {
              ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
              Log->Write("WDTClearTimer() exception");
          }
          break;

        case EC_WDTStopTimer:
          try
          {
              if (WDTStopTimer)
              {
                  ProcInfo->Status = ProcInfo->cs_Processing;
                  result = WDTStopTimer();
                  Log->Write((boost::format("WDTStopTimer() = %1%") % result).str().c_str());
                  ProcInfo->Status = ProcInfo->cs_Done;
                  if (result != CorrectResult)
                  {
                      DeviceState->OutStateCode = DSE_NOTMOUNT;
                      ChangeDeviceState();
                      return;
                  }
                  else
                  {
                      DeviceState->OutStateCode = DSE_OK;
                      ChangeDeviceState();
                  }
              }
          }
          catch(...)
          {
              ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
              Log->Write("WDTStopTimer() exception");
          }
          break;

        case EC_WDTResetModem:
          try
          {
              if (WDTResetModem)
              {
                  ProcInfo->Status = ProcInfo->cs_Processing;
                  result = WDTResetModem();
                  Log->Write((boost::format("WDTResetModem() = %1%") % result).str().c_str());
                  ProcInfo->Status = ProcInfo->cs_Done;
                  if (result != CorrectResult)
                  {
                      DeviceState->OutStateCode = DSE_NOTMOUNT;
                      ChangeDeviceState();
                      return;
                  }
                  else
                  {
                      DeviceState->OutStateCode = DSE_OK;
                      ChangeDeviceState();
                  }
              }
          }
          catch(...)
          {
              ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
              Log->Write("WDTResetModem() exception");
          }
          break;

        case EC_WDTResetComputer:
          try
          {
              if (WDTResetComputer)
              {
                  ProcInfo->Status = ProcInfo->cs_Processing;
                  result = WDTResetComputer();
                  Log->Write((boost::format("WDTResetComputer() = %1%") % result).str().c_str());
                  ProcInfo->Status = ProcInfo->cs_Done;
                  if (result != CorrectResult)
                  {
                      DeviceState->OutStateCode = DSE_NOTMOUNT;
                      ChangeDeviceState();
                      return;
                  }
                  else
                  {
                      DeviceState->OutStateCode = DSE_OK;
                      ChangeDeviceState();
                  }
              }
          }
          catch(...)
          {
              ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
              Log->Write("WDTResetComputer() exception");
          }
          break;


        case EC_WDTStopOperation:
            StopOperation = true;
            return;
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
