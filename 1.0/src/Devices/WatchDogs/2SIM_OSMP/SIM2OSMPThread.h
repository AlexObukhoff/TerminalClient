//---------------------------------------------------------------------------

#ifndef SIM2OSMPDeviceThreadH
#define SIM2OSMPDeviceThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

typedef enum
{
    UNSET           = 0,
    CLEARGSM        = 1,
    STOP            = 2,
    RESET           = 3,
    VERSION         = 4,
    SIM1            = 5,
    SIM2            = 6,
    SIMSTATUS       = 7,
    RESTART         = 8
} TSIM2OSMPCommand;

class TSIM2OSMPDeviceThread : public TDeviceThread
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
    virtual int  ChangeSIM(int SIMnumber);
    virtual int  GetState();
    void Restart();

    virtual void __fastcall ProcessLoopCommand();

    bool GetVersionExt();
public:
    AnsiString Version;
    AnsiString VersionExt;
    int SIMnumber;

    __fastcall TSIM2OSMPDeviceThread();
    virtual __fastcall ~TSIM2OSMPDeviceThread();

    virtual void SetCommand(BYTE command);
};
#endif
