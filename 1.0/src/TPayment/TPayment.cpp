//---------------------------------------------------------------------------

#include <classes.hpp>
#include <system.hpp>
#include <XMLDoc.hpp>
#include <SysUtils.hpp>
#include <IdHTTP.hpp>
#include <algorith.h>
#include <memory>
#include <boost\algorithm\string.hpp>
#include <boost\regex.hpp>
#include <boost\lexical_cast.hpp>
#include <boost\format.hpp>

#include "globals.h"
#include "TPayment.h"
#include "CryptLib2.h"
#pragma hdrstop

//---------------------------------------------------------------------------

#pragma package(smart_init)

//---------------------------------------------------------------------------
extern std::string LastEntryURL;
//---------------------------------------------------------------------------
TPayment::TPayment(AnsiString _fileName, TWConfig *_Cfg, TLogClass *_Log, TFileMap* _FileMap, TXMLInfo* _InfoFile)
{
    try
    {
        AddInfo="";
        PayProcessStarted = 0;
        PacketLoadError = false;
        PayProcessStarted = false;
        //CoInitializeEx(NULL, COINIT_MULTITHREADED);
        FileMap=_FileMap;
        Cfg = _Cfg;
        InfoFile = _InfoFile;

        XMLP = NULL;
        PS = NULL;
        InitPS = NULL;
        PaymentCreateDT = TDateTime::CurrentDateTime();

        InnerLog=false;
        if (_Log==NULL)
        {
            Log = new TLogClass("Payment");
            InnerLog=true;
        }
        else
        {
            Log=_Log;
        }
        ForcedSum=0;
        RetryAllowed = false;
        PostPaymentInfo = "";
        CancelPaymentReq = false;
        XMLP = new TPaymentPacket(Cfg, Log);
        XMLP->PaymentCreateDT = PaymentCreateDT;

        if(Cfg->StatInfo.ProcessorType == cnCyberPlatServer)
        {
            PS = new TCSPacketSender("", Cfg, Log, FileMap);
            InitPS = new TCSPacketSender("", Cfg, Log, FileMap);
        }
        else
        {
            PS = new TSSPacketSender("", Cfg, Log, FileMap);
            InitPS = new TSSPacketSender("", Cfg, Log, FileMap);
        }

        m_xmlParseError = false;        
        if ((_fileName!="")&&(XMLP))
        {
            if (!XMLP->LoadFromFile(_fileName))
            {
                PacketLoadError = true;
                m_xmlParseError = XMLP->getXmlParseError();
            }
            else
            {
                Session=XMLP->LastSession;
                Recepient=XMLP->OperatorId;
            }
        }
        IndyError=false;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        PacketLoadError=true;
    }
}

//---------------------------------------------------------------------------

