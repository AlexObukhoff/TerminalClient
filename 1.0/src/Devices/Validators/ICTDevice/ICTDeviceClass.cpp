//---------------------------------------------------------------------------
#pragma hdrstop
#include "ICTDeviceClass.h"
#include "ICTDeviceThread.h"
#include "DeviceClass.h"
#include "boost/format.hpp"
//---------------------------------------------------------------------------
#pragma package(smart_init)

TICTDeviceClass::TICTDeviceClass(int id, int ComPort,TLogClass* _Log): CValidator(id, ComPort,_Log, "ICTValidator")
{
  DataLength = 1;
  delete Port;
  COMParameters->Parity = EVENPARITY;
  COMParameters->timeout = 100;
  Port = new TComPort(COMParameters,Log,true);
  LoggingErrors = false;
}

TICTDeviceClass::~TICTDeviceClass()
{
}

void TICTDeviceClass::Start()
{
  //включаем устройство, переход в режим ожидани€
  if (DeviceState->State != NotRun)
    return;
  DeviceState->State = Wait;
  DeviceThread = new TICTDeviceThread();
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
  Log->Write((boost::format("DeviceThread->ExchangeRate=%1%") % DeviceThread->ExchangeRate).str().c_str());
  Log->Write((boost::format("DeviceThread->Currency=%1%") % DeviceThread->Currency.c_str()).str().c_str());
}

bool TICTDeviceClass::IsItYou()
{
    if (DeviceState->StateCode == 0xFF)
        return false;

    DisableBill();
    Sleep(800);
    if (DeviceState->StateCode != DISABLE)
        return false;

    EnableBill();
    Sleep(800);
    if ((DeviceState->StateCode != ENABLE))
        return false;

    DisableBill();
    return true;
}

