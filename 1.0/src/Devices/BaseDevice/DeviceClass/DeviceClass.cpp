#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#ifndef DeviceClassH
#include "ComPortClass.h"
#include "DeviceClass.h"
#include "globals.h"
#include "boost/format.hpp"

#define SleepInterval 1

TDeviceClass::TDeviceClass(int ComPort,TLogClass* _Log, AnsiString Prefix, PortType::Enum portType)
{
  Log = NULL;
  Log = _Log;
  NewLog = false;
  if (Log == NULL)
  {
    NewLog = true;
    Log = new TLogClass(Prefix.c_str());
  }

  DeviceName = "";

  Log->Write("Device start.");

  COMParametersCriticalSection = NULL;
  DeviceStateCriticalSection = NULL;
  CommandParametersCriticalSection = NULL;
  CommandCriticalSection = NULL;
  COMParameters = NULL;
  Port = NULL;
  CommandParameters = NULL;
  DeviceState = NULL;
  DisabledNominals = NULL;
  DeviceStarted = NULL;
  DeviceStopped = NULL;
  DevicePaused = NULL;
  CommandStarted = NULL;
  CommandPaused = NULL;
  CommandFinished = NULL;
  ChangeEvent = NULL;
  LoggingErrors = true;

  //GlobalStop = false;
  GlobalStop = true;
  DeviceThread = NULL;

  COMParametersCriticalSection = new TCriticalSection();
  DeviceStateCriticalSection = new TCriticalSection();
  CommandParametersCriticalSection = new TCriticalSection();
  CommandCriticalSection = new TCriticalSection();
  DeviceState = new TDeviceState(DeviceStateCriticalSection);
  DeviceState->SetParameters(COMParameters, "“естовое устройство",NotRun,0xff,0xff,0xff,0xff);

  COMParameters = new TComPortInitParameters(COMParametersCriticalSection);
  //COMParameters->SetParameters(ComPort,9600,ONESTOPBIT,NOPARITY,8,0,DTR_CONTROL_HANDSHAKE,600);
  COMParameters->SetParameters(ComPort,9600,ONESTOPBIT,NOPARITY,8,0,DTR_CONTROL_ENABLE,600);
  //если класс параметров будет перекрыватьс€ потомком, то создание класса
  //будем делать не здесь, а в классе потомке
  CommandParameters = new TCommandParameters(CommandParametersCriticalSection);
  CommandParameters->SetParameters(NULL,0,0);

  ClearCommand();
  ClearAnswer();


  DeviceStateChanged = StateChanged;

  DataLengthIndex = 2;
  BeginByte = 0x02;
  LastError = 0;
  EndByte = 0;
  CRCLength = 0;
  DataLength = 0;
  ThreadLifeTime = 5000;
  FrozenTimeOut = 30;
  DeviceState->StateCode = DeviceState->OldStateCode = 0;
  DeviceState->SubStateCode = 0;

  DisabledNominals = new TList();

  MaxEventsCount = 30;
  Events = new TList();
  EventID = 1;

  if (ComPort != 0)
      Port = new TComPort(COMParameters, Log, true, portType);

  //SetEvent(DeviceState);
  Initialized = false;
  TimeStamp1 = clock();
  TimeStamp2 = clock();
}

TDeviceClass::~TDeviceClass()
{
  try
  {
      try
      {
        Log->Write("Device stop.");
        if (Port != NULL)
        {
            GlobalStop = true;
            Stop();
            if (Port)
                delete Port;
            Port = NULL;
        }

        if (DeviceState)
            delete DeviceState;
        delete DeviceStateCriticalSection;
        delete CommandParametersCriticalSection;
        delete CommandCriticalSection;
        delete COMParametersCriticalSection;
        delete COMParameters;
        delete CommandParameters;

        DeviceStateCriticalSection = NULL;
        CommandParametersCriticalSection = NULL;
        CommandCriticalSection = NULL;
        COMParametersCriticalSection = NULL;
        COMParameters = NULL;
        CommandParameters = NULL;
        DeviceState = NULL;
        if (DisabledNominals != NULL)
          delete DisabledNominals;
        if (Events != NULL)
        {
            for (int i=0; i<Events->Count; i++)
            {
                TDeviceState* event = (TDeviceState*)Events->Items[i];
                if (event)
                  delete event;
            }
            delete Events;
        }
      }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        }
  }
  __finally
  {
      if ((NewLog)&&(Log != NULL))
      {
        delete Log;
        Log = NULL;
      }
  }
}

