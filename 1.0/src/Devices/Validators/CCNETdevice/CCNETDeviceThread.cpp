//---------------------------------------------------------------------------


#pragma hdrstop

#include <math.h>
#include "CCNETDeviceThread.h"
#include "CValidator.h"
#include "boost/format.hpp"

//---------------------------------------------------------------------------

#pragma package(smart_init)

#define POLYNOMINAL 0x08408
#define PollingInterval 100

__fastcall TCCNETDeviceThread::TCCNETDeviceThread() : TDeviceThread(true)
{
  bill_count_old = bill_count = 0;
  mask = 0xffff;// bill mask
  count_inkass = bill_numb = 0;
  lang = 1;
  LastEnDisCommand = 0;
}

__fastcall TCCNETDeviceThread::~TCCNETDeviceThread()
{
}

void __fastcall TCCNETDeviceThread::ProcessLoopCommand()
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
      if (Log) Log->Write("CCNET Device has found.");
      ChangeDeviceState();
      Reset();
    }
    else
    {
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        if (Log != NULL)
          if (Log) Log->Write("No CCNET Device Present...");
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

void TCCNETDeviceThread::ParseAnswer(int mode)
{
  try
  {
    ClearBuffer(data);
    *len_data = 0;
    if (DeviceState)
      AnswerSize = DeviceState->AnswerSize;
    if ((AnswerSize>3)&&(PollingMode))
    {
       int len_command = (int)Answer[2];
       if (Answer[3] != 0)
           DeviceState->StateCode = Answer[3];
       if (len_command >= 7)
          DeviceState->SubStateCode = Answer[4];
       if (len_command > 7)
       {
         *len_data = len_command - 7;
         memcpy(data,&Answer[7],*len_data);
       }
    }
    BYTE command = CommandParameters->CommandCode;
    CommandParameters->SetParameters(Command,CommandSize,command,DeviceState->StateCode);
  }
  __finally
  {
  }
}

void TCCNETDeviceThread::calc_crc(BYTE byte)
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

unsigned short TCCNETDeviceThread::Calc_CRC16CCNET(BYTE* DataBuf, unsigned short  BufLen)
{
   unsigned short i;
   inter_crc_h = inter_crc_l = 0;
   for (i=0;i<BufLen;i++) calc_crc(DataBuf[i]);
   i=inter_crc_h;
   i<<=8;
   i+=inter_crc_l;
   return i;
}

void TCCNETDeviceThread::SendPacket(BYTE command,int len_packet, BYTE* data)
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

    Command[0] = 0x2;
    Command[1] = 0x3;
    BYTE len_char = (BYTE)len_packet;
    int n_sm = 4;
    if (len_packet<256)
       len_char = (BYTE)len_packet;
    else
    {
      Command[4] = (BYTE)len_packet;
      Command[5] = (BYTE)(len_packet >> 8);
      n_sm = 6;
    }
    Command[2] = (BYTE)len_char;
    Command[3] = (BYTE)command;
    if (data!=NULL)
      memcpy(&Command[n_sm],data,len_packet-n_sm-2);

    Calc_CRC16CCNET(Command,(unsigned short)(len_packet-2));
    Command[len_packet-2] = inter_crc_h;
    Command[len_packet-1] = inter_crc_l;
    Command[len_packet] = 3;

    CommandParameters->SetParameters(Command,len_packet,command,0);
    CommandSize = len_packet;
  }
  __finally
  {
  }
}

void TCCNETDeviceThread::GetStay()//POLL command
{
  try
  {
    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0xff;
      return;
    }
    BYTE command = 0x33;
    SendPacket(command,6,NULL);
    ProcessCommand();
    if (Terminated) return;

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
    if (DeviceState->StateCode == 0xff)
       return;

    if (Terminated) return;

    SendACK();
  }
  __finally
  {
  }
}

void TCCNETDeviceThread::Reset()
{
  try
  {
    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0xff;
      return;
    }
    BYTE command=0x30;
    SendPacket(command,6,NULL);
    ProcessCommand();
    Log->Write("RESET");
    if (Terminated) return;

    if (DeviceState->StateCode == 0xff)
       return;

    if (DeviceState->StateCode == 0)
       bill_nominal = 0;
  }
  __finally
  {
  }
}

void TCCNETDeviceThread::EnableAll()
{
  try
  {
    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0xff;
      return;
    }
    DeviceState->Idle = false;
    Log->Write("Idle = false");
    BYTE command=0x34;
    BYTE billmask[10];
    memset (billmask,0xff,10); //enable all
    if (mask!=0)
    {
      billmask[1] = (BYTE)( (mask>>8) & 0xff);
      billmask[2] = (BYTE)(mask & 0xff);
      // 0 - skip (16-23 bill type)
      billmask[4] = billmask[1];
      billmask[5] = billmask[2];
      // 3 - skip (16-23 bill type)
    }
    SendPacket(command,12,billmask);
    ProcessCommand();
    Log->Write("ENABLE BILL");
    LastEnDisCommand = 2;
  }
  __finally
  {
  }
}

