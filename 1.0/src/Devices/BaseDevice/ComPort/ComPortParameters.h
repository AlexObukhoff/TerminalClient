#ifndef ComPortParametersH
#define ComPortParametersH

#include <SyncObjs.hpp>


class TComPortInitParameters
{
private:
public:
  AnsiString PortNumberString;
  BYTE PortNumber;

  //Критическая секция для доступа к классу полей параметров СОМ порта
  TCriticalSection* COMParametersCriticalSection;

  void SetPortNumber(BYTE value);
  DWORD BaudRate;
  BYTE StopBits;
  BYTE Parity;
  BYTE ByteSize;
  DWORD fParity;
  DWORD DtrControl;
  DWORD RtsControl;
  DWORD timeout;
  WORD Xon;
  WORD Xoff;
  bool fNull;

  TComPortInitParameters(TCriticalSection* CS);
  ~TComPortInitParameters();

  AnsiString GetPortNumberString();
  void SetParameters(BYTE PortNumber,DWORD BaudRate,BYTE StopBits,BYTE Parity,BYTE ByteSize,DWORD fParity,DWORD DtrControl,DWORD timeout);
  void GetParameters(BYTE& PortNumber,DWORD& BaudRate,BYTE& StopBits,BYTE& Parity,BYTE& ByteSize,DWORD& fParity,DWORD& DtrControl,DWORD& timeout);
};

#endif
