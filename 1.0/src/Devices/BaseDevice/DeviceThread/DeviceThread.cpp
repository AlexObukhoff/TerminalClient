//---------------------------------------------------------------------------
#include <vcl.h>
#include <math.h>
#pragma hdrstop

#include "DeviceThread.h"
#include "boost/format.hpp"
#pragma package(smart_init)

#define SleepInterval 100
#define PollingInterval 100

__fastcall TDeviceThread::TDeviceThread(bool CreateSuspended, bool _PollingMode, DWORD _LifeTime) : TThread(CreateSuspended)
{
  ThreadTerminated = CreateSuspended;
  FreeOnTerminate = false;
  //поток будет выполнять одиночную команду и завершаться
  PollingMode = _PollingMode;
  //FreeOnTerminate = true;
  _PollingInterval = 100;

  DeviceStarted = NULL;
  DeviceStopped = NULL;
  DevicePaused = NULL;
  DeviceStateChanged = NULL;
  CommandStarted = NULL;
  CommandPaused = NULL;
  CommandFinished = NULL;

  Answer = NULL;
  AnswerSize = 0;
  Command = NULL;
  CommandSize = 0;

  LoggingErrors = true;
  LoggingTimeoutErrors = false;
  LastError = 0;
  DataLengthIndex = -1;
  DataLength = -1;
  TerminateCommand = false;
  ChangeEvent = NULL;
  Sending = true;
  OnlyPnP = false;

  CreationTime = clock();
  LifeTime = _LifeTime;

  EnterLoop = true;
  ExchangeRate = 1;
  MaxCash = 0;
  MinCash = 1;
  bill_nominal = 0;
  count_inkass = 0;
  bill_numb = 0;
  Currency = "rur";
  bill_count = 0;
  bill_count_old = 0;
  WriteReadTimeout = 0;
  CleanPortBeforeReading = false;
  ExtCommand = EXTC_Free;
  Initialized = false;
  TimeStamp1 = clock();
  TimeStamp2 = clock();
  BillsSensitivity = 0;
  BillsInhibit = 0;
}

void __fastcall TDeviceThread::ProcessCommand(int mode)
{
  DeviceState->State = CommandExecuting;
  try
  {
    //if (PollingMode)
    SendCommand(Command,CommandSize,Answer,AnswerSize,SendType, mode);
    //else
        //SendSingleCommand(Command,CommandSize,Answer,AnswerSize,SendType);
  }
  __finally
  {
    DeviceState->State = Wait;
    Sending = false;
  }
}

void __fastcall TDeviceThread::Execute()
{
  Sending = true;
  //отправляем одиночную команду, или постоянно опрашиваем устройство
  if (!PollingMode)
    ProcessCommand();
  else
    ProcessLoopCommand();
}

void TDeviceThread::LoopOfCommands()
{
    while(!Terminated)
    {
        if (Terminated) return;
        clock_t BeginTime = clock();
        PollingLoop();
        clock_t EndTime = clock();
        long delta = 0;
        delta = PollingInterval - (EndTime - BeginTime)/CLK_TCK * 1000;
        if (delta > PollingInterval)
        {
            //delta = PollingInterval;
            continue;
        }
        if (delta > 0)
        {
            int ticks = floor(delta/10);
            for(int i=0; i < ticks; i++)
            {
              Sleep(10);
              if (Terminated) return;
            }
        }
    }
}

__fastcall TDeviceThread::~TDeviceThread()
{
    if ((Log)&&(LoggingErrors))
        Log->Write("Exit from thread...");
}

void __fastcall TDeviceThread::ProcessLoopCommand()
{
  ThreadTerminated = false;
  try
  {
    if (Terminated) return;
    GetStay();
    if (Terminated) return;
    //if (DeviceState->StateCode != 0xff && DeviceState->StateCode != 0x00)
    if (DeviceState->StateCode != 0xff)
    {
        DeviceState->OutStateCode = DSE_OK;
        if (Log != NULL)
            if (Log) Log->Write("Device is mount.");
    }
    else
    {
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        if (Log != NULL)
            if (Log) Log->Write("No Device Present.");
    }
    while(!Terminated)
    {
        clock_t BeginTime = clock();
        if (Terminated)
          return;
        PollingLoop();
        clock_t EndTime = clock();
        DWORD delta = PollingInterval - (EndTime - BeginTime)/CLK_TCK * 1000;
        if (delta > 0)
            Sleep(delta);
    }
    /*while(!Terminated)
    {
      int ticks = floor(PollingInterval/10);
      for(int i=0; i <= ticks; i++)
      {
        Sleep(10);
        //Application->ProcessMessages();
        if (Terminated == true)
          return;
      }
      if (Terminated)
        return;
      PollingLoop();
    }*/
  }
  __finally
  {
  }
}