void TCCNETDeviceThread::Disable()
{
  try
  {
    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0xff;
      return;
    }
    if (DeviceState->StateCode == 0xff)
      return;
    BYTE command=0x34;
    SendPacket(command,12,(BYTE*)"\x00\x00\x00\x00\x00\x00");
    ProcessCommand();
    Log->Write("DISABLE BILL");
    LastEnDisCommand = 1;
  }
  __finally
  {
  }
}

void TCCNETDeviceThread::CommandDisable()
{
  try
  {
    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0xff;
      return;
    }
    if (DeviceState->StateCode == 0xff)
      return;
    BYTE command=0x34;
    SendPacket(command,12,(BYTE*)"\x00\x00\x00\x00\x00\x00");
  }
  __finally
  {
  }
}

void TCCNETDeviceThread::Return(void)
{
  try
  {
    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0xff;
      return;
    }
    BYTE command=0x36;
    SendPacket(command,6,NULL);
    ProcessCommand();
    if (Terminated) return;

    if (DeviceState->StateCode == 0xff)
       return;

    if (Terminated)
      return;
    GetStay();
    if (Terminated) return;
    for (int i=0;i<100 && (DeviceState->StateCode == 0x18);i++)
    {
      if (Terminated) return;
      GetStay();
      if (Terminated) return;
    }
  }
  __finally
  {
  }
}

void TCCNETDeviceThread::Hold(void)
{
  try
  {
    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0xff;
      return;
    }
    BYTE command=0x38;
    SendPacket(command,6,NULL);
    ProcessCommand();
  }
  __finally
  {
  }
}

int TCCNETDeviceThread::Stack()
{
  int stacked=0;
  try
  {
    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0xff;
      return 0;
    }
    BYTE command=0x35;
    SendPacket(command,6,NULL);
    ProcessCommand();
  }
  __finally
  {
  }
  return stacked;
}

void TCCNETDeviceThread::SendACK()
{
  TSendType old = SendType;
  SendType = NotRecieveAnswer;
  SendPacket(0,6,NULL);
  ProcessCommand();
  SendType = old;
}

void TCCNETDeviceThread::SendNAK()
{
  TSendType old = SendType;
  SendType = NotRecieveAnswer;
  SendPacket(0xFF,6,NULL);
  ProcessCommand();
  SendType = old;
}

