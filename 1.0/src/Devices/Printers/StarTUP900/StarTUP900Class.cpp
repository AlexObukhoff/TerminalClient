//---------------------------------------------------------------------------
#pragma hdrstop
#include "StarTUP900Class.h"
#include "globals.h"
#include "boost/format.hpp"
#include "DeviceThread.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

CStarTUP900::CStarTUP900(int ComPort,int BaudRate,TLogClass* _Log, PortType::Enum portType) : CPrinter(ComPort,_Log, "StarTUP900", portType)
{
  DataLength = 1;
  /*delete Port;
  //COMParameters->Parity = NOPARITY;
  COMParameters->BaudRate = 9600;
  Port = NULL;
  Port = new TComPort(COMParameters,Log,true);*/
  if (BaudRate > 0)
  {
      COMParameters->BaudRate = BaudRate;
      if (Port)
        Port->ReopenPort();
  }
  LoggingErrors = true;
  DeviceName = "StarTUP900";
}

CStarTUP900::~CStarTUP900()
{
}

void CStarTUP900::SendCommand()
{
  DeviceThread = new TDeviceThread(true,false);
  Start();
  delete DeviceThread;
  DeviceThread = NULL;
}

void CStarTUP900::PrintString(AnsiString text)
{
  PrintLine(text);
  SendCommand();
}

void CStarTUP900::PrintCheck(TStringList* Text)
{
  char subtext[100];
  for(int i=0; i<Text->Count; i++)
  {
      memset(subtext,0,100);
      CharToOem(Text->Strings[i].c_str(), subtext);
      //PrintString(AnsiString(subtext));
      PrintBigString(AnsiString(subtext));
  }
  Cut();
}

void CStarTUP900::SendPacket(BYTE* command, int bytes_count,int datalen, BYTE* data, bool datafirst)
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

std::string CStarTUP900::GetStateDescription()
{
  return GetStatusDescription(DeviceState->StateCode);
}

void CStarTUP900::SetCodePage()
{
   BYTE command[4];
   command[0] = 0x1B;
   command[1] = 0x1D;
   command[2] = 0x74;
   command[3] = 34;
   SendType = NotRecieveAnswer;
   SendPacket(command,4,0,NULL);
}

void CStarTUP900::PrintLine(AnsiString text)
{
   BYTE command[2];
   command[0] = 0x0D;
   command[1] = 0x0A;
   int datalen = text.Length();
   SendType = NotRecieveAnswer;
   SendPacket(command,2,datalen,(char *)text.c_str(),true);
}

void CStarTUP900::PrintBigString(AnsiString text)
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

void CStarTUP900::Feed(int count)
{
    for(int i = 1; i<= count; i++)
    {
       BYTE command[2];
       command[0] = 0x0D;
       command[1] = 0x0A;
       SendType = NotRecieveAnswer;
       SendPacket(command,2,0,NULL,true);
       SendCommand();
    }
}

void CStarTUP900::ShriftOptionsEx(BYTE option1, BYTE option2)//true - заводской шрифт false - пользовательский
{
   BYTE command[4];
   command[0] = 0x1B;
   command[1] = 0x69;
   command[2] = option1;
   command[3] = option2;
   SendType = NotRecieveAnswer;
   SendPacket(command,4,0,NULL);
   SendCommand();
}

void CStarTUP900::Init()
{
   BYTE command[2];
   command[0] = 0x1B;
   command[1] = 0x40;
   SendType = NotRecieveAnswer;
   SendPacket(command,2,0,NULL);
   SendCommand();
}

void CStarTUP900::Cut()
{
   BYTE command[3];
   command[0] = 0x1B;
   command[1] = 0x64;
   command[2] = 0;
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
}

