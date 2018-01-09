//---------------------------------------------------------------------------

#ifndef GptComPortClassH
#define GptComPortClassH
//---------------------------------------------------------------------------

#include <Classes.hpp>

#include "LogClass.h"
#include "ComPortParameters.h"


//#include "global.h"
#define BufferSize 500




class TGptComPortInitParameters
{
private:
public:
  AnsiString PortNumberString;
  BYTE PortNumber;

  //Критическая секция для доступа к классу полей параметров СОМ порта
//  TCriticalSection* COMParametersCriticalSection;

  void SetPortNumber(BYTE value);
  DWORD BaudRate;
  BYTE StopBits;
  BYTE Parity;
  BYTE ByteSize;
  DWORD fParity;
  DWORD DtrControl;
  DWORD timeout;
  WORD Xon;
  WORD Xoff;

  TGptComPortInitParameters();
  ~TGptComPortInitParameters();

  AnsiString GetPortNumberString();
  void SetParameters(BYTE PortNumber,DWORD BaudRate,BYTE StopBits,BYTE Parity,BYTE ByteSize,DWORD fParity,DWORD DtrControl,DWORD timeout);
  void GetParameters(BYTE& PortNumber,DWORD& BaudRate,BYTE& StopBits,BYTE& Parity,BYTE& ByteSize,DWORD& fParity,DWORD& DtrControl,DWORD& timeout);
};




class TGptComPort
{
private:
protected:

  DWORD dWritten;
  TComPortInitParameters* COMParams;

  DCB dcb;

  DWORD LastError;
public:
  TLogClass* Log;
  bool PortInit;
  HANDLE Port;
  DWORD timeout;

  TGptComPort(TComPortInitParameters* COMParameters, bool logging);
  ~TGptComPort();
  virtual bool ReadFromPort(BYTE* data,unsigned int datesize);
  virtual bool WriteToPort (BYTE* data,unsigned int datesize);
  virtual bool Release();
  virtual bool Init();
  bool ClearCOMPort();

};
#endif
