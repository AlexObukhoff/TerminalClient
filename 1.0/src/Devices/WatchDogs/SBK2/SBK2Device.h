#ifndef SBK2DeviceH
#define SBK2DeviceH

#include <math.h>

#include "SBK2Class.h"

class TSBK2Device : public TSBK2DeviceClass
{
private:
    bool WaitForComplete(DWORD WaitTime = 0);
protected:
    virtual void StartDevice();
public:
    TSBK2Device(TLogClass* _Log = NULL);
    virtual ~TSBK2Device();

    bool IsInitialized();
    void ClearGSM();
    void ResetPC();
    void StartTimer();
    void StopTimer();
    bool IsItYou();
};

#endif
