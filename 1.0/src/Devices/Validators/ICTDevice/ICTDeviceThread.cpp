//---------------------------------------------------------------------------


#pragma hdrstop

#include <math.h>
#include "globals.h"
#include "boost/format.hpp"
#include "ICTDeviceThread.h"
#include "CValidator.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

#define PollingInterval 100

__fastcall TICTDeviceThread::TICTDeviceThread() : TDeviceThread(true)
{
    DataLength = 1;
    WriteReadTimeout = 100;
}

__fastcall TICTDeviceThread::~TICTDeviceThread()
{
}

void TICTDeviceThread::SendPacket(BYTE command,int len_packet, BYTE* data)
{
    UNREFERENCED_PARAMETER(data);
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

        Command[0] = command;
        CommandParameters->SetParameters(Command,len_packet,command,0);
        CommandSize = 1;
    }
    __finally
    {
    }
}

void TICTDeviceThread::SendACK()
{
    if (!Port->PortInit)
    {
        DeviceState->StateCode = 0xff;
        return;
    }
    Command[0] = 0x02;
    CommandSize = 1;
    CommandParameters->SetParameters(Command,1,Command[0],0);

    TSendType old = SendType;
    SendType = NotRecieveAnswer;
    SendPacket(Command[0],1,NULL);
    ProcessCommand();
    SendType = old;
}

void TICTDeviceThread::GetStay()//POLL command
{
    if (!Port->PortInit)
    {
        DeviceState->StateCode = 0xff;
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        return;
    }
    Command[0] = 0x0C;
    CommandSize = 1;
    CommandParameters->SetParameters(Command,0,0x0C,0);

    SendType = RecieveAnswer;
    ProcessCommand();
    if (DeviceState->AnswerSize > 0)
    {
        OfflineCount = 0;
    }
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
}

void TICTDeviceThread::Reset()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            return;
        }
        BYTE command=0x30;
        SendPacket(command,1,NULL);

        SendType = NotRecieveAnswer;
        ProcessCommand();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}


