//---------------------------------------------------------------------------

#ifndef JCMDeviceThreadH
#define JCMDeviceThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"
    typedef enum
    {
        STACKMOTORFAILURE     = 0xA2,
        FEEDMOTORSPEEDFAILURE = 0xA5,
        FEEDMOTORFAILURE      = 0xA6,
        CASHBOXNOTREADY       = 0xAB,
        VALIDATORHEADREMOVE   = 0xAF,
        BOOTROMFAILURE        = 0xB0,
        EXTERNALROMFAILURE    = 0xB1,
        ROMFAILURE            = 0xB2,
        EXTERNALROMWRITEFAILURE = 0xB3
    } TJCMSubState;

    typedef enum
    {
        ENABLE      = 0x11,
        ACCEPTING   = 0x12,
        ESCROW      = 0x13,
        STACKING    = 0x14,
        VENDVALID   = 0x15,
        STACKED     = 0x16,
        REJECTING   = 0x17,
        RETURNING   = 0x18,
        HOLDING     = 0x19,
        DISABLE     = 0x0A,
        _DISABLE     = 0x1A,
        INITIALIZE  = 0x1B,
        POWERUP     = 0x40,
        POWUPBILLACC= 0x41,
        POWUPBILLST = 0x42,
        STACKERFULL = 0x43,
        STACKEROPEN = 0x44,
        JAMINACCEPTOR = 0x45,
        JAMINSTACKER= 0x46,
        PAUSE       = 0x47,
        CHEATED     = 0x48,
        FAILURE     = 0x49,
        COMMUNICERROR = 0x4A,
        INVALIDCOMMAND= 0x4B
    } TJCMState;

class TJCMDeviceThread : public TDeviceThread
{
private:
    BYTE inter_crc_h;
    BYTE inter_crc_l;
    AnsiString BillDescr;
    int ResetCount;
    clock_t DisableTime;
    bool InitDisable;
    //bool EnterLoop;

    int OfflineCount;

    unsigned short Calc_CRC16CCNET(BYTE* DataBuf, unsigned short  BufLen);
    void calc_crc(BYTE byte);
    void Stack1();
    void Stack2();
    std::string GetStatusDescription(BYTE StatusCode);
    std::string GetRejectionDescription(BYTE StatusCode);
    std::string GetFailureDescription(BYTE StatusCode);
    virtual std::string GetStateDescription(int code);
protected:
    std::string RejectingDescr;
    std::string FailureDescr;

    WORD CRC16(BYTE* data, int len_packet);
    virtual void SendPacket(BYTE command,int len_packet, BYTE* data);
    virtual void ParseAnswer(int mode = 0);
    virtual void SendACK();
    virtual void GetStay();
    virtual void Reset();
    virtual void Return();
    virtual void Hold();
    virtual void Wait();
    virtual int  Stack();
    virtual void EnableAll();
    virtual void Disable();
    virtual void WriteEnableDenomination();
    virtual void PollingLoop();
    virtual void __fastcall ProcessLoopCommand();

    virtual void Security();
    virtual void ProcessOutCommand();
public:
    __fastcall TJCMDeviceThread();
    virtual __fastcall ~TJCMDeviceThread();
    bool IsItYou();
};
#endif
