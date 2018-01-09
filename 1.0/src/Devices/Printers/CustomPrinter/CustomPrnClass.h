//---------------------------------------------------------------------------

#ifndef CustomPrnClassH
#define CustomPrnClassH
//---------------------------------------------------------------------------
#endif

#include "CPrinter.h"

class CCustomPrn : public CPrinter
{
private:
  virtual std::string GetStateDescription();
  std::string GetStatusDescription(BYTE StatusCode, int ByteNo);
  void SendCommand();

  virtual void PrintString(AnsiString text);
  void PrintBigString(AnsiString text);
  virtual void PrintLine(AnsiString text);
  virtual void ShriftOptionsEx(BYTE option = 0);
  virtual void Init();
  virtual void Cut();
  virtual void Feed(int count = 1);
  virtual void SetCodePage();
  virtual void SetCharacterSet();
  virtual void ClearDispenser();
  virtual void Dispense();
  virtual void Retracting();
  virtual void SetLeftMargin();

  virtual void SetBarCodeHeight(BYTE n);
  virtual void SetBarCodeWidth(BYTE n);
  virtual void SetBarCodeFont(BYTE n);
  virtual void SetBarCodeHRIposition(BYTE n);
  virtual void PrintBarCode(std::string text = "");
protected:
  virtual void SendPacket(BYTE* command, int bytes_count,int datalen, BYTE* data, bool datafirst = false);
public:
  CCustomPrn(int ComPort,int BaudRate = 0,TLogClass* _Log = NULL);
  virtual ~CCustomPrn();

  virtual bool IsPrinterEnable();
  virtual void PrintCheck(TStringList* Text);
  virtual void PrintCheck(AnsiString text, std::string barcode = "");
  virtual void GetState();
  virtual bool IsItYou();
  int Initialize();
};
