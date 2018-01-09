//---------------------------------------------------------------------------

#ifndef FairPayWDDeviceThreadH
#define FairPayWDDeviceThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

enum
{
    FP_InitDevice         = 1,
    FP_DeInit             = 2,
    FP_StartWork          = 3,
    FP_SetValues          = 4,
    FP_ResetModem         = 5,
    FP_WriteIdleReport    = 6,
} FP_WD_Commands;

//---------------------------------------------------------------------------

typedef BYTE __declspec(dllimport) (__cdecl* _InitDevice)();
typedef void __declspec(dllimport) (__cdecl* _DeInit)();
typedef BYTE __declspec(dllimport) (__cdecl* _StartWork)();
typedef BYTE __declspec(dllimport) (__cdecl* _SetValues)(DWORD PowerOffIntervalModem);
typedef BYTE __declspec(dllimport) (__cdecl* _ResetModem)();
typedef BYTE __declspec(dllimport) (__cdecl* _WriteIdleReport)();

class TFairPayWDDeviceThread : public TDeviceThread
{
protected:
    bool CardFound;
    bool StopOperation;

    virtual void PollingLoop();
    virtual void ProcessOutCommand();
    virtual void __fastcall ProcessLoopCommand();
public:
    HINSTANCE hDll;

    _info*            ProcInfo;

    _InitDevice       InitDevice;
    _DeInit           DeInit;
    _StartWork        StartWork;
    _SetValues        SetValues;
    _ResetModem       ResetModem;
    _WriteIdleReport  WriteIdleReport;

    __fastcall TFairPayWDDeviceThread();
    virtual __fastcall ~TFairPayWDDeviceThread();
};
#endif
