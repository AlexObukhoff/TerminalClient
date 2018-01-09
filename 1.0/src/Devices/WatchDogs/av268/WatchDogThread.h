//---------------------------------------------------------------------------

#ifndef WatchDogDeviceThreadH
#define WatchDogDeviceThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

typedef enum
{
    STARTGSM        = 1,
    STOPGSM         = 2,
    MONITORON       = 3,
    MONITOROFF      = 4,
    DELAY5MIN       = 5,
    VERSION         = 6,
    PAUSE         = 7,
    UNSET           = 0
} TWatchDogCommand;

class TWatchDogDeviceThread : public TDeviceThread
{
protected:
    BYTE _command;

    virtual void SendPacket(BYTE command);
    virtual void PollingLoop();
    virtual void Start();
    virtual void Life();
    virtual void Pause();
    virtual void StartGSM();
    virtual void StopGSM();
    virtual void MonitorON();
    virtual void MonitorOFF();
    virtual void GetVersion();
    virtual void Delay5min();

    void ReadAnswer();
    void ProcessOutCommand();
    virtual void __fastcall ProcessLoopCommand();
public:
    AnsiString Version;
    __fastcall TWatchDogDeviceThread();
    virtual __fastcall ~TWatchDogDeviceThread();

    virtual void SetCommand(BYTE command);
};
#endif
