//---------------------------------------------------------------------------

#ifndef KtekKeybDeviceThreadH
#define KtekKeybDeviceThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"
typedef enum
{
} TKtekKeybState;

class TKtekKeybDeviceThread : public TDeviceThread
{
private:
    BYTE GetMapCode(BYTE value);
    void Poll();
protected:
    void CheckState(BYTE code);

    virtual void __fastcall ProcessLoopCommand();
    virtual bool ChangeDeviceState(bool wait = false);
public:
    HWND ParentWindow;

    __fastcall TKtekKeybDeviceThread();
    virtual __fastcall ~TKtekKeybDeviceThread();
};
#endif
