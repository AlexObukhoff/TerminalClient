//---------------------------------------------------------------------------

#ifndef CitizenCPP8001ClassH
#define CitizenCPP8001ClassH
//---------------------------------------------------------------------------
#endif

#include "CPrinter.h"

class CCitizenCPP8001 : public CPrinter
{
private:
  virtual std::string GetStateDescription();
  void SendCommand();

  virtual void PrintString(AnsiString text);
  virtual void PrintLine(AnsiString text);
  virtual void SetUnderline(bool option);
  virtual void SetBold(bool option);
  virtual void SetDoublePrint(bool option);
  virtual void Init();
  virtual void Cut(bool option);//true - полный отрез false - неполный отрез чека
  virtual void Feed(int count = 1);
  virtual void SetCodeTable();
  virtual void SetCharacterSize();
  virtual void SelectFont();

protected:
  virtual void SendPacket(BYTE* command, int bytes_count,int datalen, BYTE* data, bool datafirst = false);
public:
  CCitizenCPP8001(int ComPort,int BaudRate = 0,TLogClass* _Log = NULL);
  virtual ~CCitizenCPP8001();

  virtual bool IsPrinterEnable();
  virtual void PrintCheck(TStringList* Text);
  virtual void PrintCheck(AnsiString text, std::string barcode = "");
  virtual void GetState();
  virtual bool IsItYou();
  int Initialize();
};