bool TDeviceThread::ClearCOMPortError()
{
  if (!Port->Port) return false;
  COMSTAT cs;
  LPVOID lpMsgBuf = NULL;

  if ( ClearCommError(Port->Port, &LastError, &cs) )
    return true;
  else
  {
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        LastError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0,
        NULL
    );
    if (Log != NULL)
      Log->Write((boost::format("Port reset error! %1%") % ((char*)lpMsgBuf)).str().c_str());
    LocalFree(lpMsgBuf);
    return false;
  }
}

bool TDeviceThread::ClearCOMPort()
{
  if (!Port->Port) return false;
  //if ( PurgeComm(Port->Port,PURGE_TXCLEAR) && PurgeComm(Port->Port,PURGE_RXCLEAR) )
  if ( PurgeComm(Port->Port,PURGE_TXCLEAR || PURGE_RXCLEAR || PURGE_TXABORT || PURGE_RXABORT) )
    return true;
  else
  {
    LPVOID lpMsgBuf;
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
      Log->Write((boost::format("Port cleanup error! %1%") % ((char*)lpMsgBuf)).str().c_str());
    LocalFree(lpMsgBuf);
    return false;
  }
}

void TDeviceThread::AddSimpleAnswer(BYTE value)
{
    SimpleAnswers.push_back(value);
}

std::vector<BYTE> TDeviceThread::GetSimpleAnswers()
{
  return SimpleAnswers;
}

void TDeviceThread::move(BYTE *Dest, BYTE *Sour, unsigned int count)
{
  if ((Dest == NULL)||(Sour == NULL))
    return;
  for (unsigned int i=0; i<count; i++)
    Dest[i] = Sour[i];
  return;
}

void TDeviceThread::ClearBuffer(BYTE* Buffer)
{
  memset(Buffer,0,DTBufferSize);
}

int TDeviceThread::SendCommand(BYTE* Command, int CommandSize, BYTE*& Answer, int& AnswerSize, TSendType type, int mode)
{
  int result = -1;
  try
  {
    CommandCriticalSection->Enter();
    result = SendLowLevelCommand(Command,CommandSize,Answer,AnswerSize,type);
    DeviceState->AnswerSize = AnswerSize;
    if (SendType == RecieveAnswer)
      ParseAnswer(mode);

    /*int times = 0;

    while(times < 1)
    {
      if (SendType == RecieveAnswer)
        ParseAnswer();
      else
        break;
      if ((PollingMode)&&(DeviceState->StateCode == 0xFF)) //NAK
      {
        result = SendLowLevelCommand(Command,CommandSize,Answer,AnswerSize,type);
        DeviceState->AnswerSize = AnswerSize;
        times++;
      }
      else
        break;
    }*/
  }
  __finally
  {
    CommandCriticalSection->Leave();
  }
  return result;
}

int TDeviceThread::SendSingleCommand(BYTE* Command, int CommandSize, BYTE*& Answer, int& AnswerSize, TSendType type)
{
  int result = -1;
  try
  {
    CommandCriticalSection->Enter();
    result = SendLowLevelCommand(Command,CommandSize,Answer,AnswerSize,type);
    DeviceState->AnswerSize = AnswerSize;
  }
  __finally
  {
    CommandCriticalSection->Leave();
  }
  return result;
}

DWORD TDeviceThread::ReadPort(BYTE* Buffer, DWORD& BytesCount)
{
    COMSTAT cs;
    ClearBuffer(Buffer);
    DWORD LastError;
    ClearCommError(Port->Port, &LastError, &cs);
    // читаем из порта
    if ( !ReadFile(Port->Port, Buffer, cs.cbInQue, &BytesCount, NULL))
    {
      if (Log != NULL)
        Log->Write("Can't read answer!");
      return -1;
    }
    if ((Log != NULL)&&(LoggingErrors)&&(BytesCount>0))
    {
      if (BytesCount > 1)
          Log->Write("Big Answer ReadPort:");
      else
          Log->Write("ReadPort:");
      Log->WriteBuffer(Buffer, BytesCount);
    }
    return BytesCount;
}

