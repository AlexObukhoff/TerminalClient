//---------------------------------------------------------------------------

#ifndef CardReaderH
#define CardReaderH
//---------------------------------------------------------------------------

#include "DeviceClass.h"
#include "LogClass.h"
#include "CardReaderParams.h"

class CCardReader : public TDeviceClass
{
protected:

    bool _ServerConnected;
    virtual void SetServerStatus(bool value);
    virtual bool GetServerStatus();
public:
    CCardReader(int ComPort = 0,TLogClass* _Log = NULL, AnsiString Prefix = "");
    virtual ~CCardReader();

    _init_info*       InitInfo;
    _findcard_info*   FindCardInfo;
    _getmenu_info*    GetMenuInfo;
    _writecard_info*  WriteCardInfo;

    __property bool ServerConnected = {read = GetServerStatus, write = SetServerStatus};

    virtual int Init();
    virtual int FindCard();
    virtual int GetMenu();
    virtual int WriteCard();
    virtual void StopOperation(){};
};

#endif
