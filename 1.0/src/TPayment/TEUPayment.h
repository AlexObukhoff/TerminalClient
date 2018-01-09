//---------------------------------------------------------------------------

#ifndef TEUPaymentH
#define TEUPaymentH

#include <TPinPayment.h>

class TEUPayment : public TPinPayment
{
  AnsiString PinValue;
  AnsiString CheckResult;
  virtual void ParseLocation(AnsiString Location);
  virtual AnsiString GetMessageText(AnsiString SessionNumber);
  virtual AnsiString GetMessageTextForCheck(bool);
  virtual AnsiString GetMessageTextForPayment();
  virtual void PostProcessFirstCheck(int,int);
  virtual AnsiString GetErrorDescr(int);
  virtual AnsiString Connect(AnsiString URL,bool bProcessMessages = false);
public:
  TEUPayment(AnsiString, TWConfig*, TLogClass*, TFileMap*, TXMLInfo*);
  };
#endif
