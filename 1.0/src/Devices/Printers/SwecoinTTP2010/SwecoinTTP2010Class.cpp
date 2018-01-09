//---------------------------------------------------------------------------


#pragma hdrstop

#include "SwecoinTTP2010Class.h"
#include "DeviceThread.h"
#include "boost/format.hpp"
#include <boost/lexical_cast.hpp>

//---------------------------------------------------------------------------

#pragma package(smart_init)
//==============================================================================
CSwecoinTTP2010::CSwecoinTTP2010(int ComPort,int BaudRate,TLogClass* _Log) : CPrinter(ComPort,_Log, "SwecoinTTP2010")
{
    DeviceName = "SwecoinTTP2010";
    DataLength = 1;
    COMParameters->Parity = NOPARITY;
    COMParameters->BaudRate = 115200;//9600;
    if (BaudRate > 0)
      COMParameters->BaudRate = BaudRate;
    if(Port)
    {
        delete Port;
        Port = NULL;
    }
    Port = new TComPort(COMParameters,Log,true);
    LoggingErrors = true;
    Font = "0";
    //Init();
    //Sleep(1000);
//    Initialize();
    _MinLinesCount = 4;
    //ShriftOptionsEx(0); // 0 (стандартный шрифт), 1, 2 ... 7
    //GetState();
    //SetInitialized();
    //Sleep(1000);
    //GetID();
}
CSwecoinTTP2010::~CSwecoinTTP2010()
{
    if(Port)
    {
        delete Port;
        Port = NULL;
    }
}
//==============================================================================
void CSwecoinTTP2010::Init()
{
   BYTE command[2];
   command[0] = 0x1B;
   command[1] = 0x40;
   SendType = NotRecieveAnswer;
   SendPacket(command,2,0,NULL);
   SendCommand();
}
//==============================================================================
void CSwecoinTTP2010::Feed(int count)
{
    for(int i=0; i<count; i++)
    {
        BYTE command[1];
        command[0] = 0x0A;
        SendType = NotRecieveAnswer;
        SendPacket(command,1,0,NULL,true);
        SendCommand();
    }
}
//==============================================================================
void CSwecoinTTP2010::ShriftOptionsEx(int option)// 0 (стандартный шрифт), 1, 2 ... 7
{
   BYTE command[3];
   command[0] = 0x1B;
   command[1] = 0x21;
   command[2] = (BYTE)option; // 0 ... 7
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
}
//==============================================================================
void CSwecoinTTP2010::Tab()
{
   BYTE command[1];
   command[0] = 0x09;
   SendType = NotRecieveAnswer;
   SendPacket(command,1,0,NULL);
   SendCommand();
}
//==============================================================================
void CSwecoinTTP2010::Cut()  // Cut and Eject
{
   BYTE command[1];
   command[0] = 0x1E;
   SendType = NotRecieveAnswer;
   SendPacket(command,1,0,NULL);
   SendCommand();
}
//==============================================================================
bool CSwecoinTTP2010::IsPrinterEnable()
{
  GetState();
  return PrinterEnable;
}
//==============================================================================
void CSwecoinTTP2010::GetState()
{
  State = 0x00;
  SubState = 0x00;
  StateDescr = "";
  SubStateDescr = "";
  BYTE command[3];
  std::string sStatusDescription;

  command[0] = 0x1B;
  command[1] = 0x05;
  command[2] = 0x01;
  SendType = RecieveAnswer;
  SendPacket(command,3,0,NULL);
  SendCommand();
  if (DeviceState->AnswerSize > 0)
      State = ((DeviceState->AnswerSize == 1) ? Answer[0] : Answer[1]);
  else
  {
      DeviceState->StateCode = 0xFF;
      DeviceState->SubStateCode = 0x00;
      Error = 1;
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
  sStatusDescription = GetStatusDescription((BYTE)State);
  Log->Write((boost::format("Printer State=%1%") % sStatusDescription.c_str()).str().c_str());

  DeviceState->StateCode = State;
  DeviceState->StateDescription = sStatusDescription;
  DeviceState->SubStateCode = 0x00;
  DeviceState->SubStateDescription = "";
  if (State == 0x00)
  {
    if (Error == 1)
    {
      Error = 0;
      DeviceState->StateDescription = "OK";
      DeviceState->OutStateCode = DSE_OK;
      ChangeDeviceState();
      /*DeviceState->StateChange = true;
      if (ChangeEvent)
        StateChanged(DeviceState);*/
    }
  }
}
//==============================================================================
std::string CSwecoinTTP2010::GetStateDescription()
{
  return GetStatusDescription((BYTE)DeviceState->StateCode);
}
//==============================================================================
std::string CSwecoinTTP2010::GetStatusDescription(BYTE StatusCode)
{
    std::string result = "";
    DeviceState->SubStateCode = 0x00;
    DeviceState->SubStateDescription = "";
    if (DeviceState->AnswerSize == 1 && StatusCode == 0x06)
    {
      result += "OK";
      Error = 0;
      DeviceState->OutStateCode = DSE_OK;
      PrinterEnable = true;
    }
    else
    {
        switch (StatusCode)
        {
          case 0x02:
              DeviceState->OutStateCode = DSE_PAPERJAM;
              result += " Cutter jammed!";
          break;

          case 0x03:
              DeviceState->OutStateCode = DSE_NOTPAPER;
              result += " Out of paper!";
          break;

          case 0x04:
              //DeviceState->OutStateCode = DSE_MAINERROR;
              result += " Printhead lifted!";
          break;

          case 0x05:
               result += " Paper-feed error. No paper detected in presenter although 10 cm has been \
printed. Paper might be wound around the platen or, in some way, has been \
forced above the presenter module.";
          break;

          case 0x06:
              //DeviceState->OutStateCode = DSE_MAINERROR;
              result += " Temperature error. The printhead temperature has exceeded the 60 °C maximum limit.";
          break;

          case 0x07:
              result += " Presenter not running!";
          break;

          case 0x08:
              DeviceState->OutStateCode = DSE_PAPERJAM;
              result += " Paper jam during retract!";
          break;

          case 0x10:
                        result += " Retract function timed out. If the customer doesn’t take the paper and the \
printer clears the presenter due to a timeout, the pending error bit is set and \
error code NAK 10h is reported.";
          break;

          case 0xFF:
              //DeviceState->OutStateCode = DSE_NOTMOUNT;
              result += " No Answer!";
          break;

          default:
              result += (boost::format(" Unresearch error! StatusCode: %1%.") % StatusCode).str();
        }

        Error = 1;
        PrinterEnable = false;
    }

    if (OldState != StatusCode)
    {
      OldState = StatusCode;
      DeviceState->StateCode = StatusCode;
      DeviceState->StateDescription = result;
      ChangeDeviceState();
      /*DeviceState->StateChange = true;
      if (ChangeEvent)
        StateChanged(DeviceState);*/
    }
    return StateDescr = result;
}
//==============================================================================
void CSwecoinTTP2010::PrintLine(AnsiString text)
{
   BYTE command[2];
   command[0] = 0x0D;
   command[1] = 0x0A;
   int datalen = text.Length();
   SendType = NotRecieveAnswer;
   SendPacket(command,2,datalen,text.c_str(),true);
}
//==============================================================================
void CSwecoinTTP2010::PrintString(AnsiString text)
{
  PrintLine(text);
  SendCommand();
}
//==============================================================================
void CSwecoinTTP2010::PrintCheck(TStringList* Text)
{
  char subtext[100];
  for(int i=0; i<Text->Count; i++)
  {
      memset(subtext,0,100);
      CharToOem(Text->Strings[i].c_str(), subtext);
      PrintString(AnsiString(subtext));
  }
  Cut();
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CSwecoinTTP2010::PrintCheck(AnsiString text, std::string barcode)
{
  //new 18-06-2007
  EnforcedClearPresenter();
  Sleep(300);

  CharToOem(text.c_str(), text.c_str());
  //Tab();
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
//  char _subtext[1024];
  LinesCount = 0;
  while(true)
  {
    int pos = text.Pos(delim);
      if (pos == 0)
      {
          AnsiString subtext = text;
          PrintString(subtext);
          LinesCount++;
          break;
      }
    AnsiString subtext = text.SubString(0,pos-1);
    text = text.SubString(pos+1,text.Length()-pos);
    //if (!subtext.IsEmpty())
    //{
      /*memset(_subtext,0,1024);
      //CharToOem(subtext.c_str(), _subtext);
      OemToChar(subtext.c_str(), _subtext);
      PrintString(_subtext);*/
      LinesCount++;
      PrintString(subtext);
    //}
  }
  FeedToMinLinesCount();
  Cut();    // Cut and Eject
  delete strings;
}
//==============================================================================
void CSwecoinTTP2010::SendCommand()
{
  DeviceThread = new TDeviceThread(true,false);
  Start();
  delete DeviceThread;
  DeviceThread = NULL;
}
//==============================================================================
void CSwecoinTTP2010::SendPacket(BYTE* command, int bytes_count,int datalen, BYTE* data, bool datafirst)
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
//==============================================================================
AnsiString CSwecoinTTP2010::GetID()
{    /*
    char sPrinterID[64];
    BYTE command[3];
    command[0] = 0x1B;
    command[1] = 0x05;
    command[2] = 0x63;

    SendType = RecieveAnswer;
    SendPacket(command,3,0,NULL,true);
    SendCommand();

    int i;
    for(i = 1; Answer[i] != '\0'; i++)  sPrinterID[i-1] = Answer[i];
    sPrinterID[i] = '\0';
    return (AnsiString)sPrinterID + "<Swecoin TTP2010>";
    */
    return "omited - <Swecoin TTP2010>";
}
bool CSwecoinTTP2010::IsItYou()
{
    BYTE command[3];
    command[0] = 0x1B;
    command[1] = 0x05;
    command[2] = 0x01;
    SendType = RecieveAnswer;
    SendPacket(command,3,0,NULL);
    SendCommand();
    if (DeviceState == NULL)            return false;
    if (DeviceState->AnswerSize == 0 || Answer[0] == 0xFF || Answer[0] == 0)   return false;
    return true;
}
/*void CSwecoinTTP2010::TestPrint()
{

}
void CSwecoinTTP2010::LoadFont(const AnsiString File_SWF_Name)
{
   BYTE command[3];
   command[0] = 0x1B;
   command[1] = 0x26;
   command[2] = 0x00;
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0, NULL,true);
}//*/

int CSwecoinTTP2010::Initialize()
{
    Init();
    Sleep(1000);
    int fontNum = 0;
    try
    {
        fontNum = boost::lexical_cast<int>(Font);
    }
    catch(...)
    {
    }
//    int font = boost::lexical_cast<int>(Peripherals.Printer.Font);
    ShriftOptionsEx(fontNum); // 0 (стандартный шрифт), 1, 2 ... 7
    GetState();
    SetInitialized();
    Log->Write(GetID().c_str());
}

//new 18-06-2007
void CSwecoinTTP2010::ClearPresenter()
{
   BYTE command[1];
   command[0] = 0x05;
   SendType = NotRecieveAnswer;
   SendPacket(command,1,0,NULL);
}

void CSwecoinTTP2010::EnforcedClearPresenter()
{
   BYTE command[2];
   command[0] = 0x19;
   command[1] = 100;
   SendType = NotRecieveAnswer;
   SendPacket(command,2,0,NULL);
}

