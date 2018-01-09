//---------------------------------------------------------------------------

#ifndef NRIDeviceClassH
#define NRIDeviceClassH
//---------------------------------------------------------------------------
#include "CCoinAcceptor.h"

class TNRIDeviceClass  : public CCoinAcceptor
{
protected:
    virtual void Start();
public:
    TNRIDeviceClass(int id, int ComPort,TLogClass* _Log = NULL);
    virtual ~TNRIDeviceClass();
};

#endif
