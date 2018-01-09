//---------------------------------------------------------------------------

#ifndef ID003_1DeviceClassH
#define ID003_1DeviceClassH
//---------------------------------------------------------------------------
#include "CValidator.h"

class TID003_1DeviceClass  : public CValidator
{
protected:
    virtual void Start();
public:
    TID003_1DeviceClass(int id, int ComPort,TLogClass* _Log = NULL);
    virtual ~TID003_1DeviceClass();
    virtual bool IsItYou();
};

#endif
