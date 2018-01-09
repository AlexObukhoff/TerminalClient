//---------------------------------------------------------------------------
#pragma hdrstop
#include <math.h>
#include "NRIDeviceThread.h"
#include "CCoinAcceptor.h"
#include "globals.h"
#include "boost/format.hpp"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#define POLYNOMINAL 0x08408
#define PollingInterval 200


__fastcall TNRIDeviceThread::TNRIDeviceThread() : TDeviceThread(true)
{
    DataLengthIndex = 1;
    DataLength = 0;
    CRCLength = 1;
    SendType = RecieveAnswer;
    if (Port)
    {
        Port->timeout = 200;
    }

    EquipmentID = "";
    memset(Coins,1,12);
}

__fastcall TNRIDeviceThread::~TNRIDeviceThread()
{
}

unsigned short TNRIDeviceThread::Calc_CRC(BYTE* DataBuf, int count)
{
   BYTE CRC = 0;
   for (int i = 0; i < count; i++)
      CRC += DataBuf[i];
   CRC = (BYTE)(256 - CRC);
   return CRC;
}

void TNRIDeviceThread::SendPacket(int header, BYTE* _data, int datalen)
{
  if (!Port->PortInit)
  {
    DeviceState->StateCode = 0xff;
    return;
  }
  try
  {
    ClearCommand();
    ClearAnswer();

    Command[0] = 2;
    Command[1] = (BYTE)datalen;
    Command[2] = 1;
    Command[3] = (BYTE)header;
    if (_data)
        memcpy(&Command[4],_data,datalen);
    Command[4 + datalen] = Calc_CRC(Command,4 + datalen);
    CommandSize = 5 + datalen;
  }
  __finally
  {
  }
}

int TNRIDeviceThread::PrepareAnswer(BYTE*& Buffer,int count)
{
    if ((count == 0)||(Buffer == NULL))
      return -1;

    //если принятых байт меньше, то выходим
    if (count <= 0 )
      return -1;
    //смотрим, уместился ли байт, где указана длина команды, в буфер
    int _DataLengthIndex = DataLengthIndex + CommandSize;
    if ((count - _DataLengthIndex) <= 0 )
      return -1;
    //определяем длину данной команды
    int answ_len = Buffer[_DataLengthIndex] + 5;
    int datalen = CommandSize + answ_len;

    //определяем, пришла ли вся команда целиком
    if (count < datalen)
      return -1;

    //сдвигаем в буфере данные, если впереди команды пришёл мусор
    ClearBuffer(TempBuffer);
    memcpy((BYTE*)TempBuffer, (BYTE*)&Buffer[CommandSize],answ_len);
    ClearBuffer(Buffer);
    memcpy((BYTE*)Buffer,(BYTE*)TempBuffer,answ_len);

    return answ_len;
}

void TNRIDeviceThread::ParseAnswer()
{
  try
  {
    if (DeviceState)
      AnswerSize = DeviceState->AnswerSize;
    if (AnswerSize == 5)
    {
         ClearBuffer(data);
         AnswerDataSize = 0;
         if ((Log)&&(LoggingErrors))
         {
            Log->Write("Answer is:");
            Log->WriteBuffer(Answer, AnswerSize);
         }
    }
    else
    if (AnswerSize > 5)
    {
         ClearBuffer(data);
         int count = AnswerDataSize = Answer[1];
         memcpy((BYTE*)data,&Answer[4],count);
         if ((Log)&&(LoggingErrors))
         {
            Log->Write("Answer is:");
            Log->WriteBuffer(Answer, AnswerSize);
         }
    }
  }
  __finally
  {
  }
}

