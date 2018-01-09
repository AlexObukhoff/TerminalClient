#pragma hdrstop

#include "DeviceState.h"
#include "boost/format.hpp"
#pragma package(smart_init)

TDeviceState::TDeviceState(TCriticalSection* CS)
{
  _StatusesBufferSize = StatusesBufferSize;
  _ErrorsBufferSize = StatusesBufferSize;

  StateCode = 0;
  SubStateCode = 0;
  OldStateCode = 0;

  OutStateCode = DSE_UNKNOWN;
  OldOutStateCode = DSE_UNKNOWN;
  StateDescription = "";
  SubStateDescription = "";

  OutStateCodeEx = DSE_UNKNOWN_CODE;
  OldOutStateCodeEx = DSE_UNKNOWN_CODE;
  OutErrorCodeEx = DSE_UNKNOWN_CODE;
  OldOutErrorCodeEx = DSE_UNKNOWN_CODE;

  DeviceStateCriticalSection = CS;
  Global = 0;
  Count = 0;
  StateChange = false;
  Processing = false;
  Billing = false;
  Stacking = false;
  Stacked = false;
  BillEnable = false;
  Idle = true;
  DSR_CTS = 0;
  memset(Errors,0,StatusesBufferSize);
  memset(Statuses,0,StatusesBufferSize);
  ErrorsCount = CriticalErrorsCount = 0;
  StatusesCount = CriticalStatusesCount = 0;
  ID = 0;

  scannerDataValue = "";
  oldScannerDataValue = "";
  scannerDataType = 0;
  oldScannerDataType = 0;
}

TDeviceState::~TDeviceState()
{
}

void TDeviceState::SetParameters(TComPortInitParameters* COMParameters,AnsiString DeviceName,TDeviceStateEnum State,int StateCode,BYTE CommandCode,BYTE LastError,BYTE ResultCode)
{
  if (DeviceStateCriticalSection == NULL) return;
  DeviceStateCriticalSection->Acquire();
  try
  {
    this->COMParameters = COMParameters;
    this->DeviceName = DeviceName;
    this->State = State;
    this->StateCode = StateCode;
    this->CommandCode = CommandCode;
    this->LastError = LastError;
    this->ResultCode = ResultCode;
    SubStateCode = 0;
  }
  __finally
  {
    DeviceStateCriticalSection->Release();
  }
}

void TDeviceState::GetParameters(TComPortInitParameters* COMParameters,AnsiString& DeviceName,TDeviceStateEnum& State,int& StateCode,BYTE& CommandCode,BYTE& LastError,BYTE& ResultCode)
{
    UNREFERENCED_PARAMETER(COMParameters);
  if (DeviceStateCriticalSection == NULL) return;
  DeviceStateCriticalSection->Acquire();
  try
  {
    COMParameters = this->COMParameters;
    DeviceName = this->DeviceName;
    State = this->State;
    StateCode = this->StateCode;
    CommandCode = this->CommandCode;
    LastError = this->LastError;
    ResultCode = this->ResultCode;
  }
  __finally
  {
    DeviceStateCriticalSection->Release();
  }
}

std::string TDeviceState::CurrOutStateDescr()
{
    return OutStateDescription(OutStateCode);
}

