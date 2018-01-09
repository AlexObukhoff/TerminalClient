//---------------------------------------------------------------------------


#pragma hdrstop

#include "NRIDeviceClass.h"
#include "NRIDeviceThread.h"
#include "DeviceClass.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

TNRIDeviceClass::TNRIDeviceClass(int id, int ComPort,TLogClass* _Log): CCoinAcceptor(id, ComPort,_Log, "NRICoinAcceptor")
{
  DataLengthIndex = 1;
  BeginByte = 0x02;
  //EndByte = 0x03;
  CRCLength = 1;
  delete Port;
  COMParameters->DtrControl = DTR_CONTROL_DISABLE;
  COMParameters->RtsControl = RTS_CONTROL_DISABLE;
  COMParameters->timeout = 3000;
  Port = new TComPort(COMParameters,Log,true);
  LoggingErrors = false;
}

TNRIDeviceClass::~TNRIDeviceClass()
{
    //Disable();
    //Sleep(1500);
}

void TNRIDeviceClass::Start()
{
  //включаем устройство, переход в режим ожидани€
  if (DeviceState->State != NotRun)
    return;
  DeviceState->State = Wait;
  DeviceState->OutStateCode = DSE_NOTMOUNT;
  DeviceThread = new TNRIDeviceThread();
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
  DeviceThread->MaxCash = MaxCash;
  DeviceThread->MinCash = MinCash;

  DeviceThread->ChangeEvent = ChangeEvent;
  DeviceThread->DisabledNominals = DisabledNominals;
  DeviceState->Nominal = 0;

  DeviceThread->ExchangeRate = ExchangeRate;
  DeviceThread->Currency = Currency;
}

