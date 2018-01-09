//---------------------------------------------------------------------------

#ifndef ID003_2DeviceClassH
#define ID003_2DeviceClassH
//---------------------------------------------------------------------------
#include "CValidator.h"

class TID003_2DeviceClass  : public CValidator
{
protected:
    virtual void Start();
public:
    TID003_2DeviceClass(int id, int ComPort,TLogClass* _Log = NULL);
    virtual ~TID003_2DeviceClass();
    virtual bool IsItYou();
};

#endif
