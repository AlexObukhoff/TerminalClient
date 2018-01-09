//---------------------------------------------------------------------------

#ifndef BD2ClassH
#define BD2ClassH
//---------------------------------------------------------------------------
#include "CPrinter.h"
#include "DeviceThread.h"

const BYTE FONTB = 0x01;
const BYTE HIGHLIGHT = 0x08;
const BYTE DOUBLE_HEIGHT = 0x10;
const BYTE DOUBLE_WIDTH = 0x20;
const BYTE UNDERLINE = 0x80;

class bd2Class : public CPrinter
{
private:
 void sendCmd(const BYTE* buffer, unsigned int bufferSize);
 void cut();
 void setPrintMode(BYTE flags);

public:
  bd2Class(int ComPort,int BaudRate = 0,TLogClass* _Log = NULL);
  ~bd2Class();

  void PrintCheck(AnsiString text, std::string barcode = "");
  void GetState();
  int Initialize();
};

#endif
