//---------------------------------------------------------------------------

#ifndef CStarTSP700ThreadH
#define CStarTSP700ThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

typedef enum
{
    UNKNOWN     = 0xFF,
} StarTSP700State;

class CStarTSP700Thread : public TDeviceThread
{
protected:
public:
    __fastcall CStarTSP700Thread();
    __fastcall ~CStarTSP700Thread();
};
#endif
