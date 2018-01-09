//---------------------------------------------------------------------------

#ifndef CPrinterH
#define CPrinterH

#include "DeviceClass.h"
#include "LogClass.h"

typedef enum
{
  Nothing = 0,
  Pull = 1,
  PullAndPush = 2
} TPushActionType;

class CPrinter : public TDeviceClass
{
private:
protected:
  bool PrinterEnable;
  int _MinLinesCount;
  int LinesCount;

  virtual void FeedToMinLinesCount();
  virtual void SendCommand();
public:
  int OldState;
  int OldSubState;
  BYTE Error;
  int State;
  int SubState;
  std::string StateDescr;
  std::string SubStateDescr;

  bool Fiscal;
  bool SessionOpened;
  bool ZReportInBuffer;
  bool AutoOpenShift;
  int MinLinesCount;
  std::string Font;
  TPushActionType PushActionType;

  CPrinter(int ComPort,TLogClass* _Log = NULL, AnsiString Prefix = "", PortType::Enum portType = PortType::com);
  virtual ~CPrinter(){}

  virtual void Start();
  virtual void Cut(){}
  virtual void Feed(int count = 1){}
  virtual void PrintLine(AnsiString text){}
  virtual void GetState() = 0;
  virtual void TestPrint(){}
  virtual void Settings(){}
  virtual AnsiString GetID(){return "";}
  virtual std::string GetStatusDescription(BYTE StatusCode){return "";}
  virtual std::string GetStatusDescription(BYTE StatusCode, BYTE ByteNo){return "";}

  virtual void PrintCheck(TStringList* Text){}
  virtual void PrintCheck(AnsiString Text, std::string barcode = ""){}
  virtual void PrintNotFiscalCheck(AnsiString Text);
  virtual void PrintCheck(double Money, AnsiString Text){}
  virtual void PrintXReport(AnsiString Text = "");
  virtual void PrintZReport(AnsiString Text = "");
  virtual bool PrintZReportsFromBuffer(int BeginSessionNumber = 0, int EndSessionNumber = 0);
  virtual void CashIncassation(double Money = 0){};
  virtual bool CommandCancelCheque(){return true;};
  virtual bool IsPrinterEnable();
  virtual bool IsFiscal(){return Fiscal;}
  virtual void SetBarCodeHeight(BYTE n) {}
  virtual void SetBarCodeWidth(BYTE n) {}
  virtual void SetBarCodeFont(BYTE n) {}
  virtual void SetBarCodeHRIposition(BYTE n) {}
  virtual void PrintBarCode(std::string text = "") {}
};

//---------------------------------------------------------------------------
#endif

