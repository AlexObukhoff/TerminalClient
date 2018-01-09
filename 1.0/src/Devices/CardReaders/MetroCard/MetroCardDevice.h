#ifndef MetroCardDeviceH
#define MetroCardDeviceH

#include <math.h>

#include "MetroCardClass.h"

typedef enum
{
  AI_Init_Success      = 0,
  AI_No_BSK_Connect    = 1,
  AI_No_Server_Connect = 2
} AI_T_ASKOPMInit;

typedef enum
{
  AFC_Card_Activated    = 0,
  AFC_No_BSK_Connect    = 1,
  AFC_Card_DontRead     = 2
} T_ASKOPMFindCard;

typedef enum
{
  AGM_Menu_Completed    = 0,
  AGM_No_BSK_Connect    = 1,
  AGM_No_Server_Connect = 2,
  AGM_Card_DontRead     = 3
} T_ASKOPMGetMenu;

typedef enum
{
  AWC_Card_Writed       = 0,
  AWC_No_BSK_Connect    = 1,
  AWC_No_Server_Connect = 2,
  AWC_Card_DontRead     = 3,
  AWC_Invalid_Summ      = 4,
  AWC_Invalid_Card      = 5,
  AWC_Invalid_Menu      = 6
} T_ASKOPMWriteCard;

class TMetroCardDevice : public TMetroCardDeviceClass
{
private:
    bool WaitForComplete(_info* Info, DWORD WaitTime = 0);
protected:
    virtual void StartDevice();
    virtual void GetServerStatus(bool value);
    virtual bool SetServerStatus();
public:
    TMetroCardDevice(int ComPort = 0,TLogClass* _Log = NULL);
    virtual ~TMetroCardDevice();

    int Init();
    int FindCard();
    int GetMenu();
    int WriteCard();
    void StopOperation();

    bool IsItYou();
};

#endif
