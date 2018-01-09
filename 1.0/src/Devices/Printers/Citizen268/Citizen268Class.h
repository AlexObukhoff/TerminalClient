//---------------------------------------------------------------------------

#ifndef Citizen268ClassH
#define Citizen268ClassH
//---------------------------------------------------------------------------
#endif

#include "CPrinter.h"

class CCitizen268 : public CPrinter
{
private:
  virtual std::string GetStateDescription();
  std::string GetStatusDescription(BYTE StatusCode);
  void SendCommand();

  virtual void PrintString(AnsiString text);
  void PrintBigString(AnsiString text);
  virtual void PrintLine(AnsiString text);
  virtual void ShriftOptions(bool option);//true - заводской шрифт false - пользовательский
  virtual void ShriftOptionsEx(BYTE option);//true - заводской шрифт false - пользовательский
  virtual void SetUnderline(bool option);
  virtual void SetBold(bool option);
  virtual void SetDoublePrint(bool option);
  virtual void Init();
  virtual void Cut(bool option);//true - полный отрез false - неполный отрез чека
  virtual void Feed(int count=1);

protected:
  virtual void SendPacket(BYTE* command, int bytes_count,int datalen, BYTE* data, bool datafirst = false);
public:
  CCitizen268(int ComPort,int BaudRate = 0,TLogClass* _Log = NULL);
  virtual ~CCitizen268();

  virtual bool IsPrinterEnable();
  virtual void PrintCheck(TStringList* Text);
  virtual void PrintCheck(AnsiString text, std::string barcode = "");
  virtual void GetState();
  virtual bool IsItYou();
  int Initialize();
};
