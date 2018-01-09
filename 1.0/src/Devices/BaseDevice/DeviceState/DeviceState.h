#ifndef DeviceStateH
#define DeviceStateH

#include <SyncObjs.hpp>
#include <string>
#include "ComPortParameters.h"
#include "LogClass.h"

typedef enum
{
  DsrCtsOff = 0,
  DsrCtsOn = 1,
  DsrOn = 2,
  CtsOn = 3
} TDsrCts;

typedef enum
{
  DSE_OK = 0,
  DSE_NOTMOUNT = 1,
  DSE_NOTPAPER = 2,
  DSE_NEARENDPAPER = 3,
  DSE_PAPERJAM = 4,
  DSE_PAPERRELOAD = 5,  //new 10-07-2008
  //DSE_MAINERROR = 5,
  
  //for validator
  DSE_STACKEROPEN = 6,
  DSE_STACKERFULL = 7,
  DSE_BILLJAM = 8,
  DSE_CHEATED = 9,
  //DSE_FATALERROR = 10,
  DSE_SETCASSETTE = 11,

  //new 29-06-2007
  DSE_HARDWARE_ERROR = 12,
  DSE_BILLREJECT = 10,

  //new 24-09-2007 for watchdog
  DSE_SENSOR_1_ON   = 30,
  DSE_SENSOR_1_OFF  = 31,
  DSE_SENSOR_2_ON   = 32,
  DSE_SENSOR_2_OFF  = 33,
  DSE_SENSOR_3_ON   = 34,
  DSE_SENSOR_3_OFF  = 35,

  // astafiev for CardReader
  DSE_NODATA    = 100,

  DSE_PRN_HEADUP = 101,
  DSE_UNKNOWN = 255
} TDeviceStatesAndErrors;

