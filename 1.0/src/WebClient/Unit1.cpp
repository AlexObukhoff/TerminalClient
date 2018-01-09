//---------------------------------------------------------------------------

#include <vcl.h>
#include <algorith.h>
#include <mmsystem.h>
#include <except.h>
#include <tlhelp32.h>

#include "TModem.h"
#include "Unit1.h"
#include <IdHTTP.hpp>
#include "EMailSender.h"
#include "Unit2.h"
#include "Cheque.h"
#include "common.h"
#include "SetupUnit.h"
#include "globals.h"
#include "TMoneyTransferPayment.h"
#include <map>
#include <sstream>
#include <fstream>
#include <locale>
#include "TLocationParser.h"
#include "JSONDocument.h"
#include "localize.h"
#include "DeviceState.h"
#include "CryptLib2.h"
#include "ExpressPayDevice.h"

#include <boost\format.hpp>
#include <boost\algorithm\string.hpp>
#include <boost\lexical_cast.hpp>
#include <boost\regex.hpp>

#pragma hdrstop

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "SHDocVw_OCX"
#pragma link "SevenZipVCL"
#pragma resource "*.dfm"
TForm1 *Form1;

const char* AInterfaceMain = "main.html";
const double cnThreadTimeOut = double(10)/24/60;
const double cnInterfaceErrorTimeOut = double(10)/24/60;
const double cnCfgCheckTimeOut = double(60)/24/60;
//const double cnStartGetCardsInfoThread = double(5)/24/60;
const int MaxFatalBillAcceptorErrors=5;
const bool cnFirstTime = true;
const int MaxEnterPassword=3;
bool paymentLocation = false;
std::string LastEntryURL = "";
CICommand tICommand;
bool firstAccepting = true; 
bool startup = false; 
bool isExplorerMayKill = true;

enum FinishPaymentCommands {cnFPCmdNone, cnFPCmdRetry, cnFPCmdCancel,cnFPCmdStoreCanceledPayment,cnFPCmdForcePrintCheck};

//---------------------------------------------------------------------------
_fastcall TForm1::TForm1(TComponent* Owner) : TForm(Owner) , cSendTokenMinute(60) , m_menuFileName(".\\config\\menu")
{
    try
    {
        CoInitializeEx(NULL, COINIT_MULTITHREADED);
        InitFinished=false;
        ValidatorID = 0;
        PrevTerminalState = 0;
        SendingSMSAllowed = false;
        LogWriteErrorFound = false;
        ModemThread = NULL;
        Payment = NULL;
//        Crypt = NULL;
        Validator = NULL;
        Printer = NULL;
        WatchDog = NULL;
        Keyboard = NULL;
        CardReader = NULL;
        FileMap = NULL;
        InfoFile = NULL;
        GetCardsInfoThread = NULL;
        Sound = NULL;

        // да-да-да
        // мы обожаем костыли сделанные из трухлявого пня
        ValidatorPrevStatus = DSE_UNKNOWN_CODE - 1;
        ValidatorPrevMode = DSE_UNKNOWN_CODE - 1;
        CoinAcceptorPrevStatus = DSE_UNKNOWN_CODE - 1;
        CoinAcceptorPrevMode = DSE_UNKNOWN_CODE - 1;
        PrinterPrevMode = DSE_UNKNOWN_CODE - 1;
        PrinterPrevStatus = DSE_UNKNOWN_CODE - 1;
        CardReaderPrevMode = DSE_UNKNOWN_CODE - 1;
        WatchDogPrevMode = DSE_UNKNOWN_CODE - 1;

        RegisterCommandDT.Val=0;
        NextCheckDBUpdateTime.Val=0;
        LogErrorDone = false;
        //InterfaceOK = false;
        ProgramStartDT=TDateTime::CurrentDateTime();
        ReadVersion();
        Log = new TLogClass("webclient");

        Localization.setLog(Log);
        //Log->SetLogError(&LogError);
        Log->Write("--------------------------------");
        //Log->WriteInLine("Version "+AnsiString(version));
        if (AlreadyRunning())
        {
            Log->Write("Another WebClient instance is already running, exiting...");
            delete Log;
            Log = NULL;
            Application->Terminate();
            return;
        }
/*	if (!CopyFile(".\\config\\config.xml",".\\config\\config.int", false))
		Log->Write("Error copying file config.xml!");

	if (!CopyFile(".\\config\\operators.xml",".\\config\\operators.int", false))
		Log->Write("Error copying file operators.xml!");*/

//  std::auto_ptr <JSONDocument> JSONDoc( new JSONDocument(Log) );
//  JSONDoc->OpenFile("c:\\temp\\48435.dat");

        CfgFileName = ".\\config\\config";
        OperCfgFileName = ".\\config\\operators";

        Cfg = new TWConfig(CfgFileName+".xml",OperCfgFileName+".xml",Log);
        if (!Cfg)
        {
            Log->Write("Config is not defined!");
        }

        Cfg->SetDefaultValues();
        if (!Cfg->isXMLParserOK())
        {
            Log->Write("MSXML parser error!");
        }

        Cfg->JSMaker->Tab="\t";
        Cfg->JSMaker->Clear();
      //  Cfg->JSMaker->AddString("var $ga_jcfg = {");
        Cfg->JSMaker->AddString("$ga_jcfg = {");
        Cfg->JSMaker->Level++;

        CheckMonSrvrConnectRQ = false;
        if (!Cfg->ProcessConfigFile(false,true,true))
        {
            Log->Write("Processing Config.xml error!");// не удалось загрузить config.xml
            if (Cfg->RestoreFile(CfgFileName))         // восстанавливаем config.xml из config.lastgood
                Cfg->ProcessConfigFile(false,true,true);
        }
        else// config.xml успешно открыт, выставляем флаг
        {
            if ((Cfg->GetStatServerHost().LowerCase()=="none")||(Cfg->GetStatServerHost().LowerCase()=="")||(Cfg->GetStatServerHost().LowerCase()=="external_sender"))
            {
              if (!CopyFile((CfgFileName+".xml").c_str(),(CfgFileName+".lastgood").c_str(),false))
                  Log->Write((boost::format("Can not copy file %1%.xml to %1%.lastgood !") % CfgFileName.c_str()).str().c_str());
              else
                  Log->Write((boost::format("File %1%.xml copied to %1%.lastgood.") % CfgFileName.c_str()).str().c_str());
            }
            else
            {
                CheckMonSrvrConnectRQ = true;
            }
        }
        if (!Cfg->readMenu(m_menuFileName+".xml"))
        {
            Log->Write("Processing menu.xml error!");
            if (Cfg->RestoreFile(m_menuFileName.c_str()))         // восстанавливаем config.xml из config.lastgood
                Cfg->readMenu(m_menuFileName+".xml");
        }
        else
        {
            if (!CopyFile((m_menuFileName+".xml").c_str(),(m_menuFileName+".lastgood").c_str(),false))
            {
                Log->Write((boost::format("Can not copy file %1%.xml to %1%.lastgood !") % m_menuFileName.c_str()).str().c_str());
            }
            else
            {
                Log->Write((boost::format("File %1%.xml copied to %1%.lastgood.") % m_menuFileName.c_str()).str().c_str());
            }
        }


        AnsiString JSContent = Cfg->JSMaker->Content;

        //Log->Write("Processing Operators.xml - before GetOperatorsInfo");
        if (!Cfg->GetOperatorsInfo())
        {                                                       // не удалось загрузить operators.xml
            if (Cfg->RestoreFile(OperCfgFileName))              // восстанавливаем operators.xml из operators.lastgood
            {
                Cfg->JSMaker->Content = JSContent.c_str();
                if (Cfg->GetOperatorsInfo())
                {
                    if (!CopyFile((OperCfgFileName+".xml").c_str(),(OperCfgFileName+".lastgood").c_str(),false))
                        Log->Write((boost::format("Can not copy file %1%.xml to %1%.lastgood!") % OperCfgFileName.c_str()).str().c_str());
                    if (!CopyFile((m_menuFileName+".xml").c_str(),(m_menuFileName+".lastgood").c_str(),false))
                        Log->Write((boost::format("Can not copy file %1%.xml to %1%.lastgood!") % m_menuFileName.c_str()).str().c_str());
                }
            }
        }
        else                                                    // operators.xml успешно открыт,
        {                                                       // сохраняем его как operators.lastgood
            if (!CopyFile((OperCfgFileName+".xml").c_str(),(OperCfgFileName+".lastgood").c_str(),false))
                  Log->Write((boost::format("Can not copy file %1%.xml to .%1%.lastgood!") % OperCfgFileName.c_str()).str().c_str());
        }

        //Log->Write("Processing Operators.xml - before GetMenuTree");
        Cfg->GetMenuTree(NULL);
        //Log->Write("Processing Operators.xml - after  GetMenuTree");

        Cfg->JSMaker->CloseChild(false);
        Cfg->JSMaker->AddString("//  - - - - - всегда в true, для динамического include файла\n$iface_config_js = true;");
        std::string str=Cfg->JSMaker->Content;
        StoreStringToFile(ChangeChars(AnsiString(Cfg->Dirs.InterfaceDir.c_str()),"/","\\")+"\\iface_config.js",Cfg->JSMaker->Content,Log);
        Cfg->JSMaker->Clear();

        if (!Cfg->isOperatorExists(999))
            Cfg->Dirs.DBNumCapacityUpdateAddress="";

//  double Sum = Cfg->GetComission(0, 120);

        if (!Cfg->SMSInfo.SendStartUpSMS)
            SendingSMSAllowed = true;

        AnsiString CommandFileName = (Cfg->Dirs.CommandsInbound+"\\-1.pkt").c_str();
        if ((Cfg->Dirs.DBNumCapacityUpdateAddress=="")&&(FileExists(CommandFileName)))
        {
            Log->Write((boost::format("Trying to delete %1% file...") % CommandFileName.c_str()).str().c_str());
            if (DeleteFile(CommandFileName))
                Log->Append("OK.");
            else
                Log->Append("Error!");
        }

        CommandFileName = (Cfg->Dirs.CommandsInbound+"\\-1.ok").c_str();
        if ((Cfg->Dirs.DBNumCapacityUpdateAddress=="")&&(FileExists(CommandFileName)))
        {
            Log->Write((boost::format("Trying to delete %1% file...") % CommandFileName.c_str()).str().c_str());
            if (DeleteFile(CommandFileName))
                Log->Append("OK.");
            else
                Log->Append("Error!");
        }

        InfoFile = new TXMLInfo(Cfg->Dirs.StatusFileName.c_str(), Log);
        Cfg->Terminal.ChequeCounter = ::GetInt(InfoFile->Read("Program","ChequeCounter"));

        RenameTempFiles(Cfg->Dirs.PaymentsOutbound.c_str());
        RenameTempFiles(Cfg->Dirs.StatOutbound.c_str());
        RenameTempFiles(Cfg->Dirs.EMailOutbound.c_str());

        if (!Cfg->Terminal.ShowCursor)
        {
            int res=ShowCursor(false);
            Log->Write((boost::format("ShowCursor: %1%") % res).str().c_str());
            while (res>=0)
                res=ShowCursor(false);
            Log->Append((boost::format("|%1%.") % res).str().c_str());
        }

        FileMap = new TFileMap(Log);

        if (FileMap != NULL)
        {
            FileMap->Create("$share$");
            FileMap->Clear();
            FileMap->PutWCTimeMark();
            FileMap->ChequeCounter = Cfg->Terminal.ChequeCounter;
            if (InfoFile)
            {
                if (InfoFile->Read("Program","BlockMode")=="1")
                    FileMap->SetWCState(cnTerminalForceBlock);
            }
        }

        DeleteOldStatPackets(Cfg->Dirs.StatOutbound.c_str());
        DeleteOldStatPackets(Cfg->Dirs.StatOutboundTemp.c_str());

        crypt::init(Log);
        for(size_t i = 0; i < Cfg->Keys.size(); i++)
            crypt::addKeys(Cfg->Keys[i].SecKeyPath, Cfg->Keys[i].PubKeyPath, Cfg->Keys[i].SecKeyPassword, Cfg->Keys[i].PubKeySerial);


        Cfg->FillCountryCodes(".\\config\\country_codes.txt");

        USB1_OVERCURRENT_MSG=RegisterWindowMessage("WM_WDT_USB1_OVERCURRENT");
        USB2_OVERCURRENT_MSG=RegisterWindowMessage("WM_WDT_USB2_OVERCURRENT");
        WDT_KEYUP_MSG=RegisterWindowMessage("WM_WDT_KEYUP");
        WDT_KEYDOWN_MSG=RegisterWindowMessage("WM_WDT_KEYDOWN");

        BillAcceptorFatalErrorsCount=0;
        BillAcceptorFatalErrorSent=false;
        FinishPaymentInitiated=false;

        //Log->Write("FPI=false");
        if (FileExists("\\WebUpdate\\update.exe"))// Стираем .exe апдейт, если он существует
        {
            if (DeleteFile("\\WebUpdate\\update.exe"))                                  // во избежание повторного запуска.
                Log->Write("\\WebUpdate\\update.exe deleted.");
            else
                Log->Write("Error deleting \\WebUpdate\\update.exe!");
        }

        if (DirectoryExists("\\WebUpdate\\update"))                                   // Стираем .7z апдейт, если он существует
        {
            if (DeleteDir("\\WebUpdate\\update"))                                       // во избежание повторного запуска.
                Log->Write("\\WebUpdate\\update directory deleted.");
            else
                Log->Write("Error deleting \\WebUpdate\\update directory!");
        }

        if (Cfg->Terminal.InterfaceSoundVolume!=-1)
        {
            if (!SetSoundVolume((DWORD)(0xFFFF*1.0*Cfg->Terminal.InterfaceSoundVolume/100.0)))
                Log->Write("Can't set sound volume!");
        }
        if (Cfg->CDebug.sound)
          Sound = new TPSound(AnsiString(Cfg->Terminal.InterfaceSoundVolume), Log);

        if (Cfg->Terminal.SetWebclientHighPriority)
        {
            Log->Write("Setting high priority...");
            HANDLE hProcess;

            hProcess = GetCurrentProcess();
            if (SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS))
                Log->Append("OK.");
            else
                Log->Append(ShowError("error.").c_str());
        }
        saveEmpty_iface_details();
        saveDataToFile("");
        if (InfoFile)
        {
            if (InfoFile->Read("MoneyTransfer","token") == "")
                InfoFile->Write("MoneyTransfer","token","0");
            if (InfoFile->Read("MoneyTransfer","DateUseToken") == "")
                InfoFile->Write("MoneyTransfer","DateUseToken",TDateTime::CurrentDateTime());
            long token = boost::lexical_cast<long>(InfoFile->Read("MoneyTransfer","token").c_str());
            if(FileMap)
                FileMap->moneyTransferToken = token;
        }
        //zh_sp
        PB = new TPaymentBook(Cfg, Log, FileMap, InfoFile);        //инициализируем записную книжку
        //zh_sp
        //читаем файл шаблона штрих-кода
        //Cfg->processBarCodeTemplateFile((Cfg->Dirs.WorkDir + "config\\templates\\IncassBarCode.xml").c_str());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        throw;
    }
}

//---------------------------------------------------------------------------

