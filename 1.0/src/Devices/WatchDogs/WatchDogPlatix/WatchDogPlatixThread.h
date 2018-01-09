//---------------------------------------------------------------------------

#ifndef WatchDogPlatixDeviceThreadH
#define WatchDogPlatixDeviceThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

typedef enum
{
    UNSET           = 0,
    CLEARGSM        = 1,
    STOP            = 2,
    START            = 5,
    RESET           = 3,
    VERSION         = 4
} TWatchDogPlatixCommand;

class TWatchDogPlatixDeviceThread : public TDeviceThread
{
protected:
    virtual void SendPacket(BYTE command, BYTE* _data = NULL, int _datalen = 0);
    virtual void PollingLoop();
    virtual void StartTimer();
    virtual void StopTimer();
    virtual void Life();
    virtual void Reset(int number = 0);
    virtual void ClearGSM();
    virtual void GetVersion();
    virtual void ProcessOutCommand();
    virtual void __fastcall ProcessLoopCommand();
    virtual void ParseAnswer();
public:
    AnsiString Version;

    __fastcall TWatchDogPlatixDeviceThread();
    virtual __fastcall ~TWatchDogPlatixDeviceThread();
};
#endif