std::string TDeviceState::OutStateDescription(int code, int lang)
{
    std::string result = "";
    
    if (lang == 0)
    {
        result = "�� ����������";
        switch(code)
        {
            case DSE_OK:
                result = "DSE_OK";
                break;

            case DSE_NOTMOUNT:
                result = "DSE_NOTMOUNT";
                break;

            case DSE_NOTPAPER:
                result = "DSE_NOTPAPER";
                break;

            case DSE_NEARENDPAPER:
                result = "DSE_NEARENDPAPER";
                break;

            case DSE_PAPERJAM:
                result = "DSE_PAPERJAM";
                break;

            case DSE_STACKEROPEN:
                result = "DSE_STACKEROPEN";
                break;

            case DSE_STACKERFULL:
                result = "DSE_STACKERFULL";
                break;

            case DSE_BILLJAM:
                result = "DSE_BILLJAM";
                break;

            case DSE_CHEATED:
                result = "DSE_CHEATED";
                break;

            case DSE_SETCASSETTE:
                result = "DSE_SETCASSETTE";
                break;

            case DSE_HARDWARE_ERROR:
                result = "DSE_HARDWARE_ERROR";
                break;
                 
            case DSE_BILLREJECT:
                result = "DSE_BILLREJECT";
                break;

            case DSE_UNKNOWN:
                result = "DSE_UNKNOWN";
                break;

            case DSE_SENSOR_1_ON:
                result = "DSE_SENSOR_1_ON";
                break;

            case DSE_SENSOR_1_OFF:
                result = "DSE_SENSOR_1_OFF";
                break;

            case DSE_SENSOR_2_ON:
                result = "DSE_SENSOR_2_ON";
                break;

            case DSE_SENSOR_2_OFF:
                result = "DSE_SENSOR_2_OFF";
                break;

            case DSE_SENSOR_3_ON:
                result = "DSE_SENSOR_3_ON";
                break;

            case DSE_SENSOR_3_OFF:
                result = "DSE_SENSOR_3_OFF";
                break;
            case DSE_PRN_HEADUP:
                result = "DSE_PRN_HEADUP";
                break;
        }
    }

    if (lang == 1)
    {
        result = "�� ����������";
        switch(code)
        {
            case 0:
                result = "OK";
                break;
            case 1:
                result = "���������� ����������";
                break;
            case 2:
                result = "��� ������";
                break;
            case 3:
                result = "������ �������������";
                break;
            case 4:
                result = "������ ��������";
                break;
            case 5:
                result = "������������� ������";
                break;
            case 6:
                result = "������ ������";
                break;
            case 7:
                result = "������ �����";
                break;
            case 8:
                result = "������ ��������";
                break;
            case 9:
                result = "�����";
                break;
            case 10:
                result = "������ ������";
                break;
            case 11:
                result = "������� �����������";
                break;
            case 12:
                result = "���������� ������";
                break;
            case DSE_SENSOR_1_ON:
                result = "������ 1 �������";
                break;
            case DSE_SENSOR_1_OFF:
                result = "������ 1 ��������";
                break;
            case DSE_SENSOR_2_ON:
                result = "������ 2 �������";
                break;
            case DSE_SENSOR_2_OFF:
                result = "������ 2 ��������";
                break;
            case DSE_SENSOR_3_ON:
                result = "������ 3 �������";
                break;
            case DSE_SENSOR_3_OFF:
                result = "������ 3 ��������";
                break;
            case DSE_PRN_HEADUP:
                result = "������� ������� ��������";
                break;
            case 255:
                result = "������������� ���������";
                break;
        }
    }
    
    return result;
}

