//---------------------------------------------------------------------------

#ifndef CCitizenPPU231ThreadH
#define CCitizenPPU231ThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

typedef enum
{
    UNKNOWN     = 0xFF,
} CitizenPPU231State;

class CCitizenPPU231Thread : public TDeviceThread
{
protected:
public:
    __fastcall CCitizenPPU231Thread();
    __fastcall ~CCitizenPPU231Thread();
};
#endif
