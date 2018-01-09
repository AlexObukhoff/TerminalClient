//---------------------------------------------------------------------------

#ifndef CKeyboardH
#define CKeyboardH

#include "DeviceClass.h"
#include "LogClass.h"

class CKeyboard : public TDeviceClass
{
protected:
    HWND _ParentWindow;
    bool CheckFrozen();
    virtual HWND GetParentWindow();
    virtual void SetParentWindow(HWND value);
public:
    int ID;
    __property HWND ParentWindow = {read = GetParentWindow, write = SetParentWindow};

    CKeyboard(int id = 0, int ComPort = 1,TLogClass* _Log = NULL, AnsiString Prefix = "");
    virtual ~CKeyboard();

    virtual bool IsItYou();
    virtual bool IsInitialized();
    virtual void StartDevice();
    virtual bool CheckState();
};

//---------------------------------------------------------------------------
#endif

