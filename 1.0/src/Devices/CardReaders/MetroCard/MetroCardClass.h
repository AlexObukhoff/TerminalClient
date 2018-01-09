//---------------------------------------------------------------------------

#ifndef MetroCardDeviceClassH
#define MetroCardDeviceClassH
//---------------------------------------------------------------------------
#include "CardReader.h"
#include "MetroCardThread.h"

class TMetroCardDeviceClass  : public CCardReader
{
protected:
    HINSTANCE hDll;

    _ASKOPMInit ASKOPMInit;
    _ASKOPMFindCard ASKOPMFindCard;
    _ASKOPMGetMenu ASKOPMGetMenu;
    _ASKOPMWriteCard ASKOPMWriteCard;

    virtual void Start();
public:
    TMetroCardDeviceClass(int ComPort = 1,TLogClass* _Log = NULL);
    virtual ~TMetroCardDeviceClass();
};

#endif
