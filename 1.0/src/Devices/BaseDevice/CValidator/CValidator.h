//---------------------------------------------------------------------------

#ifndef CValidatorH
#define CValidatorH

#include "DeviceClass.h"
#include "LogClass.h"

enum OutCommands
{
    oc_DisableBill    = 1,
    oc_EnableBill     = 2,
    oc_getVersion     = 3
};

class CValidator : public TDeviceClass
{
private:
    bool m_enable;
protected:
    bool CheckFrozen();
    // 7 6 5 4 3 2 1 0
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
    union _BillsSensitivity
    {
        struct bytebits ByteBitCode;
        BYTE  ByteCode;
    } BillsSensitivity, BillsInhibit;

    virtual void ProcessOutCommand(){};


public:

    int ID;
    int mode;
    double ExchangeRate;
    double MinCash;

    CValidator(int id = 0, int ComPort = 1,TLogClass* _Log = NULL, AnsiString Prefix = "", int _mode = 0);
    virtual ~CValidator();

    virtual bool IsItYou();
    virtual bool IsInitialized();
    virtual void SetMaxCash(double value);
    virtual void SetMinCash(double value);
    virtual void EnableBill();
    virtual void DisableBill();
    virtual void ClearMoney();
    virtual double GetMoney();
    virtual bool CheckState();
    virtual void StartDevice();
    virtual bool SetBillsSensitivity(AnsiString value);
    bool isEnableBill();
    void getVersion();
};

//---------------------------------------------------------------------------
#endif