void TDeviceClass::ClearBuffer(BYTE* Buffer)
{
  memset(Buffer,0,DCBufferSize);
}

void TDeviceClass::ClearCommand()
{
  memset(Command,0,DCBufferSize);
  CommandSize = 0;
}

void TDeviceClass::ClearAnswer()
{
  memset(Answer,0,DCBufferSize);
  AnswerSize = 0;
}

void TDeviceClass::Start()
{
  //включаем устройство, переход в режим ожидани€
  if ((DeviceState == NULL)||(DeviceState->State != NotRun)) return;
  //включаем устройство, переход в режим ожидани€
  if (DeviceState != NULL)    DeviceState->State = Wait;

  DeviceState->State = Wait;
  DeviceThread = new TDeviceThread(true);
  DeviceThread->LifeTime = ThreadLifeTime;
  DeviceThread->Log = Log;
  DeviceThread->DeviceState = DeviceState;
  DeviceThread->CommandParameters = CommandParameters;
  DeviceThread->SendType = SendType;
  DeviceThread->Port = Port;
  DeviceThread->Command = Command;
  DeviceThread->CommandSize = CommandSize;
  DeviceThread->Answer = Answer;
  DeviceThread->AnswerSize = AnswerSize;
  DeviceThread->data = data;
  DeviceThread->len_data = &len_data;
  DeviceThread->MaxCash = MaxCash;

  DeviceThread->CommandCriticalSection = CommandCriticalSection;

  DeviceThread->DataLengthIndex = DataLengthIndex;
  DeviceThread->BeginByte = BeginByte;
  DeviceThread->LoggingErrors = LoggingErrors;
  DeviceThread->LastError = LastError;
  DeviceThread->EndByte = EndByte;
  DeviceThread->CRCLength = CRCLength;
  DeviceThread->DataLength = DataLength;

  DeviceThread->DeviceStarted = DeviceStarted;
  DeviceThread->DeviceStopped = DeviceStopped;
  DeviceThread->DevicePaused = DevicePaused;
  DeviceThread->DeviceStateChanged = DeviceStateChanged;
  DeviceThread->CommandStarted = CommandStarted;
  DeviceThread->CommandPaused = CommandPaused;
  DeviceThread->CommandFinished = CommandFinished;

  DeviceThread->ChangeEvent = ChangeEvent;
  DeviceThread->DisabledNominals = DisabledNominals;
  DeviceThread->Currency = Currency;
  DeviceState->Nominal = 0;
}

void TDeviceClass::Stop()
{
  GlobalStop = true;
  DeviceState->Scanner = false;
  StopThread();
}

void TDeviceClass::StopThread()
{
  if (DeviceThread == NULL)
    return;
  //if ((DeviceState)&&(DeviceState->State == NotRun))
    //return;

  if (DeviceThread)
    DeviceThread->TerminateCommand = true;

  if (DeviceThread != NULL)
  {
    AnswerSize = DeviceThread->AnswerSize;
    DeviceThread->Terminate();
    TerminateThread((HANDLE)DeviceThread->Handle,0);
    delete DeviceThread;
    //TerminateThread((HANDLE)DeviceThread->Handle,0);
    DeviceThread = NULL;
  }
}

