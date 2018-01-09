//---------------------------------------------------------------------------

#ifndef MEIDeviceClassH
#define MEIDeviceClassH
//---------------------------------------------------------------------------
#include "CValidator.h"

class TMEIDeviceClass  : public CValidator
{
protected:
    virtual void Start();
public:
    TMEIDeviceClass(int id, int ComPort, TLogClass* _Log = NULL, int _mode = 0);
    virtual ~TMEIDeviceClass();
    virtual bool IsItYou();
};

//=============================================
const modeCount = 3;        //количество вариантов запрос/ответ      (обычный режим, расширенный режим, запрос версии прошивки)
const BYTE sendDataCount[modeCount] = {4, 6, 4};
const BYTE receiveDataCount[modeCount] = {7, 25, 10};
const double ExtendedModeFromFirmware = 2.80;
const double ExtendedModeFromProject = 81807;

namespace MEI_MODE
{
    enum MEI_MODE
    {
        Simple = 0,
        Extended = 1,
        Version = 2
    };
    const BYTE ACK_MASK = 0x01;
    const BYTE MSGTYPE_MASK = 0xF0;
    const Extended_mode_packetType = 0x70;
}

#endif
