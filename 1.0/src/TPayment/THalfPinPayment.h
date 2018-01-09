#ifndef THalfPinPaymentH
#define THalfPinPaymentH

#include <TPayment.h>
#include <TPinPayment.h>
#include <boost\format.hpp>
#include <boost\lexical_cast.hpp>

class THalfPinPayment : public TPinPayment
{
private:
    virtual AnsiString GetMessageTextForCheck(bool bFirstCheck,const std::string& existInquiry);
    virtual AnsiString GetMessageText(AnsiString SessionNumber);
protected:
    virtual void ParseLocation(AnsiString Location);
    virtual bool Check(bool bFirstCheck, AnsiString AForcedOffline = "");
    virtual AnsiString GetMessageTextForPayment(const std::string& existInquiry);
    virtual bool Payment(bool bProcessMessages, bool bSendStatusChange = true);
public:
    THalfPinPayment(AnsiString _fileName, TWConfig *_Cfg, TLogClass *_Log, TFileMap* _FileMap, TXMLInfo* _InfoFile);
    AnsiString getRentCommision();
    virtual bool Process();
    virtual ~THalfPinPayment();
};
#endif
