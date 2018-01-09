//---------------------------------------------------------------------------

#ifndef WatchDogAlarmDeviceThreadH
#define WatchDogAlarmDeviceThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

typedef enum
{
    UNSET           = 0,
    CLEARGSM        = 1,
    LOCK            = 2,
    UNLOCK          = 3,
    STATE1          = 4,
    STATE2          = 5
} TWatchDogAlarmCommand;

class TWatchDogAlarmDeviceThread : public TDeviceThread
{
protected:
    virtual void SendPacket(BYTE command);
    virtual void PollingLoop();

    virtual void StartTimer();
    virtual void Life();
    virtual void ClearGSM();
    virtual void Lock();
    virtual void UnLock();
    virtual void ReadAnswer();
    virtual void ProcessOutCommand();
    virtual void __fastcall ProcessLoopCommand();
    virtual void CheckAnswer(BYTE value);
public:
    __fastcall TWatchDogAlarmDeviceThread();
    virtual __fastcall ~TWatchDogAlarmDeviceThread();

    virtual void GetState1();
    virtual void GetState2();
};
#endif