void TCCNETDeviceThread::PollingLoop()
{
    AnsiString mess;
//    int count_inkass=0;
    int error=0;

    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0xff;
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      return;
    }
    if (Terminated) return;

    //new 16-03-2007
    ProcessOutCommand();

    if (Terminated) return;

    int OldState = DeviceState->StateCode;
    GetStay();
    int State = DeviceState->StateCode;

    if (Terminated) return;

    try
    {
      if ((OldState == State)&&(EnterLoop == false))
      {
          //if (Terminated) return;
          //SendACK();
          return;
      }

      if(State == 0)
          return;

      //вход в цикл, при условии что состояние не изменилось, только когда клиент разрешает\запрещает приём купюр
      EnterLoop = false;
      //DeviceState->Billing = false;

      DeviceState->Enabling = false;
      switch (State)
      {
          case 0x14: //Ожидание
            if (Terminated) return;
            //SendACK();
            if (DeviceState->Stacked)
              DeviceState->Processing = false;
            DeviceState->Stacked = false;
            DeviceState->Enabling = true;
            if ((!DeviceState->BillEnable)||(LastEnDisCommand==1))
                Disable();
            if (LastEnDisCommand == 2)
                LastEnDisCommand = 0;
            DeviceState->OutStateCode = DSE_OK;
            DeviceState->SetOutCodes(DSE_NOTSET,VLD_BILL_ENABLE);
            break;

          case 0x19: //недоступен
            SetInitialized();
            if (Terminated) return;
            //SendACK();
            DeviceState->Idle = true;
            Log->Write("Idle = true");
            error=0;
            if (Terminated) return;
            if ((DeviceState->BillEnable)||(LastEnDisCommand==2))
              EnableAll();
            if (LastEnDisCommand == 1)
                LastEnDisCommand = 0;
            DeviceState->OutStateCode = DSE_OK;
            DeviceState->SetOutCodes(DSE_NOTSET,VLD_BILL_DISABLE);
            break;

          case 0x15: //акцептирование
            error = 0;
            if (Terminated) return;
            //SendACK();
            DeviceState->Processing = true;
            DeviceState->SetOutCodes(DSE_NOTSET,VLD_ACCEPTING);
            if (!DeviceState->BillEnable)
              Return();
            break;

          case 0x13: //инициализация
            if (Terminated) return;
            DeviceState->SetOutCodes(DSE_NOTSET,VLD_INITIALIZE);
            //SendACK();
            break;

          case 0x41: // Касета полная
            if (Terminated) return;
            DeviceState->OutStateCode = DSE_STACKERFULL;
            DeviceState->SetOutCodes(VLD_STACKER_FULL,VLD_STACKER_FULL);
            //SendACK();
            if (Terminated) return;
            Reset();
            error = 1;
          break;

          case 0x42: // Вынули кассету
            if (Terminated) return;
            //SendACK();
            DeviceState->OutStateCode = DSE_STACKEROPEN;
            DeviceState->SetOutCodes(DSE_NOTSET,VLD_STACKER_OPENED);
            break;

          case 0x11://Включили искричество with BILL
              if (Terminated) return;
              DeviceState->SetOutCodes(DSE_NOTSET,VLD_POWER_UP_WITH_BILL_IN_ACCEPTOR);
              Reset();
              break;
          case 0x12://Включили искричество with BILL
              if (Terminated) return;
              DeviceState->SetOutCodes(DSE_NOTSET,VLD_POWER_UP_WITH_BILL_IN_STACKER);
              Reset();
              break;
          case 0x10://Включили искричество
            if (Terminated) return;
            DeviceState->SetOutCodes(DSE_NOTSET,VLD_INITIALIZE);
            //SendACK();
            if (Terminated) return;
            Reset();
            break;

          case 0x43: //Замяло купюру
            DeviceState->OutStateCode = DSE_BILLJAM;
            if (Terminated) return;
            //SendACK();
            DeviceState->SetOutCodes(VLD_BILL_JAM,VLD_BILL_JAM);
            if (Terminated) return;
            if (error == 0)
              Reset();
            error=1;
            break;

          case 0x1C:// rejecting Error
              Log->Write((boost::format("Main Failure Description: %1%") % GetMainFailureDescription(DeviceState->SubStateCode).c_str()).str().c_str());
              //DeviceState->OutStateCode = DSE_MAINERROR;
              DeviceState->OutStateCode = DSE_BILLREJECT;
              switch (DeviceState->SubStateCode)
              {
                case 0x60:
                case 0x61:
                case 0x62:
                case 0x63:
                case 0x64:
                case 0x67:
                case 0x69:
                case 0x6A:
                  DeviceState->OutStateCode = DSE_BILLREJECT;
              }
              error = 1;
              if (Terminated) return;
              //SendACK();
            break;

          case 0x45: //cheated
            if (Terminated) return;
            //SendACK();
            DeviceState->SetOutCodes(VLD_CHEATED,VLD_CHEATED);
            //DeviceState->OutStateCode = DSE_CHEATED;
            break;

          case 0x44:// cassete jammed
            if (Terminated) return;
            //SendACK();
            if (Terminated) return;
            DeviceState->SetOutCodes(VLD_BILL_JAM,VLD_BILL_JAM);
            if (DeviceState->OutStateCode != DSE_BILLJAM)
              Reset();
            error = 1;
            DeviceState->OutStateCode = DSE_BILLJAM;
            break;

          case 0x47: // сбой оборудования
              error = 1;
              Log->Write((boost::format("Hardware Failure Description: %1%") % GetHardwareFailureDescription(DeviceState->SubStateCode).c_str()).str().c_str());
              DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
              if (Terminated) return;
              //SendACK();
              break;

          case 0xFF:
            //error = 1;
            //DeviceState->OutStateCode = DSE_NOTMOUNT;
            break;

          case 0x80:  //ESCROW
          {
             bill_nominal = 0;
             DeviceState->SetOutCodes(DSE_NOTSET,VLD_ACCEPTING);
             DeviceState->Stacking = true;
             Log->Write((boost::format("Укладка купюры с кодом %1%") % DeviceState->SubStateCode).str().c_str());
             if (Currency.LowerCase() == "rur")
             {
                 BillDescr = " RUR";
                 Log->Write(BillDescr.c_str());
                 switch (DeviceState->SubStateCode)
                 {
                    case 1:
                         bill_nominal = 5;break;
                    case 2:
                         bill_nominal = 10;break;
                    case 3:
                         bill_nominal = 50;break;
                    case 4:
                         bill_nominal = 100;break;
                    case 5:
                         bill_nominal = 500;break;
                    case 6:
                         bill_nominal = 1000;break;
                    case 7:
                         bill_nominal = 5000;break;
                    case 23:
                         break;
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
                    case 1:
                         bill_nominal = 200;break;
                    case 2:
                         bill_nominal = 500;break;
                    case 3:
                         bill_nominal = 1000;break;
                    case 4:
                         bill_nominal = 2000;break;
                    case 5:
                         bill_nominal = 5000;break;
                    case 6:
                         bill_nominal = 10000;break;
                    default:
                         bill_nominal = 0;break;
                 }
             }
             if (Currency.LowerCase() == "uah")
             {
                 BillDescr = " UAH";
                 Log->Write(BillDescr.c_str());
                 switch (DeviceState->SubStateCode)
                 {
                    case 0:
                         bill_nominal = 1;break;
                    case 1:
                         bill_nominal = 2;break;
                    case 2:
                         bill_nominal = 5;break;
                    case 3:
                         bill_nominal = 10;break;
                    case 4:
                         bill_nominal = 20;break;
                    case 5:
                         bill_nominal = 50;break;
                    case 6:
                         bill_nominal = 100;break;
                    case 7:
                         bill_nominal = 200;break;
                    case 8:
                         bill_nominal = 500;break;
                    default:
                         bill_nominal = 0;break;
                 }
             }
             if (Currency.LowerCase() == "eur")
             {
                 BillDescr = " EUR";
                 Log->Write(BillDescr.c_str());
                 switch (DeviceState->SubStateCode)
                 {
                    case 2: bill_nominal = 5;break;
                    case 3: bill_nominal = 10;break;
                    case 4: bill_nominal = 20;break;
                    case 5: bill_nominal = 50;break;
                    case 6: bill_nominal = 100;break;
                    case 7: bill_nominal = 500;break;
                    default:
                         bill_nominal = 0;break;
                 }
             }

             if (Currency.LowerCase() == "uzs")
             {
                 BillDescr = " UZS";
                 Log->Write(BillDescr.c_str());
                 switch (DeviceState->SubStateCode)
                 {
                    case 1: bill_nominal = 100;break;
                    case 2: bill_nominal = 200;break;
                    case 3: bill_nominal = 500;break;
                    case 4: bill_nominal = 1000;break;
                    default:
                         bill_nominal = 0;break;
                 }
             }

            DeviceState->Nominal = bill_nominal;
            if (Terminated) return;
            //SendACK();
            if (this->bill_nominal == 0)
            {
                DeviceState->Stacking = false;
                Return();
            }
            else
            if ((this->bill_nominal > MaxCash)||(this->bill_nominal < MinCash))
            {
                DeviceState->Billing = false;
                DeviceState->Stacking = false;
                mess.sprintf("Returning nominal = %.2f",bill_nominal);
                Return();
                if (Log) Log->Write(mess.c_str());
            }
            else
            {
                if (IsNominalEnabled(this->bill_nominal))
                {
                  mess.sprintf("Stacking nominal = %.2f",bill_nominal);
                  if (Log) Log->Write(mess.c_str());
                  Stack();
                }
                else
                {
                  DeviceState->Billing = false;
                  mess.sprintf("Returning nominal = %.2f",bill_nominal);
                  DeviceState->Stacking = false;
                  Return();
                  if (Log) Log->Write(mess.c_str());
                }
            }
          }
          break;

          case 0x17:
              DeviceState->SetOutCodes(DSE_NOTSET,VLD_STACKING);
              if (Log) Log->Write("Stacking");
              /*DeviceState->Stacking = true;
              for (int i=0;i<100 && (DeviceState->StateCode==0x17);i++)
              {
                if (Terminated) return;
                GetStay();
              }*/
              break;

          case 0x81:  //STACKED
            DeviceState->Billing = false;
            DeviceState->OutStateCode = DSE_OK;
            if (DeviceState->Stacking == false)
            {
                Log->Write("STACKED with Stacking = false. Error!");
                //08-07-2007
                if (Terminated) return;
                for (int i=0;i<200 && (DeviceState->StateCode == 0x81);i++)
                {
                  if (Terminated) return;
                  SendACK();
                  Sleep(50);
                  GetStay();
                }
                if (DeviceState->StateCode == 0x81)
                {
                    Log->Write("Validator frozen in state STACKED");
                    DeviceState->OutStateCode = DSE_NOTMOUNT;
                    Reset();
                    Sleep(1000);
                    EnableAll();
                }
                //break;
                return;
            }
            else
            {
                DeviceState->SetOutCodes(DSE_NOERROR,VLD_STACKED);
                DeviceState->Billing = true;
                DeviceState->Stacking = false;
                DeviceState->Stacked = true;
                bill_count += bill_nominal;
                DeviceState->Global += bill_nominal;
                bill_numb++;
                DeviceState->Count++;
                mess.sprintf("Stacked nominal = %.2f",bill_nominal);
                if (Log) Log->Write(mess.c_str());
                DeviceState->OutStateCode = DSE_OK;
                error = 0;
            }
            break;

          case 0x82: // bill returned
            error = 0;
            DeviceState->SetOutCodes(VLD_RETURNED,VLD_RETURNED);
            if (Terminated) return;
            //SendACK();
            break;

          default:
              if (Terminated) return;
              //SendACK();
      }

      if (Terminated)
        return;

      GetStateDescription();
      Log->Write((boost::format("State = %1% - [%2%] %3%; OldState = %4%; OutState = %5%") % DeviceState->StateCode % State % DeviceState->StateDescription % OldState % DeviceState->OutStateCode).str().c_str());
      if (DeviceState->OldStateCode == 0x42)
          //DeviceState->OutStateCode = DSE_SETCASSETTE;
          DeviceState->OutStateCode = DSE_OK;
      DeviceState->OldStateCode = OldState;
      if(DeviceState->StateCode == 0)
          DeviceState->StateCode = State;
      if ((DeviceState->OldOutStateCode != DeviceState->OutStateCode) ||
          (DeviceState->Stacked) ||
          (DeviceState->Processing) ||
          (DeviceState->Enabling))
      {
        DeviceState->StateChange = true;
        if ((DeviceState->OldOutStateCode == DeviceState->OutStateCode)&&(DeviceState->Billing == true))
            DeviceState->StateChange = false;
        Log->Write((boost::format("OldOutStateCode=%1%; OutStateCode=%2%; Billing=%3%") % DeviceState->OldOutStateCode % DeviceState->OutStateCode % DeviceState->Billing).str().c_str());
        if (DeviceStateChanged)
          DeviceStateChanged(DeviceState);
        if (State == 0x81) //STACKED
        {
            DeviceState->Billing = false;
            DeviceState->Nominal = 0;
        }
        DeviceState->OldOutStateCode = DeviceState->OutStateCode;
      }
    }
    __finally
    {
    }
}