int TNRIDeviceThread::SendLowLevelCommand(BYTE* Command, int CommandSize, BYTE*& Answer, int& AnswerSize, TSendType type)
{
  if (!Port->Port) return -1;

  LPVOID lpMsgBuf = NULL;
  try
  {
      TerminateCommand = false;
      COMSTAT cs;//структура, хранящая статус порта
      DWORD BytesProcessed = 0;//число байт в порту
      int BytesCount = 0;
      clock_t Clock,time;//метка времени
      LastError = 0;// текущая ошибка

      // записываем команду в файл
      if (LoggingErrors)
      {
        if ((Log != NULL)&&(LoggingErrors))
        {
          Log->Write("Command:");
          Log->WriteBuffer(Command,CommandSize);
        }
      }

      if ( !WriteFile(Port->Port, Command, CommandSize, &BytesProcessed, NULL))
      {
          FormatMessage(
              FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
              NULL,
              GetLastError(),
              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
              (LPTSTR) &lpMsgBuf,
              0,
              NULL
          );
          if (Log != NULL)
            Log->Write((boost::format("Can't write command in port! %1%") % lpMsgBuf).str().c_str());
          LocalFree(lpMsgBuf);
          return -1;
      }

      if ((type == NotRecieveAnswer)||(Answer == NULL))
          return 0;

      TimeStamp1 = clock();

      ClearBuffer(Answer);
      AnswerSize = 0;
      // очищаем содержимое буфера
      ClearBuffer(MainBuffer);
      ClearBuffer(SecondBuffer);
      ClearBuffer(TempBuffer);


      // засекаем время
      BytesCount = 0;
      DWORD mask;
      SetCommMask(Port->Port,EV_RXCHAR);
      WaitCommEvent(Port->Port,&mask,NULL);
      Clock = clock();

      do
      {
          //если внешний поток прерывает процесс, выходим
          if (Terminated)
          {
            AnswerSize = 0;
            TerminateCommand = false;
            return -1;
          }
          // очищаем буфер
          ClearBuffer(TempBuffer);

          if (!ClearCommBreak(Port->Port))
          {
              FormatMessage(
                  FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL,
                  GetLastError(),
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR) &lpMsgBuf,
                  0,
                  NULL
              );
              if (Log != NULL)
                Log->Write((boost::format("ClearCommBreak() Error! %1%") % lpMsgBuf).str().c_str());
              LocalFree(lpMsgBuf);
              return -1;
          }
          EscapeCommFunction(Port->Port, CLRRTS);

          // сбрасываем ошибки порта
          if ( !ClearCommError(Port->Port, &LastError, &cs) )
          {
              if (Log != NULL)
                Log->Write("Can't reset COM port errors!");
              AnswerSize = 0;
              return -1;
          }
          if ( cs.cbInQue == 0 )//если принято 0 байт, идём дальше
          {
              Sleep(1);
              //SetCommMask(Port->Port,EV_RXCHAR);
              //WaitCommEvent(Port->Port,&mask,NULL);
              continue;
          }

          if ( !ReadFile(Port->Port, TempBuffer, cs.cbInQue, &BytesProcessed, NULL))
          {
              FormatMessage(
                  FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL,
                  GetLastError(),
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR) &lpMsgBuf,
                  0,
                  NULL
              );
              if (Log != NULL)
                Log->Write((boost::format("Can't read answer! %1%") % lpMsgBuf).str().c_str());
              LocalFree(lpMsgBuf);
              return -1;
          }

          //if ((Log)&&(LoggingErrors))
              //Log->Write("Readed bytes count = "+AnsiString(BytesProcessed));
          //добавляем в буфер ответа считанный кусок
          for (int i = BytesCount; (unsigned)i<(BytesCount + BytesProcessed); i++)
            SecondBuffer[i] = TempBuffer[i-BytesCount];
          //длина ответа выросла
          BytesCount += BytesProcessed;

          //разбираем ответ
          //возможно есть мусор в ответе, обрезаем его
          AnswerSize = PrepareAnswer(SecondBuffer,BytesCount);
          //смотрим, полный ли ответ принят
          if (AnswerSize >0 )
          {
            move(MainBuffer,SecondBuffer,AnswerSize);
            move(Answer,SecondBuffer,AnswerSize);
            if (LoggingErrors)
            {
              if (Log != NULL)
              {
                time = clock();
                double delta = ((double)time - (double)Clock)/1000;
                AnsiString mess;
                mess.sprintf("Интервал получения ответа %.4f сек.",delta);
                Log->Write(mess.c_str());
                Log->Write("Received answer:");
                Log->WriteBuffer(MainBuffer, AnswerSize);
              }
            }
            return AnswerSize;
          }
      }
      while ( (clock()-Clock) < (signed)Port->timeout  );

      //правильного ответа в буфере нет, ошибка тайм-аута
      time = clock();
      double delta = ((double)time - (double)Clock)/1000;
      AnsiString mess;
      mess.sprintf("Интервал получения ответа %.4f сек.",delta);
      AnswerSize = 0;
      if ((Log != NULL)&&(LoggingTimeoutErrors))
      {
        Log->Write("Timeout. No answer from device");
        Log->Write((boost::format("Bytes received: %1%, interval %2% sec.") % BytesCount % delta).str().c_str());
        Log->WriteBuffer(SecondBuffer, BytesCount);
      }
      return -1;
  }
  __finally
  {
  }
  return AnswerSize;
}

