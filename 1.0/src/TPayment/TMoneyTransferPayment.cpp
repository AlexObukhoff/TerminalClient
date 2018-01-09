#include <memory>
#include <fstream>
#include "TMoneyTransferPayment.h"
#include "TLocationParser.h"
#include "JSONDocument.h"
#include "globals.h"
#include "CryptLib2.h"
#pragma hdrstop
#pragma package(smart_init)

TMoneyTransferPayment::TMoneyTransferPayment(AnsiString _fileName, TWConfig *_Cfg, TLogClass *_Log, TFileMap* _FileMap, TXMLInfo* _InfoFile)
: TPayment(_fileName, _Cfg, _Log, _FileMap, _InfoFile), cWaitFlagTimeoutSecond(60)
{
    CS = new TCriticalSection();
    m_errorsListFileName = Localization.getLocalePath() + "mt_ga_errors_list.js";
    m_amount_all = "";
    m_amount = "";
    m_system_commission = "";
    m_rent_commission = "";
    RetryAllowed = false;
    LastErrorCode = 0;

    m_IdName[810] = "руб";
    m_IdName[840] = "USD";
    m_IdName[978] = "EUR";

    cEmpty = "";
    cSendTransfer = "SEND_TRANSFER";
    cSendTransferConfirm = "SEND_TRANSFER_CONFIRM";
    cCancel = "CANCEL";
}

TMoneyTransferPayment::~TMoneyTransferPayment()
{
}

