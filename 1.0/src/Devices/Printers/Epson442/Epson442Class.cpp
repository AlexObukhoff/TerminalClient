//---------------------------------------------------------------------------
#pragma hdrstop
#include "Epson442Class.h"
#include "globals.h"
#include "boost/format.hpp"
#include "DeviceThread.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

CEpson442::CEpson442(int ComPort,int BaudRate,TLogClass* _Log) : CPrinter(ComPort,_Log, "Epson442")
{
  DataLength = 1;
  delete Port;
  COMParameters->Parity = NOPARITY;
  COMParameters->BaudRate = 19200;
  if (BaudRate > 0)
      COMParameters->BaudRate = BaudRate;
  Port = new TComPort(COMParameters,Log,true);
  LoggingErrors = true;
  //Init();
  //GetState();
  //SetCodeTable();
  State = 0x00;
  OldState = 0x00;
  SubState = 0x00;
  OldSubState = 0x00;
  DeviceName = "Epson442";
}

CEpson442::~CEpson442()
{
}

void CEpson442::SendCommand()
{
  DeviceThread = new TDeviceThread(true,false);
  Start();
  delete DeviceThread;
  DeviceThread = NULL;
}

void CEpson442::PrintString(AnsiString text)
{
  PrintLine(text);
  SendCommand();
}

void CEpson442::PrintCheck(TStringList* Text)
{
  char subtext[100];
  for(int i=0; i<Text->Count; i++)
  {
      memset(subtext,0,100);
      CharToOem(Text->Strings[i].c_str(), subtext);
      PrintString(Text->Strings[i]);
  }
  Cut(true);
}

void CEpson442::SendPacket(BYTE* command, int bytes_count,int datalen, BYTE* data, bool datafirst)
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

std::string CEpson442::GetStateDescription()
{
  return GetStatusDescription((BYTE)DeviceState->StateCode);
}

void CEpson442::PrintLine(AnsiString text)
{
   BYTE command[2];
   command[0] = 0x0A;
   command[1] = 0x0D;
   int datalen = text.Length();
   SendType = NotRecieveAnswer;
   SendPacket(command,2,datalen,text.c_str(),true);
}

void CEpson442::PrintBigString(AnsiString text)
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

void CEpson442::Feed(int count)
{
    for(int i=1; i<=count; i++)
    {
        /*BYTE command[1];
        command[0] = 0x0A;
        SendType = NotRecieveAnswer;
        SendPacket(command,1,0,NULL);*/
        BYTE command[2];
        command[0] = 0x0A;
        command[1] = 0x0D;
        SendType = NotRecieveAnswer;
        SendPacket(command,2,0,NULL);
        SendCommand();
    }
}

void CEpson442::Init()
{
   BYTE command[2];
   command[0] = 0x1B;
   command[1] = 0x40;
   SendType = NotRecieveAnswer;
   SendPacket(command,2,0,NULL);
   SendCommand();
}

void CEpson442::Cut(bool option)//true - полный отрез false - неполный отрез чека
{
    UNREFERENCED_PARAMETER(option);
   BYTE command[2];
   command[0] = 0x1B;
   //command[1] = 0x69;
   command[1] = 0x6D;
   /*if (option)
     command[1] = 0x69;
   else
     command[1] = 0x6D;*/
   SendType = NotRecieveAnswer;
   SendPacket(command,2,0,NULL);
   SendCommand();
}

