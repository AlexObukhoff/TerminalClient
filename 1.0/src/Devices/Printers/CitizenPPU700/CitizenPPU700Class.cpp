//---------------------------------------------------------------------------
#pragma hdrstop
#include "CitizenPPU700Class.h"
#include "globals.h"
#include "boost/format.hpp"
#include "DeviceThread.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

CCitizenPPU700::CCitizenPPU700(int ComPort,int BaudRate,TLogClass* _Log) : CPrinter(ComPort,_Log, "CitizenPPU700")
{
  DataLength = 1;
  delete Port;
  COMParameters->Parity = NOPARITY;
  COMParameters->BaudRate = 19200;
  if (BaudRate > 0)
      COMParameters->BaudRate = BaudRate;
  Port = new TComPort(COMParameters,Log,true);
  LoggingErrors = false;
  //Init();
  //GetState();
  //SetCodeTable();
  State = 0x00;
  OldState = 0x00;
  SubState = 0x00;
  OldSubState = 0x00;
  DeviceName = "CitizenPPU700";
  MinLinesCount += 3;
  //SetCharacterSize();
}

CCitizenPPU700::~CCitizenPPU700()
{
}

void CCitizenPPU700::SendCommand()
{
  DeviceThread = new TDeviceThread(true,false);
  Start();
  delete DeviceThread;
  DeviceThread = NULL;
}

void CCitizenPPU700::PrintString(AnsiString text)
{
  PrintLine(text);
  SendCommand();
}

void CCitizenPPU700::PrintCheck(TStringList* Text)
{
  char subtext[100];
  for(int i=0; i<Text->Count; i++)
  {
      memset(subtext,0,100);
      CharToOem(Text->Strings[i].c_str(), subtext);
      PrintString(Text->Strings[i]);
      //PrintString(AnsiString(subtext));
      //PrintBigString(AnsiString(subtext));
  }
  Cut(true);
}

void CCitizenPPU700::SendPacket(BYTE* command, int bytes_count,int datalen, BYTE* data, bool datafirst)
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

std::string CCitizenPPU700::GetStateDescription()
{
  return GetStatusDescription((BYTE)DeviceState->StateCode);
}

void CCitizenPPU700::PrintLine(AnsiString text)
{
   //SelectFont();
   /*BYTE command[200];
   memset(command,0,200);
   int ind = text.Length();
   memcpy(text.c_str(),command,text.Length());
   command[ind] = 0x0A;
   SendType = NotRecieveAnswer;
   SendPacket(command,text.Length()+1,0,NULL);
   SendCommand();*/
   BYTE command[2];
   command[0] = 0x0A;
   command[1] = 0x0D;
   int datalen = text.Length();
   SendType = NotRecieveAnswer;
   SendPacket(command,2,datalen,text.c_str(),true);
}

void CCitizenPPU700::PrintBigString(AnsiString text)
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

void CCitizenPPU700::SetBarCodeHeight(BYTE n)  //ставит высоту, 255 - максимум
{
   n = n ? n : 0xFF;

   BYTE command[2];
   command[0] = 0x1D;
   command[1] = 0x68;

   int datalen = 1;
   BYTE data[1];
   data[0] = n;
   SendType = NotRecieveAnswer;
   SendPacket(command,2,datalen,data);
   SendCommand();
}

void CCitizenPPU700::SetBarCodeWidth(BYTE n)   //задает ширину минимально полоски
{
   n = ((n >= 2) && (n <= 6)) ? n : 2;

   BYTE command[2];
   command[0] = 0x1D;
   command[1] = 0x77;

   int datalen = 1;
   BYTE data[1];
   data[0] = n;             //2..6 - ширина в у.е.
   SendType = NotRecieveAnswer;
   SendPacket(command,2,datalen,data);
   SendCommand();
}
void CCitizenPPU700::SetBarCodeFont(BYTE n)  //задает фонт, 3 типа: А, B или C
{
   n = ((n <= 2) || ((n >=48 ) && (n <= 50))) ? n : 2;

   BYTE command[2];
   command[0] = 0x1D;
   command[1] = 0x66;

   int datalen = 1;
   BYTE data[1];
   data[0] = n;              //== 0-2 или 48-50
   SendType = NotRecieveAnswer;
   SendPacket(command,2,datalen,data);
   SendCommand();
}

