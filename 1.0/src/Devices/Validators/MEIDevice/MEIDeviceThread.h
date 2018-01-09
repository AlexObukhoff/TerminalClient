//---------------------------------------------------------------------------

#ifndef MEIDeviceThreadH
#define MEIDeviceThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"
#include "MEIDeviceClass.h"
typedef enum
{
} TMEIState;
typedef enum
{
} TMEISubState;

// реальное расположение байтов в структуре будет c b7(hi) - b0(low)
struct bytebits
{
    unsigned b0 : 1;
    unsigned b1 : 1;
    unsigned b2 : 1;
    unsigned b3 : 1;
    unsigned b4 : 1;
    unsigned b5 : 1;
    unsigned b6 : 1;
    unsigned b7 : 1;
};
union States
{
    struct bytebits ByteBitCode;
    BYTE            ByteCode;
};
//=============================================
struct acc_bytebits0
{
    unsigned Idling     : 1;
    unsigned Accepting  : 1;
    unsigned Escrowed   : 1;
    unsigned Stacking   : 1;
    unsigned Stacked    : 1;
    unsigned Returning  : 1;
    unsigned Returned   : 1;
    unsigned Reserved   : 1;
};
union acc_States0
{
    struct acc_bytebits0  ByteBitCode;
    BYTE              ByteCode;
};
//=============================================
struct acc_bytebits1
{
    unsigned Cheated      : 1;
    unsigned Rejected     : 1;
    unsigned Jammed       : 1;
    unsigned CassetteFull : 1;
    unsigned LRC_Status   : 1;
    unsigned Paused       : 1;
    unsigned Calibration  : 1;
    unsigned Reserved     : 1;
};
union acc_States1
{
    struct acc_bytebits1  ByteBitCode;
    BYTE              ByteCode;
};
//=============================================
struct acc_bytebits2
{
    unsigned PowerUp      : 1;
    unsigned InvalidCommand : 1;
    unsigned Failure      : 1;
    unsigned NoteValue0   : 1;
    unsigned NoteValue1   : 1;
    unsigned NoteValue2   : 1;
    unsigned Always0      : 1;
    unsigned Reserved     : 1;
};
union acc_States2
{
    struct acc_bytebits2  ByteBitCode;
    BYTE              ByteCode;
};
//=============================================
struct acc_bytebits3
{
    unsigned NoPushMode   : 1;
    unsigned FlashDownload  : 1;
    unsigned Pre_Stack    : 1;
    unsigned Always0_0    : 1;
    unsigned Always0_1    : 1;
    unsigned Always0_2    : 1;
    unsigned Always0_3    : 1;
    unsigned Reserved     : 1;
};
union acc_States3
{
    struct acc_bytebits3  ByteBitCode;
    BYTE              ByteCode;
};
//=============================================
//=============================================
struct host_bytebits0
{
    unsigned Denomination0  : 1;
    unsigned Denomination1  : 1;
    unsigned Denomination2  : 1;
    unsigned Denomination3  : 1;
    unsigned Denomination4  : 1;
    unsigned Denomination5  : 1;
    unsigned Denomination6  : 1;
    unsigned Reserved       : 1;
};
union host_Data0
{
    struct host_bytebits0   ByteBitCode;
    BYTE                    ByteCode;
};
//=============================================
struct host_bytebits1
{
    unsigned InterruptMode  : 1;
    unsigned Security       : 1;
    unsigned Orientation0   : 1;
    unsigned Orientation1   : 1;
    unsigned EscrowEnable   : 1;
    unsigned Stack          : 1;
    unsigned Return         : 1;
    unsigned Reserved       : 1;
};
union host_Data1
{
    struct host_bytebits1   ByteBitCode;
    BYTE                    ByteCode;
};
//=============================================
struct host_bytebits2
{
    unsigned NoPushMode            : 1;
    unsigned EnableDecodBarCode  : 1;
    unsigned EnablePowerUpBSeq   : 1;
    unsigned EnablePowerUpCSeq   : 1;
    unsigned Mode                : 1;
    unsigned Reserved0_1         : 1;
    unsigned Reserved0_2         : 1;
    unsigned Reserved            : 1;
};
union host_Data2
{
    struct host_bytebits2   ByteBitCode;
    BYTE                    ByteCode;
};
//=============================================
class TMEIDeviceThread : public TDeviceThread
{
private:
    int OfflineCount;
    unsigned short Calc_CRC(BYTE* DataBuf);
    BYTE SendData[6];
    BYTE AnswerData[25];

    acc_States0 acc_Byte0;
    acc_States1 acc_Byte1;
    acc_States2 acc_Byte2;
    acc_States3 acc_Byte3;

    host_Data0  host_Byte0;
    host_Data1  host_Byte1;
    host_Data2  host_Byte2;

    BYTE LastACK;
    bool ACKAnswered;
    BYTE GetACKNumber();
    __property BYTE ACK = {read = GetACKNumber};

    void ClearByteFields();
    int GetStateCodeFromByteFields();
    int GetCurrentNominal(MEI_MODE::MEI_MODE mode = MEI_MODE::Simple);
    double FirmwareVersion;
    void getVersion();
protected:
    virtual void SendPacket(BYTE* _data);
    virtual void ParseAnswer(int mode = 0);

    void Poll(int mode = 0);
    void Stack();
    void Escrow();
    void Return();
    void Enable();
    void Disable();
    void Reset();

    virtual void __fastcall TMEIDeviceThread::ProcessLoopCommand();
    virtual void PollingLoop();
    virtual void ProcessOutCommand();
public:
    __fastcall TMEIDeviceThread(int mode = 0);
    virtual __fastcall ~TMEIDeviceThread();
    bool IsItYou();
};
#endif