int TDeviceThread::SendLowLevelCommand(BYTE* Command, int CommandSize, BYTE*& Answer, int& AnswerSize, TSendType type)
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
          Log->Write("Command to send:");
          Log->WriteBuffer(Command,CommandSize);
        }
      }

      if(Port->m_portType == PortType::com)
      {
          ClearCommError(Port->Port, &LastError, &cs);
          if (cs.cbInQue > 0)
              if ((Log != NULL)&&(LoggingTimeoutErrors))
              {
                  Log->Write((boost::format("Port is not empty! size=%1%; The trailing data is:") % cs.cbInQue).str().c_str());
                  ReadFile(Port->Port, TempBuffer, cs.cbInQue, &BytesProcessed, NULL);
                  Log->WriteBuffer(TempBuffer,BytesProcessed);
              }
          // очищаем порт
          if (CleanPortBeforeReading)
          {
              Port->ClearCOMPort();
              if (Log != NULL)
                  Log->Write("Port was cleared before reading");
          }

          DeviceState->DSR_CTS = Port->DSR_CTS();
      }

      //Log->Write("send command to device...");
      //Log->WriteBuffer(Command, CommandSize);
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
            Log->Write((boost::format("Can't write command in port %1%") % ((char*)lpMsgBuf)).str().c_str());
          LocalFree(lpMsgBuf);
          return -1;
      }

      TimeStamp1 = clock();

      if ((type == NotRecieveAnswer)||(Answer == NULL))
          return 0;

      ClearBuffer(Answer);
      AnswerSize = 0;
      // очищаем содержимое буфера
      ClearBuffer(MainBuffer);
      ClearBuffer(SecondBuffer);
      ClearBuffer(TempBuffer);


      // засекаем время
      BytesCount = 0;
      if (WriteReadTimeout > 0)
          Sleep(WriteReadTimeout);
      Clock = clock();

      do
      {
          //если внешний поток прерывает процесс, выходим
          ////Application->ProcessMessages();
          if (Terminated)
          {
            AnswerSize = 0;
            TerminateCommand = false;
            return -1;
          }
          // очищаем буфер
          ClearBuffer(TempBuffer);

          DWORD bytesToRead = 1;
          if(Port->m_portType == PortType::com)
          {
              Port->DSR_CTS();
              GetCommState(Port, &dcb);

              // сбрасываем ошибки порта
              if ( !ClearCommError(Port->Port, &LastError, &cs) )
              {
                  if (Log != NULL)
                    Log->Write("Can't reset error status of COM port!");
                  AnswerSize = 0;
                  return -1;
              }
              if ( cs.cbInQue == 0 )//если принято 0 байт, идём дальше
              {
                  Sleep(1);
                  continue;
              }
              bytesToRead = cs.cbInQue;
          }

              if ( !ReadFile(Port->Port, TempBuffer, /*cs.cbInQue*/bytesToRead, &BytesProcessed, NULL))
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
                    Log->Write((boost::format("Can't read answer! %1%") % ((char*)lpMsgBuf)).str().c_str());
                  LocalFree(lpMsgBuf);
                  return -1;
              }
          
          if ( BytesProcessed == 0 )//если принято 0 байт, идём дальше
          {
              Sleep(1);
              continue;
          }
          //Log->Write("Readed bytes count = "+AnsiString(BytesProcessed));
          //добавляем в буфер ответа считанный кусок
          for (int i = BytesCount; (unsigned)i<(BytesCount + BytesProcessed); i++)
            SecondBuffer[i] = TempBuffer[i-BytesCount];
          //длина ответа выросла
          BytesCount += BytesProcessed;
          //Log->Write("receive answer from device...");
          //Log->WriteBuffer(SecondBuffer, BytesCount);

          //разбираем ответ
          //смотрим есть ли в буфере однобайтовый ответ
          AnswerSize = IsBufferContainSimpleAnswer(SecondBuffer);
          if (AnswerSize != -1)
          {
            move(MainBuffer,&SecondBuffer[AnswerSize],1);
            move(Answer,&SecondBuffer[AnswerSize],1);
            if (LoggingErrors)
            {
              if ((Log != NULL)&&(LoggingErrors))
              {
                time = clock();
                //double delta = ((double)time - (double)Clock)/1000;
                AnsiString mess;
                //mess.sprintf("Интервал получения ответа %.4f сек.",delta);
                //Log->Write(mess);
                Log->Write("Short answer received:");
                Log->WriteBuffer(MainBuffer,1);
              }
            }
            return AnswerSize = 1;
          }

          if (DataLength <= 0)
          {
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
                    Log->Write("Полученный ответ:");
                    Log->WriteBuffer(MainBuffer,AnswerSize);
                  }
                }
                return AnswerSize;
              }
          }
          else//просто считываем нужное количество байт
          {
              if (BytesCount >= DataLength)
              {
                AnswerSize = BytesCount;
                move(MainBuffer,SecondBuffer,BytesCount);
                move(Answer,SecondBuffer,BytesCount);
                if ((Log != NULL)&&(BytesCount > 1)&&(DataLength == 1))
                {
                      Log->Write((boost::format("Big Answer size is: %1%") % AnswerSize).str().c_str());
                      Log->WriteBuffer(Answer,AnswerSize);
                }
                return AnswerSize;
              }
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
        Log->Write("Timeout! No proper answer from device.");
        Log->Write((boost::format("Butes received: %1%, interval %2% sec.") % BytesCount % delta).str().c_str());
        Log->WriteBuffer(SecondBuffer, BytesCount);
      }
      return -1;
  }
  __finally
  {
  }
  return AnswerSize;
}

