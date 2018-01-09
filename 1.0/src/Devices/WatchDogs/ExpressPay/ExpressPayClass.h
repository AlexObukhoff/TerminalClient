//---------------------------------------------------------------------------

#ifndef ExpressPayDeviceClassH
#define ExpressPayDeviceClassH
//---------------------------------------------------------------------------
#include "CWatchDog.h"

class TExpressPayDeviceClass  : public CWatchDog
{
protected:
    virtual void Start();
public:
    TExpressPayDeviceClass(int ComPort = 1,TLogClass* _Log = NULL);
    virtual ~TExpressPayDeviceClass();
};

#endif
