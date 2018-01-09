//---------------------------------------------------------------------------
#ifndef ModemH
#define ModemH

#include "DeviceClass.h"
#include "LogClass.h"
#include <string>
//---------------------------------------------------------------------------
#define DATA_COMMAND    0
#define AT_COMMAND      1
#define CALL_COMMAND    2


class CModem : public TDeviceClass
{
private:
protected:
    BYTE EndSignal;
    void GetOperatorItems(AnsiString Answer, int index, int& OperatorID, AnsiString& OperatorName);
    bool ModemEnable;

    bool ReopenOn19200();
    bool ReopenOn115200();

    void SendCommand();
    void SendPacket(BYTE* command, int bytes_count,int datalen, BYTE* data, bool datafirst = false);
    bool SendATCommand(BYTE* ATCommand = (BYTE *)"", int AtCommand = AT_COMMAND);
    std::string GetAnswerBuffer(long timer_loop_limit, const char* WaitWord = NULL, bool Scan = true);
    DWORD ReadPort(BYTE* Buffer);
public:
    BYTE Error;

    CModem(int ComPort,TLogClass* _Log = NULL, AnsiString Prefix = "");
    virtual ~CModem(){}

    AnsiString GetModemType();
    virtual void    Start();
    virtual float   SignalQuality() = 0;
    virtual double GetBalance(AnsiString BalanceNumber, int RepeatRequest = 5){return 0;};
    virtual bool    SendSMS(const char* SmsNumber, const char* SmsText);
    virtual int     InitModem();
    virtual AnsiString GetOperatorName(int ID);
    virtual AnsiString GetOperatorName();
    //Siemens MC39
    virtual bool ResetModem();
    virtual bool ChangeSIM_No(int SIM_No);
    virtual bool ChangeSIM();
    AnsiString SendCommand(AnsiString Command);
};
#endif
 