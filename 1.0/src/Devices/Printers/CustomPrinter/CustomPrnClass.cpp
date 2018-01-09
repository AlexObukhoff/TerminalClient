//---------------------------------------------------------------------------
#pragma hdrstop
#include "CustomPrnClass.h"
#include "globals.h"
#include "boost/format.hpp"
#include "DeviceThread.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

CCustomPrn::CCustomPrn(int ComPort,int BaudRate,TLogClass* _Log) : CPrinter(ComPort,_Log, "CustomPrn")
{
  DataLength = 1;
  delete Port;
  Port = NULL;
  COMParameters->Parity = NOPARITY;
  COMParameters->BaudRate = 19200;
  if (BaudRate > 0)
      COMParameters->BaudRate = BaudRate;
  Port = new TComPort(COMParameters,Log,true);
  LoggingErrors = true;
  //Init();
  DeviceName = "custom";
  Error = 0;
  //GetState();
}

CCustomPrn::~CCustomPrn()
{
}

void CCustomPrn::SendCommand()
{
  DeviceThread = new TDeviceThread(true,false);
  Start();
  delete DeviceThread;
  DeviceThread = NULL;
}

void CCustomPrn::PrintString(AnsiString text)
{
  PrintLine(text);
  SendCommand();
}

void CCustomPrn::PrintCheck(TStringList* Text)
{
  char subtext[100];
  ClearDispenser();
  for(int i=0; i<Text->Count; i++)
  {
      memset(subtext,0,100);
      try
      {
        CharToOem(Text->Strings[i].c_str(), subtext);
      }
      catch(...)
      {
          ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        if (Log)
          Log->Write("PrintCheck() CharToOem Exception");
      }
      try
      {
        PrintString(AnsiString(subtext));
      }
      catch(...)
      {
          ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        if (Log)
          Log->Write("PrintCheck() PrintString Exception");
      }
      //PrintBigString(AnsiString(subtext));
  }
  Cut();
  Dispense();
  int TimeOut=1000;
  while (TimeOut>0) {
      Application->ProcessMessages();
      Sleep(10);
      TimeOut--;
      }
//  Sleep(5000);
  Retracting();
}

void CCustomPrn::SendPacket(BYTE* command, int bytes_count,int datalen, BYTE* data, bool datafirst)
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

std::string CCustomPrn::GetStateDescription()
{
  return GetStatusDescription((BYTE)DeviceState->StateCode,0);
}

void CCustomPrn::SetCodePage()
{
   BYTE command[3];
   command[0] = 0x1B;
   command[1] = 0x74;
   command[2] = 0;
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
}

void CCustomPrn::SetCharacterSet()
{
   BYTE command[3];
   command[0] = 0x1B;
   command[1] = 0x52;
   command[2] = 0x00;
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
}

/*void CCustomPrn::PrintLine(AnsiString text)
{
   BYTE command[2];
   command[0] = 0x0D;
   command[1] = 0x0A;
   int datalen = text.Length();
   SendType = NotRecieveAnswer;
   SendPacket(command,2,datalen,text.c_str(),true);
}*/

void CCustomPrn::PrintLine(AnsiString text)
{
//        Log->Write("CCustomPrn::PrintLine "+text);
         BYTE command[1];
   command[0] = 0x0A;
   int datalen = text.Length();
   SendType = NotRecieveAnswer;
   SendPacket(command,1,datalen,text.c_str(),true);
}
void CCustomPrn::PrintBigString(AnsiString text)
{
   ClearCommand();
   ClearAnswer();
   Command[0] = 0x1B;
   Command[1] = 0x69;
   Command[2] = 1;
   Command[3] = 1;
   memcpy(&Command[4],text.c_str(),text.Length());
   CommandSize = 4 + text.Length();
   Command[CommandSize] = 0x0A;
   Command[CommandSize+1] = 0x0D;
   CommandSize += 2;
   SendType = NotRecieveAnswer;
   SendCommand();
}

void CCustomPrn::Feed(int count)
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

void CCustomPrn::ShriftOptionsEx(BYTE option)//true - заводской шрифт false - пользовательский
{
   BYTE command[3];
   command[0] = 0x1D;
   command[1] = 0x21;
   command[2] = option;
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
}

void CCustomPrn::Init()
{
   BYTE command[2];
   command[0] = 0x1B;
   command[1] = 0x40;
   SendType = NotRecieveAnswer;
   SendPacket(command,2,0,NULL);
   SendCommand();
}

void CCustomPrn::Cut()
{
   BYTE command[2];
   command[0] = 0x1B;
   command[1] = 0x69;
   SendType = NotRecieveAnswer;
   SendPacket(command,2,0,NULL);
   SendCommand();
}

