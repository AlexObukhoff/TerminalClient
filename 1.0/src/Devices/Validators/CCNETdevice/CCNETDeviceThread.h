//---------------------------------------------------------------------------

#ifndef CCNETDeviceThreadH
#define CCNETDeviceThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

class TCCNETDeviceThread : public TDeviceThread
{
protected:
    BYTE inter_crc_h;
    BYTE inter_crc_l;
    unsigned int mask;

    int OfflineCount;

    AnsiString BillDescr;
    AnsiString RejectingDescr;
    AnsiString FailureDescr;

    int ResetCount;
    clock_t DisableTime;
    bool InitDisable;
    int LastEnDisCommand;//0-ok,1-disable,2-enable;

    virtual void SendPacket(BYTE command,int len_packet, BYTE* data);
    virtual void calc_crc(BYTE byte);
    unsigned short Calc_CRC16CCNET(BYTE* DataBuf, unsigned short  BufLen);

    virtual void ParseAnswer(int mode = 0);
    virtual void GetStay();
    virtual void Reset();
    virtual void Return();
    virtual void Hold();
    virtual int  Stack();
    virtual void EnableAll();
    virtual void Disable();
    virtual void PollingLoop();
    virtual void SendACK();
    virtual void SendNAK();

    std::string GetMainFailureDescription(BYTE StatusCode);
    std::string GetHardwareFailureDescription(BYTE StatusCode);
    std::string GetStatusDescription(BYTE StatusCode);
    AnsiString GetRejectionDescription(BYTE StatusCode);
    AnsiString GetFailureDescription(BYTE StatusCode);
    virtual std::string GetStateDescription();
    virtual void __fastcall ProcessLoopCommand();

    virtual void ProcessOutCommand();
public:
    __fastcall TCCNETDeviceThread();
    virtual __fastcall ~TCCNETDeviceThread();
    void CommandDisable();
    bool IsItYou();
};
#endif
