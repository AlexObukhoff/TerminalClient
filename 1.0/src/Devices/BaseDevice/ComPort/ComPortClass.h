#ifndef ComPortClassH
#define ComPortClassH
#include <Classes.hpp>

#include "LogClass.h"
#include "ComPortParameters.h"
#include "DeviceState.h"

namespace PortType
{
    enum Enum
    {
        com,
        lpt
    };
}

typedef enum
{
  RecieveAnswer = 0,
  NotRecieveAnswer = 1
} TSendType;

class TComPort
{
private:
protected:
  TLogClass* Log;
  //структура для инициализации СОМ порта
  DCB dcb;

  //код ошибки последней операции
  DWORD LastError;
  virtual bool Init();
public:
  COMMTIMEOUTS to;
  bool PortInit;
  HANDLE Port;
  DWORD timeout;
  DWORD ModemStat;
  PortType::Enum m_portType;
  TComPortInitParameters* COMParameters;

  TComPort(TComPortInitParameters* _COMParameters,TLogClass* Log, bool logging, PortType::Enum portType = PortType::com);
  ~TComPort();
  bool Release();
  bool ClearCOMPort();
  int DSR_CTS();
  void ReopenPort();
};

#endif