void CCustomPrn::GetState()
{
   State = 0xFF;
   SubState = 0x00;
   StateDescr = "";
   SubStateDescr = "";
   BYTE command[3];
   std::string Descr;
   //command[0] = 0x10;
   //command[1] = 0x04;
   command[0] = 0x1B;
   command[1] = 0x76;
   SendType = RecieveAnswer;
   SendPacket(command,2,0,NULL);
   SendCommand();
   if (DeviceState->AnswerSize > 0)
     State = Answer[0];

   DeviceState->StateCode = State;
   //DeviceState->StateCode = 0x0f;

   Descr = GetStatusDescription((BYTE)DeviceState->StateCode,0);
   Log->Write((boost::format("Printer State=%1%") % Descr.c_str()).str().c_str());

  //DeviceState->StateDescription = Descr;
  DeviceState->SubStateCode = 0x00;
  DeviceState->SubStateDescription = "";
  if (Error == 1)
      return;
  if (DeviceState->StateCode == 0xff)
      return;

//  int State1 = State;

   command[0] = 0x10;
   command[1] = 0x04;
   command[2] = 20;
   SendType = RecieveAnswer;
   DataLength = 6;
   SendPacket(command,3,0,NULL);
   SendCommand();
   DataLength = 1;
   State = 0xFF;
   if (DeviceState->AnswerSize > 0)
     State = Answer[4];

   if (State == 0xff)
   {
      DeviceState->StateCode = State;
      Descr = GetStatusDescription((BYTE)DeviceState->StateCode,5);
      return;
   }

   //======== Paper near end disable
   State &= 0x7C;
   //======== Paper near end disable

//    int State2 = State;
    /*if ((State1 == 0x00)&&(State2 == 0x00))
    {
      if (Error == 1)
      {
        Error = 0;
        DeviceState->OutStateCode = DSE_OK;
        Log->Write("Printer is OK!");
        DeviceState->StateChange = true;
        if (ChangeEvent)
          StateChanged(DeviceState);
      }
    }*/
   if (State == 0)
      return;

   DeviceState->StateCode = State;
   Descr = GetStatusDescription((BYTE)DeviceState->StateCode,5);
   //DeviceState->SubStateDescription = Descr;
   Log->Write((boost::format("Printer SubState=%1%") % Descr.c_str()).str().c_str());

}


void CCustomPrn::PrintCheck(AnsiString text, std::string barcode)
{
//        Log->Write("CCustomPrn::PrintCheck "+text);
//        BYTE xON = 0x11;
  BYTE xOFF = 0x13;
  DWORD count = 0;
  if (DeviceThread)
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
  }

  GetState();
  Retracting();

  //if (State != 0)
    //return;
  SetCodePage();
  ClearDispenser();
  //SetLeftMargin();
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
      //PrintBigString(AnsiString(_subtext));
      //PrintBigString(subtext);
      LinesCount++;
      PrintString(_subtext);
    //}
  }

  //for(int i=0; i<7; i++)
      //Feed();
  if(!barcode.empty())
  {
      SetBarCodeHeight(0xA0);
      SetBarCodeHRIposition(0x01);
      SetBarCodeFont(0x01);
      SetBarCodeWidth(0x02);
      PrintBarCode(barcode);
  }
  FeedToMinLinesCount();
  Cut();
  Dispense();
  //PrintCheck(strings);
  delete strings;
}

void CCustomPrn::ClearDispenser()
{
   BYTE command[3];
   command[0] = 0x1D;
   command[1] = 0x65;
   command[2] = 0x05;
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
}

void CCustomPrn::Dispense()
{
   BYTE command[4];
   command[0] = 0x1D;
   command[1] = 0x65;
   command[2] = 0x03;
   command[3] = 0x1e;
//   command[3] = 0x04;
   SendType = NotRecieveAnswer;
   SendPacket(command,4,0,NULL);
   SendCommand();
}

void CCustomPrn::Retracting()
{
   BYTE command[3];
   command[0] = 0x1D;
   command[1] = 0x65;
   command[2] = 0x02;
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
}

void CCustomPrn::SetBarCodeHeight(BYTE n)  //ставит высоту, 255 - максимум
{
   n = n ? n : 0xFF;

   BYTE command[3];
   command[0] = 0x1D;
   command[1] = 0x68;
   command[2] = n;
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
}

