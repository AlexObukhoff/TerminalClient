//---------------------------------------------------------------------------


#pragma hdrstop

#include "ShtrihPrinter.h"
#include "globals.h"
#include <boost/format.hpp>

//---------------------------------------------------------------------------

#pragma package(smart_init)
#include <oleauto.hpp>

CShtrihPrinter::CShtrihPrinter(TLogClass* _Log) : CPrinter(0,_Log, "ShtrihPrinter")
{
  if (Log)
    Log->Write("Start Shtrih FR.");
  DrvEnabled = false;
  DeviceName = "ShtrihFR";

  Password = 30;
  Tax1 = 0;
  Tax2 = 0;
  Tax3 = 0;
  Tax4 = 0;
  Depart = "1";
  Summ1 = 0;
  Summ2 = 0;
  Summ3 = 0;
  Summ4 = 0;
  Store = 1;

  TryCount = 0;
  _MinLinesCount = 0;

    try
    {
        ECR = Variant::CreateObject("AddIn.DrvFR");
        DrvEnabled = true;
        CutterError = false;

        Connect();
        GetPrnState();
        SetInitialized();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        PrinterEnable = false;
        if (DeviceState)
        {
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            ChangeDeviceState();
        }
    }
}

CShtrihPrinter::~CShtrihPrinter()
{
    try
    {
        if (DrvEnabled)
        {
            Disconnect();
            //ECR = Unassigned;
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CShtrihPrinter::Sale(AnsiString strings)
{
  try
  {
     if (WaitForContinue() < 0)
       return;
     PropertySet password("Password");
     PropertySet price("Price");
     PropertySet quantity("Quantity");
     PropertySet store("Department");
     PropertySet tax1("Tax1");
     PropertySet tax2("Tax2");
     PropertySet tax3("Tax3");
     PropertySet tax4("Tax4");
     PropertySet string("StringForPrinting");

     password << Password;
     price << Price;
     quantity << Quantity;
     store << Store;
     tax1 << Tax1;
     tax2 << Tax2;
     tax3 << Tax3;
     tax4 << Tax4;

     //Summ1 = (Price *  Quantity);
     //Summ = Summ.sprintf("%f",dSumm);

     Log->Write((boost::format("price=%1%") % Price).str().c_str());
     Log->Write((boost::format("quantity=%1%") % Quantity).str().c_str());
     Log->Write((boost::format("store=%1%") % Store).str().c_str());
     Log->Write((boost::format("Summ=%1%") % Summ1).str().c_str());
     ECR.Exec(password);
     ECR.Exec(price);
     ECR.Exec(quantity);
     ECR.Exec(store);
     ECR.Exec(tax1);
     ECR.Exec(tax2);
     ECR.Exec(tax3);
     ECR.Exec(tax4);

     /*for(int i= 0; i<strings->Count; i++)
     {
       AnsiString text = strings->Strings[i];
       PrintString(text);
     }*/

//      int ind = 0;
      //Log->Write("Text to printer: "+ strings);
      AnsiString delim = "\r\n";
      while(true)
      {
        int pos = strings.Pos(delim);
        if (pos == 0)
          break;
        strings = strings.Delete(pos,2);
        strings = strings.Insert("|",pos);
      }
      delim = "|";
      LinesCount = 0;
      while(true)
      {
        int pos = strings.Pos(delim);
        if (pos == 0)
          break;
        AnsiString subtext = "";
        subtext = strings.SubString(0,pos-1);
        strings = strings.SubString(pos+1,strings.Length()-pos);
        //if (!subtext.IsEmpty())
        //{
          LinesCount++;
          PrintString(subtext);
          //Log->Write("Text to printer: "+ subtext);
        //}
      }
     string << " ";
     ECR.Exec(string);

     ECR.Exec(sale);

     Variant res = ECR.Exec(error);
     ResultCode = res.ChangeType(varInteger);
     Variant resDesc = ECR.Exec(errorDesc);

     ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
     Log->Write((boost::format("Sale() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
     OperatorNumber = ECR.Exec(operatornum);

     if (ResultCode == 0)
     {
          TryCount = 0;
          PrinterEnable = true;
          CutterError = false;
          DeviceState->OutStateCode = DSE_OK;
          DeviceState->StateCode = ResultCode;
          DeviceState->StateDescription = WideCharToString(resDesc.VOleStr).c_str();
          ChangeDeviceState();
     }
     else
     if (res.ChangeType(varInteger) < 0)
     {
        PrinterEnable = false;
        //DeviceState->OldOutStateCode = DSE_NOTMOUNT;
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        DeviceState->StateCode = res.ChangeType(varInteger);
        DeviceState->StateDescription = WideCharToString(resDesc.VOleStr).c_str();
        ChangeDeviceState();
     }
     else
     if (res.ChangeType(varInteger) > 0)
     {
        PrinterEnable = true;
        ///new 07-12-2006
        if (res.ChangeType(varInteger) > 0)
        {
            CheckResult(ResultCode);
            DeviceState->StateCode = ResultCode;
            DeviceState->StateDescription = WideCharToString(resDesc.VOleStr).c_str();
            if (ResultCode != 113)
                ChangeDeviceState();
            //ChangeDeviceState();
            TryCount = 0;
            /*while((ResultCode == 113)&&(TryCount<10))
            {
                CutCheck();
                TryCount++;
            }*/
            if (ResultCode == 113)
            {
                PrinterEnable = false;
                CutterError = true;
                ChangeDeviceState();
            }
           if (ResultCode == 0)
           {
                TryCount = 0;
                PrinterEnable = true;
                CutterError = false;
                DeviceState->OutStateCode = DSE_OK;
                DeviceState->StateCode = ResultCode;
                DeviceState->StateDescription = WideCharToString(resDesc.VOleStr).c_str();
                ChangeDeviceState();
           }
        }
     }

     CloseCheck();
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CShtrihPrinter::ShowProperties()
{
    try
    {
       ECR.Exec(sp);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CShtrihPrinter::Connect()
{
  try
  {
     try
     {
       ECR.Exec(_connect);
     }
     catch(...)
     {}
     Variant res = ECR.Exec(error);
     Variant resDesc = ECR.Exec(errorDesc);
     ErrorMsg = IntToStr(int(res)) + ": " + resDesc;
     if (res.ChangeType(varInteger) < 0)
     {
        PrinterEnable = false;
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        DeviceState->StateCode = res.ChangeType(varInteger);
        DeviceState->StateDescription = WideCharToString(resDesc.VOleStr).c_str();
        ChangeDeviceState();
     }
     else
        PrinterEnable = true;
     Log->Write((boost::format("Connect() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CShtrihPrinter::Disconnect()
{
    try
    {
       ECR.Exec(disconnect);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CShtrihPrinter::Beep()
{
  try
  {
     if (WaitForContinue() < 0)
       return;
     PropertySet password("Password");

     password << Password;
     ECR.Exec(password);
     ECR.Exec(beep);
     Variant res = ECR.Exec(error);
     Variant resDesc = ECR.Exec(errorDesc);
     ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
     Log->Write((boost::format("Beep() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
     OperatorNumber = ECR.Exec(operatornum);
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CShtrihPrinter::Charge()
{
  try
  {
     if (WaitForContinue() < 0)
       return;
     PropertySet password("Password");
     PropertySet summ("Summ1");
     PropertySet tax1("Tax1");
     PropertySet tax2("Tax2");
     PropertySet tax3("Tax3");
     PropertySet tax4("Tax4");
     PropertySet string("StringForPrinting");

     password << Password;
     summ << Summ1;
     tax1 << Tax1;
     tax2 << Tax2;
     tax3 << Tax3;
     tax4 << Tax4;
     string << Text;

     ECR.Exec(password);
     ECR.Exec(summ);
     ECR.Exec(tax1);
     ECR.Exec(tax2);
     ECR.Exec(tax3);
     ECR.Exec(tax4);
     ECR.Exec(string);

     ECR.Exec(charge);

     Variant res = ECR.Exec(error);
     Variant resDesc = ECR.Exec(errorDesc);
     ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
     Log->Write((boost::format("Charge() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
     OperatorNumber = ECR.Exec(operatornum);
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CShtrihPrinter::PrintString(AnsiString text)
{
  try
  {
      PropertySet password("Password");
      PropertySet string("StringForPrinting");

      string << text;
      password << Password;
      ECR.Exec(password);
      ECR.Exec(string);
      ECR.Exec(printstring);

      Log->Write((boost::format("PrintString() text = %1%") % text.c_str()).str().c_str());
     Variant res = ECR.Exec(error);
     Variant resDesc = ECR.Exec(errorDesc);
     ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
     if (static_cast<bool>(res))
        Log->Write((boost::format("PrintString() ErrorMsg: %1%" ) % ErrorMsg.c_str()).str().c_str());
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CShtrihPrinter::ReturnSale()
{
  try
  {
     if (WaitForContinue() < 0)
       return;
     PropertySet password("Password");
     PropertySet price("Price");
     PropertySet quantity("Quantity");
     PropertySet store("Department");
     PropertySet tax1("Tax1");
     PropertySet tax2("Tax2");
     PropertySet tax3("Tax3");
     PropertySet tax4("Tax4");
     PropertySet string("StringForPrinting");

     password << Password;
     price << Price;
     quantity << Quantity;
     store << Store;
     tax1 << Tax1;
     tax2 << Tax2;
     tax3 << Tax3;
     tax4 << Tax4;
     string << Text;

     ECR.Exec(password);
     ECR.Exec(price);
     ECR.Exec(quantity);
     ECR.Exec(store);
     ECR.Exec(tax1);
     ECR.Exec(tax2);
     ECR.Exec(tax3);
     ECR.Exec(tax4);
     ECR.Exec(string);

     ECR.Exec(returnsale);

     Variant res = ECR.Exec(error);
     Variant resDesc = ECR.Exec(errorDesc);
     ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
     Log->Write((boost::format("ReturnSale() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
     OperatorNumber = ECR.Exec(operatornum);

     CloseCheck();
     //CutCheck();
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CShtrihPrinter::Discount()
{
  try
  {
     if (WaitForContinue() < 0)
       return;
     PropertySet password("Password");
     PropertySet summ("Summ1");
     PropertySet tax1("Tax1");
     PropertySet tax2("Tax2");
     PropertySet tax3("Tax3");
     PropertySet tax4("Tax4");
     PropertySet string("StringForPrinting");

     password << Password;
     summ << Summ1;
     tax1 << Tax1;
     tax2 << Tax2;
     tax3 << Tax3;
     tax4 << Tax4;
     string << Text;

     ECR.Exec(password);
     ECR.Exec(summ);
     ECR.Exec(tax1);
     ECR.Exec(tax2);
     ECR.Exec(tax3);
     ECR.Exec(tax4);
     ECR.Exec(string);

     ECR.Exec(discount);

     Variant res = ECR.Exec(error);
     Variant resDesc = ECR.Exec(errorDesc);
     ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
     Log->Write((boost::format("Discount() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
     OperatorNumber = ECR.Exec(operatornum);
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CShtrihPrinter::CloseCheck()
{
  try
  {
     if (WaitForContinue() < 0)
       return;
     PropertySet password("Password");
     PropertySet summ1("Summ1");
     PropertySet summ2("Summ2");
     PropertySet summ3("Summ3");
     PropertySet summ4("Summ4");
     PropertySet tax1("Tax1");
     PropertySet tax2("Tax2");
     PropertySet tax3("Tax3");
     PropertySet tax4("Tax4");
     PropertySet string("StringForPrinting");

     password << Password;
     Log->Write((boost::format("CloseCheck() Summ1=%1%") % Summ1).str().c_str());
     summ1 << Summ1;
     summ2 << Summ2;
     summ3 << Summ3;
     summ4 << Summ4;
     tax1 << Tax1;
     tax2 << Tax2;
     tax3 << Tax3;
     tax4 << Tax4;
     string << Text;

     ECR.Exec(password);
     ECR.Exec(summ1);
     ECR.Exec(summ2);
     ECR.Exec(summ3);
     ECR.Exec(summ4);
     ECR.Exec(tax1);
     ECR.Exec(tax2);
     ECR.Exec(tax3);
     ECR.Exec(tax4);
     ECR.Exec(string);

     ECR.Exec(closecheck);

     Variant res = ECR.Exec(error);
     Variant resDesc = ECR.Exec(errorDesc);
     ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
     OperatorNumber = ECR.Exec(operatornum);
     Log->Write((boost::format("CloseCheck() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
     Change = ECR.Exec(change);

     PresenterPush();
     Sleep(1000);
     CutCheck();
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CShtrihPrinter::CancelCheck()
{
  try
  {
     if (WaitForContinue() < 0)
       return;
     PropertySet password("Password");

     password << Password;
     ECR.Exec(password);

     ECR.Exec(cancelcheck);

     Variant res = ECR.Exec(error);
     Variant resDesc = ECR.Exec(errorDesc);
     ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
     Log->Write((boost::format("CancelCheck() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
     OperatorNumber = ECR.Exec(operatornum);

     //CutCheck();
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CShtrihPrinter::PrintXReport(AnsiString Text)
{
    if (Fiscal)
      PrintReportWithoutCleaning();
    else
      PrintCheck(Text);
}


void CShtrihPrinter::PrintReportWithoutCleaning()
{
  try
  {
     //if (WaitForContinue() < 0)
       //return;
     PropertySet password("Password");

     password << Password;
     ECR.Exec(password);
     ECR.Exec(reportwithoutcleaning);
     Variant res = ECR.Exec(error);
     Variant resDesc = ECR.Exec(errorDesc);
     ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
     Log->Write((boost::format("PrintReportWithoutCleaning() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
     OperatorNumber = ECR.Exec(operatornum);

     //CutCheck();
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CShtrihPrinter::PrintReportWithCleaning()
{
  try
  {
     //if (WaitForContinue() < 0)
       //return;
     PropertySet password("Password");

     password << Password;
     ECR.Exec(password);
     ECR.Exec(reportwithcleaning);
     Variant res = ECR.Exec(error);
     Variant resDesc = ECR.Exec(errorDesc);
     ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
     Log->Write((boost::format("PrintReportWithCleaning() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
     OperatorNumber = ECR.Exec(operatornum);

     //CutCheck();
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CShtrihPrinter::ContinuePrint()
{
  try
  {
     //if (WaitForContinue() < 0)
       //return;
     PropertySet password("Password");

     password << Password;

     ECR.Exec(password);

     ECR.Exec(continueprint);

     Variant res = ECR.Exec(error);
     Variant resDesc = ECR.Exec(errorDesc);
     ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
     Log->Write((boost::format("ContinuePrint() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
     OperatorNumber = ECR.Exec(operatornum);
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

bool CShtrihPrinter::IsPrinterEnable()
{
   /*CutCheck();
   if (ResultCode < 0)
   {
      PrinterEnable = false;
      Log->Write("PrinterEnable = false, DeviceState->OutStateCode = DSE_NOTMOUNT, "+DeviceState->StateDescription);
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      DeviceState->StateCode = ResultCode;
      ChangeDeviceState();
      return false;
   }
   if (ResultCode != 0)
   {
      CheckResult(ResultCode);
      if ((DeviceState->OutStateCode == DSE_HARDWARE_ERROR)||(DeviceState->OutStateCode == DSE_NOTPAPER))
      {
        Log->Write("PrinterEnable = false, DeviceState->OutStateCode = "+DeviceState->OutStateDescription(DeviceState->OutStateCode)+", "+DeviceState->StateDescription);
        DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
        DeviceState->StateCode = ResultCode;
        ChangeDeviceState();
        return false;
      }
   }*/


   GetPrnState(true);
   //new 24-09-2007
   //canceling the cheque before new printing
   if (DeviceState->StateCode == OpenedDoc)
      //CloseCheck();
       CancelCheck();
   return PrinterEnable;
}

void CShtrihPrinter::GetState()
{
    GetPrnState(true);
}

void CShtrihPrinter::GetPrnState(bool SendNotification)
{
  try
  {
     State = 0xFF;
     SubState = 0xFF;
     StateDescr = "";
     SubStateDescr = "";

     PropertySet password("Password");

     password << StrToInt(Password);

     ECR.Exec(password);

     ECR.Exec(getstatus);

     State = -1;
     Variant res = ECR.Exec(error);
     Variant resDesc = ECR.Exec(errorDesc);
     ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
     Log->Write((boost::format("GetState() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
     int rescode = res.ChangeType(varInteger);
     if (res.ChangeType(varInteger) < 0)
     {
        PrinterEnable = false;
        Log->Write("PrinterEnable = false, DeviceState->OutStateCode = DSE_NOTMOUNT");
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        DeviceState->StateCode = res.ChangeType(varInteger);
        DeviceState->StateDescription = WideCharToString(resDesc.VOleStr).c_str();
        if (SendNotification)
            ChangeDeviceState();
     }

     /*if (res.ChangeType(varInteger) == 0)
     {
        PrinterEnable = true;
        TryCount = 0;
        Log->Write("PrinterEnable = true");
        DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateCode = res.ChangeType(varInteger);
        DeviceState->StateDescription = WideCharToString(resDesc.VOleStr).c_str();
        ChangeDeviceState();
     }*/

     if (res.ChangeType(varInteger) < 0)
        return;
     OperatorNumber = ECR.Exec(operatornum);
     DeviceState->StateCode = State = ECRMode = ECR.Exec(mode).ChangeType(varInteger);
     DeviceState->StateDescription = StateDescr = ECRModeDescr = WideCharToString(ECR.Exec(modedescr).VOleStr).c_str();
     DeviceState->SubStateCode = SubState = ECRAdvancedMode = ECR.Exec(advmode).ChangeType(varInteger);
     DeviceState->SubStateDescription = SubStateDescr = ECRAdvancedModeDescr = WideCharToString(ECR.Exec(advmodedescr).VOleStr).c_str();
     Log->Write((boost::format("ECRMode %1%: %2%") % ECRMode % ECRModeDescr).str().c_str());
     Log->Write((boost::format("ECRAdvancedMode %1%: %2%") % ECRAdvancedMode % ECRAdvancedModeDescr).str().c_str());
     //смотрим, истекли ли 24 часа
     ShiftOver  = bool(ECR.Exec(shiftover));

     if (SubState == 3)
       ContinuePrint();

     switch(SubState)
     {
        case 1:
        case 2:
            PrinterEnable = false;
            Log->Write("PrinterEnable = false, DeviceState->OutStateCode = DSE_NOTPAPER");
            DeviceState->OutStateCode = DSE_NOTPAPER;
            if (SendNotification)
                ChangeDeviceState();
            //break;
            return;
        case 0:
        case 3:
        case 4:
        case 5:
            if (DeviceState->OutStateCode != DSE_OK)
            {
              if ((SendNotification)&&(CutterError == false))
              {
                  DeviceState->OutStateCode = DSE_OK;
                  TryCount = 0;
                  PrinterEnable = true;
                  Log->Write("PrinterEnable = true");
                  ChangeDeviceState();
              }
              else
              {
                 if (CutterError)
                 {
                     PrinterEnable = false;
                     Log->Write("Error! CutterError = true, PrinterEnable = false");
                     DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
                 }
              }
            }
            break;
     };

     SessionOpened = true;
     if ((ShiftOver)||(State == 3)||(State == 4))
     {
       Log->Write((boost::format("ECRMode: %1%; ECRAdvancedMode: %2%; ShiftOver = true") % ECRModeDescr.c_str() % ECRAdvancedModeDescr.c_str()).str().c_str());
       //DeviceState->OutStateCode = DSE_MAINERROR;
       //PrinterEnable = false;
       if ((ShiftOver)||(State == 3))
         SessionOpened = false;
       if (State == 4)
         SessionOpened = true;
       OpenNewSession();
       State = 0;
     }

     if ((rescode == 0)&&(CutterError == false))
     {
        PrinterEnable = true;
        TryCount = 0;
        Log->Write("PrinterEnable = true");
        DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateCode = res.ChangeType(varInteger);
        DeviceState->StateDescription = WideCharToString(resDesc.VOleStr).c_str();
        ChangeDeviceState();
     }
     else
     {
         if (CutterError)
         {
             PrinterEnable = false;
             Log->Write("Error! CutterError = true, PrinterEnable = false");
             DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
         }
     }

     //new 24-09-2007
     //canceling the cheque before new printing
     //if (State == OpenedDoc)
         //CancelCheck();
     DeviceState->StateCode = State;
 }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CShtrihPrinter::CutCheck()
{
  try
  {
     //if (WaitForContinue() < 0)
       //return;
     PropertySet password("Password");
     PropertySet cuttype("CutType");

     password << Password;
     cuttype << false;

     ECR.Exec(password);
     ECR.Exec(cuttype);

     ECR.Exec(cutcheck);

     Variant res = ECR.Exec(error);
     Variant resDesc = ECR.Exec(errorDesc);
     ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
     ResultCode = res.ChangeType(varInteger);
     if (ResultCode == 113)
        CutterError = true;
     if (ResultCode == 0)
        CutterError = false;
     DeviceState->StateDescription = WideCharToString(resDesc.VOleStr).c_str();
     Log->Write((boost::format("CutCheck() ErrorMsg: %1%")  % ErrorMsg.c_str()).str().c_str());
     OperatorNumber = ECR.Exec(operatornum);
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CShtrihPrinter::FeedDocument(int count)
{
  try
  {
     if (WaitForContinue() < 0)
       return;
     PropertySet password("Password");
     PropertySet stringquantity("StringQuantity");

     password << Password;
     if (count <= 0)
         count = 1;
     stringquantity << count;

     ECR.Exec(password);
     ECR.Exec(stringquantity);

     ECR.Exec(feeddocument);

     Variant res = ECR.Exec(error);
     Variant resDesc = ECR.Exec(errorDesc);
     ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
     Log->Write((boost::format("FeedDocument() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
     OperatorNumber = ECR.Exec(operatornum);
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

bool CShtrihPrinter::Continue()
{
    bool result = false;
    GetPrnState();
    switch ((PrnState)ECRAdvancedMode)
    {
      case OK:
        result = true;
        break;
      case NoPaperActive:
      case NoPaperPassive:
      case Printing:
      case LongPrinting:

      //22-11-2006
      case ReportPrinting:
      case EKLZReportPrinting:
      //---------

      case Unknown:
        result = false;
        break;
      case PaperEnable:
        ContinuePrint();
        result = false;
        break;
    }
    return result;
}

bool CShtrihPrinter::WaitForPrinting()
{
  bool result = false;
  try
  {
     PropertySet password("Password");

     password << Password;

     ECR.Exec(password);
     ECR.Exec(waitforprinting);

     Variant res = ECR.Exec(error);
     Variant resDesc = ECR.Exec(errorDesc);
     ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
     Log->Write((boost::format("WaitForPrinting() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
     int Result = res.ChangeType(varInteger);

      switch (Result)
      {
        case OK:
          result = true;
          break;
        case NoPaperActive:
        case NoPaperPassive:
        case Printing:
        case LongPrinting:
        case ReportPrinting:
        case EKLZReportPrinting:
        case Unknown:
          result = false;
          break;
        case PaperEnable:
          ContinuePrint();
          result = false;
          break;
      }
      return result;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return result;
    }
}

int CShtrihPrinter::WaitForContinue()
{
  while (!Continue())
  {
    if (((PrnState)ECRAdvancedMode == NoPrinter)||((PrnState)ECRAdvancedMode == Unknown))
      return -1;
    Sleep(100);
  }
  return 0;
}

void CShtrihPrinter::PresenterPush()
{
  try
  {
     //if (WaitForContinue() < 0)
       //return;
     PropertySet password("Password");

     password << StrToInt(Password);

     ECR.Exec(password);
     ECR.Exec(presenterpush);

     Variant res = ECR.Exec(error);
     Variant resDesc = ECR.Exec(errorDesc);
     ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
     Log->Write((boost::format("PresenterPush() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
     OperatorNumber = ECR.Exec(operatornum);
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CShtrihPrinter::OpenScreen()
{
  try
  {
     //if (WaitForContinue() < 0)
       //return;
     PropertySet password("Password");

     password << Password;

     ECR.Exec(password);
     ECR.Exec(openscreen);

     Variant res = ECR.Exec(error);
     Variant resDesc = ECR.Exec(errorDesc);
     ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
     Log->Write((boost::format("OpenScreen() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
     OperatorNumber = ECR.Exec(operatornum);
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}


void CShtrihPrinter::CloseScreen()
{
  try
  {
     //if (WaitForContinue() < 0)
       //return;
     PropertySet password("Password");

     password << Password;

     ECR.Exec(password);
     ECR.Exec(closescreen);

     Variant res = ECR.Exec(error);
     Variant resDesc = ECR.Exec(errorDesc);
     ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
     Log->Write((boost::format("CloseScreen() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
     OperatorNumber = ECR.Exec(operatornum);
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CShtrihPrinter::OpenSession()
{
  try
  {
     //if (WaitForContinue() < 0)
       //return;
     PropertySet password("Password");

     password << Password;

     ECR.Exec(password);
     ECR.Exec(opensession);

     Variant res = ECR.Exec(error);
     Variant resDesc = ECR.Exec(errorDesc);
     ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
     Log->Write((boost::format("OpenSession() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
     OperatorNumber = ECR.Exec(operatornum);
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CShtrihPrinter::SaleEx(double Money, AnsiString strings)
{
  Quantity = 1;
  Price = Money;
  Summ1 = Money;
  Sale(strings);
}

void CShtrihPrinter::PrintCheck(double Money, AnsiString Text)
{
  Log->Write((boost::format("PrintCheck( %1%, %2%)") % Money % Text.c_str()).str().c_str());
  Quantity = 1;
  Price = Money;
  Summ1 = Money;
  Sale(Text);
}

void CShtrihPrinter::OpenNewSession()
{
  try
  {
      if (AutoOpenShift == false)
      {
          if (ZReportInBuffer)//пробуем печатать Z отчёт во внутренний буфер ФР
          {
              int result = _PrintZReportInBuffer();
              if (result != 0)//не получилось, выдаём все Z отчёты на печать и печатаем обычный Z отчёт
                  Log->Write("OpenNewSession: AutoOpenShif = false, Session Closed.");
          }
          PropertySet password("Password");
          password << Password;
          ECR.Exec(password);
          ECR.Exec(getstatus);
          Variant res = ECR.Exec(error);
          Variant resDesc = ECR.Exec(errorDesc);
          ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
          Log->Write((boost::format("OpenNewSession() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
          if (res.ChangeType(varInteger) < 0)
          {
              PrinterEnable = false;
              Log->Write("OpenNewSession() PrinterEnable = false");
           }
           else
           {
              PrinterEnable = true;
              Log->Write("OpenNewSession() PrinterEnable = true");
           }
           if (res.ChangeType(varInteger) < 0)
              return;
           DeviceState->StateCode = State = ECRMode = ECR.Exec(mode).ChangeType(varInteger);
           StateDescr = ECRModeDescr = WideCharToString(ECR.Exec(modedescr).VOleStr).c_str();
           DeviceState->SubStateCode = SubState = ECRAdvancedMode = ECR.Exec(advmode).ChangeType(varInteger);
           SubStateDescr = ECRAdvancedModeDescr = WideCharToString(ECR.Exec(advmodedescr).VOleStr).c_str();
           Log->Write((boost::format("ECRMode %1%: %2%") % ECRMode % ECRModeDescr).str().c_str());
           Log->Write((boost::format("ECRAdvancedMode %1%: %2%") % ECRAdvancedMode % ECRAdvancedModeDescr).str().c_str());
           ShiftOver  = ECR.Exec(shiftover);
           if ((ShiftOver)||(State == 3))
             SessionOpened = false;
           if (State == 4)
             SessionOpened = true;
           Log->Write((boost::format("ECRMode: %1%; ECRAdvancedMode: %2%; ShiftOver = true") % ECRModeDescr.c_str() % ECRAdvancedModeDescr.c_str()).str().c_str());

           return;
      }

      //печатаем Z отчёт только если смена открыта
      if (DeviceState->StateCode != 4)
      //PrintReportWithCleaning();
      {
          if (ZReportInBuffer)//пробуем печатать Z отчёт во внутренний буфер ФР
          {
              int result = _PrintZReportInBuffer();
              Wait();
              if (result != 0)//не получилось, выдаём все Z отчёты на печать и печатаем обычный Z отчёт
              {
                  _PrintZReportFromBuffer();
                  /*int Timeout = 60; //seconds
                  int Interval = 1000; //miliseconds
                  int ticks = Timeout*1000/Interval;
                  while((WaitForContinue()<0)&&(ticks>0))
                  {
                      ticks--;
                      Sleep(Interval);
                  }*/
                  //WaitForPrinting();
                  Wait();
                  PrintReportWithCleaning();
              }
          }
          else
              PrintReportWithCleaning();
      }
      OpenSession();
      CloseScreen();
      PresenterPush();
      OpenScreen();

     PropertySet password("Password");
     password << Password;
     ECR.Exec(password);
     ECR.Exec(getstatus);

     Variant res = ECR.Exec(error);
     Variant resDesc = ECR.Exec(errorDesc);
     ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
     Log->Write((boost::format("OpenNewSession() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
     if (res.ChangeType(varInteger) < 0)
     {
        PrinterEnable = false;
        Log->Write("OpenNewSession() PrinterEnable = false");
        //DeviceState->OldOutStateCode = DSE_NOTMOUNT;
     }
     else
     {
        PrinterEnable = true;
        Log->Write("OpenNewSession() PrinterEnable = true");
     }
     if (res.ChangeType(varInteger) < 0)
        return;
     OperatorNumber = ECR.Exec(operatornum);
     DeviceState->StateCode = State = ECRMode = ECR.Exec(mode).ChangeType(varInteger);
     StateDescr = ECRModeDescr = WideCharToString(ECR.Exec(modedescr).VOleStr).c_str();
     DeviceState->SubStateCode = SubState = ECRAdvancedMode = ECR.Exec(advmode).ChangeType(varInteger);
     SubStateDescr = ECRAdvancedModeDescr = WideCharToString(ECR.Exec(advmodedescr).VOleStr).c_str();
     Log->Write((boost::format("ECRMode %1%: %2%") % ECRMode % ECRModeDescr).str().c_str());
     Log->Write((boost::format("ECRAdvancedMode %1%: %2%") % ECRAdvancedMode % ECRAdvancedMode).str().c_str());
     ShiftOver  = ECR.Exec(shiftover);
     if ((ShiftOver)||(State == 3))
       SessionOpened = false;
     if (State == 4)
       SessionOpened = true;
     Log->Write((boost::format("ECRMode: %1%; ECRAdvancedMode: %2%; ShiftOver = true") % ECRModeDescr.c_str() % ECRAdvancedModeDescr.c_str()).str().c_str());
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CShtrihPrinter::PrintCheck(TStringList* strings)
{
   Log->Write("PrintCheck()");
   for(int i= 0; i<strings->Count; i++)
   {
     AnsiString text = strings->Strings[i];
     PrintString(text);
   }
   //for(int i= 0; i<8; i++)
     //PrintString(" ");
   FeedToMinLinesCount();
   CutCheck();
   PresenterPush();
}

void CShtrihPrinter::Feed(int count)
{
    FeedDocument(count);
    //PrintString(" ");
}

void CShtrihPrinter::PrintCheck(AnsiString text, std::string barcode)
{
  try
  {
//      int ind = 0;
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
      LinesCount = 0;
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
        LinesCount++;
        strings->Add(subtext);
        /*if (!subtext.IsEmpty())
        {
          LinesCount++;
          strings->Add(subtext);
        }*/
      }
      PrintCheck(strings);
      delete strings;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CShtrihPrinter::CheckResult(int value)
{
    PrinterEnable = true;
    switch (value)
    {
        case 0:
            PrinterEnable = true;
            DeviceState->OutStateCode = DSE_OK;
            break;
        case 194:
            PrinterEnable = true;
            //DeviceState->OutStateCode = DSE_MAINERROR;
            break;
        case 38:
        case 64:
        case 67:
        case 56:
        case 57:
        case 100:
        case 103:
        case 113:
        case 116:
        case 117:
        case 118:
        case 119:
        case 123:
        case 128:
        case 129:
        case 130:
        case 131:
        case 160:
        case 161:
        case 163:
        case 164:
        case 165:
        case 166:
        case 167:
            PrinterEnable = false;
            DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
            break;
        case 107:
        case 108:
            PrinterEnable = false;
            DeviceState->OutStateCode = DSE_NOTPAPER;
            break;
    }
}


//печать Z отчёта во внутренний буфер ФР, поддерживается до 5 раз
int CShtrihPrinter::_PrintZReportInBuffer()
{
    try
    {
         int result = 0;
         PropertySet password("Password");

         password << Password;

         ECR.Exec(password);
         ECR.Exec(printinbuffer);

         Variant res = ECR.Exec(error);
         Variant resDesc = ECR.Exec(errorDesc);
         result = res.ChangeType(varInteger);
         ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
         Log->Write((boost::format("_PrintZReportInBuffer() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
         return result;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return -1;
    }
}

//печать всех Z отчётов из буфера ФР
int CShtrihPrinter::_PrintZReportFromBuffer()
{
    try
    {
         PropertySet password("Password");

         password << Password;

         ECR.Exec(password);
         ECR.Exec(printfrombuffer);

         Variant res = ECR.Exec(error);
         Variant resDesc = ECR.Exec(errorDesc);
         ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
         Log->Write((boost::format("_PrintZReportFromBuffer() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
         return res.ChangeType(varInteger);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return -1;
    }
}

void CShtrihPrinter::PrintZReport(AnsiString Text)
{
    //ждём возможности начала печати отчёта
    Wait();

    if (Fiscal)//фискальный режим
    {
        if (ZReportInBuffer)//пробуем печатать Z отчёт во внутренний буфер ФР
        {
            /*int result = _PrintZReportInBuffer();
            if (result != 0)//не получилось, выдаём все Z отчёты на печать и печатаем обычный Z отчёт
            {
                _PrintZReportFromBuffer();
                PrintReportWithCleaning();
            }*/
            _PrintZReportFromBuffer();
            Wait();
            PrintReportWithCleaning();
            /*if (_PrintZReportFromBuffer() != 0)
            {
                Wait();
                PrintReportWithCleaning();
            }*/
            /*int Timeout = 60; //seconds
            int Interval = 1000; //miliseconds
            int ticks = Timeout*1000/Interval;
            while((WaitForContinue()<0)&&(ticks>0))
            {
                ticks--;
                Sleep(Interval);
            }*/
            //WaitForPrinting();
            //PrintReportWithCleaning();
        }
        else
        {
            PrintReportWithCleaning();
        }
    }
    else//нефискальный режим
    {
        PrintCheck(Text);
        /*if (ZReportInBuffer)
        {
            int result = _PrintZReportInBuffer();
            if (result != 0)
            {
                _PrintZReportFromBuffer();
                PrintCheck(Text);
            }
        }
        else
            PrintCheck(Text);*/
    }
}

void CShtrihPrinter::FeedToMinLinesCount()
{
    int Count = MinLinesCount - LinesCount;
    if (Log)
    if (Count>0)
    {
       Log->Write((boost::format("Feeding to %1%  lines") % Count).str().c_str());
       //Feed(Count);
       for(int i=1; i<=Count; i++)
       {
          PrintString(" ");
       }
    }
}

void CShtrihPrinter::CashIncassation(double Money)
{
    AnsiString CashName = "";
    if (Money <= 0)
    {
        try
        {
             Money = 0;
             PropertySet password("Password");
             PropertySet registernumber("RegisterNumber");
             PropertyGet ContOfCashReg("ContentsOfCashRegister");
             PropertyGet namecashreg("NameCashReg");

             password << Password;
             registernumber << 241;

             ECR.Exec(password);
             ECR.Exec(registernumber);
             ECR.Exec(getcashreg);
             double ResultMoney = 0;
             CashName = (AnsiString)ECR.Exec(namecashreg);
             Variant _ResultMoney = ECR.Exec(ContOfCashReg);
             ResultMoney = (double)_ResultMoney;


             Variant res = ECR.Exec(error);
             Variant resDesc = ECR.Exec(errorDesc);
             ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
             Log->Write((boost::format("ContentsOfCashRegister() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
             if (ResultMoney > 0)
                Money = ResultMoney;
             else
             {
                 Log->Write((boost::format("Money in the printer=%1%; No further incassation.") % Money).str().c_str());
                 return;
             }
        }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            Log->Write("ContentsOfCashRegister() Exception");
        }
    }

    Log->Write((boost::format("Money taking out the printer=%1%; Name of CashRegistr=%2%") % Money % CashName.c_str()).str().c_str());
    try
    {
         PropertySet password("Password");
         PropertySet summ1("Summ1");

         password << Password;
         summ1 << Money;

         ECR.Exec(password);
         ECR.Exec(summ1);
         ECR.Exec(cashoutcome);

         Variant res = ECR.Exec(error);
         Variant resDesc = ECR.Exec(errorDesc);
         ErrorMsg = IntToStr(static_cast<int>(res.ChangeType(varInteger))) + ": " + WideCharToString(resDesc.VOleStr);
         Log->Write((boost::format("CashIncassation() ErrorMsg: %1%") % ErrorMsg.c_str()).str().c_str());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

bool CShtrihPrinter::Wait()
{
    bool result = false;
    try
    {
        int TimeOut = 30*1000;
        clock_t BeginTime = clock();
        while(WaitForPrinting() == false)
        {
            clock_t EndTime = clock();
            long delta = (EndTime - BeginTime)/CLK_TCK * 1000;
            if (delta > TimeOut)
            {
                result = false;
                break;
            }

            for(int i=0; i <= 100; i++)
            {
                Sleep(10);
                Application->ProcessMessages();
            }
        }
        return result;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return result;
    }
}