std::string TCCNETDeviceThread::GetStateDescription()
{
  if (DeviceState)
  {
    switch (DeviceState->StateCode)
    {
      case 0x00:
        //DeviceState->OutStateCode = DSE_OK;
        break;
      case 0xFF:
        //DeviceState->OutStateCode = DSE_NOTMOUNT;
        break;
      case 0x10:
      case 0x11:
      case 0x12:
        DeviceState->OutStateCode = DSE_OK;
        break;
      case 0x1C:
        //DeviceState->OutStateCode = DSE_MAINERROR;
        DeviceState->OutStateCode = DSE_BILLREJECT;
        switch (DeviceState->SubStateCode)
        {
          case 0x60:
          case 0x61:
          case 0x62:
          case 0x63:
          case 0x64:
          case 0x67:
          case 0x69:
          case 0x6A:
            DeviceState->OutStateCode = DSE_BILLREJECT;
        }
        break;
      case 0x47:
        DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
        break;
      case 0x13:
        DeviceState->OutStateCode = DSE_OK;
        break;
      case 0x14:
        DeviceState->OutStateCode = DSE_OK;
        break;
      case 0x15:
        //DeviceState->OutStateCode = DSE_OK;
        break;
      case 0x17:
        //DeviceState->OutStateCode = DSE_OK;
        break;
      case 0x18:
        DeviceState->OutStateCode = DSE_OK;
        break;
      case 0x19:
        //DeviceState->OutStateCode = DSE_OK;
        break;
      case 0x1A:
        //DeviceState->OutStateCode = DSE_OK;
        break;
      case 0x1B:
        //DeviceState->OutStateCode = DSE_OK;
        break;
      case 0x1D:
        //DeviceState->OutStateCode = DSE_OK;
        break;
      case 0x1E:
        //DeviceState->OutStateCode = DSE_OK;
        break;
      case 0x21:
        //DeviceState->OutStateCode = DSE_SETCASSETTE;
        DeviceState->OutStateCode = DSE_OK;
        break;
      case 0x25:
        //DeviceState->OutStateCode = DSE_OK;
        break;
      case 0x26:
        //DeviceState->OutStateCode = DSE_OK;
        break;
      case 0x28:
        //DeviceState->OutStateCode = DSE_MAINERROR;
        break;
      case 0x29:
        //DeviceState->OutStateCode = DSE_SETCASSETTE;
        DeviceState->OutStateCode = DSE_OK;
        break;
      case 0x30:
        //DeviceState->OutStateCode = DSE_OK;
        break;
      case 0x41:
        DeviceState->OutStateCode = DSE_STACKERFULL;
        break;
      case 0x42:
        DeviceState->OutStateCode = DSE_STACKEROPEN;
        break;
      case 0x43:
        DeviceState->OutStateCode = DSE_BILLJAM;
        break;
      case 0x44:
        DeviceState->OutStateCode = DSE_BILLJAM;
        break;
      case 0x45:
        //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
        //DeviceState->OutStateCode = DSE_CHEATED;
        break;
      case 0x46:
        DeviceState->OutStateCode = DSE_OK;
        break;
      case 0x80:
        //DeviceState->OutStateCode = DSE_OK;
        break;
      case 0x81:
        DeviceState->OutStateCode = DSE_OK;
        break;
      case 0x82:
        //DeviceState->OutStateCode = DSE_OK;
        break;
    }


    if (lang == 0)
    switch (DeviceState->StateCode)
    {
      case 0x00:
        //DeviceState->StateDescription = "ACK";
        break;
      case 0xFF:
        DeviceState->StateDescription = "NAK";
        break;
      case 0x10:
      case 0x11:
      case 0x12:
        DeviceState->StateDescription = "POWER UP";
        break;
      case 0x1C:
        DeviceState->StateDescription = "REJECTING";
        break;
      case 0x47:
        //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
        DeviceState->StateDescription = "HARDWARE FAILURE";
        break;
      case 0x13:
        //DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "INITIALIZE";
        break;
      case 0x14:
        ////DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "IDLING";
        break;
      case 0x15:
        ////DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "ACCEPTING";
        break;
      case 0x17:
        ////DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "STACKING";
        break;
      case 0x18:
        //DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "RETURNING";
        break;
      case 0x19:
        //DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "DISABLED";
        break;
      case 0x1A:
        ////DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "HOLDING";
        break;
      case 0x1B:
        //DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "BUSY";
        break;
      case 0x1D:
        //DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "DISPENSING";
        break;
      case 0x1E:
        //DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "UNLOADING";
        break;
      case 0x21:
        //DeviceState->OutStateCode = DSE_SETCASSETTE;
        DeviceState->StateDescription = "SETTING TYPE CASSETTE";
        break;
      case 0x25:
        //DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "DISPENSED";
        break;
      case 0x26:
        //DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "UNLOADED";
        break;
      case 0x28:
        //DeviceState->OutStateCode = DSE_MAINERROR;
        DeviceState->StateDescription = "INVALID BILL NUMBER";
        break;
      case 0x29:
        //DeviceState->OutStateCode = DSE_SETCASSETTE;
        DeviceState->StateDescription = "SET TYPE CASSETTE";
        break;
      case 0x30:
        //DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "INVALID COMMAND";
        break;
      case 0x41:
        //DeviceState->OutStateCode = DSE_STACKERFULL;
        DeviceState->StateDescription = "DROP CASSETTE  FULL";
        break;
      case 0x42:
        //DeviceState->OutStateCode = DSE_STACKEROPEN;
        DeviceState->StateDescription = "DROP CASSETTE REMOVED";
        break;
      case 0x43:
        //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
        DeviceState->StateDescription = "JAM IN ACCEPTOR";
        break;
      case 0x44:
        //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
        DeviceState->StateDescription = "JAM IN STACKER";
        break;
      case 0x45:
        //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
        DeviceState->StateDescription = "CHEATED";
        break;
      case 0x46:
        //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
        DeviceState->StateDescription = "PAUSED";
        break;
      case 0x80:
        ////DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "ESCROW";
        break;
      case 0x81:
        //DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "PACKED, STACKED";
        break;
      case 0x82:
        //DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "RETURNED";
        break;
    }
    if (lang == 1)
    switch (DeviceState->StateCode)
    {
      case 0x00:
        //DeviceState->OutStateCode = DSE_OK;
        //DeviceState->StateDescription = "ACK";
        break;
      case 0xFF:
        //DeviceState->OutStateCode = DSE_NOTMOUNT;
        DeviceState->StateDescription = "NAK";
        break;
      case 0x10:
      case 0x11:
      case 0x12:
        //DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "Включение питания";
        break;
      case 0x1C:
        //DeviceState->OutStateCode = DSE_MAINERROR;
        DeviceState->StateDescription = "Выброс купюры";
        break;
      case 0x47:
        //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
        DeviceState->StateDescription = "Сбой оборудования";
        break;
      case 0x13:
        //DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "Инициализация";
        break;
      case 0x14:
        ////DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "Ожидание";
        break;
      case 0x15:
        ////DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "Разрешение";
        break;
      case 0x17:
        ////DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "Укладка купюры";
        break;
      case 0x18:
        //DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "Возврат";
        break;
      case 0x19:
        //DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "Недоступен";
        break;
      case 0x1A:
        ////DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "Удержание";
        break;
      case 0x1B:
        DeviceState->StateDescription = "Занят";
        //DeviceState->OutStateCode = DSE_OK;
        break;
      case 0x1D:
        //DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "Выдача";
        break;
      case 0x1E:
        //DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "Незагружен";
        break;
      case 0x21:
        //DeviceState->OutStateCode = DSE_SETCASSETTE;
        DeviceState->StateDescription = "Установка кассеты";
        break;
      case 0x25:
        //DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "Выдано";
        break;
      case 0x26:
        //DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "Разгружено";
        break;
      case 0x28:
        //DeviceState->OutStateCode = DSE_MAINERROR;
        DeviceState->StateDescription = "Неверный код купюры";
        break;
      case 0x29:
        //DeviceState->OutStateCode = DSE_SETCASSETTE;
        DeviceState->StateDescription = "Кассета установлена";
        break;
      case 0x30:
        //DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "Неверная команда";
        break;
      case 0x41:
        //DeviceState->OutStateCode = DSE_STACKERFULL;
        DeviceState->StateDescription = "Кассета переполнена";
        break;
      case 0x42:
        //DeviceState->OutStateCode = DSE_STACKEROPEN;
        DeviceState->StateDescription = "Кассета снята";
        break;
      case 0x43:
        //DeviceState->StateDescription = "Зажёвывание в акцепторе";
        //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
        DeviceState->StateDescription = "Купюра застряла";
        break;
      case 0x44:
        //DeviceState->StateDescription = "Зажёвывание в стэкере";
        //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
        DeviceState->StateDescription = "Купюра застряла";
        break;
      case 0x45:
        //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
        DeviceState->StateDescription = "Взлом";
        break;
      case 0x46:
        //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
        DeviceState->StateDescription = "Пауза";
        break;
      case 0x80:
        ////DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "Приём купюры";
        break;
      case 0x81:
        //DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "Купюра уложена";
        break;
      case 0x82:
        //DeviceState->OutStateCode = DSE_OK;
        DeviceState->StateDescription = "Возвращена";
        break;
    }
    //DeviceState->StateDescription = DeviceState->StateDescription;
  }
  return DeviceState->StateDescription;
}