void CCustomPrn::SetBarCodeWidth(BYTE n)   //задает ширину минимально полоски
{
   n = (n < 7) ? n : 1;

   BYTE command[3];
   command[0] = 0x1D;
   command[1] = 0x77;
   command[2] = n;              //0..6 - 1/8, 1/4, 3/8, 1/2, 5/8, 3/4, 7/8 [мм]
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
}
void CCustomPrn::SetBarCodeFont(BYTE n)  //задает фонт, 2 типа: А или В
{
   n = ((n < 2) || ((n < 50) && (n > 47))) ? n : 1;

   BYTE command[3];
   command[0] = 0x1D;
   command[1] = 0x66;
   command[2] = n;               //== 0-1 или 48-49
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
}

void CCustomPrn::SetBarCodeHRIposition(BYTE n)  //печать цифирек баркода
{
   n = ((n < 4) || ((n < 52) && (n > 47))) ? n : 1;

   BYTE command[3];
   command[0] = 0x1D;
   command[1] = 0x48;
   command[2] = 1;       //== 0..3 или 48..51: не печатать/над/под/под и над
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand(); 
}

void CCustomPrn::PrintBarCode(std::string text)
{
    try
    {
        const int text_size = 25;
        const int command_size = 3 + 1 + text_size;
        BYTE command[command_size];
        memset(command, 0, command_size);

        command[0] = 0x1D;
        command[1] = 0x6B;
        command[2] = 73;
        command[3] = text_size;

        std::string filler = " ";
        text = "{B" + text;
        memcpy(&command[4], fill(text, filler, text_size, SIDE::LEFT).c_str(), text_size);
        SendType = NotRecieveAnswer;
        SendPacket(command,command_size,0,NULL);
        SendCommand();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

std::string CCustomPrn::GetStatusDescription(BYTE StatusCode, int ByteNo)
{
    std::string result = "";
    DeviceState->SubStateCode = 0x00;
    DeviceState->SubStateDescription = "";
    Log->Write("Printer Binary State = " + StatusCode);

    if (StatusCode == 0x00)
    {
      result = "OK";
      PrinterEnable = true;
      DeviceState->StateDescription = result;
      DeviceState->OutStateCode = DSE_OK;
      Error = 0;
      if (DeviceState->OldOutStateCode != DeviceState->OutStateCode)
      {
        DeviceState->OldOutStateCode = DeviceState->OutStateCode;
        OldState = DeviceState->StateCode;
        DeviceState->StateCode = StatusCode;
        DeviceState->StateDescription = result;
        DeviceState->StateChange = true;
        Log->Write("State sended: printer availible");
        if (ChangeEvent)
          StateChanged(DeviceState);
      }
      return StateDescr = result;
    }
    if (StatusCode == 0xFF)
    {
      result = " Принтер недоступен";
      Error = 1;
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      PrinterEnable = false;
      //if (OldState != StatusCode)
      if (DeviceState->OldOutStateCode != DeviceState->OutStateCode)
      {
        DeviceState->OldOutStateCode = DeviceState->OutStateCode;
        OldState = DeviceState->StateCode;
        DeviceState->StateCode = StatusCode;
        DeviceState->StateDescription = result;
        DeviceState->StateChange = true;
        Log->Write("State sended: printer not availible");
        if (ChangeEvent)
          StateChanged(DeviceState);
      }
      return StateDescr = result;
    }
    if (ByteNo == 0)
    {
        int code = StatusCode&0x0C;
        if (code > 0)
        {
          result = " Бумага закончилась";
          Error = 1;
          DeviceState->OutStateCode = DSE_NOTPAPER;
          PrinterEnable = false;
          //if (OldState != StatusCode)
          if (DeviceState->OutStateCode != DeviceState->OldOutStateCode)
          {
            DeviceState->OldOutStateCode = DeviceState->OutStateCode;
            OldState = DeviceState->StateCode;
            DeviceState->StateCode = StatusCode;
            DeviceState->StateDescription = result;
            DeviceState->StateChange = true;
            if (ChangeEvent)
            {
              Log->Write("State sended: paper end");
              StateChanged(DeviceState);
            }
          }
          return StateDescr = result;
        }

        //paper near end
        code = StatusCode&0x03;
        if (code > 0)
        {
          result = " OK";
          Error = 1;
          PrinterEnable = true;
          DeviceState->OutStateCode = DSE_OK;
          if (DeviceState->OutStateCode != DeviceState->OldOutStateCode)
          {
            DeviceState->OldOutStateCode = DeviceState->OutStateCode;
            OldState = DeviceState->StateCode;
            DeviceState->StateCode = StatusCode;
            DeviceState->StateDescription = result;
            DeviceState->StateChange = true;
            if (ChangeEvent)
              StateChanged(DeviceState);
          }
          return StateDescr = result;
        }
        /*code = StatusCode&0x03;
        if (code > 0)
        {
          result = " Бумага заканчивается";
          Error = 1;
          PrinterEnable = true;
          DeviceState->OutStateCode = DSE_NEARENDPAPER;
          if (DeviceState->OutStateCode != DeviceState->OldOutStateCode)
          {
            DeviceState->OldOutStateCode = DeviceState->OutStateCode;
            OldState = DeviceState->StateCode;
            DeviceState->StateCode = StatusCode;
            DeviceState->StateDescription = result;
            DeviceState->StateChange = true;
            if (ChangeEvent)
              StateChanged(DeviceState);
          }
          return StateDescr = result;
        }*/
    }
    if (ByteNo == 5)
    {
        int code = StatusCode&0x01;
        if (code > 0)
        {
          result = " Ошибка температуры печатающей головки";
          Error = 1;
          PrinterEnable = true;
          //DeviceState->OutStateCode = DSE_MAINERROR;
          if (DeviceState->OutStateCode != DeviceState->OldOutStateCode)
          {
            DeviceState->OldOutStateCode = DeviceState->OutStateCode;
            OldState = DeviceState->StateCode;
            DeviceState->StateCode = StatusCode;
            DeviceState->StateDescription = result;
            DeviceState->StateChange = true;
            if (ChangeEvent)
              StateChanged(DeviceState);
          }
          return StateDescr = result;
        }

        code = StatusCode&0x02;
        if (code > 0)
        {
          result = " Ошибка RS232";
          Error = 1;
          PrinterEnable = true;
          DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
          if (DeviceState->OutStateCode != DeviceState->OldOutStateCode)
          {
            DeviceState->OldOutStateCode = DeviceState->OutStateCode;
            OldState = DeviceState->StateCode;
            DeviceState->StateCode = StatusCode;
            DeviceState->StateDescription = result;
            DeviceState->StateChange = true;
            if (ChangeEvent)
              StateChanged(DeviceState);
          }
          return StateDescr = result;
        }

        code = StatusCode&0x08;
        if (code > 0)
        {
          result = " Сбой питания";
          Error = 1;
          PrinterEnable = true;
          DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
          if (DeviceState->OutStateCode != DeviceState->OldOutStateCode)
          {
            DeviceState->OldOutStateCode = DeviceState->OutStateCode;
            OldState = DeviceState->StateCode;
            DeviceState->StateCode = StatusCode;
            DeviceState->StateDescription = result;
            DeviceState->StateChange = true;
            if (ChangeEvent)
              StateChanged(DeviceState);
          }
          return StateDescr = result;
        }

        code = StatusCode&0x20;
        if (code > 0)
        {
          result = " Нераспознанная команда";
          Error = 1;
          PrinterEnable = true;
          //DeviceState->OutStateCode = DSE_MAINERROR;
          if (DeviceState->OutStateCode != DeviceState->OldOutStateCode)
          {
            DeviceState->OldOutStateCode = DeviceState->OutStateCode;
            OldState = DeviceState->StateCode;
            DeviceState->StateCode = StatusCode;
            DeviceState->StateDescription = result;
            DeviceState->StateChange = true;
            if (ChangeEvent)
              StateChanged(DeviceState);
          }
          return StateDescr = result;
        }

        code = StatusCode&0x40;
        if (code > 0)
        {
          result = " Бумага зажёвана";
          Error = 1;
          DeviceState->OutStateCode = DSE_PAPERJAM;
          PrinterEnable = false;
          if (DeviceState->OutStateCode != DeviceState->OldOutStateCode)
          {
            DeviceState->OldOutStateCode = DeviceState->OutStateCode;
            OldState = DeviceState->StateCode;
            DeviceState->StateCode = StatusCode;
            DeviceState->StateDescription = result;
            DeviceState->StateChange = true;
            if (ChangeEvent)
              StateChanged(DeviceState);
          }
          return StateDescr = result;
        }
    }

    return StateDescr = result;
}

void CCustomPrn::SetLeftMargin()
{
   BYTE command[4];
   command[0] = 0x1D;
   command[1] = 0x4C;
   command[2] = 0x0A;
   command[3] = 0x00;
   SendType = NotRecieveAnswer;
   SendPacket(command,4,0,NULL);
   SendCommand();
}

bool CCustomPrn::IsPrinterEnable()
{
  GetState();
  return PrinterEnable;
}

bool CCustomPrn::IsItYou()
{
   BYTE command[3];
   AnsiString Descr;
   command[0] = 0x1B;
   command[1] = 0x76;
   SendType = RecieveAnswer;
   SendPacket(command,2,0,NULL);
   SendCommand();
   if (DeviceState->AnswerSize > 0)
     return true;
   return false;
}

int CCustomPrn::Initialize()
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

