//---------------------------------------------------------------------------


#pragma hdrstop

#include "WatchDogOSMPClass.h"
#include "WatchDogOSMPThread.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

TWatchDogOSMPDeviceClass::TWatchDogOSMPDeviceClass(int ComPort,TLogClass* _Log): CWatchDog(ComPort,_Log, "WatchDogOSMP")
{
  delete Port;
  COMParameters->Parity = NOPARITY;
  COMParameters->BaudRate = 9600;
  Port = new TComPort(COMParameters,Log,true);
}

TWatchDogOSMPDeviceClass::~TWatchDogOSMPDeviceClass()
{
}

void TWatchDogOSMPDeviceClass::Start()
{
  //�������� ����������, ������� � ����� ��������
  if (DeviceState->State != NotRun)
    return;
  DeviceState->State = Wait;
  DeviceThread = new TWatchDogOSMPDeviceThread();
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
