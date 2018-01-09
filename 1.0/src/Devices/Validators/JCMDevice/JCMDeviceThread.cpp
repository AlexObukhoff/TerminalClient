//---------------------------------------------------------------------------


#pragma hdrstop

#include <math.h>
#include "JCMDeviceThread.h"
#include "CValidator.h"
#include "globals.h"
#include "boost/format.hpp"

//---------------------------------------------------------------------------

#pragma package(smart_init)

#define POLYNOMINAL 0x08408
#define PollingInterval 100

__fastcall TJCMDeviceThread::TJCMDeviceThread() : TDeviceThread(true)
{
  ResetCount = 1;
}

__fastcall TJCMDeviceThread::~TJCMDeviceThread()
{
}

unsigned short TJCMDeviceThread::Calc_CRC16CCNET(BYTE* DataBuf, unsigned short  BufLen)
{
   unsigned short i;
   inter_crc_h = inter_crc_l = 0;
   for (i=0;i<BufLen;i++) calc_crc(DataBuf[i]);
   i=inter_crc_h;
   i<<=8;
   i+=inter_crc_l;
   return i;
}

void TJCMDeviceThread::calc_crc(BYTE byte)
{
   unsigned char i,c;
   unsigned short temp_crc;
   inter_crc_h ^= byte;
   temp_crc = inter_crc_l;
   temp_crc <<= 8;
   temp_crc |= inter_crc_h;
   for (i=0;i<8;i++)
   {
      c = (unsigned char)(temp_crc & 0x01);
      temp_crc >>= 1;
      if (c) temp_crc ^= POLYNOMINAL;
   }
   inter_crc_l = (unsigned char)(temp_crc >> 8);
   inter_crc_h = temp_crc;
}

void TJCMDeviceThread::SendPacket(BYTE command,int len_packet, BYTE* data)
{
  if (!Port->PortInit)
  {
    DeviceState->StateCode = 0xff;
    return;
  }
  try
  {
    //здесь в буфер пишем команду для выполнения
    ClearCommand();
    ClearAnswer();

    Command[0] = 0xFC;
    int n_sm = 3;

    Command[1] = (BYTE)len_packet;
    Command[2] = (BYTE)command;
    if (data!=NULL)
      memcpy(&Command[n_sm],data,len_packet-n_sm-2);

    Calc_CRC16CCNET(Command,(unsigned short)(len_packet-2));
    Command[len_packet-2] = inter_crc_h;
    Command[len_packet-1] = inter_crc_l;

    CommandParameters->SetParameters(Command,len_packet,command,0);
    CommandSize = len_packet;
  }
  __finally
  {
  }
}

void TJCMDeviceThread::SendACK()
{
  if (!Port->PortInit)
  {
    DeviceState->StateCode = 0xff;
    return;
  }

  Command[0] = 0xFC;
  Command[1] = 0x05;
  Command[2] = 0x50;
  Command[3] = 0xAA;
  Command[4] = 0x05;

  CommandSize = 5;
  CommandParameters->SetParameters(Command,5,Command[2],0);

  TSendType old = SendType;
  SendType = NotRecieveAnswer;
  SendPacket(Command[2],CommandSize,NULL);
  ProcessCommand();
  SendType = old;
}

void TJCMDeviceThread::GetStay()//POLL command
{
  try
  {
    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0xff;
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      return;
    }
    Command[0] = 0xFC;
    Command[1] = 0x05;
    Command[2] = 0x11;
    Command[3] = 0x27;
    Command[4] = 0x56;
    CommandSize = 5;
    CommandParameters->SetParameters(Command,5,0x11,0);

    ProcessCommand();

    if (DeviceState->AnswerSize > 0)
        OfflineCount = 0;
    else
    {
        OfflineCount++;
        if (OfflineCount >= 50)
        {
            OfflineCount = 50;
            if (DeviceState->OutStateCode != DSE_NOTMOUNT)
            {
                DeviceState->OutStateCode = DSE_NOTMOUNT;
                ChangeDeviceState();
                return;
            }
        }
    }
    //new 15-03-2007
    if (Terminated) return;
    //SendACK();
  }
  __finally
  {
  }
}

