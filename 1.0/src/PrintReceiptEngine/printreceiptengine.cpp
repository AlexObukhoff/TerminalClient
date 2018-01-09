#include "printreceiptengine.h"
#include <boost\format.hpp>
#include <boost\algorithm\string.hpp>
#include <boost\regex.hpp>
#include <tinyxml.h>
#include "unit1.h"
#include "localize.h"
#include "LogClass.h"


PrintReceiptTemplate::PrintReceiptTemplate()
{
    std::auto_ptr<std::string> pTempstr0(new std::string(Localization["denumbanknote"]));
    
    TPermanetReceiptParametersPair prpp[] = {
        make_pair<std::string, std::string*>("%INT_DEALER_NAME%", &Form1->Cfg->DealerInfo.DealerName),
        make_pair<std::string, std::string*>("%INT_DEALER_ADDRESS%", &Form1->Cfg->DealerInfo.DealerAddress),
        make_pair<std::string, std::string*>("%INT_BUSINESS_DEALER_ADDRESS%", &Form1->Cfg->DealerInfo.BusinessDealerAddress),
        make_pair<std::string, std::string*>("%INT_DEALER_INN%", &Form1->Cfg->DealerInfo.DealerINN),
        make_pair<std::string, std::string*>("%INT_DEALER_PHONE%", &Form1->Cfg->DealerInfo.DealerPhone),
        make_pair<std::string, std::string*>("%INT_POINT_ADDRESS%", &Form1->Cfg->DealerInfo.PointAddress),
        make_pair<std::string, std::string*>("%TERMNUMBER%", &Form1->Cfg->Terminal.NumberForCheque),
        make_pair<std::string, std::string*>("%INT_CURRENCY%", &Form1->Cfg->CurrencyInfo.CurrencyName),
        make_pair<std::string, std::string*>("%INT_CONTRACT_NUMBER%", &Form1->Cfg->DealerInfo.ContractNumber),
        make_pair<std::string, std::string*>("%INT_BANK_NAME%", &Form1->Cfg->DealerInfo.BankName),
        make_pair<std::string, std::string*>("%INT_BANK_BIK%", &Form1->Cfg->DealerInfo.BankBIK),
        make_pair<std::string, std::string*>("%INT_BANK_PHONE%", &Form1->Cfg->DealerInfo.BankPhone),
        make_pair<std::string, std::string*>("%INT_BANK_ADDRESS%", &Form1->Cfg->DealerInfo.BankAddress),
        make_pair<std::string, std::string*>("%INT_BANK_INN%", &Form1->Cfg->DealerInfo.BankINN),
        make_pair<std::string, std::string*>("%INT_DEALER_POINT_CODE%", &Form1->Cfg->Terminal.Number),
        make_pair<std::string, std::string*>("%MONEYTYPE%", pTempstr0.get())
    };

    m_PermanetReceiptParameters = TPermanetReceiptParameters(prpp,  prpp + sizeof(prpp) / sizeof(prpp[0]));

    TStandatdReceiptParametersPair srpp[] = {
        make_pair<std::string, std::string>("%BILLAMOUNT%", "     0"),
        make_pair<std::string, std::string>("%TOTALSUMM%", "     0"),
    };
    m_StandatdReceiptParameters = TStandatdReceiptParameters(srpp,  srpp + sizeof(srpp) / sizeof(srpp[0]));

    pTempstr0.release();
}

PrintReceiptTemplate::~PrintReceiptTemplate()
{
}

