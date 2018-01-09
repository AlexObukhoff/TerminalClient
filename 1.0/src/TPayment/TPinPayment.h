//---------------------------------------------------------------------------

#ifndef TPinPaymentH
#define TPinPaymentH

#include <TPayment.h>

class TPinPayment : public TPayment
{
private:
    std::string GetAmountYE();
protected:
    virtual void ParseLocation(AnsiString Location);
    virtual AnsiString GetMessageTextForCheck(bool,const std::string& existInquiry);
    AnsiString CardID;
    double CardValue;
    AnsiString CardName;
    virtual void SetSum(double Sum);
    virtual void PostProcessFirstCheck(int,int);
    virtual void PostProcessPayment(int,int) {};
    virtual AnsiString GetErrorDescr(int);
    int GetStatusWithPIN();
    AnsiString GetOldestFileByMask(AnsiString FindDir, AnsiString FileNameMask);
    bool UseSavedFile;
    TPaymentPacket *SavedPaymentPkt;
    bool PaymentAlreadySent;
    virtual bool Payment(bool bProcessMessages, bool bSendStatusChange = true);
public:
    TPinPayment(AnsiString, TWConfig*, TLogClass*, TFileMap*, TXMLInfo*);
    virtual ~TPinPayment();
    virtual bool Process() { return true; };
    AnsiString GetCardsInfo(int Operator);
    virtual bool InitDone(PaymentDoneCmd Command = cnPDCmdStore);
    bool IsTimedOut() { return true; };
//  virtual double GetLimMax();
//  virtual double GetPaymentMinSum();
    AnsiString AutoGetCard(int OpId, AnsiString CardId, AnsiString CardName, AnsiString Sum, AnsiString CardStringToSave);
    AnsiString CompletePayment();
    virtual bool InitPayment(AnsiString Location);
    AnsiString GetPaymentData(int _OpId, AnsiString CardName);
    virtual AnsiString GetMessageTextForPayment(const std::string& existInquiry);
};
#endif