void CStarTUP900::GetState()
{
   //Init();
   State = 0xFF;
   //State = 0x00;
   SubState = 0x00;
   StateDescr = "";
   SubStateDescr = "";


   BYTE command[4];
   /*
   command[0] = 0x1B;
   command[1] = 0x1E;
   command[2] = 0x61;
   command[3] = 0x00;
   SendType = NotRecieveAnswer;
   SendPacket(command,4,0,NULL);
   SendCommand();*/

   /*SendType = NotRecieveAnswer;
   command[0] = 0x1B;
   command[1] = 0x40;
   command[2] = 0x18;
   SendPacket(command,3,0,NULL);
   SendCommand();

   Sleep(100);*/

   /*SendType = RecieveAnswer;
   command[0] = 0x05;
   SendPacket(command,1,0,NULL);
   SendCommand(); */

   /*command[0] = 0x1B;
   command[1] = 0x06;
   command[2] = 0x01;*/
   //command[3] = 0x04;
   command[0] = 0x04;
   SendType = RecieveAnswer;
   SendPacket(command,1,0,NULL);
   SendCommand();

   if (DeviceState->AnswerSize > 0)
     State = Answer[0];
   Log->Write((boost::format("Printer State=%1%") % GetStatusDescription(State).c_str()).str().c_str());

  DeviceState->StateCode = State;
  DeviceState->StateDescription = GetStatusDescription(State);
  DeviceState->SubStateCode = 0x00;
  DeviceState->SubStateDescription = "";
}

std::string CStarTUP900::GetStatusDescription(BYTE StatusCode)
{
    std::string result = "";
    DeviceState->StateCode = StatusCode;
    DeviceState->SubStateCode = 0x00;
    DeviceState->SubStateDescription = "";

    Log->Write((boost::format("Printer DSR_CTS=%1%") % DeviceState->DSR_CTS).str().c_str());

    if (StatusCode == 0xFF)
    {
      result = " Принтер недоступен!";
      Error = 1;
      PrinterEnable = false;
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      //if (OldState != StatusCode)
      if (DeviceState->OldOutStateCode != DeviceState->OutStateCode)
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

    int code = StatusCode&0x08;
    if (code > 0)
    {
      result = " Бумага закончилась";
      Error = 1;
      PrinterEnable = false;
      DeviceState->OutStateCode = DSE_NOTPAPER;
      //if (OldState != StatusCode)
      if (DeviceState->OldOutStateCode != DeviceState->OutStateCode)
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
      PrinterEnable = false;
      DeviceState->OutStateCode = DSE_PAPERJAM;
      //if (OldState != StatusCode)
      if (DeviceState->OldOutStateCode != DeviceState->OutStateCode)
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

    code = StatusCode&0x04;
    if (code > 0)
    {
      result = " Бумага скоро кончится";
      Error = 1;
      PrinterEnable = true;
      DeviceState->OutStateCode = DSE_NEARENDPAPER;
      //if (OldState != StatusCode)
      if (DeviceState->OldOutStateCode != DeviceState->OutStateCode)
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

    if(Port->m_portType == PortType::com)
    {
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
                      DeviceState->OldOutStateCode = DeviceState->OutStateCode;
                      DeviceState->OldStateCode = DeviceState->StateCode;
                      DeviceState->StateDescription = "Выключена сигнальная линия";
                      DeviceState->StateChange = true;
                      if (ChangeEvent)
                        StateChanged(DeviceState);
                    }
                    return result;
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
                      DeviceState->OldOutStateCode = DeviceState->OutStateCode;
                      DeviceState->StateDescription = "Выключена сигнальная линия";
                      DeviceState->StateDescription = result;
                      DeviceState->StateChange = true;
                      if (ChangeEvent)
                        StateChanged(DeviceState);
                    }
                    return result;
                }
    //            break;
        }
    }
    result = "OK";
    Error = 0;
    PrinterEnable = true;
    DeviceState->OutStateCode = DSE_OK;
    //if (OldState != StatusCode)
    if (DeviceState->OldOutStateCode != DeviceState->OutStateCode)
    {
      DeviceState->OldOutStateCode = DeviceState->OutStateCode;
      OldState = DeviceState->StateCode;
      DeviceState->StateCode = StatusCode;
      DeviceState->StateDescription = result;
      DeviceState->StateChange = true;
      if (ChangeEvent)
        StateChanged(DeviceState);
      return StateDescr = result;
    }

    return StateDescr = result;
}

