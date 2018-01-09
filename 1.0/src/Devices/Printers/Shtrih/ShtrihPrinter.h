//---------------------------------------------------------------------------
#ifndef CShtrihPrinterH
#define CShtrihPrinterH
//---------------------------------------------------------------------------
#include "LogClass.h";
#include "CPrinter.h";
#include <string>;

//סגמיסעגא ט לועמהû הנאיגונא
PropertyGet error("ResultCode");
PropertyGet errorDesc("ResultCodeDescription");
PropertyGet operatornum("OperatorNumber");
PropertyGet change("Change");

PropertyGet advmode("ECRAdvancedMode");
PropertyGet advmodedescr("ECRAdvancedModeDescription");
PropertyGet mode("ECRMode");
PropertyGet modedescr("ECRModeDescription");
PropertyGet shiftover("IsFM24HoursOver");

Procedure  beep("Beep");
Procedure  sp("ShowProperties");
Procedure  _connect("Connect");
Procedure  disconnect("Disconnect");
Procedure  sale("Sale");
Procedure  returnsale("ReturnSale");
Procedure  buy("Buy");
Procedure  storno("Storno");
Procedure  returnbuy("ReturnBuy");
Procedure  discount("Discount");
Procedure  stornodiscount("StornoDiscount");
Procedure  charge("Charge");
Procedure  stornocharge("StornoCharge");
Procedure  closecheck("CloseCheck");
Procedure  reportwithcleaning("PrintReportWithCleaning");
Procedure  reportwithoutcleaning("PrintReportWithoutCleaning");
Procedure  cancelcheck("CancelCheck");
Procedure  continueprint("ContinuePrint");
Procedure  cutcheck("CutCheck");
Procedure  feeddocument("FeedDocument");
Procedure  presenterpush("PresenterPush");
Procedure  openscreen("OpenScreen");
Procedure  closescreen("CloseScreen");
Procedure  getstatus("GetECRStatus");
Procedure  getshortstatus("GetShortECRStatus");
Procedure  opensession("OpenSession");
Procedure  printstring("PrintString");
Procedure  waitforprinting("WaitForPrinting");

Procedure  printinbuffer("PrintZReportInBuffer");
Procedure  printfrombuffer("PrintZReportFromBuffer");
Procedure  cashoutcome("CashOutcome");
Procedure  getcashreg("GetCashReg");

class CShtrihPrinter : public CPrinter
{
private:
  bool DrvEnabled;
  Variant  ECR;

  int ECRAdvancedMode;
  std::string ECRAdvancedModeDescr;
  int ECRMode;
  std::string ECRModeDescr;
  bool ShiftOver;

  int TryCount;
  int ResultCode;
  bool CutterError;

  AnsiString ErrorMsg;
  AnsiString OperatorNumber;
  double Price;
  int Quantity;
  int Store;
  double Summ1;
  double Summ2;
  double Summ3;
  double Summ4;
  AnsiString Change;
  AnsiString Depart;
  int Tax1;
  int Tax2;
  int Tax3;
  int Tax4;

  enum PrnState
  {
    OK = 0,
    Printing = 4,
    NoPaperActive = 2,
    NoPaperPassive = 1,
    PaperEnable = 3,
    LongPrinting = 5,
    ReportPrinting = 11,
    EKLZReportPrinting = 12,
    NoPrinter = -12,
    Unknown = 100
  };
  enum PrnStateCommon
  {
    WorkState = 0,
    DataTransmition = 1,
    OpenedSession = 2,
    OpenedSessionCloseNeed = 3,
    ClosedSession = 4,
    OpenedDoc = 8,
    //ReportPrinting = 11,
    //EKLZReportPrinting = 12
  };

  int WaitForContinue();
  bool WaitForPrinting();
  bool Wait();
  bool Continue();

  void ShowProperties();
  void Connect();
  void Disconnect();
  void Beep();
  void Sale(AnsiString strings);
  void PrintString(AnsiString text);
  void ReturnSale();
  void Discount();
  void Charge();
  void CloseCheck();
  void CancelCheck();
  void PrintReportWithoutCleaning();
  void PrintReportWithCleaning();
  void ContinuePrint();
  virtual void CutCheck();
  virtual void FeedDocument(int count = 1);
  void PresenterPush();
  void OpenScreen();
  void CloseScreen();
  void OpenSession();
  void OpenNewSession();
  void CheckResult(int value);
  void GetPrnState(bool SendNotification = true);

  int _PrintZReportInBuffer();
  int _PrintZReportFromBuffer();

protected:
  void Feed(int count = 1);
  void FeedToMinLinesCount();

public:
  AnsiString Text;
  int Password;
//methods
public:
  CShtrihPrinter(TLogClass* _Log = NULL);
  virtual ~CShtrihPrinter();

  virtual void PrintXReport(AnsiString Text);
  virtual void PrintZReport(AnsiString Text);
  void SaleEx(double Money, AnsiString strings);
  virtual void PrintCheck(double Money, AnsiString Text);
  virtual void PrintCheck(TStringList* strings);
  virtual void PrintCheck(AnsiString text, std::string barcode = "");
  virtual void GetState();
  virtual void CashIncassation(double Money = 0);
  virtual bool IsPrinterEnable();
};

#endif