std::string PrintReceiptTemplate::Print(const char *FileName)
{
    std::string ReceiptString;
    TiXmlDocument ReceiptTemplate(FileName);
    bool fCoinCassette = false;

    TiXmlBase::SetCondenseWhiteSpace(false);
    if(ReceiptTemplate.LoadFile()) {
        TiXmlNode *Root = ReceiptTemplate.FirstChild("body");
        TiXmlNode *Node = Root->FirstChild("String");
        while(Node) {
            const char *chstr = Node->ToElement()->GetText();
            ReceiptString += (boost::format("%1%\r\n") % (chstr ? chstr : "")).str().c_str();
            Node = Node->NextSibling("String");
        }

        for(TStandatdReceiptParameters::iterator i = m_ConstantReceiptParameters.begin(); i != m_ConstantReceiptParameters.end(); i++) {
            boost::regex exspr(i->first);
            ReceiptString = regex_replace(ReceiptString, exspr, i->second);
        }
        
        std::auto_ptr<std::string> pTempstr1(new std::string(Localization["denumcoin"]));
        for(TStandatdReceiptParameters::iterator i = m_StandatdReceiptParameters.begin(); i != m_StandatdReceiptParameters.end(); i++) {
            if ((i->first == "%KASETTENUMBER%") && (i->second == "1"))
              m_PermanetReceiptParameters["%MONEYTYPE%"] = pTempstr1.get();
            boost::regex exspr(i->first);
            ReceiptString = regex_replace(ReceiptString, exspr, i->second);
        }

        for(TPermanetReceiptParameters::iterator i = m_PermanetReceiptParameters.begin(); i != m_PermanetReceiptParameters.end(); i++) {
            boost::regex exspr(i->first);
            ReceiptString = regex_replace(ReceiptString, exspr, i->second->c_str());
        }

        // Parce raw parameters
        static const char *delimiter = "\"";
        static const char *rawparametername = "%RAW_PARAMETER_NAME%";
        static const char *rawparameterdata = "%RAW_PARAMETER_DATA%";
        int delimiterpos = m_RawParameters.find(delimiter, 0);
        while(delimiterpos != m_RawParameters.npos)
        {
            int valuepos = m_RawParameters.find("=", delimiterpos);
            if(valuepos != m_RawParameters.npos)
            {
                int datastertpos = delimiterpos + strlen(delimiter);
                int rawparameternamepos = ReceiptString.find(rawparametername);
                if(rawparameternamepos != ReceiptString.npos)
                    ReceiptString.replace(rawparameternamepos, strlen(rawparametername), m_RawParameters.substr(datastertpos, valuepos - datastertpos));
                delimiterpos = m_RawParameters.find(delimiter, datastertpos);
                datastertpos = delimiterpos + strlen(delimiter);
                int rawparameterdatapos = ReceiptString.find(rawparameterdata);
                if(rawparameterdatapos != ReceiptString.npos)
                    ReceiptString.replace(rawparameterdatapos, strlen(rawparameterdata), m_RawParameters.substr(valuepos + 1, datastertpos - valuepos - 2));
                delimiterpos = m_RawParameters.find(delimiter, datastertpos);
            }
        }
        while(true)
        {
            int valuepos = ReceiptString.find(rawparametername);
            if(std::string::npos==valuepos)
                break;
            valuepos = ReceiptString.rfind("\n",valuepos)+1;
            ReceiptString.erase(valuepos-1,ReceiptString.find("\n",valuepos)-valuepos+1);
        }
        while(true)
        {
            int valuepos = ReceiptString.find(rawparameterdata);
            if(std::string::npos==valuepos)
                break;
            valuepos = ReceiptString.rfind("\n",valuepos)+1;
            ReceiptString.erase(valuepos-1,ReceiptString.find("\n",valuepos)-valuepos+1);
        }

        const char *moneymacros[] = {"BILLNOMINAL", "BILLAMOUNT", "BILLSUMM"};
        for(int i = 0; i < 3; i++) {
            delimiterpos = ReceiptString.find(moneymacros[i]);
            while(delimiterpos != ReceiptString.npos) {
                int beginpos = ReceiptString.rfind("\r", delimiterpos);
                int endpos = ReceiptString.find("\r", delimiterpos);
                ReceiptString.erase(beginpos, endpos - beginpos);
                delimiterpos = ReceiptString.find("BILLNOMINAL", delimiterpos);
            }
        }
    }
    else
    {
        Form1->Log->Write((boost::format("Error parsing receipt %1% line %2% column %3% \"%4%\"") % FileName % ReceiptTemplate.ErrorRow() % ReceiptTemplate.ErrorCol() % ReceiptTemplate.ErrorDesc()).str().c_str());
    }

   return ReceiptString;
}

void PrintReceiptTemplate::SetStringParameter(const std::string &StringParameters, const char *Delimiter)
{
    if(""==StringParameters)
        return;
    std::string srchstr = StringParameters + Delimiter;
    int prevdelimiterpos(0);
    int delimiterpos = srchstr.find(Delimiter, prevdelimiterpos);
    while(delimiterpos != srchstr.npos)
    {
        int valuepos = srchstr.find("=", prevdelimiterpos);
        if(valuepos != srchstr.npos)
        {
            std::string parametername = srchstr.substr(prevdelimiterpos, valuepos - prevdelimiterpos);
            std::string value = srchstr.substr(valuepos + 1, delimiterpos - valuepos - 1);
            SetParameter((boost::format("%%%1%%%") % parametername).str().c_str(), value.c_str());
            prevdelimiterpos = delimiterpos + strlen(Delimiter);
            delimiterpos = srchstr.find(Delimiter, prevdelimiterpos);
        }
    }
}

const char *PrintReceiptTemplate::GetStandardParameter(const char *ParameterName)
{
    TStandatdReceiptParameters::iterator i = m_StandatdReceiptParameters.find(ParameterName);
    return i != m_StandatdReceiptParameters.end() ? i->second.c_str() : "";
}

void PrintReceiptTemplate::SetRawParameter(const std::string &RawParameters)
{
    m_RawParameters = RawParameters;
}

void PrintReceiptTemplate::SetParameter(const char *ParameterName, const char *ParameterValue)
{
    m_StandatdReceiptParameters[ParameterName] = ParameterValue;
}

void PrintReceiptTemplate::SetParameter(const char *ParameterName, char ParameterValue)
{
    m_StandatdReceiptParameters[ParameterName] = (boost::format("%1%") % ParameterValue).str();
}

void PrintReceiptTemplate::SetParameter(const char *ParameterName, int ParameterValue)
{
    m_StandatdReceiptParameters[ParameterName] = (boost::format("%1%") % ParameterValue).str();
}

void PrintReceiptTemplate::SetParameter(const char *ParameterName, long ParameterValue)
{
    m_StandatdReceiptParameters[ParameterName] = (boost::format("%1%") % ParameterValue).str();
}

void PrintReceiptTemplate::SetParameter(const char *ParameterName, double ParameterValue)
{
    m_StandatdReceiptParameters[ParameterName] = (boost::format("%.02f") % ParameterValue).str();
}

void PrintReceiptTemplate::SetParameter(const char *ParameterName, float ParameterValue)
{
    m_StandatdReceiptParameters[ParameterName] = (boost::format("%.02f") % ParameterValue).str();
}

void PrintReceiptTemplate::SetParameter(const char *ParameterName, short ParameterValue)
{
    m_StandatdReceiptParameters[ParameterName] = (boost::format("%1%") % ParameterValue).str();
}

