//---------------------------------------------------------------------------

#ifndef WatchDogPlatixDeviceClassH
#define WatchDogPlatixDeviceClassH
//---------------------------------------------------------------------------
#include "CWatchDog.h"

class TWatchDogPlatixDeviceClass  : public CWatchDog
{
protected:
    virtual void Start();
public:
    TWatchDogPlatixDeviceClass(int ComPort = 1,TLogClass* _Log = NULL);
    virtual ~TWatchDogPlatixDeviceClass();
};

#endif
