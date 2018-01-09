//---------------------------------------------------------------------------

#ifndef CWinPrinterThreadH
#define CWinPrinterThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

typedef enum
{
    UNKNOWN     = 0xFF,
} WinPrinterState;

class CWinPrinterThread : public TDeviceThread
{
protected:
public:
    __fastcall CWinPrinterThread();
    __fastcall ~CWinPrinterThread();
};
#endif
