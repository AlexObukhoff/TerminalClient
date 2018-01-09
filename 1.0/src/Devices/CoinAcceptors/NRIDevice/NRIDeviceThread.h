//---------------------------------------------------------------------------

#ifndef NRIDeviceThreadH
#define NRIDeviceThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

enum CoinAcceptorErrorCode
{
    err_NullEvent                     = 0,
    err_RejectCoin                    = 1,
    err_InhibitedCoin                 = 2,
    err_MultipleWindow                = 3,
    err_WakeUpTimeOut                 = 4,
    err_ValidationTimeOut             = 5,
    err_CreditSensorTimeOut           = 6,
    err_SorterOptoTimeOut             = 7,
    err_SecondCloseCoinError          = 8,
    err_AcceptGateNotReady            = 9,
    err_CreditSensorNotReady          = 10,
    err_SorterNotReady                = 11,
    err_RejectedCoinNotCleared        = 12,
    err_ValidationSensorNotReady      = 13,
    err_CreditSensorBlocked           = 14,
    err_SorterOptoBlocked             = 15,
    err_CreditSequenceError           = 16,
    err_CoinGoingBackwards            = 17,
    err_CoinTooFast                   = 18,
    err_CoinTooSlow                   = 19,
    err_COSMechanismActivated         = 20,
    err_DSEOptoTimeOut                = 21,
    err_DCEOptoNotSeen                = 22,
    err_CreditSensorReachedTooEarly   = 23,
    err_RejectCoinAlt                 = 24,
    err_RejectSlug                    = 25,
    err_RejectSensorBlocked           = 26,
    err_GamesOverload                 = 27,
    err_MaxCoinMeterPulsesExceeded    = 28,
    err_AcceptGateOpenNotClosed       = 29,
    err_AcceptGateClosedNotOpen       = 30,
    err_InhibitedCoinType1            = 128,
    err_InhibitedCoinType32           = 159,
    err_DataBlockRequest              = 253,
    err_CoinReturnMechanismActivated  = 254,
    err_UnspecifiedAlarmCode          = 255
};

class TNRIDeviceThread : public TDeviceThread
{
private:
    BYTE Coins[12];
    int coinsNominals[16];
    AnsiString EquipmentID;

    BYTE NewEvents;
    BYTE LastEventCounter;
    BYTE EventCounter;
    BYTE EventA[5];
    BYTE EventB[5];

    int CoinID;
    int CoinNominal;
    //int GetCoinNominal(int id);
    std::string GetStateDescription(int code);
    std::string GetFaultDescription(int code, int subcode = 0);

    unsigned short Calc_CRC(BYTE* DataBuf, int count);
protected:
    AnsiString BillDescr;
    AnsiString RejectingDescr;
    AnsiString FailureDescr;
    int AnswerDataSize;

    virtual void SendPacket(int header, BYTE* _data = NULL, int datalen = 0);
    int PrepareAnswer(BYTE*& Buffer,int count);
    virtual void ParseAnswer();
    int SendLowLevelCommand(BYTE* Command, int CommandSize, BYTE*& Answer, int& AnswerSize, TSendType type);

    void Poll();
    void GetID();
    void GetStatus();
    int ReadLastCreditOrError();
    int PerformSelfCheck();
    void ModifyInhibitStatus();
    void Enable();
    void Disable();
    void getCoinNominals();

    virtual void __fastcall ProcessLoopCommand();
    virtual void PollingLoop();
    virtual void ProcessOutCommand();
public:
    __fastcall TNRIDeviceThread();
    virtual __fastcall ~TNRIDeviceThread();
    bool IsItYou();
};
#endif
