//---------------------------------------------------------------------------

#ifndef CCitizenPPU700ThreadH
#define CCitizenPPU700ThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

typedef enum
{
    UNKNOWN     = 0xFF,
} CitizenPPU700State;

class CCitizenPPU700Thread : public TDeviceThread
{
protected:
public:
    __fastcall CCitizenPPU700Thread();
    __fastcall ~CCitizenPPU700Thread();
};
#endif
