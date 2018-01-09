//---------------------------------------------------------------------------

#ifndef MetroCardDeviceThreadH
#define MetroCardDeviceThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"
#include "CardReaderParams.h"

typedef enum
{
    EC_ASKOPMInit          = 1,
    EC_ASKOPMFindCard      = 2,
    EC_ASKOPMGetMenu       = 3,
    EC_ASKOPMWriteCard     = 4,
    EC_StopOperation       = 5
} TMetroCardCommand;

/*
  short int __declspec(dllexport) __cdecl ASKOPMInit(long nSystemCode, long nDeviceCode, short nCom);
  int __declspec(dllexport) __cdecl ASKOPMFindCard(char *psCardNum);
  int __declspec(dllexport) __cdecl ASKOPMGetMenu(long *pnOrderId, char *psCardNum, ASKOPM_Menu *pAMenu, short *pnItemsNum, char *psCardStatus);
  int __declspec(dllexport) __cdecl ASKOPMWriteCard(long pnOrderId, char *psCardNum, long nMenuItemId, short nAcceptedInRub, long nAuthoriseCode, char *psCardStatus);
*/

typedef short __declspec(dllimport) (__cdecl* _ASKOPMInit)(long,long,short);
typedef int __declspec(dllimport) (__cdecl* _ASKOPMFindCard)(char*);
typedef int __declspec(dllimport) (__cdecl* _ASKOPMGetMenu)(long*,char*,ASKOPM_Menu*,short*,char*);
typedef int __declspec(dllimport) (__cdecl* _ASKOPMWriteCard)(long,char*,long,short,long,char*);

/*
typedef WINAPI short (__stdcall *_ASKOPMInit)(long,long,short);
typedef WINAPI int (__stdcall *_ASKOPMFindCard)(char*);
typedef WINAPI int (__stdcall *_ASKOPMGetMenu)(long,char*,ASKOPM_Menu*,short,char*);
typedef WINAPI int (__stdcall *_ASKOPMWriteCard)(long,char*,long,short,long,char*);
*/

class TMetroCardDeviceThread : public TDeviceThread
{
protected:
    bool StopOperation;

    bool _ServerConnected;
    virtual void SetServerStatus(bool value);
    virtual bool GetServerStatus();

    virtual void PollingLoop();
    virtual void ProcessOutCommand();
    virtual void __fastcall ProcessLoopCommand();
public:
    HINSTANCE hDll;

    _ASKOPMInit ASKOPMInit;
    _ASKOPMFindCard ASKOPMFindCard;
    _ASKOPMGetMenu ASKOPMGetMenu;
    _ASKOPMWriteCard ASKOPMWriteCard;

    _init_info*       InitInfo;
    _findcard_info*   FindCardInfo;
    _getmenu_info*    GetMenuInfo;
    _writecard_info*  WriteCardInfo;

     __property bool ServerConnected = {read = GetServerStatus, write = SetServerStatus};

    bool CardFound;

    __fastcall TMetroCardDeviceThread();
    virtual __fastcall ~TMetroCardDeviceThread();
};
#endif
