//---------------------------------------------------------------------------


#pragma hdrstop

#include "TTaxPayment.h"
#include <boost\algorithm\string.hpp>
#include <boost\algorithm\string_regex.hpp>
#include "globals.h"
#include "boost\format.hpp"
#include "JSONDocument.h"

#pragma hdrstop

//---------------------------------------------------------------------------

#pragma package(smart_init)

const std::string varFLP = "ФИО";
const std::string varINN = "ИНН";
const std::string varAdress = "Адрес";
const std::string varRecipientBank = "Банк получателя";
const std::string varKorBill = "Кор.Счет";
const std::string varBIK = "БИК";
const std::string varRecipient = "Получатель";
const std::string varBill = "Счет";
const std::string varINN2 = "ИНН плательщика";
const std::string varKPP = "КПП";
const std::string varOKATO = "ОКАТО";
const std::string varKBK = "КБК";
const std::string varPeriod = "Период";
const std::string varPayerType = "Тип платежа";
const std::string varPayer = "Плательщик";

//---------------------------------------------------------------------------

TTaxPayment::TTaxPayment(AnsiString _fileName, TWConfig *_Cfg, TLogClass *_Log, TFileMap* _FileMap, TXMLInfo* _InfoFile)
: TPayment(_fileName, _Cfg, _Log, _FileMap, _InfoFile)
{
    variableNames[varFLP] = "name";
    variableNames[varINN] = "inn";
    variableNames[varAdress] = "adress";
    variableNames[varRecipientBank] = "recipientBank";
    variableNames[varKorBill] = "korBill";
    variableNames[varBIK] = "bik";
    variableNames[varRecipient] = "recipient";
    variableNames[varBill] = "bill";
    variableNames[varINN2] = "innfns";
    variableNames[varKPP] = "kpp";
    variableNames[varOKATO] = "okato";
    variableNames[varKBK] = "kbk";
    variableNames[varPeriod] = "period";
    variableNames[varPayerType] = "payerType";
    variableNames[varPayer] = "Payer";

    initVariableValues();
}

