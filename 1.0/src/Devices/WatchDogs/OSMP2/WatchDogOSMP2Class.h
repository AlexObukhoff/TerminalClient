//---------------------------------------------------------------------------

#ifndef WatchDogOSMP2DeviceClassH
#define WatchDogOSMP2DeviceClassH
//---------------------------------------------------------------------------
#include "CWatchDog.h"

class TWatchDogOSMP2DeviceClass  : public CWatchDog
{
protected:
    virtual void Start();
public:
    TWatchDogOSMP2DeviceClass(int ComPort = 1,TLogClass* _Log = NULL);
    virtual ~TWatchDogOSMP2DeviceClass();
};

#endif
