//---------------------------------------------------------------------------

#ifndef TTaxPaymentH
#define TTaxPaymentH
//---------------------------------------------------------------------------
#include "TPayment.h"

class TTaxPayment : public TPayment
{
private:
    void initVariableValues();
    
    std::map<std::string, std::string> variableNames;
    std::map<std::string, std::string> variableValues;
protected:
    void ParseLocation(AnsiString);
    void ParseAnswer(TStringList*);

    std::string payerName;
    std::string taxesPeriod;
public:
    TTaxPayment(AnsiString, TWConfig*, TLogClass*, TFileMap*, TXMLInfo*);
    std::string getParameter(std::string& paramName);
};

#endif