int TDeviceThread::GetBytePosition(BYTE* Buffer, int size ,BYTE value)
{
  for(int i=0; i<size; i++)
    if (Buffer[i] == value)
      return i;
  return -1;
}

int TDeviceThread::IsBufferContainSimpleAnswer(BYTE* Buffer)
{
    for(std::size_t i=0; i<SimpleAnswers.size(); i++)
    {
        int pos = GetBytePosition(Buffer,sizeof(Buffer),SimpleAnswers[i]);
        if ( pos != -1 )
          return pos;
    }
    return -1;
}

int TDeviceThread::PrepareAnswer(BYTE*& Buffer,int count)
{
  if ((count == 0)||(Buffer == NULL))
    return -1;
  int posBegin = GetBytePosition(Buffer,count,BeginByte);
  if ( posBegin == -1 )
    return -1;

  if (DataLengthIndex == -1)
  {   //если в команде не указывается длина принятых байт, обрезаем по начальному
      //и конечному байту, как в фискальных регистраторах
      int posEnd = GetBytePosition(Buffer,count,EndByte);
      if ( posEnd == -1 )
        return -1;
      //если индекс последнего байта буфера меньше положенной длины вместе с CRC, значит
      // CRC ещё не дошло и надо подождать
      if ( (count-1) < posEnd+CRCLength)
        return -1;

      ClearBuffer(TempBuffer);
      for(int i=posBegin; i<=posEnd+CRCLength; i++)
        TempBuffer[i-posBegin] = Buffer[i];
      ClearBuffer(Buffer);
      count = posEnd-posBegin+1+CRCLength;
      move(Buffer,TempBuffer,count);
  }
  else
  {   //если указана длина команды, как в купюрниках, то считываем определённое количество байт
      //смотрим сколько принято байт, от первого байта в команде
      int len = count - posBegin;
      //если принятых байт меньше, то выходим
      if (len <= 0 )
        return -1;
      //смотрим, уместился ли байт, где указана длина команды, в буфер
      if ((count - (posBegin + DataLengthIndex)) <= 0 )
        return -1;
      //определяем длину данной команды
      BYTE datalen = Buffer[posBegin + DataLengthIndex];
      //определяем, пришла ли вся команда целиком
      if (len < datalen)
        return -1;

      //сдвигаем в буфере данные, если впереди команды пришёл мусор
      ClearBuffer(TempBuffer);
      for(int i=posBegin; i<=posBegin+datalen-1; i++)
        TempBuffer[i-posBegin] = Buffer[i];
      ClearBuffer(Buffer);
      count = datalen;
      move(Buffer,TempBuffer,count);
  }

  return count;
}


void TDeviceThread::ClearCommand()
{
  memset(Command,0,DTBufferSize);
  CommandSize = 0;
}

void TDeviceThread::ClearAnswer()
{
  memset(Answer,0,DTBufferSize);
  AnswerSize = 0;
}


bool TDeviceThread::IsNominalEnabled(int value)
{
  if (DisabledNominals)
    if ( DisabledNominals->IndexOf((void*)value) >= 0 )
      return false;
  return true;
}                                        

