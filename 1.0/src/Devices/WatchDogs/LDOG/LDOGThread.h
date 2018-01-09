//---------------------------------------------------------------------------

#ifndef LDOGDeviceThreadH
#define LDOGDeviceThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

typedef enum
{
    WD2_UNSET           = 0,
    WD2_CLEARGSM        = 1,
    WD2_RESET           = 3,
    WD2_VERSION         = 4,
    WD2_GETSENSORS      = 5
} TLDOGCommand;

class TLDOGDeviceThread : public TDeviceThread
{
private:
    int AnswerDataSize;
    int StartPCTimeOut;
    int LifeTimeOut;
    int ResetTimeOut;
    int ModemTimeOut;

    int GetInt16(BYTE* _data, int ind);
    long GetInt32(BYTE* _data, int ind);
    int BufferToCOM(BYTE* _data, int& length);
    int COMToBuffer(BYTE* _data, int& length);
    void SetInt16ToBuffer(int Value, BYTE* Buffer, int ind);

protected:
    virtual void SendPacket(BYTE command, BYTE* data = NULL, int datalen = 0);
    void ParseAnswer();

    virtual void PollingLoop();
    virtual void Life();
    virtual void ClearGSM();
    virtual void ResetPC();
    virtual void GetVersion();
    void         GetTimeOuts();
    void         SetTimeOuts();
    void         GetDoorSensors();
    void         SetID();
    virtual void ProcessOutCommand();
    virtual void __fastcall ProcessLoopCommand();
public:
    long ID;
    AnsiString version;
    BYTE Door_1;
    BYTE Door_2;

    __fastcall TLDOGDeviceThread();
    virtual __fastcall ~TLDOGDeviceThread();
};
#endif
