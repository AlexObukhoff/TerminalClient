//---------------------------------------------------------------------------
#include "windows.h"
#include "winspool.h"
#pragma hdrstop
#include "boost/format.hpp"
#include "WinPrnClass.h"
#pragma package(smart_init)

CWinPrinter::CWinPrinter(HANDLE ComPort, int FontSize, TLogClass* _Log) : CPrinter(0,_Log, "WinPrinter")
{
    DeviceName = "WinPrinter";
    MainWindow = ComPort;
    PrintForm = new TRichEdit(MainWindow);
    PrintForm->Font->Name="Courier New";
    if (FontSize!=0)
      PrintForm->Font->Size=FontSize;
      else
      PrintForm->Font->Size=9;
    PrintForm->Font->Style = TFontStyles()<< fsBold;
    strings = new TStringList();
    SetInitialized();
    _MinLinesCount = 0;

    //Application->CreateForm(__classid(TPrintForm), &PrintForm);
    //PrintForm->Visible = false;
}

CWinPrinter::~CWinPrinter()
{
    delete strings;
    strings = NULL;

    delete PrintForm;
    PrintForm = NULL;
}

void CWinPrinter::PrintCheck(TStringList* Text)
{
    PrintForm->Clear();
    for(int i=0; i<Text->Count; i++)
        PrintForm->Lines->Add(Text->Strings[i]);
    PrintForm->Print("");
}

AnsiString CWinPrinter::GetStateDescription()
{
  return "";
}

