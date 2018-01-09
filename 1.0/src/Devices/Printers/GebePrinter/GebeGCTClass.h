//---------------------------------------------------------------------------

#ifndef Citizen268ClassH
#define Citizen268ClassH
//---------------------------------------------------------------------------
#endif

#include "CPrinter.h"

class GebeGCT: public CPrinter
{
private:
  void SendCommand();
  std::string getOneMessage(BYTE);
public:
  GebeGCT(int ComPort,int BaudRate = 0,TLogClass* _Log = NULL);
  virtual ~GebeGCT();

  virtual void Cut();  // полный отрез
  virtual void Feed(); // сдвиг на одну строку
  virtual void PrintLine(AnsiString text); // печать одной строки (ширина печати 28 символов)
  virtual void Init(); // инициализация
  virtual void GetState();  // получение состояний   ****************(не описано!!!)

  virtual void PrintCheck(TStringList* Text); //печать чека (формируется заранее)
  virtual void PrintCheck(AnsiString text, std::string barcode = "");   //печать чека (формируется заранее)

  virtual bool IsPrinterEnable(); //откликается ли принтер


protected:
  virtual void SendPacket(BYTE* command, int bytes_count,int datalen, BYTE* data, bool datafirst = false);

};


