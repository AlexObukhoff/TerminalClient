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
  //буфер для команды
  BYTE Command[DCBufferSize];
  BYTE Answer[DCBufferSize];
  BYTE data[DCBufferSize];
  int len_data;
  int CommandSize;
  int AnswerSize;

  //писать ли низкоуровневые команды в лог
  bool LoggingErrors;
  //код ошибки последней операции
  DWORD LastError;
  //байт с которого начинается сложный ответ
  BYTE BeginByte;
  //байт которым заканчивается сложный ответ
  BYTE EndByte;
  //длина CRC в конце сложной команды
  int CRCLength;
  //индекс в буфере, где указывается длина принимаемых данных
  int DataLengthIndex;
  //длина команды, если >0 то используется для определения длины считываемых данных
  int DataLength;
  //выставляем флаг если инициализация завершена
  bool Initialized;
  //метки времени
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
  //Критическая секция для доступа к классу полей параметров СОМ порта
  TCriticalSection* COMParametersCriticalSection;
  //Критическая секция для доступа к классу полей, определяющему состояние устройства
  TCriticalSection* DeviceStateCriticalSection;
  //Критическая секция для доступа к классу полей, определяющему поля команды
  TCriticalSection* CommandParametersCriticalSection;
  //Критическая секция для выполнения команды
  TCriticalSection* CommandCriticalSection;
  //класс потока выполняющий функцию для работы с устройством, только один поток
  TDeviceThread* DeviceThread;

  //класс СОМ порта
  TComPort* Port;
  //класс параметров СОМ порта
  TComPortInitParameters* COMParameters;

  //класс отражающий состояние устройства
  TDeviceState* DeviceState;
  //класс определяющий команду на отправку
  TCommandParameters* CommandParameters;
  //класс логирования
  TLogClass* Log;
  //указывает, принимать или нет ответ при передаче команды в порт
  TSendType SendType;
  AnsiString Currency;
  double  MaxCash;

protected:
  //список событий
  TList*  Events;
  DWORD EventID;

  //время жизни потока (для принтеров)
  DWORD ThreadLifeTime;
  DWORD FrozenTimeOut;//время в секундах, через которое поток будет считаться зависшим (для купюрников)
  //метки времени
  clock_t GetTimeStamp1();
  clock_t GetTimeStamp2();


  virtual void Start();
  //начать выполнение потока
  void Resume();

  void StopThread();
  bool GlobalStop;

  virtual void _fastcall StateChanged(TDeviceState* State);
  void WaitSendingFinish();
  //номиналы купюр запрещённые к приёму
  TList* DisabledNominals;

  //callback функции для вызова в основном потоке, аналог делегата события в .NET
  //использовать для вызова Synchronize и проверку на NULL
  void _fastcall (__closure *DeviceStarted)(TDeviceState*);
  void _fastcall (__closure *DeviceStopped)(TDeviceState*);
  void _fastcall (__closure *DevicePaused)(TDeviceState*);
  void _fastcall (__closure *DeviceStateChanged)(TDeviceState*);
  //события сообщающие основному процессу состояние выполнения команды
  void _fastcall (__closure *CommandStarted)(TDeviceState*);
  void _fastcall (__closure *CommandPaused)(TDeviceState*);
  void _fastcall (__closure *CommandFinished)(TDeviceState*);

  //Выполнить команду
  //type = RecieveAnswer говорит о том, что нужно ждать ответа от устройства
  virtual void ExecuteCommand(TSendType type = RecieveAnswer);
  //virtual void SendCommand();

public:

  //TDeviceClass(int ComPort, int BaudRate = 0, TLogClass* _Log = NULL, AnsiString Prefix = "");
  TDeviceClass(int ComPort, TLogClass* _Log = NULL, AnsiString Prefix = "", PortType::Enum portType = PortType::com);
  virtual ~TDeviceClass();

  TTimer* ChangeEvent;
  int MaxEventsCount;

  //остановить и убить работающий поток
  virtual void Stop();
  //запустить циклическую команду, параметры которой определяются в CommandParameters
  virtual void StartPooling();
  //остановить циклическую команду
  virtual void StopPooling();
  //получить состояние во время опроса устройства
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

