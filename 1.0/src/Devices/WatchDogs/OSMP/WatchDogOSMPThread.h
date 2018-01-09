//---------------------------------------------------------------------------

#ifndef WatchDogOSMPDeviceThreadH
#define WatchDogOSMPDeviceThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

typedef enum
{
    UNSET           = 0,
    CLEARGSM        = 1,
    STOP            = 2,
    RESET           = 3,
    VERSION         = 4
} TWatchDogOSMPCommand;

class TWatchDogOSMPDeviceThread : public TDeviceThread
{
protected:
    //BYTE _command;

    virtual void SendPacket(BYTE command);
    virtual void PollingLoop();
    virtual void StartTimer();
    virtual void StopTimer();
    virtual void Life();
    virtual void ClearGSM();
    virtual void ResetPC();
    virtual void GetVersion();
    virtual void ReadAnswer();
    virtual void ProcessOutCommand();
    virtual void __fastcall ProcessLoopCommand();
    //virtual void ProcessLoopCommand();
public:
    AnsiString Version;

    __fastcall TWatchDogOSMPDeviceThread();
    virtual __fastcall ~TWatchDogOSMPDeviceThread();

    virtual void SetCommand(BYTE command);
};
#endif
