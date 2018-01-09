//---------------------------------------------------------------------------

#ifndef CCNETDeviceClassH
#define CCNETDeviceClassH
//---------------------------------------------------------------------------
#include "CValidator.h"

class TCCNETDeviceClass  : public CValidator
{
protected:
    virtual void Start();
public:
    TCCNETDeviceClass(int id, int ComPort,TLogClass* _Log = NULL);
    virtual ~TCCNETDeviceClass();
    virtual bool IsItYou();
};

#endif