void TMoneyTransferPayment::ParseLocation(AnsiString LocationString)
{
    try
    {
        TLocationParser Location (LocationString.c_str());
        if (Location.HasParameter("recepient"))
        {
            this->Recepient = GetInt(Location.GetParameter("recepient").c_str());
        }
        else
        {
            Log->Write("Error: can't get recepient info!");
            return;
        }

        XMLP->AddParam("currency",Cfg->Operator(Recepient).CurrencyId.c_str());

        XMLP->AddParam("si_id",SenderInfo.id.c_str());
        XMLP->AddParam("si_last_name",SenderInfo.last_name.c_str());
        XMLP->AddParam("si_first_name",SenderInfo.first_name.c_str());
        XMLP->AddParam("si_middle_name",SenderInfo.middle_name.c_str());
        XMLP->AddParam("si_birth_date",SenderInfo.birth_date.c_str());
        XMLP->AddParam("si_birth_place",SenderInfo.birth_place.c_str());
        XMLP->AddParam("si_country",SenderInfo.country.c_str());
        XMLP->AddParam("si_resident",SenderInfo.resident.c_str());
        XMLP->AddParam("si_phone",SenderInfo.phone.c_str());
        XMLP->AddParam("si_email",SenderInfo.email.c_str());
        XMLP->AddParam("si_mobile",SenderInfo.mobile.c_str());
        XMLP->AddParam("si_address",SenderInfo.address.c_str());

        XMLP->AddParam("si_id_type", SenderIdInfo.type.c_str());
        XMLP->AddParam("si_id_serial", SenderIdInfo.serial.c_str());
        XMLP->AddParam("si_id_number", SenderIdInfo.number.c_str());
        XMLP->AddParam("si_id_issue_date", SenderIdInfo.issue_date.c_str());
        XMLP->AddParam("si_id_country", SenderIdInfo.country.c_str());
        XMLP->AddParam("si_id_authority", SenderIdInfo.authority.c_str());
        XMLP->AddParam("si_id_authority_code", SenderIdInfo.authority_code.c_str());

        int keyNum = Cfg->GetKeysNum(Cfg->Operator(Recepient).KeysId);
        std::string cryptedNumber = crypt::encrypt(keyNum, LoginInfo.CardNumber.c_str()).c_str();

        XMLP->AddParam("number", cryptedNumber.c_str());

        if (Location.HasParameter("recepient_mt"))
        {
            _recipient_info ri=LastRecipients[boost::lexical_cast<int>(Location.GetParameter("recepient_mt").c_str())];
            this->TransferInfo.recipient_bank_id = ri.recipient_bank_id;
            this->RecipientInfo.last_name = ri.last_name;
            this->RecipientInfo.first_name = ri.first_name;
            this->RecipientInfo.middle_name = ri.middle_name;
            this->RecipientInfo.email = ri.email;
            this->RecipientInfo.mobile = ri.mobile;
            this->RecipientInfo.country = ri.country;
            this->RecipientInfo.resident = ri.resident;
            this->RecipientInfo.bank_address = ri.bank_address;
            this->RecipientInfo.mt_system_name = ri.mt_system_name;
            this->RecipientInfo.currencyId = ri.currencyId;
            this->RecipientInfo.id = ri.id;

            XMLP->AddParam("ri_id",ri.id.c_str());
            XMLP->AddParam("ri_recipient_bank_id",ri.recipient_bank_id.c_str());
            XMLP->AddParam("ri_last_name",ri.last_name.c_str());
            XMLP->AddParam("ri_first_name",ri.first_name.c_str());
            XMLP->AddParam("ri_middle_name",ri.middle_name.c_str());
            XMLP->AddParam("ri_email",ri.email.c_str());
            XMLP->AddParam("ri_mobile",ri.mobile.c_str());
            XMLP->AddParam("ri_country",ri.country.c_str());
            XMLP->AddParam("ri_resident",ri.resident.c_str());
            XMLP->AddParam("ri_bank_address",ri.bank_address.c_str());
            XMLP->AddParam("ri_mt_system_name",ri.mt_system_name.c_str());
            XMLP->AddParam("ri_mt_system",ri.mt_system.c_str());
            XMLP->AddParamI("ri_currency_id",ri.currencyId);
            AFieldsForInterface=(boost::format("recepient=%1%&field100=%2%") % Recepient % (ri.first_name+" "+ri.middle_name+" "+ri.last_name)).str().c_str();
        }

        if (Location.HasParameter("amount_total"))
        {
            this->TransferInfo.amount_total = double(GetInt(Location.GetParameter("amount_total").c_str()))/100;
            XMLP->AddParam("AMOUNT",this->TransferInfo.amount_total);
            XMLP->AddParam("AMOUNT_ALL",this->TransferInfo.amount_total);
        }
        else
        {
            Log->Write("Not amount and amount_total prePay");
        }
    }
    catch(...)
    {
        LastErrorCode=-1;
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TMoneyTransferPayment::setRecepientID(int Recepient)
{
    this->Recepient = XMLP->OperatorId = Recepient;
}

void TMoneyTransferPayment::saveConnect(const XMLMessageType type,const double sum,AnsiString* result)
{
    HANDLE hMutex = NULL;
    try
    {
        double sleepSec = 0;
        *result = "";
        #ifdef __CONN__
            Log->Write("Geting token from CONN");
        #else
            Log->Write("Geting token from WC");
        #endif
        while(true)
        {
            hMutex = ::CreateMutex(NULL, TRUE, "MoneyTransferMutex");
            DWORD lastError = ::GetLastError();
            if(ERROR_SUCCESS == lastError)
            {
                #ifdef __CONN__
                    Log->Write("CONN, token done");
                #else
                    Log->Write("WC, token done");
                #endif
                MessageText = MakeXMLMessage(type,sum).c_str();
                flushall();
                Log->Write(MessageText.c_str());
                *result = Connect(Cfg->Operator(Recepient).LoginURL.c_str(),true);
                break;
            }
            else if(ERROR_ALREADY_EXISTS == lastError)
            {
                ReleaseMutex(hMutex);
                CloseHandle(hMutex);
                hMutex = NULL;
            }
            else
            {
                throw std::runtime_error((boost::format("Can not create Mutex, last error: %1%") % lastError).str().c_str());
            }

            Sleep(500);
            #ifdef __CONN__
                Log->Write((boost::format("CONN, wait token %1% sec") % sleepSec).str().c_str());
            #else
                Log->Write((boost::format("WC, wait token %1% sec") % sleepSec).str().c_str());
            #endif
            sleepSec += 0.5;
            /*
            if(sleepSec > cWaitFlagTimeoutSecond)
            {
                #ifdef __CONN__
                    Log->Write("Timeout, from CONN, break");
                #else
                    Log->Write("Timeout, token from WC, break");
                #endif
                FileMap->incrementMoneyTransferToken = false;
                break;
            }
            */
        }
        FileMap->incrementMoneyTransferToken = false;
    }
    catch(...)
    {
        if(NULL != hMutex)
        {
            ReleaseMutex(hMutex);
            CloseHandle(hMutex);
            hMutex = NULL;
            #ifdef __CONN__
                Log->Write("Release token, from CONN");
            #else
                Log->Write("Release token from WC");
            #endif
        }
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    if(NULL != hMutex)
    {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        hMutex = NULL;
        #ifdef __CONN__
            Log->Write("Release token, from CONN");
        #else
            Log->Write("Release token from WC");
        #endif
    }
}

bool TMoneyTransferPayment::PreInit(int OperatorId, AnsiString Login, AnsiString Password)
{
    #ifdef __CONN__
    return true;// для conn логинится не надо, надо только отправить перевод
    #endif
    try
    {
        LoginInfo.CardNumber = Login.c_str();
        LoginInfo.PasswordMD5 = Password.c_str();
        LoginInfo.NewPasswordMD5 = "NewPassword";
        XMLP->OperatorId = Recepient = OperatorId;

        if(!GetErrorsList())
            Log->Write("Can not get ErrorList!");

        AnsiString LoginResult;
        saveConnect(mtMTLogin,0,&LoginResult);
        /*
        MessageText = MakeXMLMessage(mtMTLogin).c_str();
        Log->Write(MessageText.c_str());
        AnsiString LoginResult;
        LoginResult = Connect(Cfg->Operator(Recepient).LoginURL.c_str(),true);
        */

        if (ParseXMLAnswer(LoginResult, mtMTLogin,true))
        {
            if(LastErrorCode==0)
                MakePostLoginJSFile();
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        LastErrorCode=-1;
    }

    return (LastErrorCode==0);
}

bool TMoneyTransferPayment::ChangePassword(const char* Login,const char* OldPassword,const std::string NewPassword,const std::string NewPassword1)
{
    if(NewPassword != NewPassword1)
    {
        LastErrorCode = 815;
        return false;
    }
    try
    {
        int keyNum = Cfg->GetKeysNum(Cfg->Operator(Recepient).KeysId);
        std::string cryptedNumber = crypt::encrypt(keyNum, Login).c_str();
        LoginInfo.CardNumber = cryptedNumber;

        if (OldPassword)
        {
            std::string cryptedOldNumber = crypt::encrypt(keyNum, OldPassword).c_str();
            LoginInfo.PasswordMD5 = cryptedOldNumber;
        }
        std::string cryptedNewPassword = crypt::encrypt(keyNum, NewPassword).c_str();
        LoginInfo.NewPasswordMD5 = cryptedNewPassword;
        AnsiString result;
        saveConnect(mtMTChangePassword,0,&result);
        MessageText = MakeXMLMessage(mtMTChangePassword).c_str();
        Log->Write(MessageText.c_str());
        AnsiString Result = Connect(Cfg->Operator(Recepient).LoginURL.c_str(),true);
        ParseXMLAnswer(result, mtMTChangePassword,true);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        LastErrorCode=-1;
    }
    return LastErrorCode == 0;
}

bool TMoneyTransferPayment::Check(bool bFirstCheck, AnsiString AForcedOffline)
{
    UNREFERENCED_PARAMETER(bFirstCheck);
    Log->Write("Check");
    try
    {
        XMLP->LastErrorCode=-1;
        if(Session=="")
        {
            Session=GetSessionNumber().c_str();
            XMLP->InitialSessionNum=Session.c_str();
        }
        double amountAll=10;

        if(XMLP->GetParamValue("AMOUNT_ALL")!="")
            amountAll=boost::lexical_cast<double>(XMLP->GetParamValue("AMOUNT_ALL").c_str());
        else if(Cfg->Operator(Recepient).CheckAmount > 0)
            amountAll=Cfg->Operator(Recepient).CheckAmount;

        AnsiString result;
        saveConnect(mtMTCalculateTransferSumsAmountAll,amountAll,&result);
        ParseXMLAnswer(result, mtMTCalculateTransferSumsAmountAll,true);
        /*
        MessageText = MakeXMLMessage(mtMTCalculateTransferSumsAmountAll,amountAll).c_str();
        Log->Write(("CalculateTransferSums: "+MessageText).c_str());
        AnsiString CalcResult = Connect(Cfg->Operator(Recepient).LoginURL.c_str(),true);
        ParseXMLAnswer(CalcResult, mtMTCalculateTransferSumsAmountAll,true);
        */
        XMLP->AddParam("ri_cur_valute",Cfg->CurrencyInfo.CurrencyName.c_str());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        LastErrorCode=-1;
    }
    return LastErrorCode == 0;
}

bool TMoneyTransferPayment::Payment(bool bProcessMessages, bool bSendStatusChange)
{
    UNREFERENCED_PARAMETER(bSendStatusChange);

    if(XMLP->SavedData != cSendTransferConfirm)
    {
        if(!sendTransfer(bProcessMessages))
            return false;
    }

    bool res = sendTransferConfirm(bProcessMessages);
    if(res)
        XMLP->SavedData == cSendTransferConfirm;
    return res;
}

bool TMoneyTransferPayment::sendTransfer(bool bProcessMessages)
{
    bool res=true;
    XMLP->LastErrorCode=-1;
    Log->Write("sendTransfer");

    if(XMLP->SavedData == cSendTransferConfirm)//Все данные уже есть
        return true;
    try
    {
        AnsiString result;
        saveConnect(mtMTCalculateTransferSumsAmountAll,boost::lexical_cast<double>(XMLP->GetParamValue("AMOUNT_ALL").c_str()),&result);
        ParseXMLAnswer(result, mtMTCalculateTransferSumsAmountAll,true);
        /*
        MessageText = MakeXMLMessage(mtMTCalculateTransferSumsAmountAll,boost::lexical_cast<double>(XMLP->GetParamValue("AMOUNT_ALL").c_str())).c_str();
        Log->Write(("Payment: "+MessageText).c_str());
        AnsiString PaymentResult;
        PaymentResult = Connect(Cfg->Operator(Recepient).LoginURL.c_str(),true);
        ParseXMLAnswer(PaymentResult, mtMTCalculateTransferSumsAmountAll,true);
        */
        PaymentErrorCode = LastErrorCode;
        XMLP->LastErrorCode = LastErrorCode;
        if(0 != LastErrorCode)
            return false;

        XMLP->AddParam("ri_amount",(boost::format("%.2f") % boost::lexical_cast<double>(m_amount)).str().c_str());
        XMLP->AddParam("ri_amount_total",(boost::format("%.2f") % boost::lexical_cast<double>(m_amount_all)).str().c_str());
        XMLP->AddParam("ri_system_commission",(boost::format("%.2f") % boost::lexical_cast<double>(m_system_commission)).str().c_str());
        XMLP->AddParam("ri_rent_commission",(boost::format("%.2f") % (boost::lexical_cast<double>(m_rent_commission)) ).str().c_str());
        //zh_sp
        if(Cfg->Operator(Recepient).fix)
        {
          Cfg->Payments.Rest = boost::lexical_cast<double>(m_rent_commission);
          Cfg->Payments.rState = eYes;
          Log->Write((boost::format("Payment rest: %1%") % Cfg->Payments.Rest).str().c_str());
        }
        else
        {
          Cfg->Payments.Rest = 0;
        }
        //zh_sp

        XMLP->AddParam("ri_rec_amount",(boost::format("%.2f") % boost::lexical_cast<double>(m_rec_amount)).str().c_str());
        XMLP->AddParam("ri_rec_amount_total",(boost::format("%.2f") % boost::lexical_cast<double>(m_rec_amount_all)).str().c_str());
        XMLP->AddParam("ri_rec_system_commission",(boost::format("%.2f") % boost::lexical_cast<double>(m_rec_system_commission)).str().c_str());

        saveConnect(mtMTCheckPaymentInfo,boost::lexical_cast<double>(m_amount_all),&result);
        ParseXMLAnswer(result, mtMTProcessPayment,true);
        /*
        MessageText = MakeXMLMessage(mtMTCheckPaymentInfo,boost::lexical_cast<double>(m_amount_all)).c_str();
        Log->Write(("Payment: "+MessageText).c_str());
        PaymentResult = "";
        PaymentResult = Connect(Cfg->Operator(Recepient).LoginURL.c_str(),bProcessMessages);
        ParseXMLAnswer(PaymentResult, mtMTProcessPayment,true);
        */
        PaymentErrorCode = LastErrorCode;
        XMLP->LastErrorCode = LastErrorCode;
        if(0!=LastErrorCode)
            return false;

        XMLP->AddParam("ri_amount",(boost::format("%.2f") % boost::lexical_cast<double>(m_amount)).str().c_str());
        XMLP->AddParam("ri_amount_total",(boost::format("%.2f") % boost::lexical_cast<double>(m_amount_all)).str().c_str());
        XMLP->AddParam("ri_system_commission",(boost::format("%.2f") % boost::lexical_cast<double>(m_system_commission)).str().c_str());
        XMLP->AddParam("ri_rent_commission",(boost::format("%.2f") % boost::lexical_cast<double>(m_rent_commission)).str().c_str());

        XMLP->SavedData = cSendTransfer;
        XMLP->SaveToFile();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        LastErrorCode=-1;
        res=false;
    }
    return res;
}

bool TMoneyTransferPayment::sendTransferConfirm(bool bProcessMessages)
{
    bool res = true;
    XMLP->LastErrorCode = -1;
    Log->Write("sendTransferConfirm");
    try
    {
        AnsiString result;
        saveConnect(mtMTProcessPayment,boost::lexical_cast<double>(XMLP->GetParamValue("ri_amount_total").c_str()),&result);
        XMLP->SavedData = cSendTransferConfirm;
        XMLP->SaveToFile();

        ParseXMLAnswer(result, mtMTProcessPayment,true);
        /*
        MessageText = MakeXMLMessage(mtMTProcessPayment,boost::lexical_cast<double>(XMLP->GetParamValue("ri_amount_total").c_str())).c_str();
        Log->Write(("Payment: " + MessageText).c_str());
        XMLP->SavedData = cSendTransferConfirm;
        XMLP->SaveToFile();
        PaymentResult = Connect(Cfg->Operator(Recepient).LoginURL.c_str(),bProcessMessages);
        ParseXMLAnswer(PaymentResult, mtMTProcessPayment,true);
        */
        PaymentErrorCode = LastErrorCode;
        XMLP->LastErrorCode = LastErrorCode;

        if(0 == LastErrorCode || 821 == LastErrorCode)
            res = true;
        else
            res = false;
        XMLP->SaveToFile();
    }
    catch(...)
    {
        XMLP->SavedData = cSendTransferConfirm;
        XMLP->SaveToFile();
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        LastErrorCode = -1;
        res = false;
    }
    return res;
}

bool TMoneyTransferPayment::Process()
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

        AnsiString Temp;
        if(XMLP->SavedData == cCancel)
        {
            if(XMLP->GetParamValue("trans_id")!="")//Если поcлали запрос для статиститки, то сохраняем пакает
            {
                Log->Write("Process Done");
                for (int i=0;i<XMLP->Params->Count;i++)
                    Temp+="\r\n"+XMLP->GetParamName(i)+"="+XMLP->GetParamValue(i);
                XMLP->LastSession=XMLP->GetParamValue("trans_id");
                PS->StorePaymentComplete(TDateTime::CurrentDateTime(), XMLP->InitialSessionNum, XMLP->GetParamValue("trans_id"), 0, 30, XMLP->FirstTryDT, XMLP->OperatorId, Sum, 0,Temp);
                if(!RenamePacketsToCanceled())
                {
                    XMLP->NumOfTries++;
                    XMLP->NextTryDT = TDateTime::CurrentDateTime() + double(5*min(12,XMLP->NumOfTries))/24/60;
                    XMLP->SaveToFile();
                    return false;
                }
                return true;
            }
            if(sendTransfer(true))//Посылаем запрос на сервер для статистики
            {
                Log->Write("Process Done");
                for (int i=0;i<XMLP->Params->Count;i++)
                    Temp+="\r\n"+XMLP->GetParamName(i)+"="+XMLP->GetParamValue(i);
                XMLP->LastSession=XMLP->GetParamValue("trans_id");
                PS->StorePaymentComplete(TDateTime::CurrentDateTime(), XMLP->InitialSessionNum, XMLP->GetParamValue("trans_id"), 0, 30, XMLP->FirstTryDT, XMLP->OperatorId, Sum, 0,Temp);
                if(!RenamePacketsToCanceled())
                {
                    XMLP->NumOfTries++;
                    XMLP->NextTryDT = TDateTime::CurrentDateTime() + double(5*min(12,XMLP->NumOfTries))/24/60;
                    XMLP->SaveToFile();
                    return false;
                }
                return true;
            }
            else//Ничего не получилось пробуем еще раз
            {
                Log->Write("Process Error, must send statistic information");
                XMLP->NumOfTries++;
                XMLP->NextTryDT = TDateTime::CurrentDateTime() + double(5*min(12,XMLP->NumOfTries))/24/60;
                XMLP->SaveToFile();
                return false;
            }
        }
        if(XMLP->SavedData == cSendTransferConfirm && 0 == XMLP->LastErrorCode)
        {
            for (int i=0;i<XMLP->Params->Count;i++)
                Temp+="\r\n"+XMLP->GetParamName(i)+"="+XMLP->GetParamValue(i);
            XMLP->LastSession=XMLP->GetParamValue("trans_id");
            PS->StorePaymentComplete(TDateTime::CurrentDateTime(), XMLP->InitialSessionNum, XMLP->GetParamValue("trans_id"), 0, 7, XMLP->FirstTryDT, XMLP->OperatorId, Sum, 0,Temp);
            return true;
        }

        if(XMLP->SavedData == cEmpty) // Не прошел sendTransfer попробуем еще раз
        {
            if(!sendTransfer(true))
            {
                XMLP->NumOfTries++;
                XMLP->NextTryDT = TDateTime::CurrentDateTime() + double(5*min(12,XMLP->NumOfTries))/24/60;
                XMLP->SaveToFile();
                return false;
            }
        }

        if(XMLP->SavedData == cSendTransfer || XMLP->SavedData == cSendTransferConfirm)
        {
            if(sendTransferConfirm(true))
            {
                XMLP->SavedData = cSendTransferConfirm;
                for (int i=0; i<XMLP->Params->Count; i++)
                    Temp += "\r\n" + XMLP->GetParamName(i) + "=" + XMLP->GetParamValue(i);
                XMLP->LastSession = XMLP->GetParamValue("trans_id");
                PS->StorePaymentComplete(TDateTime::CurrentDateTime(), XMLP->InitialSessionNum, XMLP->GetParamValue("trans_id"), 0, 7, XMLP->FirstTryDT, XMLP->OperatorId, Sum, 0,Temp);
                Log->Write("Process Done, Status Good");
                return true;
            }
            else
            {
                XMLP->NumOfTries++;
                XMLP->NextTryDT = TDateTime::CurrentDateTime() + double(5*min(12,XMLP->NumOfTries))/24/60;
                XMLP->SaveToFile();
                Log->Write("Process error, payment failed");
                return false;
            }
        }
        Log->Write(("Unknown savedData: " + XMLP->SavedData).c_str());
        XMLP->NextTryDT = TDateTime::CurrentDateTime() + double(60)/24/60;
        XMLP->SaveToFile();
        return false;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

void TMoneyTransferPayment::getMTStatus(xmlGuard <_di_IXMLNode> NdContent)
{
    xmlGuard <_di_IXMLNodeList> ContentNDL (NdContent->GetChildNodes());
    if (!ContentNDL.Assigned())
        throw Exception((AnsiString)"Bad answer format: error getting content child nodes!");

    xmlGuard <_di_IXMLNode> Transfer (ContentNDL->FindNode("transfer"));
    if (!Transfer.Assigned())
        throw Exception((AnsiString)"Bad answer format: no <transfer> node found!");

    AnsiString strtmp;
    GetNodeText(Transfer,"status",strtmp);
    statusCode=boost::lexical_cast<int>(strtmp.c_str());
}

bool TMoneyTransferPayment::GetErrorsList()
{
    try
    {
        CS->Enter(); // Специально для WC когда раз в час произсходи запрос списка ошибок
        bool res;

        AnsiString result;
        saveConnect(mtGetErrorsList,0,&result);
        /*
        MessageText = MakeXMLMessage(mtGetErrorsList).c_str();
        Log->Write(("GetErrorsList: "+MessageText).c_str());
        AnsiString GetErrorsListResult=Connect(Cfg->Operator(Recepient).LoginURL.c_str(),true);
        res=ParseXMLAnswer(GetErrorsListResult, mtGetErrorsList,true);
        */
        ParseXMLAnswer(result, mtGetErrorsList,true);
        if (LastErrorCode == 0 || !FileExists(m_errorsListFileName.c_str()))
            MakePostGetErrorsListJSFile();
        Log->Write((boost::format("Received %1% error messages.") % ErrorMessages.size()).str().c_str());

        //if (LastErrorCode == 0 && !res)
        //    DeleteFile((m_errorsListFileName.substr(0,m_errorsListFileName.rfind("."))+".xml").c_str());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        LastErrorCode=-1;
    }
    CS->Leave();
    return LastErrorCode == 0;
}

std::string TMoneyTransferPayment::MakeXMLMessage(XMLMessageType MessageType,double sum)
{
    AnsiString ARes;
    try
    {
        xmlGuard <_di_IXMLDocument> dXML(NewXMLDocument ());

        dXML->XML->Clear();
        dXML->Active = true;
        dXML->NodeIndentStr="  ";
        dXML->Encoding = "utf-8";
        dXML->Options = dXML->Options << doNodeAutoIndent;

        xmlGuard <_di_IXMLNode> Root(dXML->AddChild("money_transfers"));
        Root->SetAttribute("version",AnsiString("2.0"));

        AddTextNode(Root, "date", GetUTCDateTimeString(TDateTime::CurrentDateTime()));
        AddTextNode(Root, "ap", Cfg->Keys[Cfg->GetKeysNum(Cfg->Operator(XMLP->OperatorId).KeysId)].AP.c_str());
        AddTextNode(Root, "op", Cfg->Keys[Cfg->GetKeysNum(Cfg->Operator(XMLP->OperatorId).KeysId)].OP.c_str());

        //InfoFile->reopen();
        long token = FileMap->moneyTransferToken;
        #ifdef __CONN__
            Log->Write((boost::format("CONN token number: %1%") % token).str().c_str());
        #else
            Log->Write((boost::format("WC token number: %1%") % token).str().c_str());
        #endif
        token++;
        InfoFile->Write("MoneyTransfer","token",token);
        InfoFile->Write("MoneyTransfer","DateUseToken",TDateTime::CurrentDateTime());
        FileMap->moneyTransferToken = token;
        //InfoFile->reopen();
        #ifdef __CONN__
            Log->Write((boost::format("CONN, now token number: %1%") % InfoFile->Read("MoneyTransfer","token").c_str()).str().c_str());
        #else
            Log->Write((boost::format("WC, now token number: %1%") % InfoFile->Read("MoneyTransfer","token").c_str()).str().c_str());
        #endif
        AddTextNode(Root, "token", token);

        xmlGuard <_di_IXMLNode> NdContent;
        xmlGuard <_di_IXMLNode> ndTransfer;
        switch (MessageType)
        {
            case mtMTLogin:
                AddTextNode(Root, "request_type", GetMessageTypeString(MessageType).c_str());
                NdContent = Root->AddChild("content");
                MakeMTLoginContent(NdContent);
                break;
            case mtMTChangePassword:
                AddTextNode(Root, "request_type", GetMessageTypeString(MessageType).c_str());
                NdContent = Root->AddChild("content");
                MakeMTChangePasswordContent(NdContent);
                break;
            case mtMTCalculateTransferSumsAmount:
                AddTextNode(Root, "request_type", GetMessageTypeString(MessageType).c_str());
                NdContent = Root->AddChild("content");
                MakeCalculateTransferSumsContentAmount(NdContent,sum);
                break;
            case mtMTCalculateTransferSumsAmountAll:
                AddTextNode(Root, "request_type", GetMessageTypeString(MessageType).c_str());
                NdContent = Root->AddChild("content");
                MakeCalculateTransferSumsContentAmountAll(NdContent,sum);
                break;
            case mtMTCheckPaymentInfo:
            case mtMTProcessPayment:
                AddTextNode(Root, "request_type", GetMessageTypeString(MessageType).c_str());
                NdContent = Root->AddChild("content");
                MakeMTPaymentInfoContent(NdContent, MessageType);
                break;
            case mtGetErrorsList:
                AddTextNode(Root, "request_type", GetMessageTypeString(MessageType).c_str());
                NdContent = Root->AddChild("content");
                AddTextNode(NdContent, "time_stamp", GetTimeStamp((m_errorsListFileName.substr(0,m_errorsListFileName.rfind("."))+".xml").c_str()));
                AddTextNode(NdContent, "lang", Localization.getLocale().c_str());
                break;
            default:
                AddTextNode(Root, "request_type", ("UNDEFINED_"+AnsiString(int(MessageType))).c_str());
                NdContent = Root->AddChild("content");
        }

        dXML->SaveToXML(ARes);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        LastErrorCode=-1;
    }
    return ARes.c_str();
}

AnsiString TMoneyTransferPayment::GetTimeStamp(const char* FileName)
{
    AnsiString timeStamp = "0";
    if (!FileExists(FileName))
        return timeStamp;

    xmlGuard<_di_IXMLDocument> dXML(NULL);
    AnsiString FileData;
    bool bRes = ReadAll(FileName, FileData);
    if (!bRes)
        return timeStamp;

    try
    {
        dXML = LoadXMLData(FileData);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        DeleteFile(FileName);
        return timeStamp;
    }

    if (!dXML.Assigned())
    {
        DeleteFile(FileName);
        return timeStamp;
    }

    dXML->Active = true;

    xmlGuard <_di_IXMLNode> Root (dXML->GetDocumentElement());
    if (!Root.Assigned())
    {
        DeleteFile(FileName);
        return timeStamp;
    }

    if (Root->NodeName!=AnsiString("money_transfers"))
    {
        DeleteFile(FileName);
        return timeStamp;
    }

    xmlGuard <_di_IXMLNodeList> RootNDL (Root->GetChildNodes());
    if (!RootNDL.Assigned())
    {
        DeleteFile(FileName);
        return timeStamp;
    }

    xmlGuard <_di_IXMLNode> Content (RootNDL->FindNode("content"));
    if (!Content.Assigned())
    {
        DeleteFile(FileName);
        return timeStamp;
    }

    xmlGuard <_di_IXMLNodeList> ContentNDL (Content->GetChildNodes());
    if (!ContentNDL.Assigned())
    {
        DeleteFile(FileName);
        return timeStamp;
    }

    xmlGuard <_di_IXMLNode> tsND (ContentNDL->FindNode("time_stamp"));
    if (!tsND.Assigned())
    {
        DeleteFile(FileName);
        return timeStamp;
    }
    else
    {
        timeStamp = AnsiString(tsND->Text).c_str();
    }

    xmlGuard <_di_IXMLNode> NdRowData (ContentNDL->FindNode("rowdata"));
    if (!NdRowData.Assigned())
    {
        DeleteFile(FileName);
        return timeStamp;
    }

    xmlGuard <_di_IXMLNodeList> RowDataNDL (NdRowData->GetChildNodes());
    if (RowDataNDL.Assigned())
    {
        AnsiString strtmp;

        for (int i=0; i<RowDataNDL->Count; i++)
        {
            xmlGuard<_di_IXMLNode> NdRow(RowDataNDL->Get(i));
            if ((NdRow->NodeName == AnsiString("row")) && (NdRow->HasAttribute("id")))
            {
                _error_message_info NewErrorMessageInfo;
                int id = AnsiString(NdRow->GetAttribute("id")).ToInt();
                if (NdRow->HasAttribute("lang"))
                    NewErrorMessageInfo.Language = AnsiString(NdRow->GetAttribute("lang")).c_str();
                if (NdRow->HasAttribute("system_message"))
                {
                    strtmp = (AnsiString)NdRow->GetAttribute("system_message");
                    NewErrorMessageInfo.SystemMessage = strtmp.c_str();
                }
                if (NdRow->HasAttribute("user_message"))
                {
                    strtmp = (AnsiString)NdRow->GetAttribute("user_message");
                    NewErrorMessageInfo.UserMessage = strtmp.c_str();
                }
                ErrorMessages[id] = NewErrorMessageInfo;
            }
        }
    }

    return timeStamp;
}

bool TMoneyTransferPayment::ReadAll(const char* FileName, AnsiString &FileData)
{
    TFileStream* PktFile = new TFileStream(FileName, fmOpenReadWrite | fmShareDenyNone);
    if (PktFile)
    {
        std::vector<char> Buffer( static_cast<std::size_t>(PktFile->Size+1), 0 );
        PktFile->Seek(0, soFromBeginning);
        PktFile->Read(&*Buffer.begin(), static_cast<std::size_t>(PktFile->Size));
        FileData = AnsiString(&*Buffer.begin(), static_cast<std::size_t>(PktFile->Size));
        delete PktFile;
        PktFile=NULL;
        return true;
    }
    else
    {
        Log->Write((boost::format("Open file %1% error!") % FileName).str().c_str());
        return false;
    }
}

void TMoneyTransferPayment::MakeMTLoginContent(xmlGuard <_di_IXMLNode> NdContent)
{
    xmlGuard <_di_IXMLNode> ndSenderCard (NdContent->AddChild("bank_card"));

    int keyNum = Cfg->GetKeysNum(Cfg->Operator(Recepient).KeysId);
    std::string cryptedNumber = crypt::encrypt(keyNum, LoginInfo.CardNumber.c_str()).c_str();
    //AddTextNode(ndSenderCard, "number", LoginInfo.CardNumber.c_str());
    AddTextNode(ndSenderCard, "number", cryptedNumber.c_str());
    std::string cryptedPass = crypt::encrypt(keyNum, LoginInfo.PasswordMD5.c_str()).c_str();
    //AddTextNode(ndSenderCard, "password", LoginInfo.PasswordMD5.c_str());
    AddTextNode(ndSenderCard, "password", cryptedPass.c_str());
}

void TMoneyTransferPayment::MakeMTChangePasswordContent(xmlGuard <_di_IXMLNode> NdContent)
{
    xmlGuard <_di_IXMLNode> ndSenderCard (NdContent->AddChild("bank_card"));
    AddTextNode(ndSenderCard, "number", LoginInfo.CardNumber.c_str());
    AddTextNode(ndSenderCard, "password_old", LoginInfo.PasswordMD5.c_str());
    AddTextNode(ndSenderCard, "password_new", LoginInfo.NewPasswordMD5.c_str());
}

void TMoneyTransferPayment::MakeMTPaymentInfoContent(xmlGuard <_di_IXMLNode> NdContent, XMLMessageType MessageType)
{
    xmlGuard <_di_IXMLNode> ndTransfer (NdContent->AddChild("transfer"));
    AddTextNode(ndTransfer, "mtsystem", XMLP->GetParamValue("ri_mt_system").ToInt());

    //inputs amount
    xmlGuard <_di_IXMLNode> ndInputsAmount (ndTransfer->AddChild("input_amounts"));

    double system_commission = boost::lexical_cast<double>(XMLP->GetParamValue("ri_system_commission").c_str());
    double amountTotal = boost::lexical_cast<double>(XMLP->GetParamValue("AMOUNT_ALL").c_str()) - boost::lexical_cast<double>(XMLP->GetParamValue("ri_rent_commission").c_str());
    double ri_amount = boost::lexical_cast<double>(XMLP->GetParamValue("ri_amount").c_str());
    //double ri_amount_total = boost::lexical_cast<double>(XMLP->GetParamValue("ri_amount_total").c_str());

    AddTextNode(ndInputsAmount, "currency", m_rurId);
    AddTextNode(ndInputsAmount, "fee", (boost::format("%.2f") % system_commission).str().c_str());
    AddTextNode(ndInputsAmount, "amount_total", (boost::format("%.2f") % amountTotal).str().c_str());
    AddTextNode(ndInputsAmount, "amount", (boost::format("%.2f") % ri_amount).str().c_str());
    AddTextNode(ndInputsAmount, "change", XMLP->GetParamValue("ri_rent_commission"));

    //send amount
    xmlGuard <_di_IXMLNode> ndSendAmount (ndTransfer->AddChild("send_amounts"));
    AddTextNode(ndSendAmount, "currency", XMLP->GetParamValue("ri_currency_id").c_str());
    double recAmount = boost::lexical_cast<double>(XMLP->GetParamValue("ri_rec_amount").c_str());
    double recAmountAll = boost::lexical_cast<double>(XMLP->GetParamValue("ri_rec_amount_total").c_str());
    double recSystemCommission = boost::lexical_cast<double>(XMLP->GetParamValue("ri_rec_system_commission").c_str());
    AddTextNode(ndSendAmount, "fee",(boost::format("%.2f") % recSystemCommission).str().c_str());
    AddTextNode(ndSendAmount, "amount_total", (boost::format("%.2f") % recAmountAll).str().c_str());
    AddTextNode(ndSendAmount, "amount",(boost::format("%.2f") % recAmount).str().c_str());

    AddTextNode(ndTransfer, "recipient_bank_id", XMLP->GetParamValue("ri_recipient_bank_id").c_str());
    AddTextNode(ndTransfer, "sender_message", "");

    xmlGuard <_di_IXMLNode> ndSender (ndTransfer->AddChild("sender"));
    AddTextNode(ndSender, "id", XMLP->GetParamValue("si_id").c_str());
    AddTextNode(ndSender, "last_name", XMLP->GetParamValue("si_last_name").c_str());
    AddTextNode(ndSender, "first_name", XMLP->GetParamValue("si_first_name").c_str());
    AddTextNode(ndSender, "middle_name", XMLP->GetParamValue("si_middle_name").c_str());
    AddTextNode(ndSender, "birth_date", XMLP->GetParamValue("si_birth_date").c_str());
    AddTextNode(ndSender, "birth_place", XMLP->GetParamValue("si_birth_place").c_str());
    AddTextNode(ndSender, "country", XMLP->GetParamValue("si_country").c_str());
    AddTextNode(ndSender, "address", XMLP->GetParamValue("si_address").c_str());
    AddTextNode(ndSender, "phone", XMLP->GetParamValue("si_phone").c_str());
    AddTextNode(ndSender, "email", XMLP->GetParamValue("si_email").c_str());
    AddTextNode(ndSender, "mobile", XMLP->GetParamValue("si_mobile").c_str());
    AddTextNode(ndSender, "resident", XMLP->GetParamValue("si_resident").c_str());

    xmlGuard <_di_IXMLNode> ndSender_id (ndTransfer->AddChild("document_id"));
    AddTextNode(ndSender_id, "type", XMLP->GetParamValue("si_id_type").c_str());
    AddTextNode(ndSender_id, "serial", XMLP->GetParamValue("si_id_serial").c_str());
    AddTextNode(ndSender_id, "number", XMLP->GetParamValue("si_id_number").c_str());
    AddTextNode(ndSender_id, "issue_date", XMLP->GetParamValue("si_id_issue_date").c_str());
    AddTextNode(ndSender_id, "country", XMLP->GetParamValue("si_id_country").c_str());
    AddTextNode(ndSender_id, "authority", XMLP->GetParamValue("si_id_authority").c_str());
    AddTextNode(ndSender_id, "authority_code", XMLP->GetParamValue("si_id_authority_code").c_str());

    xmlGuard <_di_IXMLNode> ndSenderCard (ndTransfer->AddChild("bank_card"));
    AddTextNode(ndSenderCard, "number", XMLP->GetParamValue("number").c_str());

    xmlGuard <_di_IXMLNode> ndRecipient(ndTransfer->AddChild("recipient"));
    AddTextNode(ndRecipient, "id", XMLP->GetParamValue("ri_id").c_str());
    AddTextNode(ndRecipient, "last_name", XMLP->GetParamValue("ri_last_name").c_str());
    AddTextNode(ndRecipient, "first_name", XMLP->GetParamValue("ri_first_name").c_str());
    AddTextNode(ndRecipient, "middle_name", XMLP->GetParamValue("ri_middle_name").c_str());
    AddTextNode(ndRecipient, "country", XMLP->GetParamValue("ri_country").c_str());
    AddTextNode(ndRecipient, "address", XMLP->GetParamValue("ri_address").c_str());
    AddTextNode(ndRecipient, "email", XMLP->GetParamValue("ri_email").c_str());
    AddTextNode(ndRecipient, "mobile", XMLP->GetParamValue("ri_mobile").c_str());
    AddTextNode(ndRecipient, "phone", XMLP->GetParamValue("ri_phone").c_str());
    AddTextNode(ndRecipient, "resident", XMLP->GetParamValue("ri_resident").c_str());
    AddTextNode(ndTransfer, "exchange_rate",XMLP->GetParamValue("ri_exchange_rate").c_str());

    xmlGuard <_di_IXMLNode> ndReceiveAmounts(ndTransfer->AddChild("receive_amounts"));
    AddTextNode(ndReceiveAmounts,"currency","");
    AddTextNode(ndReceiveAmounts,"amount","");

    xmlGuard <_di_IXMLNode> ndExtraFields(ndTransfer->AddChild("extra_fields"));
    switch (MessageType)
    {
        case mtMTProcessPayment:
            AddTextNode(ndTransfer, "trans_id",XMLP->GetParamValue("trans_id").c_str());
            AddTextNode(ndTransfer, "transfer_code",XMLP->GetParamValue("transfer_code").c_str());
            AddTextNode(ndTransfer, "direction",XMLP->GetParamValue("direction").c_str());
            AddTextNode(ndTransfer, "status",XMLP->GetParamValue("status").c_str());
            AddTextNode(ndTransfer, "date",XMLP->GetParamValue("date").c_str());
            break;
        default:
            AddTextNode(ndTransfer, "trans_id", "");
            AddTextNode(ndTransfer, "transfer_code","");
            break;
    }
}

void TMoneyTransferPayment::MakeCalculateTransferSumsContentAmount(xmlGuard <_di_IXMLNode> NdContent,double summ)
{
    xmlGuard <_di_IXMLNode> ndTransfer (NdContent->AddChild("transfer"));
    AddTextNode(ndTransfer, "mtsystem",XMLP->GetParamValue("ri_mt_system"));
    AddTextNode(ndTransfer, "recipient_bank_id",XMLP->GetParamValue("ri_recipient_bank_id").c_str());

    //input amounts
    xmlGuard <_di_IXMLNode> ndInputAmounts(ndTransfer->AddChild("input_amounts"));
    AddTextNode(ndInputAmounts, "currency",m_rurId);
    if(summ > 0)
        AddTextNode(ndInputAmounts, "amount", ChangeChars(FloatToStrF(summ,ffFixed,18,2),",","."));

    xmlGuard <_di_IXMLNode> ndSendAmounts(ndTransfer->AddChild("send_amounts"));
    AddTextNode(ndSendAmounts, "currency",XMLP->GetParamValue("ri_currency_id"));
}

void TMoneyTransferPayment::MakeCalculateTransferSumsContentAmountAll(xmlGuard <_di_IXMLNode> NdContent,double summ)
{
    xmlGuard <_di_IXMLNode> ndTransfer (NdContent->AddChild("transfer"));
    AddTextNode(ndTransfer, "mtsystem",XMLP->GetParamValue("ri_mt_system"));
    AddTextNode(ndTransfer, "recipient_bank_id",XMLP->GetParamValue("ri_recipient_bank_id").c_str());

    //input amounts
    xmlGuard <_di_IXMLNode> ndInputAmounts(ndTransfer->AddChild("input_amounts"));
    AddTextNode(ndInputAmounts, "currency",m_rurId);
    if(summ > 0)
        AddTextNode(ndInputAmounts, "amount_total", ChangeChars(FloatToStrF(summ,ffFixed,18,2),",","."));
    AddTextNode(ndInputAmounts, "fee","");
    AddTextNode(ndInputAmounts, "amount","");
    AddTextNode(ndInputAmounts, "change", XMLP->GetParamValue("ri_rent_commission"));

    //send amounts
    xmlGuard <_di_IXMLNode> ndSendAmounts(ndTransfer->AddChild("send_amounts"));
    AddTextNode(ndSendAmounts, "currency",XMLP->GetParamValue("ri_currency_id"));

    //receive amounts not use
    xmlGuard <_di_IXMLNode> ndReceiveAmounts(ndTransfer->AddChild("receive_amounts"));
    AddTextNode(ndReceiveAmounts, "currency","");
    AddTextNode(ndReceiveAmounts, "amount","");
    //extra fields not use
    xmlGuard <_di_IXMLNode> ndExtra(ndTransfer->AddChild("extra_fields"));
    AddTextNode(ndExtra, "deliveryOption","");
}

bool TMoneyTransferPayment::ParseXMLAnswer(AnsiString AnswerBody, XMLMessageType MessageType,bool test)
{
    bool bRes = false;
    try
    {
        Log->Write(("Server answer: "+AnswerBody+".").c_str());

        LastErrorCode=-1;

        if (AnswerBody == "")
        {
            LastErrorCode=-2;
            return false;
        }

        xmlGuard <_di_IXMLDocument> dXML (LoadXMLData(AnswerBody));
        if (!dXML.Assigned())
            throw Exception((AnsiString)"Bad answer format: error getting xml document!");

        dXML->Active = true;

        xmlGuard <_di_IXMLNode> Root (dXML->GetDocumentElement());
        if (!Root.Assigned())
            throw Exception((AnsiString)"Bad answer format: no root node found!");

        if (Root->NodeName!=AnsiString("money_transfers"))
            throw Exception((AnsiString)"Bad answer format: root node must have <money_transfers> name!");

        xmlGuard <_di_IXMLNodeList> RootNDL (Root->GetChildNodes());
        if (!RootNDL.Assigned())
            throw Exception((AnsiString)"Bad answer format: error getting root child nodes!");

        xmlGuard <_di_IXMLNode> Error (RootNDL->FindNode("error"));
        if (!Error.Assigned())
            throw Exception((AnsiString)"Bad answer format: no <error> node found!");

        xmlGuard <_di_IXMLNodeList> ErrorNDL (Error->GetChildNodes());
        if (!ErrorNDL.Assigned())
            throw Exception((AnsiString)"Bad answer format: error getting error child nodes!");

        xmlGuard <_di_IXMLNode> CodeND (ErrorNDL->FindNode("code"));
        if (CodeND.Assigned())
            LastErrorCode = AnsiString(CodeND->Text).ToInt();
        else
            throw Exception((AnsiString)"Bad answer format: no <code> node found!");

        Log->Write((boost::format("Error: %1%") % LastErrorCode).str().c_str());

        if (LastErrorCode==0)
        {
            bRes = true;
            xmlGuard <_di_IXMLNode> NdContent (RootNDL->FindNode("content"));
            if(MessageType == mtMTProcessPayment)
            {
                AnsiString strtmp;
                xmlGuard <_di_IXMLNode> NdDate (RootNDL->FindNode("date"));
                GetNodeText(NdDate,"date",strtmp);
                XMLP->AddParam("date",strtmp.c_str());
            }
            switch (MessageType)
            {
                case mtMTLogin:
                    GetMTLoginContent(NdContent);
                    break;
                case mtMTChangePassword:
                    break;
                case mtMTCalculateTransferSumsAmount:
                case mtMTCalculateTransferSumsAmountAll:
                    m_amount_all="";
                    m_amount="";
                    m_system_commission="";
                    GetMTCalculateTransferSumsInfo(NdContent,test);
                    break;
                case mtMTCheckPaymentInfo:
                    GetMTCheckPaymentInfo(NdContent);
                    break;
                case mtMTProcessPayment:
                    GetMTProcessPaymentInfo(NdContent);
                    break;
                case mtGetErrorsList:
                    bRes=GetMTGetErrorsListInfo(NdContent);
                    break;
                default:
                    bRes = false;
                    break;
                  //NdContent = Root->AddChild("content");
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        bRes = false;
        LastErrorCode = -1;
    }
    return bRes;
}

void TMoneyTransferPayment::getErrorChangePasswordContent(xmlGuard <_di_IXMLNode> NdContent)
{
    xmlGuard <_di_IXMLNodeList> ContentNDL (NdContent->GetChildNodes());
    if (!ContentNDL.Assigned())
        throw Exception((AnsiString)"Bad answer format: error getting content child nodes!");

    xmlGuard <_di_IXMLNode> errorNode (ContentNDL->FindNode("error"));
    if (!errorNode.Assigned())
        throw Exception((AnsiString)"Bad answer format: no <error> node found!");

    LastErrorCode=boost::lexical_cast<int>(errorNode->Text.c_bstr());
}

std::string TMoneyTransferPayment::GetMessageTypeString(XMLMessageType MType)
{
    switch (MType)
    {
        case mtMTLogin:
            return "terminal_login";
        case mtMTChangePassword:
            return "change_client_password";
        case mtMTCalculateTransferSumsAmount:
        case mtMTCalculateTransferSumsAmountAll:
            return "calculate_transfer_amounts";
        case mtMTCheckPaymentInfo:
            return "send_transfer_check";
        case mtMTProcessPayment:
            return "send_transfer_confirm";
        case mtGetErrorsList:
            return "get_errors_directory";

        default:
            return "undefined_"+boost::lexical_cast<std::string>(int(MType));
    }
}

void TMoneyTransferPayment::InitCurlLib()
{
    CURL_WC_DLL = NULL;
    CURL_WC_DLL = LoadLibrary(_T("./curl_dll/libcurl.dll"));
    if(!CURL_WC_DLL)
        throw new Exception("libcurl.dll not Load");
    curl_easy_init = NULL;
    curl_easy_cleanup = NULL;
    curl_easy_perform = NULL;
    curl_easy_setopt = NULL;
    curl_easy_strerror = NULL;

    curl_easy_init = (Tcurl_easy_init)GetProcAddress(CURL_WC_DLL, "curl_easy_init");
    curl_easy_cleanup = (Tcurl_easy_cleanup)GetProcAddress(CURL_WC_DLL, "curl_easy_cleanup");
    curl_easy_perform = (Tcurl_easy_perform)GetProcAddress(CURL_WC_DLL, "curl_easy_perform");
    curl_easy_setopt = (Tcurl_easy_setopt)GetProcAddress(CURL_WC_DLL, "curl_easy_setopt");
    curl_easy_strerror = (Tcurl_easy_strerror)GetProcAddress(CURL_WC_DLL, "curl_easy_strerror");
    if(!curl_easy_init || !curl_easy_cleanup || !curl_easy_perform || !curl_easy_setopt || !curl_easy_strerror)
        throw new Exception("Function in libcurl.dll Not Load");
}

size_t write_func(void *ptr, size_t size, size_t nmemb,std::string& s)
{
    size_t realsize = size * nmemb;
    s.append((const char*) ptr,realsize);
    return realsize;
}

bool TMoneyTransferPayment::GetMTGetErrorsListInfo(xmlGuard <_di_IXMLNode> NdContent)
{
    bool retRes=true;
    xmlGuard <_di_IXMLNodeList> ContentNDL (NdContent->GetChildNodes());
    if (!ContentNDL.Assigned())
      throw Exception((AnsiString)"Bad answer format: error getting content child nodes!");

    xmlGuard <_di_IXMLNode> NdDownloadFile (ContentNDL->FindNode("download_file"));
    if (NdDownloadFile.Assigned())
    {
        AnsiString astrtmp=NdDownloadFile->Text;
        std::string strURL=astrtmp.c_str();
        std::string return_result;

        InitCurlLib();
        CURL *curl;
        CURLcode resCode;
        curl = curl_easy_init();
        if(curl)
        {
            std::string fileName=strURL.substr(strURL.rfind("/")+1,strURL.length());
            if("none"!=Cfg->Connection().HTTPProxy.CfgType && ""!=Cfg->Connection().HTTPProxy.CfgType)
            {
                curl_easy_setopt(curl, CURLOPT_PROXY, Cfg->Connection().Proxy->Host.c_str());
                curl_easy_setopt(curl, CURLOPT_PROXYPORT, Cfg->Connection().Proxy->Port);
                if(0!=Cfg->Connection().HTTPProxy.Port)
                    curl_easy_setopt(curl, CURLOPT_PROXYPORT, Cfg->Connection().HTTPProxy.Port);
                if(Cfg->Connection().HTTPProxy.UserName!="")
                {
                    if(""!=Cfg->Connection().HTTPProxy.Password)
                        curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD,(boost::format("%1%:%2%") % Cfg->Connection().HTTPProxy.UserName % Cfg->Connection().HTTPProxy.Password).str().c_str());
                    else
                        curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD,Cfg->Connection().HTTPProxy.UserName.c_str());
                }
                switch(Cfg->Connection().Proxy->Version)
                {
                    case svSocks4:
                        curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
                        break;
                    case svSocks4A:
                        curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4A);
                        break;
                    case svSocks5:
                        curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
                        break;
                    default:
                        curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
                        break;
                }
                if("https"==Cfg->Connection().HTTPProxy.Type)
                {
                    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0 );
                    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0 );
                }
            }
            if(std::string::npos!=strURL.find("https"))
            {
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0 );
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0 );
            }
            curl_easy_setopt(curl, CURLOPT_URL, strURL.c_str());


            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &return_result);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
            //curl_easy_setopt(curl, CURLOPT_ENCODING, "gzip,deflate");
            curl_easy_setopt(curl, CURLOPT_ENCODING, "deflate");

            resCode = curl_easy_perform(curl);
            if(resCode!=CURLE_OK)
            {
                retRes=false;
                Log->Write((boost::format("CURL error code: %1% description: %2%") % resCode % curl_easy_strerror(resCode)).str().c_str());
                curl_easy_cleanup(curl);
                FreeLibrary(CURL_WC_DLL);
                return retRes;
            }
            // always cleanup
            curl_easy_cleanup(curl);

            std::stringstream output( ios_base::in | ios_base::out | ios_base::binary );
            std::stringstream input(return_result, ios_base::in | ios_base::out | ios_base::binary );

            if( GZ::gz_decompress(input,output) )
                return_result = output.str();
        }
        FreeLibrary(CURL_WC_DLL);
        if(!retRes)
            return retRes;

        Log->Write((m_errorsListFileName.substr(0,m_errorsListFileName.rfind("."))+".xml").c_str());
        std::ofstream ofs((m_errorsListFileName.substr(0,m_errorsListFileName.rfind("."))+".xml").c_str());
        ofs << return_result;
        ofs.close();

        AnsiString FileData;
        if(!ReadAll((m_errorsListFileName.substr(0,m_errorsListFileName.rfind("."))+".xml").c_str(), FileData))
            return false;
        xmlGuard <_di_IXMLDocument> dXML (LoadXMLData(FileData));

        if (!dXML.Assigned())
            throw Exception((AnsiString)"Bad answer format: error getting xml document!");

        dXML->Active = true;

        xmlGuard <_di_IXMLNode> Root (dXML->GetDocumentElement());
        if (!Root.Assigned())
            throw Exception((AnsiString)"Bad answer format: no root node found!");

        if (Root->NodeName!=AnsiString("money_transfers"))
            throw Exception((AnsiString)"Bad answer format: root node must have <money_transfers> name!");

        xmlGuard <_di_IXMLNodeList> RootNDL (Root->GetChildNodes());
        if (!RootNDL.Assigned())
            throw Exception((AnsiString)"Bad answer format: error getting root child nodes!");

        xmlGuard <_di_IXMLNode> NdContent2(RootNDL->FindNode("content"));
        if (!NdContent.Assigned())
            throw Exception((AnsiString)"Bad answer format: <root> must have content node!");

        ContentNDL = NdContent2->GetChildNodes();

        if (!ContentNDL.Assigned())
          throw Exception((AnsiString)"Bad answer format: error getting content child nodes!");

        xmlGuard <_di_IXMLNode> NdRowData (ContentNDL->FindNode("rowdata"));
        if (!NdRowData.Assigned())
            throw Exception((AnsiString)"Bad answer format: no <rowdata> node found!");

        xmlGuard <_di_IXMLNodeList> RowDataNDL (NdRowData->GetChildNodes());
        if (RowDataNDL.Assigned())
        {
            AnsiString strtmp;

            for (int i=0; i<RowDataNDL->Count; i++)
            {
                xmlGuard <_di_IXMLNode> NdRow (RowDataNDL->Get(i));
                if ((NdRow->NodeName == AnsiString("row"))&&(NdRow->HasAttribute("id")))
                {
                    _error_message_info NewErrorMessageInfo;
                    int id=AnsiString(NdRow->GetAttribute("id")).ToInt();
                    if (NdRow->HasAttribute("lang"))
                        NewErrorMessageInfo.Language = AnsiString(NdRow->GetAttribute("lang")).c_str();
                    if (NdRow->HasAttribute("system_message"))
                    {
                        strtmp=(AnsiString)NdRow->GetAttribute("system_message");
                        NewErrorMessageInfo.SystemMessage = strtmp.c_str();
                    }
                    if (NdRow->HasAttribute("user_message"))
                    {
                        strtmp=(AnsiString)NdRow->GetAttribute("user_message");
                        NewErrorMessageInfo.UserMessage = strtmp.c_str();
                    }
                    ErrorMessages[id]=NewErrorMessageInfo;
                }
            }
        }
    }
    else
    {
        //ContentNDL = NdContent->GetChildNodes();

        //if (!ContentNDL.Assigned())
        //  throw Exception((AnsiString)"Bad answer format: error getting content child nodes!");

        xmlGuard <_di_IXMLNode> NdRowData (ContentNDL->FindNode("rowdata"));
        if (!NdRowData.Assigned())
            throw Exception((AnsiString)"Bad answer format: no <rowdata> node found!");

        xmlGuard <_di_IXMLNode> NdTimeStamp (ContentNDL->FindNode("time_stamp"));
        if (!NdTimeStamp.Assigned())
            throw Exception((AnsiString)"Bad answer format: no <time_stamp> node found!");

        xmlGuard <_di_IXMLNodeList> RowDataNDL (NdRowData->GetChildNodes());
        if (RowDataNDL.Assigned())
        {
            WideString wstrtmp;
            AnsiString strtmp;
            //bool newDataExist = false;

            if (0 != RowDataNDL->Count)
            {
                xmlGuard<_di_IXMLDocument> dExistFileXML(NULL);
                AnsiString existFileData;
                std::string fileNameErrors = m_errorsListFileName.substr(0, m_errorsListFileName.rfind("."))+".xml";

                bool bRes = ReadAll(fileNameErrors.c_str(), existFileData);
                if (!bRes)
                    return false;

                try
                {
                    dExistFileXML = LoadXMLData(existFileData);
                }
                catch(...)
                {
                    ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                    DeleteFile(fileNameErrors.c_str());
                    return false;
                }

                if (!dExistFileXML.Assigned())
                {
                    DeleteFile(fileNameErrors.c_str());
                    return false;
                }

                dExistFileXML->Active = true;

                xmlGuard <_di_IXMLNode> RootExistFile (dExistFileXML->GetDocumentElement());
                if (!RootExistFile.Assigned())
                {
                    DeleteFile(fileNameErrors.c_str());
                    return false;
                }

                if (RootExistFile->NodeName!=AnsiString("money_transfers"))
                {
                    DeleteFile(fileNameErrors.c_str());
                    return false;
                }

                xmlGuard <_di_IXMLNodeList> RootExistFileNDL (RootExistFile->GetChildNodes());
                if (!RootExistFileNDL.Assigned())
                {
                    DeleteFile(fileNameErrors.c_str());
                    return false;
                }

                xmlGuard <_di_IXMLNode> ContentExistFile (RootExistFileNDL->FindNode("content"));
                if (!ContentExistFile.Assigned())
                {
                    DeleteFile(fileNameErrors.c_str());
                    return false;
                }

                xmlGuard <_di_IXMLNodeList> ContentExistFileNDL (ContentExistFile->GetChildNodes());
                if (!ContentExistFileNDL.Assigned())
                {
                    DeleteFile(fileNameErrors.c_str());
                    return false;
                }

                xmlGuard <_di_IXMLNode> tsExistFileND (ContentExistFileNDL->FindNode("time_stamp"));
                if (!tsExistFileND.Assigned())
                {
                    DeleteFile(fileNameErrors.c_str());
                    return false;
                }
                else
                {
                    tsExistFileND->Text = NdTimeStamp->Text;
                }

                xmlGuard <_di_IXMLNode> NdRowDataExistFile (ContentExistFileNDL->FindNode("rowdata"));
                if (!NdRowDataExistFile.Assigned())
                {
                    DeleteFile(fileNameErrors.c_str());
                    return false;
                }

                xmlGuard <_di_IXMLNodeList> RowDataExistFileNDL (NdRowDataExistFile->GetChildNodes());

                if (RowDataExistFileNDL.Assigned())
                {
                /*
                    // Добавляем новые данные
                    for (int i = 0; i < RowDataNDL->Count; i++)
                    {
                        RowDataExistFileNDL->Add(RowDataNDL->Get(i));
                        //RowDataExistFileNDL->ReplaceNode(RowDataNDL->Get(i));
                    }*/

                    // Обнавляем старые
                    std::map<int, _di_IXMLNode > recieveMessages;
                    for (int i=0; i < RowDataNDL->Count; i++)
                    {
                        _di_IXMLNode NdRow (RowDataNDL->Get(i));
                        recieveMessages[AnsiString(NdRow->GetAttribute("id")).ToInt()] = NdRow;
                    }
                    std::map<int, _di_IXMLNode > existMessages;
                    for (int i=0; i < RowDataExistFileNDL->Count; i++)
                    {
                        _di_IXMLNode NdRow (RowDataExistFileNDL->Get(i));
                        existMessages[AnsiString(NdRow->GetAttribute("id")).ToInt()] = NdRow;
                    }

                    for (int i=0; i < RowDataExistFileNDL->Count; i++)
                    {
                        _di_IXMLNode NdRow (RowDataExistFileNDL->Get(i));

                        int id = AnsiString(NdRow->GetAttribute("id")).ToInt();
                        if (recieveMessages.find(id) != recieveMessages.end())
                        {
                             RowDataExistFileNDL->ReplaceNode(existMessages[id], recieveMessages[id]);
                        }
                    }
                }
                else
                {
                    DeleteFile(fileNameErrors.c_str());
                    return false;
                }

                AnsiString updatedXML;
                dExistFileXML->SaveToXML(updatedXML);

                std::ofstream ofs;

                ofs.open(fileNameErrors.c_str());
                if (!ofs.is_open())
                    return false;

                ofs << updatedXML.c_str();
                ofs.close();
            }

            for (int i=0; i < RowDataNDL->Count; i++)
            {
                //newDataExist = true;
                xmlGuard <_di_IXMLNode> NdRow (RowDataNDL->Get(i));
                if ((NdRow->NodeName == AnsiString("row"))&&(NdRow->HasAttribute("id")))
                {
                    _error_message_info NewErrorMessageInfo;
                    int id=AnsiString(NdRow->GetAttribute("id")).ToInt();
                    if (NdRow->HasAttribute("lang"))
                        NewErrorMessageInfo.Language = AnsiString(NdRow->GetAttribute("lang")).c_str();
                    if (NdRow->HasAttribute("system_message"))
                    {
                        wstrtmp=(WideString)NdRow->GetAttribute("system_message");
                        strtmp=Utf8ToAnsi(wstrtmp);
                        NewErrorMessageInfo.SystemMessage = strtmp.c_str();
                    }
                    if (NdRow->HasAttribute("user_message"))
                    {
                        wstrtmp=(WideString)NdRow->GetAttribute("user_message");
                        strtmp=Utf8ToAnsi(wstrtmp);
                        NewErrorMessageInfo.UserMessage = strtmp.c_str();
                    }
                    ErrorMessages[id] = NewErrorMessageInfo;
                }
            }
            //if(newDataExist)
            //    retRes = false;//Надо удалить предыдущий файл
        }
    }

    return retRes;
}