/*
 * Encode \nn sequences into bytes and return result.
 */
AnsiString CStarTUP900::EncodeBackslashes(AnsiString s)
{
        AnsiString out;

        bool collecting = FALSE;        // TRUE means we're collecting digits of number
        AnsiString number = "";
        int i;
//        LogDebug("EBS: inv, s-'" + s + "'");
        for (i = 0; i < s.Length(); i++)
        {
                char ch = s[i+1];
//                LogDebug("i-" + (AS)i + " ch-" + ch);
//                LogDebug("-- collecting-" + (AS)(int)collecting);
                if (collecting)
                {       // we're collecting digits
                        if (ch >= '0' && ch <= '9')
                        {       // a digit - append to number
//                                LogDebug("-- digit: appending digit to number");
                                number += ch;
                        } else
                        {       // not a digit

                                // has something been collected?
                                if (! number.IsEmpty())
                                {       // number is not empty

                                        // push out number
                                        int b = number.ToInt();
                                        out += (char)b;

                                        if (ch == '\\')
                                        {       // legal \1\2, clean number, keep collecting
                                                number = "";
                                        } else
                                        {       // legal \1c, stop collecting, push out ch
                                                collecting = FALSE;
                                                out += ch;
                                        }
                                } else
                                {       // number is empty
                                        out += '\\';
                                        collecting = FALSE;
                                        if (ch != '\\')
                                        {       // illegal \c, stop collecting, push out ch
                                                out += ch;
                                        }
                                }
                        }
                } else
                {       // we're not collecting digits, wait for

                        if (ch == '\\')
                        {
//                                LogDebug("-- backslash: collecting");
                                collecting = TRUE;
                                number = "";
                        } else
                        {       // not a backslash, pass to output
                                out += ch;
                        }
                }
        } // end of char loop

        // check if last state was collecting
        if (collecting)
        {
                if (! number.IsEmpty())
                {       // push out number
                        int b = number.ToInt();
                        out += (char)b;
                } else
                {       // just keep the last '\'
                        out += '\\';
                }
        }

        return out;
}

void CStarTUP900::PrintCheck(AnsiString text, std::string barcode)
{
  //new 05-03-2007
  text = EncodeBackslashes(text);

  PresenterTimer();

//  BYTE xON = 0x11;
//  BYTE xOFF = 0x13;
//  DWORD count = 0;

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
      PrintString(subtext);
      LinesCount++;
    //}
  }
  //==========new 06-06-2007 ================
    LinesCount += 4;
  //=========================================
  FeedToMinLinesCount();
  Sleep(1000);
  Cut();
  delete strings;
}

void CStarTUP900::PresenterAutoPush()
{
   BYTE command[4];
   command[0] = 0x1B;
   command[1] = 0x16;
   command[2] = 0x31;
   command[3] = 2;
   SendType = NotRecieveAnswer;
   SendPacket(command,4,0,NULL);
   SendCommand();
}

void CStarTUP900::SetLeftMargin()
{
   BYTE command[3];
   command[0] = 0x1B;
   command[1] = 0x6C;
   command[2] = 0x02;
   SendType = NotRecieveAnswer;
   SendPacket(command,3,0,NULL);
   SendCommand();
}

bool CStarTUP900::IsPrinterEnable()
{
  GetState();
  return PrinterEnable;
}

void CStarTUP900::PresenterTimer()
{
   BYTE command[4];
   command[0] = 0x1B;
   command[1] = 0x16;
   command[2] = 0x31;
   command[3] = 60;
   SendType = NotRecieveAnswer;
   SendPacket(command,4,0,NULL);
   SendCommand();
}

bool CStarTUP900::IsItYou()
{
   BYTE command[1];
   command[0] = 0x04;
   SendType = RecieveAnswer;
   SendPacket(command,1,0,NULL);
   SendCommand();
   if (DeviceState->AnswerSize > 0)
     return true;
   return false;
}

int CStarTUP900::Initialize()
{
    try
    {
        Init();
        GetState();
        PresenterAutoPush();
        SetLeftMargin();
        ShriftOptionsEx(0x10);
        SetInitialized();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}


