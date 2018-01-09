//---------------------------------------------------------------------------

#ifndef WatchDogOSMP2DeviceThreadH
#define WatchDogOSMP2DeviceThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

typedef enum
{
    UNSET           = 0,
    CLEARGSM        = 1,
    STOP            = 2,
    RESET           = 3,
    VERSION         = 4
} TWatchDogOSMP2Command;

class TWatchDogOSMP2DeviceThread : public TDeviceThread
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

    __fastcall TWatchDogOSMP2DeviceThread();
    virtual __fastcall ~TWatchDogOSMP2DeviceThread();
};
#endif
