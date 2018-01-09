//---------------------------------------------------------------------------
#pragma hdrstop
#include "CitizenPPU232Class.h"
#include "globals.h"
#include "boost/format.hpp"
#include "DeviceThread.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

CCitizenPPU232::CCitizenPPU232(int ComPort,int BaudRate,TLogClass* _Log) : CPrinter(ComPort,_Log, "CitizenPPU232")
{
  DataLength = 1;
  MinLinesCount = 1;
  COMParameters->Parity = NOPARITY;
  COMParameters->BaudRate = 19200;
  if (BaudRate > 0)
      COMParameters->BaudRate = BaudRate;
  if (Port)
      Port->ReopenPort();
  LoggingErrors = true;
  State = 0x00;
  OldState = 0x00;
  SubState = 0x00;
  OldSubState = 0x00;
  DeviceName = "CitizenPPU232";
  //ThreadLifeTime = 5000;
}

CCitizenPPU232::~CCitizenPPU232()
{
}

/*void CCitizenPPU232::SendCommand()
{
  DeviceThread = new TDeviceThread(true,false);
  Start();
  delete DeviceThread;
  DeviceThread = NULL;
}*/

void CCitizenPPU232::PrintString(AnsiString text)
{
  PrintLine(text);
  SendCommand();
}

void CCitizenPPU232::PrintCheck(TStringList* Text)
{
  char subtext[100];
  for(int i=0; i<Text->Count; i++)
  {
      memset(subtext,0,100);
      CharToOem(Text->Strings[i].c_str(), subtext);
      PrintString(Text->Strings[i]);
  }
  Cut();
}

void CCitizenPPU232::SendPacket(BYTE* command, int bytes_count,int datalen, BYTE* data, bool datafirst)
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

    CommandSize = bytes_count+datalen;
  }
  __finally
  {
  }
}

std::string CCitizenPPU232::GetStateDescription()
{
  return GetStatusDescription((BYTE)DeviceState->StateCode);
}

void CCitizenPPU232::PrintLine(AnsiString text)
{
   BYTE command[1];
   command[0] = 0x0A;
   int datalen = text.Length();
   SendType = NotRecieveAnswer;
   SendPacket(command, 1, datalen, (BYTE *)text.c_str(), true);
}

void CCitizenPPU232::PrintBigString(AnsiString text)
{
   ClearCommand();
   ClearAnswer();
   Command[0] = 0x1B;
   Command[1] = 0x21;
   Command[2] = 0x10;
   memcpy(&Command[3],text.c_str(),text.Length());
   CommandSize = 3 + text.Length();
   Command[CommandSize] = 0x0A;
   CommandSize++;
   SendType = NotRecieveAnswer;
   SendCommand();
}

void CCitizenPPU232::Feed(int count)
{
    for(int i=1; i<=count; i++)
    {
        BYTE command[1];
        command[0] = 0x0A;
        SendType = NotRecieveAnswer;
        SendPacket(command,1,0,NULL);
        SendCommand();
    }
}

void CCitizenPPU232::ShriftOptionsEx(BYTE option)//true - заводской шрифт false - пользовательский
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

void CCitizenPPU232::SetUnderline(bool option)
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

void CCitizenPPU232::SetBold(bool option)
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

void CCitizenPPU232::SetDoublePrint(bool option)
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

void CCitizenPPU232::Init()
{
   BYTE command[2];
   command[0] = 0x1B;
   command[1] = 0x40;
   SendType = NotRecieveAnswer;
   SendPacket(command,2,0,NULL);
   SendCommand();
}

void CCitizenPPU232::Cut()
{
   BYTE command[4];
   command[0] = 0x1D;
   command[1] = 0x56;
   command[2] = 0x42;
   command[3] = 0x00;
   SendType = NotRecieveAnswer;
   SendPacket(command,4,0,NULL);
   SendCommand();
}

void CCitizenPPU232::GetState()
{
   State = 0xFF;
   SubState = 0x0;
   StateDescr = "";
   SubStateDescr = "";
   BYTE command[3];
   DeviceState->SubStateDescription = "";
   DeviceState->StateDescription = "";

   command[0] = 0x10;
   command[1] = 0x04;
   command[2] = 0x04;
   SendType = RecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
   if (DeviceState == NULL)
      return;
   if (DeviceState->AnswerSize > 0)
     DeviceState->StateCode = State = Answer[0];
   DeviceState->StateDescription = GetStatusDescription(4);
   Log->Write((boost::format("Printer State1 = %1%") % DeviceState->StateDescription.c_str()).str().c_str());

   if (DeviceState->OutStateCode != DSE_OK) return;

   command[0] = 0x10;
   command[1] = 0x04;
   command[2] = 0x03;
   SendType = RecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
   if (DeviceState == NULL)
      return;
   if (DeviceState->AnswerSize > 0)
     DeviceState->StateCode = State = Answer[0];
   DeviceState->StateDescription = GetStatusDescription(3);
   Log->Write((boost::format("Printer State2 = %1%") % DeviceState->StateDescription.c_str()).str().c_str());

    DeviceState->SubStateCode = 0x00;
    if (DeviceState->OutStateCode == DSE_OK)
    {
        if (DeviceState->OldOutStateCode != DSE_OK)
        {
          /*DeviceState->OldOutStateCode = DSE_OK;
          DeviceState->StateChange = true;
          if (ChangeEvent)
            StateChanged(DeviceState);*/
          ChangeDeviceState();
        }
    }
}

