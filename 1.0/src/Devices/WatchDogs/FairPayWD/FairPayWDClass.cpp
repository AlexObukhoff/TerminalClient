//---------------------------------------------------------------------------


#pragma hdrstop

#include "FairPayWDClass.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

TFairPayWDDeviceClass::TFairPayWDDeviceClass(TLogClass* _Log) : CWatchDog(0,_Log, "FairPayWD")
{
    if (Port)
        delete Port;
    Port = NULL;
}

TFairPayWDDeviceClass::~TFairPayWDDeviceClass()
{
}

void TFairPayWDDeviceClass::Start()
{
  //включаем устройство, переход в режим ожидани€
  if (DeviceState->State != NotRun)
    return;
  DeviceState->State = Wait;
  DeviceThread = new TFairPayWDDeviceThread();
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

  DeviceThread->ChangeEvent = ChangeEvent;
  DeviceThread->DisabledNominals = DisabledNominals;
  DeviceState->Nominal = 0;

  DeviceThread->CommandFinished = CommandFinished;

  //===================================================
  ((TFairPayWDDeviceThread*)DeviceThread)->hDll = hDll;

  ((TFairPayWDDeviceThread*)DeviceThread)->InitDevice = InitDevice;
  ((TFairPayWDDeviceThread*)DeviceThread)->DeInit = DeInit;
  ((TFairPayWDDeviceThread*)DeviceThread)->StartWork = StartWork;
  ((TFairPayWDDeviceThread*)DeviceThread)->SetValues = SetValues;
  ((TFairPayWDDeviceThread*)DeviceThread)->ResetModem = ResetModem;
  ((TFairPayWDDeviceThread*)DeviceThread)->WriteIdleReport = WriteIdleReport;
}

