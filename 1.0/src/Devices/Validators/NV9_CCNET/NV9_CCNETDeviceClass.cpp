//---------------------------------------------------------------------------


#pragma hdrstop

#include "NV9_CCNETDeviceClass.h"
#include "NV9_CCNETDeviceThread.h"
#include "DeviceClass.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

TNV9_CCNETDeviceClass::TNV9_CCNETDeviceClass(int id, int ComPort,TLogClass* _Log): CValidator(id, ComPort,_Log, "NV9_CCNETValidator")
{
  DataLengthIndex = 2;
  BeginByte = 0x02;
  LoggingErrors = true;
}

TNV9_CCNETDeviceClass::~TNV9_CCNETDeviceClass()
{
}

void TNV9_CCNETDeviceClass::Start()
{
  //включаем устройство, переход в режим ожидани€
  if (DeviceState->State != NotRun)
    return;
  DeviceState->State = Wait;
  DeviceThread = new TNV9_CCNETDeviceThread();
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
  ((TNV9_CCNETDeviceThread*)DeviceThread)->DisabledNominals = DisabledNominals;
  DeviceState->Nominal = 0;

  DeviceThread->ExchangeRate = ExchangeRate;
  DeviceThread->Currency = Currency;
}

bool TNV9_CCNETDeviceClass::IsItYou()
{
    if (DeviceState->StateCode == 0xFF)
        return false;

    DisableBill();
    Sleep(800);
    if (DeviceState->StateCode != 0x19)
        return false;

    EnableBill();
    Sleep(800);
    if (DeviceState->StateCode != 0x14)
        return false;

    DisableBill();
    return true;
}

