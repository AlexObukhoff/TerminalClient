//---------------------------------------------------------------------------

#ifndef CCoinAcceptorH
#define CCoinAcceptorH

#include "DeviceClass.h"
#include "LogClass.h"

enum ca_OutCommands
{
    oc_Disable    = 1,
    oc_Enable     = 2,
    oc_GetID      = 3
};

class CCoinAcceptor : public TDeviceClass
{
private:
    bool m_enable;
protected:
    bool CheckFrozen();
    virtual void ProcessOutCommand(){};

public:
    int ID;
    double ExchangeRate;
    double MinCash;

    CCoinAcceptor(int id = 0, int ComPort = 1,TLogClass* _Log = NULL, AnsiString Prefix = "");
    virtual ~CCoinAcceptor();

    bool isEnableBill();
    virtual bool IsItYou();
    virtual bool IsInitialized();
    virtual void SetMaxCash(double value);
    virtual void SetMinCash(double value);
    virtual void Enable();
    virtual void Disable();
    virtual void ClearMoney();
    virtual double GetMoney();
    virtual bool CheckState();
    virtual void StartDevice();
};

//---------------------------------------------------------------------------
#endif

