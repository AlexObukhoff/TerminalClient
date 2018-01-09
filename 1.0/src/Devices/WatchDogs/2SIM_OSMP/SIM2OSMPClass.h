//---------------------------------------------------------------------------

#ifndef SIM2OSMPDeviceClassH
#define SIM2OSMPDeviceClassH
//---------------------------------------------------------------------------
#include "CWatchDog.h"

class TSIM2OSMPDeviceClass  : public CWatchDog
{
protected:
    virtual void Start();
public:
    TSIM2OSMPDeviceClass(int ComPort = 1,TLogClass* _Log = NULL);
    virtual ~TSIM2OSMPDeviceClass();
};

#endif