std::string TDeviceState::GetOutCodeExDescription(int code)
{
    std::string result = "�� ����������";
    switch(code)
    {
        //    States
        case DSE_NOERROR:
            result = "���������� �������� ���������";
            break;
        //    Critical
        case DSE_OFFLINE:
            result = "���������� �� ��������";
            break;
        case DSE_UNKNOWN_CODE:
            result = "��������� �� ����������";
            break;
        case PRN_INCORRECT_DATE_TIME:
            result = "������ ���� �������";
            break;

        //========Printens===============
        //    States
        case PRN_FISCAL_MODE:
            result = "���������� �����";
            break;
        case PRN_NONFISCAL_MODE:
            result = "������������ �����";
            break;
        //    NonCritical
        case PRN_NEED_CLOSE_SHIFT:
            result = "���������� ������� �����";
            break;
        case PRN_PAPER_NEAREND:
            result = "������ ����� ��������";
            break;
        case PRN_NEED_OPEN_SESSION:
            result = "���������� ������ ������";
            break;
        case PRN_TOO_BIG_RESULT:
            result = "������� ������� ���������";
            break;
        case PRN_NO_CASH_FOR_OPERATION:
            result = "��� �������� ��� ��������";
            break;
        case PRN_NEED_SERTIFICATION:
            result = "���������� ������������";
            break;
        case PRN_RECIEVING_ERROR:
            result = "������ �����";
            break;
        //    Critical
        case PRN_NO_PAPER:
            result = "������ ���������";
            break;
        case PRN_PAPER_JAM:
            result = "������ ��������";
            break;
        case PRN_SOFTWARE_ERROR:
            result = "������ �������� ������";
            break;
        case PRN_HARDWARE_ERROR:
            result = "���� ������������";
            break;
        case PRN_DSR_OFF:
            result = "DSR �����������";
            break;
        case PRN_ROM_ERROR:
            result = "������ ���";
            break;
        case PRN_FISCAL_MEMORY_NOTMOUNT:
            result = "�� �� ����������";
            break;
        case PRN_FISCAL_MEMORY_ERROR:
            result = "������ ����� � ��";
            break;
        case PRN_CUTTER_ERROR:
            result = "������ ���������";
            break;
        case PRN_RAM_ERROR:
            result = "������ O��";
            break;
        case PRN_VOLTAGE_ERROR:
            result = "������ ����������";
            break;
        case PRN_NO_GENERATOR_SIGNAL:
            result = "��� �������� � ��������������";
            break;
        case PRN_NO_SENSOR_SIGNAL:
            result = "��� �������� ��������";
            break;
        case PRN_EKLZ_ERROR:
            result = "������ ����";
            break;
        case PRN_CONTROL_MEMORY_ERROR:
            result = "������ ����������� ������";
            break;
        case PRN_INCORRECT_PASSWORD:
            result = "�������� ������ �������� ������";
            break;
        case PRN_PRINTER_NOT_READY:
            result = "������� �� �����";
            break;
        case PRN_NEED_FISCALIZATION:
            result = "���������� ������������";
            break;
        case PRN_PRINTER_INCORRECT_STATE:
            result = "�������� ��������� ��������";
            break;
        case PRN_FISCAL_MEMORY_INIT_REQUIRED:
            result = "���������� ������������� ���������� ������";
            break;

        //========Validators=============

        //    States
        case VLD_BILL_ENABLE:
            result = "���� ������ ��������";
            break;
        case VLD_BILL_DISABLE:
            result = "���� ����� ��������";
            break;
        case VLD_STACKER_OPENED:
            result = "������ ������";
            break;
        case VLD_REJECTING:
            result = "������";
            break;
        case VLD_ACCEPTING:
            result = "�������� ������";
            break;
        case VLD_STACKING:
            result = "������� ������";
            break;
        case VLD_RETURNING:
            result = "������� ������";
            break;
        case VLD_HOLDING:
            result = "��������� ������";
            break;
        case VLD_BUSY:
            result = "�����";
            break;
        case VLD_PAUSED:
            result = "�����";
            break;
        case VLD_STACKED:
            result = "������ �������";
            break;
        case VLD_RETURNED:
            result = "������ ����������";
            break;
        case VLD_DISPENSING:
            result = "������";
            break;
        case VLD_UNLOADING:
            result = "��������";
            break;
        case VLD_SETTING_TYPE_CASSETTE:
            result = "��������� �������";
            break;
        case VLD_DISPENSED:
            result = "������";
            break;
        case VLD_UNLOADED:
            result = "���������";
            break;
        case VLD_STACKER_CLOSED:
            result = "������ ������";
            break;
        case VLD_INITIALIZE:
            result = "�������������";
            break;
        case VLD_POWER_UP:
            result = "��������� �������";
            break;

        //    NonCritical
        case VLD_INVALID_BILL_NUMBER:
            result = "�������� ������";
            break;
        case VLD_INVALID_COMMAND:
            result = "�������� �������";
            break;
        case VLD_INSERTION_ERROR:
            result = "������ ��������";
            break;
        case VLD_IDENTIFICATION_ERROR:
            result = "������ �������������";
            break;
        case VLD_VERIFICATION_ERROR:
            result = "������ ��������";
            break;
        case VLD_RETURN_BY_INHIBIT_ERROR:
            result = "������� ����������� ������";
            break;
        case VLD_LENGTH_ERROR:
            result = "������ �����";
            break;
        case VLD_POWER_UP_WITH_BILL_IN_ACCEPTOR:
            result = "��������� ������� � ������� � ���������";
            break;
        case VLD_POWER_UP_WITH_BILL_IN_STACKER:
            result = "��������� ������� � ������� � �������";
            break;
        case VLD_COMMUNICATION_ERROR:
            result = "������ �����";
            break;
        case VLD_HEAD_REMOVE:
            result = "���� ��������";
            break;
        case VLD_DENOMINATION_ACCESSING_ERROR:
            result = "����������� ������";
            break;
        case VLD_PHOTO_PATTERN_ERROR:
            result = "������ �������� ��������";
            break;
        case VLD_PHOTO_LEVEL_ERROR:
            result = "������ �������� ���� ������";
            break;
        case VLD_CHECKSUM_ERROR:
            result = "������ ����������� �����";
            break;
        case VLD_BILL_REMOVE:
            result = "������������ ������";
            break;
        case VLD_BILLFISH:
            result = "������ BILLFISH";
            break;
        case VLD_ERROR_STATUS_IS_EXCLUSION:
            result = "������ ��������� �������";
            break;
        case VLD_SLOW_DRIVE:
            result = "�������� �������� ��������";
            break;
        case VLD_STREAMING:
            result = "������� ���������";
            break;
        case VLD_DEPOSIT_DECLINE:
            result = "������ ��������";
            break;
        case VLD_UNKNOWN_ERROR:
            result = "������ ����������";
            break;

        //    Critical
        case VLD_STACKER_FULL:
            result = "������ �����";
            break;
        case VLD_BILL_JAM:
            result = "������ ������";
            break;
        case VLD_CHEATED:
            result = "������";
            break;
        case VLD_JAM_IN_ACCEPTOR:
            result = "������� ������ � ���������";
            break;
        case VLD_JAM_IN_STACKER:
            result = "������� ������ � �������";
            break;
        case VLD_DIELECTRIC_ERROR:
            result = "������ �����������";
            break;
        case VLD_BILL_IN_HEAD:
            result = "������ � ������� ���������";
            break;
        case VLD_COMPENSATION_FACTOR_ERROR:
            result = "������ ������� �����������";
            break;
        case VLD_BILL_TRANSPORT_ERROR:
            result = "������ ���������������";
            break;
        case VLD_OPTIC_SENSOR_ERROR:
            result = "������ ����������� �������";
            break;
        case VLD_CAPACISTANCE_ERROR:
            result = "������ �������";
            break;
        case VLD_OPERATION_ERROR:
            result = "������ ��������";
            break;
        case VLD_STACK_MOTOR_FAILURE:
            result = "���� ������ �������";
            break;
        case VLD_TRANSPORT_SPEED_MOTOR_FAILURE:
            result = "���� ������ �������� ���������������";
            break;
        case VLD_TRANSPORT_MOTOR_FAILURE:
            result = "����  ������ ���������������";
            break;
        case VLD_ALIGNING_MOTOR_FAILURE:
            result = "����  ������ �������";
            break;
        case VLD_CASSETTE_FAILURE:
            result = "���� �������";
            break;
        case VLD_OPTICAL_CANAL_FAILURE:
            result = "���� ����������� ������";
            break;
        case VLD_MAGNETICAL_CANAL_FAILURE:
            result = "���� ���������� ������";
            break;
        case VLD_CAPACITANCE_CANAL_FAILURE:
            result = "���� ���������� ������";
            break;
        case VLD_BOOT_ROM_FAILURE:
            result = "���� BOOT ROM";
            break;
        case VLD_EXTERNAL_ROM_FAILURE:
            result = "���� ������� ROM";
            break;
        case VLD_ROM_FAILURE:
            result = "���� ROM";
            break;
        case VLD_EXTERNAL_ROM_WRITE_FAILURE:
            result = "���� ������ ������� ROM";
            break;
        case VLD_MAG_ERROR:
            result = "���� MAG";
            break;
        case VLD_COMPENSATION_MULTIPLYING_ERROR:
            result = "���� COMPENSATION MULTIPLYING";
            break;
        case VLD_CONVEYING_ERROR:
            result = "���� ������� ������";
            break;
        case VLD_SENSOR_PROBLEM:
            result = "�������� � ��������";
            break;
        case VLD_STACKER_PROBLEM:
            result = "���� �������";
            break;

        //========WatchDogs==============
        //    States
        case WCD_LOWBOX_HIGHBOX_BOX_CLOSED:
            result = "������ ����, ������� ����, ���� ������";
            break;
        case WCD_BOX_OPENED:
            result = "���� ������";
            break;
        case WCD_HIGH_BOX_OPENED:
            result = "������� ���� ������";
            break;
        case WCD_LOW_BOX_OPENED:
            result = "������ ���� ������";
            break;
        case WCD_ACTIVATOR_OPENED:
            result = "��������� ������";
            break;
        case WCD_ACTIVATOR_CLOSED:
            result = "��������� ������";
            break;
        case WCD_ACTIVATOR_ERROR:
            result = "������ ����������";
            break;

        //    NonCritical
        case WCD_BEAT:
            result = "����";
            break;
        case WCD_INCLINATION:
            result = "������";
            break;
        case WCD_BEAT_AND_INCLINATION:
            result = "���� � ������";
            break;

        default:
            result = (boost::format("����������� ������ � ����� %1%") % code).str();
            break;
    }
    return result;
}

void TDeviceState::SetOutCodes(int ErrorCode, int StateCode)
{
    if (StateCode != DSE_NOTSET)
    {
        OldOutStateCodeEx = OutStateCodeEx;
        OutStateCodeEx = StateCode;
    }
    if (ErrorCode != DSE_NOTSET)
    {
        OldOutErrorCodeEx = OutErrorCodeEx;
        OutErrorCodeEx = ErrorCode;
    }
}

