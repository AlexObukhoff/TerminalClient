//---------------------------------------------------------------------------

#ifndef CCitizenCPP8001ThreadH
#define CCitizenCPP8001ThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

typedef enum
{
    UNKNOWN     = 0xFF,
} CitizenCPP8001State;

class CCitizenCPP8001Thread : public TDeviceThread
{
protected:
public:
    __fastcall CCitizenCPP8001Thread();
    __fastcall ~CCitizenCPP8001Thread();
};
#endif