std::string TICTDeviceThread::GetStatusDescription(BYTE StatusCode)
{
    switch(StatusCode)
    {
        case 0x00:
            //DeviceState->StateDescription = "ACK";
            break;
        case 0x80:
            //DeviceState->StateDescription = "POWERUP1";
            DeviceState->SetOutCodes(DSE_NOTSET, DSE_NOERROR);
            break;
        case 0x8F:
            //DeviceState->StateDescription = "POWERUP2";
            DeviceState->SetOutCodes(DSE_NOTSET, DSE_NOERROR);
            break;
        case 0x26:
            DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
            DeviceState->SetOutCodes(VLD_COMMUNICATION_ERROR, VLD_COMMUNICATION_ERROR);
            //DeviceState->StateDescription = "COMMUNICERROR";
            break;
        case 0x81:
            DeviceState->SetOutCodes(DSE_NOTSET, DSE_NOERROR);
            //DeviceState->StateDescription = "ESCROW";
            break;
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
        case 0x48:
        case 0x49:
            //DeviceState->StateDescription = "BILLTYPE5";
            DeviceState->SetOutCodes(DSE_NOERROR, VLD_STACKING);
            DeviceState->OutStateCode = DSE_OK;
            break;
        case 0x10:
            //DeviceState->StateDescription = "STACKED";
            DeviceState->SetOutCodes(DSE_NOERROR, VLD_STACKED);
            DeviceState->OutStateCode = DSE_OK;
            break;
        case 0x11:
            //DeviceState->StateDescription = "REJECTING";
            DeviceState->SetOutCodes(VLD_REJECTING, VLD_REJECTING);
            break;
        case 0x20:
            DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
            DeviceState->SetOutCodes(VLD_STACK_MOTOR_FAILURE, VLD_STACK_MOTOR_FAILURE);
            //DeviceState->StateDescription = "STACKMOTORFAILURE";
            break;
        case 0x21:
            //DeviceState->OutStateCode = DSE_MAINERROR;
            DeviceState->SetOutCodes(VLD_CHECKSUM_ERROR, VLD_CHECKSUM_ERROR);
            //DeviceState->StateDescription = "CHECKSUMERROR";
            break;
        case 0x22:
            DeviceState->OutStateCode = DSE_BILLJAM;
            DeviceState->SetOutCodes(VLD_BILL_JAM, VLD_BILL_JAM);
            //DeviceState->StateDescription = "BILLJAM";
            break;
        case 0x23:
            //DeviceState->OutStateCode = DSE_MAINERROR;
            DeviceState->SetOutCodes(VLD_BILL_REMOVE, VLD_BILL_REMOVE);
            //DeviceState->StateDescription = "BILLREMOVE";
            break;
        case 0x24:
            DeviceState->OutStateCode = DSE_STACKEROPEN;
            DeviceState->SetOutCodes(DSE_NOTSET, VLD_STACKER_OPENED);
            //DeviceState->StateDescription = "STACKEROPEN";
            break;
        case 0x25:
            DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
            DeviceState->SetOutCodes(VLD_OPTIC_SENSOR_ERROR, VLD_OPTIC_SENSOR_ERROR);
            //DeviceState->StateDescription = "SENSORPROBLEM";
            break;
        case 0x27:
            DeviceState->SetOutCodes(VLD_OPTIC_SENSOR_ERROR, VLD_OPTIC_SENSOR_ERROR);
            //DeviceState->StateDescription = "BILLFISH";
            break;
        case 0x28:
            DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
            DeviceState->SetOutCodes(VLD_STACKER_PROBLEM, VLD_STACKER_PROBLEM);
            //DeviceState->StateDescription = "STACKERPROBLEM";
            break;
        case 0x29:
            DeviceState->OutStateCode = DSE_BILLREJECT;
            DeviceState->SetOutCodes(VLD_REJECTING, VLD_REJECTING);
            //DeviceState->StateDescription = "BILLREJECT";
            break;
        case 0x2A:
            //DeviceState->StateDescription = "INVALIDCOMMAND";
            DeviceState->SetOutCodes(VLD_INVALID_COMMAND, VLD_INVALID_COMMAND);
            break;
        case 0x2F:
            //DeviceState->OutStateCode = DSE_MAINERROR;
            //DeviceState->StateDescription = "ERRORSTATUSISEXCLUSION";
            DeviceState->SetOutCodes(VLD_ERROR_STATUS_IS_EXCLUSION, VLD_ERROR_STATUS_IS_EXCLUSION);
            break;
        case 0x3E:
            //DeviceState->StateDescription = "ENABLE";
            if (DeviceState->OutStateCode != DSE_BILLJAM)
                DeviceState->OutStateCode = DSE_OK;
            DeviceState->SetOutCodes(DSE_NOTSET, VLD_BILL_ENABLE);
            break;
        case 0x5E:
            if (DeviceState->OutStateCode != DSE_BILLJAM)
                DeviceState->OutStateCode = DSE_OK;
            //DeviceState->StateDescription = "DISABLE";
            DeviceState->SetOutCodes(DSE_NOTSET, VLD_BILL_DISABLE);
            break;
    }


    if (lang == 0)
    //DeviceState->OutStateCode = DSE_OK;
    switch(StatusCode)
    {
        case 0x00:
            DeviceState->StateDescription = "ACK";
            break;
        case 0x80:
            DeviceState->StateDescription = "POWERUP1";
            break;
        case 0x8F:
            DeviceState->StateDescription = "POWERUP2";
            break;
        case 0x26:
            DeviceState->StateDescription = "COMMUNICERROR";
            break;
        case 0x81:
            DeviceState->StateDescription = "ESCROW";
            break;
        case 0x40:
            DeviceState->StateDescription = "BILLTYPE1";
            break;
        case 0x41:
            DeviceState->StateDescription = "BILLTYPE2";
            break;
        case 0x42:
            DeviceState->StateDescription = "BILLTYPE3";
            break;
        case 0x43:
            DeviceState->StateDescription = "BILLTYPE4";
            break;
        case 0x44:
            DeviceState->StateDescription = "BILLTYPE5";
            break;
        case 0x45:
            DeviceState->StateDescription = "BILLTYPE6";
            break;
        case 0x46:
            DeviceState->StateDescription = "BILLTYPE7";
            break;
        case 0x47:
            DeviceState->StateDescription = "BILLTYPE8";
            break;
        case 0x48:
            DeviceState->StateDescription = "BILLTYPE9";
            break;
        case 0x49:
            DeviceState->StateDescription = "BILLTYPE10";
            break;
        case 0x10:
            DeviceState->StateDescription = "STACKED";
            break;
        case 0x11:
            DeviceState->StateDescription = "REJECTING";
            break;
        case 0x20:
            DeviceState->StateDescription = "STACKMOTORFAILURE";
            break;
        case 0x21:
            DeviceState->StateDescription = "CHECKSUMERROR";
            break;
        case 0x22:
            DeviceState->StateDescription = "BILLJAM";
            break;
        case 0x23:
            DeviceState->StateDescription = "BILLREMOVE";
            break;
        case 0x24:
            DeviceState->StateDescription = "STACKEROPEN";
            break;
        case 0x25:
            DeviceState->StateDescription = "SENSORPROBLEM";
            break;
        case 0x27:
            DeviceState->StateDescription = "BILLFISH";
            break;
        case 0x28:
            DeviceState->StateDescription = "STACKERPROBLEM";
            break;
        case 0x29:
            DeviceState->StateDescription = "BILLREJECT";
            break;
        case 0x2A:
            DeviceState->StateDescription = "INVALIDCOMMAND";
            break;
        case 0x2F:
            DeviceState->StateDescription = "ERRORSTATUSISEXCLUSION";
            break;
        case 0x3E:
            DeviceState->StateDescription = "ENABLE";
            break;
        case 0x5E:
            DeviceState->StateDescription = "DISABLE";
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
            DeviceState->StateDescription = "ACK";
            break;
        case 0x80:
            DeviceState->StateDescription = "Включение энергии1";
            break;
        case 0x8F:
            DeviceState->StateDescription = "Включение энергии2";
            break;
        case 0x26:
            DeviceState->StateDescription = "Ошибка связи";
            break;
        case 0x81:
            DeviceState->StateDescription = "Разрешение";
            break;
        case 0x40:
            DeviceState->StateDescription = "Тип купюры 1";
            break;
        case 0x41:
            DeviceState->StateDescription = "Тип купюры 2";
            break;
        case 0x42:
            DeviceState->StateDescription = "Тип купюры 3";
            break;
        case 0x43:
            DeviceState->StateDescription = "Тип купюры 4";
            break;
        case 0x44:
            DeviceState->StateDescription = "Тип купюры 5";
            break;
        case 0x45:
            DeviceState->StateDescription = "Тип купюры 6";
            break;
        case 0x46:
            DeviceState->StateDescription = "Тип купюры 7";
            break;
        case 0x47:
            DeviceState->StateDescription = "Тип купюры 8";
            break;
        case 0x48:
            DeviceState->StateDescription = "Тип купюры 9";
            break;
        case 0x49:
            DeviceState->StateDescription = "Тип купюры 10";
            break;
        case 0x10:
            DeviceState->StateDescription = "Купюра положена";
            break;
        case 0x11:
            DeviceState->StateDescription = "Ошибка приёма купюры";
            break;
        case 0x20:
            DeviceState->StateDescription = "Сбой мотора стекера";
            break;
        case 0x21:
            DeviceState->StateDescription = "Ошибка проверки контрольной суммы";
            break;
        case 0x22:
            DeviceState->StateDescription = "Замятие купюры";
            break;
        case 0x23:
            DeviceState->StateDescription = "Изъятие купюры";
            break;
        case 0x24:
            DeviceState->StateDescription = "Стекер открыт";
            break;
        case 0x25:
            DeviceState->StateDescription = "Сбой сенсора";
            break;
        case 0x27:
            DeviceState->StateDescription = "BILLFISH";
            break;
        case 0x28:
            DeviceState->StateDescription = "Сбой стекера";
            break;
        case 0x29:
            DeviceState->StateDescription = "Выброс купюры";
            break;
        case 0x2A:
            DeviceState->StateDescription = "Неверная команда";
            break;
        case 0x2F:
            DeviceState->StateDescription = "Ошибочный статус";
            break;
        case 0x3E:
            DeviceState->StateDescription = "Доступен";
            break;
        case 0x5E:
            DeviceState->StateDescription = "Недоступен";
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



void TICTDeviceThread::EnableAll()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            return;
        }
        BYTE command=0x3E;
        SendPacket(command,1,NULL);

        SendType = NotRecieveAnswer;
        ProcessCommand();
        Log->Write("Command EnableAll()");
        DeviceState->Idle = false;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TICTDeviceThread::Return()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            return;
        }
        BYTE command=0x0F;
        SendPacket(command,1,NULL);

        SendType = NotRecieveAnswer;
        ProcessCommand();
        //DeviceState->Idle = false;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TICTDeviceThread::Disable()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            return;
        }
        BYTE command=0x5E;
        SendPacket(command,1,NULL);

        SendType = NotRecieveAnswer;
        ProcessCommand();
        Log->Write("Command Disable()");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TICTDeviceThread::PollingLoop()
{
    AnsiString mess;

    if (!Port->PortInit)
    {
        DeviceState->StateCode = 0xff;
        return;
    }

    if (Terminated) return;

    ClearBuffer(Answer);
    AnswerSize = ReadPort(Answer,AnswerSize);
    for(int i=0; i<AnswerSize; i++)
    {
        if (DeviceState->AnswerSize > 1)
            Log->Write((boost::format("ReadPort(size=%1%): Answer[%2%] =") % AnswerSize % i % GetByteExp(Answer[i])).str().c_str());
        if (Terminated) return;
        DeviceState->StateCode = Answer[i];
        CheckState((BYTE)DeviceState->StateCode);
    }

    if (Terminated) return;

    //new 16-03-2007
    ProcessOutCommand();

    if (Terminated) return;
    GetStay();

    for(int i=0; i<DeviceState->AnswerSize; i++)
    {
        if (Terminated) return;
        if (DeviceState->AnswerSize > 1)
            Log->Write((boost::format("GetState(size=%1%): Answer[%2%] = %3%") % DeviceState->AnswerSize % i % GetByteExp(Answer[i])).str().c_str());
        DeviceState->StateCode = Answer[i];
        CheckState((BYTE)DeviceState->StateCode);
    }
}


