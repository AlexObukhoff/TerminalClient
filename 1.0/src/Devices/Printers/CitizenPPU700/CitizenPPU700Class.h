//---------------------------------------------------------------------------

#ifndef CitizenPPU700ClassH
#define CitizenPPU700ClassH
//---------------------------------------------------------------------------
#endif

#include "CPrinter.h"

class CCitizenPPU700 : public CPrinter
{
private:
  virtual std::string GetStateDescription();
  std::string GetStatusDescription(BYTE StatusCode);
  void SendCommand();

  virtual void PrintString(AnsiString text);
  void PrintBigString(AnsiString text);
  virtual void PrintLine(AnsiString text);
  virtual void ShriftOptionsEx(BYTE option);//true - заводской шрифт false - пользовательский
  virtual void SetUnderline(bool option);
  virtual void SetBold(bool option);
  virtual void SetDoublePrint(bool option);
  virtual void Init();
  virtual void Cut(bool option);//true - полный отрез false - неполный отрез чека
  virtual void Feed(int count = 1);
  virtual void SetCodeTable();
  virtual void SetCharacterSize();
  virtual void SelectFont();
  virtual void Push();
  virtual void PushEx();
  void PushEx2();
  virtual bool IsPaperInPresenter();

  virtual void SetBarCodeHeight(BYTE n);
  virtual void SetBarCodeWidth(BYTE n);
  virtual void SetBarCodeFont(BYTE n);
  virtual void SetBarCodeHRIposition(BYTE n);
  virtual void PrintBarCode(std::string text = "");
protected:
  virtual void SendPacket(BYTE* command, int bytes_count,int datalen, BYTE* data, bool datafirst = false);
public:
  CCitizenPPU700(int ComPort,int BaudRate = 0,TLogClass* _Log = NULL);
  virtual ~CCitizenPPU700();

  virtual bool IsPrinterEnable();
  virtual void PrintCheck(TStringList* Text);
  virtual void PrintCheck(AnsiString text, std::string barcode = "");
  virtual void GetState();
  virtual bool IsItYou();
  int Initialize();
  virtual void Settings();
};
