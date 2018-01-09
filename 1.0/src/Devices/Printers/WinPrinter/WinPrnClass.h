//---------------------------------------------------------------------------

#ifndef WinPrinterClassH
#define WinPrinterClassH

#include <ComCtrls.hpp>
#include "CPrinter.h"

class CWinPrinter : public CPrinter
{
private:
  virtual AnsiString GetStateDescription();
  std::string GetStatusDescription(BYTE StatusCode);
  HANDLE MainWindow;
  TRichEdit* PrintForm;
  TStringList* strings;
protected:
  void Feed();
public:
  CWinPrinter(HANDLE ComPort = 0, int FontSize = 9,TLogClass* _Log = NULL);
  virtual ~CWinPrinter();

  virtual void PrintCheck(TStringList* Text);
  virtual void PrintCheck(AnsiString text, std::string barcode = "");
  virtual void GetState();
};

//---------------------------------------------------------------------------
#endif


