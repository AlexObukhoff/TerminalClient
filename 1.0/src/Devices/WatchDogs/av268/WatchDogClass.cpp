//---------------------------------------------------------------------------


#pragma hdrstop

#include "WatchDogClass.h"
#include "WatchDogThread.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

TWatchDogDeviceClass::TWatchDogDeviceClass(int ComPort,TLogClass* _Log): CWatchDog(ComPort,_Log, "WatchDog")
{
  delete Port;
  COMParameters->Parity = NOPARITY;
  COMParameters->BaudRate = 19200;
  Port = new TComPort(COMParameters,Log,true);
  Version = "";
}

TWatchDogDeviceClass::~TWatchDogDeviceClass()
{
}

void TWatchDogDeviceClass::Start()
{
  //�������� ����������, ������� � ����� ��������
  if (DeviceState->State != NotRun)
    return;
  DeviceState->State = Wait;
  DeviceThread = new TWatchDogDeviceThread();
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
}

