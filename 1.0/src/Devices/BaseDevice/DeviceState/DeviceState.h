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
  DSE_NOERROR = 0,                             //���������� �������� ���������
  //    Critical
  DSE_OFFLINE = 1,                             //���������� �� ��������
  DSE_UNKNOWN_CODE = -2,                        //��������� �� ����������
  DSE_NOTSET = 3,                              //���������� ���

  //========Printens===============
  //    States
  PRN_FISCAL_MODE = 900,                      //���������� �����
  PRN_NONFISCAL_MODE = 901,                   //������������ �����
  //    NonCritical
  PRN_PAPER_NEAREND = 1000,                    //������ ����� ��������
  PRN_NEED_CLOSE_SHIFT = 1001,                 //���������� ������� �����
  PRN_INCORRECT_DATE_TIME = 1002,              //������ ���� �������
  PRN_NEED_OPEN_SESSION = 1003,                //���������� ������ ������
  PRN_TOO_BIG_RESULT = 1004,                   //������� ������� ���������
  PRN_NO_CASH_FOR_OPERATION = 1005,            //��� �������� ��� ��������
  PRN_NEED_SERTIFICATION = 1006,               //���������� ������������
  PRN_NEED_FISCALIZATION = 1007,               //���������� ������������
  PRN_RECIEVING_ERROR = 1008,                  //������ �����
  //    Critical
  PRN_NO_PAPER = 1100,                         //������ ���������
  PRN_PAPER_JAM = 1101,                        //������ ��������
  PRN_SOFTWARE_ERROR = 1102,                   //������ �������� ������
  PRN_HARDWARE_ERROR = 1103,                   //���� ������������
  PRN_DSR_OFF = 1104,                          //DSR �����������
  PRN_ROM_ERROR = 1105,                        //������ ���
  PRN_FISCAL_MEMORY_NOTMOUNT = 1106,           //�� �� ����������
  PRN_FISCAL_MEMORY_ERROR = 1107,              //������ ����� � ��
  PRN_CUTTER_ERROR = 1108,                     //������ ���������
  PRN_RAM_ERROR = 1109,                        //������ O��
  PRN_VOLTAGE_ERROR = 1110,                    //������ ����������
  PRN_NO_GENERATOR_SIGNAL = 1111,              //��� �������� � ��������������
  PRN_NO_SENSOR_SIGNAL = 1112,                 //��� �������� ��������
  PRN_EKLZ_ERROR = 1113,                       //������ ����
  PRN_CONTROL_MEMORY_ERROR = 1114,             //������ ����������� ������
  PRN_INCORRECT_PASSWORD = 1115,               //�������� ������ �������� ������
  PRN_PRINTER_NOT_READY = 1116,                //������� �� �����
  PRN_PRINTER_INCORRECT_STATE = 1117,          //�������� ��������� ��������
  PRN_FISCAL_MEMORY_INIT_REQUIRED = 1118,      //���������� ������������� ���������� ������
  //========Validators=============

  //    States
  VLD_BILL_ENABLE = 2001,                      //���� ������ ��������
  VLD_BILL_DISABLE = 2002,                     //���� ����� ��������
  VLD_STACKER_OPENED = 2003,                   //������ ������
  VLD_REJECTING = 2004,                        //������
  VLD_ACCEPTING = 2005,                        //�������� ������
  VLD_STACKING = 2006,                         //������� ������
  VLD_RETURNING = 2007,                        //������� ������
  VLD_HOLDING = 2008,                          //��������� ������
  VLD_BUSY = 2009,                             //�����
  VLD_PAUSED = 2010,                           //�����
  VLD_STACKED = 2011,                          //������ �������
  VLD_RETURNED = 2012,                         //������ ����������
  VLD_DISPENSING = 2013,                       //������
  VLD_UNLOADING = 2014,                        //��������
  VLD_SETTING_TYPE_CASSETTE = 2015,            //��������� �������
  VLD_DISPENSED = 2016,                        //������
  VLD_UNLOADED = 2017,                         //���������
  VLD_STACKER_CLOSED = 2018,                   //������ ������
  VLD_INITIALIZE = 2019,                       //�������������
  VLD_POWER_UP = 2020,                         //��������� �������

  //    NonCritical
  VLD_INVALID_BILL_NUMBER = 2101,              //�������� ������
  VLD_INVALID_COMMAND = 2102,                  //�������� �������
  VLD_INSERTION_ERROR = 2103,                  //������ ��������
  VLD_IDENTIFICATION_ERROR = 2104,             //������ �������������
  VLD_VERIFICATION_ERROR = 2105,               //������ ��������
  VLD_RETURN_BY_INHIBIT_ERROR = 2106,          //������� ����������� ������
  VLD_LENGTH_ERROR = 2107,                     //������ �����
  VLD_POWER_UP_WITH_BILL_IN_ACCEPTOR = 2108,   //��������� ������� � ������� � ���������
  VLD_POWER_UP_WITH_BILL_IN_STACKER = 2109,    //��������� ������� � ������� � �������
  VLD_COMMUNICATION_ERROR = 2110,              //������ �����
  VLD_HEAD_REMOVE = 2111,                      //���� ��������
  VLD_DENOMINATION_ACCESSING_ERROR = 2112,     //����������� ������
  VLD_PHOTO_PATTERN_ERROR = 2113,              //������ �������� ��������
  VLD_PHOTO_LEVEL_ERROR = 2114,                //������ �������� ���� ������
  VLD_CHECKSUM_ERROR = 2115,                   //������ ����������� �����
  VLD_BILL_REMOVE = 2116,                      //������������ ������
  VLD_BILLFISH = 2117,                         //������ BILLFISH
  VLD_ERROR_STATUS_IS_EXCLUSION = 2118,        //������ ��������� �������
  VLD_SLOW_DRIVE = 2119,                       //�������� �������� ��������
  VLD_STREAMING = 2120,                        //������� ���������
  VLD_DEPOSIT_DECLINE = 2121,                  //������ ��������
  VLD_UNKNOWN_ERROR = 2122,                    //������ ����������

  //    Critical
  VLD_STACKER_FULL = 2200,                     //������ �����
  VLD_BILL_JAM = 2202,                         //������ ������
  VLD_CHEATED = 2203,                          //������
  VLD_JAM_IN_ACCEPTOR = 2204,                  //������� ������ � ���������
  VLD_JAM_IN_STACKER = 2205,                   //������� ������ � �������
  VLD_DIELECTRIC_ERROR = 2206,                 //������ �����������
  VLD_BILL_IN_HEAD = 2207,                     //������ � ������� ���������
  VLD_COMPENSATION_FACTOR_ERROR = 2208,        //������ ������� �����������
  VLD_BILL_TRANSPORT_ERROR = 2209,             //������ ���������������
  VLD_OPTIC_SENSOR_ERROR = 2210,               //������ ����������� �������
  VLD_CAPACISTANCE_ERROR = 2211,               //������ �������
  VLD_OPERATION_ERROR = 2212,                  //������ ��������
  VLD_STACK_MOTOR_FAILURE = 2213,              //���� ������ �������
  VLD_TRANSPORT_SPEED_MOTOR_FAILURE = 2214,    //���� ������ �������� ���������������
  VLD_TRANSPORT_MOTOR_FAILURE = 2215,          //����  ������ ���������������
  VLD_ALIGNING_MOTOR_FAILURE = 2216,           //����  ������ �������
  VLD_CASSETTE_FAILURE = 2217,                 //���� �������
  VLD_OPTICAL_CANAL_FAILURE = 2218,            //���� ����������� ������
  VLD_MAGNETICAL_CANAL_FAILURE = 2219,         //���� ���������� ������
  VLD_CAPACITANCE_CANAL_FAILURE = 2220,        //���� ���������� ������
  VLD_BOOT_ROM_FAILURE = 2221,                 //���� BOOT ROM
  VLD_EXTERNAL_ROM_FAILURE = 2222,             //���� ������� ROM
  VLD_ROM_FAILURE = 2223,                      //���� ROM
  VLD_EXTERNAL_ROM_WRITE_FAILURE = 2224,       //���� ������ ������� ROM
  VLD_MAG_ERROR = 2225,                        //���� MAG
  VLD_COMPENSATION_MULTIPLYING_ERROR = 2226,   //���� COMPENSATION MULTIPLYING
  VLD_CONVEYING_ERROR = 2227,                  //���� ������� ������
  VLD_SENSOR_PROBLEM = 2228,                   //�������� � ��������
  VLD_STACKER_PROBLEM = 2229,                  //���� �������

  //========WatchDogs==============
  //    States
  WCD_LOWBOX_HIGHBOX_BOX_CLOSED = 3001,        //������ ����, ������� ����, ���� ������
  WCD_BOX_OPENED = 3002,                       //���� ������
  WCD_HIGH_BOX_OPENED = 3003,                  //������� ���� ������
  WCD_LOW_BOX_OPENED = 3004,                   //������ ���� ������
  WCD_ACTIVATOR_OPENED = 3005,                 //��������� ������
  WCD_ACTIVATOR_CLOSED = 3006,                 //��������� ������
  WCD_ACTIVATOR_ERROR = 3007,                  //������ ����������

  //    NonCritical
  WCD_BEAT = 3100,                             //����
  WCD_INCLINATION = 3101,                      //������
  WCD_BEAT_AND_INCLINATION = 3102              //���� � ������
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
  //����������� ������ ��� ������� � ������ �����, ������������ ��������� ����������
  //����� �����������, ����� ���������� ���� �����
  TCriticalSection* DeviceStateCriticalSection;
public:
  TDeviceState(TCriticalSection* CS);
  ~TDeviceState();

  //����� ���������� ��� �����
  TComPortInitParameters* COMParameters;
  AnsiString DeviceName;

  int StateCode;
  int OldStateCode;
  //int OldState;

  int SubStateCode;

  int OutStateCode;
  int OldOutStateCode;

  //��������� ��������� ����������
  int OutStateCodeEx;
  int OldOutStateCodeEx;
  //��������� �������� ������ ����������
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
  double       FirmWare;         //������ ��������
  double       ProjectNumber;    //������ ������� �������
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

  //������������� �������, ������������ ��� �������� ����� �����
  DWORD       Global;
  DWORD       Count;
  bool        Processing; //��������� ������ 
  bool        Billing;
  bool        Stacking;
  bool        Idle;  //�������/��������� ���������
  bool        Stacked;   //������ �����������
  bool        Enabling;  //������� ���������, �������� �������
  double      Nominal;//������� �����
  bool        BillEnable;
  bool        Done; //������������ � true ����� ������ ������ ������

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