void TDeviceClass::Resume()
{
  if (DeviceThread == NULL)
    return;
  if (DeviceState->State == CommandExecuting)
    return;
  DeviceThread->ThreadTerminated = false;
  GlobalStop = false;
  DeviceState->State = CommandExecuting;
  if (DeviceThread != NULL)
    DeviceThread->Resume();
}

//типова€ функци€ обращени€ к устройству
void TDeviceClass::ExecuteCommand(TSendType type)
{
  try
  {
    SendType = type;
    //заполн€ем класс команды нужными значени€ми
    //буфер заполн€етс€ заранее в методах ответственных за выполнение конкретной команды
    //останавливаем уже существующий поток
    StopThread();
    //создаЄм поток
    Start();
    ClearAnswer();
    //запускаем поток
    Resume();
    WaitSendingFinish();
  }
  __finally
  {
  }
}

//ѕеред выполнением этой команды нужно заполнить класс команды на отправку
void TDeviceClass::StartPooling()
{
  try
  {
    if (!GlobalStop)
      return;
    SendType = RecieveAnswer;
    ClearAnswer();
    //StopThread();
    Stop();
    //создаЄм поток
    Start();
    if (DeviceThread)
    {
      DeviceThread->PollingMode = true;
      //DeviceThread->_PollingInterval = 200;
    }
    //запускаем поток
    Resume();
  }
  __finally
  {
  }
}
//останавливаем поток опроса
void TDeviceClass::StopPooling()
{
  if (GlobalStop) return;
  GlobalStop = true;
  if (DeviceThread)
    DeviceThread->ThreadTerminated = true;
  StopThread();
}

TCommandParameters* TDeviceClass::GetResult()
{
  TCommandParameters* result = NULL;
  try
  {
    CommandParametersCriticalSection->Acquire();
    result = CommandParameters;
  }

  __finally
  {
    CommandParametersCriticalSection->Release();
  }
  return result;
}

void _fastcall TDeviceClass::StateChanged(TDeviceState* State)
{
  if (DeviceState == NULL)
    return;
  //эта функци€ выполн€етс€ по завершению отправки любой из команд в потоке
  //получаем параметры
  if (DeviceThread)
    AnswerSize = DeviceThread->AnswerSize;

  //new 09-01-2007
  //добавл€ем событие в очередь событий
  SetEvent(State);

  //в основном потоке включаем таймер на одноразовую отработку событи€ по завершению отправки команды
  if (ChangeEvent)
  {
    int ticks = 0;
    while((ChangeEvent->Enabled == true)&&(ticks<30))
    {
      ticks++;
      for(int i=0;i<100;i++)
      {
        Application->ProcessMessages();
        Sleep(10);
        if (ChangeEvent->Enabled == false)
          break;
      }
      if (ChangeEvent->Enabled == false)
        break;
    }
    ChangeEvent->Enabled = true;
  }
}

