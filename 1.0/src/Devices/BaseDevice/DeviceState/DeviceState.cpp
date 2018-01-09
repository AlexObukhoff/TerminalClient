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
        result = "Не определено";
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
        result = "Не определено";
        switch(code)
        {
            case 0:
                result = "OK";
                break;
            case 1:
                result = "Устройство недоступно";
                break;
            case 2:
                result = "Нет бумаги";
                break;
            case 3:
                result = "Бумага заканчивается";
                break;
            case 4:
                result = "Бумага застряла";
                break;
            case 5:
                result = "Некритическая ошибка";
                break;
            case 6:
                result = "Стэкер открыт";
                break;
            case 7:
                result = "Стэкер полон";
                break;
            case 8:
                result = "Купюра застряла";
                break;
            case 9:
                result = "Взлом";
                break;
            case 10:
                result = "Выброс купюры";
                break;
            case 11:
                result = "Кассета установлена";
                break;
            case 12:
                result = "Аппаратная ошибка";
                break;
            case DSE_SENSOR_1_ON:
                result = "Сенсор 1 включён";
                break;
            case DSE_SENSOR_1_OFF:
                result = "Сенсор 1 выключен";
                break;
            case DSE_SENSOR_2_ON:
                result = "Сенсор 2 включён";
                break;
            case DSE_SENSOR_2_OFF:
                result = "Сенсор 2 выключен";
                break;
            case DSE_SENSOR_3_ON:
                result = "Сенсор 3 включён";
                break;
            case DSE_SENSOR_3_OFF:
                result = "Сенсор 3 выключен";
                break;
            case DSE_PRN_HEADUP:
                result = "Поднята головка принтера";
                break;
            case 255:
                result = "Неопределённое состояние";
                break;
        }
    }
    
    return result;
}

