//---------------------------------------------------------------------------

#ifndef KtekKeybDeviceClassH
#define KtekKeybDeviceClassH
//---------------------------------------------------------------------------
#include "CKeyboard.h"

class TKtekKeybDeviceClass  : public CKeyboard
{
protected:
    virtual void Start();
    virtual void SetParentWindow(HWND value);
public:
    TKtekKeybDeviceClass(int id, int ComPort,TLogClass* _Log = NULL);
    virtual ~TKtekKeybDeviceClass();

    virtual bool IsItYou();
};

#endif
