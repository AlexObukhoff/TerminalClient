//---------------------------------------------------------------------------


#pragma hdrstop

#include "Citizen268Class.h"
//#include "Citizen268Thread.h"
#include "DeviceThread.h"
#include "globals.h"
#include "boost/format.hpp"

//---------------------------------------------------------------------------

#pragma package(smart_init)

CCitizen268::CCitizen268(int ComPort,int BaudRate,TLogClass* _Log) : CPrinter(ComPort,_Log, "Citizen268")
{
  DataLength = 1;
  delete Port;
  COMParameters->Parity = NOPARITY;
  COMParameters->BaudRate = 115200;
  if (BaudRate > 0)
      COMParameters->BaudRate = BaudRate;
  Port = new TComPort(COMParameters,Log,true);
  LoggingErrors = true;
  //Init();
  //GetState();
  DeviceName = "citizen268";
  _MinLinesCount = 0;
  //ShriftOptionsEx(0x10);
}

CCitizen268::~CCitizen268()
{
}

void CCitizen268::SendCommand()
{
  DeviceThread = new TDeviceThread(true,false);
  Start();
  delete DeviceThread;
  DeviceThread = NULL;
}

void CCitizen268::PrintString(AnsiString text)
{
  PrintLine(text);
  SendCommand();
}

void CCitizen268::PrintCheck(TStringList* Text)
{
  char subtext[100];
  LinesCount = 0;
  for(int i=0; i<Text->Count; i++)
  {
      memset(subtext,0,100);
      CharToOem(Text->Strings[i].c_str(), subtext);
      //PrintString(AnsiString(subtext));
      PrintBigString(AnsiString(subtext));
      LinesCount++;
  }
  FeedToMinLinesCount();
  Cut(true);
}

void CCitizen268::SendPacket(BYTE* command, int bytes_count,int datalen, BYTE* data, bool datafirst)
{
  if (!Port->PortInit)
    return;
  try
  {
    //здесь в буфер пишем команду для выполнения
    ClearCommand();
    ClearAnswer();

    if (!datafirst)
    {
      for(int i=0; i<bytes_count; i++)
        Command[i] = command[i];

      if (datalen != 0)
      {
        if (data!=NULL)
          memcpy(&Command[bytes_count],data,datalen);
      }
    }
    else
    {
      if (datalen != 0)
      {
        if (data!=NULL)
          memcpy(&Command[0],data,datalen);
      }
      for(int i=datalen; i<datalen+bytes_count; i++)
        Command[i] = command[i-datalen];
    }

    //CommandParameters->SetParameters(Command,bytes_count+datalen,command[0],0);
    CommandSize = bytes_count+datalen;
  }
  __finally
  {
  }
}

std::string CCitizen268::GetStateDescription()
{
  return GetStatusDescription(DeviceState->StateCode);
}

void CCitizen268::PrintLine(AnsiString text)
{
   BYTE command[2];
   command[0] = 0x0D;
   command[1] = 0x0A;
   int datalen = text.Length();
   //SendType = NotRecieveAnswer;
   SendType = RecieveAnswer;
   SendPacket(command,2,datalen,text.c_str(),true);
}

void CCitizen268::PrintBigString(AnsiString text)
{
   ClearCommand();
   ClearAnswer();
   Command[0] = 0x1B;
   Command[1] = 0x21;
   Command[2] = 0x10;
   memcpy(&Command[3],text.c_str(),text.Length());
   CommandSize = 3 + text.Length();
   Command[CommandSize] = 0x0A;
   Command[CommandSize+1] = 0x0D;
   CommandSize += 2;
   SendType = NotRecieveAnswer;
   SendCommand();
}

void CCitizen268::Feed(int count)
{
    for(int i=1; i<=count; i++)
    {
        BYTE command[2];
        command[0] = 0x0D;
        command[1] = 0x0A;
        SendType = NotRecieveAnswer;
        SendPacket(command,2,0,NULL,true);
        SendCommand();
    }
}

// бит 0 - удвоенный размер по обеим осям
// бит 1 - резерв
// бит 2 - резерв
// бит 3 - повышенная интенсивность
// бит 4 - растяжение по Х
// бит 5 - рпстяжение по У
// бит 6 - инверсия печати
// бит 7 - подчёркивание
void CCitizen268::ShriftOptionsEx(BYTE option)//true - заводской шрифт false - пользовательский
{
   BYTE command[2];
   command[0] = 0x1B;
   command[1] = 0x21;
   int datalen = 1;
   BYTE data[1];
   data[0] = option;
   SendType = NotRecieveAnswer;
   SendPacket(command,2,datalen,data);
   SendCommand();
}

