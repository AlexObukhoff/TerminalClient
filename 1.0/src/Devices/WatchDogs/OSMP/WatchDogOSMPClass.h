//---------------------------------------------------------------------------

#ifndef WatchDogOSMPDeviceClassH
#define WatchDogOSMPDeviceClassH
//---------------------------------------------------------------------------
#include "CWatchDog.h"

class TWatchDogOSMPDeviceClass  : public CWatchDog
{
protected:
    virtual void Start();
public:
    TWatchDogOSMPDeviceClass(int ComPort = 1,TLogClass* _Log = NULL);
    virtual ~TWatchDogOSMPDeviceClass();
};

#endif