void TICTDeviceThread::CheckState(BYTE code)
{
    int error=0;
    AnsiString mess;

//    int State = code;

    if (Terminated) return;

    if ((DeviceState->OldStateCode == code)&&(EnterLoop == false)||(code == 0))
        return;

    EnterLoop = false;
    DeviceState->Billing = false;
    DeviceState->Enabling = false;

    switch (code)
    {
          case ENABLE: //Ожидание
            if (DeviceState->Stacked)
              DeviceState->Processing = false;
            DeviceState->Stacked = false;
            DeviceState->Enabling = true;
            DeviceState->Idle = false;
            //if (!DeviceState->BillEnable)
            if ((DeviceState->BillEnable == false)&&(DeviceState->Stacking == false))
              Disable();
            break;

          case DISABLE: //недоступен
            SetInitialized();
            DeviceState->Idle = true;
            if (Terminated)
              return;
            if (DeviceState->BillEnable)
            {
              if (Log)
                Log->Write("From DISABLE: Command EnableAll()");
              EnableAll();
            }
            //засекаем когда _DISABLE
            /*if (InitDisable)
                DisableTime = clock();
            InitDisable = false;
            if ((clock()-DisableTime)>10000)
            {
                if (DeviceState->OutStateCode != DSE_OK)
                {
                   DeviceState->OutStateCode = DSE_OK;
                   Log->Write("OldOutStateCode="+AnsiString(DeviceState->OldOutStateCode)+"; OutStateCode="+AnsiString(DeviceState->OutStateCode));
                   DeviceState->StateChange = true;
                    if (DeviceStateChanged)
                      DeviceStateChanged(DeviceState);
                    else
                      Log->Write("TIMER IS NULL!");
                }
                DeviceState->OldOutStateCode = DeviceState->OutStateCode;
            }*/
            break;

          case POWERUP1:
            SendACK();
            break;
          case POWERUP2:
            SendACK();
            if (Terminated) return;
            Reset();
            if (Terminated) return;
            if (!DeviceState->BillEnable)
              Disable();
           break;

          case STACKEROPEN: // Вынули кассету
            DeviceState->OutStateCode = DSE_STACKEROPEN;
            break;

          case BILLJAM:
            DeviceState->OutStateCode = DSE_BILLJAM;
            count_inkass = 0;
            if (Terminated) return;
            if (error == 0)
              Reset();
            error=1;
            break;

          case BILLACCEPTFAIL:
              DeviceState->Billing = false;
              mess.sprintf("Bill accept failed nominal = %.2f",bill_nominal);
              if (Log) Log->Write(mess.c_str());
              break;

          case BILLREJECT:
              DeviceState->Stacking = false;
              DeviceState->Billing = false;
              mess.sprintf("Rejected nominal = %.2f",bill_nominal);
              if (Log) Log->Write(mess.c_str());
              break;

          //case 0x00:
          case 0xFF:
            //DeviceState->OutStateCode = DSE_NOTMOUNT;
            break;


          case ESCROW://Поместили купюру в условное депонирование
            DeviceState->Processing = true;
            DeviceState->Stacking = true;
            DeviceState->Idle = false;
            //if (Log) Log->Write("Escrowing");
            //DeviceState->OutStateCode = DSE_OK;
            //error = 0;
            break;

          case COMMUNICERROR:
          case STACKMOTORFAILURE:
          case SENSORPROBLEM:
          case STACKERPROBLEM:
          case ERRORSTATUSISEXCLUSION:
            //DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
            error = 1;
            break;

          case BILLTYPE1:
          case BILLTYPE2:
          case BILLTYPE3:
          case BILLTYPE4:
          case BILLTYPE5:
          case BILLTYPE6:
          case BILLTYPE7:
          {
             //DeviceState->OutStateCode = DSE_OK;
             DeviceState->Idle = false;
             DeviceState->Stacking = true;
             if (Currency.LowerCase() == "rur")
             {
                 BillDescr = " RUR";
                 Log->Write(BillDescr.c_str());
                 switch (DeviceState->StateCode)
                 {
                    case 0x40 ://0x63
                         bill_nominal = 10;break;
                    case 0x41://0x64
                         bill_nominal = 50;break;
                    case 0x42://0x65
                         bill_nominal = 100;break;
                    case 0x43://0x66
                         bill_nominal = 500;break;
                    case 0x44://0x67
                         bill_nominal = 1000;break;
                    default:
                         bill_nominal = 0;break;
                 }
             }
             if (Currency.LowerCase() == "kzt")
             {
                 BillDescr = " KZT";
                 Log->Write(BillDescr.c_str());
                 switch (DeviceState->StateCode)
                 {
                    case 0x41 :
                         bill_nominal = 200;break;
                    case 0x42:
                         bill_nominal = 500;break;
                    case 0x43:
                         bill_nominal = 1000;break;
                    case 0x44:
                         bill_nominal = 2000;break;
                    case 0x45:
                         bill_nominal = 5000;break;
                    case 0x46:
                         bill_nominal = 10000;break;
                    default:
                         bill_nominal = 0;break;
                 }
             }
             /*if (Currency.LowerCase() == "uah1")
             {
                 BillDescr = " UAH";
                 Log->Write(BillDescr+"1");
                 switch (DeviceState->StateCode)
                 {
                    case 0x40 :
                         bill_nominal = 5;break;
                    case 0x41:
                         bill_nominal = 10;break;
                    case 0x42:
                         bill_nominal = 20;break;
                    case 0x43:
                         bill_nominal = 50;break;
                    case 0x44:
                         bill_nominal = 100;break;
                    case 0x45:
                         bill_nominal = 200;break;
                    default:
                         bill_nominal = 0;break;
                 }
             }*/
             if (Currency.LowerCase() == "uah")
             {
                 BillDescr = " UAH";
                 Log->Write(BillDescr.c_str());
                 switch (DeviceState->StateCode)
                 {
                    case 0x40 :
                         bill_nominal = 1;break;
                    case 0x41:
                         bill_nominal = 2;break;
                    case 0x42:
                         bill_nominal = 5;break;
                    case 0x43:
                         bill_nominal = 10;break;
                    case 0x44:
                         bill_nominal = 20;break;
                    case 0x45:
                         bill_nominal = 50;break;
                    default:
                         bill_nominal = 0;break;
                 }
             }

            DeviceState->Nominal = bill_nominal;
            if (Log) Log->Write((boost::format("Escrowing nominal=%1%") % bill_nominal).str().c_str());

            //new
            if (this->bill_nominal == 0)
            {
                mess.sprintf("Returning nominal = %.2f",bill_nominal);
                Return();
                if (Log) Log->Write(mess.c_str());
            }
            else
            if ((this->bill_nominal > MaxCash)||(this->bill_nominal < MinCash))
            {
                DeviceState->Billing = false;
                mess.sprintf("Returning nominal = %.2f",bill_nominal);
                Return();
                if (Log) Log->Write(mess.c_str());
            }
            else
            {
                if (IsNominalEnabled(this->bill_nominal)&&(this->bill_nominal != 0))
                {
                  SendACK();
                  DeviceState->Billing = false;
                  DeviceState->Stacking = true;
                  mess.sprintf("Stacking nominal = %.2f",bill_nominal);
                  if (Log) Log->Write(mess.c_str());
                  /*
                  if (Log) Log->Write(mess);
                  DeviceState->OutStateCode = DSE_OK;
                  DeviceState->Billing = true;
                  mess.sprintf("Stacked nominal = %.2f",bill_nominal);
                  bill_count += bill_nominal;
                  DeviceState->Global += bill_nominal;
                  bill_numb++;
                  DeviceState->Count++;
                  */
                }
                else
                {
                  DeviceState->Billing = false;
                  DeviceState->Stacking = false;
                  mess.sprintf("Returning nominal = %.2f",bill_nominal);
                  Return();
                  if (Log) Log->Write(mess.c_str());
                }
            }
          }
          break;

          case STACKED:
              DeviceState->Billing = false;
              DeviceState->OutStateCode = DSE_OK;
              if (DeviceState->Stacking == false)
              {
                  if (Log) Log->Write("State STACKED arose without stacking! Incorrect.");
                  return;
              }
              if (Log) Log->Write("Nominal stacked.");
              DeviceState->Stacking = false;
              DeviceState->Billing = true;
              DeviceState->Stacked = true;
              mess.sprintf("Stacked nominal = %.2f",bill_nominal);
              bill_count += bill_nominal;
              DeviceState->Global += bill_nominal;
              bill_numb++;
              DeviceState->Count++;
              if (Log) Log->Write(mess.c_str());
              //DeviceState->StateChange = true;
              //if (DeviceStateChanged)
                //DeviceStateChanged(DeviceState);
              break;

          default:
            break;
    }

    if (DeviceState->OldStateCode == STACKEROPEN)
      DeviceState->OutStateCode = DSE_OK;
    Log->Write((boost::format("StateCode =%1% %2%; OldStateCode =%3%") % GetByteExp(code) % GetStatusDescription(code) % GetByteExp((BYTE)DeviceState->OldStateCode)).str().c_str());
    DeviceState->OldStateCode = code;
    ChangeDeviceState(false);
}

