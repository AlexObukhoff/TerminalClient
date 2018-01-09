//---------------------------------------------------------------------------

#ifndef Epson442ClassH
#define Epson442ClassH
//---------------------------------------------------------------------------
#endif

#include "CPrinter.h"

class CEpson442 : public CPrinter
{
private:
  virtual std::string GetStateDescription();
  virtual std::string GetStatusDescription(BYTE StatusCode);
  virtual std::string GetStatusDescription(BYTE StatusCode, BYTE ByteNo);
  void SendCommand();

  virtual void PrintString(AnsiString text);
  void PrintBigString(AnsiString text);
  virtual void PrintLine(AnsiString text);
  virtual void Init();
  virtual void Cut(bool option);//true - полный отрез false - неполный отрез чека
  virtual void Feed(int count = 1);
  virtual void SetCodeTable();
  virtual void SetLeftMargin();
  virtual void SetCharacterSet();
  virtual void SetCharacterSize();
  virtual void SelectFont();

protected:
  virtual void SendPacket(BYTE* command, int bytes_count,int datalen, BYTE* data, bool datafirst = false);
public:
  CEpson442(int ComPort,int BaudRate = 0,TLogClass* _Log = NULL);
  virtual ~CEpson442();

  virtual void PrintCheck(TStringList* Text);
  virtual void PrintCheck(AnsiString text, std::string barcode = "");
  virtual void GetState();
  virtual bool IsItYou();
  int Initialize();
};
