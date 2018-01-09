//---------------------------------------------------------------------------

#ifndef TAviaPaymentH
#define TAviaPaymentH
//---------------------------------------------------------------------------
#include <TPayment.h>
#include <string>

class TAviaPayment : public TPayment
{
private:
    std::string m_answer;
    virtual AnsiString GetMessageText(AnsiString);
    virtual AnsiString GetMessageTextForCheck(bool,const std::string& existInquiry);
    virtual bool Check(bool bFirstCheck, AnsiString AForcedOffline = "");
    bool Pay();
    void StoreDeltaMoney();
protected:
    virtual bool Payment(bool bProcessMessages, bool bSendStatusChange = true);
public:
    TAviaPayment(AnsiString, TWConfig*, TLogClass*, TFileMap*, TXMLInfo*);
    const char* GetAnswer();
    virtual double GetPaymentMinSum();
    virtual bool Process();
    virtual int GetStatus(bool StatPacketSendingAllowed, bool bTestingOnly = false);
    virtual void Update();
    std::string GetAnswerForCheque();
    void SetAnswer();
    virtual double GetComission();
};
#endif