void TNRIDeviceThread::Poll()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            return;
        }

        SendPacket(254);
        ProcessCommand();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TNRIDeviceThread::GetID()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            return;
        }

        SendPacket(245);
        ProcessCommand();

        EquipmentID = "";
        EquipmentID = AnsiString((char*)data);
        if (Log)
            Log->Write((boost::format("EquipmentID = %1%") % EquipmentID.c_str()).str().c_str());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TNRIDeviceThread::GetStatus()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            return;
        }

        SendPacket(248);
        ProcessCommand();

        DeviceState->StateCode = int(data[1]);
        switch (DeviceState->StateCode)
        {
            case 0:
                DeviceState->StateDescription = "OK (Operating)";
                break;

            case 1:
                DeviceState->StateDescription = "Flight Deck open (return lever pressed)";
                break;
        }
        if (DeviceState->StateCode != DeviceState->OldStateCode)
        {
            if (Log)
                Log->Write((boost::format("Status = %1%") % DeviceState->StateCode).str().c_str());
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

int TNRIDeviceThread::ReadLastCreditOrError()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            return 0;
        }

        //SendPacket(235);
        SendPacket(229);
        ProcessCommand();

        memset(EventA,0,5);
        memset(EventB,0,5);
        EventCounter, NewEvents = 0;
        EventCounter = int(data[0]);
        if (EventCounter > 0)
        {
            if (EventCounter < LastEventCounter)
            {
                NewEvents = (BYTE)((255 - LastEventCounter) + EventCounter);
            }
            else
            {
                NewEvents = (BYTE)(EventCounter - LastEventCounter);
            }
            LastEventCounter = EventCounter;

            if ((Log)&&(LoggingErrors)&&(NewEvents > 0))
                Log->Write((boost::format("NewEvents = %1%") % NewEvents).str().c_str());

            if (NewEvents > 5)
            {
                if(Log) Log->Write((boost::format("Error occured, there are the lost events. Lost events count = %1%") % (NewEvents-5)).str().c_str());
                NewEvents = 5;
            }

            int position = 1;
            for(int i=0; i<NewEvents; i++)
            {
                EventA[i] = data[position]; position++;
                EventB[i] = data[position]; position++;
            }
        }
        else
        {
            if (LastEventCounter > 0)
                if(Log) Log->Write("Error occured, there are power fail. Some events may be lost.");
            LastEventCounter = 0;
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }

    return NewEvents;
}

