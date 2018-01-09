//---------------------------------------------------------------------------

#ifndef PRN609_012RH
#define PRN609_012RH
#include "CPrinter.h"
#include "LogClass.h"
#include "ComPortParameters.h"


class PRN609_012R : public CPrinter
{

public:
  PRN609_012R(int ComPort,int BaudRate = 0,TLogClass* _Log = NULL);
  virtual ~PRN609_012R();

  virtual void Cut();  // полный отрез
  virtual void Feed(); // сдвиг на одну строку
  virtual void PrintLine(AnsiString text); // печать одной строки (ширина печати 28 символов)
  virtual void Init(); // инициализация
  virtual void GetState();  // получение состояний   ****************(не описано!!!)

  virtual void PrintCheck(TStringList* Text); //печать чека (формируется заранее)
  virtual void PrintCheck(AnsiString text, std::string barcode = "");   //печать чека (формируется заранее)
  virtual bool IsPrinterEnable(); //откликается ли принтер
};

bool sendComandToGptDevice_GetAnswer(const BYTE* COMMAND, const int size_command, BYTE*& ANSWER , const int size_answer, const TComPortInitParameters* conf);

#endif
