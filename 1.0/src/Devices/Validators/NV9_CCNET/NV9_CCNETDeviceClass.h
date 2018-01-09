//---------------------------------------------------------------------------

#ifndef NV9_CCNETDeviceClassH
#define NV9_CCNETDeviceClassH
//---------------------------------------------------------------------------
#include "CValidator.h"

class TNV9_CCNETDeviceClass  : public CValidator
{
protected:
    virtual void Start();
public:
    TNV9_CCNETDeviceClass(int id, int ComPort,TLogClass* _Log = NULL);
    virtual ~TNV9_CCNETDeviceClass();
    virtual bool IsItYou();
};

#endif