void TICTDeviceThread::ParseAnswer(int mode)
{
    ClearBuffer(data);
    *len_data = 0;
    if (DeviceState)
    {
        AnswerSize = DeviceState->AnswerSize;
        DeviceState->SubStateCode = 0;
        DeviceState->StateCode = Answer[0];
    }
}

std::string TICTDeviceThread::GetStateDescription()
{
    return GetStatusDescription((BYTE)DeviceState->StateCode);
}

bool TICTDeviceThread::IsItYou()
{
    GetStay();
    if (DeviceState->StateCode == 0xFF)
        return false;

    Disable();
    GetStay();
    if (DeviceState->StateCode != DISABLE)
        return false;

    return true;
}

void __fastcall TICTDeviceThread::ProcessLoopCommand()
{
    ThreadTerminated = false;
    if (Terminated) return;
    Log->Write("ProcessLoopCommand()");
    SendACK();
    Reset();
    Disable();
    GetStay();
    if (Terminated) return;
    if (DeviceState->StateCode != 0xff && DeviceState->StateCode != 0x00)
    {
        DeviceState->OutStateCode = DSE_OK;
        if (Log) Log->Write("ICT Device has found.");
        Reset();
        ChangeDeviceState();
    }
    else
    {
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        if (Log != NULL)
          if (Log) Log->Write("No ICT Device Present...");
        /*DeviceState->StateChange = true;
        if (DeviceStateChanged)
          DeviceStateChanged(DeviceState);
        DeviceState->OldOutStateCode = DeviceState->OutStateCode;*/
        ChangeDeviceState();
    }

    LoopOfCommands();
}