void TMoneyTransferPayment::GetMTLoginContent(xmlGuard <_di_IXMLNode> NdContent)
{
    xmlGuard <_di_IXMLNodeList> ContentNDL (NdContent->GetChildNodes());
    if (!ContentNDL.Assigned())
        throw Exception((AnsiString)"Bad answer format: error getting content child nodes!");

    xmlGuard <_di_IXMLNode> Sender (ContentNDL->FindNode("sender"));
    xmlGuard <_di_IXMLNodeList> SenderNdl (Sender->GetChildNodes());
    if (!Sender.Assigned())
        throw Exception((AnsiString)"Bad answer format: no <sender> node found!");

    AnsiString strtmp;

    GetNodeText(Sender,"id",strtmp);
    SenderInfo.id=strtmp.c_str();

    GetNodeText(Sender,"last_name",strtmp);
    SenderInfo.last_name=strtmp.c_str();
    GetNodeText(Sender,"first_name",strtmp);
    SenderInfo.first_name=strtmp.c_str();
    GetNodeText(Sender,"middle_name",strtmp);
    SenderInfo.middle_name=strtmp.c_str();
    GetNodeText(Sender,"birth_date",strtmp);
    SenderInfo.birth_date=strtmp.c_str();
    GetNodeText(Sender,"birth_place",strtmp);
    SenderInfo.birth_place=strtmp.c_str();
    GetNodeText(Sender,"country",strtmp);
    SenderInfo.country=strtmp.c_str();
    GetNodeText(Sender,"phone",strtmp);
    SenderInfo.phone=strtmp.c_str();
    GetNodeText(Sender,"email",strtmp);
    SenderInfo.email=strtmp.c_str();
    GetNodeText(Sender,"address",strtmp);
    SenderInfo.address = strtmp.c_str();
    GetNodeText(Sender,"mobile",strtmp);
    SenderInfo.mobile=strtmp.c_str();
    GetNodeText(Sender,"resident",strtmp);
    SenderInfo.resident=strtmp.c_str();

    xmlGuard <_di_IXMLNode> SenderId (SenderNdl->FindNode("document_id"));
    if (!SenderId.Assigned())
      throw Exception((AnsiString)"Bad answer format: no <document_id> node found!");

    GetNodeText(SenderId,"type",strtmp);
    SenderIdInfo.type=strtmp.c_str();
    GetNodeText(SenderId,"serial",strtmp);
    SenderIdInfo.serial=strtmp.c_str();

    GetNodeText(SenderId, "number", strtmp);
    /*
    int keyNum = Cfg->GetKeysNum(Cfg->Operator(Recepient).KeysId);
    std::string decryptedNumber = crypt::decrypt(keyNum, strtmp.c_str()).c_str();
    if ("" == decryptedNumber)
        throw std::runtime_error("Can not decrypt number");
    SenderIdInfo.number=decryptedNumber.c_str();
    */
    SenderIdInfo.number=strtmp.c_str();

    GetNodeText(SenderId,"issue_date",strtmp);
    SenderIdInfo.issue_date=strtmp.c_str();
    GetNodeText(SenderId,"country",strtmp);
    SenderIdInfo.country=strtmp.c_str();
    GetNodeText(SenderId,"authority",strtmp);
    SenderIdInfo.authority=strtmp.c_str();
    GetNodeText(SenderId,"authority_code",strtmp);
    SenderIdInfo.authority_code=strtmp.c_str();

    xmlGuard <_di_IXMLNode> Destionation (SenderNdl->FindNode("destination_list"));
    xmlGuard <_di_IXMLNodeList> DestionationNdl (Destionation->GetChildNodes());

    LastRecipients.clear();

    if (Destionation.Assigned() && DestionationNdl.Assigned())
    {
        for (int i=0;i<DestionationNdl->GetCount();i++)
        {
            xmlGuard <_di_IXMLNodeList> LastDestinationNdl (DestionationNdl->Get(i)->GetChildNodes());
            xmlGuard <_di_IXMLNode> LastDestination (DestionationNdl->Get(i));
            if (!LastDestination.Assigned() || !LastDestinationNdl.Assigned())
                throw Exception((AnsiString)"Bad answer format: error getting destionation child nodes!");

            _recipient_info ri;

            GetNodeText(LastDestination,"mtsystem",strtmp);
            ri.mt_system=strtmp.c_str();
            GetNodeText(LastDestination,"mtsystem_name",strtmp);
            ri.mt_system_name=strtmp.c_str();
            GetNodeText(LastDestination,"recipient_bank_address",strtmp);
            ri.bank_address=strtmp.c_str();
            GetNodeText(LastDestination,"currency",strtmp);
            ri.currencyId=boost::lexical_cast<int>(strtmp.c_str());
            GetNodeText(LastDestination,"recipient_bank_id",strtmp);
            ri.recipient_bank_id=strtmp.c_str();


            xmlGuard <_di_IXMLNode> Recipient(LastDestinationNdl->FindNode("recipient"));
            GetNodeText(Recipient,"id",strtmp);
            ri.id=strtmp.c_str();
            GetNodeText(Recipient,"last_name",strtmp);
            ri.last_name=strtmp.c_str();
            GetNodeText(Recipient,"first_name",strtmp);
            ri.first_name=strtmp.c_str();
            GetNodeText(Recipient,"middle_name",strtmp);
            ri.middle_name=strtmp.c_str();
            GetNodeText(Recipient,"email",strtmp);
            ri.email=strtmp.c_str();
            GetNodeText(Recipient,"mobile",strtmp);
            ri.mobile=strtmp.c_str();
            GetNodeText(Recipient,"resident",strtmp);
            ri.resident=strtmp.c_str();
            GetNodeText(Recipient,"country",strtmp);
            ri.country=strtmp.c_str();

            LastRecipients.push_back(ri);
        }
    }
}

