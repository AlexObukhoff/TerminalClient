//---------------------------------------------------------------------------

#ifndef DeviceCheckerH
#define DeviceCheckerH
//---------------------------------------------------------------------------

#include "LogClass.h"
#include "DeviceDescriptor.h"
#include "CValidator.h"
#include "CPrinter.h"
#include "CWatchDog.h"
#include "Modem.h"
#include "CKeyboard.h"

#include "ICTDevice.h"
#include "ID003_2Device.h"
#include "ID003_1Device.h"
#include "CCNETdevice.h"
#include "JCMdevice.h"

#include "Citizen268Class.h"
#include "ShtrihPrinter.h"
#include "cbm1000t2.h"
#include "CitizenPPU700Class.h"
#include "CitizenPPU231Class.h"
#include "CitizenPPU232Class.h"
#include "CustomPrnClass.h"
#include "Epson442Class.h"
#include "SwecoinTTP2010Class.h"
#include "StarTUP900Class.h"
#include "StarTSP700Class.h"
#include "ShtrihPrinter.h"
#include "WinPrnClass.h"
#include "wp_t833.h"
#include "Prim21KClass.h"
#include "Prim08TKClass.h"

#include "WatchDogdevice.h"
#include "WatchDogOSMPdevice.h"
#include "WatchDogAlarmDevice.h"
#include "WatchDogOSMP2device.h"
#include "WatchDogPlatixDevice.h"

#include "ModemSiemensMC35i.h"

#include "IskraKeybDevice.h"

class CDeviceChecker
{
private:
    BYTE MinPortNumber;
    BYTE MaxPortNumber;
    bool NewLog;
protected:
    TLogClass* Log;
    void CheckingLoop();
    TList* Ports;
public:
    CDeviceDescriptor* Descriptor;
    TStrings* PortsList;

    CDeviceChecker(CDeviceDescriptor* descriptor, TLogClass* log = NULL);
    virtual ~CDeviceChecker();

    void GetPortsList();
};

#endif
