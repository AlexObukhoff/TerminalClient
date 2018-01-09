#ifndef FairPayWDDeviceH
#define FairPayWDDeviceH

#include <math.h>

#include "FairPayWDClass.h"

class TFairPayWDDevice : public TFairPayWDDeviceClass
{
private:
    bool WaitForComplete(DWORD WaitTime = 0);
protected:
    virtual void StartDevice();
public:
    TFairPayWDDevice(TLogClass* _Log = NULL);
    virtual ~TFairPayWDDevice();

    bool IsInitialized();
    void ClearGSM();
    void StartTimer();
    bool IsItYou();
};

#endif
