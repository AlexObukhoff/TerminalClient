//---------------------------------------------------------------------------

#ifndef WatchDogDeviceClassH
#define WatchDogDeviceClassH
//---------------------------------------------------------------------------
#include "CWatchDog.h"

class TWatchDogDeviceClass  : public CWatchDog
{
protected:
    virtual void Start();
public:
    AnsiString Version;
    TWatchDogDeviceClass(int ComPort = 1,TLogClass* _Log = NULL);
    virtual ~TWatchDogDeviceClass();
};

#endif
