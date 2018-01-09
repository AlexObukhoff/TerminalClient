//---------------------------------------------------------------------------

#ifndef IskraKeybDeviceClassH
#define IskraKeybDeviceClassH
//---------------------------------------------------------------------------
#include "CKeyboard.h"

class TIskraKeybDeviceClass  : public CKeyboard
{
protected:
    virtual void Start();
    virtual void SetParentWindow(HWND value);
public:
    TIskraKeybDeviceClass(int id, int ComPort,TLogClass* _Log = NULL);
    virtual ~TIskraKeybDeviceClass();

    virtual bool IsItYou();
};

#endif
