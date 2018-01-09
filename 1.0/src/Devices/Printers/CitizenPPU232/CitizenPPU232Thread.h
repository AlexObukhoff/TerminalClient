//---------------------------------------------------------------------------

#ifndef CCitizenPPU232ThreadH
#define CCitizenPPU232ThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

typedef enum
{
    UNKNOWN     = 0xFF,
} CitizenPPU232State;

class CCitizenPPU232Thread : public TDeviceThread
{
protected:
public:
    __fastcall CCitizenPPU232Thread();
    __fastcall ~CCitizenPPU232Thread();
};
#endif
