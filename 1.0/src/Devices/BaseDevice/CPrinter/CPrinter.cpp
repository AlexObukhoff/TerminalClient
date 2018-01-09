//---------------------------------------------------------------------------
#pragma hdrstop
#include "CPrinter.h"
#include "boost/format.hpp"
//---------------------------------------------------------------------------
#pragma package(smart_init)

CPrinter::CPrinter(int ComPort,TLogClass* _Log, AnsiString Prefix, PortType::Enum portType)
: TDeviceClass(ComPort,_Log, Prefix, portType)
{
  LoggingErrors = true;
  Error = 0;
  PrinterEnable = true;
  Fiscal = false;
  AutoOpenShift = false;
  SessionOpened = true;
  MinLinesCount = 25;
  PushActionType = Pull;
  ThreadLifeTime = 5000;
  _MinLinesCount = 4;
  ZReportInBuffer = false;
  Font = "";
}

void CPrinter::Start()
{
  if (Port == NULL)
    return;
  if (DeviceThread == NULL)
    return;
  //включаем устройство, переход в режим ожидания
  if (DeviceState != NULL)
    DeviceState->State = Wait;

  DeviceThread->Log = Log;
  DeviceThread->LifeTime = ThreadLifeTime;
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

  DeviceThread->DeviceStateChanged = DeviceStateChanged;
  DeviceThread->CommandFinished = CommandFinished;

  DeviceThread->ChangeEvent = ChangeEvent;
  //запускаем отправку команды устройству
  Resume();
  //ждём завершения отправки
  WaitSendingFinish();
}

void CPrinter::PrintXReport(AnsiString Text)
{
    PrintCheck(Text);
}

void CPrinter::PrintZReport(AnsiString Text)
{
    PrintCheck(Text);
}

void CPrinter::FeedToMinLinesCount()
{
    //if (MinLinesCount <= 0)
    MinLinesCount = LinesCount+_MinLinesCount;
    int Count = MinLinesCount - LinesCount;
    if (Count<0)
        Count = _MinLinesCount;
    if (Log)
        Log->Write((boost::format("Feeding to %1% lines") % Count).str().c_str());
    //for(int i=LinesCount; i<=MinLinesCount; i++)
    for(int i=1; i<=Count; i++)
        Feed(1);
}

bool CPrinter::IsPrinterEnable()
{
 PrinterEnable = true;
 GetState();
 
    if ((DeviceState)&&(DeviceState->OutStateCode == DSE_HARDWARE_ERROR))
       PrinterEnable = false;
    return PrinterEnable;
}

void CPrinter::PrintNotFiscalCheck(AnsiString Text)
{
    PrintCheck(Text);
}

void CPrinter::SendCommand()
{
    StopThread();
    DeviceThread = new TDeviceThread(true, false);
    Start();
    delete DeviceThread;
    DeviceThread = NULL;
}

bool CPrinter::PrintZReportsFromBuffer(int BeginSessionNumber, int EndSessionNumber)
{
    UNREFERENCED_PARAMETER(BeginSessionNumber);
    UNREFERENCED_PARAMETER(EndSessionNumber);
    PrintZReport();
    return true;
}

