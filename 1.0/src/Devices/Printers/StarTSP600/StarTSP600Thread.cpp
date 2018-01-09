//---------------------------------------------------------------------------


#pragma hdrstop

#include "StarTSP600Thread.h"
#include "StarTSP600Defines.h"
#include "boost/format.hpp"
//---------------------------------------------------------------------------

#pragma package(smart_init)

__fastcall CStarTSP600Thread::CStarTSP600Thread() : TDeviceThread(true, false)
{
  PollingMode = false;
  BeginByte = STX;
  EndByte = ETX;
  CRCLength = 2;
  LoggingTimeoutErrors = true;
  CleanPortBeforeReading = true;
  WriteReadTimeout = 400;
  CleanPortBeforeReading;
  //SimpleAnswers.push_back(ACK);
}

int CStarTSP600Thread::SendCommand(BYTE* Command, int CommandSize, BYTE*& Answer, int& AnswerSize, TSendType type, int mode)
{
    try
    {
      CommandCriticalSection->Enter();
      SendType = type;

      DeviceState->ExecCode = EC_NO_ANSWER;
      SendLowLevelCommand(Command, CommandSize, Answer, AnswerSize, NotRecieveAnswer);

      // теперь начинаем получать ответ
      ClearBuffer(Answer);
      AnswerSize = 0;
      clock_t startCmdTime = clock();

      while(true)
      {
       DWORD bufferSize = 0;
       Sleep(100);
       bufferSize = ReadPort(TempBuffer, DTBufferSize);

       for(unsigned long i = 0; i < bufferSize; i++)
       {
        // ACK нам в ответе не нужен
        if(TempBuffer[i] != ACK)
        {
         // копируем ответ
         memcpy(Answer + AnswerSize, TempBuffer + i, bufferSize - i);
         AnswerSize += bufferSize - i;
         break;
        }
       }

       if(Answer[0] == STX && Answer[AnswerSize - 1] == ETX && checkAnswer())
       {
        DeviceState->ExecCode = EC_SUCCESS;
        break;
       }
       else if(TempBuffer[0] == ACK)
       {
        startCmdTime = clock();
       }
       else
       {
        if((clock() - startCmdTime) / CLK_TCK > 3)
        {
         DeviceState->ExecCode = EC_BAD_ANSWER;
         break;
        }
       }
      }
      // почистим за собой порт
      ClearCOMPort();
    }
    __finally
    {
      CommandCriticalSection->Leave();
    }
    return AnswerSize;
}

// проверка ответа
bool CStarTSP600Thread::checkAnswer()
{
 if(Answer[1] != ERR && Answer[1] != Command[1])
  return false;

 bool result = false;

 BYTE calcBCC = 0x00;
 for(unsigned int i = 1; i < AnswerSize - 3; i++)
  calcBCC += Answer[i];


 char data[3];
 data[0] = Answer[AnswerSize - 3];
 data[1] = Answer[AnswerSize - 2];
 data[2] = 0x00;

 char *end_ptr;
 BYTE val = (BYTE)strtol(data, &end_ptr, 16);
 if(val == calcBCC)
  result = true;

 return result;
}