int TForm1::GetKeyRequestsCount(TStringList* FileNames)
{
int iRes=0;
try
  {
  TSearchRec sr;
  int iAttributes = 0;
  if (FindFirst((Cfg->Dirs.CommandsInbound+"\\*.gk").c_str(), iAttributes, sr) == 0)
    {
    do
      {
      iRes++;
      if (FileNames)
        FileNames->Add(sr.Name);
      Log->Write((boost::format("Found %1%") % sr.Name.c_str()).str().c_str());
      } while (FindNext(sr) == 0);
    FindClose(sr);
    }
  if (iRes>0)
    Log->Write((boost::format("Found %1% GetKey request(s)") % iRes).str().c_str());
    else
    Log->Write("No GetKey requests");
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return iRes;
}

//---------------------------------------------------------------------------
int TForm1::GetVolumeControlID()
{
  MIXERLINE mxl;
  mxl.cbStruct=sizeof(MIXERLINE);

  mxl.dwComponentType=MIXERLINE_COMPONENTTYPE_DST_SPEAKERS; //Master
  //для WaveOut - MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT, см. mmsystem.h

  if( ::mixerGetLineInfo(ghmx, &mxl, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE ) != MMSYSERR_NOERROR )
    return 34;
  MIXERCONTROL mxc;
  MIXERLINECONTROLS mxlc;
  mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
  mxlc.dwLineID = mxl.dwLineID;
  mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
  mxlc.cControls=1;
  mxlc.cbmxctrl=sizeof(MIXERCONTROL);
  mxlc.pamxctrl=&mxc;
  if(::mixerGetLineControls((HMIXEROBJ)ghmx, &mxlc, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR )
    return 34;
  return mxc.dwControlID;
}

// непосредственно установка громкости
bool TForm1::SetSoundVolume(DWORD dwVolume)
{
  MIXERCONTROLDETAILS mxcd;
  MIXERCONTROLDETAILS_UNSIGNED mxcd_u;
  mxcd.cbStruct = sizeof(mxcd);
  mxcd.dwControlID = GetVolumeControlID();
  mxcd.cChannels = 1;
  mxcd.cMultipleItems = 0;
  mxcd.cbDetails = 4;
  mxcd.paDetails = &mxcd_u;
  mmr=mixerGetControlDetails((HMIXEROBJ)ghmx, &mxcd,0L);
  if(MMSYSERR_NOERROR != mmr)
    return false;
  mxcd_u.dwValue=dwVolume;
  mmr=mixerSetControlDetails((HMIXEROBJ)ghmx,&mxcd,0L);
  if(MMSYSERR_NOERROR!=mmr)
    return false;
  return true;
}

//---------------------------------------------------------------------------

void TForm1::GoToMainMenu()
{
    try
    {
        //if(CppWebBrowser1->LocationURL.Pos(AInterfaceMain) == 0)
        Navigate(AInterfaceMain);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TForm1::GoToMessage(int ErrorCode, std::string msg)
{
    try
    {
        std::string errorAttr = (PB->entered ? "errornumber" : "error");
        std::auto_ptr <paymentParameters> PP ( new paymentParameters());
        
        if ((msg[0] != '&') && (msg != ""))
            msg = "&" + msg;
        if (PB->entered)
        {
            const std::string pb_regex = "(.*)((\\?|\\&)pb=[^\\&]+)(.*)";
            boost::regex expr_pb(pb_regex);
            if(boost::regex_match(msg, expr_pb))
            {
                boost::cmatch what_pb;
                boost::regex_search(msg, what_pb, expr_pb);
                boost::replace_all(msg, what_pb[2].str().c_str(), "");
            }
            std::string URL = (boost::format("%1%pstate=%2%&%3%%4%")
              % "pb.html?pb=error&"
              % PP->m_pstate
              % (ErrorCode ? (boost::format("%1%=%2%") % errorAttr % ErrorCode).str().c_str() : "")
              % ((msg != "") ? (boost::format("%1%%2%") % (PB->entered ? "" : "&msg=") % msg).str().c_str() : "")).str();

            boost::replace_all(URL, "&&", "&");
            Navigate(URL.c_str());
        }
        else
        {
            msg = "message.html?" + errorAttr + "=" + boost::lexical_cast<std::string>(ErrorCode) + msg;
            Navigate(msg.c_str());
        }
    }
    catch(...)
    {
      ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TForm1::GoToFullScreenMessage(AnsiString tParam, int ErrorCode, std::string msg)
{
try
{
std::string errorAttr = (PB->entered ? "errornumber" : "error");
std::string tParamStr = tParam.c_str();
boost::replace_all(tParamStr, "pb=checking&", "");

Navigate((boost::format("%1%%2%%3%%4%")
  % (PB->entered ? "pb.html?pb=error&" : "full-screen-message.html?")
  % tParamStr
  % (ErrorCode ? (boost::format("&%1%=%2%") % errorAttr % ErrorCode).str().c_str() : "")
  % ((msg != "") ? (boost::format("&msg=%1%") % msg).str().c_str() : "")).str().c_str());
}
catch(...)
{
  ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
}
}

//---------------------------------------------------------------------------
void TForm1::GoToPrinterError()
{
try
{
  if (FileMap->CheckDebugState(cnPrinterError))
  {
    Log->Append(" Printer Debug state!");
    /*
    if (PB->entered)
      Navigate("pb.html?pb=getpb");
    else
      GoToMainMenu();
    */
    return;
  }

  std::auto_ptr <paymentParameters> PP ( new paymentParameters());

  if (Payment)
    Navigate((boost::format("%1%DV=%2%&SUM=%3%&sum=%4%&pstate=%5%&%6%&%7%")
      % (PB->payment ? "pb.html?pb=error&errornumber=-8&" : "PrinterError.html?")
      % TDateTime::CurrentDateTime().DateString().c_str()
      % Payment->Sum
      % PP->m_s_sum
      % PP->m_pstate
      % Payment->AFieldsForInterface.c_str()
      % (Cfg->Payments.Rest ? (boost::format("&rest=%1%") % Cfg->Payments.Rest).str().c_str() : "")).str().c_str());
}
catch(...)
{
  ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
}
}

void TForm1::PreparePaymentNoPrinter()
{
  try
  {
    if (Payment)
    {
      Payment->PayProcessStarted = true;

      if(Validator)
          Validator->EnableBill();
      if(CoinAcceptor)
          CoinAcceptor->Enable();
      GoToPayment();
      PaymentTimeOutTimer->Enabled=true;  //on PreparePaymentNoPrinter
    }
    else
    {
      Log->Write("Error! PreparePaymentNoPrinter without payment!");
      GoToMainMenu();
    }
  }
  catch(...)
  {
    ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
}

std::string TForm1::getFullPathURL(std::string URL)
{
  boost::replace_all(URL, " ", "%20");

  std::string result = (Cfg) ?
    ("file:///" + Cfg->Dirs.InterfaceDir+"/" + Locale + URL) :
    ("file:///" + std::string(GetCurrentDir().c_str()) + "/interface/" + URL);
  return result;
}

std::string TForm1::makePaymentURLParams(int isButtonActive)
{
    try
    {
      std::string tstr = "";
      if ((Payment) && (!(Cfg->Payments.rState)))
        tstr = (boost::format("&rest=%1%") % Payment->XMLP->vNotes.size()).str();

      std::auto_ptr <paymentParameters> PP ( new paymentParameters());
      std::string result = (boost::format("sum=%1%&cms=%2%&mps=%3%&maxsum=%4%&pstate=%5%&%6%%7%&act=%8%&minshowsum=%9%&fixsum=%10%")
        % PP->m_s_sum
        % PP->m_s_cms
        % PP->m_s_minsum
        % PP->m_s_maxsum
        % PP->m_pstate
        % Payment->AFieldsForInterface.c_str()
        % tstr
        % isButtonActive
        % PP->m_s_minshowsum
        % PP->m_s_fixsum).str();

      return result;
    }
    catch(...)
    {
        return "";
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TForm1::GoToPayment(int isButtonActive)
{
    try
    {
      std::string URLParams = makePaymentURLParams(isButtonActive);
      if (!URLParams.empty())
        Navigate((boost::format("%1%%2%")
          % (PB->payment ? "pb.html?pb=payment&" : "Payment.html?")
          % URLParams).str().c_str());
      else
      {
        if (PB->entered)
          GoToMainMenu();
        else
          Navigate("pb.html?pb=getpb");
      }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TForm1::GoToPaymentComplete(bool t_processing, bool t_Summ, bool t_msg, char* t_cmd)
{
    try
    {
      std::auto_ptr <paymentParameters> PP ( new paymentParameters());

      std::string t_str_proc = t_processing ? "processing=1&" : "";
      std::string t_str_cmd = (std::string(t_cmd) != "") ? (boost::format("&cmd=%1%") % t_cmd).str().c_str() : "";
      std::string t_str_rest = Cfg->Payments.Rest ? ("&rest=" + FloatToStr(Cfg->Payments.Rest)).c_str() : "";
      std::string t_str_summ = "", t_str_msg = "";
      std::string t_str_phone = "";
      std::string t_str_fields = "";
      int tempRecepient = -1;

      if (Payment)
      {
        tempRecepient = Payment->Recepient;

        if (t_Summ)
          t_str_summ = (boost::format("Summ=%1%&") % Payment->Sum).str();
          //t_str_summ = strcat(t_processing ? "&" : "", ("Summ=" + FloatToStr(Payment->Sum) + "&").c_str());

        if (t_msg && (Payment->PostPaymentInfo != "" ))
          t_str_msg = (boost::format("&msg=%1%") % Payment->PostPaymentInfo.c_str()).str();
         //t_str_msg = strcat("&msg=", Payment->PostPaymentInfo.c_str());

        std::string tFields = Payment->AFieldsForInterface.c_str();
        const std::string phone_regex = "(.*)field100=([0-9]+)(.*)";
        boost::regex exprPhone(phone_regex);
        if((boost::regex_match(tFields, exprPhone)) &&
           (Cfg->Operator(Payment->Recepient).GroupId == 101))
        {
          boost::cmatch whatField;
          boost::regex_search(tFields, whatField, exprPhone);
          t_str_phone = (boost::format("&phone=%1%&AP=%2%")
            % whatField[2].str().c_str()
            % Cfg->Keys[Cfg->GetKeysNum(Cfg->Operator(Payment->Recepient).KeysId)].AP).str();
        }

        t_str_fields = Payment->FieldsFromInterface;
      }
      std::string page = PB->payment ?
        std::string("pb.html?pb=") + tICommand.GetIDescription(pbook::AllThanks) + "&" : std::string("PaymentComplete.html?");

      std::string URL_chank_1 = (boost::format("%1%%2%%3%sum=%4%&cms=%5%&mps=%6%&maxsum=%7%&recepient=%8%")
        % page
        % t_str_proc
        % t_str_summ
        % PP->m_s_sum
        % PP->m_s_cms
        % PP->m_s_minsum
        % PP->m_s_maxsum
        % tempRecepient).str();

      std::string URL_chank_2 = (boost::format("&pstate=%1%%2%%3%%4%%5%%6%")
        % PP->m_pstate
        % t_str_cmd
        % t_str_rest
        % t_str_msg
        % t_str_phone
        % t_str_fields).str();

      Navigate((URL_chank_1 + URL_chank_2).c_str());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TForm1::EnterErrorMode()
{
    try
    {
        Log->Write("Error mode entered");

        Navigate("terminal_error.html");

        if (FileMap)
            FileMap->SetWCState(cnTerminalErrorMode);

        if ((Payment)&&(!FinishPaymentInitiated)&&(Payment->PayProcessStarted))
        {
            FinishPayment(true);
            FinishPaymentInitiated = false;
        }

        if (FileMap)
            FileMap->WCIdle=true;

        Log->Append("..!");
    }
    catch(...)
    {
    ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TForm1::CheckTerminalState()
{
try
	{
  if (( FileMap->CheckWCState(cnTerminalErrorMode)) &&
      (!FileMap->CheckWCState(cnTerminalWriteError)) &&
      (!FileMap->CheckWCState(cnTerminalForceBlock)) &&
      (!FileMap->CheckWCState(cnTerminalInternalBlock))&&
      ((!FileMap->CheckWCState(cnCoinAcceptorError)) || (!FileMap->CheckWCState(cnValidatorError))) &&
      (!FileMap->CheckWCState(cnPrinterError)) &&
      (!FileMap->CheckWCState(cnCardReaderError)))
    QuitErrorMode();

  if ((!FileMap->CheckWCState(cnTerminalErrorMode)) &&
      ((FileMap->CheckWCState(cnTerminalWriteError)) ||
       (FileMap->CheckWCState(cnTerminalForceBlock)) ||
       (FileMap->CheckWCState(cnTerminalInternalBlock)) ||
       (FileMap->CheckWCState(cnValidatorError) && FileMap->CheckWCState(cnCoinAcceptorError)) ||
       (FileMap->CheckWCState(cnPrinterError)) ||
       (FileMap->CheckWCState(cnCardReaderError))))
    {
    if (FileMap->CheckWCState(cnTerminalWriteError))
      Log->Write("Write error!");

    if (FileMap->CheckWCState(cnTerminalForceBlock))
      Log->Write("Force block mode!");

    if (FileMap->CheckWCState(cnTerminalInternalBlock))
      Log->Write("Internal block mode!");

    if (FileMap->CheckWCState(cnValidatorError))
      Log->Write("Validator error!");

    if (FileMap->CheckWCState(cnCoinAcceptorError))
      Log->Write("CoinAcceptor error!");

    if (FileMap->CheckWCState(cnPrinterError))
      Log->Write("Printer error!");

    if (FileMap->CheckWCState(cnCardReaderError))
      Log->Write("CardReader error!");

    EnterErrorMode();
    }

/*  if (((cnTerminalErrorMode & FileMap->WCState)==0)&&(((cnTerminalWriteError & FileMap->WCState)==cnTerminalWriteError)||((cnTerminalForceBlock & FileMap->WCState)==cnTerminalForceBlock)||((cnTerminalBillAcceptorFatalError & FileMap->WCState)==cnTerminalBillAcceptorFatalError)||((cnTerminalPrinterFatalError & FileMap->WCState)==cnTerminalPrinterFatalError)))

  if (Validator->DeviceState->OutStateCode!=DSE_OK)
    return;

  if ((!Cfg->Peripherals->Printer.FreeseOnError)&&(Printer.DeviceState->OutStateCode!=DSE_OK)&&(Printer.DeviceState->OutStateCode!=DSE_NEARENDPAPER))
    return;


	Log->Write("Trying to quit ErrorMode!");
  QuitErrorMode();*/
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TForm1::QuitErrorMode()
{
try
	{
	Log->Write("Error mode quitted");
	if (FileMap)
    FileMap->ClearWCState(cnTerminalErrorMode);

//		FileMap->WCState = FileMap->WCState ^ cnTerminalErrorMode;
	GoToMainMenu();
	Log->Append("..!");
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void __fastcall TForm1::CloseConn()
{
try
	{
	if (!ConnHandle) {                                          									// check if conn handle is not NULL...
		Log->Write("Conn.exe has not been started.");
		return;
		}
	if (!IsProcessRunning(ConnHandle)) {                                          // check if conn process is running...
		Log->Write("Conn.exe is not running.");
		return;
		}
	Log->Write("Sending WM_CLOSE command to Conn.exe...");
	HWND handle=FindWindow(NULL, AnsiString("ConnMainForm").c_str());             // getting conn main form handle...
	if (handle!=NULL)
		{
		PostMessage(handle, WM_CLOSE, 0, 0);                                        // posting WM_CLOSE handle to comm main form...
		int ticks = 0;
		while ((ticks<2000)&&(handle!=NULL)) {                                      // waiting up to 20 seconds for the conn to close...
			if (FileMap)
				FileMap->PutWCTimeMark();
			handle=FindWindow(NULL, AnsiString("ConnMainForm").c_str());              // getting conn main form handle...
			ticks++;
			Sleep(10);
			}
		}
	if (!IsProcessRunning(ConnHandle))
		{																	                                          // check if conn process is still running...
		Log->Append("Done.");
		return;
		}
	Log->Append("Didn't work.");
	TerminateConn();
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

bool __fastcall TForm1::TerminateConn()
{
try
	{
	Log->Write("Trying to terminate conn.exe...");
 	if (!ConnHandle) {                                          									// check if conn handle is not NULL...
		Log->Write("Conn.exe has not been started.");
		return true;
		}
	if (!IsProcessRunning(ConnHandle)) {                                          // check if conn process is running...
		Log->Write("Conn.exe is not running.");
		return true;
		}
	if (!TerminateProcess(ConnHandle, NO_ERROR))                                  // terminating conn...
		Log->Append(ShowError("error.").c_str());
		else {
		Log->Append("OK.");
		CloseHandle(ConnHandle);
		ConnHandle = NULL;
		return true;
		}
	return false;
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
    UNREFERENCED_PARAMETER(Sender);
    try
    {
        try
        {
            try
            {
                if (Log)
                    Log->Write("FormClose started.");

                isExplorerMayKill = false;
                RunExplorer();

                InitFinished=true;
                if (!InitFinished)
                {
                    if (Log != NULL)
                        Log->Write("Init NOT finished - terminating close...");
                    Action=caNone;
                    return;
                }

                CheckTimeTimer->Enabled=false;
                CheckThreadsTimer->Enabled=false;
                CheckPrinterStateTimer->Enabled=false;
                StartAppTimer->Enabled=false;
                PaymentTimeOutTimer->Enabled=false;
                CCNETStateChange->Enabled=false;
                process->Enabled = false;

                //TerminateConn();
                try
                {
                    if (Payment)
                    {
                        if (Log)
                            Log->Write("Payment...");
                        try
                        {
                            if ((!FinishPaymentInitiated)&&(Payment->PayProcessStarted))
                            {
                                FinishPayment(true);
                                FinishPaymentInitiated = false;
                            }
                        }
                        catch(...)
                        {
                            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                        }
                        if (Log)
                            Log->Append("done.");
                    }

                    if (ModemThread != NULL)
                        ModemThread->Terminate();

                    if (GetCardsInfoThread != NULL)
                        GetCardsInfoThread->Terminate();

                    CloseConn();

                    if (ModemThread != NULL)
                    {
                        if (Log != NULL)
                            Log->Write("Modem thread...");
                        if (!ModemThread->Finished)
                        {
                            if (Log != NULL)
                                Log->Append("terminating...");
                            ULONG ExitCode=0;
                            TerminateThread((HANDLE)ModemThread->Handle,ExitCode);
                        }
                        delete ModemThread;
                        ModemThread=NULL;
                        if (Log != NULL)
                            Log->Append("done.");
                    }

                    if (GetCardsInfoThread != NULL)
                    {
                        if (Log != NULL)
                            Log->Write("GetCardsInfo thread...");
                        if (!GetCardsInfoThread->Finished)
                        {
                            if (Log != NULL)
                                Log->Append("terminating...");
                            ULONG ExitCode=0;
                            TerminateThread((HANDLE)GetCardsInfoThread->Handle,ExitCode);
                        }
                        delete GetCardsInfoThread;
                        GetCardsInfoThread=NULL;
                        if (Log != NULL)
                            Log->Append("done.");
                    }
/*
                    if (Validator)
                    {
                        if (Log != NULL)
                            Log->Write("Validator...");
                        delete Validator;
                        Validator = NULL;
                        if (Log != NULL)
                            Log->Append("done.");
                    }
*/
                    if (Printer)
                    {
                        if (Log != NULL)
                            Log->Write("Printer...");
                        delete Printer;
                        Printer = NULL;
                        if (Log != NULL)
                            Log->Append("done.");
                    }
                    if(scannerDevice)
                    {
                        scannerDevice->Stop();
                        delete scannerDevice;
                    }

                    if(cardReaderDevice)
                    {
                        cardReaderDevice->Stop();
                        delete cardReaderDevice;
                    }

                    if (WatchDog)
                    {
                        if (Log != NULL)
                            Log->Write("WatchDog...");
                        WatchDog->StopTimer();
                        delete WatchDog;
                        WatchDog = NULL;
                        if (Log != NULL)
                            Log->Append("done.");
                    }

                    //BillCounter=NULL;

/*				if (BillCounter0) {
					if (Log != NULL)
						Log->Write("BillCounter0...");
					delete BillCounter0;
					BillCounter0=NULL;
					if (Log != NULL)
						Log->Append("done.");
					}

				if (BillCounter1) {
					if (Log != NULL)
						Log->Write("BillCounter1...");
					delete BillCounter1;
					BillCounter1=NULL;
					if (Log != NULL)
						Log->Append("done.");
					}

				if (BillCounter2) {
					if (Log != NULL)
						Log->Write("BillCounter2...");
					delete BillCounter2;
					BillCounter2=NULL;
					if (Log != NULL)
						Log->Append("done.");
					}
*/
                    if (CardReader)
                    {
                        if (Log != NULL)
                            Log->Write("CardReader...");
                        delete CardReader;
                        CardReader=NULL;
                        if (Log != NULL)
                            Log->Append("done.");
                    }

                    if (Keyboard)
                    {
                        if (Log != NULL)
                            Log->Write("Keyboard...");
                        delete Keyboard;
                        Keyboard = NULL;
                        if (Log != NULL)
                            Log->Append("done.");
                    }

                    crypt::close();
                    
                    if (InfoFile)
                    {
                        if (Log != NULL)
                            Log->Write("InfoFile...");
                        delete InfoFile;
                        InfoFile = NULL;
                        if (Log != NULL)
                            Log->Append("done.");
                    }

                    SetupForm->Close();

                    if (Log != NULL)
                        Log->Write("SetupForm closed.");

                    if (Cfg->Terminal.SetWebclientHighPriority)
                    {
                        Log->Write("Setting back normal priority...");
                        HANDLE hProcess;

                        hProcess = GetCurrentProcess();
                        if (SetPriorityClass(hProcess, NORMAL_PRIORITY_CLASS))
                            Log->Append("OK.");
                        else
                            Log->Append(ShowError("error.").c_str());
                    }

                    if (Cfg)
                    {
                        if (Log != NULL)
                            Log->Write("Cfg...");
                        delete Cfg;
                        Cfg=NULL;
                        if (Log != NULL)
                            Log->Append("done.");
                    }

                    if (FileMap)
                    {
                        if (Log != NULL)
                            Log->Write("FileMap...");
                        delete FileMap;
                        FileMap=NULL;
                        if (Log != NULL)
                            Log->Append("done.");
                    }

                    if (Sound)
                    {
                        if (Log != NULL)
                            Log->Write("Sound...");
                        delete Sound;
                        Sound=NULL;
                        if (Log != NULL)
                            Log->Append("done.");
                    }


                    Form2->Close();

                    if (Log != NULL)
                        Log->Write("Form2 closed.");

                }
                catch(...)
                {
                    ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                }
            }
            catch(...)
            {
                ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            }
        }
        __finally
        {
            //Application->Terminate();
            //Close();
            //CoUninitialize();
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Application->Terminate();
    }
}
//---------------------------------------------------------------------------



void TForm1::CheckConnectionToMonitoringServer()
{
    try
    {
        Form2->Init(Log, 60000);
        Form2->ShowMessage(Localization["pleasewait"], 40);
//        Form2->ShowMessage("\n\nПожалуйста подождите...",40);
        Form2->DisableAll();
//        WaitForForm2Action();
        Application->ProcessMessages();
        AnsiString Result;
        if (Cfg->StatInfo.ProcessorType==cnCyberPlatServer)
        {
            std::auto_ptr <TCSPacketSender> TestPS ( new TCSPacketSender("", Cfg, Log, FileMap) );
            Result = TestPS->TestConnection();
        }
        else
        {
            std::auto_ptr <TSSPacketSender> TestPS ( new TSSPacketSender("", Cfg, Log, FileMap) );
            Result = TestPS->TestConnection();
        }
        Result = ChangeChars(ChangeChars(Result, "/", "$#@"), "$#@", "/ ");
        Form2->EnableOK(true);
        Form2->ShowMessage(Result.c_str());
        SetTopWindow(Form2->Handle);
        WaitForForm2Action();
        Form2->Close();
        SetTopWindow(Handle);
        Form2->EnableNumPad(true);
        Log->WriteInLine((boost::format("Проверка связи с сервером мониторинга: %1%") % Result.c_str()).str().c_str());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TForm1::CheckConnectionToCyberplat()
{
    try
    {
        std::auto_ptr <TPayment> TestPayment ( new TPayment("", Cfg, Log, FileMap, InfoFile) );
        Form2->DisableAll();
        Form2->Init(Log, 60000);
        Form2->ShowMessage(Localization["pleasewait"], 40);
//        Form2->ShowMessage("\n\nПожалуйста подождите...", 40);
        Application->ProcessMessages();
        AnsiString CheckResult = TestPayment->CheckConnection();
        CheckResult=ChangeChars(ChangeChars(CheckResult, "/", "$#@"), "$#@", "/ ");
        Form2->EnableOK(true);
        Form2->ShowMessage(CheckResult.c_str());
        SetTopWindow(Form2->Handle);
        WaitForForm2Action();
        Form2->Close();
        SetTopWindow(Handle);
        Log->Write((boost::format("Payment server connection check result: %1%") % CheckResult.c_str()).str().c_str());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

bool TForm1::isNavigateLogicMoneyTransfer(const TLocationParser& Location)
{
    if (FinishPaymentInitiated)
        return false;
    if ((Location.GetParameter("state") == "auth_check")&&(!Location.HasParameter("error"))) //инициализация платежа money transfer
        return true;
    else if (((Location.GetParameter("state") == "change_pswd" || Location.GetParameter("state") == "pwd_change_check"))&&(!Location.HasParameter("error")))
        return true;
    else if (Location.GetParameter("state") == "calculate" && Location.HasParameter("recepient_mt") && Location.HasParameter("enter_summ"))
        return true;
    return false;
}

void TForm1::logicMoneyTransfer(const AnsiString& LocationString,const TLocationParser& Location,const AnsiString& recepient,std::string* newLocation)
{
    if (FinishPaymentInitiated)
        return;
    *newLocation="";
    if ((Location.GetParameter("state") == "auth_check")&&(!Location.HasParameter("error"))) //инициализация платежа money transfer
    {
        Log->Write("auth_check");
        if (Payment)
        {
            delete Payment;
            Payment = NULL;
        }

        Payment = new TMoneyTransferPayment("", Cfg, Log, FileMap, InfoFile);
        if (Payment->PreInit(boost::lexical_cast<int>(recepient.c_str()),Location.GetParameter("login").c_str(),Location.GetParameter("password_md5").c_str()))
        {
            Log->Write("PreInit completed.");
            *newLocation=(boost::format("mt.html?recepient=%1%&state=auth_check&error=%2%&login=%3%") % Payment->Recepient % ((TMoneyTransferPayment*)Payment)->LastErrorCode % ((TMoneyTransferPayment*)Payment)->LoginInfo.CardNumber).str();
        }
        else
        {
            if(813==((TMoneyTransferPayment*)Payment)->LastErrorCode)
            {
                Log->Write("Need change password.");
                *newLocation=(boost::format("mt.html?recepient=%1%&state=pwd_change_first_time&login=%2%&old_password=%3%") % Payment->Recepient % ((TMoneyTransferPayment*)Payment)->LoginInfo.CardNumber  % Location.GetParameter("password_md5").c_str()).str();
            }
            else
            {
                Log->Write("PreInit failed.");
                if(Location.HasParameter("cnt_enter"))
                {
                    int cnt=boost::lexical_cast<int>(Location.GetParameter("cnt_enter"));
                    if(cnt >= (MaxEnterPassword-1))
                    {
                        *newLocation=(boost::format("mt.html?recepient=%1%&state=show_error_and_return&error=%2%&login=%3%&cnt_enter=0") % Payment->Recepient % ((TMoneyTransferPayment*)Payment)->LastErrorCode % ((TMoneyTransferPayment*)Payment)->LoginInfo.CardNumber).str();
                        ((TMoneyTransferPayment*)Payment)->clearLoginData();
                    }
                    else
                    {
                        cnt++;
                        *newLocation=(boost::format("mt.html?recepient=%1%&state=show_error_and_return&error=%2%&login=%3%&cnt_enter=%4%") % Payment->Recepient % ((TMoneyTransferPayment*)Payment)->LastErrorCode % ((TMoneyTransferPayment*)Payment)->LoginInfo.CardNumber % cnt).str();
                        ((TMoneyTransferPayment*)Payment)->clearLoginData();
                    }
                }
                else
                {
                    *newLocation=(boost::format("mt.html?recepient=%1%&state=show_error_and_return&error=%2%&login=%3%&cnt_enter=1") % Payment->Recepient % ((TMoneyTransferPayment*)Payment)->LastErrorCode % ((TMoneyTransferPayment*)Payment)->LoginInfo.CardNumber).str();
                    ((TMoneyTransferPayment*)Payment)->clearLoginData();
                }
            }
        }
    }
    else if (((Location.GetParameter("state") == "change_pswd" || Location.GetParameter("state") == "pwd_change_check"))&&(!Location.HasParameter("error")))
    {
        Log->Write("Trying change password");
        const char* oldPassw=NULL;
        std::string passwd = Location.GetParameter("old_passwd_md5");
        if("" != passwd)
            oldPassw = passwd.c_str();

        if (Payment->ChangePassword(Location.GetParameter("login").c_str(),oldPassw,Location.GetParameter("new_passwd_md5").c_str(),Location.GetParameter("new_passwd1_md5").c_str()))
        {
            Log->Write("Change password done");
            if (Payment->PreInit(GetInt(Location.GetParameter("recepient").c_str()),Location.GetParameter("login").c_str(),Location.GetParameter("new_passwd_md5").c_str()))
            {
                Log->Write("PreInit completed after ChangePassword.");
                *newLocation=(boost::format("mt.html?recepient=%1%&state=say_change_good&login=%2%") % Payment->Recepient % ((TMoneyTransferPayment*)Payment)->LoginInfo.CardNumber).str();
            }
            else
            {
                Log->Write("PreInit failed after ChangePassword.");
                *newLocation=(boost::format("mt.html?recepient=%1%&state=show_error&error=%2%&login=%3%") % Payment->Recepient % ((TMoneyTransferPayment*)Payment)->LastErrorCode % ((TMoneyTransferPayment*)Payment)->LoginInfo.CardNumber).str();
            }
        }
        else
        {
            Log->Write("Change password failed");
            *newLocation=(boost::format("mt.html?recepient=%1%&state=say_confirm_bad&error=%2%&login=%3%") % Payment->Recepient % ((TMoneyTransferPayment*)Payment)->LastErrorCode % ((TMoneyTransferPayment*)Payment)->LoginInfo.CardNumber).str();
            ((TMoneyTransferPayment*)Payment)->clearLoginData();
        }
    }
    else if (Location.GetParameter("state") == "calculate" && Location.HasParameter("recepient_mt") && Location.HasParameter("enter_summ"))
    {
        Log->Write("Trying to calculate commission");
        TMoneyTransferPayment* mtPayment=dynamic_cast<TMoneyTransferPayment*>(Payment);
        if (!mtPayment)
        {
            delete Payment;
            Payment = NULL;
            Payment = new TMoneyTransferPayment("", Cfg, Log, FileMap, InfoFile);
            mtPayment=dynamic_cast<TMoneyTransferPayment*>(Payment);
        }
        if(Location.GetParameter("enter_summ") != "")
        {
            mtPayment->CalculateCommission(LocationString.c_str(),boost::lexical_cast<int>(Location.GetParameter("enter_summ").c_str()));
            if(mtPayment->LastErrorCode!=0)
                *newLocation=(boost::format("mt.html?recepient=%1%&state=show_error&error=%2%&login=%3%") % Payment->Recepient % ((TMoneyTransferPayment*)Payment)->LastErrorCode % ((TMoneyTransferPayment*)Payment)->LoginInfo.CardNumber).str();
            else
                *newLocation=(boost::format("mt.html?recepient=%1%&state=calculate&login=%2%&recepient_mt=%3%&system_commission=%4%&amount=%5%&amount_all=%6%&rent_commission=%7%") % Payment->Recepient % mtPayment->LoginInfo.CardNumber % Location.GetParameter("recepient_mt") % mtPayment->getCalculateSystemComission() % mtPayment->getCalculateAmount() % mtPayment->getCalculateAmountAll() % mtPayment->getCalculateRentComission()).str();
        }
        else
        {
            Log->Write("Disable commission");
            *newLocation=(boost::format("mt.html?recepient=%1%&state=calculate&login=%2%&recepient_mt=%3%&commission=-1") % Payment->Recepient % mtPayment->LoginInfo.CardNumber % Location.GetParameter("recepient_mt")).str();
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::CppWebBrowser1DocumentComplete(TObject *Sender,
			LPDISPATCH pDisp, Variant *URL)
{
    UNREFERENCED_PARAMETER(Sender);
    UNREFERENCED_PARAMETER(pDisp);
    UNREFERENCED_PARAMETER(URL);

    AnsiString LocationString = "";
    try
    {
        if (FileMap)
            FileMap->PutLastEndNavigate();

        LocationString = CppWebBrowser1->LocationURL;
        TLocationParser Location(LocationString.c_str());
        AnsiString recepient=Location.GetParameter("recepient").c_str();

        /*****************************************************************************************************/
        std::auto_ptr <TLocationParser> CurrentLocation ( new TLocationParser(Location));
        std::string LocaleTemp = GetLocale(CurrentLocation);

        if ((LocaleTemp != "") && (Locale != "locale/" + LocaleTemp + "/"))
        {
            Log->Write((boost::format("Changed to \"%1%\" locale.") % LocaleTemp).str().c_str());
            Locale = "locale/" + LocaleTemp + "/";
            Navigate(CurrentLocation->URL.c_str());
            
            std::string localeSoundDir = Cfg->Dirs.InterfaceDir + "/" + Locale + "sound";
            boost::replace_all(localeSoundDir, "/", "\\");
            if (Sound)
                Sound->CashSounds(localeSoundDir.c_str());
            return;
        }
        /*****************************************************************************************************/

        AnsiString ProcessorType="";
        if(recepient!="")
            ProcessorType=Cfg->Operator(boost::lexical_cast<int>(recepient.c_str())).ProcessorType.c_str();
        ProcessorType=ProcessorType.LowerCase();

        if((Location.PageName == "Payment.html") ||
           ((Location.PageName == "pb.html") && (Location.GetParameter("pb") == "payment")))
        {
            if (Validator && !Validator->isEnableBill())
                Validator->EnableBill();
            if(CoinAcceptor && !CoinAcceptor->isEnableBill())
                CoinAcceptor->Enable();
        }

        if (Location.PageName != "pb.html")
        {
            TLocationParser locationForLog(LocationString.c_str());
            if (recepient != "")
            {
                for(std::size_t i =0; i < Cfg->Operator(boost::lexical_cast<int>(recepient.c_str())).Fields.size(); i++)
                {
                    std::string type = Cfg->Operator(boost::lexical_cast<int>(recepient.c_str())).Fields[i].Type;
                    if("textpwd" == type)
                    {
                        std::string id = Cfg->Operator(boost::lexical_cast<int>(recepient.c_str())).Fields[i].Id;
                        if (locationForLog.Parameters.find("field" + id) != locationForLog.Parameters.end())
                        {
                            locationForLog.Parameters.erase("field" + id);
                        }
                    }
                }
            }
            if (locationForLog.URL.empty())
            {
              if (LocationString.IsEmpty())
                Log->Write("URL is empty !");
              else
                Log->Write(("Bad URL: " + LocationString).c_str());
            }
            else
              Log->Write((boost::format("Entered '%1%' page.") % locationForLog.URL).str().c_str());
        }
        else if (Location.Parameters["pb"] != "checkaccount")
        {
            Log->Write((boost::format("Entered '%1%' page.") % TruncateLocation(LocationString)).str().c_str());
        }
        else
        {
            Log->Write("Check account");
        }
            
        if (Location.PageName == "main.html")
        {
          PB->Exit();
          if ((!Validator) || ((Validator) && (Validator->ErrorMode != DSE_OK) && (Cfg->Payments.rState == eNo) && (Payment == NULL)))
          {
            FileMap->SetWCState(cnValidatorError);
            CheckTerminalState();
            return;
          }
        }

        if ((Location.PageName == "data-entry.html") ||
            ((Location.PageName == "pb.html") && (Location.GetParameter("pb") == "dataentry")))
          firstAccepting = true;

        if ((Location.PageName != "Payment.html") &&
            (!((Location.PageName == "pb.html") && (Location.GetParameter("pb") == "payment"))) &&
            (Location.PageName != "command.html") &&
            paymentLocation)
        {
          if (Validator)
          {
            try
            {
              Validator->DisableBill();
              Log->Write("Validator bill disabled.");
              PaymentTimeOutTimer->Enabled=false;
            }
            catch(...)
            {
              Log->Write("Validator disabling error!");
            }
          }
        }
        
        if ((Location.PageName == "data-entry.html") &&
            (!(Cfg->Payments.rState)) &&
            (Location.GetParameter("sum") == "") &&
            (Location.GetParameter("recepient") != "999"))
        {
          try
          {
            std::auto_ptr<TLocationParser> entryLocation(new TLocationParser(LastEntryURL.c_str()));
            std::auto_ptr <paymentParameters> PP ( new paymentParameters());

            entryLocation->SetParameter("recepient", "999");
            entryLocation->SetParameter("pstate", PP->m_pstate.c_str());
            Navigate(AnsiString(entryLocation->URL.c_str()));
          }
          catch(...)
          {
            Log->Write("Error on redefining operator!");
          }
        }

        if((Location.PageName == "Payment.html") ||
           ((Location.PageName == "pb.html") && (Location.GetParameter("pb") == "payment")))
          paymentLocation = true;
        else
        {
          paymentLocation = false;
          if(Location.PageName == "data-entry.html")
            LastEntryURL = getFullPathURL(Location.URL);
          else
          if((Location.PageName == "message.html") ||
             (Location.PageName == "full-screen-message.html"))
          {
            std::auto_ptr<TLocationParser> entryLocation(new TLocationParser(LastEntryURL.c_str()));
            entryLocation->Parameters = Location.Parameters;
            LastEntryURL = getFullPathURL(entryLocation->URL);
          }
        }

        if (FileMap)
            FileMap->WCIdle = ((Location.PageName == "main.html")||(Location.PageName == "terminal_error.html"));

        ////////////////////////////////////////////////////////////////////////
        if(!scannerDevice->IsWaitingData && Location.PageName == "data-entry.html")
        {
            if(checkRecipient(GetInt(recepient)))
                scannerDevice->StartWaitData();
        }
        ////////////////////////////////////////////////////////////////////////

        // TODO: cardReader code here
        if(Cfg->Dirs.InterfaceSkinName == "svyaznoy")
        {
            if(!cardReaderDevice->IsWaitingData && Location.PageName == "data-entry.html")
            {
                cardReaderDevice->StartWaitData();
            }
        }

        if(Location.PageName=="checking.html" && savedLocation!="")
        {
            Sleep(100);
            //AnsiString recepient=Location.Parameters["recepient"];
            TLocationParser tlc(savedLocation.c_str());
            std::string newLocation;
            logicMoneyTransfer(savedLocation.c_str(),tlc,recepient,&newLocation);
            if(newLocation!="")
            {
                Navigate(newLocation.c_str());
                return;
            }
        }
        if(Location.PageName == "Payment.html" && Location.HasParameter("action"))
        {
            if((Location.Parameters["action"] == "calculate_commission"))
            {
                TMoneyTransferPayment* tmPayment=dynamic_cast<TMoneyTransferPayment*>(Payment);
                if(tmPayment && tmPayment->Sum!=0)
                {
                    tmPayment->CalculateCommission(LocationString.c_str(),Payment->Sum);
                    AnsiString asSystemCommission = tmPayment->getCalculateSystemComission().c_str();
                    AnsiString asRentCommission = tmPayment->getCalculateRentComission().c_str();
                    AnsiString asAmount = tmPayment->getCalculateAmount().c_str();
                    Navigate("Payment.html?sum="+FloatToStr(int(Payment->Sum*100))+"&system_commission="+asSystemCommission+"&mps="+FloatToStr(int(Payment->GetPaymentMinSum()*100))+"&maxsum="+AnsiString(int(Payment->GetLimMax()*100))+"&"+Payment->AFieldsForInterface+"&rent_commission=" + asRentCommission + "&amount=" + asAmount + "&act=0");
                }
                else
                {
                    GoToPayment();
                }
                return;
            }
        }
        /*
        Log->Write((boost::format("\r\npagename = %1%\r\nLocation.Parameters[""end""] = %2%\r\nLocation.Parameters[""pb""]  = %3%")
          % Location.PageName
          % Location.Parameters["end"]
          % Location.Parameters["pb"]).str().c_str());
        */
        if (((Location.PageName == "Payment.html") ||
             (Location.PageName == "PaymentComplete.html") ||
             ((Location.PageName == "pb.html") && (Location.Parameters["pb"] == "payment"))) &&
            (Location.Parameters["end"] == "1"))
        {
            TMoneyTransferPayment* tmPayment=dynamic_cast<TMoneyTransferPayment*>(Payment);
            if (!FinishPaymentInitiated)//вызов процедуры завершения оплаты
            {
                Log->Write("Finishing payment...");
                //zh_sp
                if (Payment)
                {
                    //Log->Write((boost::format("fix = %1%") % (Cfg->Operator(Payment->Recepient).fix ? 1 : 0)).str().c_str());
                    if(Cfg->Operator(Payment->Recepient).fix)
                    {
                        std::auto_ptr <paymentParameters> PP ( new paymentParameters(false));
                        Cfg->Payments.Rest = PP->m_rest;
                        Cfg->Payments.rState = eYes;
                    }
                    else if(tmPayment)
                    {
                        if(tmPayment->XMLP->GetParamValue("ri_rent_commission") != "")
                            Cfg->Payments.Rest = boost::lexical_cast<double>(tmPayment->XMLP->GetParamValue("ri_rent_commission").c_str());
                    }
                    else if(dynamic_cast<TAviaPayment*>(Payment))
                    {
                        // Для авицентра комиссия всегда нулевая, поэтому достаточно вычесть из всех внесенных денег деньги к зачеслению
                        Log->Write((boost::format("Total pay: %1%, Send money: %2%") % Payment->XMLP->GetParamValue("AMOUNT_ALL").c_str() % Payment->XMLP->GetParamValue("AMOUNT").c_str()).str().c_str());
                        Cfg->Payments.Rest = my_round(boost::lexical_cast<double>(Payment->XMLP->GetParamValue("AMOUNT_ALL").c_str()) - boost::lexical_cast<double>(Payment->XMLP->GetParamValue("AMOUNT").c_str()), true);
                    }
                    else
                    {
                        Cfg->Payments.Rest = 0;
                    }

                    if(Cfg->Payments.Rest <= 0)
                        Cfg->Payments.Rest = 0;
                    else
                        Cfg->Payments.rState = eYes;

                    if(Cfg->Payments.Rest)
                        Log->Write((boost::format("Payment rest: %1%") % Cfg->Payments.Rest).str().c_str());
                }
                //zh_sp
                FinishPayment(false);
                FinishPaymentInitiated = false;
                if ((!Validator) || ((Validator) && (Validator->ErrorMode != DSE_OK)))
                  FileMap->SetWCState(cnValidatorError);
                return;
            }
            else
            {
                Log->Write("Error! Payment.html?end=1 entered with FPI==true!");
                if(tmPayment && tmPayment->Sum!=0)
                {
                    Log->Write("Bug still life...");
                    Cfg->Payments.Rest = 0;
                    Cfg->Payments.rState = eNo;
                    FinishPayment(false);
                    FinishPaymentInitiated = false;
                    return;
                }
                FinishPaymentInitiated = false;
            }
        }

        if (Location.PageName == "checking.html" && Location.Parameters["end"] == "1")
        {
            return;
        }
        if (Location.PageName == "checking.html")
        {
            
            if(PaymentInitThread)
            {
                PaymentInitThread->Terminate();
                delete PaymentInitThread;
            }
            PaymentInitThread = new TPaymentInitThread(LocationString);
            PaymentInitThread->execFunction = std::bind1st(std::mem_fun(&TForm1::Payment_Init), Form1);
            PaymentInitThread->Resume();

            return;
        }

        if ((Location.PageName == "PaymentComplete.html")&&(Location.Parameters["cmd"] == "retry")) //повторная попытка проведения платежа
        {
            FinishPayment(false, cnFPCmdRetry);
            return;
        }
        if(Location.PageName == "PaymentComplete.html" && Location.Parameters["cmd"] == "send")
        {
            FinishPayment(false, cnFPCmdForcePrintCheck);
            FinishPaymentInitiated = false;
            return;
        }
        if(Location.PageName == "PaymentComplete.html" && Location.Parameters["cmd"] == "notsend")
        {
            Payment->XMLP->SavedData="CANCEL";
            if (!Payment->XMLP->SaveToFile())
                Log->Write("if (!Payment->XMLP->SaveToFile())");
            FinishPayment(true, cnFPCmdStoreCanceledPayment);
            FinishPaymentInitiated = false;
            return;
        }

        if ((Location.PageName == "PaymentComplete.html")&&(Location.Parameters["cmd"] == "cancel")) //отмена проведения платежа
        {
            FinishPayment(false, cnFPCmdCancel);
            FinishPaymentInitiated = false;
            return;
        }

        if ((Location.PageName == "data-entry.html")&&(ProcessorType=="cyberplat_metro")&&(Location.Parameters["cmd"] == "")) //инициализация платежа metro
        {
            if(Payment)
            {
                delete Payment;
                Payment = NULL;
            }
            Payment = new TMetroPayment(Cfg, Log, FileMap, InfoFile, CardReader);
            if (Payment->PreInit(GetInt(recepient),LocationString))
            {
                Log->Write("PreInit completed.");
                Navigate(LocationString.SubString(LocationString.Pos("data-entry.html"),LocationString.Length())+"&cmd=showmenu");
            }
            else
            {
                Log->Write("PreInit failed.");
                delete Payment;
                Payment = NULL;
                GoToMainMenu();
            }
        }

        if (((Location.PageName == "message.html") || ((Location.PageName == "pb.html") && (Location.Parameters["pb"] == "error")))
          && (Location.Parameters["res"] == "1")) //начало внесения денег
        {
            PreparePaymentNoPrinter();   //включаем валидатор(ы) и идем на payment
            return;
        }
        if ((Location.PageName=="service.html")&&(Location.HasParameter("cmd")))
        {
            AnsiString Command = Location.GetParameter("cmd").c_str();
            if ((Command=="4")||(LocationString.Pos("service.html?4")))// проверка связи с CyberPlat
            {
                if (Log != NULL)
                    Log->Write("Service menu command: Check connection to the payment system.");
                CheckConnectionToCyberplat();
                return;
            }
        }
    }
    catch(...)
    {
        Log->Write(("Browser has failed after loading page: " + LocationString).c_str());
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TForm1::BillEvent(double Nominal)
{
    try
    {
        //------------------------------!!!!!!!!!!!!!!!!!!!!!!!!!-----------------------------------
        //int nominal = (int)(Nominal / Cfg->CurrencyInfo.ExchangeRate);
        BillCounter->IncCounter(Nominal / Cfg->CurrencyInfo.ExchangeRate);
        //------------------------------!!!!!!!!!!!!!!!!!!!!!!!!!-----------------------------------
//      if (FileMap)
//      FileMap->ValidatorState=DSE_OK;
        if (FileMap)
        {
            AnsiString Specification = Validator->DeviceName.c_str() + AnsiString(Validator->ID);
            /*
            if (FileMap->ValidatorState!=DSE_OK)
                SendNotification(cnETHWOK, Specification,DSE_OK,"");
            */
            FileMap->ValidatorOutState="OK";
            FileMap->ValidatorState=DSE_OK;
        }
        if (Payment)
        {
            Payment->AddNote(Validator->ID,Nominal);
            std::auto_ptr <paymentParameters> PP ( new paymentParameters(false));

            double MaxSum = PP->m_maxsum - PP->m_sum;
            //Log->Write("Maximum amount to collect in this payment: "+AnsiString(MaxSum));
            if((Cfg->Operator(Payment->Recepient).fix) && (PP->m_sum - PP->m_cms < PP->m_maxsum))
            {
              TIntVector nominals;
              Cfg->strToIntVector(Localization["nominals"], nominals, "locale nominals");
              int max_nom = nominals.back();
              MaxSum = ceil((MaxSum + PP->m_cms)/max_nom)*max_nom;
            }
            Validator->SetMaxCash(MaxSum);
            PaymentTimeOutTimer->Enabled=false;
            PaymentTimeOutTimer->Enabled=true;
            if ((Payment->Sum >0)&&(!FinishPaymentInitiated)) //внесена купюра
            {
                THalfPinPayment* halfPin=dynamic_cast<THalfPinPayment*>(Payment);
                std::string nav;
                if ((Payment->Sum >0)&&(!FinishPaymentInitiated)) //внесена купюра
                {
                    THalfPinPayment* halfPin=dynamic_cast<THalfPinPayment*>(Payment);
                    std::string nav;
                    if(halfPin)
                    {
                        int tempMax = Payment->GetLimMax();
                        if(Cfg->Operator(Payment->Recepient).fix)
                          nav = (boost::format("Payment.html?sum=%1%&cms=%2%&mps=%3%&maxsum=%4%&money_crediting=%5%&rent_commission=%6%&%7%rest=1")
                            % double(Payment->Sum * 100)
                            % double(Payment->GetComission() * 100)
                            % my_round((tempMax + Payment->GetComission(tempMax))*100)
                            % my_round((tempMax + Payment->GetComission(tempMax))*100)
                            % halfPin->XMLP->GetParamValue("AMOUNT").c_str()
                            % halfPin->getRentCommision().c_str()
                            % Payment->AFieldsForInterface.c_str()).str().c_str();
                        else
                          nav = (boost::format("Payment.html?sum=%1%&cms=%2%&mps=%3%&maxsum=%4%&money_crediting=%5%&rent_commission=%6%&%7%")
                            % double(Payment->Sum * 100)
                            % double(Payment->GetComission() * 100)
                            % double(Payment->GetPaymentMinSum() * 100)
                            % double(tempMax * 100)
                            % halfPin->XMLP->GetParamValue("AMOUNT").c_str()
                            % halfPin->getRentCommision().c_str()
                            % Payment->AFieldsForInterface.c_str()).str().c_str();
                        Navigate(nav.c_str());
                    }
                    else
                      GoToPayment();
                    //Navigate(nav.c_str());
                }
            }
        }
        else
        {
            Log->Write((boost::format("Error! Bill %1% stacked without payment initialized!") % Nominal).str().c_str());
            Validator->DisableBill();
            Log->Write("Validator bill disabled.");
            PaymentTimeOutTimer->Enabled=false;
            if(CoinAcceptor)
                CoinAcceptor->Disable();
            GoToMainMenu();
        }
        AnsiString Bills = ","+ChangeChars(Cfg->Peripherals.Validator.ReportBillCount.c_str()," ","")+",";
        AnsiString CurrentBill = ","+AnsiString(BillCounter->TotalBill())+",";
        if ((Cfg->Peripherals.Validator.ReportBillCount!="")&&(Bills.Pos(CurrentBill)!=0))
        {
            AnsiString ChequeHead;
            for (std::size_t i = 0;i<Cfg->ChequeCaption.size();i++)
                ChequeHead+=(Cfg->ChequeCaption[i]+"\n").c_str();
//      for (int i=0;i<Cfg->CheckCaption->Count;i++)
//        {
//        ChequeHead+=Cfg->CheckCaption->Strings[i]+"\n";
//        }
            SendEMail(cnETValFull, ("Terminal #"+Cfg->Terminal.Number+" stacker nearly full: ").c_str()+AnsiString(BillCounter->TotalBill())+" bills counted.", ("Терминал #"+Cfg->Terminal.Number+" - стекер содержит ").c_str()+AnsiString(BillCounter->TotalBill())+" купюр, требуется инкассация.\n"+ChequeHead);
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//-------------------------------------------------------------------------------

void TForm1::DeviceStateChanged()
{
	try
	{
		PacketFileName = "";
		//Log->Write("DeviceStateChanged started.");
		AnsiString Specification;
		//обрабатываем смену состояния купюроприёмника
		std::auto_ptr <TDeviceState> DeviceState (new TDeviceState(NULL));

// Scanner
        if(scannerDevice)
        {
            while(scannerDevice->GetEvent(DeviceState.get()))
            {
                std::auto_ptr<TLocationParser> location (new TLocationParser(AnsiString(CppWebBrowser1->LocationURL).c_str()));
                unsigned int operatorId = GetInt(location->GetParameter("recepient").c_str());
                Navigate((boost::format("%1%?recepient=%2%%3%")
                    % location->PageName
                    % operatorId
                    % parseScannerData(operatorId, DeviceState->scannerDataValue, DeviceState->scannerDataType)).str().c_str());
            }
        }

// Validator
        if (Validator)
        {
            BillCounter = MoneyCounters.find(Validator->ID)->second;
            Specification = Validator->DeviceName.c_str() + AnsiString(Validator->ID);

            //validator events
            while(Validator->GetEvent(DeviceState.get()))
            {
                if (FileMap)
                    FileMap->PutWCTimeMark();

                if ((Payment) && (Cfg->Peripherals.Validator.Type != "none"))
                {
                    if ((DeviceState->Processing) && (firstAccepting))
                    {
                        firstAccepting = false;
                        GoToPayment(-1);
                    }

                    if (Validator->DeviceState->Enabling)
                    {
                        if(DeviceState->Processing)
                        {
                            Validator->DeviceState->Processing = false;
                            GoToPayment();
                        }
                        firstAccepting = true;
                    }
                }

                if (DeviceState->Idle)
                {
                    FileMap->ClearWCState(cnValidatorError);
                    Validator->ErrorMode = DSE_OK;
                }


                if (DeviceState->Stacked && DSE_STACKERFULL != DeviceState->OutStateCode && DSE_HARDWARE_ERROR != DeviceState->OutStateCode)
                {
                    DeviceState->Stacked = false;
                    if(DeviceState->Nominal)
                        BillEvent(DeviceState->Nominal);
                    Validator->ErrorMode = DSE_OK;
                }
                else
                {
                    bool manualResetTimer = false;
                    // отключаем timeout платежа
                    if(PaymentTimeOutTimer->Enabled)
                    {
                        manualResetTimer = true;
                        PaymentTimeOutTimer->Enabled = false;
                    }

                    Log->Write((boost::format("Validator state changed: ID: %1%; OutStates: {%2%; %3%; %4%} #%5%: %6%")
                      % DeviceState->ID
                      % DeviceState->OutStateCode
                      % DeviceState->OldOutStateCode
                      % ValidatorPrevStatus
                      % DeviceState->StateCode
                      % DeviceState->StateDescription).str().c_str());
                    switch (DeviceState->OutStateCode)
                    {
                        case DSE_OK:
                            Validator->ErrorMode = DSE_OK;
                            // Если предыдущее состояние было не DSE_BILLREJECT или если предыдущее состояние было DSE_BILLREJECT, но до этого состояние было не DSE_OK, то отправляем сообщение на сервер мониторинга.
                            if ((DeviceState->OldOutStateCode != DSE_BILLREJECT) || (DeviceState->OldOutStateCode == DSE_BILLREJECT && ValidatorPrevStatus != DSE_OK))
                            {
                                if (FileMap)
                                {
                                    FileMap->ClearWCState(cnValidatorError);
                                    FileMap->ValidatorOutState = (boost::format("#%1% %2%") % DeviceState->OutStateCode % DeviceState->OutStateDescription(DeviceState->OutStateCode)).str().c_str();
                                    FileMap->ValidatorState = DeviceState->OutStateCode;
                                }
                                if (Validator->ErrorMode != ValidatorPrevMode)
                                {
                                    SendNotification(cnETHWOK, Specification, (BYTE)DeviceState->OutStateCode,"OK");
                                }
                            }
                            break;
                        //-------------------------------------------------------------------------------------------------
                        case DSE_STACKEROPEN:

                            if (FileMap)
                            {
                                FileMap->ValidatorOutState = (boost::format("#%1% %2%") % DeviceState->OutStateCode % DeviceState->OutStateDescription(DeviceState->OutStateCode)).str().c_str();
                                FileMap->ValidatorState=DeviceState->OutStateCode;
                            }
                            if (Validator->ErrorMode != ValidatorPrevMode)
                            {
                                SendNotification(cnETHWErr, Specification, (BYTE)DeviceState->OutStateCode, DeviceState->OutStateDescription(DeviceState->OutStateCode).c_str());
                            }
                            if ((DeviceState->OldOutStateCode == DSE_UNKNOWN) || (!startup))
                            {
                                Log->Write("Warning: System started up with stacker already open!");
                                if (FileMap)
                                {
                                    FileMap->SetWCState(cnValidatorError);
                                    FileMap->ValidatorOutState=(boost::format("#%1% %2%") % DeviceState->OutStateCode % DeviceState->OutStateDescription(DeviceState->OutStateCode)).str().c_str();
                                    FileMap->ValidatorState=DeviceState->OutStateCode;
                                }
                                if (Validator->ErrorMode != ValidatorPrevMode)
                                {
                                    SendNotification(cnETHWErr, Specification,(BYTE)DeviceState->OutStateCode,DeviceState->OutStateDescription(DeviceState->OutStateCode).c_str());
                                    SendSMSNotification();
                                }
                            }
                            else
                            {
                                switch (Cfg->Peripherals.Validator.StackerOpenCommand)
                                {
                                    case cnSOIncassation:
                                        Validator->ErrorMode = DSE_OK;
                                        if (FileMap)
                                        {
                                            FileMap->SetWCState(cnValidatorError);
                                            FileMap->SetWCState(cnTerminalInternalBlock);
                                            CheckTerminalState();
                                        }
                                        Application->ProcessMessages();
                                        PerformIncassation();
                                        if (FileMap)
                                        {
                                            FileMap->ClearWCState(cnTerminalInternalBlock);
                                            CheckTerminalState();
                                        }
                                    break;
                                    case cnSOServiceMenu:
                                        Validator->ErrorMode = DSE_OK;
                                        EnterServiceMenu();
                                    break;
                                    default:
                                        Validator->ErrorMode = DSE_HARDWARE_ERROR;
                                    break;
                                }
                            }
                        break;
                        //-------------------------------------------------------------------------------------------------
                        case DSE_STACKERFULL:
                        case DSE_BILLJAM:
                        case DSE_NOTMOUNT:
                        case DSE_HARDWARE_ERROR:
                            Log->Write("Validator error!");
                            if (FileMap)
                            {
                                if (DeviceState->OldOutStateCode == DeviceState->OutStateCode && FileMap->CheckWCState(cnTerminalErrorMode))
                                {
                                    Log->Write("Already in error mode");
                                    break;
                                }
                                FileMap->SetWCState(cnValidatorError);
                                FileMap->ValidatorOutState = (boost::format("#%1% %2%") % DeviceState->OutStateCode % DeviceState->OutStateDescription(DeviceState->OutStateCode)).str().c_str();
                                FileMap->ValidatorState=DeviceState->OutStateCode;
                            }

                            if((Validator->ErrorMode != ValidatorPrevMode) &&  //если режим сменился
                                //и вдогонку за неисправностью не шлется HARDWARE_ERROR
                               (!((DeviceState->OutStateCode == DSE_HARDWARE_ERROR) && (ValidatorPrevStatus != DSE_HARDWARE_ERROR) && (ValidatorPrevMode != DSE_OK) && (Validator->ErrorMode != DSE_OK))))
                            {
                                SendNotification(cnETHWErr, Specification,(BYTE)DeviceState->OutStateCode,DeviceState->OutStateDescription(DeviceState->OutStateCode).c_str());
                                SendSMSNotification();
                            }
                        break;
                        //-------------------------------------------------------------------------------------------------
                        case DSE_BILLREJECT:
                            Log->Append((boost::format(" :%1%") % DeviceState->SubStateDescription).str().c_str());
                            BillAcceptorFatalError = true;
                            if ((BillAcceptorFatalErrorsCount>=MaxFatalBillAcceptorErrors)&&(!BillAcceptorFatalErrorSent))
                            {
                                Validator->ErrorMode = DSE_HARDWARE_ERROR;
                                if (FileMap)
                                {
                                    FileMap->ValidatorOutState = (boost::format("#%1% %2%") % DeviceState->OutStateCode % DeviceState->OutStateDescription(DeviceState->OutStateCode)).str().c_str();
                                    FileMap->ValidatorState=DeviceState->OutStateCode;
                                }
                                if (Validator->ErrorMode != ValidatorPrevMode)
                                {
                                    SendNotification(cnETHWErr, Specification,(BYTE)DeviceState->OutStateCode,DeviceState->OutStateDescription(DeviceState->OutStateCode).c_str());
                                    SendSMSNotification();
                                }
                                BillAcceptorFatalErrorSent = true;
                            }
                        break;
                    }
                    ValidatorPrevStatus = DeviceState->OutStateCode;
                    ValidatorPrevMode = Validator->ErrorMode;
                    
                    // включаем timeout платежа
                    if(manualResetTimer)
                        PaymentTimeOutTimer->Enabled = true;
                }
            }
            Validator->DeviceState->Done = true;
        }
// Coin acceptor
        if (CoinAcceptor)
        {
            BillCounter = MoneyCounters.find(CoinAcceptor->ID)->second;
            Specification = CoinAcceptor->DeviceName + AnsiString(CoinAcceptor->ID);
                    //coin acceptor events
            while(CoinAcceptor->GetEvent(DeviceState.get()))
            {
                if (FileMap)
                {
                    FileMap->PutWCTimeMark();
                }

                if (DeviceState->Idle)
                    FileMap->ClearWCState(cnCoinAcceptorError);

                if (DeviceState->Billing)
                {
                    DeviceState->Billing = false;
                    DeviceState->Nominal = ConvertCoinAcceptorValue((int)DeviceState->Nominal, 2);
                    BillEvent(DeviceState->Nominal);
                }
                else
                {
                    Log->Write((boost::format("CoinAcceptor state changed: ID: %1%; OutStates: {%2%; %3%; %4%} #%5%: %6%") % DeviceState->ID % DeviceState->OutStateCode % DeviceState->OldOutStateCode % CoinAcceptorPrevStatus % DeviceState->StateCode % DeviceState->StateDescription).str().c_str());
                    switch (DeviceState->OutStateCode)
                    {
                        case DSE_OK:
                            CoinAcceptor->ErrorMode = DSE_OK;
                            // Если предыдущее состояние было не DSE_BILLREJECT     или
                            // если предыдущее состояние было DSE_BILLREJECT, но до этого состояние было не DSE_OK, то отправляем сообщение на сервер мониторинга.
                            if ((DeviceState->OldOutStateCode!=DSE_BILLREJECT)||((DeviceState->OldOutStateCode==DSE_BILLREJECT)&&(ValidatorPrevStatus!=DSE_OK)))
                            {
                                //SendNotification(cnETHWOK, Specification, DeviceState->OutStateCode,"OK",devCoinAcceptor);
                            }
                            break;
                        //-------------------------------------------------------------------------------------------------
                        case DSE_STACKEROPEN:
                            CoinAcceptor->ErrorMode = DSE_OK;    //по умолчанию
                            if (DeviceState->OldOutStateCode != DSE_UNKNOWN)    //если не со старта
                            {
                                switch (Cfg->Peripherals.Validator.StackerOpenCommand)
                                {
                                    case cnSOIncassation:
                                        if (FileMap)
                                        {
                                            FileMap->SetWCState(cnValidatorError);
                                            FileMap->SetWCState(cnTerminalInternalBlock);
                                            CheckTerminalState();
                                        }
                                        Application->ProcessMessages();
                                        PerformIncassation();
                                        if (FileMap)
                                        {
                                            FileMap->ClearWCState(cnTerminalInternalBlock);
                                            CheckTerminalState();
                                        }
                                    break;
                                    case cnSOServiceMenu:
                                        EnterServiceMenu();
                                    break;
                                    default:
                                        CoinAcceptor->ErrorMode = DSE_HARDWARE_ERROR;
                                        if (CoinAcceptor->ErrorMode != CoinAcceptorPrevMode)
                                        {
                                            SendNotification(cnETHWErr, Specification,DeviceState->OutStateCode,DeviceState->OutStateDescription(DeviceState->OutStateCode).c_str());
                                            SendSMSNotification();
                                        }
                                        if (DeviceState->OldOutStateCode == DSE_UNKNOWN)
                                        {
                                            Log->Write("Warning: System started up with stacker already open!");
                                        }
                                    break;
                                }
                            }
                            break;
                        //-------------------------------------------------------------------------------------------------
                        case DSE_STACKERFULL:
                        case DSE_BILLJAM:
                        case DSE_NOTMOUNT:
                        case DSE_HARDWARE_ERROR:
                            CoinAcceptor->ErrorMode = DSE_HARDWARE_ERROR;
                            Log->Write("CoinAcceptor error!");
                    /*zh*/  if (FileMap)
                            {
                               FileMap->SetWCState(cnCoinAcceptorError);
                               FileMap->CoinAcceptorOutState = (boost::format("#%1% %2%") % DeviceState->OutStateCode % DeviceState->OutStateDescription(DeviceState->OutStateCode)).str().c_str();
                               FileMap->CoinAcceptorState=DeviceState->OutStateCode;
                            }
                            if (CoinAcceptor->ErrorMode != CoinAcceptorPrevMode)
                            {
                                SendNotification(cnETHWErr, Specification, DeviceState->OutStateCode, DeviceState->OutStateDescription(DeviceState->OutStateCode).c_str());
                                SendSMSNotification();
                            }
                        break;
                        //-------------------------------------------------------------------------------------------------
                        case DSE_BILLREJECT:
                            CoinAcceptor->ErrorMode = DSE_OK;
                            Log->Append((boost::format(" :%1%") % DeviceState->SubStateDescription).str().c_str());
                            BillAcceptorFatalError = true;
                            if ((BillAcceptorFatalErrorsCount>=MaxFatalBillAcceptorErrors)&&(!BillAcceptorFatalErrorSent))
                            {
                                CoinAcceptor->ErrorMode = DSE_HARDWARE_ERROR;
                                if (CoinAcceptor->ErrorMode != CoinAcceptorPrevMode)
                                {
                                    SendNotification(cnETHWErr, Specification,DeviceState->OutStateCode,DeviceState->OutStateDescription(DeviceState->OutStateCode).c_str());
                                    SendSMSNotification();
                                }
                                BillAcceptorFatalErrorSent = true;
                            }
                        break;
                    } //switch
                    CoinAcceptorPrevStatus = DeviceState->OldOutStateCode;
                    CoinAcceptorPrevMode = CoinAcceptor->ErrorMode;
                } //else
            } //while
            CoinAcceptor->DeviceState->Done = true;
        } //if CoinAcceptor

// Printer
        if (Printer)
        {
            while(Printer->GetEvent(DeviceState.get()))
            {
                if (FileMap)
                    FileMap->PutWCTimeMark();
                Specification = "Printer0";
                if (FileMap)
                {
                    FileMap->PrinterOutState = (boost::format("#%1% %2%")
                        % DeviceState->OutStateCode
                        % DeviceState->OutStateDescription(DeviceState->OutStateCode)).str().c_str();
                    FileMap->PrinterState=DeviceState->OutStateCode;
                }
                Log->Write((boost::format("Printer state changed: OutState: %1% %2% State: %3% %4%")
                    % DeviceState->OutStateCode
                    % DeviceState->OutStateDescription(DeviceState->OutStateCode)
                    % DeviceState->StateCode
                    % DeviceState->StateDescription).str());
                switch (DeviceState->OutStateCode)
                {
                    case DSE_OK:
                        Printer->ErrorMode = DSE_OK;
                        if (FileMap)
                            FileMap->ClearWCState(cnPrinterError);
                        CheckPrinterStateTimer->Enabled=false;
                        if (Printer->ErrorMode != PrinterPrevMode)
                        {
                            SendNotification(cnETHWOK, Specification,(BYTE)DeviceState->OutStateCode,"OK");
                        }
                        Log->Write("Printer OK.");
                    break;
                    //-------------------------------------------------------------------------------------------------
                    case DSE_NEARENDPAPER:
                        if (FileMap)
                            FileMap->ClearWCState(cnPrinterError);
                        CheckPrinterStateTimer->Enabled=false;
                        if (Printer->ErrorMode != PrinterPrevMode)
                        {
                            Printer->ErrorMode = DSE_NEARENDPAPER;
                            SendNotification(cnETHWErr, Specification,(BYTE)DeviceState->OutStateCode,DeviceState->OutStateDescription(DeviceState->OutStateCode).c_str());
                        }
                    break;
                    //-------------------------------------------------------------------------------------------------
                    case DSE_NOTMOUNT:
                    case DSE_NOTPAPER:
                    case DSE_PAPERJAM:
                    case DSE_HARDWARE_ERROR:
                        Printer->ErrorMode = DeviceState->OutStateCode;
                        if((Printer->ErrorMode != PrinterPrevMode) &&  //если режим сменился
                            //и вдогонку за неисправностью не шлется HARDWARE_ERROR
                           (!((DeviceState->OutStateCode == DSE_HARDWARE_ERROR) && (PrinterPrevStatus != DSE_HARDWARE_ERROR) && (PrinterPrevMode != DSE_OK) && (Printer->ErrorMode != DSE_OK))))
                        {
                            SendNotification(cnETHWErr, Specification,(BYTE)DeviceState->OutStateCode,DeviceState->OutStateDescription(DeviceState->OutStateCode).c_str());
                            SendSMSNotification();
                        }
                        if (Cfg->Peripherals.Printer.FreeseOnError)
                        {
                            Log->Write("Printer error!");
                            CheckPrinterStateTimer->Enabled=true;
                            if (FileMap)
                                FileMap->SetWCState(cnPrinterError);
                        }
                    break;
                }
                PrinterPrevMode = Printer->ErrorMode;
                PrinterPrevStatus = DeviceState->OutStateCode;
            }
            Printer->DeviceState->Done = true;
        }

//Keyboard
        if (Keyboard)
        {
            Keyboard->DeviceState->StateChange = false;
            Specification = "Keyboard0";
            while(Keyboard->GetEvent(DeviceState.get()))
            {
                Log->Write((boost::format("Keyboard state changed: OutState: %1% %2% State: %3%") % DeviceState->OutStateCode % DeviceState->OutStateDescription(DeviceState->OutStateCode) % DeviceState->StateCode).str().c_str());
                ::PostMessage(CppWebBrowser1->Handle, WM_KEYDOWN, DeviceState->StateCode, 0);
                ::PostMessage(CppWebBrowser1->Handle, WM_KEYUP, DeviceState->StateCode, 0);
            }
        }

//CardReader
        if (CardReader)
        {
            Specification = "CardReader";
            while(CardReader->GetEvent(DeviceState.get()))
            {
                if (FileMap)
                    FileMap->PutWCTimeMark();
                Log->Write((boost::format("CardReader state changed: OutState: %1% %2% State: %3% %4%") % DeviceState->OutStateCode % DeviceState->OutStateDescription(DeviceState->OutStateCode) % DeviceState->StateCode % DeviceState->StateDescription).str().c_str());
                switch (DeviceState->OutStateCode)
                {
                    case DSE_OK:
                        CardReader->ErrorMode = DSE_OK;
                        if (CardReader->ErrorMode != CardReaderPrevMode)
                        {
                            SendNotification(cnETHWOK, Specification,(BYTE)DeviceState->OutStateCode,"OK");
                        }
                        Log->Write("CardReader OK.");
                    break;
                    //-------------------------------------------------------------------------------------------------
                    case DSE_NOTMOUNT:
                        CardReader->ErrorMode = DSE_NOTMOUNT;
                        if (CardReader->ErrorMode != CardReaderPrevMode)
                        {
                            SendNotification(cnETHWErr, Specification,(BYTE)DeviceState->OutStateCode,DeviceState->OutStateDescription(DeviceState->OutStateCode).c_str());
                        }
                        if (Cfg->Peripherals.CardReader.FreeseOnError)
                        {
                            Log->Write("CardReader error!");
                            if (FileMap) FileMap->SetWCState(cnCardReaderError);
                        }
                    break;
                }
            }
            CardReader->DeviceState->Done = true;
            CardReaderPrevMode = CardReader->ErrorMode;
        }

//WatchDog
        if (WatchDog)
        {
            WatchDog->DeviceState->StateChange = false;
            Specification = "WatchDog";
            while(WatchDog->GetEvent(DeviceState.get()))
            {
                Log->Write((boost::format("WatchDog state changed: OutState: %1% %2%") % DeviceState->OutStateCode % DeviceState->OutStateDescription(DeviceState->OutStateCode)).str().c_str());
                switch (DeviceState->OutStateCode)
                {
                    case DSE_OK:
                        WatchDog->ErrorMode = DSE_OK;
                        if (WatchDog->ErrorMode != WatchDogPrevMode)
                        {
                            SendNotification(cnETHWOK, Specification,(BYTE)DeviceState->OutStateCode,"OK");
                        }
                        Log->Write("WatchDog OK.");
                    break;
                    //-------------------------------------------------------------------------------------------------
                    case DSE_NOTMOUNT:
                        WatchDog->ErrorMode = DSE_NOTMOUNT;
                        if (WatchDog->ErrorMode != WatchDogPrevMode)
                        {
                            SendNotification(cnETHWErr, Specification,(BYTE)DeviceState->OutStateCode,DeviceState->OutStateDescription(DeviceState->OutStateCode).c_str());
                        }
                    break;
                    //-------------------------------------------------------------------------------------------------
                    case DSE_SENSOR_1_ON:
                    case DSE_SENSOR_1_OFF:
                    case DSE_SENSOR_2_ON:
                    case DSE_SENSOR_2_OFF:
                    case DSE_SENSOR_3_ON:
                    case DSE_SENSOR_3_OFF:
                        //SendNotification(cnETProgMsg, Specification,(BYTE)DeviceState->OutStateCode,"Sensor state change", devWatchDog);
                        Log->Write("WatchDog sensor state changed!");
                    break;
                }
            }
            WatchDogPrevMode = WatchDog->ErrorMode;
        }
        CheckTerminalState();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void __fastcall TForm1::CCNETStateChangeTimer(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);
    try
    {
        CCNETStateChange->Enabled = false;
        DeviceStateChanged();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TForm1::CheckTimeProc()
{
    try
    {
        CheckTimeTimer->Enabled=true;
        if (FileMap)
            FileMap->PutWCTimeMark();
        int Command = FileMap->ReadCommand();
        switch (Command)
        {
            case cnCmdReboot:
                if (Payment==NULL)
                {
                    Log->Write("Reboot command received.");
                    CloseConn();
                    Reboot();
                }
                else
                {
                    if (RegisterCommandDT.Val==0)
                    {
                        RegisterCommandDT=TDateTime::CurrentDateTime();
                        Log->Write("Reboot command received. Payment in progress - waiting for completion...");
                    }
                    else
                    {
                        if (TDateTime::CurrentDateTime()>RegisterCommandDT+double(5)/24/60)
                        {
                            Log->Write("Command TimeOut.");
                            if (FileMap)
                            {
                                FileMap->SetWCState(cnTerminalInternalBlock);
                                CheckTerminalState();
                            }
                            CloseConn();
                            Reboot();
                        }
                    }
                }
                break;
            case cnCmdShutDown:
                if (Payment==NULL)
                {
                    Log->Write("ShutDown command received.");
                    CloseConn();
                    ShutDown();
                }
                else
                {
                    if (RegisterCommandDT.Val==0)
                    {
                        RegisterCommandDT=TDateTime::CurrentDateTime();
                        Log->Write("ShutDown command received. Payment in progress - waiting for completion...");
                    }
                    else
                    {
                        if (TDateTime::CurrentDateTime()>RegisterCommandDT+double(5)/24/60)
                        {
                            Log->Write("Command TimeOut.");
                            if (FileMap)
                            {
                                FileMap->SetWCState(cnTerminalInternalBlock);
                                CheckTerminalState();
                            }
                            //FinishPayment(true);
                            CloseConn();
                            ShutDown();
                        }
                    }
                }
                break;
            case cnCmdBlock:
                if (InfoFile)
                {
                    if (InfoFile->Read("Program","BlockMode")!="1")
                    {
                        Log->Write("Block command received.");
                        //FileMap->SetWCState(cnTerminalForceBlock);
                        InfoFile->Write("Program","BlockMode","1");
                        if (FileMap)
                        {
                            FileMap->WriteCommand(0);
                            FileMap->SetWCState(cnTerminalForceBlock);
                            CheckTerminalState();
                        }
                    }
                }
            break;
        case cnCmdUnblock:
            if (InfoFile)
            {
                if (InfoFile->Read("Program","BlockMode")=="1")
                {
                    Log->Write("Unblock command received.");
                    InfoFile->Write("Program","BlockMode","0");
                    if (FileMap)
                    {
                        FileMap->WriteCommand(0);
                        FileMap->ClearWCState(cnTerminalForceBlock);
                        CheckTerminalState();
                    }
                }
            }
            break;
        }

        if ((Cfg->GetStatServerHost().LowerCase()!="none")&&(Cfg->GetStatServerHost().LowerCase()!="")&&(Cfg->GetStatServerHost().LowerCase()!="external_sender")&&(CheckMonSrvrConnectRQ))
        {
            if (FileMap->MonSrvrConnectOK)
            {
                Log->Write("Monitoring server connection OK.");
                if (!CopyFile((CfgFileName+".xml").c_str(),(CfgFileName+".lastgood").c_str(),false))
                    Log->Write((boost::format("Can not copy file %1%.xml to %1%.lastgood!") % CfgFileName.c_str()).str().c_str());
                CheckMonSrvrConnectRQ = false;
            }
            else
            {
                if ((FileExists(CfgFileName+".lastgood"))&&(TDateTime::CurrentDateTime()>ProgramStartDT+cnCfgCheckTimeOut))
                {
                    bool SafeRestore = false;
                    try
                    {
                        std::auto_ptr <TWConfig> SavedCfg ( new TWConfig(CfgFileName+".lastgood","",Log) );
                        //SavedCfg->SetDefaultValues();
                        if (SavedCfg->ProcessConfigFile(false,false,false))
                        {
                            if ((SavedCfg->GetStatServerHost().LowerCase()=="none")||(SavedCfg->GetStatServerHost().LowerCase()=="")||(SavedCfg->GetStatServerHost().LowerCase()=="external_sender"))
                                Log->Write("Restore error! Monitoring server is not defined in config.lastgood!");
                            else
                                SafeRestore = true;
                        }
                        else
                        {
                            Log->Write("Processing Config.xml error!");                                 // не удалось загрузить config.lastgood
                        }
                    }
                    catch(...)
                    {
                        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                        Log->Write("Exception occured while checking config.lastgood");
                    }
                    if (SafeRestore)
                    {
                        Log->Write("Failed to connect to the monitoring server, restoring old config file...");
                        if (Cfg->RestoreFile(CfgFileName))
                        {
                            if (Payment)
                            {
                                Log->Write("Waiting for the payment to complete init...");
                                int TimeOut = 3000;
                                while ((Payment)&&(TimeOut))
                                {
                                    TimeOut--;
                                    Sleep(100);
                                    Application->ProcessMessages();
                                }
                                Log->Append("Done.");
                            }
                            FileMap->WCRestartReq = true;
                            Log->Write("Restart request sent to CONN...");
                        }
                    }
                    else
                    {
                        Log->Write("Config file restoring is unsafe - aborting...");
                        FileMap->WCRestartReq = true;
                    }
                    CheckMonSrvrConnectRQ = false;
                }
            }
        }

        if ((TDateTime::CurrentDateTime() > FileMap->LastStartNavigate+double(20)/24/60)&&((cnTerminalErrorMode & FileMap->WCState)!=cnTerminalErrorMode))
        //if ((TDateTime::CurrentDateTime() > FileMap->LastStartNavigate+double(20)/24/1)&&((cnTerminalErrorMode & FileMap->WCState)!=cnTerminalErrorMode))
        {
            Log->Write("Last navigate");
            GoToMainMenu();
        }

        if ((Cfg->Terminal.DetectWriteErrors)&&((InfoFile->FileSystemError)||(FileMap->WriteErrorFound)))
        {
            Log->Write("File write error detected, stopping terminal...");
            if (FileMap)
            {
                FileMap->SetWCState(cnTerminalWriteError);
                CheckTerminalState();
            }

            AnsiString SMSMesssage = SendSMSNotification("Oshibka zapisi na disk!", true);
            if ((SMSMesssage!= "")&&(ModemThread!=NULL))
            {
                Log->Write("Trying to send SMS...");
                ModemThread->Message = SMSMesssage;
            }
        }

        if (Validator)
        {
            BillCounter = MoneyCounters.find(Validator->ID)->second;
            if (FileMap)
                FileMap->BillsCount = BillCounter->TotalBill();
            if (FileMap)
                FileMap->BillsSum = BillCounter->TotalMoney();
        }
        if (FileMap)
        {
            if (FileMap->LastPaymentReceived.Val==0)
                FileMap->LastPaymentReceived = InfoFile->ReadDateTime("Program","LastPaymentReceived");
            if (InfoFile->ReadDateTime("Program","LastPaymentReceived")!=FileMap->LastPaymentReceived)
                InfoFile->Write("Program","LastPaymentReceived",AnsiString(FileMap->LastPaymentReceived));

            if (FileMap->LastPaymentProcessed.Val==0)
                FileMap->LastPaymentProcessed = InfoFile->ReadDateTime("Program","LastPaymentProcessed");
            if (InfoFile->ReadDateTime("Program","LastPaymentProcessed")!=FileMap->LastPaymentProcessed)
                InfoFile->Write("Program","LastPaymentProcessed",AnsiString(FileMap->LastPaymentProcessed));
        }
        if(-1 != Cfg->recepientMT)
        {
            if(TDateTime::CurrentDateTime() > (InfoFile->ReadDateTime("MoneyTransfer","DateUseToken") + double(cSendTokenMinute)/24/60) && Localization.getLocale() != "")
            {
                TMoneyTransferPayment Payment("", Cfg, Log, FileMap, InfoFile);
                Payment.setRecepientID(Cfg->recepientMT);
                Payment.GetErrorsList();
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::CheckTimeTimerTimer(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);
    CheckTimeProc();
}
//---------------------------------------------------------------------------


void __fastcall TForm1::ApplicationEvents1Message(tagMSG &Msg,
			bool &Handled)
{
try
	{
	if (Msg.message==USB1_OVERCURRENT_MSG) {
			Log->Write("USB Extender message: USB1 OverCurrent.");
			Handled = true;
			return;
			}
	if (Msg.message==USB2_OVERCURRENT_MSG) {
			Log->Write("USB Extender message: USB2 OverCurrent.");
			Handled = true;
			return;
			}
	if (Msg.message==WDT_KEYUP_MSG) {
			Log->Write((boost::format("USB Extender message: Key %1% pressed.") % Msg.wParam).str().c_str());
			Handled = true;
			if (Msg.wParam == 1)
			{
			EnterServiceMenu();
//        Stat1 = 0;
			}
			return;
			}
	if (Msg.message==WDT_KEYDOWN_MSG) {
      Log->Write((boost::format("USB Extender message: Key %1% released.") % Msg.wParam).str().c_str());
			Handled = true;
			return;
			}

    if (Keyboard)
      {
      if(Keyboard->ParentWindow==Handle)
        {
        if ((Msg.message==WM_KEYDOWN)&&(Msg.wParam == VK_BACK)) {
          Handled = true;
          ::PostMessage(CppWebBrowser1->Handle, WM_KEYDOWN, VK_DELETE, 0);
          return;
          }

        if ((Msg.message==WM_KEYUP)&&(Msg.wParam == VK_BACK)) {
          Handled = true;
          ::PostMessage(CppWebBrowser1->Handle, WM_KEYUP, VK_DELETE, 0);
          return;
          }
        }
      }


/*	if ((Msg.message==WM_KEYDOWN)&&(Msg.wParam == VK_F8)&&(Cfg->Peripherals.Validator.Type.LowerCase() == "none")) {
		Handled = true;
		BillEvent(1);
		return;
		}*/
  /*if ((Msg.message==WM_KEYDOWN))
    {
    Handled = true;
    Log->Write("WM_KEYDOWN "+ AnsiString(Msg.wParam)+"|"+AnsiString((ULONG)Msg.lParam));
    }

  if ((Msg.message==WM_KEYUP))
    {
    Handled = true;
    Log->Write("WM_KEYUP   "+ AnsiString(Msg.wParam)+"|"+AnsiString((ULONG)Msg.lParam));
    }*/

	if ((Msg.message==WM_KEYDOWN)&&(Msg.wParam == VK_F9)&&(AnsiString(Cfg->Peripherals.Validator.Type.c_str()).LowerCase() == "none")) {
		Handled = true;
		BillEvent(1);
		return;
		}

	if ((Msg.message==WM_KEYDOWN)&&(Msg.wParam == VK_F11)&&(AnsiString(Cfg->Peripherals.Validator.Type.c_str()).LowerCase() == "none")) {
		Handled = true;
		BillEvent(10);
		return;
		}

	if ((Msg.message==WM_KEYDOWN)&&(Msg.wParam == VK_F12)&&(AnsiString(Cfg->Peripherals.Validator.Type.c_str()).LowerCase() == "none")) {
		Handled = true;
		BillEvent(100);
		return;
		}
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TForm1::SendNotification(EMailTypes MessageType, AnsiString TypeDescr,BYTE Code, AnsiString CodeDescr)
{
	try
	{
		if (Cfg->StatInfo.ProcessorType==cnCyberPlatServer)
		{
      std::auto_ptr <TCSPacketSender> PS ( new TCSPacketSender("", Cfg, Log, FileMap) );
      if (PacketFileName != "")
        PS->CreatePacket(PacketFileName);
      PS->StoreError(TDateTime::CurrentDateTime(), TypeDescr, Code, CodeDescr, 0, "");
		}
    else
		{
      std::auto_ptr <TSSPacketSender> PS ( new TSSPacketSender("", Cfg, Log, FileMap) );
      if (PacketFileName != "")
        PS->CreatePacket(PacketFileName);
      PS->StoreError(TDateTime::CurrentDateTime(), TypeDescr, Code, CodeDescr, 0, "");
    }

//    SendEMail("Terminal #"+Cfg->Terminal.Number+" message: "+TypeDescr+" Error.", "Error Date/Time: "+AnsiString(DT)+", Error Info: {"+TypeDescr+", {#"+AnsiString(Code)+", "+CodeDescr+"}, {#"+SubCode+", "+SubCodeDescr+"}. ");

    SendEMail(MessageType, ("Terminal #"+Cfg->Terminal.Number+" message: ").c_str()+TypeDescr+" Error.", "Error Date/Time: "+AnsiString(TDateTime::CurrentDateTime())+", Error Info: {"+TypeDescr+", {#"+AnsiString(Code)+", "+CodeDescr+"}}. ");

/*    if (Cfg->EMailInfo->GetRecipientAddress(MessageType)!="")
    {
      std:auto_ptr <TEMailSender> EMailSender (new TEMailSender("", Cfg, Log, FileMap));
      if (PacketFileName == "")
      {
        EMailSender->StoreMessage(Cfg->EMailInfo->GetRecipientAddress(MessageType), TDateTime::CurrentDateTime(),"Terminal #"+Cfg->Terminal.Number+" message: "+TypeDescr+" Error.", "Error Date/Time: "+AnsiString(TDateTime::CurrentDateTime())+", Error Info: {"+TypeDescr+", {#"+AnsiString(Code)+", "+CodeDescr+"}}. ");
      }
      else
      {
        EMailSender->StoreMessage(Cfg->EMailInfo->GetRecipientAddress(MessageType), TDateTime::CurrentDateTime(),"Terminal #"+Cfg->Terminal.Number+" message: "+TypeDescr+" Error.", "Error Date/Time: "+AnsiString(TDateTime::CurrentDateTime())+", Error Info: {"+TypeDescr+", {#"+AnsiString(Code)+", "+CodeDescr+"}}. ",PacketFileName);
      }
    }*/
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TForm1::SendEMail(EMailTypes MessageType, AnsiString Header,AnsiString Body)
{
	try
  {
    if (Cfg->EMailInfo.GetRecipientAddress(MessageType)!="")
    {
      std::auto_ptr <TEMailSender> EMailSender (new TEMailSender("", Cfg, Log, FileMap));
      if (PacketFileName == "")
      {
        EMailSender->StoreMessage(Cfg->EMailInfo.GetRecipientAddress(MessageType), TDateTime::CurrentDateTime(), Header, Body);
      }
      else
      {
        EMailSender->StoreMessage(Cfg->EMailInfo.GetRecipientAddress(MessageType), TDateTime::CurrentDateTime(), Header, Body, PacketFileName);
      }
    }
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

AnsiString TForm1::SendSMSNotification(AnsiString DMessage, bool bDoNoStore)
{
  AnsiString Message;
	try
	{
		if (!SendingSMSAllowed)
    {
  		Log->Write("SMS sending is not allowed!");
			return "";
    }
		if ((Cfg->SMSInfo.PhoneNumber == "")||(Cfg->Peripherals.Modem.Port==0))
    {
  		Log->Write("SMS off...");
			return "";
    }

		AnsiString Message1 = "Terminal: "+AnsiString(Cfg->Terminal.Number.c_str())+"\n";
		AnsiString Message2 = TransLit(Cfg->SMSInfo.Comment.c_str());
		AnsiString Message3;
		if (Cfg->SMSInfo.Comment!="")
			Message3 += "\n";
		if (DMessage!="")
			Message3 += DMessage+"\n";
		Message3 += "Vremya: "+TDateTime::CurrentDateTime().FormatString("hh:nn")+
			"\nKupyur: "+AnsiString(FileMap->BillsCount)+
			"\nSumma: "+AnsiString(FileMap->BillsSum)+
			"\nValidator: "+GetHWErrorDescription(FileMap->ValidatorState)+
			"\nPrinter: "+GetHWErrorDescription(FileMap->PrinterState);
		if (int(TDateTime::CurrentDateTime().Val)!=int(FileMap->LastPaymentReceived.Val))
			Message3 += "\nPrin.pl.: "+FileMap->LastPaymentReceived.FormatString("dd.mm hh:nn");
		else
			Message3 += "\nPrin.pl.: "+FileMap->LastPaymentReceived.FormatString("hh:nn");

		if (int(TDateTime::CurrentDateTime().Val)!=int(FileMap->LastPaymentProcessed.Val))
			Message3 += "\nProv.pl.: "+FileMap->LastPaymentProcessed.FormatString("dd.mm hh:nn");
	  else
			Message3 += "\nProv.pl.: "+FileMap->LastPaymentProcessed.FormatString("hh:nn");

		Message = Message1 + Message2.SubString(0, 160-Message1.Length()-Message3.Length()) + Message3;
		Log->Write((boost::format("Message length: %1%") % Message.Length()).str().c_str());
		if (!bDoNoStore)
		{
      std::auto_ptr <TSMSSender> SMSSender (new TSMSSender("", Cfg, Log, FileMap));
			//SMSSender = new TSMSSender("", Cfg, Log, FileMap);
			SMSSender->StoreMessage(Message);
		}
    else
      Log->Write("Message not stored.");
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return Message;
}

//---------------------------------------------------------------------------

void TForm1::CheckThreads()
{
    try
    {
        //Log->Write("CheckThreads started.");
        //if (Cfg->Peripherals.Modem->AutoDial!=0) {
        if ((Cfg->RASConnections.size()>1)||((Cfg->RASConnections.size()==1)&&(Cfg->Connection(0).Name!="")))
        {
            if (ModemThread == NULL)
            {
                Log->Write("Creating ModemThread...");
                ModemThread = new TModemThread(Cfg,WatchDog, FileMap);
                ModemThread->Resume();
            }
            else
            {
                if (ModemThread->Finished)
                    ModemThread->Resume();
                if (TDateTime::CurrentDateTime()>ModemThread->TimeMark+cnThreadTimeOut)
                {
                    Log->Write("Terminating and restarting ModemThread...");
                    ULONG ExitCode=0;
                    TerminateThread((HANDLE)ModemThread->Handle,ExitCode);
                    delete ModemThread;
                    ModemThread = NULL;
                    ModemThread = new TModemThread(Cfg,WatchDog, FileMap);
                    ModemThread->Resume();
                    Log->Append("OK");
                }
            }
        }
        else
        {
            if (FileMap)
                FileMap->FirstConnected = true;
        }
        if (Cfg->Payments.UpdateCardsInfo!=0)
            CheckThreadsTimer->Enabled=true;
        if (Cfg->Payments.UpdateCardsInfo!=0)// && FileMap->FirstConnected)
        {
            if (GetCardsInfoThread == NULL)
            {
                Log->Write("Creating GetCardsInfoThread...");
                GetCardsInfoThread = new TGetCardsInfoThread(InfoFile, Cfg, NULL, FileMap);
                GetCardsInfoThread->Resume();
            }
            else
            {
                if (GetCardsInfoThread->Finished)
                      GetCardsInfoThread->Resume();

                if (TDateTime::CurrentDateTime()>GetCardsInfoThread->TimeMark+cnThreadTimeOut)
                {
                //if (FileMap->GetCurrentTime()>FileMap->WCGetCardsInfoThreadTimeMark+cnThreadTimeOut) {
                    Log->Write("Terminating and restarting GetCardsInfoThread...");
                    ULONG ExitCode=0;
                    TerminateThread((HANDLE)GetCardsInfoThread->Handle,ExitCode);
                    delete GetCardsInfoThread;
                    GetCardsInfoThread = NULL;
                    GetCardsInfoThread = new TGetCardsInfoThread(InfoFile, Cfg, NULL, FileMap);
                    GetCardsInfoThread->Resume();
                    Log->Append("OK");
                }
            }
            CheckThreadsTimer->Enabled=true;
            //Log->Write("CheckThreads done.");
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void __fastcall TForm1::CheckThreadsTimerTimer(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);
    CheckThreads();
    if (Cfg->Dirs.DBNumCapacityUpdateAddress!="")
        CheckForNumDBUpdate();
}

//---------------------------------------------------------------------------

void TForm1::CheckForNumDBUpdate(void)
{
    try
    {
        if (FileMap->NumDBUpdateDone)
        {
            InfoFile->Write("DBNumCapacityUpdate","LastTimeUpdated",AnsiString(TDateTime::CurrentDateTime()));
            InfoFile->Write("DBNumCapacityUpdate","CurrentUpdateDT",AnsiString(FileMap->NumDBLastUpdatedDT));
            Log->Write("DBNumCapacityUpdate info updated.");
            FileMap->NumDBUpdateDone = false;
            return;
        }

        if (InfoFile->ReadDateTime("DBNumCapacityUpdate","LastTimeUpdated")+1>TDateTime::CurrentDateTime())
            return;

        if (NextCheckDBUpdateTime > TDateTime::CurrentDateTime())
            return;

        if (FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(-1)+".ok"))
        {
            if (!DeleteFile((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(-1)+".ok"))
            {
                 Log->Write((boost::format("Can't delete file %1%\\-1.ok, aborting NumDB update...") % Cfg->Dirs.CommandsInbound.c_str()).str().c_str());
                 return;
            }
        }

        if (FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(-1)+".pkt"))
        {
            if (!DeleteFile((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(-1)+".pkt"))
            {
                Log->Write((boost::format("Can't delete file %1%\\-1.pkt, aborting NumDB update...") % Cfg->Dirs.CommandsInbound.c_str()).str().c_str());
                return;
            }
        }

        std::auto_ptr <TCommandReceiver> CommandReceiver ( new TCommandReceiver("",Cfg,Log,FileMap) );

        if (CommandReceiver->StoreHTTPFileRequestCommand(-1,("interface.7z|"+Cfg->Dirs.DBNumCapacityUpdateAddress).c_str(),InfoFile->ReadDateTime("DBNumCapacityUpdate","CurrentUpdateDT")))
            InfoFile->Write("DBNumCapacityUpdate","LastTimeUpdated",AnsiString(TDateTime::CurrentDateTime()));
        NextCheckDBUpdateTime = TDateTime::CurrentDateTime()+0.25;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TForm1::Reboot()
{
try
	{
	Log->Write("Reboot sequence started...");
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;

	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken); // Get a token for this process.

	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); // Get the LUID for the shutdown privilege.

	tkp.PrivilegeCount = 1;  // one privilege to set
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); // Get the shutdown privilege for this process.
																																					 // Cannot test the return value of AdjustTokenPrivileges.
	ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0); //Shut down the system and force all applications to close.

	Log->Write("ExitWindows done...");
	Application->Terminate();
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TForm1::Payment_Init(AnsiString LocationString)
{
    //bool bForcedOffline=false;
    try
    {
    //Log->Write("FPI=false");
        std::auto_ptr <TLocationParser> Location (new TLocationParser(LocationString.c_str()));
        FinishPaymentInitiated=false;
        AnsiString Parameters=LocationString.SubString(LocationString.Pos("?")+1,LocationString.Length());
        std::string str=Location->GetParameter("recepient");
        if (( boost::lexical_cast<int>(Location->GetParameter("recepient")) == Cfg->ServiceInfo.OperatorId)&&(Location->GetParameter("field100") == Cfg->ServiceInfo.Number.c_str()))
        {
            Log->Write("Service menu password found.");
            EnterServiceMenu();
            return;
        }
        /*
        if (Validator)
        {
          Validator->DeviceState->Billing = true;
          //Validator->DeviceState->Idle = true;
        }
        */
        int recepientId = GetInt(Location->GetParameter("recepient").c_str());
        AnsiString ProcessorType=AnsiString(Cfg->Operator(recepientId).ProcessorType.c_str()).LowerCase();
        /*
        if (ProcessorType=="cyberplat_euroset")
        {
            if (Payment)
            {
                delete Payment;
                Payment=NULL;
            }
            Payment = new TEUPayment("", Cfg, Log, FileMap, InfoFile);
        }
        */
        if (ProcessorType=="cyberplat_pin" || ProcessorType=="cyberplat_pin_trans")
        {
            if (Payment)
            {
                delete Payment;
                Payment=NULL;
            }
            Payment = new TPinPayment("", Cfg, Log, FileMap, InfoFile);
        }
        else if (ProcessorType=="half_pin")
        {
            if (Payment)
            {
                delete Payment;
                Payment=NULL;
            }
            Payment = new THalfPinPayment("", Cfg, Log, FileMap, InfoFile);
        }
        else if (ProcessorType=="cyberplat_metro")
        {
            if (Payment==NULL)
            {
                Log->Write("Error! Metro Payment initialized without payment created!");
                GoToMainMenu();
                return;
            }
        }
        else if (ProcessorType=="cyberplat_mt")
        {
            if (Payment==NULL)
            {
                Log->Write("Error! Money transfers payment initialized without payment created!");
                GoToMainMenu();
                return;
            }
        }
        else if (ProcessorType=="avia_center")
        {
            if (Payment)
            {
                delete Payment;
                Payment=NULL;
            }
            Cfg->Operator(recepientId).Offline=0;
            Cfg->Operator(recepientId).PrinterOkOnly=true;

            saveDataToFile("");
            Payment = new TAviaPayment("", Cfg, Log, FileMap, InfoFile);
        }
        else if (ProcessorType == "cyberplat_taxes")
        {
            if(Payment)
            {
                delete Payment;
                Payment = NULL;
            }

            Payment = new TTaxPayment("", Cfg, Log, FileMap, InfoFile);
        }
        else
        {
            if (Payment)
            {
                delete Payment;
                Payment=NULL;
            }

            Payment = new TPayment("", Cfg, Log, FileMap, InfoFile);
        }

        if (Payment)
        {
            BillAcceptorFatalError=false;

            if (!Payment->InitPayment(LocationString))
            {
                if (Payment->IndyError)
                {
                    Log->Write("Indy error detected... ");
                    if (Cfg->Terminal.RebootAllowed)
                    {
                        Log->Append("Reboot allowed, trying to reboot...");
                        Reboot();
                    }
                    else
                    {
                        Log->Append("Reboot not allowed.");
                    }
                }
                if(33==Payment->CheckErrorCode && ProcessorType=="avia_center")
                {
                    std::string strtmp=((AnsiString)(static_cast<TAviaPayment*>(Payment))->GetAnswer()).c_str();
                    boost::replace_all(strtmp,"\r\n","<br>");
                    GoToFullScreenMessage(Parameters, Payment->CheckErrorCode, strtmp);
                }
                else if(ProcessorType=="cyberplat_mt")
                {
                    std::string LastErrorCode=boost::lexical_cast<std::string>((dynamic_cast<TMoneyTransferPayment*>(Payment))->LastErrorCode);
                    Navigate((boost::format("mt.html?%1%&state=show_error&error=%2%") % Parameters.c_str() % LastErrorCode).str().c_str());
                    return;
                }
                else
                {
                    GoToFullScreenMessage(Parameters, Payment->CheckErrorCode);
                }
                delete Payment;
                Payment=NULL;
                return;
            }
            else
            {
                if (!Cfg->isXMLParserOK())
                {
                    Log->Write("MSXML parser error detected!");
                    if (Cfg->Terminal.RebootAllowed)
                    {
                        Log->Append("Reboot allowed, trying to reboot...");
                        Reboot();
                    }
                    else
                    {
                        Log->Append("Reboot not allowed.");
                        GoToFullScreenMessage(Parameters);
                        delete Payment;
                        Payment=NULL;
                        return;
                    }
                }
                if (Location->GetParameter("pb") != "")
                  PB->payment = true;
            }
        }
        else
        {
            return;
        }

        if (Validator)
        {
            std::auto_ptr <paymentParameters> PP ( new paymentParameters(false));

            double MaxSum = PP->m_maxsum;
            if(Cfg->Operator(Payment->Recepient).fix)
            {
              TIntVector nominals;
              Cfg->strToIntVector(Localization["nominals"], nominals, "locale nominals");
              int max_nom = nominals.back();
              MaxSum = ceil((MaxSum + PP->m_cms)/max_nom)*max_nom;
            }
            Log->Write((boost::format("Maximum amount to collect in this payment: %1%") % MaxSum).str().c_str());
            Validator->SetMaxCash(MaxSum);
            //Log->Write("Validator.MaxCash: "+AnsiString(Validator.MaxCash));
        }

        if (AnsiString(Cfg->Operator(GetInt(Location->GetParameter("recepient").c_str())).ProcessorType.c_str()).LowerCase()=="avia_center")
        {
            saveDataToFile(((TAviaPayment *)Payment)->GetAnswer());
        }

        if(Cfg->Peripherals.Printer.ShowMessageOnError)
        {
            if(Payment->AddInfo!="" && Cfg->Operator(recepientId).ShowAddInfo && ProcessorType != "avia_center")
            {
                saveDataToFile(Payment->AddInfo.c_str());
                if (Parameters != "")
                    Parameters += "&";
                Parameters+="showAddInfo=1&sum="+FloatToStr(int(Payment->Sum*100))+"&cms="+FloatToStr(int(Payment->GetComission()*100))+"&mps="+FloatToStr(int(Payment->GetPaymentMinSum()*100))+"&maxsum="+AnsiString(int(Payment->GetLimMax()*100));
            }
            else if (ProcessorType != "avia_center")
            {
                saveDataToFile("");
            }

            if (Printer == NULL)
            {
                Log->Write("Check printer failed - NO printer installed.");
                if (Cfg->Operator(recepientId).PrinterOkOnly)
                   GoToFullScreenMessage(Parameters, -4);
                else
                {
                    if (!FileMap->CheckDebugState(cnPrinterError))
                    {
                        GoToMessage(-7, std::string(Parameters.c_str()));
                    }
                    else
                    {
                        Log->Append(" Printer Debug state!");
                        PreparePaymentNoPrinter();    //включаем валидатор(ы) и идем на payment
                    }
                }
                return;
            }
            else
            {
                if (!Printer->IsPrinterEnable()||((Printer->IsFiscal())&&(!Printer->SessionOpened)))
                {
                    Log->Write("Check printer failed - printer is not enabled!");
                    if (Cfg->Operator(recepientId).PrinterOkOnly)
                    {
                        GoToFullScreenMessage(Parameters, -4);
                    }
                    else
                    {
                        if (!FileMap->CheckDebugState(cnPrinterError))
                        {
                            GoToMessage(-7, std::string(Parameters.c_str()));
                        }
                        else
                        {
                            Log->Append(" Printer Debug state!");
                            PreparePaymentNoPrinter();   //включаем валидатор(ы) и идем на payment
                        }
                    }
                    return;
                }
            }
        }

        if(Payment->AddInfo!="" && Cfg->Operator(recepientId).ShowAddInfo && ProcessorType != "avia_center")
        {
            saveDataToFile(Payment->AddInfo.c_str());
            if (PB->entered)
              Navigate("pb.html?pb=error&errornumber=showaddinfo&sum="+FloatToStr(int(Payment->Sum*100))+"&cms="+FloatToStr(int(Payment->GetComission()*100))+"&mps="+FloatToStr(int(Payment->GetPaymentMinSum()*100))+"&maxsum="+AnsiString(int(Payment->GetLimMax()*100))+"&"+Payment->AFieldsForInterface);
            else
              Navigate("addinfo.html?sum="+FloatToStr(int(Payment->Sum*100))+"&cms="+FloatToStr(int(Payment->GetComission()*100))+"&mps="+FloatToStr(int(Payment->GetPaymentMinSum()*100))+"&maxsum="+AnsiString(int(Payment->GetLimMax()*100))+"&"+Payment->AFieldsForInterface);

            return;
        }
        else if (ProcessorType != "avia_center")
        {
            saveDataToFile("");
        }

        Payment->PayProcessStarted = true;
        if (Validator)
            Validator->EnableBill();
        if(CoinAcceptor)
            CoinAcceptor->Enable();

        GoToPayment();
        PaymentTimeOutTimer->Enabled=true;  //on Payment_Init
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        GoToMainMenu();
    }
}

//------------------------------------------------------------------------------

void TForm1::FinishPayment(bool bCancel, int iCommand)
{
    bool PrintDone=false;
    bool PaymentResult = false;
    //Log->Write("FPI=true");
    FinishPaymentInitiated=true;
    TMoneyTransferPayment* tmPayment = NULL;

    try
    {
        try
        {
            PaymentTimeOutTimer->Enabled=false;

            if (Payment)
            {
                std::auto_ptr <paymentParameters> PP ( new paymentParameters(false));
                tmPayment = dynamic_cast<TMoneyTransferPayment*>(Payment);

                if ((Validator)&&(iCommand==cnFPCmdNone))
                {
                    if(CoinAcceptor)
                      CoinAcceptor->Disable();
                    if (!Validator->DeviceState->Idle)
                    {
                      Validator->DisableBill();
                      Log->Write("Validator bill disabled.");
                      PaymentTimeOutTimer->Enabled=false;
                    }
                    int TimeOut = 1500;
                    std::auto_ptr <TDeviceState> DeviceState (new TDeviceState(NULL));

                    while (((!Validator->DeviceState->Idle)||(Validator->GetEventsCount()>0)) &&
                           (TimeOut>0) &&
                           (Cfg->Payments.rState) &&
                           (Validator->ErrorMode == DSE_OK))
                    {
                        if (FileMap)
                            FileMap->PutWCTimeMark();
                        Application->ProcessMessages();
                        TimeOut--;
                        Sleep(10);
                    }

                    Log->Write((boost::format("Total payment money: %1%.") % PP->m_paysum).str().c_str());
                    if (InfoFile)
                    {
                        double FullComission=0;
                        if(Validator)
                        {
                            FullComission = GetDouble(InfoFile->Read((boost::format("Validator%1%")%Validator->ID).str().c_str(), "Comission"));
                            InfoFile->Write((boost::format("Validator%1%")%Validator->ID).str().c_str(), "Comission", FullComission + Payment->GetComission(MoneyCounters.find(Validator->ID)->second->TotalMoney()));
                        }
                        if(CoinAcceptor)
                        {
                            FullComission = GetDouble(InfoFile->Read((boost::format("Validator%1%")%CoinAcceptor->ID).str().c_str(),"Comission"));
                            InfoFile->Write((boost::format("Validator%1%")%CoinAcceptor->ID).str().c_str(), "Comission", FullComission + Payment->GetComission(MoneyCounters.find(CoinAcceptor->ID)->second->TotalMoney()));
                        }
                    }
                }

                /*if (FileMap)
                if (FileMap->WCState & cnTerminalErrorMode == cnTerminalErrorMode)
                // Если терминал перешел в состояние ошибки, остаемся в нем...
                bCancel = true;*/

                //bCancel = ((FileMap)&&(FileMap->WCState & cnTerminalErrorMode == cnTerminalErrorMode)); // Если терминал перешел в состояние ошибки, остаемся в нем...
                //bCancel = ((FileMap)&&(FileMap->WCState & 1)); // Если терминал перешел в состояние ошибки, остаемся в нем...

                if (PP->m_is_payment_will_pass)
                {
                    BillAcceptorFatalErrorsCount=0;
                    BillAcceptorFatalErrorSent=false;
                    FileMap->LastPaymentReceived=TDateTime::CurrentDateTime();
                    if (!bCancel && !tmPayment)
                    {
                        //Navigate("PaymentComplete.html?processing=1&sum="+AnsiString(int(Payment->Sum)*100)+"&recepient="+AnsiString(Payment->Recepient));
                        GoToPaymentComplete(true, false);
                        Application->ProcessMessages();
                    }
                    /*if (iCommand==cnFPCmdCancel)
                    {
                    PaymentResult = Payment->InitDone(cnPDCmdCancel);
                    //PaymentResult = false;
                    }
                    else
                    {
                    PaymentResult = Payment->InitDone();
                    }*/

                    if(iCommand!=cnFPCmdForcePrintCheck && iCommand!=cnFPCmdStoreCanceledPayment)
                        PaymentResult = Payment->InitDone( ( iCommand==cnFPCmdCancel ? cnPDCmdCancel : cnPDCmdStore) );
                    else
                        PaymentResult=false;

                    if (!PaymentResult && iCommand!=cnFPCmdCancel && tmPayment && iCommand!=cnFPCmdForcePrintCheck && iCommand!=cnFPCmdStoreCanceledPayment)
                    {
                        //Navigate("PaymentComplete.html?sum="+AnsiString(int(Payment->Sum)*100)+"&recepient="+AnsiString(Payment->Recepient)+"&cmd=askuser&msg="+Payment->PostPaymentInfo);
                        //Navigate("PaymentComplete.html?sum="+AnsiString(int(Payment->Sum)*100)+"&recepient="+AnsiString(Payment->Recepient)+"&cmd=askretry&msg="+Payment->PostPaymentInfo);
                        GoToPaymentComplete(false, false, true, "askretry");
                        return;
                    }
                    if ((!PaymentResult)&&(Payment->RetryAllowed)&&(iCommand!=cnFPCmdCancel) && !tmPayment)
                    {
                        //Navigate("PaymentComplete.html?sum="+AnsiString(int(Payment->Sum)*100)+"&recepient="+AnsiString(Payment->Recepient)+"&cmd=askretry&msg="+Payment->PostPaymentInfo);
                        GoToPaymentComplete(false, false, true, "askretry");
                    }
                    else
                    {
                        try
                        {
                            Cfg->Terminal.ChequeCounter++;
                            FileMap->ChequeCounter = Cfg->Terminal.ChequeCounter;
                            InfoFile->Write("Program","ChequeCounter",AnsiString(Cfg->Terminal.ChequeCounter));
                            std::string tmpstr = Payment->AFieldsForCheque.c_str();
                            if(cnFPCmdForcePrintCheck == iCommand)
                                PaymentResult=true;//Печатаем чек завершенного платежа
                            PrintDone = PrintCheque(tmpstr, PP->m_paysum, Payment->GetComission(), PaymentResult, Payment);
                        }
                        catch(...)
                        {
                            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                            PrintDone=false;
                        }
                        if (!bCancel)
                        {
                            if (PrintDone || FileMap->CheckDebugState(cnPrinterError))
                            {
                                Log->Write("Cheque printing done!");

                                if (AnsiString(Cfg->Operator(Payment->Recepient).ProcessorType.c_str()).LowerCase()=="avia_center")
                                {
                                    saveDataToFile(((TAviaPayment *)Payment)->GetAnswer());
                                    //Navigate("PaymentComplete.html?Summ="+AnsiString(Payment->Sum)+"&sum="+AnsiString(int(Payment->Sum)*100)+"&recepient="+AnsiString(Payment->Recepient)+"&msg="+Payment->PostPaymentInfo+"&cmd=showinfo");
                                    GoToPaymentComplete(false, true, true, "showinfo");
                                }
                                else
                                {
                                    //Navigate("PaymentComplete.html?Summ="+AnsiString(Payment->Sum)+"&sum="+AnsiString(int(Payment->Sum)*100)+"&recepient="+AnsiString(Payment->Recepient)+"&msg="+Payment->PostPaymentInfo);
                                    switch(iCommand)
                                    {
                                        case cnFPCmdStoreCanceledPayment:
                                        case cnFPCmdCancel:
                                        case cnFPCmdRetry:
                                            GoToPaymentComplete(false, true, true, "clear");
                                            break;
                                        default:
                                            GoToPaymentComplete(false, true, true);
                                    }
                                }
                            }
                            else
                            {
                                Log->Write("Cheque printing error!");
                                GoToPrinterError();
                            }
                        }
                    }
                }
                else // сумма платежа меньше минимальной или нулевая
                {
                    if (Payment->Sum == 0) // сумма платежа нулевая
                    {
                        if (BillAcceptorFatalError)
                        {
                            BillAcceptorFatalErrorsCount++;
                            Log->Write((boost::format("BillAcceptorFatalErrorsCount: %1%") % BillAcceptorFatalErrorsCount).str().c_str());
                        }
                        PaymentResult = Payment->InitDone(cnPDCmdCancel);
                        if (!bCancel)
                          GoToMainMenu();
                    }
                    else  // сумма платежа меньше минимальной
                    {
                      try
                      {
                        if (!bCancel && !tmPayment)
                        {
                            GoToPaymentComplete(false, false);
                            Application->ProcessMessages();
                        }
                        Log->Write("Payment sum is too low - cancelling payment.");
                        PaymentResult = Payment->InitDone(cnPDCmdCancel);
                        std::string tmpstr = Payment->AFieldsForCheque.c_str();
                        PrintDone = PrintCheque(tmpstr, Payment->Sum, Payment->GetComission(), false,Payment);
                        if (!bCancel)
                            GoToMainMenu();
                      }
                      catch(...)
                      {
                          ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                          PrintDone=false;
                      }

                      if (PrintDone)
                          GoToPaymentComplete(false, false);
                      else
                      {
                          Log->Write("Cheque printing error!");
                          GoToPrinterError();
                      }
                    }
                }
            }
            else
            {
                Log->Write("FinishPayment Error: Payment==NULL");
                if (!bCancel)
                    GoToMainMenu();
                return;
            }
        }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            GoToMainMenu();
        }
    }
    __finally
    {
        if (bCancel)
        {
            CheckTerminalState();
        }
        if (Payment)
        {
            if ((!PaymentResult)&&(Payment->RetryAllowed))
            {
                Log->Write("Retry allowed -> payment NOT finished yet.");
            }
            else
            {
                Log->Write("Payment finished.");

                if ((Payment->Sum >= Payment->GetPaymentMinSum()) && (Payment->Sum > 0))
                    Payment->RenamePackets();
                else if(Payment->Sum > 0)
                    Payment->RenamePacketsToUnprocess();

                if (Payment)
                {
                    delete Payment;
                    Payment = NULL;
                }
                int TimeOut = 20;
                while (TimeOut>0)
                {
                    if (FileMap)
                        FileMap->PutWCTimeMark();
                    Application->ProcessMessages();
                    TimeOut--;
                    Sleep(100);
                }
                if (FileMap)
                    FileMap->CheckPaymentDir = true;
            }
        }
    }
}

//------------------------------------------------------------------------------
void TForm1::WaitForForm2Action()
{
    while(!Form2->IsFinished())
    {
        Application->ProcessMessages();
        Sleep(100);
    }
}

//------------------------------------------------------------------------------
void TForm1::PerformBalance()
{
    AnsiString IncassMessage;
    try
    {
        Log->Write("Incassation started...");
        Form2->Init(Log, 60000);
//        Form2->ShowForm("\nПроизвести снятие баланса?\n\nЕсли необходимо, введите номер устройства.", "");
        Form2->ShowForm(Localization["performbalanceq"], "");
        SetTopWindow(Form2->Handle);
        WaitForForm2Action();
        SetTopWindow(Handle);
        if(Form2->Result)
        {
            Cfg->Terminal.ChequeCounter++;
            FileMap->ChequeCounter = Cfg->Terminal.ChequeCounter;
            InfoFile->Write("Program", "ChequeCounter", AnsiString(Cfg->Terminal.ChequeCounter));
            AnsiString BalanceMessage;
            int num=0;
            if(Form2->NumPadEdit->Text!="")
                num=Form2->NumPadEdit->Text.ToInt() - 1;
            PrintBalance(num);
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    Form2->Close();
    SetTopWindow(Handle);
}

//------------------------------------------------------------------------------
void TForm1::PerformPaperReload()
{
  if (Printer)
  {
    try
    {
      Log->Write((boost::format("Paper reloading started, cheques: %1%") % Cfg->Terminal.ChequeCounter).str().c_str());
      Form2->Init(Log, 60000);
      Form2->ShowForm(Localization["paperreload"]);
      WaitForForm2Action();

      if(Form2->Result)
      {
        Log->Write("Password OK");
        Form2->ClearFinished();
        Form2->SetMask("", true);
        Form2->Close();
        SetTopWindow(Handle);

        Cfg->Terminal.ChequeCounter = 0;
        FileMap->ChequeCounter = Cfg->Terminal.ChequeCounter;
        InfoFile->Write("Program", "ChequeCounter", "0");
        if(Printer->IsFiscal())
        {
          Log->Write("Printer is in fiscal mode, test cheque cannot be printed.");
        }
        else
        {
          Printer->DeviceState->SetOutCodes(DSE_NOTSET, DSE_NOTPAPER);
          Log->Write("Trying to print cheque...");
          try
          {
            //PrintReceiptTemplate prt;
            //std::string Report = prt.Print(Cfg->ServiceInfo.TestChequeFileName.c_str()).c_str();
            //Log->WriteLines(Report.c_str());
            //Printer->PrintCheck(Report.c_str());
          }
          catch(...)
          {
            Log->Write("Unable to print test cheque!");
          }
        }
        Log->Write("Paper reloading performed.");
        Printer->Log->Write("Paper reloading performed.");
      }
      else
      {
        Log->Write("\"No\" button pressed - paper reloading been aborted.");
        Form2->Close();
        SetTopWindow(Handle);
      }
    }
    catch (...)
    {
      ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
  }
  else
  {
    Log->Write("There is no printers in the system!");
  }
}

void TForm1::askUserToIncassation()
{
    if (!Cfg->ServiceInfo.IncassationNumberMask.empty())
    {
        Form2->Init(Log, delayForm2 * 1000);
        Form2->ShowForm(Localization["incassmsg"], Cfg->ServiceInfo.IncassationNumberMask);
        SetTopWindow(Form2->Handle);
        WaitForForm2Action();
        SetTopWindow(Handle);
    }
    else
        Form2->Result = 1;
}

//------------------------------------------------------------------------------
void TForm1::PerformIncassation()
{
    int ValidatorID = -1;
    int IncassSessionID(0);
    AnsiString IncassMessage;

    int fOpenIncass = Cfg->Peripherals.Validator.IncassOpenStacker;

    try
    {
        Log->Write("Incassation started...");
        if (fOpenIncass == cnIncassationOpen)
        {
            //определяем количество устройств
            //2 - если только есть и монетник и купюрник, показываем окно с выбором девайса
            //1 - окно с выбором девайса не показываем
            //0 - выводим инфо о том, что девайсов нет
            int validator_exist    = (Validator    != NULL) && (   Validator->DeviceState->OldOutStateCode != DSE_NOTMOUNT);
            int coinAcceptor_exist = (CoinAcceptor != NULL) && (CoinAcceptor->DeviceState->OldOutStateCode != DSE_NOTMOUNT);
            int device_quantity = validator_exist + coinAcceptor_exist;
            //0 - купюрник, 1 - монетник
            ValidatorID = validator_exist * 1 + coinAcceptor_exist * 2 - 1;
            Log->Write((boost::format("Quantity of Incasscation Devices: %1%, ValidatorID: %2%")
                % device_quantity
                % ValidatorID).str());
            switch(device_quantity)
            {
                case 0:
                    Form2->ShowForm(Localization["incass_no_devices"], "", true, form2::HideAll);
                    SetTopWindow(Form2->Handle);
                    Application->ProcessMessages();
                    Sleep(4000);
                    Form2->Close();
                    SetTopWindow(Handle);
                    return;
                break;
                case 1:
                    askUserToIncassation();
                break;
                case 2:
                    askUserToIncassation();
                    if((Form2->Result) || (fOpenIncass == cnIncassationClose))
                    {
                        Form2->ClearFinished();
                        Form2->SetMask("", true);
                        if (fOpenIncass == cnIncassationOpen)
                        {
                          Form2->ShowForm(Localization["choose_incass_device"], "", true, form2::Show2Choice);
                          SetTopWindow(Form2->Handle);
                          Application->ProcessMessages();
                          WaitForForm2Action();
                          if (!Form2->NumPadEdit->Text.IsEmpty())
                            ValidatorID = Form2->NumPadEdit->Text.ToInt();
                          else
                          {
                            Form2->Close();
                            SetTopWindow(Handle);
                            Log->Write("go to menu...");
                            return;
                          }
                        }
                    }
                break;
            }
        }

        if((Form2->Result) || (fOpenIncass == cnIncassationClose))
        {
            Log->Write(AnsiString("Device number OK." + ValidatorID).c_str());
            AnsiString AmountString = "не получен";
            if(Cfg->ServiceInfo.IncassGetAmountURL!="")
            {
                Log->Write("   Cfg->ServiceInfo.IncassGetAmountURL!=\"\"");
                if((Form2->Result) && (fOpenIncass == cnIncassationOpen))
                {
                  Log->Write("      (Form2->Result) && (fOpenIncass == cnIncassationOpen)");
                  Form2->ShowMessage(Localization["performingbalance"], 40);
                  Form2->DisableAll();
                  Application->ProcessMessages();
                }

                std::auto_ptr<TPayment> TestPayment(new TPayment("", Cfg, Log, FileMap, InfoFile));
                bool GetAmountResult = TestPayment->GetAmount(AmountString);
                Log->Write((boost::format("Amount: %1%") % AmountString.c_str()).str().c_str());
                if((!GetAmountResult) && (fOpenIncass == cnIncassationOpen))
                {
                    Form2->ShowForm(Localization["cantgetbalance"]);
                    SetTopWindow(Form2->Handle);
                    WaitForForm2Action();
                    if(Form2->Result == 0)
                    {
                        Form2->Close();
                        SetTopWindow(Handle);
                        return;
                    }
                    Form2->EnableNumPad(true);
                    if (!Form2->Result)
                    {
                        Log->Write("\"No\" button pressed - incassation's been aborted.");
                        Form2->Close();
                        SetTopWindow(Handle);
                        return;
                    }
                }
            }
            //zh

            bool PrintResult = false;

            try
            {
                IncassSessionID = GetInt(InfoFile->Read("Validator" + AnsiString(ValidatorID), "CassetteID"));
                IncassSessionID++;
                Log->Write((boost::format("Incassation ID: %1%") % IncassSessionID).str());
                Cfg->Terminal.ChequeCounter++;
                FileMap->ChequeCounter = Cfg->Terminal.ChequeCounter;
                InfoFile->Write("Program", "ChequeCounter", AnsiString(Cfg->Terminal.ChequeCounter));
                PrintResult = IncassTerminal(ValidatorID);
            }
            catch(...)
            {
                ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            }

            //zh
            if(((!PrintResult) && (!FileMap->CheckDebugState(cnPrinterError))) && (fOpenIncass == cnIncassationOpen))
            {
                Form2->ShowForm(Localization["cantprintforincass"]);
                SetTopWindow(Form2->Handle);
                WaitForForm2Action();
                Form2->EnableNumPad(true);

                if(!Form2->Result)
                {
                    Log->Write("\"No\" button pressed - incassation's been aborted.");
                    Form2->Close();
                    SetTopWindow(Handle);
                    return;
                }
            }

            if (fOpenIncass == cnIncassationOpen)
            {
                Form2->ShowForm(Localization["performingincass"], "", true, form2::HideAll);
                SetTopWindow(Form2->Handle);
                Application->ProcessMessages();
                Sleep(4000);
                Form2->Close();
                SetTopWindow(Handle);
            }
            BillCounter = MoneyCounters.find(ValidatorID)->second;
            // --------------------------- переделал --------------------------------------
            std::auto_ptr <TCSPacketSender> CSPS (new TCSPacketSender("", Cfg, Log, FileMap));
            std::auto_ptr <TSSPacketSender> SSPS (new TSSPacketSender("", Cfg, Log, FileMap));
            // Подготовливаем сообщение со статистикой
            PrintReceiptTemplate prt;
            PrepareBalance(ValidatorID, prt);
            IncassMessage = prt.Print(Cfg->ServiceInfo.BalanceReportFileName.c_str()).c_str();
            Log->Write("IncassMessage: \n" + IncassMessage);
            SendEMail(cnETIncass, (boost::format("Terminal #%1% incassation.") % Cfg->Terminal.Number).str().c_str(), IncassMessage);
            TNotesVector vNotes;
            if(BillCounter)
            {
                for(int i = 0; i < BillCounter->GetNominalCount(); i++)
                {
                    TNote Note;
                    Note.ValidatorID = ValidatorID;
                    Note.CurrencyID = Cfg->CurrencyInfo.Currency;
                    Note.Nominal = BillCounter->GetNominal(i);
                    Note.Count = BillCounter->GetCount(i);
                    vNotes.push_back(Note);
                }
            }
            double Comission = GetDouble(InfoFile->Read("Validator" + AnsiString(ValidatorID), "Comission"));
            if(Cfg->StatInfo.ProcessorType == cnCyberPlatServer)
                CSPS->StoreIncassation(TDateTime::CurrentDateTime(), vNotes, Comission, AnsiString(IncassSessionID));
            else
                SSPS->StoreIncassation(TDateTime::CurrentDateTime(), vNotes, Comission, AnsiString(IncassSessionID));
            InfoFile->Write("Validator" + AnsiString(ValidatorID), "Comission", 0);
            vNotes.clear();
            // --------------------------- переделал --------------------------------------
            TDeviceClass *device = Validators.find(ValidatorID)->second;
            if(device)
            {
                Log->Write((boost::format("ClearMoney device %1%") % ValidatorID).str().c_str());
                device->ClearMoney();
            }
            if(BillCounter)
                BillCounter->ClearCounters();
            InfoFile->Write("Validator" + AnsiString(ValidatorID), "ValidatorID", ValidatorID);
            Log->Write((boost::format("Printer type: %1%, PrintUnprintedCheques: %2%.")
                % Cfg->Peripherals.Printer.Type
                % Cfg->Peripherals.Printer.PrintUnprintedCheques).str().c_str());
            if(((Cfg->Peripherals.Printer.Type).find("shtrih-frk")) &&
                (Cfg->Peripherals.Printer.PrintUnprintedCheques))
            {
                std::auto_ptr<TStringList> SLPrInfo(new TStringList());
                InfoFile->ReadPaymentsInfo("Program", SLPrInfo.get());
                Log->Write((boost::format("Found %1% unprinted cheques.") % SLPrInfo->Count).str().c_str());
                for(int i = 0; i < SLPrInfo->Count; i++)
                {
                    try
                    {
                        if (Printer->IsPrinterEnable() || FileMap->CheckDebugState(cnPrinterError))
                            Printer->PrintCheck((SLPrInfo->Values[SLPrInfo->Names[i]]).ToDouble(),
                                                 SLPrInfo->Names[i]);
                        else
                            Log->Write("Error printing cheque, printer disabled.");
                    }
                    catch(...)
                    {
                        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                    }
                }
                InfoFile->ClearPaymentsInfo("Program");
            }
            Log->Write("Incassation performed.");
        }
        else
            Log->Write("\"No\" button pressed - incassation's been aborted.");
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
	}
    if (fOpenIncass == cnIncassationOpen)
    {
        Form2->Close();
        SetTopWindow(Handle);
    }
}
/*
void TForm1::PerformIncassation()
{
    int CassetteID = 0;
    int PrevCassetteID = 0;
    AnsiString IncassMessage;
    try	{
        Log->Write("Incassation started...");
        Form2->Init(Log, 60000);
//        Form2->ShowForm("\nПроизвести инкассацию?\n\nЕсли необходимо, введите номер кассеты.", Cfg->ServiceInfo->IncassationNumberMask);
        Form2->ShowForm("\nПроизвести инкассацию?\n\nЕсли необходимо, введите номер кассеты.", "12");
        SetTopWindow(Form2->Handle);
        WaitForForm2Action();
        SetTopWindow(Handle);
		if(Form2->Result) {
            AnsiString AmountString = "не получен";
            if(Cfg->ServiceInfo->IncassGetAmountURL!="") {
                Form2->ShowMessage("Производится получение баланса.\n\nПожалуйста подолжите...", 40);
                Form2->DisableAll();
                Application->ProcessMessages();
          		std::auto_ptr<TPayment> TestPayment(new TPayment("", Cfg, Log, Crypt, FileMap, InfoFile));
                bool GetAmountResult = TestPayment->GetAmount(AmountString);
                Log->Write("Amount: " + AmountString);
                if(!GetAmountResult) {
                    Form2->ShowForm("Не удалось произвести получение баланса. Произвести инкассацию?");
                    SetTopWindow(Form2->Handle);
                    WaitForForm2Action();
                    if(Form2->Result == 0) {
                        Form2->Close();
                        SetTopWindow(Handle);
                        return;
                    }
                    Form2->EnableNumPad(true);
                    if (!Form2->Result) {
                        Log->Write("\"No\" button pressed - incassation's been aborted.");
                        Form2->Close();
                        SetTopWindow(Handle);
                        return;
                    }
                }
            }
            CassetteID = Form2->CasetteID;
            Log->Write("New cassette id: " + AnsiString(CassetteID));
            bool PrintResult = false;
            try {
                if (Printer != NULL) {
                    PrevCassetteID = GetInt(InfoFile->Read("Validator" + AnsiString(Validator->ID), "CassetteID"));
                    PrevCassetteID++;
                    Log->Write("Incassation ID: " + AnsiString(PrevCassetteID));
                    Cfg->Terminal->ChequeCounter++;
                    InfoFile->Write("Program", "ChequeCounter", AnsiString(Cfg->Terminal->ChequeCounter));
                    PrintResult = IncassTerminal(CassetteID, IncassMessage.c_str(), AmountString.c_str());
                }
            }
			catch (Exception& error) {
                Log->Write("Balance Print Exception! "+AnsiString(error.Message));
            }
    		if(!PrintResult) {
                Form2->ShowForm("Печать отчета невозможна. Произвести инкассацию?");
                SetTopWindow(Form2->Handle);
                WaitForForm2Action();
                Form2->EnableNumPad(true);
    			if(!Form2->Result) {
        			Log->Write("\"No\" button pressed - incassation's been aborted.");
                    Form2->Close();
					SetTopWindow(Handle);
                    return;
				}
			}
            Form2->ShowMessage("Производится инкассация.\n\nПожалуйста подождите...",40);
            Form2->DisableAll();
            Application->ProcessMessages();
			switch(Validator->ID) {
				case 0:
					BillCounter = BillCounter0;
					break;
				case 1:
					BillCounter = BillCounter1;
					break;
				case 2:
					BillCounter = BillCounter2;
					break;
			}
			// --------------------------- переделать --------------------------------------
			PNote Note;
            std::auto_ptr <TCSPacketSender> CSPS (new TCSPacketSender("", Cfg, Log, FileMap));
            std::auto_ptr <TSSPacketSender> SSPS (new TSSPacketSender("", Cfg, Log, FileMap));
            SendEMail(cnETIncass, "Terminal #"+Cfg->Terminal->Number+" incassation.",IncassMessage);
			std::auto_ptr<TList> Notes(new TList());
			if((BillCounter) && (Validator)) {
    			for(int i = 0; i < BillCounter->GetNominalCount(); i++) {
    				Note = new TNote;
                    Note->ValidatorID = Validator->ID;
                    Note->CurrencyID = Cfg->Peripherals->Validator->Currency;
                    Note->Nominal = BillCounter->GetNominal(i);
                    Note->Count = BillCounter->GetCount(i);
                    Notes->Add(Note);
                }
            }
			double Comission = GetDouble(InfoFile->Read("Validator" + AnsiString(Validator->ID), "Comission"));
			if(Cfg->StatInfo->ProcessorType == cnCyberPlatServer)
   				CSPS->StoreIncassation(TDateTime::CurrentDateTime(), Notes.get(), Comission, AnsiString(PrevCassetteID));
            else
   				SSPS->StoreIncassation(TDateTime::CurrentDateTime(), Notes.get(), Comission, AnsiString(PrevCassetteID));
			InfoFile->Write("Validator" + AnsiString(Validator->ID), "Comission", 0);
			for(int i = 0; i < Notes->Count; i++) {
                Note=(PNote)Notes->Items[i];
				delete Note;
            }
			Notes->Clear();
			// --------------------------- переделать --------------------------------------
			if(Validator)
				Validator->ClearMoney();
			if(BillCounter)
				BillCounter->ClearCounters();
			InfoFile->Write("Validator" + AnsiString(Validator->ID), "CassetteID", CassetteID);
			Log->Write("Printer type: "+Cfg->Peripherals->Printer->Type.LowerCase()+", PrintUnprintedCheques: "+AnsiString(int(Cfg->Peripherals->Printer->PrintUnprintedCheques))+".");
			for(int ii = 0; ii < 5; ii++) {
			   Sleep(1000);
			   FileMap->PutWCTimeMark();
            }
			if(((Cfg->Peripherals->Printer->Type.LowerCase()).Pos("shtrih-frk"))&&(Cfg->Peripherals->Printer->PrintUnprintedCheques)) {
                std::auto_ptr <TStringList> SLPrInfo ( new TStringList() );
                InfoFile->ReadPaymentsInfo("Program",SLPrInfo.get());
                Log->Write("Found "+AnsiString(SLPrInfo->Count)+" unprinted cheques.");
                for(int i=0;i<SLPrInfo->Count;i++) {
                    try {
					    if(Printer->IsPrinterEnable())
						    Printer->PrintCheck((SLPrInfo->Values[SLPrInfo->Names[i]]).ToDouble(),SLPrInfo->Names[i]);
                        else
                            Log->Write("Error printing cheque, printer disabled.");
                    } catch (Exception &e) {
                        Log->WriteInLine("Error printing payment cheque: "+AnsiString(e.Message)+".");
                    }
					FileMap->PutWCTimeMark();
                }
                InfoFile->ClearPaymentsInfo("Program");
            }
            Log->Write("Incassation performed.");
        } else
            Log->Write("\"No\" button pressed - incassation's been aborted.");
	} catch (Exception& error) {
        Log->Write("PerformIncassation Exception! " + AnsiString(error.Message));
	}
    Form2->Close();
    SetTopWindow(Handle);
}
*/
//------------------------------------------------------------------------------

void TForm1::PrintAllUnprintedCheques()
{
    if ((Cfg->Peripherals.Printer.PrintUnprintedCheques)||(Cfg->Peripherals.Printer.SaveUnprintedCheques))
    {
        std::auto_ptr <TStringList> SLPrInfo (new TStringList());
        InfoFile->ReadPaymentsInfo("Program",SLPrInfo.get());
        Log->Write((boost::format("Found %1% unprinted cheques.") % SLPrInfo->Count).str().c_str());
        for(int i = 0; i < SLPrInfo->Count; i++)
        {
            if (PrintRawData(ChangeChars(ChangeChars(SLPrInfo->Names[i],"\n","\t"),"\t","\r\n").c_str(),(SLPrInfo->Values[SLPrInfo->Names[i]]).ToDouble()))
                InfoFile->ClearPaymentInfo("Program",SLPrInfo->Names[i],SLPrInfo->Values[SLPrInfo->Names[i]]);
            FileMap->PutWCTimeMark();
        }
    }
}

bool TForm1::PrintPaymentErrror(const PrintReceiptTemplate& prt,double Sum,TPayment* Payment)
{
    std::string Report;
    if ((Payment->Sum>=Payment->GetPaymentMinSum())&&(Payment->Sum>0))
        Report = prt.Print(Cfg->Operator(Payment->Recepient).ErrorChequeFileName.c_str());
    else
        Report = prt.Print(Cfg->Operator(Payment->Recepient).ErrorMoneyChequeFileName.c_str());
    return PrintRawData(Report, Sum);
}

bool TForm1::PrintCheque(std::string &FieldsData,double Sum, double Comission, bool PaymentResult, TPayment* Payment/*, char* technologProc*/)
{
    std::string chequeFileName=Cfg->Operator(Payment->Recepient).ChequeFileName;

    TMoneyTransferPayment* paymentMT = dynamic_cast<TMoneyTransferPayment*>(Payment);
    THalfPinPayment* halfPin = dynamic_cast<THalfPinPayment*>(Payment);
    TTaxPayment* taxPayment = dynamic_cast<TTaxPayment*>(Payment);
    PrintReceiptTemplate prt;
    prt.SetRawParameter(FieldsData.c_str());
    prt.SetParameter("%TRANSID%", Payment->XMLP->TransId.c_str());
    prt.SetParameter("%SESSNUM%", Payment->Session.c_str());
    prt.SetParameter("%DATETIME%", GetCurrentDateTime("%Y-%m-%d %H:%M:%S").c_str());
    prt.SetParameter("%AMOUNTALL%", (boost::format("% 9.02f") % Sum).str().c_str());
    /*
    prt.SetParameter("%PRINTER_MODEL%", Cfg->Peripherals.Printer.Type.c_str());
    if (std::string(technologProc) != "")
      prt.SetParameter("%PRINTER_PROC%", technologProc);
    */
    if(paymentMT)
    {
        AnsiString strtmp=paymentMT->XMLP->GetParamValue("comiss");

        if(paymentMT->XMLP->SavedData == "SEND_TRANSFER_CONFIRM")
        {
            double amount=boost::lexical_cast<double>(paymentMT->XMLP->GetParamValue("ri_amount").c_str());
            double system_commission=boost::lexical_cast<double>(paymentMT->XMLP->GetParamValue("ri_system_commission").c_str());
            double rent_commission=boost::lexical_cast<double>(paymentMT->XMLP->GetParamValue("ri_rent_commission").c_str());
            /*
            prt.SetParameter("%AMOUNT%", (boost::format("% 6i") % amount).str().c_str());
            prt.SetParameter("%COMISSION%", (boost::format("% 6i") % rent_commission).str().c_str());
            prt.SetParameter("%SYSTEM_COMISSION%", (boost::format("% 6i") % system_commission).str().c_str());
            prt.SetParameter("%TRANS_ID%",paymentMT->XMLP->GetParamValue("transfer_code").c_str());
            */
            prt.SetParameter("%AMOUNT%", (boost::format("% 9.02f") % paymentMT->getCalculateAmount()).str().c_str());
            prt.SetParameter("%COMISSION%", (boost::format("% 9.02f") % paymentMT->getCalculateRentComission()).str().c_str());
            prt.SetParameter("%SYSTEM_COMISSION%", (boost::format("% 9.02f") % paymentMT->getCalculateSystemComission()).str().c_str());
            if(1 != boost::lexical_cast<double>(paymentMT->XMLP->GetParamValue("ri_exchange_rate").c_str()))
                prt.SetParameter("%EXCHANGE_RATE%", (Localization["exchange_rate"]+(boost::format("% 9.02f") % paymentMT->XMLP->GetParamValue("ri_exchange_rate").c_str()).str()).c_str());
            else
                prt.SetParameter("%EXCHANGE_RATE%", "");
            prt.SetParameter("%RECEPIENT_BANK_ADDRESS%", (boost::format("% 9.02f") % paymentMT->XMLP->GetParamValue("ri_bank_address").c_str()).str().c_str());
            prt.SetParameter("%TRANS_ID%",paymentMT->XMLP->GetParamValue("transfer_code").c_str());
        }
        else
        {
            Comission = 0;
            chequeFileName = chequeFileName.substr(0, chequeFileName.rfind(".")) + "_no_data" + chequeFileName.substr(chequeFileName.rfind("."), chequeFileName.length());
        }

        prt.SetParameter("%CURRENT_CURRENCY%",paymentMT->XMLP->GetParamValue("ri_cur_valute").c_str());
        prt.SetParameter("%FROM_NAME%",(paymentMT->XMLP->GetParamValue("si_first_name")+" "+paymentMT->XMLP->GetParamValue("si_middle_name")+" "+paymentMT->XMLP->GetParamValue("si_last_name")).c_str());
        prt.SetParameter("%MT_SYSTEM%",paymentMT->XMLP->GetParamValue("ri_mt_system_name").c_str());
        prt.SetParameter("%TO_NAME%",(paymentMT->XMLP->GetParamValue("ri_first_name")+" "+paymentMT->XMLP->GetParamValue("ri_middle_name")+" "+paymentMT->XMLP->GetParamValue("ri_last_name")).c_str());
        prt.SetParameter("%TO_BANK%",paymentMT->XMLP->GetParamValue("ri_bank_address").c_str());
    }
    else if(halfPin)
    {
        prt.SetParameter("%COMISSION%", (boost::format("% 9.02f") % Comission).str().c_str());
        prt.SetParameter("%AMOUNT%", (boost::format("% 9.02f") % (halfPin->XMLP->GetParamValue("AMOUNT").c_str())).str().c_str());
        prt.SetParameter("%RENT_COMISSION%", (boost::format("% 9.02f") % halfPin->getRentCommision().c_str()).str().c_str());
    }
    else if(taxPayment)
    {
        prt.SetParameter("%OKATO%", taxPayment->getParameter("ОКАТО").c_str());
        prt.SetParameter("%KBK%", taxPayment->getParameter("КБК").c_str());
        prt.SetParameter("%KPP%", taxPayment->getParameter("КПП").c_str());
        prt.SetParameter("%BIK%", taxPayment->getParameter("БИК").c_str());
        prt.SetParameter("%BILL%", taxPayment->getParameter("Счет").c_str());
        prt.SetParameter("%KORBILL%", taxPayment->getParameter("Кор.Счет").c_str());
        prt.SetParameter("%RECIEVER%", taxPayment->getParameter("Получатель").c_str());
        prt.SetParameter("%RECIEVER_BANK%", taxPayment->getParameter("Банк получателя").c_str());
        prt.SetParameter("%PAYER%", taxPayment->getParameter("Плательщик").c_str());
        prt.SetParameter("%P_NAME%", taxPayment->getParameter("ФИО").c_str());
        prt.SetParameter("%P_ADRESS%", taxPayment->getParameter("Адрес").c_str());
        prt.SetParameter("%P_INN%", taxPayment->getParameter("ИНН плательщика").c_str());
        prt.SetParameter("%P_TYPE%", taxPayment->getParameter("Тип платежа").c_str());
        prt.SetParameter("%P_PERIOD%", taxPayment->getParameter("Период").c_str());

        prt.SetParameter("%COMISSION%", (boost::format("% 9.02f") % Comission).str().c_str());
        prt.SetParameter("%AMOUNT%", (boost::format("% 9.02f") % (Sum - Comission)).str().c_str());
    }
    else if(dynamic_cast<TAviaPayment*>(Payment))
    {
        prt.SetParameter("%COMISSION%", (boost::format("% 9.02f") % Payment->GetComission()).str().c_str());
        prt.SetParameter("%AMOUNT%", (boost::format("% 9.02f") % boost::lexical_cast<double>(Payment->XMLP->GetParamValue("AMOUNT").c_str())).str().c_str());
    }
    else
    {
        prt.SetParameter("%COMISSION%", (boost::format("% 9.02f") % Comission).str().c_str());
        prt.SetParameter("%AMOUNT%", (boost::format("% 9.02f") % (Sum - Comission)).str().c_str());
    }

    prt.SetParameter("%OPNAME%", Cfg->Operator(Payment->Recepient).Name.c_str());
    prt.SetParameter("%INT_RECIPIENT_NAME%", Cfg->Operator(Payment->Recepient).NameForCheque.c_str());
    prt.SetParameter("%INT_RECIPIENT_INN%", Cfg->Operator(Payment->Recepient).INNForCheque.c_str());
    prt.SetStringParameter(Payment->AUnnamedFieldsForCheque.c_str(), ",");
    prt.SetParameter("%PIN_VALUE%", crypt::decrypt(Cfg->GetKeysNum(Cfg->Operator(Payment->Recepient).KeysId), prt.GetStandardParameter("%PIN_VALUE%")).c_str());

    if(!PaymentResult)
        return PrintPaymentErrror(prt,Sum,Payment);

    if (AnsiString(Cfg->Operator(Payment->Recepient).ProcessorType.c_str()).LowerCase()=="avia_center")
        prt.SetParameter("%ANSWER%", (static_cast<TAviaPayment*>(Payment))->GetAnswerForCheque().c_str());

    std::string Report = prt.Print(chequeFileName.c_str());
    if(!PrintRawData(Report, Sum))
    {
        if((Cfg->Peripherals.Printer.SaveUnprintedCheques) || (Cfg->Peripherals.Printer.PrintUnprintedCheques))
            InfoFile->AddPaymentInfo("Program", Sum, Report.c_str()); // сохраняем информацию о платеже для последующей печати
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------

bool TForm1::PrintRawData(std::string &Report, double Sum)
{
    try {
        Log->Write("Trying to print cheque to Log...");
        Log->WriteLines(Report.c_str());
        if(Printer) {
            if(!Printer->IsPrinterEnable()||((Printer->IsFiscal())&&(!Printer->SessionOpened))) {
                Log->Write("Cheque printing error: Printer not enabled.");
            } else {
                if(Printer->IsFiscal()) {
                    Log->Write("Trying to print fiscal cheque...");
                    Printer->PrintCheck(Sum, Report.c_str());
                } else {
                    Log->Write("Trying to print cheque...");
                    Printer->PrintCheck(Report.c_str());
                }
                return true;
            }
        } else {
            Log->Write("Cheque printing error: Printer not initialized.");
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return false;
}
//------------------------------------------------------------------------------
bool TForm1::PrintBalance(int CassetteID)
{
    PrintReceiptTemplate prt;
    PrepareBalance(CassetteID, prt);
    std::string Report = prt.Print(Cfg->ServiceInfo.BalanceReportFileName.c_str());
    Log->Write(Report.c_str());
    if(Printer) {
        if (!Printer->IsPrinterEnable()) {
            Log->Write("Balance report print error: Printer not enabled.");
            return false;
        } else {
            Log->Write("Trying to print report...");
            Printer->PrintCheck(Report.c_str());
            if (Printer->Fiscal)
                Printer->PrintXReport(Report.c_str());
        }
    } else {
          Log->Write("Balance report print error: Printer not initialized. Report info:");
          return false;
    }
    return true;
}
//------------------------------------------------------------------------------
bool TForm1::IncassTerminal(int CassetteID)
{
    bool result = false;    
    try
    {
        
        PrintReceiptTemplate prt;
        PrepareBalance(CassetteID, prt);
        if(Printer || FileMap->CheckDebugState(cnPrinterError))
        {
            if (((!Printer) && FileMap->CheckDebugState(cnPrinterError)) || (!Printer->IsPrinterEnable()))
            {
                Log->Write("Balance report print error: Printer not enabled. ");
                if (FileMap->CheckDebugState(cnPrinterError))
                    Log->Append("Printer debug state! ");
                Log->Append("Reporting inkass receipt.");
                prt.SetParameter("%INCASSRECEIPTCOUNTER%", "     0");
                std::string Report = prt.Print(Cfg->ServiceInfo.IncassReportFileName.c_str());
                Log->Write(Report.c_str());
            }
            else
            {
                if (Printer->Fiscal)
                {
                    Log->Write("Performing CashIncassation...");
                    Printer->CashIncassation();
                }
                for (int i = 0; i < Cfg->ServiceInfo.IncassReportCount; i++)
                {
                    prt.SetParameter("%INCASSRECEIPTCOUNTER%", (boost::format("%6d") % (i + 1)).str().c_str());
                    std::string Report = prt.Print(Cfg->ServiceInfo.IncassReportFileName.c_str());
                    Log->Write(Report.c_str());
                    if ((Cfg->Peripherals.Printer.ZReportWithIncassation == 1)&&(Printer->Fiscal))
                        Printer->PrintZReport(Report.c_str());
                    else
                    {
                        std::string barcode_info = Cfg->Peripherals.Printer.IncassBarCode ? getBarCodeString(CassetteID) : std::string();
                        Printer->PrintCheck(Report.c_str(), barcode_info);
                    }
                    //Printer->Cut();
                }
                if ((Cfg->Peripherals.Printer.ZReportWithIncassation == 2)&&(Printer->Fiscal))
                {
                    Printer->CashIncassation();
                }
            }
            result = true;
        }
        else
        {
            Log->Write("Balance report print error: No printer!");
        }
    }
    catch (...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return result;
}
//------------------------------------------------------------------------------
void TForm1::PrepareBalance(int CassetteID, PrintReceiptTemplate &PrintTemplate)
{
    BillCounter = MoneyCounters.find(CassetteID)->second;
    if(BillCounter)
    {
        PrintTemplate.SetParameter("%INCASSPERIODFROM%", BillCounter->LastIncassation.FormatString("yyyy-mm-dd H:M:S").c_str());
        PrintTemplate.SetParameter("%CURRENTDATETIME%", GetCurrentDateTime("%Y-%m-%d %H:%M:%S").c_str());
        PrintTemplate.SetParameter("%KASETTENUMBER%", CassetteID);
        PrintTemplate.SetParameter("%BILLAMOUNT%", (boost::format("%6d") % BillCounter->TotalBill()).str().c_str());
        PrintTemplate.SetParameter("%TOTALSUMM%", (boost::format("% 9.02f") % BillCounter->TotalMoney()).str().c_str());
        PrintTemplate.SetParameter("%INCASSRECEIPTNUMBER%", (int)Cfg->Terminal.ChequeCounter);
        PrintTemplate.SetParameter("%ACCOUNTBALANCE%", (int)Cfg->Terminal.ChequeCounter);
        int MaxNominals = BillCounter->GetNominalCount();
        Log->WriteInLine((boost::format("Billcounter%1% maxnominals=%2%") % CassetteID % MaxNominals).str().c_str());
        for(int i = 0; i < MaxNominals; i++) {
            Log->WriteInLine((boost::format("Nominal: %1%") % BillCounter->GetNominal(i)).str().c_str());
            double nominal = BillCounter->GetNominal(i);
            PrintTemplate.SetParameter((boost::format("%%%1%BILLNOMINAL%%") % nominal).str().c_str(), (boost::format("%6d") % nominal).str().c_str());
            PrintTemplate.SetParameter((boost::format("%%%1%BILLAMOUNT%%") % nominal).str().c_str(), (boost::format("%6d") % BillCounter->GetCount(i)).str().c_str());
            PrintTemplate.SetParameter((boost::format("%%%1%BILLSUMM%%") % nominal).str().c_str(), (boost::format("%6d") % (BillCounter->GetCount(i) * BillCounter->GetNominal(i))).str().c_str());
        }
    }
    PrintTemplate.SetParameter("%INCASSPERIODFROM%", BillCounter->LastIncassation.FormatString("yyyy-mm-dd H:M:S").c_str());
    PrintTemplate.SetParameter("%CURRENTDATETIME%", GetCurrentDateTime("%Y-%m-%d %H:%M:%S").c_str());
    PrintTemplate.SetParameter("%KASETTENUMBER%", CassetteID);
    PrintTemplate.SetParameter("%BILLAMOUNT%", (boost::format("%6d") % BillCounter->TotalBill()).str().c_str());
    PrintTemplate.SetParameter("%TOTALSUMM%", (boost::format("% 9.02f") % BillCounter->TotalMoney()).str().c_str());
    PrintTemplate.SetParameter("%INCASSRECEIPTNUMBER%", (int)Cfg->Terminal.ChequeCounter);
    PrintTemplate.SetParameter("%ACCOUNTBALANCE%", (int)Cfg->Terminal.ChequeCounter);
    int MaxNominals = BillCounter->GetNominalCount();
    Log->WriteInLine((boost::format("%1%") % MaxNominals).str().c_str());
    for(int i = 0; i < MaxNominals; i++) {
        double nominal = BillCounter->GetNominal(i);
        PrintTemplate.SetParameter((boost::format("%%%1%BILLNOMINAL%%") % nominal).str().c_str(), (boost::format("%6d") % nominal).str().c_str());
        PrintTemplate.SetParameter((boost::format("%%%1%BILLAMOUNT%%") % nominal).str().c_str(), (boost::format("%6d") % BillCounter->GetCount(i)).str().c_str());
        PrintTemplate.SetParameter((boost::format("%%%1%BILLSUMM%%") % nominal).str().c_str(), (boost::format("%6d") % (BillCounter->GetCount(i) * BillCounter->GetNominal(i))).str().c_str());
    }
}
//------------------------------------------------------------------------------
bool TForm1::PrintZReport()
{
    try
    {
        if((Printer != NULL)&&(Printer->Fiscal))
        {
            if(!Printer->IsPrinterEnable())
            {
                Log->Write("Report print error: Printer not enabled.");
            }
            else
            {
                Printer->PrintZReport("");
                return true;
            }
        }
        else
        {
            Log->Write("Report print error: Printer not initialized.");
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return false;
}
//------------------------------------------------------------------------------
void TForm1::Navigate(AnsiString Address)
{
    std::string URL;
    try
    {
        if (LogWriteErrorFound)
            return;

        if (FileMap)
            FileMap->PutLastStartNavigate();

        std::string qURL = Address.c_str();
        URL = getFullPathURL(qURL);
        if (Cfg->CDebug.Logs.url)
        {
            std::string URL_prefix = "file:///";
            std::string fullURL = URL.substr(URL.find(URL_prefix) ? 0 : URL_prefix.length());
            Log->Write((boost::format("Navigating to {%1%}...") % (Cfg->CDebug.Logs.full ? fullURL : qURL)).str().c_str());
        }
        /*
        if (CppWebBrowser1->Busy)
            CppWebBrowser1->Stop();
        */
        TVariant vFlags = {2};//navNoHistory
        CppWebBrowser1->Navigate(WideString(URL.c_str()), &vFlags, NULL, NULL, NULL);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Log->WriteInLine((boost::format("Navigating error! URL: {%1%}.") % URL).str().c_str());
    }
}

//------------------------------------------------------------------------------

void TForm1::HardwareInit()
{
    try {
        std::string DetailsXML = Cfg->Dirs.StatusFileName;
        ////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////
        if(Cfg->Peripherals.Scanner.Type == "quantum3480")
         scannerDevice = new CQuantumScanner(Cfg->Peripherals.Scanner.Port);
        else
         scannerDevice = new CNullScanner();

        scannerDevice->ChangeEvent = CCNETStateChange;
        ////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////
        int mode = static_cast<int>((Cfg->CurrencyInfo.currencyId == 398) || (Cfg->CurrencyInfo.currencyId == 980));

        if((Cfg->Peripherals.Validator.Type == "cashcode")&&(Cfg->Peripherals.Validator.Protocol == "ccnet"))
            Validator = new TCCNETdevice(ValidatorID, Cfg->Peripherals.Validator.Port);
        if((Cfg->Peripherals.Validator.Type == "cashcode")&&(Cfg->Peripherals.Validator.Protocol == "wba003"))
            Validator = new TID003_1device(ValidatorID, Cfg->Peripherals.Validator.Port);
        if((Cfg->Peripherals.Validator.Type == "cashcode")&&(Cfg->Peripherals.Validator.Protocol == "wba003-2"))
            Validator = new TID003_2device(ValidatorID, Cfg->Peripherals.Validator.Port);
        if((Cfg->Peripherals.Validator.Type == "jcm")&&(Cfg->Peripherals.Validator.Protocol == "id003"))
            Validator = new TJCMdevice(ValidatorID, Cfg->Peripherals.Validator.Port);
        if((Cfg->Peripherals.Validator.Type == "ict")&&(Cfg->Peripherals.Validator.Protocol == "ict004"))
            Validator = new TICTDevice(ValidatorID, Cfg->Peripherals.Validator.Port);
        if((Cfg->Peripherals.Validator.Type == "gpt")&&(Cfg->Peripherals.Validator.Protocol == "v2e"))
            Validator = new TV2Edevice(ValidatorID, Cfg->Peripherals.Validator.Port);
        if((Cfg->Peripherals.Validator.Type == "nv9")&&(Cfg->Peripherals.Validator.Protocol == "ccnet"))
            Validator = new TNV9_CCNETdevice(ValidatorID, Cfg->Peripherals.Validator.Port);
        if(Cfg->Peripherals.Validator.Type == "mei_ebds")
            Validator = new TMEIDeviceClass(ValidatorID, Cfg->Peripherals.Validator.Port, NULL, mode);
        if(Cfg->Peripherals.Validator.Type == "none")
            Validator = new CValidator(ValidatorID, 0);
       if(!Validator)
            Log->Write((boost::format("Can't initialize %1% validator with %2% protocol on port COM%3%!") % Cfg->Peripherals.Validator.Type % Cfg->Peripherals.Validator.Protocol % Cfg->Peripherals.Validator.Port).str().c_str());
        else {
            MoneyCounters.insert(ValidatorID, new CMoneyCounter(ValidatorID, DetailsXML.c_str(), Log, InfoFile));
            Validators.insert(ValidatorID, Validator);
            ValidatorID++;
            Validator->ChangeEvent = CCNETStateChange;
            Validator->ExchangeRate = Cfg->CurrencyInfo.ExchangeRate;
            Validator->Currency = Cfg->CurrencyInfo.Currency.c_str();
            Validator->SetBillsSensitivity(Cfg->Peripherals.Validator.BillsSensitivity.c_str());
            Validator->SetMinCash(GetInt(Cfg->Terminal.NoteMinAmount));
            Validator->StartDevice();
            double firmware = 0;
            double project = 0;
            std::string firmwareProjectStr = "";
            if(Cfg->Peripherals.Validator.Type == "mei_ebds")
            {
                try
                {
                    Validator->getVersion();
                    activeSleep(300, 10);
                    firmware = Validator->DeviceState->FirmWare;
                    project = Validator->DeviceState->ProjectNumber;
                    if((Validator->DeviceState->FirmWare > ExtendedModeFromFirmware) ||
                       (Validator->DeviceState->ProjectNumber > ExtendedModeFromProject))
                    {
                        Validator->DeviceThread->mode = MEI_MODE::Extended;
                        Validator->mode = MEI_MODE::Extended;
                        firmwareProjectStr += " Extended mode";
                    }
                }
                catch(...)
                {}
                firmwareProjectStr += (my_round(firmware, true) == 0) ? "." : (boost::format(", firmware version = %3.02f") % firmware).str().c_str();
                firmwareProjectStr += (my_round(project, true) == 0) ? "" : (boost::format(", project version = %d") % project).str().c_str();
            }
            Log->Write((boost::format("%1% validator with %2% protocol initialized on port COM%3%%4%")
              % Cfg->Peripherals.Validator.Type
              % Cfg->Peripherals.Validator.Protocol
              % Cfg->Peripherals.Validator.Port
              % firmwareProjectStr).str().c_str());
        }
        if(Cfg->Peripherals.CoinAcceptor.Type == "nri_g13")
            CoinAcceptor = new TNRIdevice(ValidatorID,Cfg->Peripherals.CoinAcceptor.Port);
        if(Cfg->Peripherals.CoinAcceptor.Type == "none")
            CoinAcceptor = new CCoinAcceptor(ValidatorID, 0);
        if(!CoinAcceptor)
            Log->Write((boost::format("Can't initialize %1% coin acceptor on port COM%2%!") % Cfg->Peripherals.CoinAcceptor.Type % Cfg->Peripherals.CoinAcceptor.Port).str().c_str());
        else {
            MoneyCounters.insert(ValidatorID, new CMoneyCounter(ValidatorID, DetailsXML.c_str(), Log, InfoFile));
            Validators.insert(ValidatorID, CoinAcceptor);
            ValidatorID++;
            CoinAcceptor->ChangeEvent = CCNETStateChange;
            CoinAcceptor->ExchangeRate = Cfg->CurrencyInfo.ExchangeRate;
            CoinAcceptor->Currency = Cfg->CurrencyInfo.Currency.c_str();
            CoinAcceptor->SetMinCash(GetInt(Cfg->Terminal.NoteMinAmount));
            CoinAcceptor->StartDevice();
            Sleep(300);
            CoinAcceptor->Disable();
            Log->Write((boost::format("%1% coin acceptor with initialized on port COM%2%.") % Cfg->Peripherals.CoinAcceptor.Type % Cfg->Peripherals.CoinAcceptor.Port).str().c_str());
        }
        if(Cfg->Peripherals.Printer.Type == "shtrih-frk") {
            Printer = new CShtrihPrinter();
            Printer->AutoOpenShift=Cfg->Peripherals.Printer.AutoOpenShift;
            Printer->Fiscal=true;
        }
        if(Cfg->Peripherals.Printer.Type == "shtrih-frk-buffer") {
            Printer = new CShtrihPrinter();
            Printer->AutoOpenShift=Cfg->Peripherals.Printer.AutoOpenShift;
            Printer->Fiscal=true;
            Printer->ZReportInBuffer=true;
        }
        if(Cfg->Peripherals.Printer.Type == "shtrih-kiosk")
            Printer = new CShtrihPrinter();
        if(Cfg->Peripherals.Printer.Type == "prim21k-frk") {
            Printer = new CPrim21kClass(Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
            Printer->AutoOpenShift=Cfg->Peripherals.Printer.AutoOpenShift;
            Printer->Fiscal=true;
        }
        if(Cfg->Peripherals.Printer.Type == "prim21k-frk-buffer") {
            Printer = new CPrim21kClass(Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
            Printer->AutoOpenShift=Cfg->Peripherals.Printer.AutoOpenShift;
            Printer->Fiscal=true;
            Printer->ZReportInBuffer=true;
        }
        if(Cfg->Peripherals.Printer.Type == "prim21k-kiosk")
            Printer = new CPrim21kClass(Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
        if(Cfg->Peripherals.Printer.Type == "prim08tk-frk") {
            Printer = new CPrim08TKClass(Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
            Printer->AutoOpenShift=Cfg->Peripherals.Printer.AutoOpenShift;
            Printer->Fiscal=true;
        }
        if(Cfg->Peripherals.Printer.Type == "prim08tk-frk-buffer") {
            Printer = new CPrim08TKClass(Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
            Printer->AutoOpenShift=Cfg->Peripherals.Printer.AutoOpenShift;
            Printer->Fiscal=true;
            Printer->ZReportInBuffer=true;
        }

        ////////////////////////////////////////////////////////////////////////
        // astafiev
        if(Cfg->Peripherals.Printer.Type == "startsp600-frk-buffer")
        {
         Printer = new CStarTSP600("CStarTSP600",Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
         Printer->Fiscal = true;
         Printer->ZReportInBuffer = true;
        }
        if(Cfg->Peripherals.Printer.Type == "startsp600")
        {
         Printer = new CStarTSP600("CStarTSP600",Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
        }
        // Для гепарда реализация практически таже самая
        if(Cfg->Peripherals.Printer.Type == "mercmsk-frk")
        {
         Printer = new CStarTSP600("mercmsk", Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
         Printer->Fiscal = true;
        }
        if(Cfg->Peripherals.Printer.Type == "mercmsk")
        {
         Printer = new CStarTSP600("mercmsk", Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
        }
        if(Cfg->Peripherals.Printer.Type == "wp_k833")
            Printer = new wp_k833(Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
        if(Cfg->Peripherals.Printer.Type == "bd2-286")
            Printer = new bd2Class(Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);

        if(Cfg->Peripherals.Printer.Type == "felixrk")
            Printer = new CAtolPrinter(FelixRK, Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
        if(Cfg->Peripherals.Printer.Type == "felixrk-frk")
        {
            Printer = new CAtolPrinter(FelixRK, Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
            Printer->Fiscal = true;
        }
        if(Cfg->Peripherals.Printer.Type == "felixrk-frk-buffer")
        {
            Printer = new CAtolPrinter(FelixRK, Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
            Printer->ZReportInBuffer = true;
            Printer->Fiscal = true;
        }
        if(Cfg->Peripherals.Printer.Type == "felix80k")
        {
            Printer = new CAtolPrinter(Felix80K, Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
        }
        if(Cfg->Peripherals.Printer.Type == "felix80k-frk")
        {
            Printer = new CAtolPrinter(Felix80K, Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
            Printer->Fiscal = true;
//            AnsiString c;
//            c = "ООО КБ «ПЛАТИНА»\r\n123610, г.Москва,\r\nКраснопресненская наб., д.12\r\nИНН 7705012216\r\nТелефон службы поддержки:\r\n\r\n8-800-555-55-00\r\nАдрес точки приема платежей:\r\nг.Москва, МКАД 104 км, д.6\r\n     Дата     Время     Терминал\r\n  30.09.2008 17:48:03   №109\r\nНомер сессии : 30090817480300001081\r\nСумма        : 3800.00 руб.\r\nКомиссия     : 76.00 руб.\r\nК зачислению : 3724.00 руб.\r\nОператор     : ООО \"Кредит Европа Банк\"\r\nИНН: 7705148464\r\nНомер карты: 6048-1733-4708-4929\r\nФамилия: Буравцева\r\nИмя: Оксана\r\nОтчество: Мирчевна\r\nДень рождения: 29\r\nМесяц рождения: мая\r\nГод рождения: 1968\r\nМесто рождения: д.Авдотькино\r\n-----------------------------------\r\nКредитная организация: ООО КБ «ПЛАТИНА»\r\nБИК 044585931 Тел. +7 (495) 981-80-80\r\n-----------------------------------\r\nСПАСИБО, СОХРАНЯЙТЕ ЧЕК!\r\nПЛАТЕЖНАЯ СИСТЕМА КИБЕРПЛАТ";
//            Printer->PrintCheck(c);
        }

        ////////////////////////////////////////////////////////////////////////

        if(Cfg->Peripherals.Printer.Type == "prim08tk-kiosk")
            Printer = new CPrim08TKClass(Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
        if(Cfg->Peripherals.Printer.Type == "startup900")
            Printer = new CStarTUP900(Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
        if(Cfg->Peripherals.Printer.Type == "startup900lpt")
            Printer = new CStarTUP900(Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL, PortType::lpt);
        if(Cfg->Peripherals.Printer.Type == "startsp700")
            Printer = new CStarTSP700(Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
        if(Cfg->Peripherals.Printer.Type == "citizencbm1000t2")
            Printer = new CBM1000Type2(Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
        if(Cfg->Peripherals.Printer.Type == "av-268")
            Printer = new CCitizen268(Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
        if(Cfg->Peripherals.Printer.Type == "custom_vkp_80")
            Printer = new CCustomPrn(Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
        if(Cfg->Peripherals.Printer.Type == "citizen_ppu_231")
            Printer = new CCitizenPPU231(Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
        if(Cfg->Peripherals.Printer.Type == "citizen_ppu_232")
            Printer = new CCitizenPPU232(Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
        if(Cfg->Peripherals.Printer.Type == "citizen_ppu_700")
            Printer = new CCitizenPPU700(Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
        if((Cfg->Peripherals.Printer.Type == "epson_422") || (Cfg->Peripherals.Printer.Type == "epson_442"))
            Printer = new CEpson442(Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
        if(Cfg->Peripherals.Printer.Type == "prn609_012r")
            Printer = new PRN609_012R(Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
        if(Cfg->Peripherals.Printer.Type == "swecoin-ttp2010")
            Printer = new CSwecoinTTP2010(Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
        if(Cfg->Peripherals.Printer.Type == "gebe-gct")
            Printer = new GebeGCT(Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
        if(Cfg->Peripherals.Printer.Type == "wp_t833")
            Printer = new wp_t833(Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
        if(Cfg->Peripherals.Printer.Type == "windows")
            Printer = new CWinPrinter(Handle,GetInt(Cfg->Peripherals.Printer.Font.c_str()));
        if((Cfg->Peripherals.Printer.Type == "citizen_ccp_8001") || (Cfg->Peripherals.Printer.Type == "citizen_cpp_8001"))
            Printer = new CCitizenCPP8001(Cfg->Peripherals.Printer.Port, Cfg->Peripherals.Printer.PortBaudRate, NULL);
        if(!Printer)
            Log->Write((boost::format("Can't initialize %1% printer on port COM%2%!") % Cfg->Peripherals.Printer.Type % Cfg->Peripherals.Printer.Port).str().c_str());
        else {
            Printer->ChangeEvent = CCNETStateChange;
            Printer->MinLinesCount=Cfg->Peripherals.Printer.MinLinesCount;
            Printer->Font = Cfg->Peripherals.Printer.Font;
            Printer->PushActionType=(TPushActionType)Cfg->Peripherals.Printer.PresenterCommand;
            Printer->Initialize();
            Printer->GetState();
            if (Printer->DeviceState->OutStateCode != DSE_OK)
                Log->Write((boost::format("%1% printer initialized on port COM%2% with error! State = %3% SubState = %4%.") % Cfg->Peripherals.Printer.Type % Cfg->Peripherals.Printer.Port % Printer->StateDescr % Printer->SubStateDescr).str().c_str());
            else
                Log->Write((boost::format("%1% printer successfully initialized on port COM%2%.") % Cfg->Peripherals.Printer.Type % Cfg->Peripherals.Printer.Port).str().c_str());
        }
        if(Cfg->Peripherals.CardReader.Type == "metro")
            CardReader = new TMetroCardDevice(Cfg->Peripherals.CardReader.Port);
        if((Cfg->Peripherals.CardReader.Type != "none") && (Cfg->Peripherals.CardReader.Type != "")) {
            if(!CardReader)
                Log->Write((boost::format("Initialization of the %1% card reader failed!") % Cfg->Peripherals.CardReader.Type).str().c_str());
            else {
                Log->Write((boost::format("CardReader %1% initialized.") % Cfg->Peripherals.CardReader.Type).str().c_str());
                CardReader->ChangeEvent = CCNETStateChange;
                CardReader->StartDevice();
                CardReader->InitInfo->nSystemCode = Cfg->Peripherals.CardReader.SystemCode;
                CardReader->InitInfo->nDeviceCode = Cfg->Peripherals.CardReader.DeviceCode;
                CardReader->InitInfo->nCom = (short)Cfg->Peripherals.CardReader.Port;
                int res = CardReader->Init();
                switch (res)
                {
                    case AI_Init_Success:
                        Log->Write("CardReader succsessfully initialized");
                        break;
                    case AI_No_BSK_Connect:
                        Log->Write("CardReader Init Error! No connection.");
                        break;
                    case AI_No_Server_Connect:
                        Log->Write("CardReader Init Error! No connection with metro application server.");
                        break;
                    default:
                        Log->Write((boost::format("CardReader Init Unknown Error #%1%!") % res).str().c_str());
                        break;
                }
            }
        }

        if(Cfg->Peripherals.CardReader.Type == "sankyo330")
            cardReaderDevice = new CSankyoCardReader(Cfg->Peripherals.CardReader.Port);
        else
            cardReaderDevice = new CNullCardReader(Cfg->Peripherals.CardReader.Port);

        cardReaderDevice->ChangeEvent = CCNETStateChange;
        // подождать после поднятия RTS
        Sleep(1000);
        cardReaderDevice->Initialize();

        if(Cfg->Peripherals.WatchDog.Type == "osmp")
            WatchDog = new TWatchDogOSMPdevice(Cfg->Peripherals.WatchDog.Port);
        if(Cfg->Peripherals.WatchDog.Type == "osmp-2sim")
            WatchDog = new TSIM2OSMPdevice(Cfg->Peripherals.WatchDog.Port);
        if(Cfg->Peripherals.WatchDog.Type == "osmp2")
            WatchDog = new TWatchDogOSMP2device(Cfg->Peripherals.WatchDog.Port);
        if(Cfg->Peripherals.WatchDog.Type == "st1.2")
            WatchDog = new TWatchDogdevice(Cfg->Peripherals.WatchDog.Port);
        if(Cfg->Peripherals.WatchDog.Type == "alniko")
            WatchDog = new TWatchDog(Cfg->Peripherals.WatchDog.Port, WD_TYPE_ALNIKO, NULL, Cfg->Peripherals.Modem.Port);
        if(Cfg->Peripherals.WatchDog.Type == "newgt")
            WatchDog = new TWatchDog(Cfg->Peripherals.WatchDog.Port, WD_TYPE_NEW_GT, NULL, Cfg->Peripherals.Modem.Port);
        if(Cfg->Peripherals.WatchDog.Type == "alarm")
            WatchDog = new TWatchDogAlarmDevice(Cfg->Peripherals.WatchDog.Port);
        if(Cfg->Peripherals.WatchDog.Type == "platix")
            WatchDog = new TWatchDogPlatixDevice(Cfg->Peripherals.WatchDog.Port);
        if(Cfg->Peripherals.WatchDog.Type == "fairpay")
            WatchDog = new TFairPayWDDevice();
        if(Cfg->Peripherals.WatchDog.Type == "sbk2")
            WatchDog = new TSBK2Device();
        if(Cfg->Peripherals.WatchDog.Type == "ldog")
            WatchDog = new TLDOGdevice(Cfg->Peripherals.WatchDog.Port);
        if(Cfg->Peripherals.WatchDog.Type == "expresspay")
            WatchDog = new TExpressPaydevice(Cfg->Peripherals.WatchDog.Port);
        if((Cfg->Peripherals.WatchDog.Type != "none") && (Cfg->Peripherals.WatchDog.Type != ""))
        {
            if(WatchDog == NULL)
                Log->Write((boost::format("Can't initialize watchdog %1%! no WatchDog.") % Cfg->Peripherals.WatchDog.Type).str().c_str());
            else
            {
              try
              {
                WatchDog->ChangeEvent = CCNETStateChange;
                WatchDog->StartTimer();
                if (WatchDog->IsItYou())
                {
                  Log->Write((boost::format("Watchdog %1% initialized successfully.") % Cfg->Peripherals.WatchDog.Type).str().c_str());
                }
                else
                {
                  Log->Write((boost::format("Can't initialize watchdog %1%! selftest failed.") % Cfg->Peripherals.WatchDog.Type).str().c_str());
                  delete WatchDog;
                  WatchDog = NULL;
                }
              }
              catch(...)
              {
                Log->Write((boost::format("Can't initialize watchdog %1%! unknown error.") % Cfg->Peripherals.WatchDog.Type).str().c_str());
                delete WatchDog;
                WatchDog = NULL;
              }
            }
        }
        Keyboard = NULL;
        if(Cfg->Peripherals.Keyboard.Type == "iskra") {
            Keyboard = new TIskraKeybDevice(0,Cfg->Peripherals.Keyboard.Port);
            Keyboard->ChangeEvent = CCNETStateChange;
            Keyboard->StartDevice();
        }
        if(Cfg->Peripherals.Keyboard.Type == "ktek") {
            Keyboard = new TKtekKeybDevice(0,Cfg->Peripherals.Keyboard.Port);
            Keyboard->ChangeEvent = CCNETStateChange;
            Keyboard->StartDevice();
        }
        if(scannerDevice)
            scannerDevice->DeviceState->StateChange = true;

        if(CoinAcceptor)
            CoinAcceptor->DeviceState->StateChange=true;
        else
            FileMap->SetWCState(cnCoinAcceptorError);

        if (Validator)
            Validator->DeviceState->StateChange=true;
        else
            FileMap->SetWCState(cnValidatorError);

        if (Printer)
            Printer->DeviceState->StateChange=true;
        if (Keyboard)
            Keyboard->DeviceState->StateChange=true;
        if (CardReader)
            CardReader->DeviceState->StateChange=true;
        if (WatchDog)
            WatchDog->DeviceState->StateChange=true;
        SetTopWindow(Handle);
        DeviceStateChanged();
        startup = true;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

bool TForm1::AlreadyRunning(void)
{
    bool bAlreadyRunning=false;
    try
    {
        HANDLE hMutexOneInstance = ::CreateMutex( NULL, TRUE,AnsiString("WEBCLIENTAPPLICATION-088FA840-B10D-11D3-BC36-006067709674").c_str());
        bAlreadyRunning = (GetLastError() == ERROR_ALREADY_EXISTS);
        if (bAlreadyRunning)
        {
            if (hMutexOneInstance != NULL)
            {
                ReleaseMutex(hMutexOneInstance);
                hMutexOneInstance = NULL;
            }
            Sleep(3000);
            HANDLE hMutexOneInstance = ::CreateMutex( NULL, TRUE,AnsiString("WEBCLIENTAPPLICATION-088FA840-B10D-11D3-BC36-006067709674").c_str());
            bAlreadyRunning = (GetLastError() == ERROR_ALREADY_EXISTS);
        }
        if (hMutexOneInstance != NULL)
        {
            ReleaseMutex(hMutexOneInstance);
            hMutexOneInstance = NULL;
        }
        return bAlreadyRunning;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return bAlreadyRunning;
    }
}

//---------------------------------------------------------------------------

void __fastcall TForm1::processTimer(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);

    if((!Cfg->CDebug.explorerProcess) && findProgramm("explorer.exe") && isExplorerMayKill)
        killProgramm("explorer.exe", Log);

    if (findProgramm("userinit.exe"))
        killProgramm("userinit.exe", Log);

    if ((TDateTime::CurrentDateTime() > FileMap->ConnTimeMark + double(5)/60/60/24) && Cfg->CDebug.connProcess)
    {
        if (FileMap->ConnTimeMark == TDateTime(0))
        {
            Log->Write("Trying to start Conn...");
            RunConn();
        }
        else if(!findProgramm("conn.exe"))
        {
            Log->Write((boost::format("Conn timeout %1%") % DateTimeToString(FileMap->ConnTimeMark)).str().c_str());
            Log->Write("Trying to restart Conn...");
            TerminateConn();
            RunConn();
        }
        FileMap->PutConnTimeMark();
    }
}

//---------------------------------------------------------------------------

void __fastcall TForm1::PaymentTimeOutTimerTimer(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);
    PaymentTimeOutTimer->Enabled=false;
    Log->Write("Payment TimeOut, finishing...");
    int prev_vld_disable_sum = Payment->Sum;
    if (Validator)
    {
      try
      {
        if (!Validator->DeviceState->Idle)
        {
          Validator->DisableBill();
          Log->Write("Payment timeout, Validator bill disabled.");
        }
      }
      catch(...)
      {
        Log->Write("Payment timeout, Validator disabling error!");
      }
    }

    if ((Payment) && (prev_vld_disable_sum != Payment->Sum))
    {
      PaymentTimeOutTimer->Enabled=false;
      PaymentTimeOutTimer->Enabled=true;
      Log->Write("Payment timeout: next loop");
    }
    else
    {
      FinishPayment(false);
      FinishPaymentInitiated = false;
    }
    if ((!Validator) || ((Validator) && (Validator->ErrorMode != DSE_OK)))
      FileMap->SetWCState(cnValidatorError);

}

//---------------------------------------------------------------------------

void TForm1::RenameTempFiles(AnsiString Dir)
{
AnsiString NewFileName, TempFileName;
try
	{
	TSearchRec sr;
	int iAttributes = 0;
	if (FindFirst(Dir+"\\*.tmp", iAttributes, sr) == 0) {
		Log->Write((boost::format("Checking folder %1% for temp packets...") % Dir.c_str()).str().c_str());
		do {
			if (FileMap)
				FileMap->PutWCTimeMark();
			TempFileName=Dir+"\\"+sr.Name;
			NewFileName=TempFileName.SubString(0,TempFileName.Length()-3)+"pkt";
			if (FileSizeByName(TempFileName)==0)
				{
				Log->Write((boost::format("Null-sized file %1% found, trying to delete...") % TempFileName.c_str()).str().c_str());
				if (!DeleteFile(TempFileName))
					Log->Append("Error!");
					else
					Log->Append("OK.");
				}

			if (RenameFile(TempFileName,NewFileName))
				{
				Log->Write((boost::format("Temporary file %1% renamed to %2%.") % TempFileName.c_str() % NewFileName.c_str()).str().c_str());
				}
			else
				{
				Log->Write((boost::format("Cannot rename temporary file %1% to %2%.") % TempFileName.c_str() % NewFileName.c_str()).str().c_str());
				continue;
				}
			} while (FindNext(sr) == 0);
		FindClose(sr);
		}
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void __fastcall TForm1::StartAppTimerTimer(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);
    StartApp();
}

//---------------------------------------------------------------------------

void __fastcall TForm1::StartApp()
{
  TPacketSender *PS = NULL;
  try
	{
    try
    {
      StartAppTimer->Enabled=false;
      if (Cfg->StatInfo.ProcessorType==cnCyberPlatServer)
        PS = new TCSPacketSender("", Cfg, Log, FileMap);
      else
        PS = new TSSPacketSender("", Cfg, Log, FileMap);
      if (PS)
      {
        PS->CreatePacket("out-version");
        PS->StoreError(TDateTime::CurrentDateTime(), "Version", 0, FileVersion.c_str(), 0, "");
        //PS->Clear();
      }

      if ((PS)&&(InfoFile))
      {
        //    Log->Write("Cfg->ConfigMD5: "+Cfg->ConfigMD5+" InfoFile->Read ConfigMD5"+InfoFile->Read("ConfigInfo","ConfigMD5")+".");
        if (Cfg->ConfigMD5!=InfoFile->Read("ConfigInfo","ConfigMD5").c_str())
        {
          Log->Write("Config.xml changed, sending to server...");
          PS->StoreFileSend((Cfg->Dirs.WorkDir+"config\\config.xml").c_str());
          InfoFile->Write("ConfigInfo","ConfigMD5",Cfg->ConfigMD5.c_str());
          //PS->Clear();
        }
        //    Log->Write("Cfg->OperatorsMD5: "+Cfg->OperatorsMD5+" InfoFile->Read OperatorsMD5"+InfoFile->Read("ConfigInfo","OperatorsMD5")+".");
        if (Cfg->OperatorsMD5!=InfoFile->Read("ConfigInfo","OperatorsMD5").c_str())
        {
          Log->Write("Operators.xml changed, sending to server...");
          PS->StoreFileSend((Cfg->Dirs.WorkDir+"config\\operators.xml").c_str());
          InfoFile->Write("ConfigInfo","OperatorsMD5",Cfg->OperatorsMD5.c_str());
        }
      }

    //InitFinished=true;
    //return;

    //		InterfaceOK = false;
    //		InterfaceOKTimer->Enabled=true;
      FileMap->DebugState = Cfg->CDebug.PeripheralsState;
      Log->Write("Initializing hardware...");
      HardwareInit();
      Log->Write("Starting check process...");
      CheckTimeProc();
      if (Cfg->SMSInfo.SendStartUpSMS)
      {
        SendingSMSAllowed = true;
        SendSMSNotification();
      }
      Log->Write("Starting threads...");
      CheckThreads();

      Application->ProcessMessages();

      if ((FileMap)&&(FileMap->CheckWCState(cnTerminalErrorMode))) // Если терминал перешел в состояние ошибки, остаемся в нем...
      {
        Log->Write("Terminal is in error mode - navigating to the main page cancelled.");
      }
      else
      {
        Log->Write("Navigating to the main page...");
        GoToMainMenu();
      }

      Log->Write("Init completed.");
      InitFinished=true;
		}
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        }
    }
    __finally
    {
        if (PS)
            delete PS;
    }
}

//---------------------------------------------------------------------------


void __fastcall TForm1::CheckPrinterStateTimerTimer(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);
    try
    {
        if(Printer != NULL)
        {
            Printer->GetState();
            if ((Printer->DeviceState->OutStateCode == DSE_OK)||(Printer->DeviceState->OutStateCode == DSE_NEARENDPAPER))
                CheckPrinterStateTimer->Enabled=false;
        }
        //Log->Write("CheckPrinterStateTimerTimer: "+AnsiString(Printer.DeviceState->OutStateCode));
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

HANDLE TForm1::StartProgram(AnsiString ApplicationName)
{
try
	{
	Log->Write((boost::format("Starting %1%...") % ApplicationName.c_str()).str().c_str());
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.lpDesktop="WinSta0\\Default";

	//DWORD lpExitCode;
	if (!CreateProcess((LPCTSTR)ApplicationName.c_str(), NULL,
					NULL,             // Process handle not inheritable.
					NULL,             // Thread handle not inheritable.
					true,            // Set handle inheritance to FALSE.
					0,//CREATE_NEW_CONSOLE|CREATE_DEFAULT_ERROR_MODE|CREATE_NEW_PROCESS_GROUP,                // No creation flags.
					NULL,             // Use parent's environment block.
					NULL,             // Use parent's starting directory.
					&si,              // Pointer to STARTUPINFO structure.
					&pi))
		{
		Log->Append(ShowError("Starting program error").c_str());
		return NULL;
		}
	else
		{
		Log->Append("OK.");
		return pi.hProcess;
		}
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return NULL;
    }
}

void TForm1::RunExplorer()
{
    try
    {
        if (!findProgramm("explorer.exe"))
        {
            int bufferSize = 1500;
            char* buffer = new char [bufferSize];
            std::string result = "C:\\Windows";

            if(GetEnvironmentVariable("windir", buffer, bufferSize))
                result = buffer;
            delete [] buffer;
            boost::replace_all(result, "\\\\", "\\");

            StartProgram((result + "\\explorer.exe").c_str());
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void __fastcall  TForm1::RunConn()
{
    try
    {
        ConnHandle = StartProgram((Cfg->Dirs.WorkDir+"conn\\conn.exe").c_str());
        if (ConnHandle)
        {
            HANDLE wP;
            wP = OpenProcess(PROCESS_ALL_ACCESS, true, GetCurrentProcessId());
            if (wP==NULL)
                Log->Write(ShowError("Error getting handle").c_str());

            HANDLE xP;
            if (!DuplicateHandle(wP, wP, ConnHandle, &xP, PROCESS_ALL_ACCESS, true, DUPLICATE_SAME_ACCESS))
                Log->Write(ShowError("Error duplicating handle").c_str());
            else
                FileMap->WriteWCHandle((int) xP);

            CloseHandle(wP);
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

bool TForm1::IsProcessRunning(HANDLE PrHandle)
{
    try
    {
        ULONG ExitCode;
        if (!GetExitCodeProcess(PrHandle,&ExitCode))
        {
            //Log->Write(ShowError("GetExitCodeProcess error"));
        }
        else
        {
            //Log->Write("ExitCode = "+AnsiString(ExitCode));
            if (ExitCode != STILL_ACTIVE)
            {
                PROCESSENTRY32 process;
                HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
                if(INVALID_HANDLE_VALUE != snap)
                {
                    process.dwSize = sizeof(PROCESSENTRY32);
                    if(Process32First(snap, &process))
                    {
                        while(Process32Next(snap, &process))
                        {
                            std::string str(process.szExeFile);
                            if(std::string::npos != str.find("conn.exe"))
                            {
                                HANDLE hd = OpenProcess(MAXIMUM_ALLOWED, FALSE, process.th32ProcessID);
                                DWORD lastError = GetLastError();
                                Log->Write((boost::format("OpenProcess, last error: %1%") % lastError).str().c_str());
                                if (0 == lastError)
                                {
                                    TerminateProcess(hd, NO_ERROR);
                                    Log->Write((boost::format("TerminateProcess, last error: %1%") % GetLastError()).str().c_str());
                                }
                                else
                                {
                                    hd = OpenProcess(MAXIMUM_ALLOWED, TRUE, process.th32ProcessID);
                                    lastError = GetLastError();
                                    Log->Write((boost::format("OpenProcess inherit, last error: %1%") % lastError).str().c_str());
                                    if (0 == lastError)
                                    {
                                        TerminateProcess(hd, NO_ERROR);
                                        Log->Write((boost::format("TerminateProcess, last error: %1%") % GetLastError()).str().c_str());
                                    }
                                }

                                if (0 != hd)
                                    CloseHandle(hd);
                            }
                        }
                    }
                    CloseHandle(snap);
                }

                return false;
            }
        }
        return true;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

bool TForm1::DeleteDir(AnsiString DirName)
{
try
  {
  Log->Write((boost::format("  Deleting %1%...") % DirName.c_str()).str().c_str());
  char From[MAX_PATH];
  ZeroMemory(From, sizeof(From));
  strcat(From,DirName.c_str());
  strcat(From,"\0\0");
  SHFILEOPSTRUCT op;
  ZeroMemory(&op, sizeof(op));
  op.hwnd = Application->Handle;
  op.wFunc = FO_DELETE;
  op.pFrom = From;
  op.fFlags = FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_NOERRORUI|FOF_SILENT;
  if (!SHFileOperation(&op))
    {
    Log->Append(" OK.");
    return true;
    }
    else {
    Log->Append(ShowError(" Error! ").c_str());
    return false;
    }
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

void __fastcall TForm1::CppWebBrowser1BeforeNavigate2(TObject *Sender,
			LPDISPATCH pDisp, Variant *URL, Variant *Flags,
			Variant *TargetFrameName, Variant *PostData, Variant *Headers,
			VARIANT_BOOL *Cancel)
{
  std::string CurrentLocationString = "";
  std::string NewLocationString = "";

  try
  {
    UNREFERENCED_PARAMETER(Sender);
    UNREFERENCED_PARAMETER(pDisp);
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(TargetFrameName);
    UNREFERENCED_PARAMETER(PostData);
    UNREFERENCED_PARAMETER(Headers);

    CurrentLocationString = WCharToString(CppWebBrowser1->LocationURL);
    NewLocationString = WCharToString(URL->VOleStr);

    std::auto_ptr <TLocationParser> CurrentLocation ( new TLocationParser(CurrentLocationString.c_str()));
    std::auto_ptr <TLocationParser> NewLocation ( new TLocationParser(NewLocationString.c_str()));

    if((!((NewLocation->PageName == "command.html") &&
          (NewLocation->Parameters.find("cmd") != NewLocation->Parameters.end()) &&
          (NewLocation->Parameters["cmd"] == "playsound"))) && (Cfg->CDebug.Logs.url))
    {
      Log->Write((boost::format("CurrentLocation = %1%") % (Cfg->CDebug.Logs.full ? CurrentLocationString : CurrentLocation->getURL())).str().c_str());
      Log->Write((boost::format("----NewLocation = %1%") % (Cfg->CDebug.Logs.full ? NewLocationString : NewLocation->getURL())).str().c_str());
    }

    std::string loc = GetLocale(NewLocation);
    if(!loc.empty())
    {
        Localization.SetLocale(loc.c_str(), Cfg);
        Localization.Load();
    }

    //zh_sp
    if  ((NewLocation->PageName == "main.html") || ((NewLocation->PageName=="pb.html") && (NewLocation->GetParameter("pb") == "getpb")))
    {
      if (Cfg->Payments.rState > eNo)
      {
        if (Cfg->Payments.Rest)
          Log->Write((boost::format("User did`t implement rest = %1%")
            % Cfg->Payments.Rest).str().c_str());
        Cfg->Payments.rState = eNo;
        Cfg->Payments.Rest = 0;
      }
    }
    if(NewLocation->PageName == "data-entry.html")
      LastEntryURL = getFullPathURL(NewLocation->URL);
    //zh_sp

    //zh_pb
    if ((NewLocation->PageName=="pb.html") &&
        (NewLocation->GetParameter("pb") != "") &&
        (NewLocation->GetParameter("pb") != "login"))
    {
      Log->Write("PaymentBook...");
      try
      {
        PB->entered = true;
        pbook::PaymentBookInterfaceCommand icommand = tICommand.getICommand(NewLocation->GetParameter("pb"));
        switch (icommand)
        {
          case pbook::CheckLogin:
            PB->payment = false;
            PB->PrivateData.Nick = AnsiString(NewLocation->GetParameter("number").c_str());
            Navigate("pb.html?pb=loading");
            if (PB->CheckAccount())
            {
              NewLocation->SetParameter("pb", "enterpin");
              Navigate(AnsiString(NewLocation->getURL().c_str()));
            }
            else
            {
              if (PB->LastConnectionOK)
              {
                NewLocation->SetParameter("pb", "getpin");
                Navigate(AnsiString(NewLocation->getURL().c_str()));
              }
              else
                GoToMessage(-5);
            }
          break;
          case pbook::SendSmsPin:
            PB->PrivateData.Nick = AnsiString(NewLocation->GetParameter("number").c_str());
            Navigate("pb.html?pb=loading");
            PB->GetPassword();
            if (PB->LastConnectionOK)
            {
              NewLocation->SetParameter("pb", "login");
              Navigate(AnsiString(NewLocation->getURL().c_str()));
            }
            else
              GoToMessage(-6);
          break;
          case pbook::RestoreSmsPin:
            PB->PrivateData.Nick = AnsiString(NewLocation->GetParameter("number").c_str());
            Navigate("pb.html?pb=loading");
            PB->GetPassword();
            if (PB->LastConnectionOK)
            {
              NewLocation->SetParameter("pb", "login");
              Navigate(AnsiString(NewLocation->getURL().c_str()));
            }
            else
              GoToMessage(-6);
          break;
          case pbook::CheckAccount:
            PB->payment = false;
            PB->PrivateData.Nick = AnsiString(NewLocation->GetParameter("number").c_str());
            PB->PrivateData.Password = AnsiString(NewLocation->GetParameter("pin").c_str());
            Navigate("pb.html?pb=loading");
            if (PB->Enter(false))
              Navigate("pb.html?pb=getpb");
            else
              if (PB->LastConnectionOK)
                Navigate("pb.html?pb=login&error=badpin");
              else
                GoToMessage(-5);
          break;
          case pbook::ShowRecords:
            PB->payment = false;
          break;
          case pbook::Checking:
          {
            if (!PB->payment)
            {
              boost::replace_all(NewLocationString, "checking", "update");
              Navigate("pb.html?pb=loading");
              PB->Change(NewLocationString);
              if (PB->LastConnectionOK)
              {
                boost::replace_all(NewLocationString, "update", "checking");
                boost::replace_all(NewLocationString, "pb.html", "checking.html");

                if(PaymentInitThread)
                {
                    PaymentInitThread->Terminate();
                    delete PaymentInitThread;
                }
                PaymentInitThread = new TFunctionThread<AnsiString>(NewLocationString.c_str());
                PaymentInitThread->execFunction = std::bind1st(std::mem_fun(&TForm1::Payment_Init), Form1);
                PaymentInitThread->Resume();
              }
              else
                GoToMessage(-6);
            }
          }
          break;
          case pbook::Payment:

          break;
          case pbook::AllThanks:

          break;
          case pbook::Add_rec:
          {
            Navigate("pb.html?pb=loading");
            PB->Add(NewLocationString);
            if (PB->LastConnectionOK)
              Navigate("pb.html?pb=getpb");
            else
              GoToMessage(-6);
          }
          break;
          case pbook::Delete_rec:
            Navigate("pb.html?pb=loading");
            PB->Delete(NewLocationString);
            if (PB->LastConnectionOK)
              Navigate("pb.html?pb=getpb");
            else
              GoToMessage(-6);
          break;
          case pbook::Change_rec:
            Navigate("pb.html?pb=loading");
            PB->Change(NewLocationString);
            if (PB->LastConnectionOK)
              Navigate("pb.html?pb=getpb");
            else
              GoToMessage(-6);
          break;
        }
      }
      catch(...)
      {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
      }
    }
    //zh_pb

    if(scannerDevice->IsWaitingData)
    {
        if(NewLocation->PageName == "checking.html")
            scannerDevice->StopWaitData();
        if(NewLocation->PageName == "main.html" && CurrentLocation->PageName == "data-entry.html")
            scannerDevice->StopWaitData();
    }

    // TODO: insert cardReader Code here
    if(cardReaderDevice->IsWaitingData)
    {
        Log->Write("cardReader: IsWaitingData");
        if(NewLocation->PageName == "checking.html")
        {
            cardReaderDevice->StopWaitData();
            Log->Write("cardReader: stopWaitData");
        }
        if(NewLocation->PageName == "main.html" && CurrentLocation->PageName == "data-entry.html")
        {
            cardReaderDevice->StopWaitData();
            Log->Write("cardReader: stopWaitData");
        }
    }

    // НАЛОГИ
    if(((NewLocation->PageName == "Payment.html") ||
        ((NewLocation->PageName == "pb.html") && (CurrentLocation->Parameters["pb"] == "payment"))) &&
       ((CurrentLocation->PageName != "addinfo.html") ||
        (!((CurrentLocation->PageName == "pb.html") && (CurrentLocation->Parameters["pb"] == "error") && (CurrentLocation->Parameters["errornumber"] == "showaddinfo")))) &&
       ((CurrentLocation->PageName != "Payment.html") ||
        (!((NewLocation->PageName == "pb.html") && (CurrentLocation->Parameters["pb"] == "payment")))))
    {
        if(AnsiString(Cfg->Operator(GetInt(AnsiString(NewLocation->GetParameter("recepient").c_str()))).ProcessorType.c_str()).LowerCase()=="cyberplat_taxes")
        {
            AnsiString locationString = AnsiString(URL->VOleStr);
            AnsiString Parameters=locationString.SubString(locationString.Pos("?")+1,locationString.Length());
            if (PB->entered)
              Navigate("pb.html?pb=error&errornumber=showaddinfo&" + Parameters);
            else
              Navigate("addinfo.html?" + Parameters);
        }
    }

    if (NewLocation->PageName=="mt.html")
    {
        AnsiString recepient=NewLocation->GetParameter("recepient").c_str();
        TLocationParser tlc=TLocationParser(AnsiString(URL->VOleStr).c_str());
        //savedLocation=NewLocation->URL.c_str();
        savedLocation = AnsiString(URL->VOleStr).c_str();
        bool navigate=isNavigateLogicMoneyTransfer(tlc);
        if(navigate)
        {
            Navigate(("checking.html?recepient="+NewLocation->GetParameter("recepient")+"&end=1&forced=1").c_str());
            *Cancel = true;
            return;
        }
        savedLocation="";
        *Cancel = false;
        return;
    }
    if ((NewLocation->PageName == "Payment.html") && (NewLocation->Parameters["end"] == "1"))
    {
        TMoneyTransferPayment* tmPayment=dynamic_cast<TMoneyTransferPayment*>(Payment);
        if(tmPayment && tmPayment->Sum!=0)
        {
            Navigate((boost::format("PaymentComplete.html?recepient=%1%&recepient_mt=%2%&processing=1&end=1")
              % NewLocation->GetParameter("recepient")
              % NewLocation->GetParameter("recepient_mt")).str().c_str());
            *Cancel = false;
            return;
        }
    }
    if (NewLocation->PageName=="checking.html" && NewLocation->HasParameter("recepient_mt"))
    {
        if (NewLocation->HasParameter("state"))
        {
            if(CurrentLocationString == NewLocationString)
            {
                *Cancel = true;
                return;
            }
            Navigate(NewLocation->URL.c_str());
            *Cancel = false;
            return;
        }
        if (NewLocation->Parameters["forced"]=="0")
        {
            *Cancel = false;
            return;
        }
        NewLocation->Parameters["forced"]="0";
        Navigate(NewLocation->URL.c_str());
        *Cancel = true;
        return;
    }
    if ((CurrentLocation->PageName=="checking.html")&&(NewLocation->PageName=="main.html"))
    {
        //AnsiString Forced = GetParameter("forced",NewLocationString);

        if (NewLocation->HasParameter("end") && NewLocation->Parameters["forced"]=="1")
        {
            *Cancel = false;
            return;
        }
        *Cancel = true;
        Log->Write("Changing of URL by the interface cancelled.");
        return;
    }

    if ((CurrentLocation->PageName=="data-entry.html")&&(NewLocation->PageName=="main.html"))
    {
        if (Payment)
            Payment->CancelPaymentReq = true;
        return;
    }


    if (((NewLocation->PageName=="main.html") || (NewLocation->PageName=="main_selection.html"))&&(Payment)&&(!FinishPaymentInitiated))
    {
      if(Payment->PayProcessStarted)
      {
        *Cancel = true;
        Log->Write("Unfinished payment found, finishing...");
        FinishPayment(true);
        FinishPaymentInitiated = false;
        if ((Validator->ErrorMode != DSE_OK) && (!(Cfg->Payments.rState)))
          FileMap->SetWCState(cnValidatorError);
        //GoToMainMenu();
      }
      else
      {
        *Cancel = false;
        Log->Write("Error: Unfinished payment found without PayProcessStarted, deleting Payment...");
        delete Payment;
        Payment = NULL;
        //GoToMainMenu();
      }
    }

    if (NewLocation->PageName=="command.html")
    {
        *Cancel = true;
        //AnsiString Command = GetParameter("cmd",NewLocationString);
        if ((NewLocation->Parameters["cmd"]=="playsound")&&(Sound))
        {
            //AnsiString Filename = GetParameter("parm",NewLocationString);
            //if (Sound)
            Sound->Play(NewLocation->GetParameter("parm").c_str());
        }
    }
    if ((CurrentLocation->PageName!="service.html")&&(NewLocation->PageName=="service.html"))
    {
        //AnsiString Menu = GetParameter("menu",NewLocationString);
        //if (Menu!="")
        if (NewLocation->HasParameter("menu"))
        {
            *Cancel = false;
            return;
        }
        int GetKeyRQ = GetKeyRequestsCount();
        if ((Cfg->ServiceInfo.ServiceMenuPasswordMask!="")||(Cfg->ServiceInfo.ServiceShortMenuPasswordMask!=""))
        {
            Form2->Init(Log, 60000);
            Form2->ShowForm("\nEnter password:",(Cfg->ServiceInfo.ServiceMenuPasswordMask+"|"+Cfg->ServiceInfo.ServiceShortMenuPasswordMask).c_str(), false);
            //if (Keyboard)
            //    Keyboard->ParentWindow=Form2->Handle;
            SetTopWindow(Form2->Handle);
            WaitForForm2Action();
            Form2->Close();
            //if (Keyboard)
            //    Keyboard->ParentWindow=Handle;
            SetTopWindow(Handle);
            Form2->EnableNumPad(true);

            if (Form2->Result)
            {
                *Cancel=true;
                Navigate((StripFileName(NewLocationString) + "&menu=" + boost::lexical_cast<std::string>(Form2->Result) + ((GetKeyRQ > 0) ? "&gkbshow=" + boost::lexical_cast<std::string>(GetKeyRQ) : std::string())).c_str());
            }
            else
            {
                Log->Write("Service menu password not entered!");
                *Cancel=true;
                Navigate("main.html?forced=1&end");
            }
        }
        else
        {
            Log->Write("Service menu password is empty.");
            *Cancel=true;
            Navigate((StripFileName(NewLocationString) + "&menu=0" + ((GetKeyRQ > 0) ? "&gkbshow=" + boost::lexical_cast<std::string>(GetKeyRQ) : std::string())).c_str());
        }
        return;
    }

    if ((CurrentLocation->PageName=="service.html")&&(NewLocation->PageName=="service.html"))
    {
        *Cancel = false;    // возможно поможет при первом заходе на страницу, когда команда не задана
        
        AnsiString Command = NewLocation->GetParameter("cmd").c_str();
        if ((Command=="1")||(NewLocationString.find("service.html?1") != std::string::npos))                      // инкассация
        {
            *Cancel = true;
            if (Log != NULL)
                Log->Write("Service menu command: Incassation.");
            PerformIncassation();
            return;
        }

        if ((Command=="2")||(NewLocationString.find("service.html?2") != std::string::npos))// перезагрузка
        {
            *Cancel = true;
            if (Log != NULL)
                Log->Write("Service menu command: Reboot.");
            Form2->Init(Log, 60000);
            Form2->ShowForm(Localization["reloadterminalq"]);
//            Form2->ShowForm("\nПерезагрузить терминал?");
            //if (Keyboard)
            //  Keyboard->ParentWindow=Form2->Handle;
            SetTopWindow(Form2->Handle);
            WaitForForm2Action();
            Form2->Close();
            //if (Keyboard)
            //  Keyboard->ParentWindow=Handle;
            SetTopWindow(Handle);
            Form2->EnableNumPad(true);
            if (Form2->Result)
                Reboot();
            else
                Log->Write("\"No\" button pressed - command's been aborted.");
            return;
        }

        if ((Command=="3")||(NewLocationString.find("service.html?3") != std::string::npos))// проверка связи с сервером мониторинга
        {
            *Cancel = true;
            if (Log != NULL)
                Log->Write("Service menu command: Check connection to the monitoring server.");
            CheckConnectionToMonitoringServer();
            return;
        }

        /* if ((Command=="4")||(NewLocationString.Pos("service.html?4")))// проверка связи с CyberPlat
        {
            *Cancel = true;
            if (Log != NULL)
                Log->Write("Service menu command: Check connection to the payment system.");
            CheckConnectionToCyberplat();
            return;
        }*/

        if ((Command=="5")||(NewLocationString.find("service.html?5") != std::string::npos))// Выход
        {
            *Cancel = true;
            if (Log != NULL)
                Log->Write("Service menu command: Close.");
            Form2->Init(Log, 60000);
            Form2->ShowForm(Localization["exitq"]);
//            Form2->ShowForm("\nВыйти из программы?");
            //if (Keyboard)
            //  Keyboard->ParentWindow=Form2->Handle;
            SetTopWindow(Form2->Handle);
            WaitForForm2Action();
            Form2->Close();
            //if (Keyboard)
            //  Keyboard->ParentWindow=Handle;
            SetTopWindow(Handle);
            Form2->EnableNumPad(true);
            if (Form2->Result)
            {
                Close();
            }
            else
                Log->Write("\"No\" button pressed - command's been aborted.");
            return;
        }

        if ((Command=="6")||(NewLocationString.find("service.html?6") != std::string::npos))// баланс
        {
            *Cancel = true;
            if (Log != NULL)
                Log->Write("Service menu command: Get validator balance cheque.");
            if (Printer != NULL)
                PerformBalance();
            return;
        }

        if ((Command=="7")||(NewLocationString.find("service.html?7") != std::string::npos))// z-отчет
        {
            *Cancel = true;
            if (Log != NULL)
                Log->Write("Service menu command: Print Z-Report.");
            Form2->Init(Log, 60000);
            if (Printer)
            {
                if (Printer->IsFiscal())
                    Form2->ShowForm(Localization["printzreportq"]);
//                    Form2->ShowForm("\nНапечатать z-отчет?");
                else
                  Form2->ShowMessage(Localization["cantprintzreport1"] ,40);
//                  Form2->ShowMessage("\nПечать z-отчета невозможна. Установлен нефискальный принтер!",40);
            }
            else
            {
                  Form2->ShowMessage(Localization["cantprintzreport2"] ,40);
//                Form2->ShowMessage("\nПечать z-отчета невозможна. Принтер не установлен!",40);
            }

            WaitForForm2Action();
            if(Form2->Result != 0)
            {
                Form2->ShowMessage(Localization["printingzreport"],40);
//                Form2->ShowMessage("\n\nПроизводится печать z-отчета. Пожалуйста подождите...",40);
                Form2->PushState();
                Form2->DisableAll();
                Application->ProcessMessages();
                PrintZReport();
                Log->Write("Z-report printing done.");
                Form2->PopState();
            }
            else
                Log->Write("\"No\" button pressed - command's been aborted.");
            Form2->Close();
            //if (Keyboard)
            //  Keyboard->ParentWindow=Handle;
            SetTopWindow(Handle);
            Form2->EnableNumPad(true);
            return;
        }

        if ((Command=="8")||(NewLocationString.find("service.html?8") != std::string::npos))// printer paper reload (Setup form - old)
        {

            *Cancel = true;
            /*
            if (Log != NULL)
                Log->Write("Service menu command: Start service form.");
            ShowSetupForm();
            return;
            */
            if (Log != NULL)
                Log->Write("Service menu command: Printer paper reload");
            PerformPaperReload();
            return;
        }
        
        if ((Command=="9")||(NewLocationString.find("service.html?9") != std::string::npos))// minimize
        {
            isExplorerMayKill = false;
            RunExplorer();
            *Cancel = true;
            if (Log != NULL)
                    Log->Write("Service menu command: Minimize.");
            Application->Minimize();
            return;
        }

        if (Command=="10")// Print All Unprinted Cheques
        {
            *Cancel = true;
            int CHQCount = InfoFile->GetPaymentsInfoCount("Program");
            Form2->Init(Log, 60000);
            Form2->EnableNumPad(false);
            if ((Cfg->Peripherals.Printer.PrintUnprintedCheques)||(Cfg->Peripherals.Printer.SaveUnprintedCheques))
            {
                if (Log != NULL)
                    Log->Write((boost::format("Service menu command: Print %1% unprinted cheques.") % CHQCount).str().c_str());
                if (CHQCount>0)
                    Form2->ShowForm("");
                  //Form2->ShowForm((boost::format("\nСохранено %1% чека(ов). Напечатать?") % CHQCount).str().c_str());
                else
                    Form2->ShowMessage(Localization["nosavedreceipts"],40);
//                    Form2->ShowMessage("\nСохраненные чеки отсутствуют.",40);
            }
            else
            {
                if (Log != NULL)
                    Log->Write("Service menu command: Print unprinted cheques - \"save_unprinted_cheques\" disabled.");
                Form2->ShowMessage(Localization["receiptsaveoff"],40);
//                Form2->ShowMessage("\nСохранение ненапечатанных чеков отключено.",40);
            }
            //if (Keyboard)
            //  Keyboard->ParentWindow=Form2->Handle;
            SetTopWindow(Form2->Handle);
            WaitForForm2Action();
            Form2->Close();
            //if (Keyboard)
            //  Keyboard->ParentWindow=Handle;
            SetTopWindow(Handle);
            Form2->EnableNumPad(true);
            if ((Cfg->Peripherals.Printer.PrintUnprintedCheques)||(Cfg->Peripherals.Printer.SaveUnprintedCheques)&&(CHQCount>0))
                if (Form2->Result)
                    PrintAllUnprintedCheques();
                else
                    Log->Write("\"No\" button pressed - command's been aborted.");
            return;
        }

        if (Command=="11")// Start ProcessGenKeys
        {
            *Cancel = true;
            try
            {
                //ProcessGenKeys();
            }
            catch(...)
            {
                ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                Log->Write("Exception occured while peforming GenKey");
            }
            return;
        }


        if (Command=="12")
        {
            if(Log)
                Log->Write("starting Configurator.exe");

            StartProgram((Cfg->Dirs.WorkDir+"Configurator.exe").c_str());
//            exit(1);
            Close();
            return;
        }
        //Log->Write("Changing of URL by the interface cancelled.");
        return;
    }

    if (NewLocation->PageName=="terminal_error.html")
    {
        if (FileMap)
            FileMap->WCIdle=true;
        if (NewLocation->HasParameter("ifet"))
        {
            *Cancel = true;
            Log->Write((boost::format("Interface error #%1% found: %2%.") % NewLocation->Parameters["ifet"].c_str() % NewLocation->Parameters["ifem"].c_str()).str().c_str());
            if (FileMap)
            {
                FileMap->SetWCState(cnTerminalInternalBlock);
                CheckTerminalState();
            }

            if (TDateTime::CurrentDateTime()>ProgramStartDT+cnInterfaceErrorTimeOut)
            {
                if (Cfg->Terminal.RebootAllowed)
                {
                    Log->Write("Trying to reboot...");
                    Reboot();
                }
                else
                {
                    Log->Write("Can't reboot terminal - not allowed.");
                }
            }
        }
        return;
    }
    if (!(*Cancel))
        //  sndPlaySoundA(NULL, SND_ASYNC);
        if (Sound)
            Sound->StopAll();
  }
  catch(...)
  {
    Log->Write((boost::format("Browser has failed on loading page: from {%1%} to {%2%}")
      % CurrentLocationString
      % NewLocationString).str().c_str());
    ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

AnsiString TForm1::GetHWErrorDescription(int status)
{
AnsiString result = "neizvestny status";
switch (status) {
	case DSE_OK:
		result = "ok";
		break;
	case DSE_NOTMOUNT:
		result = "ne ustanovlen";
		break;
	case DSE_NOTPAPER:
		result = "net bumagi";
		break;
	case DSE_NEARENDPAPER:
		result = "bumaga zakanchivaetsya";
		break;
	case DSE_PAPERJAM:
		result = "bumaga zastryala";
		break;
/*	case DSE_MAINERROR:
		result = "nekriticheskaya oshibka";
		break;*/
	case DSE_STACKEROPEN:
		result = "kasseta snyata";
		break;
	case DSE_STACKERFULL:
		result = "kasseta perepolnena";
		break;
	case DSE_BILLJAM:
		result = "kupyura zastryala";
		break;
/*	case DSE_CHEATED:
		result = "vzlom";
		break;*/
	case DSE_BILLREJECT:
		result = "nevozmozhno vnesti kupyuru";
		break;
	case DSE_HARDWARE_ERROR:
		result = "kriticheskaya oshibka";
		break;
	case DSE_SETCASSETTE:
		result = "kasseta ustanovlena";
		break;
}
return result;
}

//---------------------------------------------------------------------------

void TForm1::EnterServiceMenu()
{
    try
    {
        Log->Write("Entering Service mode...");
        AnsiString TextParam;

        TextParam +=AnsiString(FileMap->BillsCount)+"|"+AnsiString(FileMap->BillsSum)+"|"+AnsiString(FileMap->ValidatorState)+"|"+AnsiString(FileMap->PrinterState);
        TextParam +="|"+AnsiString(FileMap->UnprocPaymentsCount)+"|"+AnsiString(FileMap->UnprocStatPacketsCount)+"|"+AnsiString(FileMap->SIMBalance)+"|"+AnsiString(FileMap->GSMSignalQuality);

        if (int(TDateTime::CurrentDateTime().Val)!=int(FileMap->LastPaymentReceived.Val))
        {
            AnsiString part = FileMap->LastPaymentReceived.FormatString("dd.mm hh:nn");
            if(part == "30.12 00:00")
                part = "--.-- --:--";
            TextParam += "|" + part;
        }
        else
        {
            TextParam += "|" + FileMap->LastPaymentReceived.FormatString("hh:nn");
        }

        if (int(TDateTime::CurrentDateTime().Val)!=int(FileMap->LastPaymentProcessed.Val))
        {
            AnsiString part = FileMap->LastPaymentProcessed.FormatString("dd.mm hh:nn");
            if(part == "30.12 00:00")
                part = "--.-- --:--";

            TextParam += "|" + part;
        }
        else
        {
            TextParam += "|" + FileMap->LastPaymentProcessed.FormatString("hh:nn");
        }

        Navigate("service.html?v="+AnsiString(FileVersion.c_str())+"&tnum="+AnsiString(Cfg->Terminal.Number.c_str())+"&asd="+TextParam);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Navigate("service.html");
    }
}

void __fastcall TForm1::LogError()
{
  if (LogErrorDone)
    return;
  LogErrorDone = true;
  if (FileMap)
  {
    FileMap->SetWCState(cnTerminalWriteError);
    CheckTerminalState();
  }
  LogWriteErrorFound = true;
}

void TForm1::ShutDown()
{
  Log->Write("Shutdown sequence started...");
  HANDLE hToken;
  TOKEN_PRIVILEGES tkp;

  OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken); // Get a token for this process.

  LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); // Get the LUID for the shutdown privilege.

  tkp.PrivilegeCount = 1;  // one privilege to set
  tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); // Get the shutdown privilege for this process.
                                                                             // Cannot test the return value of AdjustTokenPrivileges.
  ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0); //Shut down the system and force all applications to close.

  Log->Write("ExitWindows done...");
  Application->Terminate();
}

void TForm1::ShowSetupForm()
{
    return; // Из-за отсутствия дерева настроек
  Log->Write("Setup form started.");
  SetupForm->Cfg = Cfg;
  SetupForm->SetConfigTree();
  SetupForm->Show();
  while (!SetupForm->Finished)
  {
    Application->ProcessMessages();
    Sleep(10);
  }
  Log->Write("Setup form closed.");
}

void TForm1::SetTopWindow(HWND TopWindowHandle)
{
    if (Keyboard)
        Keyboard->ParentWindow=TopWindowHandle;
    ActiveWindow = TopWindowHandle;
}

void __fastcall TForm1::Timer1Timer(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);
    if (ActiveWindow == Handle)
    {
        SetWindowPos(Form2->Handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
        SetWindowPos(this->Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
    }
    else
    {
        SetWindowPos(this->Handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
        if (ActiveWindow == Form2->Handle)
            SetWindowPos(Form2->Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
    }
}
//---------------------------------------------------------------------------



void __fastcall TForm1::FormCreate(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);
}

void TForm1::saveDataToFile(const char* Data)
{
    std::string answer(Data);
    boost::replace_all(answer,"\n","[LF]");
    boost::replace_all(answer,"\r","[CR]");
    boost::replace_all(answer,"\t","[T]");

    std::ofstream ofs((Cfg->Dirs.InterfaceDir+"\\data.js").c_str());
    ofs << "function getData(){" << std::endl;
    ofs << "return '" << answer.c_str() << "';" << std::endl;
    ofs << "}" << std::endl;
}

//---------------------------------------------------------------------------
__fastcall TForm1::~TForm1()
{
    if (Log != NULL)
    {
            delete Log;
            Log=NULL;
    }
}

bool TForm1::checkRecipient(unsigned int operatorId)
{
 std::string serviceGuid = Cfg->Operator(operatorId).ServiceGuid;
 if(serviceGuid == "")
  return false;

 bool result = false;

 if(Cfg->Operator(operatorId).ServiceGuid == Moscow_JKH)
 {
  result = true;
 }
 else
 {
 }

 return result;
}

std::string TForm1::parseScannerData(unsigned int operatorId, std::string data, BYTE type)
{
 std::string serviceGuid = Cfg->Operator(operatorId).ServiceGuid;
 if(serviceGuid == "")
  return "";

 return CScannerDataParser::Parse(serviceGuid, data, type);
}

void TForm1::saveEmpty_iface_details()
{
    try
    {
        TJSMaker JSMaker;
        JSMaker.Tab="\t";
        JSMaker.Clear();
        JSMaker.AddString("$ga_jcfg['details'] = {");
        JSMaker.Level++;
        JSMaker.AddChild("pin_info");
        JSMaker.CloseChild();
        JSMaker.CloseChild(false);
        JSMaker.AddString("//  - - - - - всегда в true, для динамического include файла\n$iface_details_js = true;");

        std::ofstream ofs((Cfg->Dirs.InterfaceDir+"\\iface_details.js").c_str());
        ofs << JSMaker.Content << std::endl;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

std::string TForm1::GetLocale(std::auto_ptr <TLocationParser>& Location)
{
    std::string locale = Location->GetParameter("locale").c_str();
    return  (locale.length() == 0) ? Cfg->m_LocaleName : locale;
}

double paymentParameters::m_minshowsum = 0;

paymentParameters::paymentParameters(bool nominal)
{
  if (Form1->Payment)
  {
      double tempMax = Form1->Payment->GetLimMax();
      double tempFixMaxSum = my_round((tempMax + Form1->Payment->GetComission(tempMax))*100);
      m_cms = my_round(Form1->Payment->GetComission()*100);
      m_sum = my_round(Form1->Payment->Sum*100);

      TOpInfo tOper = Form1->Cfg->Operator(Form1->Payment->Recepient);
      std::string processorTypeLowerCase = tOper.ProcessorType;
      boost::to_lower(processorTypeLowerCase);
      bool is_processor_type_fix = (processorTypeLowerCase == "avia_center");
      bool is_operator_fix = (tOper.fix && (processorTypeLowerCase != "cyberplat_mt"));

      m_minsum = is_operator_fix ? tempFixMaxSum : my_round(Form1->Payment->GetPaymentMinSum()*100);
      m_fixsum = tempMax*100;
      if ((m_sum == 0) || ((!(Form1->Cfg->Payments.rState)) && (Form1->Payment->XMLP->vNotes.size() == 1)))
      {
          double full_minsum = m_minsum/100 + Form1->Payment->GetComission(m_minsum/100);
          TIntVector nominals;
          Form1->Cfg->strToIntVector(Localization["nominals"], nominals, "locale nominals");
          int index_min = 0;
          for (int i = 0; i < nominals.size(); i++)
          if((div(nominals[i], nominals[index_min]).rem) && (nominals[i] <= full_minsum))
          {
              i--;
              index_min++;
          }

          //если ступенчастая комиссия, смотрим, нет ли выше по диапазонам суммы,
          //меньше полученной за счет понижения комиссии
          if(tOper.CommissionInfo.size() - 1)
          {
              //определяем диапазон ступенчатой комиссии, в котором мы находимся
              int indexMinComissionLevel = 0;
              for (int i = 0; i < (tOper.CommissionInfo.size() - 1); i++)
              {
                  if (tOper.CommissionInfo[i].Min <= (m_minsum/100))
                      indexMinComissionLevel = i + 1;
              }

              //ходим по диапазону, ищем минимальную полную сумму
              for (int i = indexMinComissionLevel; i < tOper.CommissionInfo.size(); i++)
              {
                  double  minLevelSum = tOper.CommissionInfo[i].Min;
                  double fullLevelSum = minLevelSum + Form1->Payment->GetComission(minLevelSum);
                  if (fullLevelSum < full_minsum)
                      full_minsum = fullLevelSum;
              }
          }
          m_minshowsum = (full_minsum + getMinLocaleRemainder(full_minsum, nominals, index_min))*100;
      }
      m_maxsum = is_operator_fix ? tempFixMaxSum : my_round(tempMax*100);
      m_paysum = (is_operator_fix || is_processor_type_fix) ? m_minsum : m_sum;
      m_is_payment_will_pass = (is_operator_fix || is_processor_type_fix) ? (m_sum >= m_minsum) : ((m_sum - m_cms) >= m_minsum);
      m_rest = ((is_operator_fix || is_processor_type_fix) && (m_sum > m_minsum)) ? (m_sum - m_maxsum) : 0;

      if(!nominal)
      {
        m_minsum /= 100;
        m_maxsum /= 100;
        m_cms /= 100;
        if (m_sum == 0)
            m_minshowsum /= 100;
        m_sum /= 100;
        m_paysum /= 100;
        m_rest /= 100;
      }

      m_s_minsum = (boost::format("%.2f") % m_minsum).str();
      m_s_fixsum = (boost::format("%.2f") % m_fixsum).str();
      m_s_maxsum = (boost::format("%.2f") % m_maxsum).str();
      m_s_cms    = (boost::format("%.2f") % m_cms).str();
      m_s_minshowsum = (boost::format("%.2f") % m_minshowsum).str();
      m_s_sum    = (boost::format("%.2f") % m_sum).str();
      m_s_paysum = (boost::format("%.2f") % m_paysum).str();
      m_s_rest   = (boost::format("%.2f") % m_rest).str();
      m_pstate   = "";
      if (((!(Form1->Validator)) || ((Form1->Validator) && (Form1->Validator->ErrorMode != DSE_OK))) && (!Form1->FileMap->CheckDebugState(cnValidatorError)))
        m_pstate += "vld";
      if (m_pstate == "")
        m_pstate = "ok";
  }
}

void paymentParameters::swap(const paymentParameters& rhs)
{
  this->m_minsum = rhs.m_minsum;
  this->m_maxsum = rhs.m_maxsum;
  this->m_cms    = rhs.m_cms;
  this->m_minshowsum = rhs.m_minshowsum;
  this->m_sum    = rhs.m_sum;
  this->m_paysum = rhs.m_paysum;
  this->m_rest   = rhs.m_rest;

  this->m_is_payment_will_pass = rhs.m_is_payment_will_pass;

  this->m_s_minsum = rhs.m_s_minsum;
  this->m_s_maxsum = rhs.m_s_maxsum;
  this->m_s_cms    = rhs.m_s_cms;
  this->m_s_minshowsum = rhs.m_s_minshowsum;
  this->m_s_sum    = rhs.m_s_sum;
  this->m_s_paysum = rhs.m_s_paysum;
  this->m_s_rest   = rhs.m_s_rest;
  this->m_pstate   = rhs.m_pstate;
}

paymentParameters::paymentParameters(const paymentParameters& ob)
{
  swap(ob);
}

paymentParameters& paymentParameters::operator=(const paymentParameters& ob)
{
  paymentParameters tmp(ob);
  swap(tmp);
  return *this;
}

//*******************************************************************************************//
//                 Функция для расчета остатка до минимально вносимой суммы,                 //
//      параметры:                                                                           //
//                                                                                           //
//  1. int                   sum           исходная сумма                                    //
//                                                                                           //
//  2. std::vector<int>&     nominals      вектор номиналов купюр                            //
//                                                                                           //
//  3. int                   index_min     минимальной индекс купюры в векторе номиналов,    //
//                                         которая кратна все большим ее купюрам             //
//                                                                                           //
//                                                                                           //
//  Возвращаемое значение    дополнение исходной суммы до минимально вносимой                //
//                                                                                           //
//  Функция рекурсивная, число шагов рекурсии не более 4-х                                   //
//*******************************************************************************************//
double paymentParameters::getMinLocaleRemainder(double sum, std::vector<int>& nominals, int index_min)
{
    sort(nominals.begin(), nominals.end());         //сортируем вектор номиналок купюр
    //если разность исходная сумма - приближенная сумма меньше этой дельты, считаем, что поиск приближенной суммы окончен
    int delta = nominals[0]/2;
    int min_simple_sum = nominals[0]*ceil(sum/nominals[0]);   //приближенная сумма, набранная самыми маленькими купюрами
    for (int i = 0; i < index_min + 1; i++)         //ходим по номиналам, пробуем набрать сумму купюрами текущего номинала
    {
        int temp_sum = nominals[i]*ceil(sum/nominals[i]);     //собственно, очередная приближенная сумма
        if(temp_sum <= min_simple_sum)              //сравниваем сумму с минимально похожей на исходную, если меньше -
            min_simple_sum = temp_sum;              //запоминаем ее
    }

    //число купюр, достоинства - минимально кратного всем большим (МКБ), необходимого для набора исходной суммы - 1,
    //например: МКБ-купюра = 500, исходная сумма 1740, min_index_count = 3
    int min_index_count = floor(sum/nominals[index_min]);
    //"остаток для анализа". это не остаток в чистом виде!!!
    //нам надо оставить для последующего анализа исходную сумму - сумму МКБ-купюр (которыми закроется сумма) - 2,
    //а если такая купюра 1, то 1. Например, МКБ-купюра = 500, исходная сумма = 1740, max_remainder = 740.
    int max_remainder = sum - nominals[index_min]*max((min_index_count - 1), 1);
    //сумма МКБ-купюр, которая покрывает исходную сумму
    int greater_sum = nominals[index_min]*ceil(sum/nominals[index_min]);

    //заканчиваем анализ, если:
    //1. остаток = сумма, набранная минимальными купюрами - исходная сумма = меньше дельты     ИЛИ
    //2. закончились купюры, меньше уже нет     ИЛИ
    //3. сумма, набранная МКБ-купюрами меньше дельты и количество этих купюр = 1
    if((!(((greater_sum - sum) >= delta) && min_index_count)) || !(index_min) || ((min_simple_sum - sum) < delta))
    {
        return min_simple_sum - sum;  //возвращаем остаток
    }
    //..., а если нет - продолжаем анализ:
    //сумма - это теперь "остаток для анализа"
    //тот же вектор с номиналами
    //МКБ-купюра. если разница "остатка для анализа" и текущего номинала МКБ-купюры меньше дельты, то оставляем номинал,
    //ели нет - понижаем. Очевидно, в первом случае остаток будет равен разнице минимальной купюры и новой суммы
    else
    {
        return getMinLocaleRemainder(max_remainder, nominals, ((max_remainder - nominals[index_min]) > delta) ? index_min : index_min - 1);
    }
}

//------------------------------------------------------------------------------

std::string TForm1::getBarCodeString(int _validator)
{
    /*
    std::string result = Cfg->BarCodeTemplate.templateStr;

    CMoneyCounter* _MoneyCounter = MoneyCounters.find(_validator)->second; //на обращение через [] идет ругань в недрах буста
    std::string filler = Cfg->BarCodeTemplate.filler;

    boost::replace_all(result, "terminal", fill(Cfg->Keys[0].AP, filler, Cfg->BarCodeTemplate.terminal));
    boost::replace_all(result, "cheque", fill(FileMap->ChequeCounter, filler, Cfg->BarCodeTemplate.cheque));
    boost::replace_all(result, "total", fill(_MoneyCounter->TotalMoney(), filler, Cfg->BarCodeTemplate.total));
    boost::replace_all(result, "validator", fill(_validator, filler, Cfg->BarCodeTemplate.validator));
    //********************************* дата и время *******************************************
    SYSTEMTIME systime;
    GetSystemTime(&systime);
    int time_zone = 3;  // +3 часа от Гринвича

    std::string _date = Cfg->BarCodeTemplate.date;
    if(_date.find("YYYY") != std::string::npos)
    {
        boost::replace_all(_date, "YYYY", fill(systime.wYear, filler, 4));
    }
    else
    {
        boost::replace_all(_date, "YY", fill(systime.wYear, filler, 2));
    }
    boost::replace_all(_date, "MM", fill(systime.wMonth, filler, 2));
    boost::replace_all(_date, "DD", fill(systime.wDay, filler, 2));

    std::string _time = Cfg->BarCodeTemplate.time;
    boost::replace_all(_time, "HH", fill(systime.wHour + time_zone, filler, 2));
    boost::replace_all(_time, "MM", fill(systime.wMinute, filler, 2));
    boost::replace_all(_time, "SS", fill(systime.wSecond, filler, 2));

    boost::replace_all(result, "date", _date);
    boost::replace_all(result, "time", _time);
    //***************************** нотесы *****************************************************
    std::string nominals = "";
    std::string counts = "";
    std::string nominals_counts = "";
    std::string counts_nominals = "";

    TNotesVector _notes = _MoneyCounter->Notes;
    for(int i = 0; i < _notes.size(); i++)
    {
        std::string _nominal = fill(_notes[i].Nominal, filler, Cfg->BarCodeTemplate.nominal);
        std::string _count   = fill(_notes[i].Count,   filler, Cfg->BarCodeTemplate.count);

        nominals += _nominal;
        counts   += _count;
        nominals_counts += _nominal + _count;
        counts_nominals += _count + _nominal;
    }
    boost::replace_all(result, "nominal", nominals);
    boost::replace_all(result, "count",   counts);
    boost::replace_all(result, "nominalcount", nominals_counts);
    boost::replace_all(result, "countnominal", counts_nominals);
    */
    std::string result = "";
    CMoneyCounter* _MoneyCounter = MoneyCounters.find(_validator)->second; //на обращение через [] идет ругань в недрах буста
    std::string filler = "0";

    result += fill(Cfg->Keys[0].AP, filler, 8);  //id терминала
    result += fill(_MoneyCounter->TotalMoney(), filler, 8);     //общая сумма
    result += fill(_MoneyCounter->TotalBill(), filler, 4);
    result += fill(Cfg->Terminal.ChequeCounter, filler, 3);

    return result;
}

//---------------------------------------------------------------------------

int TForm1::DeleteOldStatPackets(AnsiString a_work_directory)
{
    TStringVector deletedFiles;

    try
    {
        TSearchRec sr;
        int iAttributes = 0;

        //TStringVector DeviceSpecifications;
        _filesProperties device(cnError, "device packets", Form1->Cfg, Form1->Log);
        device.includeNode("sender_name", "printer");
        device.includeNode("sender_name", "validator");
        device.includeNode("sender_name", "coinacceptor");
        device.includeNode("sender_name", "watchdog");
        device.includeNode("sender_name", "cardreader");
        device.includeNode("sender_name", "keyboard");

        _filesProperties version(cnError, "version packets", Form1->Cfg, Form1->Log);
        version.getLastFileName(a_work_directory, version.includeNode("sender_name", "version"));

        _filesProperties files_transfer(cnFileSend, "files transfer packets", Form1->Cfg, Form1->Log);
        files_transfer.getLastFileName(a_work_directory, files_transfer.includeNode("filename", "config"));
        files_transfer.getLastFileName(a_work_directory, files_transfer.includeNode("filename", "operators"));

        _filesProperties unsended(cnNoType, "unsended configuration files", NULL);
        
        if (FindFirst(a_work_directory + "\\*.*", iAttributes, sr) == 0)
        {
            do
            {
                AnsiString FileName = a_work_directory + "\\" + sr.Name;
                if (GetExtName(FileName) == "pkt")
                {
                    std::auto_ptr<TStatPacket> packet (new TStatPacket(Cfg, Log));
                    if (packet->LoadFromFile(FileName))
                    {
                        if (device.isValidValue(packet->ErrSender.c_str()))
                        {
                            device.prepareToDeletingFile(FileName, packet.get());
                        }

                        if (version.isValidValue(packet->ErrSender.c_str()))
                        {
                            version.prepareToDeletingFile(FileName, packet.get());
                        }

                        if (files_transfer.isValidValue(packet->SendFileName.c_str()))
                        {
                            files_transfer.prepareToDeletingFile(FileName, packet.get());
                        }
                    }
                }
                else
                {
                    unsended.prepareToDeletingFile(FileName, NULL, "file");
                }
            }
            while (FindNext(sr) == 0);
            FindClose(sr);

            int totalFiles = device.getCount() +
                             version.getCount() +
                             files_transfer.getCount() +
                             unsended.getCount();

            int totalDeletedFiles = device.deletingFiles() +
                                    version.deletingFiles() +
                                    files_transfer.deletingFiles() +
                                    unsended.deletingFiles();

            Log->Write(("\nDeleting old statistic files from " + a_work_directory + ":").c_str());

            Log->Append(getDeleteStatFilesString(&device).c_str());
            Log->Append(getDeleteStatFilesString(&version).c_str());
            Log->Append(getDeleteStatFilesString(&files_transfer).c_str());
            Log->Append(getDeleteStatFilesString(&unsended).c_str());
            Log->Append("\n----------------------------------------------------");
            Log->Append(getDeleteStatFilesString(NULL, totalFiles, totalDeletedFiles).c_str());

            return totalDeletedFiles;                  
        }
        else
        {
            Log->Write(("\nDeleting old statistic files from " + a_work_directory + ":").c_str());
            Log->Append(getDeleteStatFilesString(NULL).c_str());
            return 0;
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return -1;
    }
    return -1;
}


void __fastcall TForm1::ApplicationEvents1Restore(TObject *Sender)
{
    Log->Write("Restoring Main WebClient window...");
    isExplorerMayKill = true;    
}
//---------------------------------------------------------------------------