void TDeviceClass::WaitSendingFinish()
{
  try
  {
      //while ((DeviceThread != NULL)&&(DeviceThread->Sending))
      while ((DeviceThread != NULL)&&(DeviceThread->Sending)&&(DeviceThread->TerminateCommand == false))
      {
        DWORD Now = clock();
        if ( (signed)(Now - DeviceThread->CreationTime) > (signed)DeviceThread->LifeTime  )
        {
            if (DeviceState->OutStateCode != DSE_NOTMOUNT)
            {
              DeviceState->StateCode = 0xFF;
              DeviceState->StateChange = true;
              if ((ChangeEvent)&&(DeviceState->OutStateCode != DSE_NOTMOUNT))
              {
                  DeviceState->OutStateCode = DSE_NOTMOUNT;
                  ChangeDeviceState();
              }
            }
            //stop thread
            if (DeviceThread != NULL)
            {
              GlobalStop = true;
              DeviceThread->Terminate();
              TerminateThread((HANDLE)DeviceThread->Handle,0);
            }
            Log->Write("Suspend Thread was killed.");
            break;
        }

        Application->ProcessMessages();
        Sleep(1);
      }
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

std::string TDeviceClass::GetStateDescription()
{
  std::string result = "";
  if (DeviceThread)
    result = DeviceThread->GetStateDescription();
  return result;
}

bool TDeviceClass::IsNominalEnabled(int value)
{
  if (DisabledNominals)
    if ( DisabledNominals->IndexOf((void*)value) >= 0 )
      return true;
  return false;
}

void TDeviceClass::DisableNominal(int value)
{
  if (!IsNominalEnabled(value))
    DisabledNominals->Add((void*)value);
}

void TDeviceClass::EnableNominal(int value)
{
  if (IsNominalEnabled(value))
    DisabledNominals->Remove((void*)value);
}

bool TDeviceClass::IsStateChanged()
{
  return DeviceState->StateChange;
}

void TDeviceClass::StartDevice()
{}
double TDeviceClass::GetMoney()
{
    return 0;
}
void TDeviceClass::ClearMoney()
{}

bool TDeviceClass::IsItYou()
{
    return false;
}

void TDeviceClass::SignalEvent(BYTE StatusCode)
{
    if (DeviceState->OldStateCode != StatusCode)
    {
      DeviceState->StateCode = StatusCode;
      //DeviceState->StateChange = true;
      if ((DeviceState->OldOutStateCode != DeviceState->OutStateCode)||(DeviceState->Billing))
      {
        DeviceState->StateChange = true;
        if ((DeviceState->OldOutStateCode == DeviceState->OutStateCode)&&(DeviceState->Billing))
          DeviceState->StateChange = false;
        //Log->Write("OldOutStateCode="+AnsiString(DeviceState->OldOutStateCode)+"; OutStateCode="+AnsiString(DeviceState->OutStateCode)+"; Billing="+AnsiString((int)DeviceState->Billing)+";");
        if (DeviceStateChanged)
          DeviceStateChanged(DeviceState);
        DeviceState->OldOutStateCode = DeviceState->OutStateCode;
      }
      DeviceState->OldStateCode = DeviceState->StateCode;
    }
}

bool TDeviceClass::ChangeDeviceState(bool wait)
{
    if (((DeviceState)&&(DeviceState->OldOutStateCode != DeviceState->OutStateCode)) ||
        (DeviceState->Billing) ||
        (DeviceState->Stacked) ||
        (DeviceState->Processing))
    {
      DeviceState->StateChange = true;
      if ((DeviceState->OldOutStateCode == DeviceState->OutStateCode)&&(DeviceState->Billing))
          DeviceState->StateChange = false;
      //Log->Write("OutStateCode="+AnsiString(DeviceState->OutStateCode)+"; OldOutStateCode="+AnsiString(DeviceState->OldOutStateCode));
      Log->Write((boost::format("OutStateCode=%1%; OldOutStateCode=%2%") % DeviceState->OutStateDescription(DeviceState->OutStateCode).c_str() % DeviceState->OutStateDescription(DeviceState->OldOutStateCode).c_str()).str().c_str());
      if (DeviceStateChanged)
        DeviceStateChanged(DeviceState);
      DeviceState->OldOutStateCode = DeviceState->OutStateCode;
      int interval = 10;
      DWORD SleepTime = 10*1000;
      if (wait)
      {
          DeviceState->Done = false;
          while((DeviceState)&&(DeviceState->Done == false))
          {
              Application->ProcessMessages();
              Sleep(interval);
              SleepTime -= interval;
              if (SleepTime <= 0)
                  break;
          }
      }
      return true;
    }
    return false;
}


void TDeviceClass::SetEvent(TDeviceState* Event)
{
    if ((Events == NULL)||(Event == NULL))
        return;

    TDeviceState* event = NULL;
    if ((MaxEventsCount != -1)&&(MaxEventsCount <= Events->Count)&&(Events->Count > 0))
    {
        //если очередь полна - удал€ем первый элемент и пишем событие в конец очереди
        int index = 0;
        event = (TDeviceState*)Events->Items[index];
        if (event)
            delete event;
        Events->Delete(index);
    }

    event = new TDeviceState(NULL);

    event->ID = EventID;
    if (EventID >= 268435455)
        EventID = 1;
    EventID++;
    Log->Write((boost::format("Added event with ID = %1%") % event->ID).str().c_str());
    event->State = Event->State;
    event->ResultCode = Event->ResultCode;
    event->AnswerSize = Event->AnswerSize;
    event->StateCode = Event->StateCode;
    event->SubStateCode = Event->SubStateCode;
    event->OldStateCode = Event->OldStateCode;
    event->OutStateCode = Event->OutStateCode;
    event->OldOutStateCode = Event->OldOutStateCode;
    event->StateDescription = Event->StateDescription;
    event->SubStateDescription = Event->SubStateDescription;
    event->Global = Event->Global;
    event->Count = Event->Count;
    event->StateChange = Event->StateChange;
    event->Billing = Event->Billing;
    event->Stacking = Event->Stacking;
    event->BillEnable = Event->BillEnable;
    event->Idle = Event->Idle;
    event->Nominal = Event->Nominal;
    event->Done = Event->Done;
    event->DSR_CTS = Event->DSR_CTS;
    memcpy(event->Errors,Event->Errors,100);
    memcpy(event->Statuses,Event->Statuses,100);
    event->ErrorsCount = Event->CriticalErrorsCount = Event->ErrorsCount;
    event->StatusesCount = Event->CriticalStatusesCount = Event->StatusesCount;
    event->OutStateCodeEx = Event->OutStateCodeEx;
    event->OldOutStateCodeEx = Event->OldOutStateCodeEx;
    event->OutErrorCodeEx = Event->OutErrorCodeEx;
    event->OldOutErrorCodeEx = Event->OldOutErrorCodeEx;

    event->Enabling   = Event->Enabling;
    event->Processing = Event->Processing;
    event->Stacked    = Event->Stacked;
    ////////////////////////////////////////////////////////////////////////////
    event->Scanner = Event->Scanner;
    event->scannerDataValue = Event->scannerDataValue;
    event->scannerDataType = Event->scannerDataType;
    ////////////////////////////////////////////////////////////////////////////
    Events->Add((void*)event);
}

int TDeviceClass::GetEventsCount()
{
    if (Events == NULL)
        return -1;
    else
        return Events->Count;
}

bool TDeviceClass::GetEvent(TDeviceState* Event)
{
    if ((Events == NULL)||(Event == NULL)||(Events->Count == 0))
        return false;

    Event->State = Undeterminated;
    Event->ID = 0;
    Event->ResultCode = 0xFF;
    Event->AnswerSize = -1;
    Event->StateCode = 0;
    Event->SubStateCode = 0;
    Event->OldStateCode = 0;
    Event->OutStateCode = 0;
    Event->OutStateCode = DSE_UNKNOWN;
    Event->OldOutStateCode = DSE_UNKNOWN;
    Event->StateDescription = "";
    Event->SubStateDescription = "";
    Event->Global = 0;
    Event->Count = 0;
    Event->StateChange = false;
    Event->Billing = false;
    Event->Stacking = false;
    Event->BillEnable = false;
    Event->Idle = true;
    Event->DSR_CTS = 0;
    Event->Nominal = 0;
    Event->Done = false;
    memset(Event->Errors,0,Event->_ErrorsBufferSize);
    memset(Event->Statuses,0,Event->_StatusesBufferSize);
    Event->ErrorsCount = Event->CriticalErrorsCount = 0;
    Event->StatusesCount = Event->CriticalStatusesCount = 0;
    Event->OutStateCodeEx = DSE_UNKNOWN_CODE;
    Event->OldOutStateCodeEx = DSE_UNKNOWN_CODE;
    Event->OutErrorCodeEx = DSE_UNKNOWN_CODE;
    Event->OldOutErrorCodeEx = DSE_UNKNOWN_CODE;

    Event->Enabling    = false;
    Event->Processing  = false;
    Event->Stacked     = false;

    TDeviceState* event = NULL;
    int index = 0;
    event = (TDeviceState*)Events->Items[index];
    Events->Delete(index);
    Log->Write((boost::format("Read event with ID=%1%") % event->ID).str().c_str());
    if (event)
    {
        Event->State = event->State;
        Event->ID = event->ID;
        Event->ResultCode = event->ResultCode;
        Event->AnswerSize = event->AnswerSize;
        Event->StateCode = event->StateCode;
        Event->SubStateCode = event->SubStateCode;
        Event->OldStateCode = event->OldStateCode;
        Event->OutStateCode = event->OutStateCode;
        Event->OldOutStateCode = event->OldOutStateCode;
        Event->StateDescription = event->StateDescription;
        Event->SubStateDescription = event->SubStateDescription;
        Event->Global = event->Global;
        Event->Count = event->Count;
        Event->StateChange = event->StateChange;
        Event->Billing = event->Billing;
        Event->Stacking = event->Stacking;
        Event->BillEnable = event->BillEnable;
        Event->Idle = event->Idle;
        Event->Nominal = event->Nominal;
        Event->Done = event->Done;
        Event->DSR_CTS = event->DSR_CTS;
        memcpy(Event->Errors,event->Errors,Event->_ErrorsBufferSize);
        memcpy(Event->Statuses,event->Statuses,Event->_StatusesBufferSize);
        Event->ErrorsCount = Event->CriticalErrorsCount = event->ErrorsCount;
        Event->StatusesCount = Event->CriticalStatusesCount = event->StatusesCount;
        Event->OutStateCodeEx = event->OutStateCodeEx;
        Event->OldOutStateCodeEx = event->OldOutStateCodeEx;
        Event->OutErrorCodeEx = event->OutErrorCodeEx;
        Event->OldOutErrorCodeEx = event->OldOutErrorCodeEx;

        Event->Enabling   = event->Enabling;
        Event->Processing = event->Processing;
        Event->Stacked    = event->Stacked;
        ////////////////////////////////////////////////////////////////////////
        Event->Scanner = event->Scanner;
        Event->scannerDataValue = event->scannerDataValue;
        Event->scannerDataType = event->scannerDataType;
        ////////////////////////////////////////////////////////////////////////
        delete event;
    }
    else
        Log->Write("Event is NULL");

    return true;
}

BYTE TDeviceClass::GetExtCommand()
{
    if (DeviceThread)
    {
        BYTE result = DeviceThread->GetExtCommand();
        return result;
    }
    else
        return 0;
}

bool TDeviceClass::SetExtCommand(BYTE Command)
{
    if (DeviceThread)
    {
        int ticks = 10;
        bool result = false;
        //try to set external command if command is EXTC_Free result will be true
        while(((result = DeviceThread->SetExtCommand(Command)) == false)&&(ticks > 0))
        {
            ticks--;
            Application->ProcessMessages();
            Sleep(10);
        }
        return result;
    }
    else
        return false;
}

int TDeviceClass::Initialize()
{
    SetInitialized();
    return 1;
}

bool TDeviceClass::IsInitialized()
{
    return Initialized;
}

void TDeviceClass::SetInitialized()
{
    Initialized = true;
    if (Log)
        Log->Write("Device Initialized.");
}

clock_t TDeviceClass::GetTimeStamp1()
{
    if (DeviceThread)
        return TimeStamp1 = DeviceThread->GetTimeStamp1();
    else
        return TimeStamp1;
}

clock_t TDeviceClass::GetTimeStamp2()
{
    if (DeviceThread)
        return TimeStamp2 = DeviceThread->GetTimeStamp2();
    else
        return TimeStamp2;
}

#endif



