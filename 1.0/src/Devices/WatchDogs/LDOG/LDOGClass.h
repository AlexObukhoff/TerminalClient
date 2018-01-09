//---------------------------------------------------------------------------

#ifndef LDOGDeviceClassH
#define LDOGDeviceClassH
//---------------------------------------------------------------------------
#include "CWatchDog.h"

class TLDOGDeviceClass  : public CWatchDog
{
protected:
    virtual void Start();
public:
    TLDOGDeviceClass(int ComPort = 1,TLogClass* _Log = NULL);
    virtual ~TLDOGDeviceClass();
};

#endif