std::string TTaxPayment::getParameter(std::string& paramName)
{
    std::string result = "";
    try
    {
        result = variableValues[paramName];
        // т.к. шлюз непонятно как отдает ИНН плательщика
        if(paramName == "ИНН плательщика" && result == "")
            result = XMLP->GetParamValue("NUMBER").c_str();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    
    return result;
}

//---------------------------------------------------------------------------

void TTaxPayment::ParseLocation(AnsiString LocationString)
{
    std::string locString = LocationString.c_str();
    boost::algorithm::replace_all(locString, " ", "%20");

    LocationString = AnsiString(locString.c_str());
    TPayment::ParseLocation(LocationString);

    payerName = "";
    taxesPeriod = "";

    std::auto_ptr <TLocationParser> location ( new TLocationParser(LocationString.c_str()));

    bool isNumberEmpty = false;

    for (int i=0;i<XMLP->Params->Count;i++)
    {
        std::string paramName = XMLP->GetParamName(i).c_str();
        std::string paramValue = XMLP->GetParamValue(i).c_str();
        /*
        if(paramName == "ACCOUNT")
        {
            if(paramValue.length() > 64)
                paramValue = paramValue.substr(0, 64);

            // удаляем лишние пробелы
            boost::algorithm::trim(paramValue);
            boost::regex rx("  *");
            paramValue = boost::regex_replace(paramValue, rx, " ");

            payerName = paramValue;
            boost::algorithm::replace_all(paramValue, "%20", "||");

            //paramValue = "имя||фамилия||отчество";

            XMLP->SetParamValue(paramName.c_str(), paramValue.c_str());
        }
        */
        if(paramName == "NUMBER")
        {
            if(paramValue == "")
                isNumberEmpty = true;
        }
        /*
        else if(paramName == "PERIOD")
        {
            // к периоду нужно добавить некоторые поля
            std::string timePart = location->GetParameter("field108").c_str();
            std::string year = location->GetParameter("field109").c_str();

            // расширить полученный интервал времени до 2х символов (дополнить нулем)
            timePart = (boost::format("%1$02s") % timePart.c_str()).str();

            if(paramValue == "4")
            {
                paramValue = (boost::format("4||%1%") % year).str();
                taxesPeriod = year;
            }
            else
            {
                taxesPeriod = (boost::format("%1%.%2%") % timePart % year).str();
                paramValue = (boost::format("%1%||%2%") % paramValue % taxesPeriod).str();
            }

            XMLP->SetParamValue(paramName.c_str(), paramValue.c_str());
        }
        */
        else if(paramName == "ADRESS")
        {
            boost::algorithm::replace_all(paramValue, "%20", " ");
            if(paramValue.length() > 128)
                paramValue = paramValue.substr(0, 128);

            //paramValue = "красная площадь, д.1";
            XMLP->SetParamValue(paramName.c_str(), paramValue.c_str());
        }
    }

    if(isNumberEmpty)
        XMLP->DeleteParam("NUMBER");

    //XMLP->SetParamValue("INNFNS", "7701107259");
    //XMLP->SetParamValue("OKATO", "45286555");
    XMLP->AddParam("REASON", "1");
}

void TTaxPayment::ParseAnswer(TStringList *slResult)
{
    TPayment::ParseAnswer(slResult);

    std::auto_ptr <JSONDocument> JSONDoc(new JSONDocument(Log));

    /*Добиваем данные из ответа*/
    std::string addInfo = URLDecode(slResult->Values["ADDINFO"]).c_str();
    initVariableValues();

    // получить из addInfo интересующие нас поля
    size_t posStart = 0;
    size_t posEnd = 0;
    while(posEnd < addInfo.length())
    {
        posEnd = addInfo.find("\n\t", posStart);
        std::string strPart = addInfo.substr(posStart, posEnd - posStart);
        posStart = posEnd + 2;

        // разбираем strPart
        size_t pos = strPart.find(":\t", 0);
        if(pos == std::string::npos)
            continue;

        std::string paramName = strPart.substr(0, pos);
        pos += 2; // skip ':\t'
        std::string paramValue = strPart.substr(pos, strPart.length() - pos);

        variableValues[paramName] = paramValue;
    }

    // Забьем данные вручну
     /**
    variableValues[varFLP] = "ФИО";
    variableValues[varINN] = "ИНН";
    variableValues[varAdress] = "adress";
    variableValues[varRecipientBank] = "recipientBank";
    variableValues[varKorBill] = "korBill";
    variableValues[varBIK] = "bik";
    variableValues[varRecipient] = "recepinet";
    variableValues[varBill] = "bill";
    variableValues[varINN2] = "inn2";
    variableValues[varKPP] = "kpp";
    variableValues[varOKATO] = "okato";
    variableValues[varKBK] = "kbk";
    variableValues[varPeriod] = "period";
    variableValues[varPayerType] = "payertype";
    variableValues[varPayer] = "payer";
      */
    JSONItem jsAnswer;
    jsAnswer.Name = "$ga_taxes";

    JSONItem jsError;
    jsError.Name = "error";
    std::string error = slResult->Values["ERROR"].c_str();
    jsError.ChildItems.push_back(JSONItem("code", "error"));
    jsAnswer.ChildItems.push_back(jsError);

    JSONItem jsTaxData;
    jsTaxData.Name = "tax_data";

    // теперь параметры ответа
    std::map<std::string, std::string>::const_iterator it;
    for(it = variableNames.begin(); it != variableNames.end(); ++it)
    {
        JSONItem var;
        var.Name = it->second.c_str();
        var.ChildItems.push_back(JSONItem("name", it->first.c_str()));
        var.ChildItems.push_back(JSONItem("val", variableValues[it->first].c_str()));

        jsTaxData.ChildItems.push_back(var);
    }

    jsAnswer.ChildItems.push_back(jsTaxData);
    JSONDoc->RootItem.ChildItems.push_back(jsAnswer);

    JSONDoc->RootItem.ChildItems.push_back(JSONItem("$ga_taxes_js","true"));

    if (!StoreStringToFile(ChangeChars(Cfg->Dirs.InterfaceDir.c_str(),"/","\\")+"\\taxes_check.js",JSONDoc->GetJSONString(), Log))
        Log->Write("File not saved!");
}

void TTaxPayment::initVariableValues()
{
    variableValues.clear();

    std::map<std::string, std::string>::const_iterator it;
    for(it = variableNames.begin(); it != variableNames.end(); ++it)
        variableValues[it->first] = "";
}