void CWinPrinter::GetState()
{
    LPVOID lpMsgBuf = NULL;
    char Buffer[100];
    BYTE* ByteBuffer;
    DWORD size = 100;
    DWORD Needed, Returned, JobCount;
//    OSVERSIONINFO osv;
//    DWORD dwNeeded = 0;
//    PRINTER_INFO_2 *ppi2 = NULL;
//    LPTSTR pBuffer = NULL;
//    LONG lResult;
//    HMODULE hWinSpool = NULL;
//    PROC fnSetDefaultPrinter = NULL;
    HANDLE Prn = NULL;

    State = 0x00;
    SubState = 0x00;
    StateDescr = "";
    SubStateDescr = "";
    DeviceState->OutStateCode = DSE_UNKNOWN;


    bool result;
    memset(Buffer,0,100);

    result = GetDefaultPrinter(Buffer, &size);
    if (!result)
    {
      FormatMessage(
          FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
          NULL,
          GetLastError(),
          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
          (LPTSTR) &lpMsgBuf,
          0,
          NULL
      );
      if (Log != NULL)
        Log->Write((boost::format("Error GetDefaultPrinter! %1%") % lpMsgBuf).str().c_str());
      LocalFree(lpMsgBuf);
      DeviceState->OutStateCode = DSE_NOTMOUNT;
    }
    if (result == ERROR_FILE_NOT_FOUND)
       DeviceState->OutStateCode = DSE_NOTMOUNT;

    if (DeviceState->OutStateCode == DSE_NOTMOUNT)
    {
        ChangeDeviceState();
        PrinterEnable = false;
        return;
    }

    result = OpenPrinter(Buffer, &Prn, NULL);
    if (!result)
    {
      FormatMessage(
          FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
          NULL,
          GetLastError(),
          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
          (LPTSTR) &lpMsgBuf,
          0,
          NULL
      );
      if (Log != NULL)
        Log->Write((boost::format("Error OpenPrinter! %1%") % lpMsgBuf).str().c_str());
      LocalFree(lpMsgBuf);
      DeviceState->OutStateCode = DSE_NOTMOUNT;
    }
    if (!result || !Prn)
       DeviceState->OutStateCode = DSE_NOTMOUNT;

    if (DeviceState->OutStateCode == DSE_NOTMOUNT)
    {
        ChangeDeviceState();
        PrinterEnable = false;
        return;
    }

    /*SetLastError(0);
    result = GetPrinter(Prn, 2, 0, 0, &dwNeeded);
    if (!result)
    {
      FormatMessage(
          FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
          NULL,
          GetLastError(),
          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
          (LPTSTR) &lpMsgBuf,
          0,
          NULL
      );
      if (Log != NULL)
        Log->Write("Ошибка GetPrinter!"+AnsiString((char*)lpMsgBuf));
      LocalFree(lpMsgBuf);
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      if ((GetLastError() != ERROR_INSUFFICIENT_BUFFER) || (dwNeeded == 0))
        ClosePrinter(Prn);
    }

    // Allocate enough space for PRINTER_INFO_2.
    ppi2 = (PRINTER_INFO_2 *)GlobalAlloc(GPTR, dwNeeded);
    if (!ppi2)
    {
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      ClosePrinter(Prn);
    }

    // The second GetPrinter() will fill in all the current information
    // so that all you have to do is modify what you are interested in.
    result = GetPrinter(Prn, 2, (LPBYTE)ppi2, dwNeeded, &dwNeeded);
    if (!result)
    {
      FormatMessage(
          FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
          NULL,
          GetLastError(),
          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
          (LPTSTR) &lpMsgBuf,
          0,
          NULL
      );
      if (Log != NULL)
        Log->Write("Ошибка GetPrinter!"+AnsiString((char*)lpMsgBuf));
      LocalFree(lpMsgBuf);
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      ClosePrinter(Prn);
      GlobalFree(ppi2);
    }*/

    try
    {
        //result = EnumJobs(Prn, 0, 99, 1, NULL, 0, &Needed, &Returned);
        Needed = 10*1024;
        ByteBuffer = new BYTE[Needed];
        result = EnumJobs(Prn, 0, 99, 1, ByteBuffer, Needed, &Needed, &Returned);
    }
    __finally
    {
        delete [] ByteBuffer;
    }

    if (!result)
    {
      FormatMessage(
          FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
          NULL,
          GetLastError(),
          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
          (LPTSTR) &lpMsgBuf,
          0,
          NULL
      );
      if (Log != NULL)
        Log->Write((boost::format("Error EnumJobs! %1%") % lpMsgBuf).str().c_str());
      LocalFree(lpMsgBuf);
      DeviceState->OutStateCode = DSE_NOTMOUNT;
    }
    if (DeviceState->OutStateCode == DSE_NOTMOUNT)
    {
        ChangeDeviceState();
        PrinterEnable = false;
        return;
    }
    if (Needed > 0)
    {
        try
        {
            ByteBuffer = new BYTE[Needed];
            memset(ByteBuffer,0,Needed);
            memset(ByteBuffer,0,100);
            EnumJobs(Prn, 0, 99, 1, ByteBuffer, Needed, &Needed, &Returned);
            if (Returned > 0)
                JobCount = Returned;
            else
                JobCount = 0;
        }
        __finally
        {
            delete [] ByteBuffer;
        }
    }
    else
        JobCount = 0;

    if (Log != NULL)
      Log->Write((boost::format("Documents in the queue %1%") % JobCount).str().c_str());

    if (JobCount > 1)
    {
        //if (Log != NULL)
          //Log->Write("Документов в очереди "+AnsiString(JobCount));
        PrinterEnable = false;
        DeviceState->OutStateCode = DSE_NOTMOUNT;
    }

    if (DeviceState->OutStateCode == DSE_NOTMOUNT)
    {
        PrinterEnable = false;
        ChangeDeviceState();
        return;
    }

    DeviceState->OutStateCode = DSE_OK;
    if ((DeviceState->OutStateCode == DSE_OK)&&(DeviceState->OutStateCode != DeviceState->OldOutStateCode))
    {
       PrinterEnable = true;
       OldState = State;
       DeviceState->StateChange = true;
       ChangeDeviceState();
    }
}

std::string CWinPrinter::GetStatusDescription(BYTE StatusCode)
{
    UNREFERENCED_PARAMETER(StatusCode);
    return StateDescr = "";
}

void CWinPrinter::PrintCheck(AnsiString text, std::string barcode)
{
  //Log->Write("Text to printer: "+ text);
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
//  char _subtext[1024];

  while(true)
  {
    int pos = text.Pos(delim);
      if (pos == 0)
      {
          AnsiString subtext = text;
          strings->Add(subtext);
          LinesCount++;
          break;
      }
    AnsiString subtext = text.SubString(0,pos-1);
    text = text.SubString(pos+1,text.Length()-pos);
    strings->Add(subtext);
    LinesCount++;
    /*if (!subtext.IsEmpty())
    {
      strings->Add(subtext);
      LinesCount++;
    }*/
  }
  FeedToMinLinesCount();
  PrintCheck(strings);
  strings->Clear();
}

void CWinPrinter::Feed()
{
      strings->Add("");
}