void CCitizen268::ShriftOptions(bool option)//true - заводской шрифт false - пользовательский
{
   BYTE command[2];
   command[0] = 0x1B;
   command[1] = 0x25;
   int datalen = 1;
   BYTE data[1];
   if (option)
     data[0] = 0;
   else
     data[0] = 1;
   SendType = NotRecieveAnswer;
   SendPacket(command,2,datalen,data);
   SendCommand();
}

void CCitizen268::SetUnderline(bool option)
{
   BYTE command[2];
   command[0] = 0x1B;
   command[1] = 0x2D;
   int datalen = 1;
   BYTE data[1];
   if (option)
     data[0] = 1;
   else
     data[0] = 0;
   SendType = NotRecieveAnswer;
   SendPacket(command,2,datalen,data);
   SendCommand();
}

void CCitizen268::SetBold(bool option)
{
   BYTE command[2];
   command[0] = 0x1B;
   command[1] = 0x45;
   int datalen = 1;
   BYTE data[1];
   if (option)
     data[0] = 1;
   else
     data[0] = 0;
   SendType = NotRecieveAnswer;
   SendPacket(command,2,datalen,data);
   SendCommand();
}

void CCitizen268::SetDoublePrint(bool option)
{
   BYTE command[2];
   command[0] = 0x1B;
   command[1] = 0x47;
   int datalen = 1;
   BYTE data[1];
   if (option)
     data[0] = 1;
   else
     data[0] = 0;
   SendType = NotRecieveAnswer;
   SendPacket(command,2,datalen,data);
   SendCommand();
}

void CCitizen268::Init()
{
   BYTE command[2];
   command[0] = 0x1B;
   command[1] = 0x40;
   SendType = NotRecieveAnswer;
   SendPacket(command,2,0,NULL);
   SendCommand();
}

void CCitizen268::Cut(bool option)//true - полный отрез false - неполный отрез чека
{
   BYTE command[2];
   command[0] = 0x1B;
   if (option)
     command[1] = 0x69;
   else
     command[1] = 0x6D;
   SendType = NotRecieveAnswer;
   SendPacket(command,2,0,NULL);
   SendCommand();
}

void CCitizen268::GetState()
{
    //запрос статуса надо делать после команды печати
   State = 0xFF;
   //State = 0x00;
   SubState = 0x0;
   StateDescr = "";
   SubStateDescr = "";

   //new 18-12-2006
   PrintString(" ");

   BYTE command[2];
   command[0] = 0x1B;
   command[1] = 0x76;
   SendType = RecieveAnswer;
   SendPacket(command,2,0,NULL);
   SendCommand();
   if (DeviceState->AnswerSize > 0)
     State = Answer[0];
   Log->Write((boost::format("Printer State=%1%") % GetStatusDescription(State).c_str()).str().c_str());

  DeviceState->StateCode = State;
  DeviceState->StateDescription = GetStatusDescription(State);
  DeviceState->SubStateCode = 0x00;
  DeviceState->SubStateDescription = "";
  if (State == 0x00)
  {
    if (Error == 1)
    {
      Error = 0;
      DeviceState->StateChange = true;
      DeviceState->OutStateCode = DSE_OK;
      /*if (ChangeEvent)
        StateChanged(DeviceState);*/
      ChangeDeviceState();
    }
  }
}

