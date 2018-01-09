//---------------------------------------------------------------------------
#pragma hdrstop
#include "THalfPinPayment.h"
#include "globals.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------

THalfPinPayment::THalfPinPayment(AnsiString _fileName, TWConfig *_Cfg, TLogClass *_Log, TFileMap* _FileMap, TXMLInfo* _InfoFile)
: TPinPayment(_fileName, _Cfg, _Log, _FileMap, _InfoFile)
{
    StatusError = 0;
}

bool THalfPinPayment::Check(bool bFirstCheck, AnsiString AForcedOffline)
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

                if ((iResult==0)&&(iError==0))
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
    return bResult;
}

AnsiString THalfPinPayment::GetMessageText(AnsiString SessionNumber)
{
    AnsiString Temp=("SD="+Cfg->Keys[Cfg->GetKeysNum(Cfg->Operator(XMLP->OperatorId).KeysId)].SD+"\r\nAP="+Cfg->Keys[Cfg->GetKeysNum(Cfg->Operator(XMLP->OperatorId).KeysId)].AP+"\r\nOP="+Cfg->Keys[Cfg->GetKeysNum(Cfg->Operator(XMLP->OperatorId).KeysId)].OP+("\r\nSESSION="+SessionNumber).c_str()).c_str();
    Temp+="\r\nNUMBER="+XMLP->GetParamValue("NUMBER");
    return Temp;
}

AnsiString THalfPinPayment::GetMessageTextForCheck(bool bFirstCheck,const std::string& existInquiry)
{
    UNREFERENCED_PARAMETER(bFirstCheck);
    if(std::string::npos==existInquiry.find("COMMENT="))
        return "\r\nAMOUNT="+XMLP->GetParamValue("CARD_VALUE")+"\r\nCOMMENT="+XMLP->InitialSessionNum;
    else
        return "\r\nAMOUNT="+XMLP->GetParamValue("CARD_VALUE");
}

void THalfPinPayment::ParseLocation(AnsiString LocationString)
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

        std::auto_ptr <TStringList> FieldForChequeList ( new TStringList() );
        std::auto_ptr <TStringList> UnnamedFieldForChequeList ( new TStringList() );

        CardID = Location->GetParameter("cardid").c_str();
        CardName = Location->GetParameter("cardname").c_str();
        CardValue = GetDouble(Location->GetParameter("cardvalue").c_str());
        ForcedSum = CardValue;

        XMLP->AddParam("NUMBER",Location->GetParameter("field100").c_str());
        XMLP->AddParam("CARD_VALUE",CardValue);

        AFieldsForInterface = Location->GetParameters().c_str();
        AFieldsForCheque = FieldForChequeList->DelimitedText;
        AUnnamedFieldsForCheque = UnnamedFieldForChequeList->DelimitedText;

        MinSum=CardValue;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

AnsiString THalfPinPayment::GetMessageTextForPayment(const std::string& existInquiry)
{
    AnsiString Temp;
    try
    {
        Temp = "\r\nTRANSID="+XMLP->TransId + "\r\nAMOUNT="+XMLP->GetParamValue("AMOUNT")+"\r\nAMOUNT_ALL="+XMLP->GetParamValue("AMOUNT_ALL");
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

AnsiString THalfPinPayment::getRentCommision()
{
    double rent = boost::lexical_cast<double>(XMLP->GetParamValue("AMOUNT_ALL").c_str()) - GetPaymentMinSum();
    if(rent < 0)
        rent = 0;
    AnsiString ret = (boost::format("%.2f") % rent).str().c_str();
    XMLP->AddParam("RENT_COMMISION",ret);
    return ret;
}

bool THalfPinPayment::Process()
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

        if (AnsiString(Cfg->Operator(XMLP->OperatorId).ProcessorType.c_str()).LowerCase()!="half_pin")
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
                    XMLP->NextTryDT = TDateTime::CurrentDateTime() + double(5 * min(12, XMLP->NumOfTries)) / 24 / 60;
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

bool THalfPinPayment::Payment(bool bProcessMessages, bool bSendStatusChange)
{
    return TPayment::Payment(bProcessMessages,bSendStatusChange);
}

THalfPinPayment::~THalfPinPayment()
{
}
