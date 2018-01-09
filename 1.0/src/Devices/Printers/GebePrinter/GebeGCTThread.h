//---------------------------------------------------------------------------

#ifndef GebeGPTThreadH
#define GebeGPTThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

typedef enum
{
    UNKNOWN     = 0xFF,
} Citizen268State;

class GebeGCTThread : public TDeviceThread
{
protected:
public:
    __fastcall GebeGCTThread();
    __fastcall ~GebeGCTThread();
};
#endif
