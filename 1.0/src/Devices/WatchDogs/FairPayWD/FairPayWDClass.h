//---------------------------------------------------------------------------

#ifndef FairPayWDDeviceClassH
#define FairPayWDDeviceClassH
//---------------------------------------------------------------------------
#include "CWatchDog.h"
#include "FairPayWDThread.h"

class TFairPayWDDeviceClass  : public CWatchDog
{
protected:
    HINSTANCE hDll;

    _InitDevice       InitDevice;
    _DeInit           DeInit;
    _StartWork        StartWork;
    _SetValues        SetValues;
    _ResetModem       ResetModem;
    _WriteIdleReport  WriteIdleReport;

    virtual void Start();
public:
    TFairPayWDDeviceClass(TLogClass* _Log = NULL);
    virtual ~TFairPayWDDeviceClass();
};

#endif