void TMoneyTransferPayment::GetMTCalculateTransferSumsInfo(xmlGuard <_di_IXMLNode> NdContent,bool test)
{
    xmlGuard <_di_IXMLNodeList> ContentNDL (NdContent->GetChildNodes());
    if (!ContentNDL.Assigned())
        throw Exception((AnsiString)"Bad answer format: error getting content child nodes!");

    xmlGuard <_di_IXMLNode> Transfer (ContentNDL->FindNode("transfer"));
    xmlGuard <_di_IXMLNodeList> TransferNdl (Transfer->GetChildNodes());
    if (!Transfer.Assigned())
        throw Exception((AnsiString)"Bad answer format: no <transfer> node found!");

    xmlGuard <_di_IXMLNode> inputAmounts (  TransferNdl->FindNode("input_amounts"));
    AnsiString Temp;
    GetNodeText(inputAmounts,"amount",Temp);
    TransferInfo.amount = GetDouble(Temp);
    m_amount = Temp.c_str();
    GetNodeText(inputAmounts,"fee",Temp);
    TransferInfo.comiss = GetDouble(Temp);
    m_system_commission = Temp.c_str();
    GetNodeText(inputAmounts,"amount_total",Temp);
    TransferInfo.amount_total = GetDouble(Temp);
    m_amount_all=Temp.c_str();
    GetNodeText(inputAmounts,"change",Temp);
    m_rent_commission = Temp.c_str();

    xmlGuard <_di_IXMLNode> sendAmounts (  TransferNdl->FindNode("send_amounts"));
    GetNodeText(sendAmounts,"amount",Temp);
    m_rec_amount = Temp.c_str();
    GetNodeText(sendAmounts,"fee",Temp);
    m_rec_system_commission = Temp.c_str();
    GetNodeText(sendAmounts,"amount_total",Temp);
    m_rec_amount_all=Temp.c_str();

    GetNodeText(Transfer,"exchange_rate",Temp);
    XMLP->AddParam("ri_exchange_rate",Temp.c_str());
    if(!test)
    {
        XMLP->AddParam("comiss",TransferInfo.comiss);
    }
}

