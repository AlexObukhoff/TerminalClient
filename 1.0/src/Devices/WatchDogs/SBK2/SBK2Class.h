//---------------------------------------------------------------------------

#ifndef SBK2DeviceClassH
#define SBK2DeviceClassH
//---------------------------------------------------------------------------
#include "CWatchDog.h"
#include "SBK2Thread.h"

class TSBK2DeviceClass  : public CWatchDog
{
protected:
    HINSTANCE hDll;

    _WDTGetDoorSwitch   WDTGetDoorSwitch;
    _WDTSetTimer        WDTSetTimer;
    _WDTClearTimer      WDTClearTimer;
    _WDTStopTimer       WDTStopTimer;
    _WDTResetModem      WDTResetModem;
    _WDTResetComputer   WDTResetComputer;

    virtual void Start();
public:
    TSBK2DeviceClass(TLogClass* _Log = NULL);
    virtual ~TSBK2DeviceClass();
};

#endif