typedef enum
{
  //    States
  DSE_NOERROR = 0,                             //устройство работает нормально
  //    Critical
  DSE_OFFLINE = 1,                             //устройство не работает
  DSE_UNKNOWN_CODE = -2,                        //состояние не определено
  DSE_NOTSET = 3,                              //нехначимый код

  //========Printens===============
  //    States
  PRN_FISCAL_MODE = 900,                      //Фискальный режим
  PRN_NONFISCAL_MODE = 901,                   //Нефискальный режим
  //    NonCritical
  PRN_PAPER_NEAREND = 1000,                    //бумага скоро кончится
  PRN_NEED_CLOSE_SHIFT = 1001,                 //необходимо закрыть смену
  PRN_INCORRECT_DATE_TIME = 1002,              //ошибка даты времени
  PRN_NEED_OPEN_SESSION = 1003,                //необходимо начало сеанса
  PRN_TOO_BIG_RESULT = 1004,                   //слишком большой результат
  PRN_NO_CASH_FOR_OPERATION = 1005,            //нет наличных для операции
  PRN_NEED_SERTIFICATION = 1006,               //необходима сертификация
  PRN_NEED_FISCALIZATION = 1007,               //необходима фискализация
  PRN_RECIEVING_ERROR = 1008,                  //ошибка приёма
  //    Critical
  PRN_NO_PAPER = 1100,                         //бумага кончилась
  PRN_PAPER_JAM = 1101,                        //бумага застряла
  PRN_SOFTWARE_ERROR = 1102,                   //ошибка передачи данных
  PRN_HARDWARE_ERROR = 1103,                   //сбой оборудования
  PRN_DSR_OFF = 1104,                          //DSR отсутствует
  PRN_ROM_ERROR = 1105,                        //ошибка ПЗУ
  PRN_FISCAL_MEMORY_NOTMOUNT = 1106,           //ФП не обнаружена
  PRN_FISCAL_MEMORY_ERROR = 1107,              //ошибка связи с ФП
  PRN_CUTTER_ERROR = 1108,                     //ошибка отрезчика
  PRN_RAM_ERROR = 1109,                        //ошибка OЗУ
  PRN_VOLTAGE_ERROR = 1110,                    //ошибка напряжения
  PRN_NO_GENERATOR_SIGNAL = 1111,              //нет сигналов с тахогенератора
  PRN_NO_SENSOR_SIGNAL = 1112,                 //нет сигналов датчиков
  PRN_EKLZ_ERROR = 1113,                       //ошибка ЭКЛЗ
  PRN_CONTROL_MEMORY_ERROR = 1114,             //ошибка контрольной памяти
  PRN_INCORRECT_PASSWORD = 1115,               //неверный пароль передачи данных
  PRN_PRINTER_NOT_READY = 1116,                //принтер не готов
  PRN_PRINTER_INCORRECT_STATE = 1117,          //неверное состояние принтера
  PRN_FISCAL_MEMORY_INIT_REQUIRED = 1118,      //необходима инициализация фискальной памяти
  //========Validators=============

  //    States
  VLD_BILL_ENABLE = 2001,                      //приём купуюр разрешён
  VLD_BILL_DISABLE = 2002,                     //приём купюр запрешён
  VLD_STACKER_OPENED = 2003,                   //стэкер открыт
  VLD_REJECTING = 2004,                        //выброс
  VLD_ACCEPTING = 2005,                        //проверка купюры
  VLD_STACKING = 2006,                         //укладка купюры
  VLD_RETURNING = 2007,                        //возврат купюры
  VLD_HOLDING = 2008,                          //удержание купюры
  VLD_BUSY = 2009,                             //занят
  VLD_PAUSED = 2010,                           //пауза
  VLD_STACKED = 2011,                          //купюра уложена
  VLD_RETURNED = 2012,                         //купюра возвращена
  VLD_DISPENSING = 2013,                       //выдача
  VLD_UNLOADING = 2014,                        //выгрузка
  VLD_SETTING_TYPE_CASSETTE = 2015,            //установка кассеты
  VLD_DISPENSED = 2016,                        //выдано
  VLD_UNLOADED = 2017,                         //выгружено
  VLD_STACKER_CLOSED = 2018,                   //стэкер закрыт
  VLD_INITIALIZE = 2019,                       //инициализация
  VLD_POWER_UP = 2020,                         //включение питания

  //    NonCritical
  VLD_INVALID_BILL_NUMBER = 2101,              //неверная купюра
  VLD_INVALID_COMMAND = 2102,                  //неверная команда
  VLD_INSERTION_ERROR = 2103,                  //ошибка загрузки
  VLD_IDENTIFICATION_ERROR = 2104,             //ошибка идентификации
  VLD_VERIFICATION_ERROR = 2105,               //ошибка проверки
  VLD_RETURN_BY_INHIBIT_ERROR = 2106,          //возврат запрещённой купюры
  VLD_LENGTH_ERROR = 2107,                     //ошибка длины
  VLD_POWER_UP_WITH_BILL_IN_ACCEPTOR = 2108,   //включение питания с купюрой в акцепторе
  VLD_POWER_UP_WITH_BILL_IN_STACKER = 2109,    //включение питания с купюрой в стэкере
  VLD_COMMUNICATION_ERROR = 2110,              //ошибка связи
  VLD_HEAD_REMOVE = 2111,                      //снят акцептор
  VLD_DENOMINATION_ACCESSING_ERROR = 2112,     //запрещённая купюра
  VLD_PHOTO_PATTERN_ERROR = 2113,              //ошибка проверки паттерна
  VLD_PHOTO_LEVEL_ERROR = 2114,                //ошибка проверки фото уровня
  VLD_CHECKSUM_ERROR = 2115,                   //ошибка контрольной суммы
  VLD_BILL_REMOVE = 2116,                      //вытаскивание купюры
  VLD_BILLFISH = 2117,                         //ошибка BILLFISH
  VLD_ERROR_STATUS_IS_EXCLUSION = 2118,        //ошибка неверного статуса
  VLD_SLOW_DRIVE = 2119,                       //механизм работает медленно
  VLD_STREAMING = 2120,                        //попытка стриминга
  VLD_DEPOSIT_DECLINE = 2121,                  //запрет депозита
  VLD_UNKNOWN_ERROR = 2122,                    //ошибка неизвестна

  //    Critical
  VLD_STACKER_FULL = 2200,                     //стекер забит
  VLD_BILL_JAM = 2202,                         //купюра замята
  VLD_CHEATED = 2203,                          //читинг
  VLD_JAM_IN_ACCEPTOR = 2204,                  //замятие купюры в акцепторе
  VLD_JAM_IN_STACKER = 2205,                   //замятие купюры в стекере
  VLD_DIELECTRIC_ERROR = 2206,                 //ошибка диэлектрики
  VLD_BILL_IN_HEAD = 2207,                     //купюра в приёмном механизме
  VLD_COMPENSATION_FACTOR_ERROR = 2208,        //ошибка фактора компенсации
  VLD_BILL_TRANSPORT_ERROR = 2209,             //ошибка транспортировки
  VLD_OPTIC_SENSOR_ERROR = 2210,               //ошибка оптического сенсора
  VLD_CAPACISTANCE_ERROR = 2211,               //ошибка ёмкости
  VLD_OPERATION_ERROR = 2212,                  //ошибка операции
  VLD_STACK_MOTOR_FAILURE = 2213,              //сбой мотора стекера
  VLD_TRANSPORT_SPEED_MOTOR_FAILURE = 2214,    //сбой мотора скорости транспортировки
  VLD_TRANSPORT_MOTOR_FAILURE = 2215,          //сбой  мотора транспортировки
  VLD_ALIGNING_MOTOR_FAILURE = 2216,           //сбой  мотора укладки
  VLD_CASSETTE_FAILURE = 2217,                 //сбой кассеты
  VLD_OPTICAL_CANAL_FAILURE = 2218,            //сбой оптического канала
  VLD_MAGNETICAL_CANAL_FAILURE = 2219,         //сбой магнитного канала
  VLD_CAPACITANCE_CANAL_FAILURE = 2220,        //сбой ёмкостного канала
  VLD_BOOT_ROM_FAILURE = 2221,                 //сбой BOOT ROM
  VLD_EXTERNAL_ROM_FAILURE = 2222,             //сбой внешней ROM
  VLD_ROM_FAILURE = 2223,                      //сбой ROM
  VLD_EXTERNAL_ROM_WRITE_FAILURE = 2224,       //сбой записи внешней ROM
  VLD_MAG_ERROR = 2225,                        //сбой MAG
  VLD_COMPENSATION_MULTIPLYING_ERROR = 2226,   //сбой COMPENSATION MULTIPLYING
  VLD_CONVEYING_ERROR = 2227,                  //сбой укладки купюры
  VLD_SENSOR_PROBLEM = 2228,                   //проблема с сенсором
  VLD_STACKER_PROBLEM = 2229,                  //сбой стэкера

  //========WatchDogs==============
  //    States
  WCD_LOWBOX_HIGHBOX_BOX_CLOSED = 3001,        //Нижний бокс, верхний бокс, сейф закрыт
  WCD_BOX_OPENED = 3002,                       //сейф открыт
  WCD_HIGH_BOX_OPENED = 3003,                  //верхний бокс открыт
  WCD_LOW_BOX_OPENED = 3004,                   //нижний бокс открыт
  WCD_ACTIVATOR_OPENED = 3005,                 //активатор открыт
  WCD_ACTIVATOR_CLOSED = 3006,                 //активатор закрыт
  WCD_ACTIVATOR_ERROR = 3007,                  //ошибка активатора

  //    NonCritical
  WCD_BEAT = 3100,                             //Удар
  WCD_INCLINATION = 3101,                      //Наклон
  WCD_BEAT_AND_INCLINATION = 3102              //Удар и наклон
} TDeviceStatesAndErrorsEx;