void TMoneyTransferPayment::GetMTCheckPaymentInfo(xmlGuard <_di_IXMLNode> NdContent)
{
    xmlGuard <_di_IXMLNodeList> ContentNDL (NdContent->GetChildNodes());
    if (!ContentNDL.Assigned())
        throw Exception((AnsiString)"Bad answer format: error getting content child nodes!");

    xmlGuard <_di_IXMLNode> Transfer (ContentNDL->FindNode("transfer"));
    if (!Transfer.Assigned())
        throw Exception((AnsiString)"Bad answer format: no <transfer> node found!");

    AnsiString strtmp;
    GetNodeText(Transfer,"transfer_code",strtmp);
    TransferInfo.trans_id=strtmp.c_str();
    XMLP->AddParam("trans_id",strtmp.c_str());
    XMLP->TransId=strtmp;

    GetNodeText(Transfer,"transfer_code",strtmp);
    TransferInfo.transfer_code=strtmp.c_str();
    XMLP->AddParam("transfer_code",strtmp.c_str());
}

void TMoneyTransferPayment::GetMTProcessPaymentInfo(xmlGuard <_di_IXMLNode> NdContent)
{
    xmlGuard <_di_IXMLNodeList> ContentNDL (NdContent->GetChildNodes());
    if (!ContentNDL.Assigned())
        throw Exception((AnsiString)"Bad answer format: error getting content child nodes!");

    xmlGuard <_di_IXMLNode> Transfer (ContentNDL->FindNode("transfer"));
    if (!Transfer.Assigned())
        throw Exception((AnsiString)"Bad answer format: no <transfer> node found!");

    AnsiString strtmp;
    GetNodeText(Transfer,"trans_id",strtmp);
    TransferInfo.trans_id=strtmp.c_str();
    XMLP->AddParam("trans_id",strtmp.c_str());
    XMLP->TransId=strtmp;

    GetNodeText(Transfer,"transfer_code",strtmp);
    TransferInfo.transfer_code=strtmp.c_str();
    XMLP->AddParam("transfer_code",strtmp.c_str());

    GetNodeText(Transfer,"direction",strtmp);
    XMLP->AddParam("direction",strtmp.c_str());

    GetNodeText(Transfer,"status",strtmp);
    XMLP->AddParam("status",strtmp.c_str());

    GetNodeText(Transfer,"date",strtmp);
    XMLP->AddParam("date",strtmp.c_str());
}

