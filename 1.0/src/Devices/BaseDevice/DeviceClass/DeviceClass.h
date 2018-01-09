#ifndef DeviceClassH
#define DeviceClassH
#include <Classes.hpp>
#include <SyncObjs.hpp>

#include "LogClass.h"
#include "ComPortClass.h"
#include "ComPortParameters.h"
#include "DeviceThread.h"
#include "DeviceState.h"
#include "CommandParameters.h"

#define DCBufferSize 1024

class TDeviceClass
{
protected:
   bool NewLog;
  //����� ��� �������
  BYTE Command[DCBufferSize];
  BYTE Answer[DCBufferSize];
  BYTE data[DCBufferSize];
  int len_data;
  int CommandSize;
  int AnswerSize;

  //������ �� �������������� ������� � ���
  bool LoggingErrors;
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
  //���������� ���� ���� ������������� ���������
  bool Initialized;
  //����� �������
  clock_t TimeStamp1;
  clock_t TimeStamp2;


  BYTE ExtCommand;
  virtual BYTE GetExtCommand();
  virtual bool SetExtCommand(BYTE Command);

  void ClearBuffer(BYTE* Buffer);
  void ClearCommand();
  void ClearAnswer();

  virtual void SetEvent(TDeviceState* Event);

public:
  //����������� ������ ��� ������� � ������ ����� ���������� ��� �����
  TCriticalSection* COMParametersCriticalSection;
  //����������� ������ ��� ������� � ������ �����, ������������� ��������� ����������
  TCriticalSection* DeviceStateCriticalSection;
  //����������� ������ ��� ������� � ������ �����, ������������� ���� �������
  TCriticalSection* CommandParametersCriticalSection;
  //����������� ������ ��� ���������� �������
  TCriticalSection* CommandCriticalSection;
  //����� ������ ����������� ������� ��� ������ � �����������, ������ ���� �����
  TDeviceThread* DeviceThread;

  //����� ��� �����
  TComPort* Port;
  //����� ���������� ��� �����
  TComPortInitParameters* COMParameters;

  //����� ���������� ��������� ����������
  TDeviceState* DeviceState;
  //����� ������������ ������� �� ��������
  TCommandParameters* CommandParameters;
  //����� �����������
  TLogClass* Log;
  //���������, ��������� ��� ��� ����� ��� �������� ������� � ����
  TSendType SendType;
  AnsiString Currency;
  double  MaxCash;

protected:
  //������ �������
  TList*  Events;
  DWORD EventID;

  //����� ����� ������ (��� ���������)
  DWORD ThreadLifeTime;
  DWORD FrozenTimeOut;//����� � ��������, ����� ������� ����� ����� ��������� �������� (��� ����������)
  //����� �������
  clock_t GetTimeStamp1();
  clock_t GetTimeStamp2();


  virtual void Start();
  //������ ���������� ������
  void Resume();

  void StopThread();
  bool GlobalStop;

  virtual void _fastcall StateChanged(TDeviceState* State);
  void WaitSendingFinish();
  //�������� ����� ����������� � �����
  TList* DisabledNominals;

  //callback ������� ��� ������ � �������� ������, ������ �������� ������� � .NET
  //������������ ��� ������ Synchronize � �������� �� NULL
  void _fastcall (__closure *DeviceStarted)(TDeviceState*);
  void _fastcall (__closure *DeviceStopped)(TDeviceState*);
  void _fastcall (__closure *DevicePaused)(TDeviceState*);
  void _fastcall (__closure *DeviceStateChanged)(TDeviceState*);
  //������� ���������� ��������� �������� ��������� ���������� �������
  void _fastcall (__closure *CommandStarted)(TDeviceState*);
  void _fastcall (__closure *CommandPaused)(TDeviceState*);
  void _fastcall (__closure *CommandFinished)(TDeviceState*);

  //��������� �������
  //type = RecieveAnswer ������� � ���, ��� ����� ����� ������ �� ����������
  virtual void ExecuteCommand(TSendType type = RecieveAnswer);
  //virtual void SendCommand();

public:

  //TDeviceClass(int ComPort, int BaudRate = 0, TLogClass* _Log = NULL, AnsiString Prefix = "");
  TDeviceClass(int ComPort, TLogClass* _Log = NULL, AnsiString Prefix = "", PortType::Enum portType = PortType::com);
  virtual ~TDeviceClass();

  TTimer* ChangeEvent;
  int MaxEventsCount;

  //���������� � ����� ���������� �����
  virtual void Stop();
  //��������� ����������� �������, ��������� ������� ������������ � CommandParameters
  virtual void StartPooling();
  //���������� ����������� �������
  virtual void StopPooling();
  //�������� ��������� �� ����� ������ ����������
  virtual TCommandParameters* GetResult();
  std::string GetStateDescription();

  virtual void DisableNominal(int value);
  virtual void EnableNominal(int value);
  virtual bool IsNominalEnabled(int value);
  virtual bool IsStateChanged();
  virtual void StartDevice();
  virtual double GetMoney();
  virtual void ClearMoney();
  virtual bool IsInitialized();
  virtual void SetInitialized();
  virtual int Initialize();


  virtual bool IsItYou();
  virtual void SignalEvent(BYTE StatusCode);
  virtual bool ChangeDeviceState(bool wait = false);
  AnsiString DeviceName;

  virtual bool GetEvent(TDeviceState* Event);
  int GetEventsCount();

  int ErrorMode;
};

#endif

