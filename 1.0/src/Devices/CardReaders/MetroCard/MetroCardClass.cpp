//---------------------------------------------------------------------------


#pragma hdrstop

#include "MetroCardClass.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

TMetroCardDeviceClass::TMetroCardDeviceClass(int ComPort,TLogClass* _Log) : CCardReader(ComPort,_Log, "MetroCard")
{
    if (Port)
        delete Port;
    Port = NULL;
}

TMetroCardDeviceClass::~TMetroCardDeviceClass()
{
}

void TMetroCardDeviceClass::Start()
{
  //включаем устройство, переход в режим ожидани€
  if (DeviceState->State != NotRun)
    return;
  DeviceState->State = Wait;
  DeviceThread = new TMetroCardDeviceThread();
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
  ((TMetroCardDeviceThread*)DeviceThread)->hDll = hDll;

  ((TMetroCardDeviceThread*)DeviceThread)->ASKOPMInit = ASKOPMInit;
  ((TMetroCardDeviceThread*)DeviceThread)->ASKOPMFindCard = ASKOPMFindCard;
  ((TMetroCardDeviceThread*)DeviceThread)->ASKOPMGetMenu = ASKOPMGetMenu;
  ((TMetroCardDeviceThread*)DeviceThread)->ASKOPMWriteCard = ASKOPMWriteCard;

  ((TMetroCardDeviceThread*)DeviceThread)->InitInfo = InitInfo;
  ((TMetroCardDeviceThread*)DeviceThread)->FindCardInfo = FindCardInfo;
  ((TMetroCardDeviceThread*)DeviceThread)->GetMenuInfo = GetMenuInfo;
  ((TMetroCardDeviceThread*)DeviceThread)->WriteCardInfo = WriteCardInfo;
}

