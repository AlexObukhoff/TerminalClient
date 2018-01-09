//---------------------------------------------------------------------------


#pragma hdrstop

#include "CKeyboard.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

CKeyboard::CKeyboard(int id, int ComPort,TLogClass* _Log, AnsiString Prefix)
: TDeviceClass(ComPort,_Log, Prefix)
{
  ID = id;
  DeviceName = "Keyboard";
  ParentWindow = NULL;
}

CKeyboard::~CKeyboard()
{
  try
  {
    StopPooling();
  }
  __finally
  {
  }
}

bool CKeyboard::IsItYou()
{
    return false;
}

void CKeyboard::StartDevice()
{
  StartPooling();
}

bool CKeyboard::IsInitialized()
{
    if (DeviceThread)
        return DeviceThread->IsInitialized();
    else
        return Initialized;
}

bool CKeyboard::CheckFrozen()
{
    clock_t CurrentTime = clock();
    if ((Port != NULL)&&(CurrentTime - GetTimeStamp1()) > (FrozenTimeOut*1000))
    {//поток завис
        if(DeviceState)
        {
           DeviceState->OutStateCode = DSE_NOTMOUNT;
           ChangeDeviceState();
        }
        return false;
    }
    else
        return true;
}

bool CKeyboard::CheckState()
{
    if (DeviceThread)
    {
        if (CheckFrozen())
            return true;
        else
            return false;
    }
    else
        return false;
}

HWND CKeyboard::GetParentWindow()
{
    return _ParentWindow;
}

void CKeyboard::SetParentWindow(HWND value)
{
    _ParentWindow = value;
}

