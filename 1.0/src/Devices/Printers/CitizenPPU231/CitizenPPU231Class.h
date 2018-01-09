//---------------------------------------------------------------------------

#ifndef CitizenPPU231ClassH
#define CitizenPPU231ClassH
//---------------------------------------------------------------------------
#endif

#include "CPrinter.h"

class CCitizenPPU231 : public CPrinter
{
private:
  virtual std::string GetStateDescription();
  std::string GetStatusDescription(BYTE StatusCode);
  void SendCommand();

  virtual void PrintString(AnsiString text);
  void PrintBigString(AnsiString text);
  virtual void PrintLine(AnsiString text);
  virtual void ShriftOptionsEx(BYTE option);
  virtual void SetUnderline(bool option);
  virtual void SetBold(bool option);
  virtual void SetDoublePrint(bool option);
  virtual void Init();
  virtual void Cut();
  virtual void Feed(int count = 1);
  virtual void SetCodeTable();

protected:
  virtual void SendPacket(BYTE* command, int bytes_count,int datalen, BYTE* data, bool datafirst = false);
public:
  CCitizenPPU231(int ComPort,int BaudRate = 0,TLogClass* _Log = NULL);
  virtual ~CCitizenPPU231();

  virtual bool IsPrinterEnable();
  virtual void PrintCheck(TStringList* Text);
  virtual void PrintCheck(AnsiString text, std::string barcode = "");
  virtual void GetState();
  virtual bool IsItYou();
  int Initialize();
};