typedef enum
{
  NotRun = 0,
  Wait = 1,
  CommandExecuting = 2,
  Undeterminated = 0xFF
} TDeviceStateEnum;


#define StatusesBufferSize 100

class TDeviceState
{
private:
protected:
  //Критическая секция для доступа к классу полей, опредляющему состояние устройства
  //будем блокировать, перед изменением всех полей
  TCriticalSection* DeviceStateCriticalSection;
public:
  TDeviceState(TCriticalSection* CS);
  ~TDeviceState();

  //класс параметров СОМ порта
  TComPortInitParameters* COMParameters;
  AnsiString DeviceName;

  int StateCode;
  int OldStateCode;
  //int OldState;

  int SubStateCode;

  int OutStateCode;
  int OldOutStateCode;

  //подробное состояние устройства
  int OutStateCodeEx;
  int OldOutStateCodeEx;
  //подробное описание ошибки устройства
  int OutErrorCodeEx;
  int OldOutErrorCodeEx;

//====for fiscal printers=================
  BYTE  FiscalDocumentState;
  BYTE  PrinterMode;
  BYTE  ShiftState;
  DWORD ExecCode;
  BYTE  Errors[StatusesBufferSize];
  BYTE  Statuses[StatusesBufferSize];
  int   _StatusesBufferSize;
  int   _ErrorsBufferSize;
  BYTE  ResultCodeExtention;
  int ErrorsCount;
  int StatusesCount;
  int CriticalErrorsCount;
  int CriticalStatusesCount;
//======================================

  int DSR_CTS;

//======================================


  bool         StateChange;
  double       FirmWare;         //версия прошивки
  double       ProjectNumber;    //версия проекта девайса
  std::string  StateDescription;
  std::string  SubStateDescription;

  std::string  GetOutCodeExDescription(int code);
  void SetOutCodes(int ErrorCode = DSE_NOTSET, int StateCode = DSE_NOTSET);

  BYTE        CommandCode;
  BYTE        LastError;
  BYTE        ResultCode;
  int         AnswerSize;
  TDeviceStateEnum State;
  DWORD       ID;

  //универсальный счётчик, используется для хранения суммы денег
  DWORD       Global;
  DWORD       Count;
  bool        Processing; //обработка купюры 
  bool        Billing;
  bool        Stacking;
  bool        Idle;  //рабочее/нерабочее состояние
  bool        Stacked;   //купюра застекилась
  bool        Enabling;  //рабочее состояние, ожидание команды
  double      Nominal;//номинал купюр
  bool        BillEnable;
  bool        Done; //выставляется в true когда клиент примет данные

  //////////////////////////////////////////////////////////////////////////////
  // scanner
  bool Scanner;
  std::string scannerDataValue;
  BYTE scannerDataType;
  std::string oldScannerDataValue;
  BYTE oldScannerDataType;
  //////////////////////////////////////////////////////////////////////////////
  
  std::string OutStateDescription(int code, int lang = 1);
  std::string CurrOutStateDescr();

  void SetParameters(TComPortInitParameters* COMParameters,AnsiString DeviceName,TDeviceStateEnum State = NotRun,int StateCode = 0xff,BYTE CommandCode = 0xff,BYTE LastError = 0,BYTE ResultCode = 0);
  void GetParameters(TComPortInitParameters* COMParameters,AnsiString& DeviceName,TDeviceStateEnum& State,int& StateCode,BYTE& CommandCode,BYTE& LastError,BYTE& ResultCode);
};

#endif