void CCitizenPPU700::SetBarCodeHRIposition(BYTE n)  //печать цифирек баркода
{
   n = ((n <= 3) || ((n >= 48) && (n <= 51))) ? n : 1;

   BYTE command[2];
   command[0] = 0x1D;
   command[1] = 0x48;

   int datalen = 1;
   BYTE data[1];
   data[0] = n;        //== 0..3 или 48..51: не печатать/над/под/под и над
   SendType = NotRecieveAnswer;
   SendPacket(command,2,datalen,data);
   SendCommand();
}

void CCitizenPPU700::PrintBarCode(std::string text)
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

        text = "{B" + text;
        memcpy(&command[4], fill(text, " ", text_size, SIDE::LEFT).c_str(), text_size);
        SendType = NotRecieveAnswer;
        SendPacket(command,command_size,0,NULL);
        SendCommand();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CCitizenPPU700::Feed(int count)
{
   /*BYTE command[2];
   command[0] = 0x0D;
   command[1] = 0x0A;
   SendType = NotRecieveAnswer;
   SendPacket(command,1,0,NULL,true);
   SendCommand();*/
    for(int i=1; i<=count; i++)
    {
        BYTE command[2];
        command[0] = 0x0A;
        command[1] = 0x0D;
        SendType = NotRecieveAnswer;
        SendPacket(command,2,0,NULL);
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
void CCitizenPPU700::ShriftOptionsEx(BYTE option)//true - заводской шрифт false - пользовательский
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

void CCitizenPPU700::SetUnderline(bool option)
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

void CCitizenPPU700::SetBold(bool option)
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

void CCitizenPPU700::SetDoublePrint(bool option)
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

void CCitizenPPU700::Init()
{
   BYTE command[2];
   command[0] = 0x1B;
   command[1] = 0x40;
   SendType = NotRecieveAnswer;
   SendPacket(command,2,0,NULL);
   SendCommand();
}

void CCitizenPPU700::Cut(bool option)//true - полный отрез false - неполный отрез чека
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

bool CCitizenPPU700::IsPaperInPresenter()
{
   BYTE command[3];
   BYTE Code = 0;
   command[0] = 0x1D;
   command[1] = 0x72;
   command[2] = 0x01;
   SendType = RecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
   if (DeviceState == NULL)
      return false;
   if (DeviceState->AnswerSize > 0)
     Code = Answer[0];
   if (LoggingErrors)
     Log->Write((boost::format("IsPaperInPresenter() Code=%1%") % Code).str().c_str());
   Code &= 0x08;
   if (Code > 0)
      return true;
   else
      return false;
}



void CCitizenPPU700::GetState()
{
   Init();
   State = 0xFF;
   SubState = 0x0;
   StateDescr = "";
   SubStateDescr = "";
   Error = 0;
   BYTE command[2];
   command[0] = 0x1B;
   command[1] = 0x76;
   SendType = RecieveAnswer;
   SendPacket(command,2,0,NULL);
   SendCommand();
   if (DeviceState == NULL)
      return;
   if (DeviceState->AnswerSize > 0)
     State = Answer[0];
   Log->Write((boost::format("Printer State=%1%") % GetStatusDescription((BYTE)State).c_str()).str().c_str());

  DeviceState->StateCode = State;
  DeviceState->StateDescription = GetStatusDescription((BYTE)State);
  DeviceState->SubStateCode = 0x00;
  DeviceState->SubStateDescription = "";
//=====================new 27-04-2007===========================================
    std::string result = "";
    switch(DeviceState->DSR_CTS)
    {
        case DsrCtsOff:
            {
                result = "DSR Off, CTS Off";
                Log->Write(result.c_str());
                Error = 1;
                PrinterEnable = false;
                DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
                if (DeviceState->OldOutStateCode != DeviceState->OutStateCode)
                {
                  //DeviceState->OldOutStateCode = DeviceState->OutStateCode;
                  //DeviceState->OldStateCode = DeviceState->StateCode;
                  DeviceState->StateDescription = "Выключена сигнальная линия";
                  DeviceState->StateChange = true;
                  //if (ChangeEvent)
                    //StateChanged(DeviceState);
                  ChangeDeviceState();
                }
                return;
            }
//            break;
        case DsrCtsOn:
            result = "DSR On, CTS On";
            Log->Write(result.c_str());
            break;
        case DsrOn:
            result = "Only DSR On, CTS Off";
            Log->Write(result.c_str());
            break;
        case CtsOn:
            {
                result = "Only CTS On, DSR Off";
                Log->Write(result.c_str());
                Error = 1;
                PrinterEnable = false;
                DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
                if (DeviceState->OldOutStateCode != DeviceState->OutStateCode)
                {
                  //DeviceState->OldOutStateCode = DeviceState->OutStateCode;
                  DeviceState->StateDescription = "Выключена сигнальная линия";
                  DeviceState->StateDescription = result;
                  DeviceState->StateChange = true;
                  //if (ChangeEvent)
                    //StateChanged(DeviceState);
                  ChangeDeviceState();
                }
                return;
            }
//            break;
    }

    if (Error == 1) return;

    result = "OK";
    Error = 0;
    PrinterEnable = true;
    DeviceState->OutStateCode = DSE_OK;
    ChangeDeviceState();
    DeviceState->StateDescription = result;
    //if (OldState != StatusCode)
    /*if (DeviceState->OldOutStateCode != DeviceState->OutStateCode)
    {
      DeviceState->OldOutStateCode = DeviceState->OutStateCode;
      OldState = DeviceState->StateCode;
      DeviceState->StateCode = StatusCode;
      DeviceState->StateDescription = result;
      DeviceState->StateChange = true;
      if (ChangeEvent)
        StateChanged(DeviceState);
      return StateDescr = result;
    }*/
//==============================================================================
  /*if (State == 0x00)
  {
    if (Error == 1)
    {
      Error = 0;
      DeviceState->StateChange = true;
      if (ChangeEvent)
        StateChanged(DeviceState);
    }
  }*/
}

std::string CCitizenPPU700::GetStatusDescription(BYTE StatusCode)
{
    std::string result = "";
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
        DeviceState->StateCode = StatusCode;
        DeviceState->StateDescription = result;
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        DeviceState->StateChange = true;
        //if (ChangeEvent)
          //StateChanged(DeviceState);
        ChangeDeviceState();
      }
      return StateDescr = result;
    }

    BYTE code = (BYTE)StatusCode & (BYTE)0x04;
    if (code > 0)
    {
      result += "Бумага кончилась ";
      Error = 1;
      PrinterEnable = false;
      if (OldState != StatusCode)
      {
        OldState = DeviceState->StateCode;
        DeviceState->StateCode = StatusCode;
        DeviceState->OutStateCode = DSE_NOTPAPER;
        DeviceState->StateDescription = result;
        DeviceState->StateChange = true;
        //if (ChangeEvent)
          //StateChanged(DeviceState);
        ChangeDeviceState();
      }
      return StateDescr = result;
    }

    code = (BYTE)StatusCode & (BYTE)0x01;
    if (code > 0)
    {
      result += "Бумага скоро кончится ";
      Error = 1;
      PrinterEnable = true;
      if (OldState != StatusCode)
      {
        OldState = DeviceState->StateCode;
        DeviceState->StateCode = StatusCode;
        DeviceState->OutStateCode = DSE_NEARENDPAPER;
        DeviceState->StateDescription = result;
        DeviceState->StateChange = true;
        //if (ChangeEvent)
          //StateChanged(DeviceState);
        ChangeDeviceState();
      }
      return StateDescr = result;
    }

    return StateDescr = result;
}

