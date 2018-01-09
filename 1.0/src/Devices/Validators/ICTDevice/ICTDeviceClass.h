//---------------------------------------------------------------------------

#ifndef ICTDeviceClassH
#define ICTDeviceClassH
//---------------------------------------------------------------------------
#include "CValidator.h"

class TICTDeviceClass  : public CValidator
{
protected:
    virtual void Start();
public:
    TICTDeviceClass(int id, int ComPort,TLogClass* _Log = NULL);
    virtual ~TICTDeviceClass();

    virtual bool IsItYou();
};

#endif