void TJCMDeviceThread::Reset()
{
  try
  {
    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0xff;
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      return;
    }
    BYTE command=0x40;
    SendPacket(command,5,NULL);
    ProcessCommand();
    if (Terminated) return;

    if (DeviceState->StateCode == 0xff)
       return;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TJCMDeviceThread::Stack1()
{
  try
  {
    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0xff;
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      return;
    }
    BYTE command=0x41;
    SendPacket(command,5,NULL);
    ProcessCommand();
    if (Terminated) return;

    if (DeviceState->StateCode == 0xff)
       return;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TJCMDeviceThread::Stack2()
{
  try
  {
    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0xff;
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      return;
    }
    BYTE command=0x42;
    SendPacket(command,5,NULL);
    ProcessCommand();
    if (Terminated) return;

    if (DeviceState->StateCode == 0xff)
       return;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TJCMDeviceThread::Return()
{
  try
  {
    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0xff;
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      return;
    }
    BYTE command=0x43;
    SendPacket(command,5,NULL);
    ProcessCommand();
    if (Terminated) return;

    if (DeviceState->StateCode == 0xff)
       return;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TJCMDeviceThread::Hold()
{
  try
  {
    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0xff;
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      return;
    }
    BYTE command=0x44;
    SendPacket(command,5,NULL);
    ProcessCommand();
    if (Terminated) return;

    if (DeviceState->StateCode == 0xff)
       return;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

int  TJCMDeviceThread::Stack()
{
    Stack1();
    if (DeviceState->StateCode == 0xff)
      return 0;
    if (Terminated) return 0;
    GetStay();
    return 0;
}


void TJCMDeviceThread::Wait()
{
  try
  {
    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0xff;
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      return;
    }
    BYTE command=0x45;
    SendPacket(command,5,NULL);
    ProcessCommand();
    if (Terminated) return;

    if (DeviceState->StateCode == 0xff)
       return;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

std::string TJCMDeviceThread::GetStatusDescription(BYTE StatusCode)
{
    switch(StatusCode)
    {
        case 0x11:
            //DeviceState->OutStateCode = DSE_OK;
            //DeviceState->StateDescription = "ENABLE(IDLING)";
            break;
        case 0x12:
            //DeviceState->OutStateCode = DSE_OK;
            //DeviceState->StateDescription = "ACCEPTING";
            break;
        case 0x13:
            //DeviceState->OutStateCode = DSE_OK;
            //DeviceState->StateDescription = "ESCROW";
            break;
        case 0x14:
            DeviceState->OutStateCode = DSE_OK;
            //DeviceState->StateDescription = "STACKING";
            break;
        case 0x15:
            DeviceState->OutStateCode = DSE_OK;
            //DeviceState->StateDescription = "VEND VALID";
            break;
        case 0x16:
            DeviceState->OutStateCode = DSE_OK;
            //DeviceState->StateDescription = "STACKED";
            break;
        case 0x17:
            //DeviceState->OutStateCode = DSE_BILLREJECT;
            //DeviceState->StateDescription = "REJECTING";
            break;
        case 0x18:
            //DeviceState->OutStateCode = DSE_OK;
            //DeviceState->StateDescription = "RETURNING";
            break;
        case 0x19:
            //DeviceState->OutStateCode = DSE_OK;
            //DeviceState->StateDescription = "HOLDING";
            break;
        case 0x1A:
            //DeviceState->OutStateCode = DSE_OK;
            //DeviceState->StateDescription = "DISABLE(INHIBIT)";
            break;
        case 0x1B:
            //DeviceState->OutStateCode = DSE_OK;
            //DeviceState->StateDescription = "INITIALIZE";
            break;
        case 0x40:
            //DeviceState->OutStateCode = DSE_OK;
            //DeviceState->StateDescription = "POWER UP";
            break;
        case 0x41:
            //DeviceState->OutStateCode = DSE_OK;
            //DeviceState->StateDescription = "POWER UP WITH BILL IN ACCEPTOR";
            break;
        case 0x42:
            //DeviceState->OutStateCode = DSE_OK;
            //DeviceState->StateDescription = "POWER UP WITH BILL IN STACKER";
            break;
        case 0x43:
            DeviceState->OutStateCode = DSE_STACKERFULL;
            //DeviceState->StateDescription = "STACKER FULL";
            break;
        case 0x44:
            DeviceState->OutStateCode = DSE_STACKEROPEN;
            //DeviceState->StateDescription = "STACKER OPEN";
            break;
        case 0x45:
            //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
            DeviceState->OutStateCode = DSE_BILLJAM;
            //DeviceState->StateDescription = "JAM IN ACCEPTOR";
            break;
        case 0x46:
            //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
            DeviceState->OutStateCode = DSE_BILLJAM;
            //DeviceState->StateDescription = "JAM IN STACKER";
            break;
        case 0x47:
            //DeviceState->OutStateCode = DSE_OK;
            //DeviceState->StateDescription = "PAUSE";
            break;
        case 0x48:
            //DeviceState->OutStateCode = DSE_CHEATED;
            //DeviceState->StateDescription = "CHEATED";
            break;
        case 0x49:
            DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
            //DeviceState->StateDescription = "FAILURE";
            break;
        case 0x4A:
            DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
            //DeviceState->StateDescription = "COMMUNICATION ERROR";
            break;
        case 0xC4:
        case 0xC3:
        case 0xC1:
            //DeviceState->OutStateCode = DSE_OK;
            break;
        default :
            BYTE data[1];
            data[0] = StatusCode;
            Log->WriteBuffer(data,1);
            //DeviceState->StateDescription = "UNKNOWN STATUS CODE";
            break;
    }
    //AnsiString result = "";
    if (lang == 0)
    switch(StatusCode)
    {
        case 0x00:
            DeviceState->StateDescription = "NULL";
            break;
        case 0x11:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "ENABLE(IDLING)";
            break;
        case 0x12:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "ACCEPTING";
            break;
        case 0x13:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "ESCROW";
            break;
        case 0x14:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "STACKING";
            break;
        case 0x15:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "VEND VALID";
            break;
        case 0x16:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "STACKED";
            break;
        case 0x17:
            //DeviceState->OutStateCode = DSE_BILLREJECT;
            DeviceState->StateDescription = "REJECTING";
            break;
        case 0x18:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "RETURNING";
            break;
        case 0x19:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "HOLDING";
            break;
        case 0x1A:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "DISABLE(INHIBIT)";
            break;
        case 0x1B:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "INITIALIZE";
            break;
        case 0x40:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "POWER UP";
            break;
        case 0x41:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "POWER UP WITH BILL IN ACCEPTOR";
            break;
        case 0x42:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "POWER UP WITH BILL IN STACKER";
            break;
        case 0x43:
            //DeviceState->OutStateCode = DSE_STACKERFULL;
            DeviceState->StateDescription = "STACKER FULL";
            break;
        case 0x44:
            //DeviceState->OutStateCode = DSE_STACKEROPEN;
            DeviceState->StateDescription = "STACKER OPEN";
            break;
        case 0x45:
            //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
            DeviceState->StateDescription = "JAM IN ACCEPTOR";
            break;
        case 0x46:
            DeviceState->StateDescription = "JAM IN STACKER";
            //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
            break;
        case 0x47:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "PAUSE";
            break;
        case 0x48:
            //DeviceState->OutStateCode = DSE_MAINERROR;
            DeviceState->StateDescription = "CHEATED";
            break;
        case 0x49:
            //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
            DeviceState->StateDescription = "FAILURE";
            break;
        case 0x4A:
            //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
            DeviceState->StateDescription = "COMMUNICATION ERROR";
            break;
        case 0xC4:
        case 0xC3:
            DeviceState->StateDescription = "Enable С3 С4";
            break;
        case 0xC1:
            DeviceState->StateDescription = "Security was setted";
            break;
        default :
            BYTE data[1];
            data[0] = StatusCode;
            Log->WriteBuffer(data,1);
            DeviceState->StateDescription = "UNKNOWN STATUS CODE";
            break;
    }
    if (lang == 1)
    switch(StatusCode)
    {
        case 0x00:
            DeviceState->StateDescription = "NULL";
            break;
        case 0x11:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "Доступен";
            break;
        case 0x12:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "Разрешение";
            break;
        case 0x13:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "Обработка купюры";
            break;
        case 0x14:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "Транспортировка купюры";
            break;
        case 0x15:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "Проверка прошла";
            break;
        case 0x16:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "Купюра принята";
            break;
        case 0x17:
            //DeviceState->OutStateCode = DSE_BILLREJECT;
            DeviceState->StateDescription = "Выброс купюры";
            break;
        case 0x18:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "Возврат";
            break;
        case 0x19:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "Удержание";
            break;
        case 0x1A:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "Недоступен";
            break;
        case 0x1B:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "Инициализация";
            break;
        case 0x40:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "Включение питания";
            break;
        case 0x41:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "Включение питания с купюрой в акцепторе";
            break;
        case 0x42:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "Включение питания с купюрой в стэкере";
            break;
        case 0x43:
            //DeviceState->OutStateCode = DSE_STACKERFULL;
            DeviceState->StateDescription = "Кассета переполнена";
            break;
        case 0x44:
            DeviceState->StateDescription = "Кассета открыта";
            //DeviceState->OutStateCode = DSE_STACKEROPEN;
            break;
        case 0x45:
            //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
            DeviceState->StateDescription = "Зажёвывание в акцепторе";
            break;
        case 0x46:
            //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
            DeviceState->StateDescription = "Зажёвывание в стэкере";
            break;
        case 0x47:
            //DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "Пауза";
            break;
        case 0x48:
            //DeviceState->OutStateCode = DSE_MAINERROR;
            DeviceState->StateDescription = "Взлом";
            break;
        case 0x49:
            //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
            DeviceState->StateDescription = "Сбой";
            break;
        case 0x4A:
            //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
            DeviceState->StateDescription = "Ошибка связи";
            break;
        case 0xC4:
        case 0xC3:
            DeviceState->StateDescription = "Доступен С3 С4";
            break;
        case 0xC1:
            DeviceState->StateDescription = "Уровень чувствительности установлен";
            break;
        default :
            BYTE data[1];
            data[0] = StatusCode;
            Log->WriteBuffer(data,1);
            DeviceState->StateDescription = "UNKNOWN STATUS CODE";
            break;
    }
    return DeviceState->StateDescription;
}

std::string TJCMDeviceThread::GetFailureDescription(BYTE StatusCode)
{
    switch(StatusCode)
    {
        case 0xA2:
            DeviceState->SubStateDescription = "Stack motor failure";
            break;
        case 0xA5:
            DeviceState->SubStateDescription = "Transport (feed) motor speed failure";
            break;
        case 0xA6:
            DeviceState->SubStateDescription = "Transport (feed) motor failure";
            break;
        case 0xAB:
            DeviceState->SubStateDescription = "Cash box not ready";
            break;
        case 0xAF:
            DeviceState->SubStateDescription = "Validator head remove";
            break;
        case 0xB0:
            DeviceState->SubStateDescription = "BOOT ROM failure";
            break;
        case 0xB1:
            DeviceState->SubStateDescription = "External ROM failure";
            break;
        case 0xB2:
            DeviceState->SubStateDescription = "ROM failure";
            break;
        case 0xB3:
            DeviceState->SubStateDescription = "External ROM writing failure";
            break;
        default :
            DeviceState->SubStateDescription = "UNKNOWN FAILURE CODE";
            break;
    }
    return DeviceState->SubStateDescription;
}



std::string TJCMDeviceThread::GetRejectionDescription(BYTE StatusCode)
{
    switch(StatusCode)
    {
        case 0x71:
            DeviceState->SubStateDescription = "Insertion error";
            break;
        case 0x72:
            DeviceState->SubStateDescription = "Mag error";
            break;
        case 0x73:
            DeviceState->SubStateDescription = "Rejection action by remaining of bills etc.";
            break;
        case 0x74:
            DeviceState->SubStateDescription = "Compensation error multiplying factor error";
            break;
        case 0x75:
            DeviceState->SubStateDescription = "Conveying error";
            break;
        case 0x76:
            DeviceState->SubStateDescription = "Denomination assessing error";
            break;
        case 0x77:
            DeviceState->SubStateDescription = "Photo pattern error 1";
            break;
        case 0x78:
            DeviceState->SubStateDescription = "Photo level error";
            break;
        case 0x79:
            DeviceState->SubStateDescription = "Return by inhibit insertion direction, denominating error";
            break;
        case 0x7A:
            DeviceState->SubStateDescription = "";
            break;
        case 0x7B:
            DeviceState->SubStateDescription = "Operation error";
            break;
        case 0x7C:
            DeviceState->SubStateDescription = "Rejecting action by remaining of bills and such (stacker section)";
            break;
        case 0x7D:
            DeviceState->SubStateDescription = "Length error";
            break;
        case 0x7E:
            DeviceState->SubStateDescription = "Photo pattern error 2";
            break;
        default :
            DeviceState->SubStateDescription = "UNKNOWN REJECTION CODE";
            break;
    }
    return DeviceState->SubStateDescription;
}

void TJCMDeviceThread::EnableAll()
{
  try
  {
    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0xff;
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      return;
    }
    BYTE command=0xC3;
    BYTE Data[1];
    Data[0] = 0x00;
    SendPacket(command,6,Data);
    ProcessCommand();
    DeviceState->Idle = false;
    //Log->Write("Idle = false");
    if (Terminated) return;
    command=0xC4;
    Data[1];
    Data[0] = 0x00;
    SendPacket(command,6,Data);
    Log->Write("Command Enable()");
    ProcessCommand();
    if (Terminated) return;

    if (DeviceState->StateCode == 0xff)
       return;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TJCMDeviceThread::Disable()
{
  try
  {
    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0xff;
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      return;
    }
    BYTE command=0xC3;
    BYTE Data[1];
    Data[0] = 0x01;
    SendPacket(command,6,Data);
    Log->Write("Command Disable()");
    ProcessCommand();
    //DeviceState->Idle = true;
    //Log->Write("Idle = true");
    if (Terminated) return;

    if (DeviceState->StateCode == 0xff)
       return;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TJCMDeviceThread::WriteEnableDenomination()
{
  try
  {
    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0xff;
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      return;
    }
    BYTE command=0xC0;
    BYTE Data[2];
    Data[0] = 0x00;
    Data[1] = 0x00;
    SendPacket(command,7,Data);
    ProcessCommand();

  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TJCMDeviceThread::PollingLoop()
{
    std::string mess, errormsg;

    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0xff;
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      return;
    }
    if (Terminated) return;

    //new 15-03-2007
    ProcessOutCommand();

    if (Terminated) return;

    int OldState = DeviceState->StateCode;
    GetStay();
    int State = DeviceState->StateCode;

    if (Terminated) return;

    //new 31-07-2007
    if (State == VENDVALID)
        SendACK();
    //==============

    if ((OldState == State)&&(EnterLoop == false))
        return;
    if(State == 0)
        return;

    EnterLoop = false;
    DeviceState->Billing = false;
    try
    {
        DeviceState->Enabling = false;
        switch (State)
        {
        case ENABLE: //Ожидание
        {
          if (DeviceState->Stacked)
            DeviceState->Processing = false;
          DeviceState->Stacked = false;
          DeviceState->OutStateCode = DSE_OK;
          DeviceState->Enabling = true;
          DeviceState->Idle = false;
          if (!DeviceState->BillEnable)
            Disable();
          break;
        }

        case _DISABLE: //недоступен
        //case DISABLE: //недоступен
        {
            SetInitialized();
            int TimeOut = 500;
            while ((DeviceState->Billing)&&(TimeOut>0))
            {
              TimeOut--;
              Sleep(10);
            }
            DeviceState->Idle = true;
            DeviceState->OutStateCode = DSE_OK;
            count_inkass = 0;
            if (Terminated)
              return;

            if (DeviceState->BillEnable)
            {
              WriteEnableDenomination();
              EnableAll();
              DeviceState->Idle = false;
            }
            //засекаем когда _DISABLE
            if (InitDisable)
                DisableTime = clock();
            InitDisable = false;
            if ((clock()-DisableTime)>10000)
            {
                if (DeviceState->OutStateCode != DSE_OK)
                {
                   DeviceState->OutStateCode = DSE_OK;
                   Log->Write((boost::format("OldOutStateCode=%1%; OutStateCode=%2%") % DeviceState->OldOutStateCode % DeviceState->OutStateCode).str().c_str());
                   DeviceState->StateChange = true;
                    if (DeviceStateChanged)
                      DeviceStateChanged(DeviceState);
                    else
                      Log->Write("TIMER IS NULL!");
                }
                DeviceState->OldOutStateCode = DeviceState->OutStateCode;
            }
        }
        break;

        case ACCEPTING: //втягивание купюры
            DeviceState->Processing = true;
        break;

        case VENDVALID: //валидация
            SendACK();
            ResetCount = 1;
            DeviceState->Billing = false;
            if (DeviceState->Stacking)
            {
                DeviceState->Billing = true;
                DeviceState->Stacking = false;
                bill_count += bill_nominal;
                DeviceState->Global += bill_nominal;
                bill_numb++;
                DeviceState->Count++;
                Log->Write((boost::format("VENDVALID : Stacked nominal = %1%") % bill_nominal).str().c_str());
            }
            else
                Log->Write("VENDVALID with Stacking = false. Error!");
            DeviceState->OutStateCode = DSE_OK;
            //new 06-08-2007
        break;

        case INITIALIZE: //инициализация
          Security();
          break;

        case STACKERFULL: // Касета полная
            DeviceState->Idle = true;
            if (Terminated) return;
            SendACK();
            if (Terminated) return;
            //DeviceState->OutStateCode = DSE_STACKERFULL;
            DeviceState->Billing = false;
            if (DeviceState->Stacking)
            {
                DeviceState->Billing = true;
                DeviceState->Stacking = false;
                bill_count += bill_nominal;
                DeviceState->Global += bill_nominal;
                bill_numb++;
                DeviceState->Count++;
                Log->Write((boost::format("STACKING A BILL WITH STACKER FULL : Stacked nominal = %1%") % bill_nominal).str().c_str());
            }
            else
                DeviceState->OutStateCode = DSE_STACKERFULL;
            //======================================================================
        break;

        case STACKEROPEN: // Вынули кассету
        count_inkass++;
        DeviceState->OutStateCode = DSE_STACKEROPEN;
        break;

        case POWERUP://Включили искричество with BILL
        case POWUPBILLACC://Включили искричество with BILL
        case POWUPBILLST://Включили искричество
            if (Terminated) return;
            if (ResetCount<=5)
            {
              Reset();
              ResetCount++;
            }
        break;

        case JAMINSTACKER:
        case JAMINACCEPTOR: //Замяло купюру
        {
            DeviceState->Stacking = false;
            DeviceState->OutStateCode = DSE_BILLJAM;
            if (Terminated) return;
            //if (DeviceState->OutStateCode == DSE_OK)
              //ResetCount = 1;
            /*if (ResetCount<=5)
            {
              Reset();
              Log->Write("ResetCount="+AnsiString(ResetCount));
              ResetCount++;
            }*/
        }
        break;

        case REJECTING:
          {
              //DeviceState->OutStateCode = DSE_OK;
              DeviceState->Stacking = false;
              //DeviceState->Billing = false;
              errormsg = RejectingDescr = GetRejectionDescription((BYTE)DeviceState->SubStateCode);
              //if (DeviceState->OutStateCode != DSE_HARDWARE_ERROR)
              if (DeviceState->OutStateCode != DSE_HARDWARE_ERROR)
                Log->Write((boost::format("Rejecting Description: %1%") % RejectingDescr.c_str()).str().c_str());
              //DeviceState->OutStateCode = DSE_MAINERROR;
              DeviceState->OutStateCode = DSE_BILLREJECT;
              switch(DeviceState->SubStateCode)
              {
                  case 0x71:
                  case 0x72:
                  case 0x73:
                  case 0x74:
                  case 0x75:
                  case 0x78:
                  case 0x7B:
                  case 0x7C:
                      DeviceState->OutStateCode = DSE_BILLREJECT;
              }
          }
          break;

        case CHEATED: //cheated
        //DeviceState->OutStateCode = DSE_CHEATED;
        break;

        case 0x00:
        case 0xFF:
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        break;

        case ESCROW://Поместили купюру в условное депонирование
        {
        DeviceState->OutStateCode = DSE_OK;
        DeviceState->Stacking = true;
        count_inkass = 0;
        //new
         if (Currency.LowerCase() == "rur")
         {
           BillDescr = " RUR";
           Log->Write(BillDescr.c_str());
           switch (DeviceState->SubStateCode)
           {
              case 99 ://0x63
                   bill_nominal = 10;break;
              case 100://0x64
                   bill_nominal = 50;break;
              case 101://0x65
                   bill_nominal = 100;break;
              case 102://0x66
                   bill_nominal = 500;break;
              case 103://0x67
                   bill_nominal = 1000;break;
              case 104://0x67
                   bill_nominal = 5000;break;
              default:
                   bill_nominal = 0;break;
           }
         }

         if (Currency.LowerCase() == "usd")
         {
           BillDescr = " USD";
           Log->Write(BillDescr.c_str());
           switch (DeviceState->SubStateCode)
           {
              case 0x61:
                   bill_nominal = 1*ExchangeRate;break;
              case 0x62:
                   bill_nominal = 2*ExchangeRate;break;
              case 0x63:
                   bill_nominal = 5*ExchangeRate;break;
              case 0x64:
                   bill_nominal = 10*ExchangeRate;break;
              case 0x65:
                   bill_nominal = 20*ExchangeRate;break;
              case 0x66:
                   bill_nominal = 50*ExchangeRate;break;
              case 0x67:
                   bill_nominal = 100*ExchangeRate;break;
              case 0x68:
                   bill_nominal = 1000*ExchangeRate;break;
              default:
                   bill_nominal = 0;break;
           }
         }

         if (Currency.LowerCase() == "kzt")
         {
           BillDescr = " KZT";
           Log->Write(BillDescr.c_str());
           switch (DeviceState->SubStateCode)
           {
              case 0x61:
                   //bill_nominal = 1*100*ExchangeRate;break;
                   bill_nominal = 0*ExchangeRate;break;
              case 0x62:
                   bill_nominal = 2*100*ExchangeRate;break;
              case 0x63:
                   bill_nominal = 5*100*ExchangeRate;break;
              case 0x64:
                   bill_nominal = 10*100*ExchangeRate;break;
              case 0x65:
                   bill_nominal = 20*100*ExchangeRate;break;
              case 0x66:
                   bill_nominal = 50*100*ExchangeRate;break;
              case 0x67:
                   bill_nominal = 100*100*ExchangeRate;break;
              case 0x68:
                   //bill_nominal = 1000*100*ExchangeRate;break;
                   bill_nominal = 0*ExchangeRate;break;
              default:
                   bill_nominal = 0;break;
           }
         }

        DeviceState->Nominal = bill_nominal;
        //DeviceState->Billing = false;
        if (Log)
            Log->Write((boost::format("MaxCash = %1%") % MaxCash).str().c_str());
        if (this->bill_nominal == 0)
            Return();
        else
        if ((this->bill_nominal > MaxCash)||(this->bill_nominal < MinCash))
        {
            DeviceState->Billing = false;
            mess = (boost::format("Returning nominal = %.2f") % bill_nominal).str();
            Return();
            if (Log) Log->Write(mess.c_str());
        }
        else
        {
            if (IsNominalEnabled(this->bill_nominal))
            {
              Stack();
              mess = (boost::format("Stacking nominal = %.2f") % bill_nominal).str();
              if (Log) Log->Write(mess.c_str());
            }
            else
            {
              DeviceState->Billing = false;
              mess = (boost::format("Returning nominal = %.2f") % bill_nominal).str();
              Return();
              if (Log) Log->Write(mess.c_str());
            }
        }
        }
        break;

        case FAILURE:
        FailureDescr = GetFailureDescription((BYTE)DeviceState->SubStateCode);
        if (DeviceState->OutStateCode != DSE_HARDWARE_ERROR)
            Log->Write((boost::format("Failure Description: %1%") % FailureDescr.c_str()).str().c_str());
        DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
        break;

        case STACKED:
          /*
          ResetCount = 1;
          DeviceState->Billing = false;
          if (DeviceState->Stacking)
          {
              DeviceState->Billing = true;
              DeviceState->Stacking = false;
              bill_count += bill_nominal;
              DeviceState->Global += bill_nominal;
              bill_numb++;
              DeviceState->Count++;
              Log->Write("STACKED : Stacked nominal = "+AnsiString(bill_nominal));
          }
          else
              Log->Write("STACKED with Stacking = false. Error!");
          */
          DeviceState->Stacked = true;
          DeviceState->OutStateCode = DSE_OK;
          break;
        }

        if (DeviceState->StateCode != _DISABLE)
          InitDisable = true;

        if (OldState == STACKEROPEN)
          DeviceState->OutStateCode = DSE_OK;

        Log->Write((boost::format("State = %1% - [%2%] %3%; OldState = %4%; OutState = %5%") % State % DeviceState->StateCode % GetStateDescription(State) % OldState % DeviceState->OutStateCode).str().c_str());
        DeviceState->OldStateCode = OldState;
        if(State != DeviceState->StateCode)
            DeviceState->StateCode = State;

        //if ((DeviceState->OldOutStateCode != DeviceState->OutStateCode)||(State == STACKED))
        if ((DeviceState->OldOutStateCode != DeviceState->OutStateCode) ||
            (DeviceState->Stacked) ||
            (DeviceState->Processing) ||
            (DeviceState->Enabling))
        {
          DeviceState->StateChange = true;
          //if ((DeviceState->OldOutStateCode == DeviceState->OutStateCode)&&(State == STACKED))
          if ((DeviceState->OldOutStateCode == DeviceState->OutStateCode)&&(State == VENDVALID))
              DeviceState->StateChange = false;
          Log->Write((boost::format("OldOutStateCode=%1%; OutStateCode=%2%; Billing=%3%; DeviceState->StateChange = %4%") % DeviceState->OldOutStateCode % DeviceState->OutStateCode % DeviceState->Billing % DeviceState->StateChange).str().c_str());
          Log->Write((boost::format("Idle = %1%") % DeviceState->Idle).str().c_str());
          if (DeviceStateChanged)
            DeviceStateChanged(DeviceState);
          else
            Log->Write("TIMER IS NULL!");
          DeviceState->OldOutStateCode = DeviceState->OutStateCode;
        }

        /*if (DeviceState->OldState != State)
        {
        bool StackerClosed = false;
        if ((TJCMState)DeviceState->OldState == STACKEROPEN)
           StackerClosed = true;
        DeviceState->OldState = State;
        Log->Write("State = "+GetStateDescription(State));
        if (StackerClosed)
          //DeviceState->OutStateCode = DSE_SETCASSETTE;
          DeviceState->OutStateCode = DSE_OK;

        //if ((DeviceState->OldOutStateCode != DeviceState->OutStateCode)||(DeviceState->Billing == true))
        if ((DeviceState->OldOutStateCode != DeviceState->OutStateCode)||(State == STACKED))
        {
          DeviceState->StateChange = true;
          //if ((DeviceState->OldOutStateCode == DeviceState->OutStateCode)&&(DeviceState->Billing == true))
          if (State == STACKED)
              DeviceState->Billing = true;
          if ((DeviceState->OldOutStateCode == DeviceState->OutStateCode)&&(State == STACKED))
              DeviceState->StateChange = false;
          Log->Write("OldOutStateCode="+AnsiString(DeviceState->OldOutStateCode)+"; OutStateCode="+AnsiString(DeviceState->OutStateCode)+"; Billing="+AnsiString((int)DeviceState->Billing)+"; DeviceState->StateChange = "+AnsiString((int)DeviceState->StateChange));
          Log->Write("Idle = "+AnsiString((int)DeviceState->Idle));
          //DeviceState->OldOutStateCode = DeviceState->OutStateCode;
          if (DeviceStateChanged)
            DeviceStateChanged(DeviceState);
          else
            Log->Write("TIMER IS NULL!");
          DeviceState->OldOutStateCode = DeviceState->OutStateCode;
        }
        }*/
    }
    __finally
    {
    }
}

void TJCMDeviceThread::ParseAnswer(int mode)
{
  try
  {
    ClearBuffer(data);
    *len_data = 0;
    if (DeviceState)
      AnswerSize = DeviceState->AnswerSize;
    if (AnswerSize>3)
    {
       DeviceState->SubStateCode = 0;
       DeviceState->StateCode = 0;
       int len_command = (int)Answer[1];
       if ((len_command == 5)&&(Answer[2] == 0x50)) //ACK
        return;
       DeviceState->StateCode = Answer[2];

       if (len_command >= 6)
       {
          DeviceState->SubStateCode = Answer[3];
       }
       //Log->Write(GetStatusDescription(DeviceState->StateCode));
       if (len_command > 6)
       {
         *len_data = len_command - 6;
         memcpy(data,&Answer[6],*len_data);
         if (Log)
         {
            //Log->Write("Answer data:");
            //Log->WriteBuffer(data, *len_data);
         }
       }
    }
  }
  __finally
  {
  }
}

std::string TJCMDeviceThread::GetStateDescription(int code)
{
  return GetStatusDescription((BYTE)code);
}

bool TJCMDeviceThread::IsItYou()
{
    GetStay();
    if (DeviceState->StateCode == 0xFF)
        return false;

    Disable();
    GetStay();
    if ((DeviceState->StateCode != DISABLE)&&(DeviceState->StateCode != _DISABLE))
        return false;

    return true;
}

void __fastcall TJCMDeviceThread::ProcessLoopCommand()
{
  ThreadTerminated = false;
  try
  {
    if (Terminated) return;
    GetStay();
    if (Terminated) return;
    if (DeviceState->StateCode != 0xff && DeviceState->StateCode != 0x00)
    {
      DeviceState->OutStateCode = DSE_OK;
      if (Log) Log->Write("JCM Device has found.");
      ChangeDeviceState();
      Reset();
    }
    else
    {
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        if (Log != NULL)
          if (Log) Log->Write("No JCM Device Present...");
        /*DeviceState->StateChange = true;
        if (DeviceStateChanged)
          DeviceStateChanged(DeviceState);
        DeviceState->OldOutStateCode = DeviceState->OutStateCode;*/
        ChangeDeviceState();
    }

    LoopOfCommands();
  }
  __finally
  {
  }
}

void TJCMDeviceThread::Security()
{
  try
  {
    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0xff;
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      return;
    }
    BYTE command=0xC1;
    BYTE Data[2];
    Data[0] = 0x82;
    Data[1] = BillsSensitivity;
    SendPacket(command,7,Data);
    ProcessCommand();

  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
}

void TJCMDeviceThread::ProcessOutCommand()
{
    try
    {
        BYTE Command = GetExtCommand();
        switch (Command)
        {
          case oc_DisableBill:
            Log->Write("ExtCommand DisableBill");
            Disable();
            break;
          case oc_EnableBill:
            Log->Write("ExtCommand EnableBill");
            WriteEnableDenomination();
            EnableAll();
            DeviceState->Idle = false;
            break;
        }
    }
    __finally
    {
        //ExtCommand = EXTC_Free;
    }
}

