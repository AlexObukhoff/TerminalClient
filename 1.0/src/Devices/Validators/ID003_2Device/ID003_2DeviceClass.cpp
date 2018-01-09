//---------------------------------------------------------------------------


#pragma hdrstop

#include "ID003_2DeviceClass.h"
#include "ID003_2DeviceThread.h"
#include "DeviceClass.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

TID003_2DeviceClass::TID003_2DeviceClass(int id, int ComPort,TLogClass* _Log): CValidator(id, ComPort,_Log, "WBA003_2Validator")
{
  DataLengthIndex = 1;
  BeginByte = 0xFC;
  delete Port;
  COMParameters->Parity = EVENPARITY;
  Port = new TComPort(COMParameters,Log,true);
  LoggingErrors = false;
}

TID003_2DeviceClass::~TID003_2DeviceClass()
{
}

void TID003_2DeviceClass::Start()
{
  //включаем устройство, переход в режим ожидани€
  if (DeviceState->State != NotRun)
    return;
  DeviceState->State = Wait;
  DeviceState->OutStateCode = DSE_NOTMOUNT;
  DeviceThread = new TID003_2DeviceThread();
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

bool TID003_2DeviceClass::IsItYou()
{
    if (DeviceState->StateCode == 0xFF)
        return false;

    DisableBill();
    Sleep(800);
    if ((DeviceState->StateCode != DISABLE)&&(DeviceState->StateCode != _DISABLE))
        return false;

    EnableBill();
    Sleep(800);
    if ((DeviceState->StateCode != ENABLE))
        return false;

    DisableBill();
    return true;
}

