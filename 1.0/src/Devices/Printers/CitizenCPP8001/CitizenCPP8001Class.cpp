//---------------------------------------------------------------------------


#pragma hdrstop

#include "CitizenCPP8001Class.h"
//#include "CitizenCPP8001Thread.h"
#include "DeviceThread.h"
#include "globals.h"
#include "boost/format.hpp"

//---------------------------------------------------------------------------

#pragma package(smart_init)

CCitizenCPP8001::CCitizenCPP8001(int ComPort,int BaudRate,TLogClass* _Log) : CPrinter(ComPort,_Log, "CitizenCPP8001")
{
  DataLength = 1;
  delete Port;
  COMParameters->Parity = NOPARITY;
  COMParameters->BaudRate = 9600;
  if (BaudRate > 0)
      COMParameters->BaudRate = BaudRate;
  COMParameters->DtrControl = DTR_CONTROL_DISABLE;
  COMParameters->RtsControl  = DTR_CONTROL_DISABLE;
  Port = new TComPort(COMParameters,Log,true);
  LoggingErrors = true;
  State = 0x00;
  OldState = 0x00;
  SubState = 0x00;
  OldSubState = 0x00;
  DeviceName = "CitizenCPP8001";
  _MinLinesCount = 4;
  MinLinesCount += 3;
  DeviceState->OutStateCode = DSE_NOTMOUNT;
}

CCitizenCPP8001::~CCitizenCPP8001()
{
}

void CCitizenCPP8001::SendCommand()
{
  DeviceThread = new TDeviceThread(true,false);
  Start();
  delete DeviceThread;
  DeviceThread = NULL;
}

void CCitizenCPP8001::PrintString(AnsiString text)
{
  PrintLine(text);
  SendCommand();
}

void CCitizenCPP8001::PrintCheck(TStringList* Text)
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

void CCitizenCPP8001::SendPacket(BYTE* command, int bytes_count,int datalen, BYTE* data, bool datafirst)
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

std::string CCitizenCPP8001::GetStateDescription()
{
  return GetStatusDescription(DeviceState->StateCode);
}

void CCitizenCPP8001::PrintLine(AnsiString text)
{
   BYTE command[2];
   command[0] = 0x0A;
   command[1] = 0x0D;
   int datalen = text.Length();
   SendType = NotRecieveAnswer;
   SendPacket(command,2,datalen,text.c_str(),true);
}

