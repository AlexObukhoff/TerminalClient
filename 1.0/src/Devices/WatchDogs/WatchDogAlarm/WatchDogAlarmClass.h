//---------------------------------------------------------------------------

#ifndef WatchDogAlarmDeviceClassH
#define WatchDogAlarmDeviceClassH
//---------------------------------------------------------------------------
#include "CWatchDog.h"

class TWatchDogAlarmDeviceClass  : public CWatchDog
{
protected:
    virtual void Start();
public:
    TWatchDogAlarmDeviceClass(int ComPort = 1,TLogClass* _Log = NULL);
    virtual ~TWatchDogAlarmDeviceClass();
};

#endif
