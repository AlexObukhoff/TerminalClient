//---------------------------------------------------------------------------
#pragma hdrstop
#include "V2EDeviceClass.h"
#include "V2EDeviceThread.h"
#include "DeviceClass.h"
#include "boost/format.hpp"
//---------------------------------------------------------------------------
#pragma package(smart_init)

TV2EDeviceClass::TV2EDeviceClass(int id, int ComPort, TLogClass* _Log): CValidator(id, ComPort,_Log, "V2EValidator")
{
  DataLengthIndex = 3;
  BeginByte = 0x02;
  LoggingErrors = false;

  COMParameters->Parity = EVENPARITY;
  COMParameters->BaudRate = CBR_9600;
  COMParameters->DtrControl = (DWORD) DTR_CONTROL_DISABLE;
  COMParameters->RtsControl = (DWORD) RTS_CONTROL_DISABLE;
  Port->ReopenPort();
  
  Port->to.ReadIntervalTimeout = 0;
  Port->to.ReadTotalTimeoutMultiplier = 0;
  Port->to.ReadTotalTimeoutConstant = 0;
  Port->to.WriteTotalTimeoutMultiplier = 0;
  Port->to.WriteTotalTimeoutConstant = 0;
  SetCommTimeouts(Port->Port, &Port->to);
}

TV2EDeviceClass::~TV2EDeviceClass()
{
}

void TV2EDeviceClass::Start()
{
  //включаем устройство, переход в режим ожидания
  if (DeviceState->State != NotRun)
    return;
  DeviceState->State = Wait;
  DeviceThread = new TV2EDeviceThread();
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

  DeviceThread->ExchangeRate = ExchangeRate;
  DeviceThread->Currency = Currency;
  DeviceThread->_PollingInterval = 150;     //это число примерное. для GPT надо ставить 350-500 мс, на самом деле вместо 150 мс имеем 315-330 мс
  Log->Write((boost::format("DeviceThread->ExchangeRate=%1%") % DeviceThread->ExchangeRate).str().c_str());
  Log->Write((boost::format("DeviceThread->Currency=%1%") % DeviceThread->Currency.c_str()).str().c_str());
}

