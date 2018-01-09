//---------------------------------------------------------------------------

#ifndef CCitizen268ThreadH
#define CCitizen268ThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

typedef enum
{
    UNKNOWN     = 0xFF,
} Citizen268State;

class CCitizen268Thread : public TDeviceThread
{
protected:
public:
    __fastcall CCitizen268Thread();
    __fastcall ~CCitizen268Thread();
};
#endif