void CEpson442::GetState()
{
   DeviceState->StateCode = State = 0xFF;
   SubState = 0x0;
   StateDescr = "";
   SubStateDescr = "";
   BYTE command[3];
   command[0] = 0x10;
   command[1] = 0x04;
   command[2] = 4;
   SendType = RecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
   if (DeviceState == NULL)
      return;
   if (DeviceState->AnswerSize > 0)
     State = Answer[0];
   Log->Write((boost::format("Printer State byte 4=%1%") % GetStatusDescription((BYTE)State, command[2]).c_str()).str().c_str());

   if ((DeviceState->StateCode == 0)||(DeviceState->StateCode == 0xFF))
   {
       command[0] = 0x10;
       command[1] = 0x04;
       command[2] = 2;
       SendType = RecieveAnswer;
       SendPacket(command,3,0,NULL);
       SendCommand();
       if (DeviceState == NULL)
          return;
       if (DeviceState->AnswerSize > 0)
         State = Answer[0];
       Log->Write((boost::format("Printer State byte 2=%1%") % GetStatusDescription((BYTE)State,command[2]).c_str()).str().c_str());
   }

   if ((DeviceState->StateCode == 0)||(DeviceState->StateCode == 0xFF))
   {
       command[0] = 0x10;
       command[1] = 0x04;
       command[2] = 3;
       SendType = RecieveAnswer;
       SendPacket(command,3,0,NULL);
       SendCommand();
       if (DeviceState == NULL)
          return;
       if (DeviceState->AnswerSize > 0)
         State = Answer[0];
       Log->Write((boost::format("Printer State byte 3=%1%") % GetStatusDescription((BYTE)State,command[2]).c_str()).str().c_str());
   }

   //DeviceState->StateCode = State;
   DeviceState->StateDescription = GetStatusDescription((BYTE)State);
   DeviceState->SubStateCode = 0x00;
   DeviceState->SubStateDescription = "";

    if (DeviceState->StateCode == 0x00)
    {
        if (Error == 1)
        {
          Error = 0;
          DeviceState->StateChange = true;
          if (ChangeEvent)
            StateChanged(DeviceState);
        }
    }
}

std::string CEpson442::GetStatusDescription(BYTE StatusCode, BYTE ByteNo)
{
    std::string result = "";
    if (StatusCode == 0xFF)
    {
      result += "Принтер недоступен!";
      Error = 1;
      PrinterEnable = false;
      if (OldState != StatusCode)
      {
        OldState = DeviceState->StateCode;
        DeviceState->StateCode = StatusCode;
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        DeviceState->StateDescription = result;
        DeviceState->StateChange = true;
        if (ChangeEvent)
          StateChanged(DeviceState);
        return StateDescr = result;
      }
    }

    switch (ByteNo)
    {
        case 2:
            {
              BYTE code = (BYTE)StatusCode & (BYTE)0x20;
              if (code > 0)
              {
                result += "Печать остановлена";
                Error = 1;
                PrinterEnable = false;
                if (OldState != StatusCode)
                {
                  OldState = DeviceState->StateCode;
                  DeviceState->StateCode = 0x20;
                  DeviceState->OutStateCode = DSE_PAPERJAM;
                  DeviceState->StateDescription = result;
                  DeviceState->StateChange = true;
                  if (ChangeEvent)
                    StateChanged(DeviceState);
                  return StateDescr = result;
                }
              }
            }
            break;
        case 3:
            {
              BYTE code = (BYTE)StatusCode & (BYTE)0x20;
              if (code > 0)
              {
                result += "Невосстановимая ошибка";
                Error = 1;
                PrinterEnable = false;
                if (OldState != StatusCode)
                {
                  OldState = DeviceState->StateCode;
                  DeviceState->StateCode = 0x20;
                  DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
                  DeviceState->StateDescription = result;
                  DeviceState->StateChange = true;
                  if (ChangeEvent)
                    StateChanged(DeviceState);
                  return StateDescr = result;
                }
              }
            }
            break;
        case 4:
            {
              BYTE code = (BYTE)StatusCode & (BYTE)0x40;
              if (code > 0)
              {
                result += "Бумага закончилась ";
                Error = 1;
                PrinterEnable = false;
                if (OldState != StatusCode)
                {
                  OldState = DeviceState->StateCode;
                  DeviceState->StateCode = 0x40;
                  DeviceState->OutStateCode = DSE_NOTPAPER;
                  DeviceState->StateDescription = result;
                  DeviceState->StateChange = true;
                  if (ChangeEvent)
                    StateChanged(DeviceState);
                  return StateDescr = result;
                }
              }

              code = (BYTE)StatusCode & (BYTE)0x08;
              if (code > 0)
              {
                result += "Бумага скоро закончится ";
                Error = 1;
                PrinterEnable = true;
                if (OldState != StatusCode)
                {
                  OldState = DeviceState->StateCode;
                  DeviceState->StateCode = 0x08;
                  DeviceState->OutStateCode = DSE_NEARENDPAPER;
                  DeviceState->StateDescription = result;
                  DeviceState->StateChange = true;
                  if (ChangeEvent)
                    StateChanged(DeviceState);
                  return StateDescr = result;
                }
              }
            }
            break;
    }

    DeviceState->StateCode = 0;
    result += "OK";
    PrinterEnable = true;

    return StateDescr = result;
}

