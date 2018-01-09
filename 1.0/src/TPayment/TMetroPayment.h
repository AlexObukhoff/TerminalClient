//---------------------------------------------------------------------------

#ifndef TMetroPaymentH
#define TMetroPaymentH
#include "CardReader.h"
#include "MetroCardDevice.h"

#include <TPinPayment.h>

class TMetroPayment : public TPinPayment
{

  CCardReader* CardReader;
  TFileStream* MetroMenuFile;
  AnsiString GetMenuContent();
  virtual bool Check(bool bFirstCheck, AnsiString AForcedOffline = "");
  virtual bool Payment(bool bProcessMessages, bool bSendStatusChange = true);
  virtual void ParseLocation(AnsiString);
  long MenuItem;
  AnsiString MenuItemName;
  long OrderId;
  //double DesiredAmount;
  //bool StoreFile(AnsiString FileName, AnsiString Content);
public:
  TMetroPayment(TWConfig*, TLogClass*, TFileMap*, TXMLInfo*, CCardReader*);
  virtual ~TMetroPayment();
  //virtual bool InitPayment(AnsiString Location);
  AnsiString CardNumber;
  virtual bool PreInit(int OperatorId, AnsiString Login = "", AnsiString Password = "");
  };
#endif
