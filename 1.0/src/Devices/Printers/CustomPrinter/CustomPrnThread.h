//---------------------------------------------------------------------------

#ifndef CCustomPrnThreadH
#define CCustomPrnThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

typedef enum
{
    UNKNOWN     = 0xFF,
} CustomPrnState;

class CCustomPrnThread : public TDeviceThread
{
protected:
public:
    __fastcall CCustomPrnThread();
    __fastcall ~CCustomPrnThread();
};
#endif