void CCitizenCPP8001::Feed(int count)
{
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

void CCitizenCPP8001::SetUnderline(bool option)
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

void CCitizenCPP8001::SetBold(bool option)
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

void CCitizenCPP8001::SetDoublePrint(bool option)
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

void CCitizenCPP8001::Init()
{
   BYTE command[2];
   command[0] = 0x1B;
   command[1] = 0x40;
   SendType = NotRecieveAnswer;
   SendPacket(command,2,0,NULL);
   SendCommand();
}

void CCitizenCPP8001::Cut(bool option)//true - полный отрез false - неполный отрез чека
{
   BYTE command[2];
   command[0] = 0x1B;
   command[1] = 0x69;
   if (option)
     command[1] = 0x69;
   else
     command[1] = 0x6D;
   SendType = NotRecieveAnswer;
   SendPacket(command,2,0,NULL);
   SendCommand();
}

void CCitizenCPP8001::GetState()
{
   if (DeviceState == NULL)
      return;

   State = 0xFF;
   SubState = 0x0;
   StateDescr = "";
   SubStateDescr = "";
   Error = 0;
   AnsiString result;

   BYTE cmd[3];

    DeviceState->StateCode = State;
    DeviceState->StateDescription = "NOT MOUNT";
    DeviceState->SubStateCode = 0x00;
    DeviceState->SubStateDescription = "";

    //==========================================================================
    // processing byte 1
    cmd[0]= 0x10;
    cmd[1]= 0x04;
    cmd[2]= 0x01;

   SendType = RecieveAnswer;
   SendPacket(cmd,3,0,NULL);
   SendCommand();

    if (DeviceState->AnswerSize > 0)
    {
        State = Answer[0];
        DeviceState->StateCode = State;
        DeviceState->SubStateCode = 0x00;
        Log->Write((boost::format("Processing byte 1: AnswerSize = %1%; Answer = %2%") % DeviceState->AnswerSize % Answer[0]).str().c_str());
        int code = State & 0x08;
        if (code > 0)
        {
            PrinterEnable = false;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            DeviceState->StateDescription = "Принтер оффлайн";
            Log->Write((boost::format("Printer State = %1%") % DeviceState->StateDescription.c_str()).str().c_str());
            if (OldState != DeviceState->StateCode)
            {
                OldState = DeviceState->StateCode;
                DeviceState->StateChange = true;
                ChangeDeviceState();
            }
            return;
        }
    }
    else//no answer from printer
    {
        DeviceState->StateCode = State;
        DeviceState->SubStateCode = 0x00;
        PrinterEnable = false;
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        Log->Write("Printer has no answer.");
        DeviceState->StateDescription = "Принтер не подключен";
        if (OldState != DeviceState->StateCode)
        {
            OldState = DeviceState->StateCode;
            DeviceState->StateChange = true;
            ChangeDeviceState();
        }
        return;
    }
    //============================================================================
    // processing byte 2
    cmd[0]= 0x10;
    cmd[1]= 0x04;
    cmd[2]= 0x02;
    SendPacket(cmd,3,0,NULL);
    SendCommand();
    if (DeviceState->AnswerSize > 0)
    {
        State = Answer[0];
        DeviceState->StateCode = State;
        DeviceState->SubStateCode = 0x00;
        Log->Write((boost::format("Processing byte 2: AnswerSize = %1%; Answer = %2%") % DeviceState->AnswerSize % Answer[0]).str().c_str());
        int code = State & 0x40;
        if (code > 0)
        {
            PrinterEnable = false;
            DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
            DeviceState->StateDescription = "Ошибка принтера";
            Log->Write((boost::format("Printer State = %1%") % DeviceState->StateDescription.c_str()).str().c_str());
            if (OldState != DeviceState->StateCode)
            {
                OldState = DeviceState->StateCode;
                DeviceState->StateChange = true;
                ChangeDeviceState();
            }
            return;
        }
    }
    else//no answer from printer
    {
        DeviceState->StateCode = State;
        DeviceState->SubStateCode = 0x00;
        PrinterEnable = false;
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        Log->Write("Printer has no answer.");
        DeviceState->StateDescription = "Принтер не подключен";
        if (OldState != DeviceState->StateCode)
        {
            OldState = DeviceState->StateCode;
            DeviceState->StateChange = true;
            ChangeDeviceState();
        }
        return;
    }
    //============================================================================
    // processing byte 3
    cmd[0]= 0x10;
    cmd[1]= 0x04;
    cmd[2]= 0x03;
    SendPacket(cmd,3,0,NULL);
    SendCommand();
    if (DeviceState->AnswerSize > 0)
    {
        State = Answer[0];
        DeviceState->StateCode = State;
        DeviceState->SubStateCode = 0x00;
        Log->Write((boost::format("Processing byte 3: AnswerSize = %1%; Answer = %2%") % DeviceState->AnswerSize % Answer[0]).str().c_str());
        int code = State & 0x08;
        if (code > 0)
        {
            PrinterEnable = false;
            DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
            DeviceState->StateDescription = "Ошибка отрезчика";
            Log->Write((boost::format("Printer State = %1%") % DeviceState->StateDescription.c_str()).str().c_str());
            if (OldState != DeviceState->StateCode)
            {
                OldState = DeviceState->StateCode;
                DeviceState->StateChange = true;
                ChangeDeviceState();
            }
            return;
        }
        code = State & 0x20;
        if (code > 0)
        {
            PrinterEnable = false;
            DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
            DeviceState->StateDescription = "Невосстановимая ошибка принтера";
            Log->Write((boost::format("Printer State = %1%") % DeviceState->StateDescription.c_str()).str().c_str());
            if (OldState != DeviceState->StateCode)
            {
                OldState = DeviceState->StateCode;
                DeviceState->StateChange = true;
                ChangeDeviceState();
            }
            return;
        }
        code = State & 0x40;
        if (code > 0)
        {
            PrinterEnable = true;
            //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
            DeviceState->StateDescription = "Восстановимая ошибка принтера";
            Log->Write((boost::format("Printer State = %1%") % DeviceState->StateDescription.c_str()).str().c_str());
            if (OldState != DeviceState->StateCode)
            {
                OldState = DeviceState->StateCode;
                DeviceState->StateChange = true;
                ChangeDeviceState();
            }
            Init();
            return;
        }
    }
    else//no answer from printer
    {
        DeviceState->StateCode = State;
        DeviceState->SubStateCode = 0x00;
        PrinterEnable = false;
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        Log->Write("Printer has no answer.");
        DeviceState->StateDescription = "Принтер не подключен";
        if (OldState != DeviceState->StateCode)
        {
            OldState = DeviceState->StateCode;
            DeviceState->StateChange = true;
            ChangeDeviceState();
        }
        return;
    }
    //============================================================================
    // processing byte 4
    cmd[0]= 0x10;
    cmd[1]= 0x04;
    cmd[2]= 0x04;
    SendPacket(cmd,3,0,NULL);
    SendCommand();
    if (DeviceState->AnswerSize > 0)
    {
        State = Answer[0];
        DeviceState->StateCode = State;
        DeviceState->SubStateCode = 0x00;
        Log->Write((boost::format("Processing byte 4: AnswerSize = %1%; Answer = %2%") % DeviceState->AnswerSize % Answer[0]).str().c_str());
        int code = State & 0x60;
        if (code > 0)
        {
            PrinterEnable = false;
            DeviceState->OutStateCode = DSE_NOTPAPER;
            DeviceState->StateDescription = "Бумага кончилась";
            Log->Write((boost::format("Printer State = %1%") % DeviceState->StateDescription.c_str()).str().c_str());
            if (OldState != DeviceState->StateCode)
            {
                OldState = DeviceState->StateCode;
                DeviceState->StateChange = true;
                ChangeDeviceState();
            }
            return;
        }
        code = State & 0x0C;
        if (code > 0)
        {
            PrinterEnable = true;
            DeviceState->OutStateCode = DSE_NEARENDPAPER;
            DeviceState->StateDescription = "Бумага скоро кончится";
            Log->Write((boost::format("Printer State = %1%") % DeviceState->StateDescription.c_str()).str().c_str());
            if (OldState != DeviceState->StateCode)
            {
                OldState = DeviceState->StateCode;
                DeviceState->StateChange = true;
                ChangeDeviceState();
            }
            return;
        }
    }
    else//no answer from printer
    {
        DeviceState->StateCode = State;
        DeviceState->SubStateCode = 0x00;
        PrinterEnable = false;
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        Log->Write("Printer has no answer.");
        DeviceState->StateDescription = "Принтер не подключен";
        if (OldState != DeviceState->StateCode)
        {
            OldState = DeviceState->StateCode;
            DeviceState->StateChange = true;
            ChangeDeviceState();
        }
        return;
    }



    PrinterEnable = true;
    DeviceState->StateCode = 0x00;
    DeviceState->OutStateCode = DSE_OK;
    DeviceState->SubStateCode = 0x00;
    DeviceState->StateDescription = "Printer OK";
    Log->Write((boost::format("GetState() = %1%") % DeviceState->StateDescription.c_str()).str().c_str());

    if ((DeviceState->OutStateCode == DSE_OK)&&(DeviceState->OutStateCode != DeviceState->OldOutStateCode))
    {
        PrinterEnable = true;
        OldState = State;
        DeviceState->StateChange = true;
        ChangeDeviceState();
        return;
    }
}

void CCitizenCPP8001::SelectFont()
{
   BYTE command[3];
   command[0] = 0x1B;
   command[1] = 0x4D;
   command[2] = 0;
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
}

void CCitizenCPP8001::PrintCheck(AnsiString text, std::string barcode)
{
  GetState();
  SelectFont();
  SetCodeTable();
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
    memset(_subtext,0,1024);
    CharToOem(subtext.c_str(), _subtext);
    LinesCount++;
    PrintString(AnsiString(_subtext));
    Sleep(200);
  }
  FeedToMinLinesCount();
  Cut(true);
  delete strings;
}

void CCitizenCPP8001::SetCodeTable()
{
   BYTE command[3];
   command[0] = 0x1B;
   command[1] = 0x74;
   command[2] = 0x07;
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
}

void CCitizenCPP8001::SetCharacterSize()
{
   BYTE command[3];
   command[0] = 0x1D;
   command[1] = 0x21;
   command[2] = 0x77;
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
}

bool CCitizenCPP8001::IsPrinterEnable()
{
  GetState();
  return PrinterEnable;
}

bool CCitizenCPP8001::IsItYou()
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

int CCitizenCPP8001::Initialize()
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
    return 0;
}
