//---------------------------------------------------------------------------

#ifndef CEpson442ThreadH
#define CEpson442ThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

typedef enum
{
    UNKNOWN     = 0xFF,
} Epson442State;

class CEpson442Thread : public TDeviceThread
{
protected:
public:
    __fastcall CEpson442Thread();
    __fastcall ~CEpson442Thread();
};
#endif