TPayment::~TPayment()
{
    try
    {
        std::auto_ptr<TLocationParser> entryLocation(new TLocationParser(LastEntryURL.c_str()));
        std::string recipientStr = entryLocation->GetParameter("recepient");
        TOpInfo tOper;
        std::string field100_Id = "";
        if (recipientStr != "")
        {
          int tOperatorId = -1;
          try
          {
            tOperatorId = boost::lexical_cast<int>(recipientStr);
          }
          catch(...) {}
          if (tOperatorId != -1)
            tOper = Cfg->Operator(tOperatorId);
        }

        if (tOper.Properties.size())
        {
          std::string strFieldId = tOper.Properties[0].FieldId;
          boost::replace_all(strFieldId, "[#", "");
          boost::replace_all(strFieldId, "]", "");
          field100_Id = tOper.getFieldById(strFieldId).Id;
        }

        double tsum = 0;
        try
        {
          std::string tsumStr = entryLocation->GetParameter("sum");
          if (!tsumStr.empty())
            tsum = boost::lexical_cast<double>(entryLocation->GetParameter("sum"));
        }
        catch(...) {}

        if (((CheckErrorCode == db_errors::noValidPhone) ||
             (CheckErrorCode == db_errors::noOperatorCode) ||
             (CheckErrorCode == db_errors::noNumberInNumCapacity) ||
             ((!entryLocation->GetParameter("field100").empty()) &&
              (entryLocation->PageName == "data-entry.html") &&
              (tsum > 0))) && (!(Cfg->Payments.rState)))
          Cfg->Payments.Rest = Sum;
        else
          Cfg->Payments.rState -= 1;

        Clear();
        if (PS)
            delete PS;
        if (InitPS)
            delete InitPS;
        AnsiString FullPacketFileName = XMLP->PacketFileName.c_str();
        if (XMLP)
            delete XMLP;
        if (InnerLog)
            delete Log;

        //если остался временный файл платежа, удаляем его
        TSearchRec sr;
        int iAttributes = 0;
        AnsiString temp1 = GetExtName(FullPacketFileName);
        if((GetExtName(FullPacketFileName) == "tmp") && (FindFirst(FullPacketFileName, iAttributes, sr) == 0))
        {
          DeleteFile(FullPacketFileName);
          FindClose(sr);
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TPayment::Clear()
{
    ForcedSum=0;
    MessageText="";
    Session="";
    AFieldsForInterface="";
    AFieldsForCheque="";
    Sum=0;
}

//---------------------------------------------------------------------------

bool TPayment::Process()
{
    Log->Append((boost::format(" - %1% try.") % (XMLP->NumOfTries + 1)).str().c_str());
    try
    {
        if ((!XMLP)||(!PS))
            return false;

        if (PacketLoadError)
        {
            Log->Write("Payment processing error - packet not loaded!");
            return false;
        }

        if (!PS->CreatePacket())
        {
            Log->Write("Payment processing error - Error creating stat packet!");
            return false;
        }

        int Status=0;

        AnsiString processorType = AnsiString(Cfg->Operator(XMLP->OperatorId).ProcessorType.c_str()).LowerCase();
        if (processorType !="cyberplat" && processorType != "cyberplat_taxes")
        {
            if (XMLP->ResultExternal==-1)
            {
                Log->Write(("  Payment not processed - Unknown processor type: "+Cfg->Operator(XMLP->OperatorId).ProcessorType).c_str());
                XMLP->NumOfTries++;
                XMLP->NextTryDT=TDateTime::CurrentDateTime()+double(1)/24; // следующая попытка - через сутки
                return false;
            }
            else
            {
                Log->Write((boost::format("  Payment processed by an external processor %1% with error %2%.") % Cfg->Operator(XMLP->OperatorId).ProcessorType.c_str() % XMLP->ResultExternal).str().c_str());
                StatusError=XMLP->ResultExternal;
                //PS->StorePaymentComplete(DT.CurrentDateTime(), XMLP->InitialSessionNum, Session, StatusError);
                PS->StorePaymentComplete(TDateTime::CurrentDateTime(), XMLP->InitialSessionNum, Session, StatusError, XMLP->LastErrorCode, XMLP->FirstTryDT, XMLP->OperatorId, Sum, GetComission(),""); // платеж не был проведен
                return true;
            }
        }
        if (Session!="")
        {
            Log->Write("  Payment state - with session, checking status...");
            Status = GetStatus(true);
            switch (Status)
            {
                case -1: // getting payment status error
                    Log->Append("error.");
                    if (StatusError==11)
                    {
                        if (XMLP->LastPaymentErrorCode==0)
                        {
                            Log->Write((boost::format("  Server error: Server answer: payment with session number %1% does not exist, but last payment completed successfully.") % Session.c_str()).str().c_str());
                        }
                        else
                        {
                            Log->Write((boost::format("  Payment with session number %1% does not exist. Starting new processing...") % Session.c_str()).str().c_str());
                            Session="";
                        }
                    }
                    XMLP->NumOfTries++;
                    if(Cfg->Payments.nextTry > 0 && -1 == m_result && -1 == m_error)
                        XMLP->NextTryDT=TDateTime::CurrentDateTime()+double(Cfg->Payments.nextTry)/24/60;
                    else
                        XMLP->NextTryDT = TDateTime::CurrentDateTime() + double(5*min(12,XMLP->NumOfTries)) / 24 / 60;

                    break;
                case 0: // payment does not exist
                case 1:
                    Log->Write("  Payment is not registered in payment system. Starting new processing...");
                    Session="";
                    XMLP->NextTryDT=TDateTime::CurrentDateTime()+double(5)/24/60;
                    break;
                case 2: // payment in processing
                case 3:
                case 4:
                case 5:
                case 6:
                    Log->Write("  Waiting for the payment to complete...");
                    XMLP->NumOfTries++;
                    XMLP->NextTryDT=TDateTime::CurrentDateTime()+double(5*min(12,XMLP->NumOfTries))/24/60;
                    break;
                case 7: // payment completed
                    if (this->StatusError==0)
                    {
                        return true; //payment completed successfully
                    }
                    else//payment completed with error, trying to process it one more time
                    {
                        Log->Write("  Payment completed with error. Starting new processing...");
                        Status=0;
                        Session="";
                        XMLP->NumOfTries++;
                        XMLP->NextTryDT=TDateTime::CurrentDateTime()+double(5*min(12,XMLP->NumOfTries))/24/60;
                    }
                    break;
            }
        }
        else
        {
            Log->Write("  Payment State: New.");
            if (Check(false))
            {
                AnsiString PacketFileName = XMLP->PacketFileName.c_str();
                AnsiString InitialSessionNum = XMLP->InitialSessionNum;
                AnsiString PrevParams;// = XMLP->Parameters->DelimitedText;
                for (int i=0;i<XMLP->Params->Count;i++)
                    PrevParams+="\r\n"+XMLP->GetParamName(i)+"="+XMLP->GetParamValue(i);
                XMLP->CloseFile();
                Sleep(5000);
                if (XMLP->LoadFromFile(PacketFileName))
                {
                    AnsiString CurrParams;// = XMLP->Parameters->DelimitedText;
                    for (int i=0;i<XMLP->Params->Count;i++)
                        CurrParams+="\r\n"+XMLP->GetParamName(i)+"="+XMLP->GetParamValue(i);
                    if ((Session==XMLP->LastSession)&&(InitialSessionNum == XMLP->InitialSessionNum)&&(CurrParams == PrevParams))
                    {
                        Payment(false);
                        XMLP->NextTryDT=TDateTime::CurrentDateTime()+double(5)/24/60;
                    }
                    else
                    {
                        Log->Write("Error comparing saved payment info!");
                        PacketLoadError = true;
                        XMLP->NumOfTries++;
                        XMLP->NextTryDT=TDateTime::CurrentDateTime()+double(5*XMLP->NumOfTries)/24/60;
                    }
                }
                else
                {
                    PacketLoadError = true;
                    XMLP->NumOfTries++;
                    XMLP->NextTryDT=TDateTime::CurrentDateTime()+double(5*XMLP->NumOfTries)/24/60;
                }
            }
            else
            {
                XMLP->NumOfTries++;
                if(Cfg->Payments.nextTry > 0 && -1 == m_result && -1 == m_error)
                    XMLP->NextTryDT=TDateTime::CurrentDateTime()+double(Cfg->Payments.nextTry)/24/60;
                else
                    XMLP->NextTryDT=TDateTime::CurrentDateTime()+double(5*XMLP->NumOfTries)/24/60;
            }
        }
        Log->Write((boost::format("  Payment next try: %1%.") % AnsiString(XMLP->NextTryDT).c_str()).str().c_str());
        return false;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

//---------------------------------------------------------------------------

int TPayment::GetStatus(bool StatPacketSendingAllowed, bool bTestingOnly)
{
    int iError=-1;
    int iResult=-1;
    StatusError=0;
    AnsiString Temp;

    try
    {
        if (!StatPacketSendingAllowed)
            Log->Write("Stat packets sending is not allowed.");

        if (!bTestingOnly)
            Log->Write("  Getting payment status... ");
        if ((!XMLP)||(!PS))
            return -2;

        MessageText = "SESSION=" + Session + "\r\nTRANSID=" + XMLP->TransId;
        if (!bTestingOnly)
        {
            AnsiString strtmp=PrepareString(MessageText);
            Log->Append(("Message: " + strtmp).c_str());
        }

        AnsiString StatusResult = Connect(Cfg->Operator(XMLP->OperatorId).StatusURL.c_str());

        if (bTestingOnly)
        {
            Log->Write((boost::format("CheckConnection result: %1%") % ConnectResult.c_str()).str().c_str());
        }
        else
        {
            std::auto_ptr <TStringList> slResult ( new TStringList() );
            if (!slResult.get())
            {
                Log->Write("  slResult creating Error. ");
            }
            else
            {
                PrepareAnswer(StatusResult,slResult.get());

                iResult = GetAnswerIntegerValue(slResult.get(),"RESULT");
                iError = GetAnswerIntegerValue(slResult.get(),"ERROR");

                Log->Write((boost::format("  %1%") % FormatStatusAnswerForLog(AnsiString("Status") ,slResult.get(), iResult, iError).c_str()).str().c_str());
                Log->Append((boost::format(" %1%") % StatusResult.c_str()).str().c_str());

                StatusError=iError;
                if (iResult==7)
                {
                    if (iError==0)
                    {
                        for (int i=0;i<XMLP->Params->Count;i++)
                            Temp+="\r\n"+XMLP->GetParamName(i)+"="+XMLP->GetParamValue(i);
                        PS->StorePaymentComplete(TDateTime::CurrentDateTime(), XMLP->InitialSessionNum, Session, iError, XMLP->LastErrorCode, XMLP->FirstTryDT, XMLP->OperatorId, Sum, GetComission(),Temp);
                    }
                    else
                    {
                        if (StatPacketSendingAllowed)
                            PS->StorePaymentStatusChange(TDateTime::CurrentDateTime(), XMLP->InitialSessionNum, 0,iError);
                    }
                }
                else
                {
                    if ((iResult!=XMLP->Status)&&(iResult!=-1))
                    {
                        XMLP->Status=iResult;
                        if (StatPacketSendingAllowed)
                            PS->StorePaymentStatusChange(TDateTime::CurrentDateTime(), XMLP->InitialSessionNum, iResult,0);
                    }
                    else
                    {
                        if ((iResult==-1)&&(iError!=-1))
                        {
                            if (StatPacketSendingAllowed)
                                PS->StorePaymentStatusChange(TDateTime::CurrentDateTime(), XMLP->InitialSessionNum, XMLP->Status,iError);
                        }
                    }
                }
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    m_error = iError;
    m_result = iResult;
    return iResult;
}

//---------------------------------------------------------------------------

AnsiString TPayment::GetMessageTextForCheck(bool bFirstCheck,const std::string& existInquiry)
{
    AnsiString Temp;
    try
    {
        if (bFirstCheck)
        {
            if (!Cfg->Operator(XMLP->OperatorId).PaymentBeforeChequePrinting)
                Temp += "\r\nREQ_TYPE=1";
        }
        else
        {
            if(std::string::npos==existInquiry.find("COMMENT="))
                Temp = "\r\nCOMMENT="+XMLP->InitialSessionNum;
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Temp = "";
    }
    return Temp;
}

//---------------------------------------------------------------------------

bool TPayment::Check(bool bFirstCheck, AnsiString AForcedOffline)
{
    int iError=-1;
    int iResult=0;
    bool bResult=false;
    AnsiString NewSession;
    try
    {
        Log->Write("  Check. ");
        if ((!XMLP)||(!PS))
            return false;

        NewSession = GetSessionNumber().c_str();
        AnsiString OfflinePeriod;
        if ( (bFirstCheck) && ( ((Cfg->IsOffline(Recepient, OfflinePeriod))&&(AForcedOffline!="0")) || (AForcedOffline=="1") ) )
        {
            if (AForcedOffline=="1")
                Log->Append("Offline mode forced by interface. ");
            Log->Append((boost::format("No Check - Offline mode%1%. Payment info: %2%.") % OfflinePeriod.c_str() % PrepareString(GetMessageText(NewSession).c_str()).c_str()).str().c_str());
            XMLP->InitialSessionNum=NewSession;
            bResult = true;
        }
        else
        {
            if (AForcedOffline=="0")
                Log->Append("Online mode forced by interface. ");

            MessageText=GetMessageText(NewSession);
            MessageText+=GetMessageTextForCheck(bFirstCheck,MessageText.c_str());

            Log->Append((boost::format("Message: %1%") % PrepareString(MessageText).c_str()).str().c_str());

            AnsiString CheckResult=Connect(Cfg->Operator(XMLP->OperatorId).CheckURL.c_str(),bFirstCheck);

            std::auto_ptr <TStringList> slResult ( new TStringList() );
            if (!slResult.get())
            {
                Log->Write("  slResult creating Error. ");
                bResult = false;
            }
            else
            {
                PrepareAnswer(CheckResult,slResult.get());
                ParseAnswer(slResult.get());

                iResult = GetAnswerIntegerValue(slResult.get(),"RESULT");
                iError = GetAnswerIntegerValue(slResult.get(),"ERROR");

                CheckErrorCode = iError;

                XMLP->TransId = GetAnswerValue(slResult.get(),"TRANSID");
                AddInfo=GetAnswerValue(slResult.get(),"ADDINFO").c_str();

                if (bFirstCheck)
                    PostProcessFirstCheck(iResult,iError);

                if (((iResult==0)&&(iError==0)) || ((iResult != 0) && isErrorIgnored(iError)))
                {
                    bResult=true;
                    Session=GetAnswerValue(slResult.get(),"SESSION");
                    if (Session=="")
                        Session=NewSession;
                    if (!bFirstCheck)
                    {
                        XMLP->LastSession=Session;
                        if (!XMLP->SaveToFile())
                        {
                            Log->Write("if (!XMLP->SaveToFile())");
                            FileMap->WriteErrorFound = true;
                            bResult = false;
                            iResult=-1;
                            iError=-1;
                        }
                    }
                    else
                    {
                        XMLP->InitialSessionNum=Session;
                    }
                }

                if ((bFirstCheck)&&(iResult==21)&&(Cfg->IsMoneyOffline()))
                {
                    Log->Write("Check succeeded - Money Offline Mode.");
                    bResult=true;
                }

                Log->Write((boost::format("  %1%") % FormatAnswerForLog("Check", slResult.get(), iResult, iError).c_str()).str().c_str());
                Log->Append((boost::format(" %1%") % CheckResult.c_str()).str().c_str());

                if ((!bFirstCheck)&&(iResult!=-1)&&(iError!=-1)&&(iResult!=0)&&(iError!=0)&&(iError!=XMLP->LastErrorCode))
                {
                    XMLP->LastErrorCode = iError;
                    if (!XMLP->SaveToFile())
                        FileMap->WriteErrorFound = true;
                    PS->StorePaymentStatusChange(TDateTime::CurrentDateTime(), XMLP->InitialSessionNum, 0,iError);
                }
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    m_error = iError;
    m_result = iResult;

    return bResult;
}

//---------------------------------------------------------------------------

bool TPayment::isErrorIgnored(int error)
{
  TOpInfo tOperator = Cfg->Operator(Recepient);
  std::string processorTypeLowerCase = tOperator.ProcessorType;
  boost::to_lower(processorTypeLowerCase);

  bool is_operator_need_online = ((processorTypeLowerCase != "cyberplat")  || tOperator.ShowAddInfo);
  bool result = ((Cfg->Payments.IgnoredCheckErrorsList.find(error) != Cfg->Payments.IgnoredCheckErrorsList.end()) &&
                 (!is_operator_need_online));

  return result;
}

//---------------------------------------------------------------------------

AnsiString TPayment::GetMessageTextForPayment(const std::string& existInquiry)
{
    AnsiString Temp;
    try
    {
        Temp = "\r\nTRANSID="+XMLP->TransId;
        if(std::string::npos==existInquiry.find("COMMENT="))
            Temp+="\r\nCOMMENT="+XMLP->InitialSessionNum;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Temp = "";
    }
    return Temp;
}

//---------------------------------------------------------------------------

void TPayment::SalonPacketSend()
{
try
{
  TOpInfo qOperator = Cfg->Operator(Recepient);

  double skilful_sum;
  if(qOperator.fix)
    skilful_sum = GetLimMax() + GetComission();
  else
    skilful_sum = Sum;
  _keys_info* CurrentKeys = &Cfg->Keys[Cfg->GetKeysNum(qOperator.KeysId)];

  std::string MessageText =
  "SD=" + CurrentKeys->SD + "\r\n" +
  "AP=" + CurrentKeys->AP + "\r\n" +
  "OP=" + CurrentKeys->OP + "\r\n" +
  "AMOUNT=" + XMLP->GetParamValue("AMOUNT_ALL").c_str() + "\r\n"
  "DATE=" + GetCurrentDateTime("%d.%m.%y") + "\r\n" +
  "CHECK_NUM=" + std::string(XMLP->TransId.c_str()) + "\r\n" +
  "CARD_NUM=" + std::string(XMLP->GetParamValue("SALONCARD").c_str()) + "\r\n" +
  "SESSION=" + GetSessionNumber();
  
  AnsiString SignedMessage = "";
  SignedMessage = crypt::sign(Cfg->GetKeysNum(), MessageText.c_str()).c_str();
  if (SignedMessage.IsEmpty())
    Log->Write("SvyaznoyMessage: crypt error");
  else
  {
    SignedMessage = "inputmessage=" + SignedMessage;

    Log->Write(("SvyaznoySignedMessage: " + SignedMessage).c_str());
    AnsiString URL = AnsiString(qOperator.Fields[qOperator.SalonField].URL1.c_str());

    std::auto_ptr <TConnectThread> Con ( new TConnectThread(Log, Cfg, URL, SignedMessage, qOperator.KeysId) );
    if (!Con.get())
      Log->Write("  Svyaznoy: TConnectThread creating Error(1). ");
    else
    {
      //std::auto_ptr <TConnectThread> Con ( new TConnectThread(Log, Cfg, URL, SignedMessage, qOperator.KeysId) );
      if (!Con.get())
        Log->Write(" Svyaznoy: TConnectThread creating Error(2). ");
      else
      {
        Con->Resume();
        int TimeOut=Cfg->ConnectionTimeOut*100;
        while ((TimeOut>0)&&(!Con->Finished))
        {
          Application->ProcessMessages();
          Sleep(10);
          TimeOut--;
        }

        if (Con->IndyError)
          Log->Write(" Svyaznoy: Error in Indy library!");

        if (TimeOut<=0)
        {
          Log->Append(" Svyaznoy: Timed out.");
          TerminateThread((HANDLE)Con->Handle,0);
        }
        else
        {
          if (Con->Finished)
          {
            if (!Con->AnswerMessage.IsEmpty())
            {
              Log->Write((" Svyaznoy: Result = " + Con->AnswerMessage).c_str());
              AnsiString Result = crypt::verify(0, Con->AnswerMessage.c_str()).c_str();
            }
            else
              Log->Write(" Svyaznoy: AnswerMessage is empty!!");
          }
          else
            Log->Write(" Svyaznoy: Connect Thread haven`t finished!!");

        }
      }
    }
  }
}
catch(...)
{
  ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
}
}

//---------------------------------------------------------------------------

bool TPayment::Payment(bool bProcessMessages, bool bSendStatusChange)
{
    UNREFERENCED_PARAMETER(bSendStatusChange);
    int iError = -1;
    int iResult = -1;
    bool bResult=false;
    try
    {
        if ((!XMLP)||(!PS))
            return false;

        Log->Write("  Payment. ");
        MessageText=GetMessageText(Session);
        MessageText+=GetMessageTextForPayment(MessageText.c_str());
        Log->Append((boost::format("Message: %1%") % PrepareString(MessageText).c_str()).str().c_str());

        AnsiString PaymentResult = Connect(Cfg->Operator(XMLP->OperatorId).PaymentURL.c_str(),bProcessMessages);

        std::auto_ptr <TStringList> slResult ( new TStringList() );
        if (!slResult.get())
        {
            Log->Write("  slResult creating Error. ");
            bResult = false;
        }
        else
        {
            PrepareAnswer(PaymentResult,slResult.get());
            ParseAnswer(slResult.get());

            iResult = GetAnswerIntegerValue(slResult.get(),"RESULT");
            iError = GetAnswerIntegerValue(slResult.get(),"ERROR");
            XMLP->LastPaymentErrorCode = PaymentErrorCode = iError;
            try
            {
                PaymentProcessedDT=TDateTime(GetAnswerValue(slResult.get(),"DATE"));
            }
            catch (...)
            {
                PaymentProcessedDT=0;
            }

            PostProcessPayment(iResult,iError);

            if ((iResult==0)&&(iError==0))
            {
                bResult=true;
                if (Cfg->Operator(Recepient).SalonField)
                  SalonPacketSend();
            }

            Log->Write((boost::format("  %1%") % FormatAnswerForLog("Payment", slResult.get(), iResult, iError).c_str()).str().c_str());
            Log->Append((boost::format(" %1%") % PaymentResult.c_str()).str().c_str());

            if ((iResult!=-1)&&(iError!=-1)&&(iResult!=0)&&(iError!=0)&&(iError!=XMLP->LastErrorCode))
            {
                XMLP->LastErrorCode = iError;
                if (!XMLP->SaveToFile())
                    FileMap->WriteErrorFound = true;
                PS->StorePaymentStatusChange(TDateTime::CurrentDateTime(), XMLP->InitialSessionNum, 0,iError);
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    m_error = iError;
    m_result = iResult;
    return bResult;
}

//---------------------------------------------------------------------------

void TPayment::PrepareAnswer(AnsiString &ASrc, TStringList *slTrgt)
{
    try
    {
        ASrc=PrepareString(ASrc);
        slTrgt->DelimitedText=ASrc;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

AnsiString TPayment::PrepareString(AnsiString ASrc)
{
    try
    {
        ASrc=ASrc.TrimRight();
        while (ASrc.Pos("\r\n")>0)
            ASrc = ASrc.SubString(0, ASrc.Pos("\r\n") - 1) + "\",\"" +
                   ASrc.SubString(ASrc.Pos("\r\n") + 2, ASrc.Length());
        ASrc="\""+ASrc+"\"";
        return ASrc;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return "";
    }
}

//---------------------------------------------------------------------------

AnsiString TPayment::GetAnswerValue(TStringList *slSrc, const AnsiString& AName)
{
    return slSrc->Values[AName];
}

//---------------------------------------------------------------------------

bool TPayment::HasAnswerValue(TStringList *slSrc, const AnsiString& AName)
{
    return (slSrc->IndexOfName(AName)==-1) ? false : true;
}

//---------------------------------------------------------------------------

int TPayment::GetAnswerIntegerValue(TStringList *slSrc, const AnsiString AName)
{
    int result = db_errors::noValidPhone;
    try
    {
        AnsiString Temp = GetAnswerValue(slSrc,AName);
        if (Temp!="")
            result = Temp.ToInt();
    }
    catch (EConvertError &ex)
    {
        //Log->Write("Can't get "+AName+" value: {"+ex.Message+"}, setting to -1.");
    }
    catch (...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return result;
}

//---------------------------------------------------------------------------

AnsiString TPayment::FormatAnswerForLog(const char* AHeader, TStringList *slSrc, int iResult, int iError)
{
    AnsiString AToLog=AHeader;
    if ((iResult==0)&&(iError==0))
    {
        AToLog+=" succeeded.";
    }
    else
    {
        AToLog+=" failed {Result = " + AnsiString(iResult);
        if (GetAnswerValue(slSrc,"ERRMSG")!="")
            AToLog+=", Error Message = {"+GetAnswerValue(slSrc,"ERRMSG")+"}";
        AToLog+=", Error = "+AnsiString(iError);
        AToLog+=" {"+GetErrorDescr(iError)+"}}.";
    }
    return AToLog;
}

//---------------------------------------------------------------------------

AnsiString TPayment::FormatStatusAnswerForLog(AnsiString AHeader, TStringList *slSrc, int iResult, int iError)
{
    UNREFERENCED_PARAMETER(slSrc);
    AnsiString AToLog;
    //bool bSuccess=false;
    try
    {
        AToLog = AHeader;
        switch (iResult)
        {
            case 0:
            case 1:
                AToLog+=" result: Payment processing has not been started.";
                break;
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
                AToLog+=" result: Payment is in processing.";
                break;
            case 7:
                if (iError==0)
                    AToLog+=" result: Payment completed.";
                else
                    AToLog+=" result: Payment completed with error: "+AnsiString(iError)+".";
                break;
            default:
                AToLog+=" error {Result = " + AnsiString(iResult)+", Error = "+AnsiString(iError)+"}.";
            }
        return AToLog;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return AToLog;
    }
}

//---------------------------------------------------------------------------

AnsiString TPayment::StringForPaymentsLog()
{
    AnsiString Temp;
    try
    {
        Temp=("Operator: "+Cfg->Operator(XMLP->OperatorId).Name+", Initial Session: ").c_str()+XMLP->InitialSessionNum+", Last Session: "+Session;
        for (int i=0;i<XMLP->Params->Count;i++)
            Temp+=", "+XMLP->GetParamName(i)+"="+XMLP->GetParamValue(i);

        Temp+=".";
        return Temp;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return Temp;
    }
}

//---------------------------------------------------------------------------

AnsiString TPayment::GetMessageText(AnsiString SessionNumber)
{
    AnsiString Temp;
    try
    {
        Temp+=("SD="+Cfg->Keys[Cfg->GetKeysNum(Cfg->Operator(XMLP->OperatorId).KeysId)].SD+"\r\nAP="+Cfg->Keys[Cfg->GetKeysNum(Cfg->Operator(XMLP->OperatorId).KeysId)].AP+"\r\nOP="+Cfg->Keys[Cfg->GetKeysNum(Cfg->Operator(XMLP->OperatorId).KeysId)].OP+("\r\nSESSION="+SessionNumber).c_str()).c_str();
        for (int i=0;i<XMLP->Params->Count;i++)
            Temp+="\r\n"+XMLP->GetParamName(i)+"="+XMLP->GetParamValue(i);
        return Temp;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return Temp;
    }
}

//---------------------------------------------------------------------------

std::string TPayment::GetSessionNumber()
{
    ReadVersion();
    std::string ver = FileVersion;
    boost::algorithm::replace_all(ver, ".", "");
    return (boost::format("%s%08s") % GetCurrentDateTime("%d%m%y%H%M%S").c_str() % ver).str();
}

//---------------------------------------------------------------------------

void TPayment::ParseLocation(AnsiString LocationString)
{
    try
    {
        std::auto_ptr <TLocationParser> Location ( new TLocationParser(LocationString.c_str()));

        if (Location->HasParameter("recepient"))
        {
            Recepient=GetInt(Location->GetParameter("recepient").c_str());
        }
        else
        {
            Log->Write("Error: \"recepient\" parameter missing in location string!");
            return;
        }
        
        //zh_salon_sendpacket
        TOpInfo qOperator = Cfg->Operator(Recepient);
        if (qOperator.SalonField)
        {
          SalonCard = AnsiString(Location->Parameters["field" + qOperator.Fields[qOperator.SalonField].Id].c_str());
          XMLP->AddParam("SALONCARD", SalonCard);
        }
        //zh_salon_sendpacket

        //zh_sp
        if (qOperator.fix > 1)
          MinSum = GetDouble(Location->GetParameter(
            (std::string("field") + boost::lexical_cast<std::string>(qOperator.fix)).c_str()).c_str());
        //zh_sp

        int fIndex = 100;
        FieldsFromInterface = "";
        do
        {
          std::string fieldNumber = "field" + boost::lexical_cast<std::string>(fIndex);
          FieldsFromInterface += "&" + fieldNumber + "=" + Location->Parameters[fieldNumber];
          fIndex++;
        } while (Location->Parameters["field" + boost::lexical_cast<std::string>(fIndex)] != "");

        std::auto_ptr <TStringList> FieldForChequeList ( new TStringList() );
        std::auto_ptr <TStringList> UnnamedFieldForChequeList ( new TStringList() );

        AnsiString processorType = AnsiString(Cfg->Operator(Recepient).ProcessorType.c_str()).LowerCase();
        if (processorType=="cyberplat" || processorType == "cyberplat_taxes" || processorType == "avia_center")
        {
            for (unsigned int i=0; i<Cfg->Operator(Recepient).Properties.size(); i++)
            {
                AnsiString Mask=Cfg->Operator(Recepient).Properties[i].FieldId.c_str();
                std::string strName = Cfg->Operator(Recepient).Properties[i].Name;
                for (unsigned int j=0; j<Cfg->Operator(Recepient).Fields.size();j++)
                {
                    AnsiString FieldId=Cfg->Operator(Recepient).Fields[j].Id.c_str();
                    std::string tFieldParameter = Location->GetParameter(("field"+FieldId).c_str());
                    if ((strName == "ACCOUNT") || (strName == "COMMENT"))
                      boost::trim(tFieldParameter);
                    Mask = ChangeChars(Mask,"[#"+FieldId+"]", tFieldParameter.c_str()).c_str();
                }
                XMLP->AddParam(strName.c_str(),Mask);
            }
        }
        else
        {
            for (std::size_t i=0; i<Cfg->Operator(Recepient).Fields.size();i++)
                XMLP->AddParam(("field"+Cfg->Operator(Recepient).Fields[i].Id).c_str(),Location->GetParameter(("field"+Cfg->Operator(Recepient).Fields[i].Id).c_str()).c_str());
        }

        if (Cfg->Operator(Recepient).CheckAmount==-1)
        {
            AnsiString LimMaxTemp = Location->GetParameter(("field"+Cfg->Operator(Recepient).CheckAmountFieldId).c_str()).c_str();
            XMLP->AddParam("AMOUNT",LimMaxTemp);
            ForcedSum=GetInt(LimMaxTemp);
        }
        else
        {
            XMLP->AddParam("AMOUNT",Cfg->Operator(Recepient).CheckAmount);
        }

        std::string Name,Value,result="";
        for (std::size_t i=0; i<Cfg->Operator(Recepient).Fields.size();i++)
        {
            if (Cfg->Operator(Recepient).Fields[i].Type != "secretword" && Cfg->Operator(Recepient).Fields[i].Type != "textpwd")
            {
                if(result!="")
                    result=result+",";
                if (Cfg->Operator(Recepient).Fields[i].Type == "enum")
                {
                    Name=Cfg->Operator(Recepient).Fields[i].Name;
                    Value=Cfg->Operator(Recepient).Fields[i].GetEnumText(Location->GetParameter(("field"+Cfg->Operator(Recepient).Fields[i].Id).c_str()).c_str()).c_str();
                    FieldForChequeList->Add((Cfg->Operator(Recepient).Fields[i].Name+"="+Cfg->Operator(Recepient).Fields[i].GetEnumText(Location->GetParameter(("field"+Cfg->Operator(Recepient).Fields[i].Id).c_str()).c_str()).c_str()).c_str());
                    UnnamedFieldForChequeList->Add(("FIELD"+Cfg->Operator(Recepient).Fields[i].Id+"="+Cfg->Operator(Recepient).Fields[i].GetEnumText(Location->GetParameter(("field"+Cfg->Operator(Recepient).Fields[i].Id).c_str()).c_str()).c_str()).c_str());
                }
                else
                {
                    FieldForChequeList->Add((Cfg->Operator(Recepient).Fields[i].Name+"="+Location->GetParameter(("field"+Cfg->Operator(Recepient).Fields[i].Id).c_str()).c_str()).c_str());
                    UnnamedFieldForChequeList->Add(("FIELD"+Cfg->Operator(Recepient).Fields[i].Id+"="+Location->GetParameter(("field"+Cfg->Operator(Recepient).Fields[i].Id).c_str()).c_str()).c_str());

                    Name=Cfg->Operator(Recepient).Fields[i].Name;
                    Value=Location->GetParameter(("field"+Cfg->Operator(Recepient).Fields[i].Id).c_str()).c_str();
                }
                if("" != Cfg->Operator(Recepient).Fields[i].Regexp)
                {
                    Value = boost::regex_replace(Value, boost::regex(Cfg->Operator(Recepient).Fields[i].Regexp.c_str()), Cfg->Operator(Recepient).Fields[i].Format.c_str());
                }
                else
                {
                    boost::replace_all(Value,"!","");
                    boost::replace_all(Value,"~","");
                }
                result+="\""+Name+"="+Value+"\"";
            }
        }
        std::string PaymentFields = Location->GetParameters();
        std::string PaymentURLRegex = "(.*)([&|?]pb=checking)&?(.*)";
        boost::regex PaymentURLExpr(PaymentURLRegex);
        if(boost::regex_match(PaymentFields, PaymentURLExpr))
        {
          boost::cmatch whatAttr;
          boost::regex_search(PaymentFields, whatAttr, PaymentURLExpr);
          boost::replace_all(PaymentFields, whatAttr[2].str(), "");
        }
        AFieldsForInterface=PaymentFields.c_str();

        AFieldsForCheque=FieldForChequeList->DelimitedText;
        AFieldsForCheque=result.c_str();
        AUnnamedFieldsForCheque=UnnamedFieldForChequeList->DelimitedText;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}
//---------------------------------------------------------------------------

bool TPayment::InitPayment(AnsiString Location)
{
    bool res = false;
    try
    {
        try
        {
            Log->Write("New payment started.");
            MinSum=0;
            Sum = 0;

            ParseLocation(Location);

            XMLP->OperatorId=Recepient;
            XMLP->ProcessorType=Cfg->Operator(Recepient).ProcessorType.c_str();
            XMLP->FirstTryDT=TDateTime::CurrentDateTime();
            res = Check(true,(TLocationParser(Location.c_str())).GetParameter("offline").c_str());
            if (res)
            {
                AnsiString ProcessorType=AnsiString(Cfg->Operator(Recepient).ProcessorType.c_str()).LowerCase();
                if (
                      (ProcessorType == "cyberplat")
                    ||(ProcessorType == "cyberplat_pin")
                    ||(ProcessorType == "cyberplat_pin_trans")
                    ||(ProcessorType == "cyberplat_euroset")
                    ||(ProcessorType == "cyberplat_metro")
                    ||(ProcessorType == "avia_center")
                    ||(ProcessorType == "half_pin")
                    ||(ProcessorType == "cyberplat_mt")
                    ||(ProcessorType == "cyberplat_taxes")
                    )
                    XMLP->PacketFileName = XMLP->GetNewFileName("tmp","out").c_str();
                else
                    XMLP->PacketFileName = XMLP->GetNewFileName("tmp","ext").c_str();

                res = XMLP->CreateFile(XMLP->PacketFileName.c_str());

                if (!res)
                {
                    Log->Write("Error creating payment packet!");
                    CheckErrorCode = db_errors::errPacket0;
                }
                else
                {
                    res = InitPS->CreateTempPacket();
                    if (!res)
                    {
                        Log->Write("Error creating init stat packet!");
                        CheckErrorCode = db_errors::errPacket0;
                    }
                    else
                    {
                        if (Cfg->Operator(Recepient).PaymentBeforeChequePrinting)
                        {
                            res = PS->CreatePacket();
                            if (!res)
                            {
                                Log->Write("Error creating stat packet!");
                                CheckErrorCode = db_errors::errPacket0;
                            }
                        }
                    }
                }
            }
            if (!(Cfg->Payments.rState))
              AddNote(0, Cfg->Payments.Rest);
            Cfg->Payments.Rest = 0;
        }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        }
    }
    __finally
    {
        return res;
    }
}

//---------------------------------------------------------------------------
#include <boost\format.hpp>
void TPayment::SetSum(double Sum)
{
    try
    {
        double validSum = (Sum <= GetLimMax()) ? Sum : GetLimMax();
        double comission = Cfg->GetComission(XMLP->OperatorId, validSum, PaymentCreateDT);
        double Amount = validSum - comission;

        double sum_all_notes = 0;
        for(int i = 0; i < XMLP->vNotes.size(); i++)
          sum_all_notes += XMLP->vNotes[i].Nominal * XMLP->vNotes[i].Count * Cfg->CurrencyInfo.ExchangeRate;
        double Amount_All = (Sum <= GetLimMax()) ? sum_all_notes : validSum + comission;

        XMLP->AddParam("AMOUNT_ALL", Cfg->Payments.rState ? Amount_All : sum_all_notes);
        XMLP->AddParam("AMOUNT", Cfg->Operator(Recepient).fix ? validSum : Amount);
        if (!XMLP->SaveToTempFile())
            FileMap->WriteErrorFound = true;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------
double TPayment::GetComission(double Sum)
{
    try
    {
        if (ForcedSum > 0)
            return Cfg->GetComission(XMLP->OperatorId, ForcedSum, PaymentCreateDT);
        else
            return Cfg->GetComission(XMLP->OperatorId, Sum, PaymentCreateDT);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return 0;
    }
}

double TPayment::GetComission()
{
    double temps = (Sum < GetLimMax()) ? Sum : GetLimMax();

    try
    {
        if (ForcedSum>0)
            return Cfg->GetComission(XMLP->OperatorId,ForcedSum, PaymentCreateDT);
        else
            if(Cfg->Operator(Recepient).fix)
              return Cfg->GetComission(XMLP->OperatorId, temps, PaymentCreateDT);
            else
              return Cfg->GetComission(XMLP->OperatorId, Sum, PaymentCreateDT);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return 0;
    }
}

//---------------------------------------------------------------------------

double TPayment::GetPaymentMinSum()
{
    try
    {
        if (ForcedSum>0)
            return ForcedSum+Cfg->GetComission(XMLP->OperatorId,ForcedSum, PaymentCreateDT);
        else
            return Cfg->GetPaymentMinSum(XMLP->OperatorId,MinSum,PaymentCreateDT);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return 0;
    }
}

//---------------------------------------------------------------------------

AnsiString TPayment::ConvertFieldsString(AnsiString Fields)
{
    try
    {
        AnsiString Temp = Fields;
        while (Temp.Pos("\",\""))
            Temp=Temp.SubString(0,Temp.Pos("\",\"")-1)+"|"+Temp.SubString(Temp.Pos("\",\"")+3,Temp.Length());

        while (Temp.Pos("\""))
            Temp=Temp.SubString(0,Temp.Pos("\"")-1)+Temp.SubString(Temp.Pos("\"")+1,Temp.Length());

        return Temp;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return "";
    }
}

//---------------------------------------------------------------------------

void TPayment::AddNote(int ValidatorID, double Nominal)
{
    try
    {
        Log->Write((boost::format("Note %1% added to payment.") % Nominal).str().c_str());
        if ((!XMLP)||(!PS))
            return;
        Sum = Sum + Nominal * Cfg->CurrencyInfo.ExchangeRate;
        UpdateNotes(ValidatorID, Nominal);

        std::string processorTypeLowerCase = Cfg->Operator(Recepient).ProcessorType;
        boost::to_lower(processorTypeLowerCase);
        bool is_processor_type_fix = (processorTypeLowerCase == "avia_center") ||
                                     (processorTypeLowerCase == "cyberplat_taxes") ||
                                     (processorTypeLowerCase == "cyberplat_mt") ||
                                     (processorTypeLowerCase == "cyberplat_pin") ||
                                     (processorTypeLowerCase == "cyberplat_pin_trans") ||
                                     (processorTypeLowerCase == "half_pin") ||
                                     (processorTypeLowerCase == "avia_center") ||
                                     (processorTypeLowerCase == "cyberplat_mt") ||
                                     (processorTypeLowerCase == "cyberplat_metro");

        double statComission = GetComission()*100;
        double statSum = Sum;
        double fixSum  = GetLimMax() + GetComission(GetLimMax());
        double statFixSum = fixSum;
        bool is_payment_will_pass = (Sum >= statFixSum);

        TNotesVector tNotes = XMLP->vNotes;
        if ((is_processor_type_fix || Cfg->Operator(Recepient).fix) && is_payment_will_pass && Cfg->Payments.rState)
        {
            TNote spNote;
            spNote.ValidatorID = ValidatorID;
            spNote.CurrencyID = Cfg->CurrencyInfo.Currency;
            spNote.Nominal = statFixSum;
            spNote.Count = 1;
            tNotes.clear();
            tNotes.push_back(spNote);

            statComission = (statFixSum - GetLimMax())*100;
            statSum = GetLimMax();
        }

        InitPS->StorePaymentInitTemp(TDateTime::CurrentDateTime(), XMLP->OperatorId, XMLP->InitialSessionNum, statComission/statSum, tNotes, getStatFields(AFieldsForInterface.c_str()).c_str());
        SetSum(Sum);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

bool TPayment::RenamePackets()
{
    bool bResult = true;
    try
    {
        InitPS->StorePaymentInitComplete();
        XMLP->RenameTempFile();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        bResult = false;
    }
    return bResult;
}

bool TPayment::RenamePacketsToUnprocess()
{
    bool bResult = true;
    try
    {
        std::string packetFileName=XMLP->PacketFileName.c_str();
        std::size_t pos=packetFileName.find_last_of('\\');
        if(pos == std::string::npos && 0 == Sum)
            return true;
        std::string strtmp = Cfg->Dirs.PaymentsUnprocessed+packetFileName.substr(pos,packetFileName.length());
        XMLP->RenamePaketFileFullPath(strtmp.c_str());
        XMLP->RenameTempFile();
        PS->StorePaymentComplete(TDateTime::CurrentDateTime(), XMLP->InitialSessionNum, Session, StatusError, XMLP->LastErrorCode, XMLP->FirstTryDT, XMLP->OperatorId, Sum, GetComission(),"");
        PS->StorePaymentStatusChange(TDateTime::CurrentDateTime(), XMLP->InitialSessionNum, 100, 0);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        bResult = false;
    }
    return bResult;
}
//---------------------------------------------------------------------------

bool TPayment::InitDone(PaymentDoneCmd Command)
{
    bool bResult = true;
    try
    {
        if ((!XMLP)||(!PS))
            return true;

        Session = XMLP->InitialSessionNum;
        if(Cfg->Operator(XMLP->OperatorId).PaymentBeforeChequePrinting)
        {
            if (Command == cnPDCmdCancel)
            {
                Log->Write("Cancel payment");
                bResult=false;
                RetryAllowed=false;
                if (Sum < GetPaymentMinSum() && Sum > 0)
                    PaymentErrorCode = 7;
                else
                    XMLP->DeleteTempFile();
                if(AnsiString(Cfg->Operator(XMLP->OperatorId).ProcessorType.c_str()).LowerCase()=="avia_center")
                    PaymentErrorCode = 0;
            }
            else
            {
                if(RetryAllowed)
                    Log->Write("Initializing payment incomplete, retry.");
                bResult = Payment(true);
                Update();
                AnsiString Temp;
                for (int i=0;i<XMLP->Params->Count;i++)
                    Temp+="\r\n"+XMLP->GetParamName(i)+"="+XMLP->GetParamValue(i);

                XMLP->LastErrorCode = PaymentErrorCode;

                if (!bResult)
                {
                    PS->StorePaymentStatusChange(TDateTime::CurrentDateTime(), Session, 0,PaymentErrorCode);
                    PS->CreatePacket();
                    PaymentErrorCode = PCCTimedOut;
                }
                else
                {
                    PS->StorePaymentComplete(TDateTime::CurrentDateTime(), Session, Session, PaymentErrorCode, 0, XMLP->FirstTryDT, XMLP->OperatorId, Sum, GetComission(),Temp);
                    Log->Write("Initializing payment complete.");
                }
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        bResult = false;
    }
    return bResult;
}

//---------------------------------------------------------------------------

AnsiString TPayment::Connect(AnsiString URL, bool bProcessMessages)
{
    AnsiString Result="";
    try
    {
        TransportError = false;
        if ((!XMLP)||(!PS))
            return "";

        AnsiString SignedMessage,Signature;

        if (Cfg->Operator(XMLP->OperatorId).SignatureType==1)
        {
            std::string signatureMessage = "";
            SignedMessage=crypt::signD(Cfg->GetKeysNum(Cfg->Operator(XMLP->OperatorId).KeysId), MessageText.c_str(), signatureMessage).c_str();
            Signature = AnsiString(signatureMessage.c_str());
        }
        else
        {
            SignedMessage=crypt::sign(Cfg->GetKeysNum(Cfg->Operator(XMLP->OperatorId).KeysId), MessageText.c_str()).c_str();
            if (SignedMessage=="")
            {
                Log->Write("Error with keys ERROR=-2");
                return "ERROR=-2";
            }
            SignedMessage = "inputmessage="+SignedMessage;
        }

        //Log->Write("SignedMessage: " + SignedMessage);
        //Log->Write("Signature: " + Signature);

        if (SignedMessage!="")
        {
            std::auto_ptr <TConnectThread> Con ( new TConnectThread(Log, Cfg, URL, SignedMessage, Cfg->Operator(XMLP->OperatorId).KeysId, Signature) );
            if (!Con.get())
            {
                Log->Write("  TConnectThread creating Error. ");
            }
            else
            {
                Con->Resume();
                int TimeOut=Cfg->ConnectionTimeOut*100;
                while ((TimeOut>0)&&(!Con->Finished))
                {
                    if (bProcessMessages)
                        Application->ProcessMessages();
                    Sleep(10);
                    TimeOut--;
                }

                IndyError = Con->IndyError;
                if (Con->IndyError)
                {
                    Log->Write("Error in Indy library!");
                }

                if (TimeOut<=0)
                {
                    Log->Append("Timed out.");
                    TerminateThread((HANDLE)Con->Handle,0);
                }
                else
                {
                    if ((Con->Finished)&&(Con->AnswerMessage!=""))
                    {
                    //Log->Write(("Result :{"+Con->AnswerMessage+"}").c_str());
                        if (Cfg->Operator(XMLP->OperatorId).SignatureType==1)
                        {
                            Result = crypt::verifyD(Cfg->GetKeysNum(Cfg->Operator(XMLP->OperatorId).KeysId), Con->AnswerMessage.c_str(), Con->DetachedSignature.c_str()).c_str();
                        }
                        else
                        {
                        //Result = Con->AnswerMessage;
                            Result = crypt::verify(Cfg->GetKeysNum(Cfg->Operator(XMLP->OperatorId).KeysId), Con->AnswerMessage.c_str()).c_str();
                        }
                    }
                }
                ConnectResult = Con->ConnectResult;
                TransportError = Con->TransportError;

                FileMap->PutLastCPConnectionDT();
                if (Con->ConnectOK)
                {
                    FileMap->PutLastSuccessfullCPConnectionDT();
                }
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return Result;
}

//---------------------------------------------------------------------------

AnsiString TPayment::GetErrorDescr(int val)
{
    AnsiString res = "";
    switch (val)
    {
        case 1:
            res = "Сессия с таким номером уже существует";
            break;
        case 2:
            res = "Неправильный код дилера";
            break;
        case 3:
            res = "Неправильный код точки приема";
            break;
        case 4:
            res = "Неправильный код оператора";
            break;
        case 5:
            res = "Неправильный формат кода сессии";
            break;
        case 6:
            res = "Неправильная ЭЦП";
            break;
        case 7:
            res = "Неправильный формат суммы или значение суммы вне допустимого диапазона";
            break;
        case 8:
            res = "Неправильный формат номера телефона";
            break;
        case 9:
            res = "Неправильный формат номера лицевого счета";
            break;
        case 10:
            res = "Неправильный формат документа";
            break;
        case 11:
            res = "Сессия с таким номером не существует";
            break;
        case 12:
            res = "Запрос сделан с другого IP";
            break;
        case 15:
            res = "Платежи данному оператору не поддерживаются системой";
            break;
        case 17:
            res = "Номер телефона не соответствует введенному ранее";
            break;
        case 18:
            res = "Сумма платежа не соответствует введенной ранее";
            break;
        case 19:
            res = "Номер счета (контракта) не соответствует введенному ранее";
            break;
        case 20:
            res = "Платеж находится в состоянии завершения";
            break;
        case 21:
            res = "Не хватает средств на счете дилера для проведения платежа";
            break;
        case 22:
            res = "Не прошел CyberCheck (списание средств со счета дилера)";
            break;
        case 23:
            res = "Не прошел платеж у оператора связи (нет такого телефона)";
            break;
        case 24:
            res = "Невозможно связаться с сервером  оператора связи(технологический перерыв)";
            break;
        case 26:
            res = "Невозможно связаться с сервером оператора связи (технологический перерыв)";
            break;
        case 30:
            res = "Общая ошибка системы (CyberPlat)";
            break;
        case 32:
            res = "Повторный платеж в течение 60 минут с момента окончания платежа (CyberPlat)";
            break;
        case 33:
            res = "Превышен максимальный интервал между проверкой номера и платежом (24 часа)";
            break;
        case 34:
            res = "Транзакция с таким номером не найдена";
            break;
        case 35:
            res = "Ошибка при изменении состояния платежа";
            break;
        case 39:
            res = "Неверный счет";
            break;
        case 41:
            res = "Ошибка сохранения платежа в системе";
            break;
        case 44:
            res = "Клиент не может работать на этом Торг.Сервере";
            break;
        case 45:
            res = "Отсутствуют права доступа в данный раздел";
            break;
        case 46:
            res = "Не удалось завершить ошибочный платеж";
            break;
        case 47:
            res = "Сработало временнОе ограничение прав";
            break;
        case 50:
            res = "Проведение платежей в системе временно невозможно";
            break;
        case 51:
            res = "Не найдены данные в системе";
            break;
        case 52:
            res = "Возможно дилер заблокирован";
            break;
        case 54:
            res = "Заблокированы точка или оператор";
            break;
        case 55:
            res = "Неверный формат кода брони.";
            break;
        case 133:
            res = "Общая ошибка системы";
            break;
        case 134:
            res = "Не найден ключ по указанному коду, неверно указан код точки/код дилера";
            break;
        case 333:
            res = "Неизвестная ошибка";
            break;
        default:
            res = "Неизвестная ошибка";
    }
    return res;
}

//---------------------------------------------------------------------------

void TPayment::Update()
{
    try
    {
        if (PacketLoadError)
        {
            Log->Write("Payment updating error - packet not loaded!");
            return;
        }
        if (!XMLP)
            return;
        XMLP->LastSession=Session;
        if (!XMLP->SaveToFile())
            FileMap->WriteErrorFound = true;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

bool TPayment::getXmlParseError() const
{
    return m_xmlParseError;
}

//---------------------------------------------------------------------------

void TPayment::UpdateNotes(int _ValidatorID, double _Nominal)
{
    try
    {
        if (!XMLP)
            return;
        XMLP->AddNote(_ValidatorID, _Nominal);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

bool TPayment::IsOnTime()
{
    try
    {
        if (!XMLP)
            return false;
        if (XMLP->NextTryDT<=TDateTime::CurrentDateTime())
            return true;
        return false;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

//---------------------------------------------------------------------------

bool TPayment::IsTimedOut()
{
    AnsiString Temp;
    try
    {
        if ((!XMLP)||(!PS))
            return false;
        if (Cfg->Payments.ErrorPaymentsDeleteHours==0)
            return false;
        if ((XMLP->FirstTryDT+double(Cfg->Payments.ErrorPaymentsDeleteHours)/24<=TDateTime::CurrentDateTime())&&(XMLP->NumOfTries>0))
        {
            if (Session!="")
            {
                Log->Append((boost::format(" Timed out with session: %1%.") % Session.c_str()).str().c_str()); //платеж был проведен, таймаут по статусу
                for (int i=0;i<XMLP->Params->Count;i++)
                    Temp+="\r\n"+XMLP->GetParamName(i)+"="+XMLP->GetParamValue(i);
                PS->StorePaymentComplete(TDateTime::CurrentDateTime(), XMLP->InitialSessionNum, Session, PCCTimedOutWSession, XMLP->LastErrorCode, XMLP->FirstTryDT, XMLP->OperatorId, Sum, GetComission(),Temp); // платеж был проведен
            }
            else
            {
                Log->Append(" Timed out.");
                for (int i=0;i<XMLP->Params->Count;i++)
                    Temp+="\r\n"+XMLP->GetParamName(i)+"="+XMLP->GetParamValue(i);
                PS->StorePaymentComplete(TDateTime::CurrentDateTime(), XMLP->InitialSessionNum, XMLP->InitialSessionNum, PCCTimedOut, XMLP->LastErrorCode, XMLP->FirstTryDT, XMLP->OperatorId, Sum, GetComission(),Temp); // платеж не был проведен
            }
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

//---------------------------------------------------------------------------

bool TPayment::IsCancelled()
{
    AnsiString Temp;
    try
    {
        if (!XMLP)
            return false;
        if (XMLP->Cancelled)
        {
            Log->Append(" Cancelled.");// платеж не был проведен
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

//---------------------------------------------------------------------------

void TPayment::CancelPayment()
{
    try
    {
        if (!XMLP)
            return;
        XMLP->Cancelled=true;
        XMLP->NextTryDT=TDateTime::CurrentDateTime();
        if (!XMLP->SaveToFile())
            FileMap->WriteErrorFound = true;
        Log->Write("Payment cancelled.");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

bool TPayment::CanCancel()
{
    bool bCancel = false;
    try
    {
        if (!XMLP)
            return false;
        int Status = GetStatus(false);
        switch (Status)
        {
            case -1: // getting payment status error
                if (this->StatusError==11)
                {
                    if (XMLP->LastPaymentErrorCode==0)
                    {
                        Log->Append((boost::format("Server error: Server answer: payment with session number %1% does not exist, but last payment completed successfully. Cancelling is NOT allowed!") % Session.c_str()).str().c_str());
                        bCancel = false;
                    }
                    else
                    {
                        Log->Append("Session number does not exists in payment system. Cancelling is allowed.");
                        bCancel = true;
                    }
                    break;
                }
                Log->Write("Error getting status. Cancelling is NOT allowed.");
                bCancel = false;
                break;
            case 0: // payment does not exist
            case 1:
                Log->Write("Payment processing has not been started. Cancelling is allowed.");
                bCancel = true;
                break;
            case 2: // payment in processing
            case 3:
            case 4:
            case 5:
            case 6:
                Log->Write("Payment is in processing. Cancelling is NOT allowed.");
                bCancel = false;
                break;
            case 7: // payment completed
                if (this->StatusError==0)
                {
                    Log->Write("Payment completed. Cancelling is NOT allowed.");
                    bCancel = true;
                }
                else
                { //payment completed with error, trying to process it one more time
                    Log->Write("  Payment completed with error. Cancelling is allowed.");
                    bCancel = true;
                }
                break;
        }
        if (bCancel)
        {
            if ((Status!=7)||(StatusError!=0))
            {
                AnsiString Temp;
                for (int i=0;i<XMLP->Params->Count;i++)
                    Temp+="\r\n"+XMLP->GetParamName(i)+"="+XMLP->GetParamValue(i);
                PS->StorePaymentComplete(TDateTime::CurrentDateTime(), XMLP->InitialSessionNum, XMLP->InitialSessionNum, PCCCancelledFromServer, XMLP->LastErrorCode, XMLP->FirstTryDT, XMLP->OperatorId, Sum, GetComission(),Temp); // платеж не был проведен
            }
        }
        else
        {
            XMLP->NextTryDT=TDateTime::CurrentDateTime()+double(20)/24/60;
            Log->Write((boost::format("Next status check: %1%.") % AnsiString(XMLP->NextTryDT).c_str()).str().c_str());
            Update();
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        bCancel = false;
    }
    return bCancel;
}

//---------------------------------------------------------------------------

void TPayment::ParseAnswer(TStringList *slResult)
{
    try
    {
        std::auto_ptr <TStringList> slFieldsForCheque ( new TStringList());
        slFieldsForCheque->DelimitedText=AFieldsForCheque;
        std::auto_ptr <TStringList> slReturnedFields ( new TStringList());
        slReturnedFields->DelimitedText=AUnnamedFieldsForCheque;
        for (int i=0;i<slResult->Count;i++)
        {
            if (Cfg->Operator(Recepient).HasReceiveProperty(slResult->Names[i].c_str()))
            {
                if (Cfg->Operator(Recepient).ReceiveProperties[slResult->Names[i].c_str()].Description != "") //if (Cfg->GetRPropertyDescription(Recepient, slResult->Names[i])!="")
                {
                    if (slFieldsForCheque->IndexOfName(slResult->Names[i])==-1)
                    {
                        slFieldsForCheque->Add((Cfg->Operator(Recepient).ReceiveProperties[slResult->Names[i].c_str()].Description+"="+slResult->Values[slResult->Names[i]].c_str()).c_str());
                        AFieldsForInterface+="&"+slResult->Names[i]+"="+slResult->Values[slResult->Names[i].c_str()];
                    }
                    else
                    {
                        slFieldsForCheque->Values[slResult->Names[i]]=slResult->Values[slResult->Names[i]];
                    }
                }

                if (slReturnedFields->IndexOfName(slResult->Names[i])==-1)
                    slReturnedFields->Add(slResult->Names[i]+"="+slResult->Values[slResult->Names[i]]);
                else
                    slReturnedFields->Values[slResult->Names[i]]=slResult->Values[slResult->Names[i]];
            }
        }
        std::string strtmp="";
        for(std::size_t i=0;i<slFieldsForCheque->Count;i++)
        {
            if(""!=strtmp)
                strtmp+=",";
            std::string Names=slFieldsForCheque->Names[i].c_str();
            std::string Values=slFieldsForCheque->Values[Names.c_str()].c_str();
            strtmp+="\""+Names+"="+Values+"\"";
        }
        AFieldsForCheque=slFieldsForCheque->DelimitedText;
        AFieldsForCheque=strtmp.c_str();
        AUnnamedFieldsForCheque=slReturnedFields->DelimitedText;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TPayment::PostProcessFirstCheck(int _ResultCode, int _ErrorCode)
{
    UNREFERENCED_PARAMETER(_ResultCode);
    UNREFERENCED_PARAMETER(_ErrorCode);
}

//---------------------------------------------------------------------------

void TPayment::PostProcessPayment(int _ResultCode, int _ErrorCode)
{
    try
    {
        if ((_ResultCode==0)&&(_ErrorCode==0))
        {
            if (!XMLP->SaveToFile())
                FileMap->WriteErrorFound = true;
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

double TPayment::GetLimMax()
{
    try
    {
        if (ForcedSum>0)
            return ForcedSum; //+Cfg->GetComission(XMLP->OperatorId,ForcedSum,PaymentCreateDT);
        else
            if (Cfg->Operator(Recepient).fix > 1)
              return MinSum;
            else
              return Cfg->Operator(Recepient).LimMax;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return 0;
    }
}

//---------------------------------------------------------------------------

bool TPayment::ResurrectPayment(AnsiString _Parameters)
{
    bool res = false;
    try
    {
        try
        {
            if (!XMLP || !PS)
                return false;

            _Parameters = "/checking.html" + _Parameters;
            Log->Write((boost::format("Resurrecting payment, parameters: %1%") % _Parameters.c_str()).str().c_str());

            //double Amount
            XMLP->ClearForResurrect();
            ParseLocation(_Parameters);
            XMLP->OperatorId=Recepient;
            XMLP->ProcessorType=Cfg->Operator(Recepient).ProcessorType.c_str();

            Sum = GetDouble(XMLP->GetParamValue("AMOUNT_ALL"));

            SalonCard = XMLP->GetParamValue("SALONCARD");

            float Amount=Sum-Cfg->GetComission(XMLP->OperatorId,Sum,PaymentCreateDT);
            XMLP->AddParam("AMOUNT",Amount);

            XMLP->FirstTryDT=TDateTime::CurrentDateTime();
            XMLP->NextTryDT=TDateTime::CurrentDateTime();
            if (!XMLP->SaveToFile())
                FileMap->WriteErrorFound = true;
            PS->StorePaymentInitTemp(TDateTime::CurrentDateTime(), XMLP->OperatorId, XMLP->InitialSessionNum, (Cfg->GetComission(XMLP->OperatorId,Sum,PaymentCreateDT)*100)/Sum, XMLP->vNotes, getStatFields(AFieldsForInterface.c_str()).c_str());
            PS->StorePaymentInitComplete();
            res = true;
        }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            res = false;
        }
    }
    __finally
    {
        return res;
    }
}

//---------------------------------------------------------------------------

AnsiString TPayment::CheckConnection()
{
    try
    {
        Recepient=0;
        XMLP->OperatorId=0;
        Session = "12345678901234567890";
        GetStatus(false, true);
        return ConnectResult;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return ConnectResult + "\r\nException occured ";
    }
}

//---------------------------------------------------------------------------

bool TPayment::CreateMenu()
{
    return false;
}

//---------------------------------------------------------------------------

bool TPayment::PreInit(int OperatorId, AnsiString Login, AnsiString Password)
{
    UNREFERENCED_PARAMETER(OperatorId);
    return false;
}

//---------------------------------------------------------------------------

bool TPayment::GetAmount(AnsiString &Result)
{
    int iError=-1;
    bool bResult=false;
    try
    {
        if ((!XMLP)||(!PS))
            return false;

        Log->Write("  GetAmount. ");
        _keys_info currentKeys = Cfg->Keys[Cfg->GetKeysNum(Cfg->Operator(XMLP->OperatorId).KeysId)];
        MessageText = ("SD=" + currentKeys.SD + "\r\n"
                       "AP=" + currentKeys.AP + "\r\n"
                       "OP=" + currentKeys.OP + "\r\n").c_str();

        Log->Append((boost::format("Message: %1%") % PrepareString(MessageText).c_str()).str().c_str());
        AnsiString GetAmountResult = Connect(Cfg->ServiceInfo.IncassGetAmountURL.c_str(), true);

        std::auto_ptr <TStringList> slResult ( new TStringList() );
        if (!slResult.get())
        {
            Log->Write("  slResult creating Error. ");
            bResult = false;
        }
        else
        {
            PrepareAnswer(GetAmountResult,slResult.get());
            ParseAnswer(slResult.get());

            iError = GetAnswerIntegerValue(slResult.get(),"ERROR");
            Result = GetAnswerValue(slResult.get(),"REST");

            bResult = (iError==0);

            Log->Append(GetAmountResult.c_str());
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    m_error = iError;
    m_result = -1;
    return bResult;
}

//---------------------------------------------------------------------------

std::string TPayment::getStatFields(std::string fieldsFromURL)
{
  try
  {
    //складываем в покет статистики только филды, указанные в теге request-property у оператора...
    //делим Operator.Properties.FieldId (например [#101]::[#102]::[#103]) на id`шники и складываем из в вектор
    typedef std::vector<property> requestProperties;
    TStringVector PropertiesIds;
    requestProperties Properties = Cfg->Operator(Recepient).Properties;
    for(int i = 0; i < Properties.size(); i++)
    {
      std::string request_property_elem = Properties[i].FieldId;
      boost::erase_all(request_property_elem, " ");
      boost::replace_all(request_property_elem, "][", "]::[");
      boost::erase_all(request_property_elem, "[#");
      boost::erase_all(request_property_elem, "]");
      boost::replace_all(request_property_elem, "::", ":");

      TStringVector request_property_field_ids;
      boost::split(request_property_field_ids, request_property_elem, boost::is_any_of(":"));
      for (int j = 0; j < request_property_field_ids.size(); j++)
        PropertiesIds.push_back(request_property_field_ids[j]);
    }

    //делим AFieldsForInterface на пары fieldNNN=value, складываем в другой вектор
    TStringVector FieldAttrs;
    boost::split(FieldAttrs, fieldsFromURL, boost::is_any_of("&"));

    //ходим по вектору, делим эти пары на field и value и складываем в мап
    typedef std::map<std::string, std::string> FieldParameters;
    typedef pair<std::string, std::string> FieldParameterElement;
    FieldParameters requestFieldsParameters;
    for (int k = 0; k < FieldAttrs.size(); k++)
    {
      TStringVector requestParameters;
      boost::split(requestParameters, FieldAttrs[k], boost::is_any_of("="));
      if (requestParameters.size())
        requestFieldsParameters.insert(FieldParameterElement(requestParameters[0], requestParameters[1]));
    }

    //ходим по пропертям и сравниваем с мапом, если проверка проходит, дописываем пару fieldNNN=value в initFields через "&"
    std::string initFields = "";
    for(int n = 0; n < PropertiesIds.size(); n++)
    {
      std::string request_fieldId = "field" + PropertiesIds[n];
      std::string temp_str = requestFieldsParameters[request_fieldId];
      initFields += request_fieldId + "=" + requestFieldsParameters[request_fieldId] + ((n < (PropertiesIds.size() - 1)) ? "&" : "");
    }

    //приклеиваем к initFields строку с recepient`ом. Для перепроводки.
    initFields += (boost::format("%1%recepient=%2%") % (initFields.empty() ? "" : "&") % Recepient).str();

    return initFields;
  }
  catch(...)
  {
    ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    return std::string();
  }
}