std::string CCitizenPPU232::GetStatusDescription(BYTE ByteNo)
{
    std::string result = "";
    BYTE StatusCode = (BYTE)DeviceState->StateCode;

    if (StatusCode == 0x00)
    {
      result += "OK";
      PrinterEnable = true;
      DeviceState->OutStateCode = DSE_OK;
      DeviceState->StateDescription = result;
      return StateDescr = result;
    }
    if (StatusCode == 0xFF)
    {
      result += "Принтер недоступен!";
      Error = 1;
      PrinterEnable = false;
      if (OldState != StatusCode)
      {
        OldState = DeviceState->StateCode;
        DeviceState->StateDescription = result;
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        DeviceState->StateChange = true;
        //if (ChangeEvent)
          //StateChanged(DeviceState);
        ChangeDeviceState();
      }
      return StateDescr = result;
    }

    BYTE code = 0;
    switch (ByteNo)
    {
        case 4:
        {
            code = (BYTE)StatusCode & (BYTE)0x20;
            if (code > 0)
            {
              result += "Бумага кончилась ";
              Error = 1;
              PrinterEnable = false;
              if (OldState != StatusCode)
              {
                OldState = DeviceState->StateCode;
                DeviceState->OutStateCode = DSE_NOTPAPER;
                DeviceState->StateDescription = result;
                DeviceState->StateChange = true;
                //if (ChangeEvent)
                  //StateChanged(DeviceState);
                ChangeDeviceState();
              }
              return StateDescr = result;
            }

            code = (BYTE)(StatusCode & 0x04) | (BYTE)(StatusCode & 0x08);
            if (code > 0)
            {
              result += "Бумага скоро кончится ";
              Error = 1;
              PrinterEnable = true;
              if (OldState != StatusCode)
              {
                OldState = DeviceState->StateCode;
                DeviceState->OutStateCode = DSE_NEARENDPAPER;
                DeviceState->StateDescription = result;
                DeviceState->StateChange = true;
                //if (ChangeEvent)
                  //StateChanged(DeviceState);
                ChangeDeviceState();
              }
              return StateDescr = result;
            }
        }

        case 3:
        {
            code = (BYTE)(StatusCode & 0x04) | (BYTE)(StatusCode & 0x08) | (BYTE)(StatusCode & 0x20);
            if (code > 0)
            {
              result += "Аппаратная ошибка принтера ";
              Error = 1;
              PrinterEnable = true;
              if (OldState != StatusCode)
              {
                OldState = DeviceState->StateCode;
                DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
                DeviceState->StateDescription = result;
                DeviceState->StateChange = true;
                //if (ChangeEvent)
                  //StateChanged(DeviceState);
                ChangeDeviceState();
              }
              return StateDescr = result;
            }
        }
    }


    return StateDescr = result;
}

void CCitizenPPU232::PrintCheck(AnsiString text, std::string barcode)
{
  GetState();
  if (DeviceState->StateCode == 0xff)
    return;
  SetCodeTable();
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
          PrintString(AnsiString(_subtext));
          LinesCount++;
          break;
      }
    AnsiString subtext = text.SubString(0,pos-1);
    text = text.SubString(pos+1,text.Length()-pos);
    memset(_subtext,0,1024);
    CharToOem(subtext.c_str(), _subtext);
    LinesCount++;
    PrintString(AnsiString(_subtext));
  }
  FeedToMinLinesCount();
  Cut();
  delete strings;
}

void CCitizenPPU232::SetCodeTable()
{
   BYTE command[3];
   command[0] = 0x1B;
   command[1] = 0x74;
   command[2] = 0x00;
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
}

bool CCitizenPPU232::IsPrinterEnable()
{
  GetState();
  return PrinterEnable;
}


bool CCitizenPPU232::IsItYou()
{
   BYTE command[3];
   command[0] = 0x1D;
   command[1] = 0x49;
   command[2] = 0x01;
   SendType = RecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
   if (DeviceState == NULL)
      return false;
   if (DeviceState->AnswerSize == 0)
      return false;
   else
      if (Answer[0] == 0x75)
      {
          Log->Write("Printer type is CCitizenPPU232");
          return true;
      }
   return true;
}

int CCitizenPPU232::Initialize()
{
    try
    {
      Init();
      GetState();
      //SetCodeTable();
      SetInitialized();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}


