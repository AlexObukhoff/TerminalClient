//---------------------------------------------------------------------------

#ifndef BaseCardReaderH
#define BaseCardReaderH
//---------------------------------------------------------------------------
#include "DeviceClass.h"
#include "LogClass.h"

class CBaseCardReader : public TDeviceClass
{
public:
    CBaseCardReader(unsigned int comPortNum, std::string prefix, TLogClass* log);
    virtual ~CBaseCardReader() {};

    // можно было конечно не выжовываться и просто использовать существующие методы StartPooling / StopPooling
    virtual void StartWaitData();
    virtual void StopWaitData();
    virtual int Initialize();
    bool IsWaitingData;
};

class CNullCardReader : public CBaseCardReader
{
public:
    CNullCardReader(unsigned int comPortNum, TLogClass* pLog = 0);
    ~CNullCardReader();

    // from CBaseCardReader
    virtual void StartWaitData();
    virtual void StopWaitData();

    // from TDeviceClass
    virtual void Start();
    virtual void Stop();
};
#endif
