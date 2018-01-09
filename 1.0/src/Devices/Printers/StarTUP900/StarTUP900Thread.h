//---------------------------------------------------------------------------

#ifndef CStarTUP900ThreadH
#define CStarTUP900ThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

typedef enum
{
    UNKNOWN     = 0xFF,
} StarTUP900State;

class CStarTUP900Thread : public TDeviceThread
{
protected:
public:
    __fastcall CStarTUP900Thread();
    __fastcall ~CStarTUP900Thread();
};
#endif