bool TICTDeviceThread::ChangeDeviceState(bool wait)
{
    if (((DeviceState)&&(DeviceState->OldOutStateCode != DeviceState->OutStateCode))||(DeviceState->Billing))
    {
        DeviceState->StateChange = true;
        if ((DeviceState->OldOutStateCode == DeviceState->OutStateCode)&&(DeviceState->Billing))
            DeviceState->StateChange = false;
        Log->Write((boost::format("OutStateCode=%1%; OldOutStateCode=%2%") % DeviceState->OutStateCode % DeviceState->OldOutStateCode).str().c_str());
        DeviceState->Done = false;
        if (DeviceStateChanged)
            DeviceStateChanged(DeviceState);
        DeviceState->OldOutStateCode = DeviceState->OutStateCode;
        int interval = 10;
        DWORD SleepTime = 10*1000;
        if (wait)
            while((DeviceState)&&(DeviceState->Done == false))
            {
                //Application->ProcessMessages();
                Sleep(interval);
                SleepTime -= interval;
                if (SleepTime <= 0)
                    break;
            }
        return true;
    }
    return false;
}

void TICTDeviceThread::ProcessOutCommand()
{
    try
    {
        BYTE Command = GetExtCommand();
        switch (Command)
        {
          case oc_DisableBill:
            if (DeviceState->Stacking == true)
            {
                if (Log) Log->Write("Can not perform command DisableBill while stacking!");
                SetExtCommand(oc_DisableBill);
            }
            else
            {
                if (Log) Log->Write("Perform external command DisableBill");
                Disable();
            }
            break;
          case oc_EnableBill:
            if (Log) Log->Write("Perform external command EnableBill");
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

