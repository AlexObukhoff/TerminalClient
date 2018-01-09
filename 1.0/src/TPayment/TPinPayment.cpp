#include <string>
//---------------------------------------------------------------------------
#pragma hdrstop
#include "TPinPayment.h"
#include "globals.h"
#include <boost\format.hpp>
#include <boost\lexical_cast.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------

TPinPayment::TPinPayment(AnsiString _fileName, TWConfig *_Cfg, TLogClass *_Log, TFileMap* _FileMap, TXMLInfo* _InfoFile)
: TPayment(_fileName, _Cfg, _Log, _FileMap, _InfoFile)
{
    try
    {
        PaymentAlreadySent = false;
        SavedPaymentPkt = NULL;
        SavedPaymentPkt = new TPaymentPacket(Cfg, Log);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

TPinPayment::~TPinPayment()
{
    if (SavedPaymentPkt)
    {
        delete SavedPaymentPkt;
        SavedPaymentPkt = NULL;
    }
}
//---------------------------------------------------------------------------

/*bool TPinPayment::Process()
{
return true;
}*/

//---------------------------------------------------------------------------


AnsiString TPinPayment::GetCardsInfo(int Operator)
{
    UNREFERENCED_PARAMETER(Operator);
    AnsiString CardList="NULL";
    try
    {
        Log->Write("  GetCardsInfo. ");
        XMLP->OperatorId = Recepient;
        //MessageText="SD="+Cfg->KeysInfo->SD+"\r\nAP="+Cfg->KeysInfo->AP+"\r\nOP="+Cfg->KeysInfo->OP+"\r\n";
        MessageText=("SD="+Cfg->Keys[Cfg->GetKeysNum(Cfg->Operator(XMLP->OperatorId).KeysId)].SD+"\r\nAP="+Cfg->Keys[Cfg->GetKeysNum(Cfg->Operator(XMLP->OperatorId).KeysId)].AP+"\r\nOP="+Cfg->Keys[Cfg->GetKeysNum(Cfg->Operator(XMLP->OperatorId).KeysId)].OP+"\r\n").c_str();
        std::string mvno_id = Cfg->Operator(XMLP->OperatorId).MNVO_ID;
        if(!mvno_id.empty())
            MessageText += (boost::format("MNVO_ID=%1%") % mvno_id).str().c_str();
        Log->Append((boost::format("Message: %1%") % PrepareString(MessageText).c_str()).str().c_str());
        AnsiString GetCardsResult=Connect(Cfg->Operator(Recepient).GetCardsURL.c_str());
        std::auto_ptr <TStringList> slResult ( new TStringList() );
        if (!slResult.get())
        {
            Log->Write("  slResult creating Error. ");
        }
        else
        {
            Log->Write(("Answer from server:"+GetCardsResult).c_str());
            PrepareAnswer(GetCardsResult,slResult.get());
            Log->Write((boost::format("Answer: %1%") % GetCardsResult.SubString(0,GetCardsResult.Pos(",\"\",\"END\",") - 1).c_str()).str().c_str());

            if (HasAnswerValue(slResult.get(),"CARD_LIST"))
                CardList = GetAnswerValue(slResult.get(),"CARD_LIST");
            Log->Write((boost::format("CardList: %1%") % CardList.c_str()).str().c_str());
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return CardList;
}

//---------------------------------------------------------------------------

void TPinPayment::ParseLocation(AnsiString LocationString)
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

        XMLP->AddParam("NUMBER",CardID);

/*		for (int i=0; i<Cfg->Operator(Recepient)->Fields.size();i++)
			{
			if (Cfg->Operator(Recepient)->Fields[i].Type != "secretword")
				if (Cfg->Operator(Recepient)->Fields[i].Type == "enum")
					{
					FieldForChequeList->Add(Cfg->Operator(Recepient)->Fields[i].Name+"="+Cfg->Operator(Recepient)->Fields[i].GetEnumText(GetParameter("field"+Cfg->Operator(Recepient)->Fields[i].Id,Location)));
					UnnamedFieldForChequeList->Add("FIELD"+Cfg->Operator(Recepient)->Fields[i].Id+"="+Cfg->Operator(Recepient)->Fields[i].GetEnumText(GetParameter("field"+Cfg->Operator(Recepient)->Fields[i].Id,Location)));
					}
					else
					{
					FieldForChequeList->Add(Cfg->Operator(Recepient)->Fields[i].Name+"="+Mask(GetParameter("field"+Cfg->Operator(Recepient)->Fields[i].Id,Location),Cfg->Operator(Recepient)->Fields[i].Mask));
					UnnamedFieldForChequeList->Add("FIELD"+Cfg->Operator(Recepient)->Fields[i].Id+"="+Mask(GetParameter("field"+Cfg->Operator(Recepient)->Fields[i].Id,Location),Cfg->Operator(Recepient)->Fields[i].Mask));
					}
			}*/

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

//---------------------------------------------------------------------------

std::string TPinPayment::GetAmountYE()
{
    std::vector<std::string> tokens;
    std::string strtmp = Cfg->Operator(Recepient).ACardsInfo;
    std::map<std::string, std::string> Parameters;
    std::string separator = ":";
    std::string amount_ye = XMLP->GetParamValue("AMOUNT_YE").c_str();

    if("" == amount_ye)
    {
        while(true)
        {
            if(std::string::npos == strtmp.find(separator))
            {
                tokens.push_back(strtmp);
                break;
            }

            tokens.push_back(strtmp.substr(0, strtmp.find(separator)));
            strtmp = strtmp.substr(strtmp.find(separator) + 1,strtmp.length());
        }

        strtmp = "";

        separator = "=";
        std::vector<std::string> tokensVal;
        for(std::size_t i = 0; i<tokens.size() ; i++)
        {
            strtmp = tokens[i];
            tokensVal.clear();

            while(true)
            {
                if(std::string::npos == strtmp.find(separator))
                {
                    tokensVal.push_back(strtmp);
                    break;
                }

                tokensVal.push_back(strtmp.substr(0, strtmp.find(separator)));
                strtmp = strtmp.substr(strtmp.find(separator) + 1,strtmp.length());
            }
            if(tokensVal.size() == 5)
            {
                if(CardID.c_str() == tokensVal[1]) // id пина
                    amount_ye = tokensVal[3];
            }
        }
    }

    return amount_ye;
}

AnsiString TPinPayment::GetMessageTextForCheck(bool bFirstCheck,const std::string& existInquiry)
{
    try
    {
        if (bFirstCheck)
        {
            AnsiString Temp=FloatToStrF(CardValue,ffFixed,18,2);
            AnsiString Temp_ALL=FloatToStrF(CardValue+Cfg->GetComission(XMLP->OperatorId,CardValue,PaymentCreateDT),ffFixed,18,2);
            if (Temp.Pos(",") != 0)
                Temp = Temp.SubString(0,Temp.Pos(",")-1) + "." + Temp.SubString(Temp.Pos(",") + 1,Temp.Length());
            if (Temp_ALL.Pos(",") != 0)
                Temp_ALL = Temp_ALL.SubString(0, Temp_ALL.Pos(",")-1) + "." + Temp_ALL.SubString(Temp_ALL.Pos(",") + 1, Temp_ALL.Length());

            AnsiString ProcessorType=AnsiString(Cfg->Operator(Recepient).ProcessorType.c_str()).LowerCase();
            if(ProcessorType == "cyberplat_pin_trans")
            {
                std::string amount_ye = GetAmountYE();
                if("" != amount_ye)
                    Temp_ALL += ("\r\nAMOUNT_YE=" + amount_ye).c_str();
            }
            return "\r\nAMOUNT="+Temp+"\r\nAMOUNT_ALL="+Temp_ALL;
//    return "\r\nAMOUNT="+Temp+"\r\nREQ_TYPE=1";
        }
        else
        {
            if(std::string::npos==existInquiry.find("COMMENT="))
                return "\r\nCOMMENT="+XMLP->InitialSessionNum;
            else
                return "";
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return "";
    }
}

AnsiString TPinPayment::GetMessageTextForPayment(const std::string& existInquiry)
{
    AnsiString res = TPayment::GetMessageTextForPayment(existInquiry);
    AnsiString ProcessorType=AnsiString(Cfg->Operator(Recepient).ProcessorType.c_str()).LowerCase();
    if(ProcessorType == "cyberplat_pin_trans")
    {
        std::string amount_ye = GetAmountYE();
        if("" != amount_ye)
        {
            res += ("\r\nAMOUNT_YE=" + amount_ye).c_str();
        }
    }

    return res;
}



//---------------------------------------------------------------------------

bool TPinPayment::InitDone(PaymentDoneCmd Command)
{
    bool bResult = false;
    AnsiString Temp;
    try
    {
        for (int i=0;i<XMLP->Params->Count;i++)
            Temp+="\r\n"+XMLP->GetParamName(i)+"="+XMLP->GetParamValue(i);
        if (Command==cnPDCmdCancel)
        {
            bResult=false;
            RetryAllowed=false;
            if ((Sum<GetPaymentMinSum())&&(Sum>0))
            {
                Log->Write((boost::format("Payment sum is too low (%1%) to process it!") % GetPaymentMinSum()).str().c_str());
                PaymentErrorCode = 7;
                if (!bResult)
                {
                    if (UseSavedFile)
                    {
                        InitPS->StorePaymentInitTemp(TDateTime::CurrentDateTime(), XMLP->OperatorId, Session, (GetComission()*100)/Sum, XMLP->vNotes, AFieldsForInterface);
                    }
                    PS->StorePaymentStatusChange(TDateTime::CurrentDateTime(), Session, 0,PaymentErrorCode);
                    PS->CreatePacket();
                    PaymentErrorCode = PCCTimedOut;
                }
            }
            Log->Write((boost::format("Initializing PIN payment complete %1%.") % (bResult ? "successfully" : (boost::format("with error #%1%") % PaymentErrorCode).str().c_str())).str().c_str());
            PS->StorePaymentComplete(TDateTime::CurrentDateTime(), Session, Session, PaymentErrorCode, 0, XMLP->FirstTryDT, XMLP->OperatorId, Sum, GetComission(),Temp);
        }
        else
        {
            if (UseSavedFile)
            {
                Session = SavedPaymentPkt->InitialSessionNum;
                XMLP->InitialSessionNum = Session;
                PaymentErrorCode = 0;
                AUnnamedFieldsForCheque = SavedPaymentPkt->SavedData;
                for (int i=0;i<SavedPaymentPkt->Params->Count;i++)
                    Temp+="\r\n"+SavedPaymentPkt->GetParamName(i)+"="+SavedPaymentPkt->GetParamValue(i);
                //bResult = false;
                Log->Write((boost::format("Initializing PIN payment complete %1%." ) % (bResult ? "successfully" : (boost::format("with error #%1%") % PaymentErrorCode).str().c_str())).str().c_str());
                InitPS->StorePaymentInitTemp(TDateTime::CurrentDateTime(), XMLP->OperatorId, Session, (GetComission()*100)/Sum, XMLP->vNotes, AFieldsForInterface);
                PS->StorePaymentComplete(TDateTime::CurrentDateTime(), Session, Session, PaymentErrorCode, 0, XMLP->FirstTryDT, XMLP->OperatorId, Sum, GetComission(),Temp);
                //LP->SavedData = "";
                SavedPaymentPkt->DeleteTempFile();
                bResult = true;
            }
            else
            {
                RetryAllowed = true;
                bResult = TPayment::InitDone(Command);
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return bResult;
}

//---------------------------------------------------------------------------

void TPinPayment::SetSum(double Sum)
{
    try
    {
        int validSum = ((Sum < GetLimMax()) ? Sum : GetLimMax());
        float Amount=Sum-Cfg->GetComission(XMLP->OperatorId, validSum, PaymentCreateDT);

        XMLP->AddParam("AMOUNT_ALL", Sum);
        XMLP->AddParam("AMOUNT", Amount);
        if (CardValue<=Amount)
            XMLP->AddParam("AMOUNT",CardValue);
        if (!XMLP->SaveToTempFile())
            FileMap->WriteErrorFound = true;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TPinPayment::PostProcessFirstCheck(int _ResultCode, int _ErrorCode)
{
    try
    {
        if ((_ResultCode==1)&&(_ErrorCode==23))
        {
            Log->Write((boost::format("Deleting %1% card info...") % CardName.c_str()).str().c_str());
            AnsiString Temp = Cfg->DeleteCardInfo(Recepient,CardID);
            Cfg->SetCardsInfo(Recepient,Temp,-1);
            InfoFile->Write("CardsInfo","Op"+AnsiString(Recepient),Temp);
            InfoFile->Write("CardsInfo","Op"+AnsiString(Recepient)+"DT",AnsiString(-1));
            Log->Append("Done.");
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

/*void TPinPayment::PostProcessPayment(int _ResultCode, int _ErrorCode)
{
}*/

//---------------------------------------------------------------------------

AnsiString TPinPayment::GetErrorDescr(int val)
{
    AnsiString res;
    if (val ==33)
        res = "Нет доступных номиналов.";
    else
        res = TPayment::GetErrorDescr(val);
    return res;
}

//---------------------------------------------------------------------------

/*bool TPinPayment::IsTimedOut()
{
return true;
}*/

//---------------------------------------------------------------------------

int TPinPayment::GetStatusWithPIN()
{
    int iError=-1;
    int iResult=-1;
    try
    {
        if ((!XMLP)||(!PS))
            return false;

        Log->Write("  GetStatusWithPIN. ");
        MessageText=("AP="+Cfg->Keys[Cfg->GetKeysNum(Cfg->Operator(XMLP->OperatorId).KeysId)].AP+"\r\nOP="+Cfg->Keys[Cfg->GetKeysNum(Cfg->Operator(XMLP->OperatorId).KeysId)].OP+("\r\nSESSION="+Session+"\r\nTRANSID="+XMLP->TransId+"\r\nPIN_DATA=1").c_str()).c_str();
        Log->Append((boost::format("Message: %1%") % PrepareString(MessageText).c_str()).str().c_str());
        AnsiString StatusResult;
        StatusResult=Connect(Cfg->Operator(XMLP->OperatorId).StatusURL.c_str(),false);
        std::auto_ptr <TStringList> slResult ( new TStringList() );
        if (!slResult.get())
        {
            Log->Write("  slResult creating Error. ");
        }
        else
        {
            PrepareAnswer(StatusResult,slResult.get());
            ParseAnswer(slResult.get());

            iResult = GetAnswerIntegerValue(slResult.get(),"RESULT");
            iError = GetAnswerIntegerValue(slResult.get(),"ERROR");

            Log->Write((boost::format("  %1%") % FormatStatusAnswerForLog(AnsiString("Status"), slResult.get(), iResult, iError).c_str()).str().c_str());
            Log->Append((boost::format(" %1%") % StatusResult.c_str()).str().c_str());

            StatusError=iError;
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return iResult;
}

//---------------------------------------------------------------------------

AnsiString TPinPayment::AutoGetCard(int OpId, AnsiString CardId, AnsiString CardName, AnsiString CardSum, AnsiString CardDataToSave)
{
    AnsiString Result;
    bool res;
    try
    {
        Log->Write((boost::format("AutoGetCard(%1%, %2%, %3%)") % OpId % CardId.c_str() % CardSum.c_str()).str().c_str());

        MinSum=0;
        Sum=0;

        AnsiString Location = "http://dummy.html?recepient="+AnsiString(OpId)+"&cardname="+CardName+"&cardvalue="+CardSum+"&cardid="+CardId+"";
        
        ParseLocation(Location);

        XMLP->OperatorId=Recepient;
        XMLP->ProcessorType=Cfg->Operator(Recepient).ProcessorType.c_str();
        XMLP->FirstTryDT=TDateTime::CurrentDateTime();
        XMLP->SavedCardString = CardDataToSave;
        res = Check(true,"0");
//  res = true;
        if (res)
        {
            XMLP->PacketFileName = ((Cfg->Dirs.PaymentsOutbound+"\\out-pin_").c_str()+AnsiString(Recepient)+"_"+CardID+"_"+CardSum+"_"+(TDateTime::CurrentDateTime()).FormatString("yymmddhhnnsszzz")+".get").c_str();
            XMLP->GetNewFileName("pkt","pin");
            res = XMLP->CreateFile(XMLP->PacketFileName.c_str());
            XMLP->AddParam("AMOUNT_ALL",CardSum);
            XMLP->AddParam("AMOUNT",CardSum);
            Update();
            if (!res)
            {
                Log->Write("Error creating payment packet!");
                CheckErrorCode = db_errors::errPacket0;
            }
            else
            {
                bool PaymentRes = false;
                int i = 2;
                while ((!PaymentRes)&&(i>0))
                {
                    i--;
                    PaymentRes = Payment(false,false);
                    //PaymentRes = false;//Payment();
                    if (PaymentRes)
                    {
                        XMLP->SavedData = AUnnamedFieldsForCheque;
                        XMLP->SaveToFile();
                        XMLP->RenameTempFile("pin");
                        InitPS->StorePaymentInitTemp(TDateTime::CurrentDateTime(), XMLP->OperatorId, XMLP->InitialSessionNum, 0, XMLP->vNotes, AFieldsForInterface);
                        InitPS->StorePaymentInitComplete();
                        Log->Write("Auto GetCard completed.");
                    }
                    else
                    {
                        Log->Write("Error getting PIN data, checking status...");
                        int status = GetStatusWithPIN();
                        //status = 3;
                        switch (status)
                        {
                            case 0:
                            case 1:
                                XMLP->DeleteTempFile();
                                PaymentRes = true;
                                break;
                            case 7:
                                if (StatusError==0)
                                {
                                    XMLP->SavedData = AUnnamedFieldsForCheque;
                                    XMLP->SaveToFile();
                                    XMLP->RenameTempFile("pin");
                                    InitPS->StorePaymentInitTemp(TDateTime::CurrentDateTime(), XMLP->OperatorId, XMLP->InitialSessionNum, 0, XMLP->vNotes, AFieldsForInterface);
                                    InitPS->StorePaymentInitComplete();
                                    Log->Write("Auto GetCard completed.");
                                }
                                else
                                {
                                    XMLP->DeleteTempFile();
                                }
                                PaymentRes = true;
                                break;
                        }
                    }
                }
                //Log->Write("AFieldsForInterface: "+AFieldsForInterface);
                //Log->Write("AFieldsForCheque: "+AFieldsForCheque);
                //Log->Write("AUnnamedFieldsForCheque: "+AUnnamedFieldsForCheque);
            }
        }
//  this->InitPayment();
        return Result;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return "";
    }
}

//---------------------------------------------------------------------------

AnsiString TPinPayment::CompletePayment()
{
    AnsiString Result;
    try
    {
        Log->Write("CompletePayment()");
        int status = GetStatusWithPIN();
        switch (status)
        {
            case 0:
            case 1:
                Log->Write("Payment is not started - canceling.");
                XMLP->DeleteTempFile();
                break;
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
                Log->Write("Payment in progress - canceling.");
                XMLP->DeleteTempFile();
                break;
            case 7:
                if (StatusError==0)
                {
                    XMLP->SavedData = AUnnamedFieldsForCheque;
                    XMLP->SaveToFile();
                    XMLP->RenameTempFile("pin");
                    InitPS->StorePaymentInitTemp(TDateTime::CurrentDateTime(), XMLP->OperatorId, XMLP->InitialSessionNum, 0, XMLP->vNotes, AFieldsForInterface);
                    InitPS->StorePaymentInitComplete();
                    Log->Write("Auto GetCard completed.");
                }
                else
                {
                    XMLP->DeleteTempFile();
                }
                break;
        }
        return Result;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return "";
    }
}

//---------------------------------------------------------------------------

AnsiString TPinPayment::GetOldestFileByMask(AnsiString FindDir, AnsiString FileNameMask)
{
    try
    {
        TDateTime MinFileCreateDT;
        AnsiString OldestFileName;

        TSearchRec sr;
        int iAttributes = 0;

        if (FindFirst(FindDir+"\\"+FileNameMask, iAttributes, sr) == 0)
        {
            do
            {
                try
                {
                    AnsiString CurrentFileName = FindDir+"\\"+sr.Name;
                    TDateTime CurrentFileDT = FileDateToDateTime(FileAge(CurrentFileName));
                    Log->Write((boost::format("Found file %1%, created %2%.") % sr.Name.c_str() % AnsiString(CurrentFileDT).c_str()).str().c_str());
                    if ((MinFileCreateDT.Val==0)||(CurrentFileDT<MinFileCreateDT))
                    {
                        MinFileCreateDT = CurrentFileDT;
                        OldestFileName = CurrentFileName;
                    }
                }
                catch(...)
                {
                    ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                    Log->Write((boost::format("Error getting '%1%' file") % sr.Name.c_str()).str().c_str());
                }
            }
            while (FindNext(sr) == 0);
            FindClose(sr);
        }
        return OldestFileName;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return "";
    }
}

bool TPinPayment::InitPayment(AnsiString Location)
{
//return TPayment::InitPayment(Location);
    bool res = false;
    try
    {
        try
        {
            UseSavedFile = false;
            AnsiString PinCardID = (TLocationParser(Location.c_str())).GetParameter("cardid").c_str();
            AnsiString PinCardValue = (TLocationParser(Location.c_str())).GetParameter("cardvalue").c_str();
            AnsiString Operator = (TLocationParser(Location.c_str())).GetParameter("recepient").c_str();

            Recepient = GetInt(Operator);

            AnsiString FileNameMask = "out-pin_"+Operator+"_"+PinCardID+"_*.pin";
            AnsiString FileName  = GetOldestFileByMask(Cfg->Dirs.PaymentsOutbound.c_str(),FileNameMask);
            Log->Write((boost::format("Using %1%...") % XMLP->TruncateFileName(FileName).c_str()).str().c_str());

            if ((FileName!="")&&(SavedPaymentPkt))
            {
                if (!SavedPaymentPkt->LoadFromFile(FileName))
                {
                    Log->Write((boost::format("Unable to load %1% packet!") % XMLP->TruncateFileName(FileName).c_str()).str().c_str());
                    PacketLoadError = true;
                }
                else
                {
                    UseSavedFile = true;
                    Session = GetSessionNumber().c_str();
                    Log->Write((boost::format("Temporarily using '%1%' session number...") % Session.c_str()).str().c_str());
                    XMLP->InitialSessionNum = Session;
                    XMLP->OperatorId = Recepient;
        //Session=SavedPaymentPkt->LastSession;
        //Recepient=SavedPaymentPkt->OperatorId;
                }
            }

            if ((UseSavedFile)&&(!PacketLoadError))
            {
                Log->Write("New payment taken from file.");
                MinSum=0;
                Sum=Cfg->Payments.Rest;
                Cfg->Payments.Rest = 0;

                ParseLocation(Location);
                XMLP->FirstTryDT=TDateTime::CurrentDateTime();

                XMLP->PacketFileName = XMLP->GetNewFileName("tmp","out").c_str();
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
            else
            {
                res = TPayment::InitPayment(Location);
            }
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

AnsiString TPinPayment::GetPaymentData(int _OpId, AnsiString CardName)
{
    AnsiString Res;
    try
    {
        try
        {
            AnsiString FileNameMask = "out-pin_"+AnsiString(_OpId)+"_"+CardName+"_*.pin";
            AnsiString FileName  = GetOldestFileByMask(Cfg->Dirs.PaymentsOutbound.c_str(),FileNameMask);
            Log->Write((boost::format("Using %1%...") % XMLP->TruncateFileName(FileName).c_str()).str().c_str());

            if ((FileName!="")&&(XMLP))
            {
                if (XMLP->LoadFromFile(FileName))
                {
                    Res = XMLP->SavedCardString;
                }
            }
        }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        }
    }
    __finally
    {
        return Res;
    }
}

bool TPinPayment::Payment(bool bProcessMessages, bool bSendStatusChange)
{
    AnsiString Result;
    try
    {
        if (PaymentAlreadySent)
        {
            Log->Write("Payment already sent, checking status...");
            int status = GetStatusWithPIN();
            switch (status)
            {
                case 0:
                case 1:
                    Log->Write("Payment is not started - trying one more time...");
                    return TPayment::Payment(bProcessMessages, bSendStatusChange);
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                    Log->Write("Payment in progress - waiting for completion.");
                    return false;
                case 7:
                    if (StatusError==0)
                    {
                        Log->Write("Payment completed successfully.");
                        return true;
                    }
                    else
                    {
                        Log->Write("Payment completed with error!");
                        return false;
                    }
                default:
                    Log->Write("Error getting payment status!");
                    return false;
            }
        }
        else
        {
            PaymentAlreadySent = true;
            return TPayment::Payment(bProcessMessages, bSendStatusChange);
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

