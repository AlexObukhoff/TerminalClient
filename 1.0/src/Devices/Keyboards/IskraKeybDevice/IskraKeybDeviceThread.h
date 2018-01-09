//---------------------------------------------------------------------------

#ifndef IskraKeybDeviceThreadH
#define IskraKeybDeviceThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"
typedef enum
{
} TIskraKeybState;

class TIskraKeybDeviceThread : public TDeviceThread
{
private:
    BYTE CommandNumber;
    BYTE GetCommandNumber();
    BYTE GetMapCode(BYTE value);
protected:
    void CheckState(BYTE code);

    WORD CRC16(BYTE* data, int len_packet);
    virtual void __fastcall ProcessLoopCommand();
    virtual void SendPacket(BYTE command,int len_packet = 0, BYTE* data = NULL);
    virtual void ParseAnswer();
    virtual void GetStay();
    virtual void Reset();
    virtual void PollingLoop();
    virtual bool ChangeDeviceState(bool wait = false);
public:
    HWND ParentWindow;

    __fastcall TIskraKeybDeviceThread();
    virtual __fastcall ~TIskraKeybDeviceThread();
};
#endif