bool TDeviceThread::ChangeDeviceState(bool wait)
{
    if(DeviceState->Scanner)
    {
        if (DeviceState->oldScannerDataValue != DeviceState->scannerDataValue ||
            DeviceState->oldScannerDataType != DeviceState->scannerDataType)
        {
            DeviceStateChanged(DeviceState);
            return true;
        }
        else
        {
            return false;
        }
        //DeviceStateChanged(DeviceState);
        //return true;
    }
    if (((DeviceState)&&(DeviceState->OldOutStateCode != DeviceState->OutStateCode))||
        (DeviceState->Billing) ||
        (DeviceState->Processing))
    {
        DeviceState->StateChange = true;
        if ((DeviceState->OldOutStateCode == DeviceState->OutStateCode)&&(DeviceState->Billing))
          DeviceState->StateChange = false;
        //Log->Write("OutStateCode="+AnsiString(DeviceState->OutStateCode)+"; OldOutStateCode="+AnsiString(DeviceState->OldOutStateCode));
        Log->Write((boost::format("OutStateCode=%1%; OldOutStateCode=%2%") % DeviceState->OutStateDescription(DeviceState->OutStateCode).c_str() % DeviceState->OutStateDescription(DeviceState->OldOutStateCode).c_str()).str().c_str());
        if (DeviceStateChanged)
          DeviceStateChanged(DeviceState);
        DeviceState->OldOutStateCode = DeviceState->OutStateCode;
        if (wait)
        {
            int interval = 10;
            DWORD SleepTime = 10*1000;
            while((DeviceState)&&(DeviceState->Done == false))
            {
                //Application->ProcessMessages();
                Sleep(interval);
                SleepTime -= interval;
                if (SleepTime <= 0)
                    break;
            }
        }
        return true;
    }
    return false;
}

BYTE TDeviceThread::GetExtCommand()
{
    BYTE result = ExtCommand;
    ExtCommand = EXTC_Free;
    return result;
}

bool TDeviceThread::SetExtCommand(BYTE Command)
{
    //ExtCommand == EXTC_Free - command is free for exucution
    bool result = false;
    switch (ExtCommand)
    {
        case EXTC_Free:
            result = true;
            ExtCommand = Command;
            break;
        default :
            result = false;
            break;
    }
    return result;
}

bool TDeviceThread::IsInitialized()
{
    return Initialized;
}

void TDeviceThread::SetInitialized()
{
    if ((Log)&&(!Initialized))
        Log->Write("Device initialized in the thread.");
    Initialized = true;
}

clock_t TDeviceThread::GetTimeStamp1()
{
    return TimeStamp1;
}

clock_t TDeviceThread::GetTimeStamp2()
{
    return TimeStamp2;
}

std::string TDeviceThread::GetByteExp(BYTE value)
{
    std::string mess;
    mess = (boost::format(" %02X") % value).str();
    return mess;
}

bool TDeviceThread::CheckDeviceActivity(int parameter)//by default parameter = DeviceState->AnswerSize - as sign of device activity
{
    bool result = false;
    //test feature for notification about answer/non-answered states of validator
    //if host has no answer from validator - send notification once
    if (parameter <= 0)
    {
        PollCounter++;
        if ((DeviceState->OutStateCode != DSE_NOTMOUNT)&&(PollCounter > PollCounterLimit))
        {
            PollCounter = PollCounterLimit + 1;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            ChangeDeviceState();
            result = true;
        }
    }//PollCounter resets in ParseAnswer()
    //if host recieved answer from validator beyond its non-answered state - send notification
    else
    {
        if ((DeviceState->OutStateCode == DSE_NOTMOUNT)&&(PollCounter < PollCounterLimit))
        //if (DeviceState->OutStateCode == DSE_NOTMOUNT)
        {
            //PollCounter = 0;
            DeviceState->OutStateCode = DSE_OK;
            ChangeDeviceState();
            result = true;
        }
    }

    if (result)
        switch (DeviceState->OutStateCode)
        {
            case DSE_NOTMOUNT:
                if (Log) Log->Write("Device state has been changed to DSE_NOTMOUNT.");
                break;
            case DSE_OK:
                if (Log) Log->Write("Device state has been changed to DSE_OK.");
                break;                                                
        }

    return result;
}

//==============================================================================

_info::_info()
{
    _Status = cs_Wait;
}
void _info::SetStatus(int value)
{
    _Status = value;
}

int _info::GetStatus()
{
    switch (_Status)
    {
        case cs_Wait:
            return _Status;
        case cs_Processing:
            return _Status;
        case cs_Done:
            _Status = cs_Wait;
            return cs_Done;
    }
    return _Status;
}

void _info::SetResult(int value)
{
    _Result = value;
}

int _info::GetResult()
{
    return _Result;
}

void _info::ClearMembers()
{
    _Result = 0;
    _Status = 0;
}
//==============================================================================

