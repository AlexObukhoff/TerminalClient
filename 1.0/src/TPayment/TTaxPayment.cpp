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

const std::string varFLP = "���";
const std::string varINN = "���";
const std::string varAdress = "�����";
const std::string varRecipientBank = "���� ����������";
const std::string varKorBill = "���.����";
const std::string varBIK = "���";
const std::string varRecipient = "����������";
const std::string varBill = "����";
const std::string varINN2 = "��� �����������";
const std::string varKPP = "���";
const std::string varOKATO = "�����";
const std::string varKBK = "���";
const std::string varPeriod = "������";
const std::string varPayerType = "��� �������";
const std::string varPayer = "����������";

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
        // �.�. ���� ��������� ��� ������ ��� �����������
        if(paramName == "��� �����������" && result == "")
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

            // ������� ������ �������
            boost::algorithm::trim(paramValue);
            boost::regex rx("  *");
            paramValue = boost::regex_replace(paramValue, rx, " ");

            payerName = paramValue;
            boost::algorithm::replace_all(paramValue, "%20", "||");

            //paramValue = "���||�������||��������";

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
            // � ������� ����� �������� ��������� ����
            std::string timePart = location->GetParameter("field108").c_str();
            std::string year = location->GetParameter("field109").c_str();

            // ��������� ���������� �������� ������� �� 2� �������� (��������� �����)
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

            //paramValue = "������� �������, �.1";
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

    /*�������� ������ �� ������*/
    std::string addInfo = URLDecode(slResult->Values["ADDINFO"]).c_str();
    initVariableValues();

    // �������� �� addInfo ������������ ��� ����
    size_t posStart = 0;
    size_t posEnd = 0;
    while(posEnd < addInfo.length())
    {
        posEnd = addInfo.find("\n\t", posStart);
        std::string strPart = addInfo.substr(posStart, posEnd - posStart);
        posStart = posEnd + 2;

        // ��������� strPart
        size_t pos = strPart.find(":\t", 0);
        if(pos == std::string::npos)
            continue;

        std::string paramName = strPart.substr(0, pos);
        pos += 2; // skip ':\t'
        std::string paramValue = strPart.substr(pos, strPart.length() - pos);

        variableValues[paramName] = paramValue;
    }

    // ������ ������ ������
     /**
    variableValues[varFLP] = "���";
    variableValues[varINN] = "���";
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

    // ������ ��������� ������
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