void TMoneyTransferPayment::MakePostGetErrorsListJSFile()
{
    try
    {
        std::auto_ptr<JSONDocument> JSONDoc(new JSONDocument(Log));

        JSONDoc->RootItem.ChildItems.push_back(JSONItem("Money transfers errors list","",true));
        JSONItem jErrorList;
        jErrorList.Name = "$ga_mt_errors_list";

        std::map<int,_error_message_info>::const_iterator it;
        for (it = ErrorMessages.begin(); it != ErrorMessages.end(); ++it)
        {
            JSONItem jErrorInfo;

            jErrorInfo.Name = (boost::format("error%1%") % it->first).str().c_str();
            jErrorInfo.ChildItems.push_back(JSONItem("id", (boost::lexical_cast<std::string>(it->first)).c_str()));
            //jErrorInfo.ChildItems.push_back(JSONItem("lang", it->second.Language.c_str()));
            jErrorInfo.ChildItems.push_back(JSONItem("system_message", it->second.SystemMessage.c_str()));
            jErrorInfo.ChildItems.push_back(JSONItem("user_message", it->second.UserMessage.c_str()));
            jErrorList.ChildItems.push_back(jErrorInfo);
        }

        JSONDoc->RootItem.ChildItems.push_back(jErrorList);
        JSONDoc->RootItem.ChildItems.push_back(JSONItem(" - - - - - всегда в true, для динамического include файла","",true));
        JSONDoc->RootItem.ChildItems.push_back(JSONItem("$ga_mt_errors_list_js","true"));

        if (!StoreStringToFile(m_errorsListFileName.c_str(),JSONDoc->GetJSONString(), Log))
            Log->Write("File not saved!");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TMoneyTransferPayment::MakePostLoginJSFile()
{
    try
    {
        std::auto_ptr <JSONDocument> JSONDoc(new JSONDocument(Log));

        JSONDoc->RootItem.ChildItems.push_back(JSONItem("Client login answer","",true));

        JSONItem jAnswer, jError, jLastRecipients, jLogin;

        jAnswer.Name = "$ga_mt";
        jError.Name = "error";
        jLastRecipients.Name = "last_recipients";
        jLogin.Name = "login_info";

        jError.ChildItems.push_back(JSONItem("code", LastErrorCode));
        jLogin.ChildItems.push_back(JSONItem("first_name", replaceLF(SenderInfo.first_name).c_str()));
        jLogin.ChildItems.push_back(JSONItem("middle_name", replaceLF(SenderInfo.middle_name).c_str()));
        jLogin.ChildItems.push_back(JSONItem("last_name", replaceLF(SenderInfo.last_name).c_str()));
        jAnswer.ChildItems.push_back(jError);

        for (std::size_t i=0;i<LastRecipients.size();i++)
        {
            JSONItem jRecipient;

            jRecipient.Name = (boost::format("recipient%1%") % i).str().c_str();
            jRecipient.ChildItems.push_back(JSONItem("last_name", replaceLF(LastRecipients[i].last_name).c_str()));
            jRecipient.ChildItems.push_back(JSONItem("first_name", replaceLF(LastRecipients[i].first_name).c_str()));
            jRecipient.ChildItems.push_back(JSONItem("middle_name", replaceLF(LastRecipients[i].middle_name).c_str()));
            jRecipient.ChildItems.push_back(JSONItem("email", replaceLF(LastRecipients[i].email).c_str()));
            jRecipient.ChildItems.push_back(JSONItem("mobile", replaceLF(LastRecipients[i].mobile).c_str()));
            jRecipient.ChildItems.push_back(JSONItem("country", replaceLF(LastRecipients[i].country).c_str()));
            jRecipient.ChildItems.push_back(JSONItem("resident", replaceLF(LastRecipients[i].resident).c_str()));
            jRecipient.ChildItems.push_back(JSONItem("recipient_bank_id", replaceLF(LastRecipients[i].recipient_bank_id).c_str()));
            jRecipient.ChildItems.push_back(JSONItem("mt_system", replaceLF(LastRecipients[i].mt_system_name).c_str()));
            jRecipient.ChildItems.push_back(JSONItem("bank_address", replaceLF(LastRecipients[i].bank_address).c_str()));
            jLastRecipients.ChildItems.push_back(jRecipient);
        }

        jAnswer.ChildItems.push_back(jLastRecipients);
        JSONDoc->RootItem.ChildItems.push_back(jAnswer);
        JSONDoc->RootItem.ChildItems.push_back(jLogin);

        JSONDoc->RootItem.ChildItems.push_back(JSONItem(" - - - - - всегда в true, для динамического include файла","",true));

        JSONDoc->RootItem.ChildItems.push_back(JSONItem("$ga_mt_js","true"));

        if (!StoreStringToFile(ChangeChars(Cfg->Dirs.InterfaceDir.c_str(),"/","\\")+"\\mt_ga.js",JSONDoc->GetJSONString(), Log))
            Log->Write("File not saved!");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        LastErrorCode=-1;
    }
}

std::string TMoneyTransferPayment::replaceLF(const std::string& value)
{
    std::string strtmp = value;
    boost::replace_all(strtmp,"\n"," ");
    return strtmp;
}

void TMoneyTransferPayment::SetSum(double Sum)
{
    int validSum = ((Sum <= GetLimMax()) ? Sum : GetLimMax());
    XMLP->AddParam("AMOUNT_ALL", validSum);
    XMLP->AddParam("AMOUNT", validSum);
    if (!XMLP->SaveToTempFile())
        FileMap->WriteErrorFound = true;
}

std::string TMoneyTransferPayment::getCalculateSystemComission()
{
    return ("" == m_system_commission) ? std::string("-1") : (m_system_commission +" "+ currencyToStr("currency"));
}

std::string TMoneyTransferPayment::getCalculateAmount()
{
    return ("" == m_rec_amount) ? std::string("-1") : (m_rec_amount +" "+ currencyToStr("ri_currency_id"));
}

std::string TMoneyTransferPayment::getCalculateAmountAll()
{
    if("" != m_amount_all)
    {
        double amount_all = boost::lexical_cast<double> (m_amount_all) + boost::lexical_cast<double>(m_rent_commission);
        std::string strtmp = (boost::format("%.2f") % amount_all).str();
        strtmp += " " + currencyToStr("currency");
        return strtmp;
    }
    else
    {
        return "-1";
    }
}

std::string TMoneyTransferPayment::getCalculateRentComission()
{
    return ("" == m_rent_commission) ? std::string("0") : (m_rent_commission +" "+ currencyToStr("currency"));
}

std::string TMoneyTransferPayment::currencyToStr(const char* paramName)
{
    std::string curStr = "";

    try
    {
        if(m_IdName.end() != m_IdName.find(boost::lexical_cast<int>(XMLP->GetParamValue(paramName).c_str())))
            curStr = m_IdName[boost::lexical_cast<int>(XMLP->GetParamValue(paramName).c_str())];

    }
    catch(...)
    {
    }

    return curStr;
}

double TMoneyTransferPayment::GetComission()
{
    return -1;
}

bool TMoneyTransferPayment::CalculateCommission(const char* location,const double sumTransfer)
{
    double tmpSumm = sumTransfer;
    std::string curValute=Cfg->CurrencyInfo.CurrencyName;
    try
    {
        ParseLocation(location);

        AnsiString result;
        saveConnect(mtMTCalculateTransferSumsAmountAll,tmpSumm,&result);
        ParseXMLAnswer(result, mtMTCalculateTransferSumsAmountAll, false);
        /*
        MessageText = MakeXMLMessage(mtMTCalculateTransferSumsAmountAll,tmpSumm).c_str();
        Log->Write(("Payment: "+MessageText).c_str());
        AnsiString PaymentResult;
        PaymentResult = Connect(Cfg->Operator(Recepient).LoginURL.c_str(),true);
        ParseXMLAnswer(PaymentResult, mtMTCalculateTransferSumsAmountAll,false);
        */
        if(0!=LastErrorCode)
        {
            m_system_commission = "";
            m_rent_commission = "";
            m_amount = "";
            m_amount_all = "";
            return false;
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        LastErrorCode = -1;
        return false;
    }
    return (0==LastErrorCode);
}

void TMoneyTransferPayment::AddNote(int ValidatorID, double Nominal)
{
    try
    {
        Log->Write((boost::format("Note %1% added to payment.") % Nominal).str().c_str());
        if ((!XMLP)||(!PS))
            return;
        Sum=Sum+Nominal*Cfg->CurrencyInfo.ExchangeRate;
        UpdateNotes(ValidatorID, Nominal);
        InitPS->StorePaymentInitTemp(TDateTime::CurrentDateTime(), XMLP->OperatorId, XMLP->InitialSessionNum, 0, XMLP->vNotes, AFieldsForInterface);
        SetSum(Sum);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

bool TMoneyTransferPayment::RenamePacketsToCanceled()
{
    bool result=true;
    try
    {
        InitPS->StorePaymentInitComplete();
        PS->StorePaymentComplete(TDateTime::CurrentDateTime(), "", "", 0, 30, XMLP->FirstTryDT, XMLP->OperatorId, Sum, 0,"");

        std::string packetFileName=XMLP->PacketFileName.c_str();
        std::size_t pos=packetFileName.find_last_of('\\');
        std::string strtmp=Cfg->Dirs.PaymentsOutboundCanceled+packetFileName.substr(pos,packetFileName.length()-pos);
        result=XMLP->RenamePaketFileFullPath(strtmp.c_str());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        result=false;
    }
    return result;
}

void TMoneyTransferPayment::clearLoginData()
{
    try
    {
        std::auto_ptr <JSONDocument> JSONDoc(new JSONDocument(Log));

        JSONDoc->RootItem.ChildItems.push_back(JSONItem("clear data","",true));
        JSONItem jAnswer, jError;

        jAnswer.Name = "$ga_mt";
        jError.Name = "error";

        jError.ChildItems.push_back(JSONItem("code", LastErrorCode));
        jAnswer.ChildItems.push_back(jError);

        JSONDoc->RootItem.ChildItems.push_back(jAnswer);
        JSONDoc->RootItem.ChildItems.push_back(JSONItem(" - - - - - всегда в true, для динамического include файла","",true));
        JSONDoc->RootItem.ChildItems.push_back(JSONItem("$ga_mt_js","true"));

        if (!StoreStringToFile(ChangeChars(Cfg->Dirs.InterfaceDir.c_str(),"/","\\")+"\\mt_ga.js",JSONDoc->GetJSONString(), Log))
            Log->Write("File not saved!");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

