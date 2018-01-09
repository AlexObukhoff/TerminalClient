//---------------------------------------------------------------------------
#pragma hdrstop
#include "TAviaPayment.h"
#include <string>
#include "globals.h"
#include "boost/format.hpp"
#include "boost/lexical_cast.hpp"

//---------------------------------------------------------------------------
#pragma package(smart_init)

TAviaPayment::TAviaPayment(AnsiString _fileName, TWConfig *_Cfg, TLogClass *_Log, TFileMap* _FileMap, TXMLInfo* _InfoFile)
: TPayment(_fileName, _Cfg, _Log, _FileMap, _InfoFile)
{
    m_answer = "";

    try
    {
        RetryAllowed = true;
        XMLP->AddParam("TOTALAMT",0);
    }
    catch(...)
    {
        PacketLoadError = true;
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

AnsiString TAviaPayment::GetMessageText(AnsiString SessionNumber)
{
    AnsiString Temp;
    AnsiString CRLF="\r\n";
    Temp+=("SD="+Cfg->Keys[Cfg->GetKeysNum(Cfg->Operator(XMLP->OperatorId).KeysId)].SD+"\r\nAP="+Cfg->Keys[Cfg->GetKeysNum(Cfg->Operator(XMLP->OperatorId).KeysId)].AP+"\r\nOP="+Cfg->Keys[Cfg->GetKeysNum(Cfg->Operator(XMLP->OperatorId).KeysId)].OP+("\r\nSESSION="+SessionNumber).c_str()).c_str();
    Temp+=CRLF+"NUMBER="+XMLP->GetParamValue("NUMBER").UpperCase();
    //Temp+=CRLF+"AMOUNT="+XMLP->GetParamValue("TOTALAMT");
    if(XMLP->GetParamValue("TOTALAMT")=="0.00")
        Temp+=CRLF+"AMOUNT="+XMLP->GetParamValue("AMOUNT");
    else
        Temp+=CRLF+"AMOUNT="+XMLP->GetParamValue("TOTALAMT");

    Temp+="\r\nACCOUNT="+XMLP->GetParamValue("ACCOUNT");

    return Temp;
}

AnsiString TAviaPayment::GetMessageTextForCheck(bool bFirstCheck,const std::string& existInquiry)
{
    UNREFERENCED_PARAMETER(existInquiry);
    return (bFirstCheck) ? "\r\nREQ_TYPE=1\r\nMETHOD=CHECK" : "\r\nMETHOD=CHECK";
}

bool TAviaPayment::Check(bool bFirstCheck, AnsiString AForcedOffline)
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

        if(XMLP->LastSession=="")
            NewSession = GetSessionNumber().c_str();
        else
            NewSession = XMLP->LastSession;

        MessageText=GetMessageText(NewSession);
        MessageText+=GetMessageTextForCheck(bFirstCheck,MessageText.c_str());

        Log->Append((boost::format("Message: %1%") % PrepareString(MessageText).c_str()).str().c_str());
        AnsiString CheckResult=Connect(Cfg->Operator(XMLP->OperatorId).CheckURL.c_str(),true);

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

            if (bFirstCheck)
                PostProcessFirstCheck(iResult,iError);

            if ((0==iResult && 0==iError) || (7==iError && 1==iResult))
            {
                bResult=true;
                Session=GetAnswerValue(slResult.get(),"SESSION");
                if (Session=="")
                    Session=NewSession;

                m_answer=URLDecode(GetAnswerValue(slResult.get(),"ADDINFO")).c_str();

                MinSum=atof(GetAnswerValue(slResult.get(),"TOTALAMOUNT").c_str());
                if(0==MinSum)
                    return false;
                XMLP->SetParamValue("TOTALAMT",MinSum);
                //MinSum+=Cfg->GetComission(XMLP->OperatorId,MinSum,PaymentCreateDT);

                if (!bFirstCheck)
                {
                    if (!XMLP->SaveToFile())
                    {
                        Log->Write("if (!XMLP->SaveToFile())");
                        FileMap->WriteErrorFound = true;
                        bResult = false;
                        iResult = -1;
                        iError = -1;
                    }
                }
                else
                {
                    XMLP->InitialSessionNum=Session;
                }
            }
            else if(33==iError && 1==iResult)
            {
                m_answer=URLDecode(GetAnswerValue(slResult.get(),"ADDINFO")).c_str();
            }

            Log->Write((boost::format("%1%") % URLDecode(FormatAnswerForLog("Check",slResult.get(),iResult,iError)).c_str()).str().c_str());
            Log->Append((boost::format("%1% ") % URLDecode(CheckResult).c_str()).str().c_str());

            if ((!bFirstCheck)&&(iResult!=-1)&&(iError!=-1)&&(iResult!=0)&&(iError!=0)&&(iError!=XMLP->LastErrorCode))
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
    return bResult;
}

double TAviaPayment::GetPaymentMinSum()
{
    return (ForcedSum+MinSum);
}

const char* TAviaPayment::GetAnswer()
{
    return m_answer.c_str();
}

bool TAviaPayment::Payment(bool bProcessMessages, bool bSendStatusChange)
{
    UNREFERENCED_PARAMETER(bSendStatusChange);
    UNREFERENCED_PARAMETER(bProcessMessages);

    if(XMLP->LastSession=="")
    {
        if(!Check(false,NULL))
            return false;
        XMLP->LastSession=Session;
        XMLP->SavedData="CHECK";
    }

    if(XMLP->SavedData!="PAY")
    {
        m_answer = "";
        Session = XMLP->LastSession;
        MessageText = GetMessageText(Session);
        MessageText += "\r\nAMOUNT_ALL="+XMLP->GetParamValue("AMOUNT_ALL");
        if(!Pay())
        {
            return false;
        }
        XMLP->SavedData = "PAY";
    }

    if(m_answer=="")
    {
        GetStatus(true);
        if(11==XMLP->LastErrorCode)
            XMLP->LastSession="";
        if(m_answer=="")//Данные для печати на чеке не получены
            return false;
    }
    return true;
}

bool TAviaPayment::Pay()
{
    int iError = -1;
    int iResult = 0;
    bool bResult = false;
    try
    {
        if ((!XMLP)||(!PS))
            return false;

        Log->Write("  Payment.");
        Log->Append((boost::format("Message: %1%") % PrepareString(MessageText).c_str()).str().c_str());
        AnsiString PaymentResult;

        PaymentResult = Connect(Cfg->Operator(XMLP->OperatorId).PaymentURL.c_str(),true);

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
            catch(...)
            {
                PaymentProcessedDT=0;
            }

            PostProcessPayment(iResult,iError);

            if ((iResult==0)&&(iError==0))
                bResult=true;

            AnsiString strtmp=URLDecode(FormatAnswerForLog("Payment",slResult.get(),iResult,iError));
            Log->Write((boost::format("%1%") % strtmp.c_str()).str().c_str());
            Log->Append((boost::format("%1%") % URLDecode(PaymentResult).c_str()).str().c_str());

            if (iResult!=0 && iError!=0 && iError!=XMLP->LastErrorCode)
            {
                XMLP->LastErrorCode = iError;

                if (!XMLP->SaveToFile())
                    FileMap->WriteErrorFound = true;
                PS->StorePaymentStatusChange(TDateTime::CurrentDateTime(), XMLP->InitialSessionNum, 0,iError);
            }
            else
            {
                m_answer="";

                int CHECK=1;
                AnsiString warning=URLDecode(GetAnswerValue(slResult.get(),"WARNING"));
                while(true)
                {
                    if(GetAnswerValue(slResult.get(),(boost::format("CHECK_%1%") % CHECK).str().c_str()) == "")
                        break;
                    m_answer += URLDecode(GetAnswerValue(slResult.get(),(boost::format("CHECK_%1%") % CHECK).str().c_str())).c_str();
                    CHECK++;
                }
                if(m_answer!="")
                {
                    iResult=7;
                    AnsiString Temp="";
                    for (int i=0;i<XMLP->Params->Count;i++)
                        Temp+="\r\n"+XMLP->GetParamName(i)+"="+XMLP->GetParamValue(i);
                    PS->StorePaymentComplete(TDateTime::CurrentDateTime(), XMLP->InitialSessionNum, Session, iError, XMLP->LastErrorCode, XMLP->FirstTryDT, XMLP->OperatorId, Sum, GetComission(),Temp);
                }
                XMLP->SaveToFile();
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    if(bResult)
    {
        StoreDeltaMoney();
    }
    else
    {
        XMLP->LastSession="";
        XMLP->SavedData="";
        Session="";
        m_answer="";
    }
    return bResult;
}

bool TAviaPayment::Process()
{
    Log->Append((boost::format("%1% try") % (XMLP->NumOfTries+1)).str().c_str());
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

        if(XMLP->LastSession=="")
        {
            Session = GetSessionNumber().c_str();
            if(!Check(false,NULL))
                return false;

            XMLP->LastSession=Session;
            XMLP->SavedData="CHECK";

            if((atof(XMLP->GetParamValue("TOTALAMT").c_str()) - atof(XMLP->GetParamValue("AMOUNT").c_str())) > 0.001)
            {
                Log->Write("Not enough money, sending CHECK for statistic");//Внесли недостаточну сумму, отправили запрос CHECK для сохранения логов
                if(7==XMLP->LastErrorCode)
                {
                    XMLP->SaveToFile();
                    XMLP->CloseFile();
                    return true;
                }
            }
        }
        Session=XMLP->LastSession;
        if((atof(XMLP->GetParamValue("TOTALAMT").c_str()) - atof(XMLP->GetParamValue("AMOUNT").c_str())) > 0.001)
            return true;

        if(XMLP->SavedData!="PAY")
        {
            m_answer="";
            MessageText=GetMessageText(Session);
            MessageText+="\r\n"+XMLP->GetParamValue("AMOUNT");
            if(!Pay())
            {
                XMLP->NumOfTries++;
                XMLP->NextTryDT=TDateTime::CurrentDateTime()+double(5)/24/60;
                return false;
            }
            XMLP->SavedData="PAY";
        }
        if(m_answer!="")
            return true;

        if(m_answer=="")//Данные для печати на чеке не получены
        {
            GetStatus(true);
            if(m_answer=="")
            {
                Log->Write("Not have ANSWER, next try after 5 minute.");
                XMLP->NumOfTries++;
                XMLP->NextTryDT=TDateTime::CurrentDateTime()+double(5)/24/60;
                return false;
            }
        }
        Log->Write("Payment done...");
        return true;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

int TAviaPayment::GetStatus(bool StatPacketSendingAllowed, bool bTestingOnly)
{
    int iError=-1;
    int iResult=-1;
    StatusError=0;
    AnsiString Temp;
    TStringList* slResult=NULL;

    try
    {
        if (!StatPacketSendingAllowed)
            Log->Write("Stat packets sending is not allowed.");

        if (!bTestingOnly)
            Log->Write("  Getting payment status... ");
        if ((!XMLP)||(!PS))
            return -2;

        Session=XMLP->LastSession;
        MessageText="SESSION="+Session+"\r\nTRANSID="+XMLP->TransId;
        if (!bTestingOnly)
            Log->Append((boost::format("Message: %1%") % PrepareString(MessageText).c_str()).str().c_str());
        AnsiString StatusResult=Connect(Cfg->Operator(XMLP->OperatorId).StatusURL.c_str(),true);

        if (bTestingOnly)
        {
            Log->Write((boost::format("CheckConnection result: %1%") % ConnectResult.c_str()).str().c_str());
        }
        else
        {
            slResult=new TStringList();
            PrepareAnswer(StatusResult,slResult);

            iResult = GetAnswerIntegerValue(slResult,"RESULT");
            iError = GetAnswerIntegerValue(slResult,"ERROR");

            Log->Write((boost::format("%1%") % URLDecode(FormatStatusAnswerForLog(AnsiString("Status"),slResult,iResult,iError)).c_str()).str().c_str());
            Log->Append((boost::format("%1%") % URLDecode(StatusResult).c_str()).str().c_str());

            m_answer="";
            int CHECK=1;
            while(true)
            {
                if(GetAnswerValue(slResult,(boost::format("CHECK_%1%") % CHECK).str().c_str())=="")
                    break;
                m_answer+=URLDecode(GetAnswerValue(slResult,(boost::format("CHECK_%1%") % CHECK).str().c_str())).c_str();
                CHECK++;
            }
            delete slResult;
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
        if(m_answer!="")
        {
            iResult=7;
            AnsiString Temp="";
            for (int i=0;i<XMLP->Params->Count;i++)
                Temp+="\r\n"+XMLP->GetParamName(i)+"="+XMLP->GetParamValue(i);
            PS->StorePaymentComplete(TDateTime::CurrentDateTime(), XMLP->InitialSessionNum, Session, iError, XMLP->LastErrorCode, XMLP->FirstTryDT, XMLP->OperatorId, Sum, GetComission(),Temp);
        }
        XMLP->SaveToFile();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return iResult;
}

void TAviaPayment::Update()
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
        if (!XMLP->SaveToFile())
            FileMap->WriteErrorFound = true;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

std::string TAviaPayment::GetAnswerForCheque()
{
    std::string buffer = "";
    int posEnd=0,posStart=0;

    while(std::string::npos!=m_answer.find("\n",posEnd))
    {
        posStart=posEnd;
        posEnd=m_answer.find("\n",posEnd)+1;
        std::string strtmp=m_answer.substr(posStart,posEnd-posStart);
        while(strtmp.length()>Cfg->Peripherals.Printer.TapeLength)
        {
            buffer+=strtmp.substr(0,Cfg->Peripherals.Printer.TapeLength);
            if(std::string::npos!=buffer.find("\t"))
                buffer[buffer.find("\t")]=' ';
            strtmp=strtmp.substr(Cfg->Peripherals.Printer.TapeLength);
            buffer+="\r\n";
        }
        buffer+=strtmp;
    }

    posEnd=0;
    posStart=0;
    std::string result="";
    while(std::string::npos!=buffer.find("\n",posEnd))
    {
        posStart=posEnd;
        posEnd=buffer.find("\n",posEnd)+1;
        std::string strtmp=buffer.substr(posStart,posEnd-posStart);
        if(std::string::npos!=strtmp.find("\t"))
        {
            result+=strtmp.substr(0,strtmp.find("\t")).c_str();
            result+=" ";

            int tmp=1;// \n уже есть
            if(std::string::npos!=strtmp.find("\r"))
                tmp++;
            int spaces=Cfg->Peripherals.Printer.TapeLength-strtmp.length()+tmp;
            for(int i=0;i<spaces;i++)
                result+=" ";
            result+=strtmp.substr(strtmp.find("\t")+1);
        }
        else
        {
            result+=strtmp.c_str();
        }
    }

    return result;
}

void TAviaPayment::StoreDeltaMoney()
{
    double needPay=GetDouble(XMLP->GetParamValue("TOTALAMT"));
    double pay=GetDouble(XMLP->GetParamValue("AMOUNT"));
    double delta=pay-needPay;

    if(delta<=0)
        return;

    TPaymentPacket dXMLP(Cfg, Log);
    dXMLP.PaymentCreateDT = PaymentCreateDT;
    dXMLP.OperatorId=XMLP->OperatorId;
    dXMLP.ProcessorType=XMLP->ProcessorType;
    dXMLP.FirstTryDT=XMLP->FirstTryDT;
    dXMLP.PacketFileName = ((Cfg->Dirs.PaymentsUnprocessed+"\\out-").c_str()+Session+".pkt").c_str();
    dXMLP.AddParam("AMOUNT",delta);
    dXMLP.AddParam("field100",XMLP->GetParamValue("field100"));
    dXMLP.CreateFile(dXMLP.PacketFileName.c_str());
    dXMLP.SaveToFile();
}

void TAviaPayment::SetAnswer()
{
    m_answer="1";
}

double TAviaPayment::GetComission()
{
    //double comm = 0;
    if(XMLP->GetParamValue("AMOUNT") != "" && XMLP->GetParamValue("TOTALAMT") != "")
    {
        if(boost::lexical_cast<double>(XMLP->GetParamValue("AMOUNT").c_str()) >= boost::lexical_cast<double>(XMLP->GetParamValue("TOTALAMT").c_str()))
            XMLP->AddParam("AMOUNT", XMLP->GetParamValue("TOTALAMT").c_str());
        //comm = boost::lexical_cast<double>(XMLP->GetParamValue("AMOUNT").c_str()) - boost::lexical_cast<double>(XMLP->GetParamValue("TOTALAMT").c_str());
        //if(comm < 0)
        //    comm = 0;
    }
    //return comm;

    return 0;
}
