//---------------------------------------------------------------------------


#pragma hdrstop

#include "MEIDeviceThread.h"
#include "DeviceClass.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

TMEIDeviceClass::TMEIDeviceClass(int id, int ComPort, TLogClass* _Log, int _mode): CValidator(id, ComPort, _Log, "MEIValidator", _mode)
{
  DataLengthIndex = 1;
  BeginByte = 0x02;
  //EndByte = 0x03;
  CRCLength = 1;
  delete Port;
  //if (BaudRate > 0)
      //COMParameters->BaudRate = BaudRate;
  COMParameters->Parity = EVENPARITY;
  COMParameters->timeout = 300;
  COMParameters->ByteSize = 7;
  Port = new TComPort(COMParameters,Log,true);
  LoggingErrors = false;
}

TMEIDeviceClass::~TMEIDeviceClass()
{
}

void TMEIDeviceClass::Start()
{
  //включаем устройство, переход в режим ожидани€
  if (DeviceState->State != NotRun)
    return;
  DeviceState->State = Wait;
  DeviceState->OutStateCode = DSE_NOTMOUNT;
  DeviceThread = new TMEIDeviceThread(mode);
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

bool TMEIDeviceClass::IsItYou()
{
    if ((DeviceState)&&(DeviceState->OutStateCode != DSE_NOTMOUNT)&&(DeviceState->OutStateCode != DSE_UNKNOWN))
        return true;
    else
        return false;
}

