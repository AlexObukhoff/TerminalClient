#pragma hdrstop

#include "wp_t833.h"
#include "globals.h"
#include "boost/format.hpp"

wp_t833::wp_t833(int ComPort, int BaudRate, TLogClass* _Log):CPrinter(0, _Log, "wp_t833")
{
    try
    {
        DeviceName = "wp_t833";
        Error = 0;
        ComNumber.sprintf("\\\\.\\COM%i",ComPort);
        if (Log)
          Log->Write((boost::format("Com Printer Port = %1%") % ComNumber.c_str()).str().c_str());
        LoggingErrors = true;
        //ComSpeed="19200";
        ComSpeed=19200;
        if (BaudRate > 0)
            ComSpeed = ComSpeed;
        FeedLine=25;
        AutoBuzzer=false;
        AutoFeed=true;
        AutoCut=true;

        PrintData = new TStringList;
        Clear();
        COMOpen();
        //Init();
        //SetCodeTable();
        //Sleep(10);
        //MinLineFeed();
        //Font(0x01);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

wp_t833::~wp_t833()
{
  try
  {
        delete PrintData;
        COMClose();
  }
  __finally
  {
  }
}

void wp_t833::COMOpen()
{
        hCom=CreateFile(ComNumber.c_str(),GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
        if(hCom!=INVALID_HANDLE_VALUE)
        {
                GetCommState(hCom,&oldDCB);
                newDCB=oldDCB;
                GetCommTimeouts(hCom,&oldCTO);
                newCTO=oldCTO;

                //newDCB.BaudRate=ComSpeed.ToInt();
                //newDCB.BaudRate=19200;
                newDCB.BaudRate=ComSpeed;
                if (Log != NULL)
                  Log->Write((boost::format("Port speed = %1%") % ComSpeed).str().c_str());
                newDCB.fParity=NOPARITY;
                newDCB.fOutxCtsFlow=false;
                newDCB.fOutxDsrFlow=false;
                newDCB.fDtrControl=DTR_CONTROL_ENABLE;
                newDCB.fDsrSensitivity=false;
                newDCB.fOutX=false;
                newDCB.fInX=false;
                newDCB.fNull=false;
                newDCB.fRtsControl=DTR_CONTROL_ENABLE;
                newDCB.fAbortOnError=false;
                newDCB.ByteSize=8;
                newDCB.Parity=NOPARITY;
                newDCB.StopBits=ONESTOPBIT;

                if(!SetCommState(hCom,&newDCB))
          {
            //MessageBox(NULL,"SetCommState","ERROR!",NULL);
            Log->Write("SetCommState - ERROR!");
          }

                if(!SetupComm(hCom,2,2))
          {
            //MessageBox(NULL,"SetupComm","ERROR!",NULL);
            Log->Write("SetupComm - ERROR!");
          }

          newCTO.ReadIntervalTimeout=MAXDWORD;
                newCTO.ReadTotalTimeoutMultiplier=MAXDWORD;
                newCTO.ReadTotalTimeoutConstant=50;
                newCTO.WriteTotalTimeoutMultiplier=0;
                newCTO.WriteTotalTimeoutConstant=0;

                if(!SetCommTimeouts(hCom,&newCTO))
          {
            //MessageBox(NULL,"SetCommTimeouts","ERROR!",NULL);
                                                Log->Write("SetCommTimeouts - ERROR!");
          }

                return;
        }
        //MessageBox(NULL,"Cannot open COM port!","ERROR!",NULL);
        Log->Write("Cannot open COM port! ERROR!");
}

void wp_t833::COMClose()
{
        SetCommState(hCom,&oldDCB);
        SetCommTimeouts(hCom,&oldCTO);
        CloseHandle(hCom);
}

void wp_t833::Init()
{
    if ((LoggingErrors)&&(Log))
        Log->Write("Init()");
    unsigned char cmd[2]={0x1b,0x40};
    for (int i=0; i<2; i++)
            WriteFile(hCom,&cmd[i],1,&dWritten,NULL);
}

void wp_t833::SetCodeTable()
{
    if ((LoggingErrors)&&(Log))
        Log->Write("SetCodeTable()");
    PurgeComm(hCom,PURGE_TXCLEAR);
    PurgeComm(hCom,PURGE_RXCLEAR);

    //BYTE cmd[3]={0x1b,0x74,0x07};
    BYTE cmd[3]={0x1b,0x74,0x11};
    for (int i=0; i<3; i++)
            WriteFile(hCom,&cmd[i],1,&dWritten,NULL);
    Sleep(100);
}


void wp_t833::MinLineFeed()
{
    if ((LoggingErrors)&&(Log))
        Log->Write("MinLineFeed()");
    BYTE cmd[3]={0x1b,0x33,0x00};
    for (int i=0; i<3; i++)
            WriteFile(hCom,&cmd[i],1,&dWritten,NULL);
}

void wp_t833::CharacterSet()
{
    if ((LoggingErrors)&&(Log))
        Log->Write("CharacterSet()");
    BYTE cmd[6]={0x1b,0x74,0x07,0x1b,0x21,0x00};
    for (int i=0; i<6; i++)
            WriteFile(hCom,&cmd[i],1,&dWritten,NULL);
}

void wp_t833::Add(AnsiString Line)
{
        PrintData->Add(Line);
}

void wp_t833::Clear()
{
        PrintData->Clear();
}

void wp_t833::Print()
{
    //PurgeComm(hCom,PURGE_TXCLEAR);
    //PurgeComm(hCom,PURGE_RXCLEAR);

    if (AutoBuzzer) Buzzer();

    for(int y=0; y<PrintData->Count; y++)
    {
        WriteFile(hCom,PrintData->Strings[y].c_str(),PrintData->Strings[y].Length(),&dWritten,NULL);
        _Feed();
    }
    FeedToMinLinesCount();

    if (AutoCut) Cut();

    if (AutoBuzzer) Buzzer();
}

void wp_t833::Buzzer()
{
        unsigned char cmd[2]={0x1b, 0x1e};
        for (int i=0; i<2; i++)
                WriteFile(hCom,&cmd[i],1,&dWritten,NULL);
}

void wp_t833::_Feed(int count)
{
    for(int i=1; i<=count; i++)
    {
        unsigned char cmd[2]={0x0a,0x0d};
        for (int i=0; i<2; i++)
                WriteFile(hCom,&cmd[i],1,&dWritten,NULL);
    }
}

void wp_t833::Feed(int count)
{
    for(int i=1; i<=count; i++)
    {
        unsigned char cmd[3]={0x20,0x0a,0x0d};
        for (int i=0; i<3; i++)
                WriteFile(hCom,&cmd[i],1,&dWritten,NULL);
    }
}

void wp_t833::Cut()
{
    if ((LoggingErrors)&&(Log))
        Log->Write("Cut()");
        //unsigned char cmd[3]={0x1d, 0x56, 0x01};
        unsigned char cmd[2]={0x1b, 0x69};
        for (int i=0; i<2; i++)
                WriteFile(hCom,&cmd[i],1,&dWritten,NULL);
}

void wp_t833::Font(unsigned char f)
{
    if ((LoggingErrors)&&(Log))
        Log->Write("Font(unsigned char f)");
        unsigned char cmd[3]={0x1b,0x21,0x20};
        cmd[3]=f;
        for (int i=0; i<3; i++)
                WriteFile(hCom,&cmd[i],1,&dWritten,NULL);
}

void wp_t833::SelectPrinter()
{
    if ((LoggingErrors)&&(Log))
        Log->Write("SelectPrinter()");
        unsigned char cmd[3]={0x1b,0x3d,0x01};
        for (int i=0; i<3; i++)
                WriteFile(hCom,&cmd[i],1,&dWritten,NULL);
}

void wp_t833::PrintCheck(AnsiString text, std::string barcode)
{
  Log->Write("PrintCheck");
  GetState();
  SetCodeTable();
  LinesCount = 0;

  char _subtext[1024];
  PrintData->Clear();
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
  while(true)
  {
      int pos = text.Pos(delim);
      if (pos == 0)
      {
          AnsiString subtext = text;
          memset(_subtext,0,1024);
          CharToOem(subtext.c_str(), _subtext);
          PrintData->Add(AnsiString(_subtext));
          LinesCount++;
          break;
      }
      AnsiString subtext = text.SubString(0,pos-1);
      text = text.SubString(pos+1,text.Length()-pos);
      if (subtext.IsEmpty())
      subtext=" ";
      memset(_subtext,0,1024);
      CharToOem(subtext.c_str(), _subtext);
      PrintData->Add(AnsiString(_subtext));
      LinesCount++;
  }
  PrintData->Add(" ");
  PrintData->Add(" ");
  PrintData->Add(" ");
  LinesCount+=3;
  if ((LoggingErrors)&&(Log))
      Log->Write("PrintCheck()");
  Print();
}

AnsiString wp_t833::GetStateDescription(BYTE code)
{
    std::string result;
    switch(code)
    {
        case 0x12:
            result = "OK";
            DeviceState->StateDescription = result;
            DeviceState->OutStateCode = DSE_OK;
            PrinterEnable = true;
            break;
        case 0x7E:
            Error = 1;
            DeviceState->StateChange = true;
            DeviceState->OutStateCode = DSE_NOTPAPER;
            result = "Бумага закончилась";
            DeviceState->StateDescription = result;
            PrinterEnable = false;
            if (OldState != code)
            {
              OldState = DeviceState->StateCode;
              DeviceState->StateCode = code;
              DeviceState->StateChange = true;
              if (ChangeEvent)
                StateChanged(DeviceState);
            }
            break;
        case 0x1E:
            Error = 1;
            DeviceState->StateChange = true;
            PrinterEnable = true;
            result = "Бумага скоро кончится";
            DeviceState->StateDescription = result;
            if(OldState != code)
            {
              OldState = DeviceState->StateCode;
              DeviceState->StateCode = code;
              DeviceState->OutStateCode = DSE_NEARENDPAPER;
              DeviceState->StateChange = true;
              if (ChangeEvent)
                StateChanged(DeviceState);
            }
            break;
        case 0xFF:
            Error = 1;
            DeviceState->StateChange = true;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            result = "Принтер недоступен";
            DeviceState->StateDescription = result;
            PrinterEnable = false;
            if (OldState != code)
            {
              OldState = DeviceState->StateCode;
              DeviceState->StateCode = code;
              DeviceState->StateChange = true;
              if (ChangeEvent)
                StateChanged(DeviceState);
            }
            break;
        default:
            //PrinterEnable = false;
            //DeviceState->OutStateCode = DSE_NOTMOUNT;
            PrinterEnable = true;
            DeviceState->OutStateCode = DSE_OK;
            result = "Unknown";
    }

}

void wp_t833::GetPrinterID()
{
  PurgeComm(hCom,PURGE_TXCLEAR || PURGE_RXCLEAR || PURGE_TXABORT || PURGE_RXABORT);
  BYTE cmd[3]={0x1D, 0x49, 49};
  for (int i=0; i<3; i++)
    WriteFile(hCom,&cmd[i],1,&dWritten,NULL);
  BYTE Buffer[32];
  DWORD LastError, BytesCount = 0;
  memset(Buffer,0,32);
  Sleep(1000);
  COMSTAT cs;
  ClearCommError(hCom, &LastError, &cs);
  ReadFile(hCom, Buffer, cs.cbInQue, &BytesCount, NULL);
  if ((LoggingErrors)&&(Log))
        Log->Write((boost::format("AnswerSize = %1%; Answer = %2%") % BytesCount % Buffer[0]).str().c_str());
}


void wp_t833::GetState()
{
  State = 0xFF;
  SubState = 0x00;
  std::string result = "";
  PurgeComm(hCom,PURGE_TXCLEAR || PURGE_RXCLEAR || PURGE_TXABORT || PURGE_RXABORT);
  //BYTE cmd[3]={0x1D, 0x72, 49};

  //==================== new 22-03-2007 ========================================
  // processing byte 2
  BYTE cmd[3]={0x10, 0x04, 2};
  WriteFile(hCom,cmd,3,&dWritten,NULL);
  BYTE Buffer[32];
  DWORD LastError, BytesCount = 0;
  memset(Buffer,0,32);
  Sleep(1000);
  COMSTAT cs;
  ClearCommError(hCom, &LastError, &cs);
  ReadFile(hCom, Buffer, cs.cbInQue, &BytesCount, NULL);
  if ((LoggingErrors)&&(Log))
        Log->Write((boost::format("Processint byte 2: AnswerSize = %1%; Answer = %2%") % BytesCount % Buffer[0]).str().c_str());
  if (BytesCount>0)
    State = Buffer[0];

  DeviceState->StateCode = State&0x40;
  DeviceState->SubStateCode = 0x00;
  if (DeviceState->StateCode > 0)
  {
      Error = 1;
      PrinterEnable = false;
      DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
      Log->Write("Processint byte 2: Printer Error! bit 6 = 1");
      result = "Ошибка принтера";
      DeviceState->StateDescription = result;
      if (OldState != DeviceState->StateCode)
      {
        OldState = DeviceState->StateCode;
        DeviceState->StateChange = true;
        if (ChangeEvent)
          StateChanged(DeviceState);
      }
      return;
  }
  //============================================================================
  // processing byte 3
  cmd[0] = 0x10;
  cmd[1] = 0x04;
  cmd[2] = 0x03;
  WriteFile(hCom,cmd,3,&dWritten,NULL);
  LastError, BytesCount = 0;
  memset(Buffer,0,32);
  Sleep(1000);
  ClearCommError(hCom, &LastError, &cs);
  ReadFile(hCom, Buffer, cs.cbInQue, &BytesCount, NULL);
  if ((LoggingErrors)&&(Log))
        Log->Write((boost::format("Processint byte 3: AnswerSize = %1%; Answer = %2%") % BytesCount % Buffer[0]).str().c_str());
  if (BytesCount>0)
    State = Buffer[0];

  DeviceState->StateCode = State&0x08;
  DeviceState->SubStateCode = 0x00;
  if (DeviceState->StateCode > 0)
  {
      Error = 1;
      PrinterEnable = false;
      DeviceState->OutStateCode = DSE_PAPERJAM;
      Log->Write("Processint byte 3: AutoCutter Error! bit 3 = 1");
      result = "Бумага застряла";
      DeviceState->StateDescription = result;
      if (OldState != DeviceState->StateCode)
      {
        OldState = DeviceState->StateCode;
        DeviceState->StateChange = true;
        if (ChangeEvent)
          StateChanged(DeviceState);
      }
      return;
  }

  DeviceState->StateCode = State&0x20;
  DeviceState->SubStateCode = 0x00;
  if (DeviceState->StateCode > 0)
  {
      Error = 1;
      PrinterEnable = false;
      DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
      Log->Write("Processint byte 3: Unrecoverable Error! bit 5 = 1");
      result = "Невосстановимая ошибка принтера";
      DeviceState->StateDescription = result;
      if (OldState != DeviceState->StateCode)
      {
        OldState = DeviceState->StateCode;
        DeviceState->StateChange = true;
        if (ChangeEvent)
          StateChanged(DeviceState);
      }
      return;
  }

  DeviceState->StateCode = State&0x40;
  DeviceState->SubStateCode = 0x00;
  if (DeviceState->StateCode > 0)
  {
      Error = 0;
      PrinterEnable = true;
      //DeviceState->OutStateCode = DSE_MAINERROR;
      Log->Write("Processint byte 3: Auto recoverable Error! bit 6 = 1");
      result = "Восстановимая ошибка принтера";
      DeviceState->StateDescription = result;
      if (OldState != DeviceState->StateCode)
      {
        OldState = DeviceState->StateCode;
        DeviceState->StateChange = true;
        if (ChangeEvent)
          StateChanged(DeviceState);
      }
      return;
  }
  //============================================================================

  cmd[0] = 0x10;
  cmd[1] = 0x04;
  cmd[2] = 0x04;
  //for (int i=0; i<3; i++)
    //WriteFile(hCom,&cmd[i],1,&dWritten,NULL);
  WriteFile(hCom,cmd,3,&dWritten,NULL);
  LastError, BytesCount = 0;
  memset(Buffer,0,32);
  Sleep(1000);
  ClearCommError(hCom, &LastError, &cs);
  ReadFile(hCom, Buffer, cs.cbInQue, &BytesCount, NULL);
  if ((LoggingErrors)&&(Log))
        Log->Write((boost::format("Processing byte 1: AnswerSize = %1%; Answer = %2%") % BytesCount % Buffer[0]).str().c_str());
  if (BytesCount>0)
    State = Buffer[0];

  PrinterEnable = true;
  DeviceState->StateCode = State;
  GetStateDescription(State);
  DeviceState->SubStateCode = 0x00;
  DeviceState->SubStateDescription = "";
  if ((LoggingErrors)&&(Log))
        Log->Write((boost::format("GetState() = %1%; %2%") % State % DeviceState->StateDescription).str().c_str());

  if ((DeviceState->OutStateCode == DSE_OK)&&(DeviceState->OutStateCode != DeviceState->OldOutStateCode))
  {
    PrinterEnable = true;
    OldState = State;
    DeviceState->OldOutStateCode = DeviceState->OutStateCode;
    DeviceState->StateChange = true;
    if (ChangeEvent)
        StateChanged(DeviceState);
    return;
  }
}

bool wp_t833::IsPrinterEnable()
{
  GetState();
  return PrinterEnable;
}

int wp_t833::Initialize()
{
    try
    {
      Init();
      Sleep(10);
      MinLineFeed();
      GetState();
      SetInitialized();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

bool wp_t833::IsItYou()
{
    //BYTE cmd[3]={0x1D, 0x72, 49};
    BYTE cmd[3]={0x10, 0x04, 4};
    for (int i=0; i<3; i++)
      WriteFile(hCom,&cmd[i],1,&dWritten,NULL);
    COMSTAT cs;
    BYTE Buffer[32];
    DWORD LastError = 0, BytesCount = 0;
    memset(Buffer,0,32);
    Sleep(100);
    ClearCommError(hCom, &LastError, &cs);
    ReadFile(hCom, Buffer, cs.cbInQue, &BytesCount, NULL);
    if (BytesCount == 0)
        return false;
    return true;
}

#pragma package(smart_init)