std::string TCCNETDeviceThread::GetMainFailureDescription(BYTE StatusCode)
{
    switch (StatusCode)
    {
      case 0x60:  //DeviceState->SubStateDescription = "Insertion_error";
          DeviceState->SetOutCodes(VLD_INSERTION_ERROR,VLD_INSERTION_ERROR);
          break;
      case 0x61:  //DeviceState->SubStateDescription = "Dielectric_error";
          DeviceState->SetOutCodes(VLD_DIELECTRIC_ERROR,VLD_DIELECTRIC_ERROR);
          break;
      case 0x62:  //DeviceState->SubStateDescription = "Previously_inserted_bill_remains_in_head";
          DeviceState->SetOutCodes(VLD_BILL_IN_HEAD,VLD_BILL_IN_HEAD);
          break;
      case 0x63:  //DeviceState->SubStateDescription = "Compensation__factor_error";
          DeviceState->SetOutCodes(VLD_COMPENSATION_FACTOR_ERROR,VLD_COMPENSATION_FACTOR_ERROR);
          break;
      case 0x64:  //DeviceState->SubStateDescription = "Bill_transport_error";
          DeviceState->SetOutCodes(VLD_BILL_TRANSPORT_ERROR,VLD_BILL_TRANSPORT_ERROR);
          break;
      case 0x65:  //DeviceState->SubStateDescription = "Identification_error";
          DeviceState->SetOutCodes(VLD_IDENTIFICATION_ERROR,VLD_IDENTIFICATION_ERROR);
          break;
      case 0x66:  //DeviceState->SubStateDescription = "Verification_error";
          DeviceState->SetOutCodes(VLD_VERIFICATION_ERROR,VLD_VERIFICATION_ERROR);
          break;
      case 0x67:  //DeviceState->SubStateDescription = "Optic_sensor_error";
          DeviceState->SetOutCodes(VLD_OPTIC_SENSOR_ERROR,VLD_OPTIC_SENSOR_ERROR);
          break;
      case 0x68:  //DeviceState->SubStateDescription = "Return_by_inhibit_error";
          DeviceState->SetOutCodes(VLD_RETURN_BY_INHIBIT_ERROR,VLD_RETURN_BY_INHIBIT_ERROR);
          break;
      case 0x69:  //DeviceState->SubStateDescription = "Capacistance_error";
          DeviceState->SetOutCodes(VLD_CAPACISTANCE_ERROR,VLD_CAPACISTANCE_ERROR);
          break;
      case 0x6A:  //DeviceState->SubStateDescription = "Operation_error";
          DeviceState->SetOutCodes(VLD_OPERATION_ERROR,VLD_OPERATION_ERROR);
          break;
      case 0x6C:  //DeviceState->SubStateDescription = "Length_error";
          DeviceState->SetOutCodes(VLD_LENGTH_ERROR,VLD_LENGTH_ERROR);
          break;
      default : //DeviceState->SubStateDescription = "Unknown_error";
          //DeviceState->SetOutCodes(,);
          break;
    }

    if (lang == 0)
    switch (StatusCode)
    {
      case 0x60:  DeviceState->SubStateDescription = "Insertion_error";break;
      case 0x61:  DeviceState->SubStateDescription = "Dielectric_error";break;
      case 0x62:  DeviceState->SubStateDescription = "Previously_inserted_bill_remains_in_head";break;
      case 0x63:  DeviceState->SubStateDescription = "Compensation__factor_error";break;
      case 0x64:  DeviceState->SubStateDescription = "Bill_transport_error";break;
      case 0x65:  DeviceState->SubStateDescription = "Identification_error";break;
      case 0x66:  DeviceState->SubStateDescription = "Verification_error";break;
      case 0x67:  DeviceState->SubStateDescription = "Optic_sensor_error";break;
      case 0x68:  DeviceState->SubStateDescription = "Return_by_inhibit_error";break;
      case 0x69:  DeviceState->SubStateDescription = "Capacistance_error";break;
      case 0x6A:  DeviceState->SubStateDescription = "Operation_error";break;
      case 0x6C:  DeviceState->SubStateDescription = "Length_error";break;
      default : DeviceState->SubStateDescription = "Unknown_error";break;
    }
    if (lang == 1)
    switch (StatusCode)
    {
      case 0x60:  DeviceState->SubStateDescription = "Insertion_error";break;
      case 0x61:  DeviceState->SubStateDescription = "Dielectric_error";break;
      case 0x62:  DeviceState->SubStateDescription = "Previously_inserted_bill_remains_in_head";break;
      case 0x63:  DeviceState->SubStateDescription = "Compensation__factor_error";break;
      case 0x64:  DeviceState->SubStateDescription = "Bill_transport_error";break;
      case 0x65:  DeviceState->SubStateDescription = "Identification_error";break;
      case 0x66:  DeviceState->SubStateDescription = "Verification_error";break;
      case 0x67:  DeviceState->SubStateDescription = "Optic_sensor_error";break;
      case 0x68:  DeviceState->SubStateDescription = "Return_by_inhibit_error";break;
      case 0x69:  DeviceState->SubStateDescription = "Capacistance_error";break;
      case 0x6A:  DeviceState->SubStateDescription = "Operation_error";break;
      case 0x6C:  DeviceState->SubStateDescription = "Length_error";break;
      default : DeviceState->SubStateDescription = "Unknown_error";break;
    }
    return DeviceState->SubStateDescription;
}

