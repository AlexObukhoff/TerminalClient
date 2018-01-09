//---------------------------------------------------------------------------

#ifndef SBK2DeviceThreadH
#define SBK2DeviceThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

enum
{
    EC_WDTGetDoorSwitch         = 1,
    EC_WDTSetTimer              = 2,
    EC_WDTClearTimer            = 3,
    EC_WDTStopTimer             = 4,
    EC_WDTResetModem            = 5,
    EC_WDTResetComputer         = 6,
    EC_WDTStopOperation            = 7
} TEC_WD_Commands;

typedef enum
{
    WD_OK                   = 0,
    WD_ERROR                = 1
} TSBK2Command;

typedef WORD __declspec(dllimport) (__cdecl* _WDTGetDoorSwitch)(BYTE DSOut);
typedef WORD __declspec(dllimport) (__cdecl* _WDTSetTimer)(BYTE Sec);
typedef WORD __declspec(dllimport) (__cdecl* _WDTClearTimer)();
typedef WORD __declspec(dllimport) (__cdecl* _WDTStopTimer)();
typedef WORD __declspec(dllimport) (__cdecl* _WDTResetModem)();
typedef WORD __declspec(dllimport) (__cdecl* _WDTResetComputer)();

class TSBK2DeviceThread : public TDeviceThread
{
private:
protected:
    bool CardFound;
    bool StopOperation;

    int  Sensor1,
         Sensor2,
         Sensor3;
    void CheckDoorSwitch(int value);

    virtual void PollingLoop();
    virtual void ProcessOutCommand();
    virtual void __fastcall ProcessLoopCommand();
public:
    HINSTANCE hDll;

    BYTE switches;

    _info*            ProcInfo;

    _WDTGetDoorSwitch   WDTGetDoorSwitch;
    _WDTSetTimer        WDTSetTimer;
    _WDTClearTimer      WDTClearTimer;
    _WDTStopTimer       WDTStopTimer;
    _WDTResetModem      WDTResetModem;
    _WDTResetComputer   WDTResetComputer;

    __fastcall TSBK2DeviceThread();
    virtual __fastcall ~TSBK2DeviceThread();
};
#endif
