//---------------------------------------------------------------------------

#ifndef StarTSP700ClassH
#define StarTSP700ClassH
//---------------------------------------------------------------------------
#endif

#include "CPrinter.h"

class CStarTSP700 : public CPrinter
{
private:
  virtual std::string GetStateDescription();
  std::string GetStatusDescription(BYTE StatusCode);
  void SendCommand();

  virtual void PrintString(AnsiString text);
  void PrintBigString(AnsiString text);
  virtual void PrintLine(AnsiString text);
  virtual void ShriftOptionsEx(BYTE option1 = 1, BYTE option2 = 1);
  virtual void Init();
  virtual void Cut();
  virtual void Feed(int count = 1);
  virtual void SetCodePage();
  virtual void PresenterAutoPush();
  void         SetLeftMargin();
  void         PresenterTimer();

protected:
  virtual void SendPacket(BYTE* command, int bytes_count,int datalen, BYTE* data, bool datafirst = false);
public:
  CStarTSP700(int ComPort,int BaudRate = 0,TLogClass* _Log = NULL);
  virtual ~CStarTSP700();

  virtual bool IsPrinterEnable();
  virtual void PrintCheck(TStringList* Text);
  virtual void PrintCheck(AnsiString text, std::string barcode = "");
  virtual void GetState();
  virtual bool IsItYou();
  virtual int Initialize();
  virtual AnsiString EncodeBackslashes(AnsiString s);
};