int TNRIDeviceThread::PerformSelfCheck()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            return 0;
        }

        SendPacket(232);
        ProcessCommand();

        DeviceState->StateCode = int(data[0]);
        if (AnswerDataSize == 2)
            DeviceState->SubStateCode = int(data[1]);
        else
            DeviceState->SubStateCode = 0;

        GetFaultDescription(DeviceState->StateCode, DeviceState->SubStateCode);
        if (DeviceState->StateCode != 0)
        {
            if(DeviceState->StateCode != DeviceState->OldStateCode)
            {
                Log->Write((boost::format("Self Check result = %1%") % DeviceState->StateDescription.c_str()).str().c_str());
                ChangeDeviceState();
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return DeviceState->StateCode;
}

void TNRIDeviceThread::ModifyInhibitStatus()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            return;
        }

        BYTE Byte1 = 0, Byte2 = 0;
        for(int i=0; i<8; i++)
        {
            int value = 1;
            value  <<= i;
            if (Coins[i] == 1)
                Byte1 |= (BYTE)value;//set bit #i
        }

        for(int i=8; i<11; i++)
        {
            int value = 1;
            value  <<= i-8;
            if (Coins[i] == 1)
                Byte2 |= (BYTE)value;//set bit #i
        }

        BYTE CoinsInhibit[2];
        CoinsInhibit[0] = Byte1;
        CoinsInhibit[1] = Byte2;

        SendPacket(231, CoinsInhibit, 2);
        ProcessCommand();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TNRIDeviceThread::Enable()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            return;
        }

        BYTE CoinsInhibit[2];
        CoinsInhibit[0] = 0x01;

        SendPacket(228, CoinsInhibit, 1);
        ProcessCommand();
        Log->Write("Command Enable");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TNRIDeviceThread::Disable()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            return;
        }

        BYTE CoinsInhibit[2];
        CoinsInhibit[0] = 0x00;

        SendPacket(228, CoinsInhibit, 1);
        ProcessCommand();
        Log->Write("Command Disable");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TNRIDeviceThread::getCoinNominals()
{
    if(!Port->PortInit)
    {
        DeviceState->StateCode = 0xFF;
        return;
    }

    Log->Write("getCoinsNominal");
    memset(coinsNominals, 1, sizeof(coinsNominals));
    try
    {
        for(int i = 1; i <= sizeof(coinsNominals) / sizeof(int); ++i)
        {
            BYTE cmd[1];
            cmd[0] = i;
            SendPacket(184, cmd, sizeof(cmd));
            ProcessCommand();
            ParseAnswer();
            if(AnswerDataSize == 6)
            {
                std::string nom = (boost::format("%1%%2%%3%") % data[2] % data[3] % data[4]).str();
                int nominal = atoi(nom.c_str());
                if(nominal < 10)
                {
                    // проверим второй символ
                    if(data[3] == 'K')
                        nominal *= 1000;
                }

                coinsNominals[i - 1] = nominal;
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    Log->Write("done");
}

void __fastcall TNRIDeviceThread::ProcessLoopCommand()
{
  ThreadTerminated = false;
  try
  {
    if (Terminated) return;

    Poll();
    GetID();
    PerformSelfCheck();
    Log->Write((boost::format("Self Check result = %1%") % DeviceState->StateDescription.c_str()).str().c_str());
    if (DeviceState->StateCode == 0)
        SetInitialized();
    ModifyInhibitStatus();
    ReadLastCreditOrError();
    getCoinNominals();
//    Disable();
    GetStatus();
    switch (DeviceState->StateCode)
    {
        case 0:
            DeviceState->StateDescription = "OK (Operating)";
            if (LoggingErrors)
                Log->Write(DeviceState->StateDescription.c_str());
            break;

        case 1:
            DeviceState->StateDescription = "Flight Deck open (return lever pressed)";
            if (LoggingErrors)
                Log->Write(DeviceState->StateDescription.c_str());
            break;
    }

    if (Terminated) return;

    if (DeviceState->AnswerSize < 5)
    {
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        if (Log) Log->Write("No NRI Device Present...");
        DeviceState->StateChange = true;
        if (DeviceStateChanged)
          DeviceStateChanged(DeviceState);
        DeviceState->OldOutStateCode = DeviceState->OutStateCode;
    }

    LoopOfCommands();
  }
  __finally
  {
  }
}

/*
int TNRIDeviceThread::GetCoinNominal(int id)
{
    int result = 0;
    switch (id)
    {
        case 1:
            result = 1;
        break;

        case 2:
            result = 2;
        break;

        case 3:
            result = 5;
        break;

        case 4:
            result = 10;
        break;
    }
    return result;
}
*/
std::string TNRIDeviceThread::GetStateDescription(int code)
{
    switch (code)
    {
        case err_NullEvent:
            DeviceState->StateDescription = "Null Event";
        break;
        case err_RejectCoin:
            DeviceState->StateDescription = "Reject Coin";
        break;
        case err_InhibitedCoin:
            DeviceState->StateDescription = "Inhibited Coin";
        break;
        case err_MultipleWindow:
            DeviceState->StateDescription = "Multiple Window";
        break;
        case err_WakeUpTimeOut:
            DeviceState->StateDescription = "Wake Up TimeOut";
        break;
        case err_ValidationTimeOut:
            DeviceState->StateDescription = "Validation TimeOut";
        break;
        case err_CreditSensorTimeOut:
            DeviceState->StateDescription = "Credit Sensor TimeOut";
        break;
        case err_SorterOptoTimeOut:
            DeviceState->StateDescription = "Sorter Opto TimeOut";
        break;
        case err_SorterNotReady:
            DeviceState->StateDescription = "Sorter Not Ready";
        break;
        case err_SecondCloseCoinError:
            DeviceState->StateDescription = "Second Close Coin Error";
        break;
        case err_AcceptGateNotReady:
            DeviceState->StateDescription = "Accept Gate Not Ready";
        break;
        case err_CreditSensorNotReady:
            DeviceState->StateDescription = "Credit Sensor Not Ready";
        break;
        case err_RejectedCoinNotCleared:
            DeviceState->StateDescription = "Rejected Coin Not Cleared";
        break;
        case err_ValidationSensorNotReady:
            DeviceState->StateDescription = "Validation Sensor Not Ready";
        break;
        case err_CreditSensorBlocked:
            DeviceState->StateDescription = "Credit Sensor Blocked";
        break;
        case err_SorterOptoBlocked:
            DeviceState->StateDescription = "Sorter Opto Blocked";
        break;
        case err_CreditSequenceError:
            DeviceState->StateDescription = "Credit Sequence Error";
        break;
        case err_CoinGoingBackwards:
            DeviceState->StateDescription = "Coin Going Backwards";
        break;
        case err_CoinTooFast:
            DeviceState->StateDescription = "Coin Too Fast";
        break;
        case err_CoinTooSlow:
            DeviceState->StateDescription = "Coin Too Slow";
        break;
        case err_COSMechanismActivated:
            DeviceState->StateDescription = "COS Mechanism Activated";
        break;
        case err_DSEOptoTimeOut:
            DeviceState->StateDescription = "DSE Opto TimeOut";
        break;
        case err_DCEOptoNotSeen:
            DeviceState->StateDescription = "DCE Opto Not Seen";
        break;
        case err_CreditSensorReachedTooEarly:
            DeviceState->StateDescription = "Credit Sensor Reached Too Early";
        break;
        case err_RejectCoinAlt:
            DeviceState->StateDescription = "Reject Coin Alt";
        break;
        case err_RejectSlug:
            DeviceState->StateDescription = "Reject Slug";
        break;
        case err_RejectSensorBlocked:
            DeviceState->StateDescription = "Reject Sensor Blocked";
        break;
        case err_GamesOverload:
            DeviceState->StateDescription = "Games Overload";
        break;
        case err_MaxCoinMeterPulsesExceeded:
            DeviceState->StateDescription = "Max Coin Meter Pulses Exceeded";
        break;
        case err_AcceptGateOpenNotClosed:
            DeviceState->StateDescription = "Accept Gate Open Not Closed";
        break;
        case err_AcceptGateClosedNotOpen:
            DeviceState->StateDescription = "Accept Gate Closed Not Open";
        break;
        case err_DataBlockRequest:
            DeviceState->StateDescription = "Data Block Request";
        break;
        case err_CoinReturnMechanismActivated:
            DeviceState->StateDescription = "Coin Return Mechanism Activated";
        break;
        case err_UnspecifiedAlarmCode:
            DeviceState->StateDescription = "Unspecified Alarm Code";
        break;

        default:
            DeviceState->StateDescription = "Unspecified Alarm Code";
    }

    if ((code>=err_InhibitedCoinType1)&&(code<=err_InhibitedCoinType32))
    {
        int CoinTypeNo = code - 127;
        DeviceState->StateDescription = (boost::format("Inhibited Coin Type %1%") % CoinTypeNo).str();
    }

    return DeviceState->StateDescription;
}

void TNRIDeviceThread::PollingLoop()
{
    if (!Port->PortInit)
    {
        DeviceState->StateCode = 0xff;
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        ChangeDeviceState();
        return;
    }

    try
    {
        if (Terminated) return;
        //проверяем состояние устройства
        PerformSelfCheck();

        if (Terminated) return;
        //обрабатываем внешние команды во внутреннем цикле потока
        ProcessOutCommand();

        if (Terminated) return;

        DeviceState->Billing = false;
        //опрашиваем монетоприёмник на предмет новых событий
        ReadLastCreditOrError();
        //проходим по списку новых событий
        for(int i=0; i<NewEvents; i++)
        {
            if (EventA[i] > 0)
            {//приняли деньги
                CoinID = EventA[i];
                //CoinNominal = GetCoinNominal(CoinID);
                CoinNominal = coinsNominals[CoinID - 1]; // в копейках!!!
                Log->Write((boost::format("Recieved coin with id = %1%; Nominal = %2%; in sorted path = %3%") % CoinID % CoinNominal % EventB[i]).str().c_str());
                DeviceState->Billing = true;
                DeviceState->Nominal = CoinNominal;
                DeviceState->Global += CoinNominal;
                DeviceState->Count++;
            }
            if (EventA[i] == 0)
            {//ошибка или выброс монеты
                Log->Write((boost::format("Rejected coin or error with description = %1%") % GetStateDescription(EventB[i]).c_str()).str().c_str());
                DeviceState->OutStateCode = DSE_BILLREJECT;
            }
        }

        if (Terminated) return;

        ChangeDeviceState();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

bool TNRIDeviceThread::IsItYou()
{
    if (EquipmentID == "Coin Acceptor")
        return true;
    else
        return false;
}

void TNRIDeviceThread::ProcessOutCommand()
{
    try
    {
        BYTE Command = GetExtCommand();
        switch (Command)
        {
          case oc_Disable:
            Log->Write("ExtCommand DisableCoin");
            DeviceState->Idle = true;
            Disable();
            break;

          case oc_Enable:
            Log->Write("ExtCommand EnableCoin");
            DeviceState->Idle = false;
            Enable();
            break;

          /*
          case oc_GetID:
            Log->Write("ExtCommand GetID");
            GetID();
            break;
          */
        }
    }
    __finally
    {
        //ExtCommand = EXTC_Free;
    }
}

std::string TNRIDeviceThread::GetFaultDescription(int code, int subcode)
{
    DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
    switch (code)
    {
        case 0:
            DeviceState->StateDescription = "OK";
            DeviceState->OutStateCode = DSE_OK;
        break;

        case 1:
            DeviceState->StateDescription = "EEPROM checksum corrupted";
        break;

        case 2:
            DeviceState->StateDescription = (boost::format("Fault on inductive coil %1%") % subcode).str();
        break;

        case 3:
            DeviceState->StateDescription = "Fault on credit sensor";
        break;

        case 4:
            DeviceState->StateDescription = "Fault on piezo sensor";
        break;

        case 5:
            DeviceState->StateDescription = "Fault on preflective sensor";
        break;

        case 6:
            DeviceState->StateDescription = "Fault on diameter sensor";
        break;

        case 7:
            DeviceState->StateDescription = "Fault on wake-up sensor";
        break;

        case 8:
            DeviceState->StateDescription = "Fault on sorter exit sensor";
        break;

        case 9:
            DeviceState->StateDescription = "NVRAM checksum error";
        break;

        case 10:
            DeviceState->StateDescription = "Coin dispensing error";
        break;

        case 11:
            DeviceState->StateDescription = "Low level sensor error";
        break;

        case 12:
            DeviceState->StateDescription = "High level sensor error";
        break;

        case 13:
            DeviceState->StateDescription = "Coin counting error";
        break;

        case 14:
            DeviceState->StateDescription = (boost::format("Keypad error with key %1%") % subcode).str();
        break;

        case 15:
            DeviceState->StateDescription = "Button error";
        break;

        case 16:
            DeviceState->StateDescription = "Display error";
        break;

        case 17:
            DeviceState->StateDescription = "Coin auditing error";
        break;

        case 18:
            DeviceState->StateDescription = "Fault on reject sensor";
        break;

        case 19:
            DeviceState->StateDescription = "Fault on coin return mechanism";
        break;

        case 20:
            DeviceState->StateDescription = "Fault on C.O.S. mechanism";
        break;

        case 21:
            DeviceState->StateDescription = "Fault on rim sensor";
        break;

        case 22:
            DeviceState->StateDescription = "Fault on thermistor";
        break;

        case 23:
            DeviceState->StateDescription = "Payout motor fault";
        break;

        case 24:
            DeviceState->StateDescription = "Payout timeout";
        break;

        case 25:
            DeviceState->StateDescription = "Payout jammed";
        break;

        case 26:
            DeviceState->StateDescription = "Payout sensor fault";
        break;

        case 27:
            DeviceState->StateDescription = "Level sensor error";
        break;

        case 28:
            DeviceState->StateDescription = "Personality module not fitted";
        break;

        case 29:
            DeviceState->StateDescription = "Personality checksum corrupted";
        break;

        case 30:
            DeviceState->StateDescription = "ROM checksum corrupted";
        break;

        case 31:
            DeviceState->StateDescription = (boost::format("Missing slave device %1%") % subcode).str();
        break;

        case 32:
            DeviceState->StateDescription = (boost::format("Internal comms bad %1%") % subcode).str();
        break;

        case 33:
            DeviceState->StateDescription = "Supply voltage outside operating limits";
        break;

        case 34:
            DeviceState->StateDescription = "Temperature outside operating limits";
        break;

        case 35:
            DeviceState->StateDescription = "D.C.E. fault";
        break;

        case 36:
            DeviceState->StateDescription = "Fault on bill validation sensor";
        break;

        case 37:
            DeviceState->StateDescription = "Fault on bill transport motor";
        break;

        case 38:
            DeviceState->StateDescription = "Fault on stacker";
        break;

        case 39:
            DeviceState->StateDescription = "Bill jammed";
            DeviceState->OutStateCode = DSE_BILLJAM;
        break;

        case 40:
            DeviceState->StateDescription = "RAM test fail";
        break;

        case 41:
            DeviceState->StateDescription = "Fault on string sensor";
        break;

        case 42:
            DeviceState->StateDescription = "Accept gate failed open";
        break;

        case 43:
            DeviceState->StateDescription = "Accept gate failed closed";
        break;

        case 255:
            DeviceState->StateDescription = "Unspecified fault code";
            DeviceState->OutStateCode = DSE_OK;
        break;

        default:
            DeviceState->StateDescription = "Unspecified fault code";
            DeviceState->OutStateCode = DSE_OK;
        break;
    }
    return DeviceState->StateDescription;
}