std::string TDeviceState::GetOutCodeExDescription(int code)
{
    std::string result = "Не определено";
    switch(code)
    {
        //    States
        case DSE_NOERROR:
            result = "устройство работает нормально";
            break;
        //    Critical
        case DSE_OFFLINE:
            result = "устройство не работает";
            break;
        case DSE_UNKNOWN_CODE:
            result = "состояние не определено";
            break;
        case PRN_INCORRECT_DATE_TIME:
            result = "ошибка даты времени";
            break;

        //========Printens===============
        //    States
        case PRN_FISCAL_MODE:
            result = "Фискальный режим";
            break;
        case PRN_NONFISCAL_MODE:
            result = "Нефискальный режим";
            break;
        //    NonCritical
        case PRN_NEED_CLOSE_SHIFT:
            result = "необходимо закрыть смену";
            break;
        case PRN_PAPER_NEAREND:
            result = "бумага скоро кончится";
            break;
        case PRN_NEED_OPEN_SESSION:
            result = "необходимо начало сеанса";
            break;
        case PRN_TOO_BIG_RESULT:
            result = "слишком большой результат";
            break;
        case PRN_NO_CASH_FOR_OPERATION:
            result = "нет наличных для операции";
            break;
        case PRN_NEED_SERTIFICATION:
            result = "необходима сертификация";
            break;
        case PRN_RECIEVING_ERROR:
            result = "ошибка приёма";
            break;
        //    Critical
        case PRN_NO_PAPER:
            result = "бумага кончилась";
            break;
        case PRN_PAPER_JAM:
            result = "бумага застряла";
            break;
        case PRN_SOFTWARE_ERROR:
            result = "ошибка передачи данных";
            break;
        case PRN_HARDWARE_ERROR:
            result = "сбой оборудования";
            break;
        case PRN_DSR_OFF:
            result = "DSR отсутствует";
            break;
        case PRN_ROM_ERROR:
            result = "ошибка ПЗУ";
            break;
        case PRN_FISCAL_MEMORY_NOTMOUNT:
            result = "ФП не обнаружена";
            break;
        case PRN_FISCAL_MEMORY_ERROR:
            result = "ошибка связи с ФП";
            break;
        case PRN_CUTTER_ERROR:
            result = "ошибка отрезчика";
            break;
        case PRN_RAM_ERROR:
            result = "ошибка OЗУ";
            break;
        case PRN_VOLTAGE_ERROR:
            result = "ошибка напряжения";
            break;
        case PRN_NO_GENERATOR_SIGNAL:
            result = "нет сигналов с тахогенератора";
            break;
        case PRN_NO_SENSOR_SIGNAL:
            result = "нет сигналов датчиков";
            break;
        case PRN_EKLZ_ERROR:
            result = "ошибка ЭКЛЗ";
            break;
        case PRN_CONTROL_MEMORY_ERROR:
            result = "ошибка контрольной памяти";
            break;
        case PRN_INCORRECT_PASSWORD:
            result = "неверный пароль передачи данных";
            break;
        case PRN_PRINTER_NOT_READY:
            result = "принтер не готов";
            break;
        case PRN_NEED_FISCALIZATION:
            result = "необходима фискализация";
            break;
        case PRN_PRINTER_INCORRECT_STATE:
            result = "неверное состояние принтера";
            break;
        case PRN_FISCAL_MEMORY_INIT_REQUIRED:
            result = "необходима инициализация фискальной памяти";
            break;

        //========Validators=============

        //    States
        case VLD_BILL_ENABLE:
            result = "приём купуюр разрешён";
            break;
        case VLD_BILL_DISABLE:
            result = "приём купюр запрешён";
            break;
        case VLD_STACKER_OPENED:
            result = "стэкер открыт";
            break;
        case VLD_REJECTING:
            result = "выброс";
            break;
        case VLD_ACCEPTING:
            result = "проверка купюры";
            break;
        case VLD_STACKING:
            result = "укладка купюры";
            break;
        case VLD_RETURNING:
            result = "возврат купюры";
            break;
        case VLD_HOLDING:
            result = "удержание купюры";
            break;
        case VLD_BUSY:
            result = "занят";
            break;
        case VLD_PAUSED:
            result = "пауза";
            break;
        case VLD_STACKED:
            result = "купюра уложена";
            break;
        case VLD_RETURNED:
            result = "купюра возвращена";
            break;
        case VLD_DISPENSING:
            result = "выдача";
            break;
        case VLD_UNLOADING:
            result = "выгрузка";
            break;
        case VLD_SETTING_TYPE_CASSETTE:
            result = "установка кассеты";
            break;
        case VLD_DISPENSED:
            result = "выдано";
            break;
        case VLD_UNLOADED:
            result = "выгружено";
            break;
        case VLD_STACKER_CLOSED:
            result = "стэкер закрыт";
            break;
        case VLD_INITIALIZE:
            result = "инициализация";
            break;
        case VLD_POWER_UP:
            result = "включение питания";
            break;

        //    NonCritical
        case VLD_INVALID_BILL_NUMBER:
            result = "неверная купюра";
            break;
        case VLD_INVALID_COMMAND:
            result = "неверная команда";
            break;
        case VLD_INSERTION_ERROR:
            result = "ошибка загрузки";
            break;
        case VLD_IDENTIFICATION_ERROR:
            result = "ошибка идентификации";
            break;
        case VLD_VERIFICATION_ERROR:
            result = "ошибка проверки";
            break;
        case VLD_RETURN_BY_INHIBIT_ERROR:
            result = "возврат запрещённой купюры";
            break;
        case VLD_LENGTH_ERROR:
            result = "ошибка длины";
            break;
        case VLD_POWER_UP_WITH_BILL_IN_ACCEPTOR:
            result = "включение питания с купюрой в акцепторе";
            break;
        case VLD_POWER_UP_WITH_BILL_IN_STACKER:
            result = "включение питания с купюрой в стэкере";
            break;
        case VLD_COMMUNICATION_ERROR:
            result = "ошибка связи";
            break;
        case VLD_HEAD_REMOVE:
            result = "снят акцептор";
            break;
        case VLD_DENOMINATION_ACCESSING_ERROR:
            result = "запрещённая купюра";
            break;
        case VLD_PHOTO_PATTERN_ERROR:
            result = "ошибка проверки паттерна";
            break;
        case VLD_PHOTO_LEVEL_ERROR:
            result = "ошибка проверки фото уровня";
            break;
        case VLD_CHECKSUM_ERROR:
            result = "ошибка контрольной суммы";
            break;
        case VLD_BILL_REMOVE:
            result = "вытаскивание купюры";
            break;
        case VLD_BILLFISH:
            result = "ошибка BILLFISH";
            break;
        case VLD_ERROR_STATUS_IS_EXCLUSION:
            result = "ошибка неверного статуса";
            break;
        case VLD_SLOW_DRIVE:
            result = "механизм работает медленно";
            break;
        case VLD_STREAMING:
            result = "попытка стриминга";
            break;
        case VLD_DEPOSIT_DECLINE:
            result = "запрет депозита";
            break;
        case VLD_UNKNOWN_ERROR:
            result = "ошибка неизвестна";
            break;

        //    Critical
        case VLD_STACKER_FULL:
            result = "стекер забит";
            break;
        case VLD_BILL_JAM:
            result = "купюра замята";
            break;
        case VLD_CHEATED:
            result = "читинг";
            break;
        case VLD_JAM_IN_ACCEPTOR:
            result = "замятие купюры в акцепторе";
            break;
        case VLD_JAM_IN_STACKER:
            result = "замятие купюры в стекере";
            break;
        case VLD_DIELECTRIC_ERROR:
            result = "ошибка диэлектрики";
            break;
        case VLD_BILL_IN_HEAD:
            result = "купюра в приёмном механизме";
            break;
        case VLD_COMPENSATION_FACTOR_ERROR:
            result = "ошибка фактора компенсации";
            break;
        case VLD_BILL_TRANSPORT_ERROR:
            result = "ошибка транспортировки";
            break;
        case VLD_OPTIC_SENSOR_ERROR:
            result = "ошибка оптического сенсора";
            break;
        case VLD_CAPACISTANCE_ERROR:
            result = "ошибка ёмкости";
            break;
        case VLD_OPERATION_ERROR:
            result = "ошибка операции";
            break;
        case VLD_STACK_MOTOR_FAILURE:
            result = "сбой мотора стекера";
            break;
        case VLD_TRANSPORT_SPEED_MOTOR_FAILURE:
            result = "сбой мотора скорости транспортировки";
            break;
        case VLD_TRANSPORT_MOTOR_FAILURE:
            result = "сбой  мотора транспортировки";
            break;
        case VLD_ALIGNING_MOTOR_FAILURE:
            result = "сбой  мотора укладки";
            break;
        case VLD_CASSETTE_FAILURE:
            result = "сбой кассеты";
            break;
        case VLD_OPTICAL_CANAL_FAILURE:
            result = "сбой оптического канала";
            break;
        case VLD_MAGNETICAL_CANAL_FAILURE:
            result = "сбой магнитного канала";
            break;
        case VLD_CAPACITANCE_CANAL_FAILURE:
            result = "сбой ёмкостного канала";
            break;
        case VLD_BOOT_ROM_FAILURE:
            result = "сбой BOOT ROM";
            break;
        case VLD_EXTERNAL_ROM_FAILURE:
            result = "сбой внешней ROM";
            break;
        case VLD_ROM_FAILURE:
            result = "сбой ROM";
            break;
        case VLD_EXTERNAL_ROM_WRITE_FAILURE:
            result = "сбой записи внешней ROM";
            break;
        case VLD_MAG_ERROR:
            result = "сбой MAG";
            break;
        case VLD_COMPENSATION_MULTIPLYING_ERROR:
            result = "сбой COMPENSATION MULTIPLYING";
            break;
        case VLD_CONVEYING_ERROR:
            result = "сбой укладки купюры";
            break;
        case VLD_SENSOR_PROBLEM:
            result = "проблема с сенсором";
            break;
        case VLD_STACKER_PROBLEM:
            result = "сбой стэкера";
            break;

        //========WatchDogs==============
        //    States
        case WCD_LOWBOX_HIGHBOX_BOX_CLOSED:
            result = "Нижний бокс, верхний бокс, сейф закрыт";
            break;
        case WCD_BOX_OPENED:
            result = "сейф открыт";
            break;
        case WCD_HIGH_BOX_OPENED:
            result = "верхний бокс открыт";
            break;
        case WCD_LOW_BOX_OPENED:
            result = "нижний бокс открыт";
            break;
        case WCD_ACTIVATOR_OPENED:
            result = "активатор открыт";
            break;
        case WCD_ACTIVATOR_CLOSED:
            result = "активатор закрыт";
            break;
        case WCD_ACTIVATOR_ERROR:
            result = "ошибка активатора";
            break;

        //    NonCritical
        case WCD_BEAT:
            result = "Удар";
            break;
        case WCD_INCLINATION:
            result = "Наклон";
            break;
        case WCD_BEAT_AND_INCLINATION:
            result = "Удар и наклон";
            break;

        default:
            result = (boost::format("Неизвестная ошибка с кодом %1%") % code).str();
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