void CCitizenPPU700::SelectFont()
{
   BYTE command[3];
   command[0] = 0x1B;
   command[1] = 0x4D;
   command[2] = 0;
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
}

void CCitizenPPU700::PrintCheck(AnsiString text, std::string barcode)
{
  GetState();
  Push();
  SelectFont();
  if (DeviceState->StateCode == 0xff)
    return;
  SetCodeTable();
  //SetCharacterSize();
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
      LinesCount++;
      PrintString(AnsiString(_subtext));
    //}
  }
  if(!barcode.empty())
  {
      SetBarCodeHeight(0xA0);        //высота баркода, 0 .. 255 - в у.е.
      SetBarCodeHRIposition(1);      //место печати цифирек баркода, 0..3 или 48..51: не печатать/над/под/под и над
      SetBarCodeFont(1);             //фонт, 0 .. 2 или 48 .. 50: 3 типа: А, B или C, А->C: меньше и уже
      SetBarCodeWidth(2);            //ширину минимально полоски, 2..6 - в у.е.
      PrintBarCode(barcode);
      Feed(2);
  }
  //for(int i=0; i<7; i++)
    //Feed();
  //==========new 06-06-2007 ================
  _MinLinesCount = 3;
  //=========================================
  FeedToMinLinesCount();
  Cut(true);
  //PrintCheck(strings);
  switch (PushActionType)
  {
      case Nothing:
          PushEx2();
          break;
      case Pull:
          Push();
          break;
      case PullAndPush:
          PushEx();
          break;
      default:
          //Push();
          break;
  }
  delete strings;
}