std::string CEpson442::GetStatusDescription(BYTE StatusCode)
{
    std::string result = "";
    if (StatusCode == 0xFF)
    {
      result += "Принтер недоступен!";
      Error = 1;
      PrinterEnable = false;
      if (OldState != StatusCode)
      {
        OldState = DeviceState->StateCode;
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        DeviceState->StateCode = StatusCode;
        DeviceState->StateDescription = result;
        DeviceState->StateChange = true;
        if (ChangeEvent)
          StateChanged(DeviceState);
      }
      return StateDescr = result;
    }

    BYTE code = (BYTE)StatusCode & (BYTE)0x40;
    if (code > 0)
    {
      result += "Бумага кончилась ";
      Error = 1;
      PrinterEnable = false;
      if (OldState != StatusCode)
      {
        OldState = DeviceState->StateCode;
        DeviceState->StateCode = 0x40;
        DeviceState->OutStateCode = DSE_NOTPAPER;
        DeviceState->StateDescription = result;
        DeviceState->StateChange = true;
        if (ChangeEvent)
          StateChanged(DeviceState);
      }
      return StateDescr = result;
    }

    code = (BYTE)StatusCode & (BYTE)0x08;
    if (code > 0)
    {
      result += "Бумага скоро кончится ";
      Error = 1;
      PrinterEnable = true;
      if (OldState != StatusCode)
      {
        OldState = DeviceState->StateCode;
        DeviceState->OutStateCode = DSE_NEARENDPAPER;
        DeviceState->StateCode = 0x08;
        DeviceState->StateDescription = result;
        DeviceState->StateChange = true;
        if (ChangeEvent)
          StateChanged(DeviceState);
      }
      return StateDescr = result;
    }

    DeviceState->StateCode = 0;
    result += "OK";
    PrinterEnable = true;

    return StateDescr = result;
}

void CEpson442::SelectFont()
{
   BYTE command[3];
   command[0] = 0x1B;
   command[1] = 0x4D;
   command[2] = 0;
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
}

void CEpson442::SetLeftMargin()
{
   BYTE command[4];
   command[0] = 0x1D;
   command[1] = 0x4C;
   command[2] = 10;
   command[3] = 0;
   SendType = NotRecieveAnswer;
   SendPacket(command,4,0,NULL);
   SendCommand();
}

void CEpson442::SetCharacterSet()
{
   BYTE command[3];
   command[0] = 0x1B;
   command[1] = 0x74;
   command[2] = 17;
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
}

void CEpson442::PrintCheck(AnsiString text, std::string barcode)
{
  GetState();
  SetCharacterSet();
  SelectFont();
  //SetLeftMargin();
  if (DeviceState->StateCode == 0xff)
    return;
  SetCodeTable();
  //Log->Write("Text to printer: "+ text);
  TStringList* strings = new TStringList();
  AnsiString delim = "\r\n";
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
  LinesCount = 0;
  while(true)
  {
    int pos = text.Pos(delim);
      if (pos == 0)
      {
          AnsiString subtext = text;
          memset(_subtext,0,1024);
          CharToOem(subtext.c_str(), _subtext);
          PrintString(AnsiString(_subtext));
          LinesCount++;
          break;
      }
    AnsiString subtext = text.SubString(0,pos-1);
    text = text.SubString(pos+1,text.Length()-pos);
    //if (!subtext.IsEmpty())
    //{
      memset(_subtext,0,1024);
      CharToOem(subtext.c_str(), _subtext);
      PrintString(AnsiString(_subtext));
      LinesCount++;
    //}
  }
  //for(int i=0; i<7; i++)
    //Feed();
  FeedToMinLinesCount();
  Cut(true);
  delete strings;
}

void CEpson442::SetCodeTable()
{
   BYTE command[3];
   command[0] = 0x1B;
   command[1] = 0x74;
   command[2] = 0x07;
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
}

void CEpson442::SetCharacterSize()
{
   BYTE command[3];
   command[0] = 0x1D;
   command[1] = 0x21;
   command[2] = 0x77;
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
}

bool CEpson442::IsItYou()
{
   BYTE command[3];
   command[0] = 0x10;
   command[1] = 0x04;
   command[2] = 4;
   SendType = RecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
   if (DeviceState == NULL)
      return false;
   if (DeviceState->AnswerSize == 0)
      return false;
   return true;
}

int CEpson442::Initialize()
{
    try
    {
      Init();
      GetState();
      SetCodeTable();
      SetInitialized();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