std::string CCitizen268::GetStatusDescription(BYTE StatusCode)
{
    std::string result = "";
    if (StatusCode == 0x00)
    {
      result = "OK";
      PrinterEnable = true;
      if (OldState != StatusCode)
      {
        OldState = DeviceState->StateCode;
        DeviceState->StateCode = StatusCode;
        DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = result;
        DeviceState->StateChange = true;
        /*if (ChangeEvent)
          StateChanged(DeviceState);*/
        ChangeDeviceState();
      }
      return StateDescr = result;
    }
    if (StatusCode == 0xFF)
    {
      result = "Принтер недоступен!";
      //Error = 1;
      //Error = 0;
      //PrinterEnable = false;
      PrinterEnable = true;
      if (OldState != StatusCode)
      {
        OldState = DeviceState->StateCode;
        DeviceState->StateCode = StatusCode;
        //DeviceState->OutStateCode = DSE_NOTMOUNT;
        DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = result;
        DeviceState->StateChange = true;
        /*if (ChangeEvent)
          StateChanged(DeviceState);*/
        ChangeDeviceState();
      }
        return StateDescr = result;
    }
    BYTE code = StatusCode&0x01;
    if (code > 0)
    {
      result = "Принтер не инициализирован ";
      /*Error = 1;
      PrinterEnable = false;
      if (OldState != StatusCode)
      {
        OldState = DeviceState->StateCode;
        DeviceState->StateCode = StatusCode;
        DeviceState->StateDescription = result;
        DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
        DeviceState->StateChange = true;
        //if (ChangeEvent)
          //StateChanged(DeviceState);
        ChangeDeviceState();
        }*/
        Error = 0;
        PrinterEnable = true;
        return StateDescr = result;
    }

    code = StatusCode&0x02;
    if (code > 0)
    {
      result = "Сбой температуры ";
      Error = 1;
      PrinterEnable = true;
      if (OldState != StatusCode)
      {
        OldState = DeviceState->StateCode;
        DeviceState->StateCode = StatusCode;
        DeviceState->StateDescription = result;
        //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
        DeviceState->StateChange = true;
        /*if (ChangeEvent)
          StateChanged(DeviceState);*/
        ChangeDeviceState();
      }
        return StateDescr = result;
    }

    code = StatusCode&0x04;
    if (code > 0)
    {
      result = "Бумага кончилась ";
      Error = 1;
      PrinterEnable = false;
      if (OldState != StatusCode)
      {
        OldState = DeviceState->StateCode;
        DeviceState->StateCode = StatusCode;
        DeviceState->OutStateCode = DSE_NOTPAPER;
        DeviceState->StateDescription = result;
        DeviceState->StateChange = true;
        /*if (ChangeEvent)
          StateChanged(DeviceState);*/
        ChangeDeviceState();
      }
        return StateDescr = result;
    }


    code = StatusCode&0x08;
    if (code > 0)
    {
      result = "Печатающий механизм не готов ";
      Error = 1;
      PrinterEnable = false;
      if (OldState != StatusCode)
      {
        OldState = DeviceState->StateCode;
        DeviceState->StateCode = StatusCode;
        DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
        DeviceState->StateDescription = result;
        DeviceState->StateChange = true;
        /*if (ChangeEvent)
          StateChanged(DeviceState);*/
        ChangeDeviceState();
      }
        return StateDescr = result;
    }

    code = StatusCode&0x20;
    if (code > 0)
    {
      result = "Сбой принтера ";
      Error = 1;
      PrinterEnable = false;
      if (OldState != StatusCode)
      {
        OldState = DeviceState->StateCode;
        DeviceState->StateCode = StatusCode;
        DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
        DeviceState->StateDescription = result;
        DeviceState->StateChange = true;
        /*if (ChangeEvent)
          StateChanged(DeviceState);*/
        ChangeDeviceState();
      }
        return StateDescr = result;
    }

    return StateDescr = result;
}

void CCitizen268::PrintCheck(AnsiString text, std::string barcode)
{
//  BYTE xON = 0x11;
//  BYTE xOFF = 0x13;
//  DWORD count = 0;
  /*if (DeviceThread)
  {
    DeviceThread->ReadPort(Answer,count);
    int ticks = 0;
    while((count>=1)&&(Answer[0] == xOFF))
    {
      ticks++;
      if (ticks > 1000)
      {
        Log->Write("Printer answer: NAK!");
        return;
      }
      Sleep(10);
      DeviceThread->ReadPort(Answer,count);
    }
  }*/


  Log->Write((boost::format("Printer enable=%1%") % PrinterEnable).str().c_str());
  Log->Write((boost::format("Text to printer: %1%") % text.c_str()).str().c_str());
  //Log->Write("Text to printer: "+ text);
  TStringList* strings = new TStringList();
  AnsiString delim = "\r\n";
  LinesCount = 0;
  while(true)
  {
    int pos = text.Pos(delim);
    if (pos == 0)
      break;
    text = text.Delete(pos,2);
    text = text.Insert("|",pos);
  }
  delim = "|";
  char _subtext[1024];
  while(true)
  {
    int pos = text.Pos(delim);
      if (pos == 0)
      {
          AnsiString subtext = text;
          memset(_subtext,0,1024);
          CharToOem(subtext.c_str(), _subtext);
          PrintBigString(AnsiString(_subtext));
          LinesCount++;
          break;
      }
    AnsiString subtext = text.SubString(0,pos-1);
    text = text.SubString(pos+1,text.Length()-pos);
    //if (!subtext.IsEmpty())
    //{
      memset(_subtext,0,1024);
      CharToOem(subtext.c_str(), _subtext);
      PrintBigString(AnsiString(_subtext));
      LinesCount++;
    //}
  }
  FeedToMinLinesCount();
  Cut(true);
  //PrintCheck(strings);
  delete strings;
}

bool CCitizen268::IsPrinterEnable()
{
  GetState();
  return PrinterEnable;
}

bool CCitizen268::IsItYou()
{
   BYTE command[2];
   command[0] = 0x1B;
   command[1] = 0x76;
   SendType = RecieveAnswer;
   SendPacket(command,2,0,NULL);
   SendCommand();
   if (DeviceState->AnswerSize > 0)
     return true;
   return false;
}

int CCitizen268::Initialize()
{
    try
    {
      Init();
      GetState();
      SetInitialized();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}