void CCitizenPPU700::SetCodeTable()
{
   BYTE command[3];
   command[0] = 0x1B;
   command[1] = 0x74;
   command[2] = 0x07;
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
}

void CCitizenPPU700::SetCharacterSize()
{
   BYTE command[3];
   command[0] = 0x1D;
   command[1] = 0x21;
   command[2] = 0x77;
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
}

bool CCitizenPPU700::IsPrinterEnable()
{
  GetState();
  return PrinterEnable;
}

void CCitizenPPU700::Push()
{
   if (IsPaperInPresenter() == false)
      return;
   BYTE command[3];
   command[0] = 0x1D;
   command[1] = 0x52;
   command[2] = 0x30;
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
}

void CCitizenPPU700::PushEx()
{
   if (IsPaperInPresenter() == false)
      return;
   BYTE command[4];
   command[0] = 0x1D;
   command[1] = 0x52;
   command[2] = 0x31;
   command[3] = 30;
   SendType = NotRecieveAnswer;
   SendPacket(command,4,0,NULL);
   SendCommand();
}

void CCitizenPPU700::PushEx2()
{
   //if (IsPaperInPresenter() == false)
      //return;
   BYTE command[4];
   command[0] = 0x1D;
   command[1] = 0x52;
   command[2] = 0x31;
   command[3] = 0xFF;
   SendType = NotRecieveAnswer;
   SendPacket(command,4,0,NULL);
   SendCommand();
}

bool CCitizenPPU700::IsItYou()
{
   BYTE command[2];
   command[0] = 0x1B;
   command[1] = 0x76;
   SendType = RecieveAnswer;
   SendPacket(command,2,0,NULL);
   SendCommand();
   if (DeviceState == NULL)
      return false;
   if (DeviceState->AnswerSize == 0)
      return false;
   return true;
}

int CCitizenPPU700::Initialize()
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

void CCitizenPPU700::Settings()
{
   /*BYTE command[];
   command[0] = 0x1D;
   command[1] = 0x28;
   command[2] = 0x45;
   command[3] = 0x;
   command[4] = 0x;
   command[5] = 0x03;
   //bit 1
   command[] = 0x04;
   command[] = 48;
   //bit 2
   command[] = 0x04;
   command[] = 48;
   //bit 3
   command[] = 0x04;
   command[] = 48;
   //bit 4
   command[] = 0x04;
   command[] = 49;
   //bit 5
   command[] = 0x04;
   command[] = 48;
   //bit 6
   command[] = 0x04;
   command[] = 48;
   //bit 7
   command[] = 0x04;
   command[] = 48;
   //bit 8
   command[] = 0x04;
   command[] = 48;

   SendType = RecieveAnswer;
   SendPacket(command,2,0,NULL);
   SendCommand();*/
}

