//---------------------------------------------------------------------------

#ifndef DeviceThreadH
#define DeviceThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <SyncObjs.hpp>
#include <time.h>
//---------------------------------------------------------------------------
#include "LogClass.h"
#include "ComPortClass.h"
#include "ComPortParameters.h"
#include "DeviceState.h"
#include "CommandParameters.h"
#include "CommandParameters.h"
#include "common.h"
#include <vector>

#define DTBufferSize 1024

typedef enum
{
  EXTC_Free = 0,
  EXTC_1 = 1,
  EXTC_2 = 2,
  EXTC_3 = 3,
  EXTC_4 = 4,
  EXTC_5 = 5,
  EXTC_6 = 6,
  EXTC_7 = 7,
  EXTC_8 = 8,
  EXTC_9 = 9,
  EXTC_10 = 10,
  EXTC_11 = 11
} TExtCommand;

//---------------------------------------------------------------------------

class _info
{
protected:
    int _Status;
    virtual void SetStatus(int value);
    virtual int GetStatus();

    int _Result;
    virtual void SetResult(int value);
    virtual int GetResult();
public:
    enum
    {
        cs_Wait = 0,
        cs_Processing = 1,
        cs_Done = 2
    } TCompleteStatus;

    virtual void ClearMembers();
    _info();
    virtual ~_info(){};
    __property int Status = {read = GetStatus, write = SetStatus};
    __property int Result = {read = GetResult, write = SetResult};
};

//==============================================================================

class TDeviceThread : public TThread
{

protected:
    BYTE MainBuffer[DTBufferSize];
    BYTE SecondBuffer[DTBufferSize];
    BYTE TempBuffer[DTBufferSize];

    void ClearCommand();
    void ClearAnswer();
    void move(BYTE* Dest, BYTE* Sour, unsigned int count);
    void ClearBuffer(BYTE* Buffer);

    DCB dcb;
    bool ClearCOMPortError();
    bool Initialized;

    virtual int SendLowLevelCommand(BYTE* Command, int CommandSize, BYTE*& Answer, int& AnswerSize, TSendType type);

    //����� ����, �������������� ��������� ������������ �������� ������
    std::vector<BYTE> SimpleAnswers;
    BYTE lang; //0- english, 1 - russian
    //����� ������� ��� �������� ���������
    clock_t TimeStamp1;
    clock_t TimeStamp2;

    int PollCounter;
    int PollCounterLimit;

    int IsBufferContainSimpleAnswer(BYTE* Buffer);
    int GetBytePosition(BYTE* Buffer, int size ,BYTE value);
    virtual int PrepareAnswer(BYTE*& Buffer,int count);
    std::string GetByteExp(BYTE value);

    //������� ������� � ���� � ���� ������
    virtual int SendCommand(BYTE* Command, int CommandSize, BYTE*& Answer, int& AnswerSize, TSendType type = RecieveAnswer, int mode = 0);
    virtual int SendSingleCommand(BYTE* Command, int CommandSize, BYTE*& Answer, int& AnswerSize, TSendType type = RecieveAnswer);

    //������ ������
    virtual void ParseAnswer(int mode = 0){};

    void __fastcall Execute();
    virtual void __fastcall ProcessCommand(int mode = 0);
    virtual void __fastcall ProcessLoopCommand();
    virtual void LoopOfCommands();
    virtual void GetStay(){};
    virtual void PollingLoop(){};
    virtual bool ChangeDeviceState(bool wait = false);

    virtual bool CheckDeviceActivity(int parameter);//by default parameter = DeviceState->AnswerSize - as sign of device activity
public:
    __fastcall TDeviceThread(bool CreateSuspended, bool _PollingMode = false, DWORD _LifeTime = 5000);
    virtual __fastcall ~TDeviceThread();

    //callback ������� ��� ������ � �������� ������, ������ �������� ������� � .NET
    void _fastcall (__closure *DeviceStarted)(TDeviceState*);
    void _fastcall (__closure *DeviceStopped)(TDeviceState*);
    void _fastcall (__closure *DevicePaused)(TDeviceState*);
    void _fastcall (__closure *DeviceStateChanged)(TDeviceState*);
    //������� ���������� ��������� �������� ��������� ���������� �������
    void _fastcall (__closure *CommandStarted)(TDeviceState*);
    void _fastcall (__closure *CommandPaused)(TDeviceState*);
    void _fastcall (__closure *CommandFinished)(TDeviceState*);

    TCriticalSection* COMParametersCriticalSection;
    TCriticalSection* DeviceStateCriticalSection;
    TCriticalSection* CommandParametersCriticalSection;
    TCriticalSection* CommandCriticalSection;

    TLogClass* Log;
    TDeviceState* DeviceState;
    TCommandParameters* CommandParameters;
    TSendType SendType;
    TComPort* Port;
    TTimer* ChangeEvent;
    void AddSimpleAnswer(BYTE value);
    std::vector<BYTE> GetSimpleAnswers();
    //�������� ����������
    bool TerminateCommand;
    //���� ������������
    bool LoggingErrors;
    bool LoggingTimeoutErrors;
    //��� ������ ��������� ��������
    DWORD LastError;
    //���� � �������� ���������� ������� �����
    BYTE BeginByte;
    //���� ������� ������������� ������� �����
    BYTE EndByte;
    //����� CRC � ����� ������� �������
    int CRCLength;
    //������ � ������, ��� ����������� ����� ����������� ������
    int DataLengthIndex;
    //����� �������, ���� >0 �� ������������ ��� ����������� ����� ����������� ������
    int DataLength;
    //���� ��������
    bool Sending;
    bool OnlyPnP;

    bool ThreadTerminated;
    int CommandSize;
    BYTE* Command;
    int AnswerSize;
    BYTE* Answer;
    BYTE* data;
    int* len_data;

    //true ���� ����������� �����
    bool PollingMode;
    DWORD _PollingInterval;
    DWORD WriteReadTimeout;
    bool CleanPortBeforeReading;

    double ExchangeRate;
    double MaxCash;
    double MinCash;
    double bill_nominal;//������� �����
    int count_inkass;
    double bill_numb;
    AnsiString Currency;
    double bill_count;//���������� �����
    double bill_count_old;
    //�������� ����� ����������� � �����
    TList* DisabledNominals;
    bool EnterLoop;
    bool IsNominalEnabled(int value);

    //����� ������� ��� �������� ���������
    clock_t GetTimeStamp1();
    clock_t GetTimeStamp2();

    BYTE BillsSensitivity;
    BYTE BillsInhibit;
    
    //������� ��� ���������� ����� ������ �����
    BYTE ExtCommand;
    virtual BYTE GetExtCommand();
    virtual bool SetExtCommand(BYTE Command);
    virtual bool IsInitialized();
    virtual void SetInitialized();


    clock_t CreationTime; //����� ������� �������� ������
    DWORD LifeTime; //����� ����� ������ � ��

    virtual DWORD ReadPort(BYTE* Buffer, DWORD& BytesCount);
    bool ClearCOMPort();
    virtual std::string GetStateDescription(int code = 0){ return ""; };
    virtual bool IsItYou(){ return true; };
    virtual void SetCommand(BYTE command){};

    int mode;
};
//---------------------------------------------------------------------------
#endif
