//---------------------------------------------------------------------------

#ifndef V2EDeviceClassH
#define V2EDeviceClassH
//---------------------------------------------------------------------------
#include "CValidator.h"

class TV2EDeviceClass  : public CValidator
{
protected:
    virtual void Start();
public:
    TV2EDeviceClass(int id, int ComPort,TLogClass* _Log = NULL);
    virtual ~TV2EDeviceClass();
};

#endif
