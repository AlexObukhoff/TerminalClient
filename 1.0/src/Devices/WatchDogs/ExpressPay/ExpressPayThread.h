//---------------------------------------------------------------------------

#ifndef ExpressPayDeviceThreadH
#define ExpressPayDeviceThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

typedef enum
{
    UNSET           = 0,
    CLEARGSM        = 1,
    STOP            = 2,
    RESET           = 3,
    VERSION         = 4
} TExpressPayCommand;

class TExpressPayDeviceThread : public TDeviceThread
{
protected:
    virtual void SendPacket(BYTE command);
    virtual void PollingLoop();
    virtual void StartTimer();
    virtual void Life();
    virtual void ClearGSM();
    virtual void GetVersion();
    virtual void ProcessOutCommand();
    virtual void __fastcall ProcessLoopCommand();
public:
    AnsiString Version;

    __fastcall TExpressPayDeviceThread();
    virtual __fastcall ~TExpressPayDeviceThread();
};
#endif