std::string TCCNETDeviceThread::GetHardwareFailureDescription(BYTE StatusCode)
{
    switch (StatusCode)
    {
        case 0x50:  //DeviceState->SubStateDescription = "Stack_motor_falure";
            DeviceState->SetOutCodes(VLD_STACK_MOTOR_FAILURE,VLD_STACK_MOTOR_FAILURE);
            break;
        case 0x51:  //DeviceState->SubStateDescription = "Transport_speed_motor_falure";
            DeviceState->SetOutCodes(VLD_TRANSPORT_SPEED_MOTOR_FAILURE,VLD_TRANSPORT_SPEED_MOTOR_FAILURE);
            break;
        case 0x52:  //DeviceState->SubStateDescription = "Transport-motor_falure";
            DeviceState->SetOutCodes(VLD_TRANSPORT_MOTOR_FAILURE,VLD_TRANSPORT_MOTOR_FAILURE);
            break;
        case 0x53:  //DeviceState->SubStateDescription = "Aligning_motor_falure";
            DeviceState->SetOutCodes(VLD_ALIGNING_MOTOR_FAILURE,VLD_ALIGNING_MOTOR_FAILURE);
            break;
        case 0x54:  //DeviceState->SubStateDescription = "Initial_cassete_falure";
            DeviceState->SetOutCodes(VLD_CASSETTE_FAILURE,VLD_CASSETTE_FAILURE);
            break;
        case 0x55:  //DeviceState->SubStateDescription = "Optical_canal_falure";
            DeviceState->SetOutCodes(VLD_OPTICAL_CANAL_FAILURE,VLD_OPTICAL_CANAL_FAILURE);
            break;
        case 0x56:  //DeviceState->SubStateDescription = "Magnetical_canal_falure";
            DeviceState->SetOutCodes(VLD_MAGNETICAL_CANAL_FAILURE,VLD_MAGNETICAL_CANAL_FAILURE);
            break;
        case 0x5F:  //DeviceState->SubStateDescription = "Capacitance_canal_falure";
            DeviceState->SetOutCodes(VLD_CAPACITANCE_CANAL_FAILURE,VLD_CAPACITANCE_CANAL_FAILURE);
            break;
        default : //DeviceState->SubStateDescription = "Unknown_error";
            //DeviceState->SetOutCodes(,);
            break;
    }

    switch (StatusCode)
    {
        case 0x50:  DeviceState->SubStateDescription = "Stack_motor_falure";break;
        case 0x51:  DeviceState->SubStateDescription = "Transport_speed_motor_falure";break;
        case 0x52:  DeviceState->SubStateDescription = "Transport-motor_falure";break;
        case 0x53:  DeviceState->SubStateDescription = "Aligning_motor_falure";break;
        case 0x54:  DeviceState->SubStateDescription = "Initial_cassete_falure";break;
        case 0x55:  DeviceState->SubStateDescription = "Optical_canal_falure";break;
        case 0x56:  DeviceState->SubStateDescription = "Magnetical_canal_falure";break;
        case 0x5F:  DeviceState->SubStateDescription = "Capacitance_canal_falure";break;
        default : DeviceState->SubStateDescription = "Unknown_error";break;
    }
    //DeviceState->SubStateDescription = DeviceState->SubStateDescription;
    return DeviceState->SubStateDescription;
}

bool TCCNETDeviceThread::IsItYou()
{
    GetStay();
    if (DeviceState->StateCode == 0xFF)
        return false;

    Disable();
    Sleep(100);
    GetStay();
    if (DeviceState->StateCode != 0x19)
        return false;

    return true;
}

void TCCNETDeviceThread::ProcessOutCommand()
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

