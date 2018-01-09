//---------------------------------------------------------------------------

#ifndef ICTDeviceThreadH
#define ICTDeviceThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"
    typedef enum
    {
        POWERUP1     = 0x80,
        POWERUP2     = 0x8F,
        COMMUNICERROR= 0x26,
        ESCROW       = 0x81,
        BILLTYPE1    = 0x40,
        BILLTYPE2    = 0x41,
        BILLTYPE3    = 0x42,
        BILLTYPE4    = 0x43,
        BILLTYPE5    = 0x44,
        //new 06-02-2006
        BILLTYPE6    = 0x45,
        BILLTYPE7    = 0x46,
        BILLTYPE8    = 0x47,
        BILLTYPE9    = 0x48,
        BILLTYPE10    = 0x49,

        STACKED      = 0x10,
        BILLACCEPTFAIL        = 0x11,
        STACKMOTORFAILURE      = 0x20,
        CHECKSUMERROR          = 0x21,
        BILLJAM                = 0x22,
        BILLREMOVE             = 0x23,
        STACKEROPEN            = 0x24,
        SENSORPROBLEM          = 0x25,
        BILLFISH               = 0x27,
        STACKERPROBLEM         = 0x28,
        BILLREJECT             = 0x29,
        INVALIDCOMMAND         = 0x2A,
        ERRORSTATUSISEXCLUSION = 0x2F,
        ENABLE                 = 0x3E,
        DISABLE                = 0x5E
    } TICTState;

class TICTDeviceThread : public TDeviceThread
{
private:
    AnsiString BillDescr;
    std::string GetStatusDescription(BYTE StatusCode);
    virtual std::string GetStateDescription();
    bool InitDisable;
    clock_t DisableTime;
protected:
    std::string RejectingDescr;
    AnsiString FailureDescr;
    void CheckState(BYTE code);
    int OfflineCount;

    WORD CRC16(BYTE* data, int len_packet);
    virtual void __fastcall ProcessLoopCommand();
    virtual void SendPacket(BYTE command,int len_packet, BYTE* data);
    virtual void ParseAnswer(int mode = 0);
    virtual void SendACK();
    virtual void GetStay();
    virtual void Reset();
    virtual void Return();
    virtual void EnableAll();
    virtual void Disable();
    virtual void PollingLoop();
    virtual void ProcessOutCommand();
    virtual bool ChangeDeviceState(bool wait = false);
public:
    __fastcall TICTDeviceThread();
    virtual __fastcall ~TICTDeviceThread();

    virtual bool IsItYou();
};
#endif
