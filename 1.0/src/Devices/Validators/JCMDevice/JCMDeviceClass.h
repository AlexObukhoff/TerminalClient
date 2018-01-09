//---------------------------------------------------------------------------

#ifndef JCMDeviceClassH
#define JCMDeviceClassH
//---------------------------------------------------------------------------
#include "CValidator.h"

class TJCMDeviceClass  : public CValidator
{
protected:
    virtual void Start();
public:
    TJCMDeviceClass(int id, int ComPort,TLogClass* _Log = NULL);
    virtual ~TJCMDeviceClass();
    virtual bool IsItYou();
};

#endif
