//---------------------------------------------------------------------------


#pragma hdrstop

#include "Prim21kThread.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

#define DefRepsCount                              20
#define ERRORSCOUNT                               20


__fastcall CPrim21kThread::CPrim21kThread() : TDeviceThread(true, false)
{
  PollingMode = false;
  BeginByte = 0x02;
  EndByte = 0x03;
  CRCLength = 4;
  SimpleAnswers.push_back(0x06);
  SimpleAnswers.push_back(0x18);
  SimpleAnswers.push_back(0x15);
  LoggingTimeoutErrors = true;
  CleanPortBeforeReading = true;
}

__fastcall CPrim21kThread::~CPrim21kThread()
{
}

BYTE CPrim21kThread::SendCommand(BYTE *Command, unsigned int CommandCount, BYTE *Answer, unsigned int& AnswerCount)
{
        int RepCount = 1;
        bool ContinueLoop = true;
        BYTE Count = 1;
        LastError = PE_LINK_ERROR;
        int  OldCommandCount = CommandCount;
        int  OldAnswerCount = AnswerCount;

        do
        {
                AnswerCount = OldAnswerCount;

                // проверка числа попыток
                if (Count>RepCount)
                {
                  if (LoggingErrors)
                      Log->Write("Send command ttries overcount.");
                  return LastError = PE_LINK_ERROR;
                }

                Count++;
                // посылаем команду
                int count = SendLowLevelCommand(Command,CommandCount,Answer,AnswerCount,RecieveAnswer);
                AnswerSize = AnswerCount = count;
                //if (count == 0)
                if (count <= 0)
                {//если нет ответа
                    Log->Write("Wrong length of device answer.");
                    return LastError = PE_LINK_ERROR;
                    /*if (NAKInAnswer(Answer))
                    {
                        Command[0] = STX;
                        CommandCount = OldCommandCount;
                        ContinueLoop = true;
                        continue;
                    }*/
                }
                else
                {//если ответ есть, смотрим его корректность и соответствие формату
                    // если ошибка в отличительных байтах - посылаем команду заново
                    if (count > 1)
                        if (Command[5] != Answer[1])
                            {
                                Log->Write("Wrong device answer check byte.");
                                LastError == PE_DIFFERBYTE_ERROR;
                                Command[0] = STX;
                                CommandCount = OldCommandCount;
                                ContinueLoop = true;
                                continue;
                            }

                    // ошибка в контрольной сумме - посылка NAK
                    if (count > 1)
                    {
                        if (!CompareBCC(Answer,AnswerCount))
                        {
                            Log->Write("Device answer checksumm error.");
                            LastError = PE_BCC_ERROR;
                            Command[0] = NAK;
                            CommandCount = 1;
                            ContinueLoop = true;
                            continue;
                        }
                    }
                }

                Log->Write("Device answer wrong.");
                return LastError = PE_OK;

        } while (ContinueLoop);

        return LastError = PE_LINK_ERROR;
}

int CPrim21kThread::SendCommand(BYTE* Command, int CommandSize, BYTE*& Answer, int& AnswerSize, TSendType type, int mode)
{
    int result = -1;
    try
    {
      CommandCriticalSection->Enter();
      SendType = type;

      DeviceState->ExecCode = PE_OK;
      DeviceState->ExecCode = SendCommand(Command, CommandSize, Answer, AnswerSize);
      DeviceState->AnswerSize = AnswerSize;

      switch (DeviceState->ExecCode)
      {
        case PE_OK:
          if (CheckStatus(Answer, AnswerSize))
          {
              result = AnswerSize;
          }
          else
          {
              if (DeviceState->CriticalErrorsCount>0)
                DeviceState->ExecCode = DeviceState->Errors[0];
              else
              {
                if ( DeviceState->CriticalStatusesCount>0)
                  DeviceState->ExecCode = DeviceState->Statuses[0];
              };
              if ((DeviceState->ResultCode == PE_OK)&&(DeviceState->CriticalErrorsCount > 0))
                  DeviceState->ResultCode = DeviceState->Errors[0];
              DeviceState->AnswerSize = result = -1;
          }
          break;

        case PE_LINK_ERROR:
          DeviceState->AnswerSize = result = -1;
          DeviceState->ResultCode = PE_LINK_ERROR;
          Log->Write("PE_LINK_ERROR");
          break;
      }

      return result;
    }
    __finally
    {
      CommandCriticalSection->Leave();
    }
    return result;
}

bool CPrim21kThread::CompareBCC(BYTE *Buffer, unsigned int Count)
{
  WORD BCC = 0;
  char CalcBCC[5];
  char AnswBCC[5];
  bool result = true;

  memset(CalcBCC,0,5);
  memset(AnswBCC,0,5);
  for(unsigned int i = 0; i<Count-4; i++)
          BCC += Buffer[i];
  sprintf(CalcBCC,"%04X",BCC);

  AnswBCC[0] = Buffer[Count-2];
  AnswBCC[1] = Buffer[Count-1];
  AnswBCC[2] = Buffer[Count-4];
  AnswBCC[3] = Buffer[Count-3];
  AnswBCC[4] = 0;

  if ( strcmp( (char *)CalcBCC, (char *)AnswBCC) )
    result = false;

  return result;
}

bool CPrim21kThread::NAKInAnswer(BYTE *Buffer)
{
    if (Buffer[0] == NAK)
            return true;
    else
            return false;
}

bool CPrim21kThread::ACKInAnswer(BYTE *Buffer)
{
    if (Buffer[0] == ACK)
            return true;
    else
            return false;
}

bool CPrim21kThread::CANInAnswer(BYTE *Buffer)
{
    if (Buffer[0] == CAN)
            return true;
    else
            return false;
}

bool CPrim21kThread::TruncateAnswer(BYTE *Buffer, DWORD &Count)
{
  unsigned int i;
  unsigned int size = 0;
  unsigned int pos = 0;
  BYTE *p = NULL;
  bool result = false;

  if ((Buffer == NULL)||(Count == 0))
    return false;

  for (i = 0; i<Count; i++)
    if (Buffer[i] != 0)
            size++;

  if (size == Count)
          return false;

  if (size == 0)
  {
      memset(Buffer,0,Count);
      Count = 0;
      return false;
  }

  p = new BYTE[size];
  memset(p,0,size);

  pos = 0;
  for (i = 0; i<Count; i++)
    if (Buffer[i] != 0)
    {
      p[pos] = Buffer[i];
      pos++;
    }

  memset(Buffer,0, Count);
  move(Buffer, p, size);

  Count = size;

  delete []  p;
  return result;
}

bool CPrim21kThread::IsBufferEmpty(BYTE *Buffer, int size)
{
    int i;
    bool result = false;

    for(i=0; i<size; i++)
      if (Buffer[i] != 0)
              result = true;

    return !result;
}

int CPrim21kThread::FindByteInBuffer(BYTE *Buffer, int BufferCount, BYTE Value, int Position)
{
  int result = -1;
  int i;

  for (i = Position; i<BufferCount; i++)
  {
    if (Buffer[i] == Value)
    {
      result = i;
      return result;
    }
  }

  return result;
}

BYTE CPrim21kThread::PrepareAnswer(BYTE *Buffer, unsigned int& Count)
{
  BYTE result = PE_DIFFERBYTE_ERROR;
  BYTE *p = NULL;
  int STXpos, ETXpos;
  unsigned int size;

  if ((Buffer == NULL)||(Count == 0))
                return result;

  STXpos = FindByteInBuffer(Buffer,Count,STX,0);
  if (STXpos == -1)
          return result;

  ETXpos = FindByteInBuffer(Buffer,Count,ETX, STXpos);
  if (ETXpos == -1)
          return result;

  if ((STXpos > ETXpos))
          return result;

  result = PE_OK;

  size = ETXpos-STXpos+1+4;
  p = new BYTE[size];
  memset(p,0,size);

  move((BYTE*)&Buffer[STXpos], p, size);
  memset(Buffer,0, Count);
  move(p, Buffer, size);

  Count = size;
  delete []  p;

  return result;
}

BYTE CPrim21kThread::CharToBYTE(BYTE ch)
{
  BYTE result = 0;

  if ((ch>=0x41)&&(ch<=0x46))
    result = ch - 0x37;
  if ((ch>=0x30)&&(ch<=0x39))
    result = ch - 0x30;
  return result;
}


BYTE CPrim21kThread::GetBYTE(BYTE *Buffer, int index)
{
  BYTE result = 0;

  result = (BYTE)CharToBYTE(Buffer[index])<<4;
  result |= ((BYTE)CharToBYTE(Buffer[index+1]));

  return result;
}

BYTE CPrim21kThread::GetConstantStatusIndex()
{
  BYTE pos = 5;
  return pos;
}

BYTE CPrim21kThread::GetCurrentStatusIndex()
{
  BYTE pos = 8;
  return pos;
}

BYTE CPrim21kThread::GetResultCommandIndex()
{
  BYTE pos = 13;
  return pos;
}

BYTE CPrim21kThread::GetPrinterStatusIndex()
{
  BYTE pos = 18;
  return pos;
}


WORD CPrim21kThread::GetWORD(BYTE *Buffer, int index)
{
  WORD result = 0;

  result = (WORD)CharToBYTE(Buffer[index])<<4;
  result |= ((WORD)CharToBYTE(Buffer[index+1]));
  result |= ((WORD)CharToBYTE(Buffer[index+2])<<12);
  result |= ((WORD)CharToBYTE(Buffer[index+3])<<8);

  return result;
}

bool CPrim21kThread::CheckStatus(BYTE *Buffer, unsigned int BufferCount)
{
  bool result = true;
  BYTE errorscount = 0;
  BYTE statusescount = 0;

  //DeviceState->SetOutCodes(DSE_UNKNOWN_CODE, DSE_UNKNOWN_CODE);
  DeviceState->SetOutCodes(DSE_NOERROR, DSE_NOERROR);

  DeviceState->CriticalErrorsCount=0;

  // сюда будем писать критические ошибки и состояния
  // в конец последовательности записываем байт END_OF_SEQUENCE
  memset(DeviceState->Errors,0,100);
  //DeviceState->ErrorsCount = ERRORSCOUNT;
  DeviceState->ErrorsCount = 0;

  // сюда будем писать НЕ критические ошибки и состояния
  // в конец последовательности записываем байт END_OF_SEQUENCE
  memset(DeviceState->Statuses,0,100);
  //DeviceState->StatusesCount = ERRORSCOUNT;
  DeviceState->StatusesCount = 0;

  // Анализируем постоянный статус регистратора
  StatusErrorCode.ByteCode = GetBYTE(Buffer,GetConstantStatusIndex());
  if ( StatusErrorCode.ByteBitCode.b7 == 1)
  {
    DeviceState->Errors[errorscount] = PE_HARDWARE_ERROR;
    DeviceState->SetOutCodes(PE_HARDWARE_ERROR);
    errorscount ++;
    result = false;
  }

  if ( StatusErrorCode.ByteBitCode.b6 == 1)
  {
    DeviceState->Errors[errorscount] = PE_WORKING_MEMORY_ERROR;
    errorscount ++;
    DeviceState->SetOutCodes(PE_WORKING_MEMORY_ERROR);
    result = false;
  }

  if ( StatusErrorCode.ByteBitCode.b5 == 1)
  {
    DeviceState->Errors[errorscount] = PE_FISCAL_MEMORY_ERROR;
    errorscount ++;
    DeviceState->SetOutCodes(PE_FISCAL_MEMORY_ERROR);
    result = false;
  }

  if ( StatusErrorCode.ByteBitCode.b4 == 0)
  {
    DeviceState->Statuses[statusescount] = PE_FISCAL_MODE_NOT_SET;
    DeviceState->SetOutCodes(DSE_NOTSET, PE_FISCAL_MODE_NOT_SET);
    statusescount ++;
  }

  if ( StatusErrorCode.ByteBitCode.b3 == 1)
  {
    DeviceState->Statuses[statusescount] = PE_FISCAL_MEMORY_ABOUT_OVERFLOW;
    DeviceState->SetOutCodes(DSE_NOTSET, PE_FISCAL_MEMORY_ABOUT_OVERFLOW);
    statusescount ++;
  }

  if ( StatusErrorCode.ByteBitCode.b2 == 1)
  {
    DeviceState->Errors[errorscount] = PE_FISCAL_MEMORY_OVERFLOW;
    DeviceState->SetOutCodes(PE_FISCAL_MEMORY_OVERFLOW);
    errorscount ++;
    result = false;
  }

  if ( StatusErrorCode.ByteBitCode.b1 == 1)
  {
    DeviceState->Errors[errorscount] = PE_REGISTRATION_COUNT_OVERFLOW;
    DeviceState->SetOutCodes(PE_REGISTRATION_COUNT_OVERFLOW);
    errorscount ++;
    result = false;
  }

  if ( StatusErrorCode.ByteBitCode.b0 == 0)
  {
    //DeviceState->Errors[errorscount] = PE_HARDWARE_SERIAL_NUMBER_NOT_SET;
    //errorscount ++;
    //result = false;
    DeviceState->Statuses[statusescount] = PE_HARDWARE_SERIAL_NUMBER_NOT_SET;
    DeviceState->SetOutCodes(DSE_NOTSET, PE_HARDWARE_SERIAL_NUMBER_NOT_SET);
    statusescount ++;
  }

  // Анализируем результат выполнения команды
  DeviceState->ResultCode = 0xFF;
  DeviceState->ResultCodeExtention = 0xFF;
  DeviceState->ResultCode = GetBYTE(Buffer,GetResultCommandIndex());
  DeviceState->ResultCodeExtention = GetBYTE(Buffer,GetResultCommandIndex()+2);

  /*if ((DeviceState->ResultCode!=PE_OK)
  &&(DeviceState->ResultCode!=PE_PAPER_ABOUT_OVER)
  //&&(DeviceState->ResultCode!=PE_EKLZ_NOT_FOUND)
  )
  {
    DeviceState->Errors[errorscount] = DeviceState->ResultCode;
    errorscount ++;
    result = false;
  }*/

  if (DeviceState->ResultCode==PE_PAPER_ABOUT_OVER)
  {
    DeviceState->Statuses[statusescount] = PE_PAPER_ABOUT_OVER;
    DeviceState->SetOutCodes(PE_PAPER_ABOUT_OVER);
    statusescount ++;
  }

  // Анализируем текущий статус регистратора
   StatusErrorCode.WordCode = GetWORD(Buffer,GetCurrentStatusIndex());
   DeviceState->FiscalDocumentState = FISCAL_DOCUMENT_UNDEFINED;

  // записываем состояние фискального документа
  BYTE fiscal_doc_state = (BYTE)( StatusErrorCode.WordCode && 0x0007);
  DeviceState->FiscalDocumentState = fiscal_doc_state;

  if ( StatusErrorCode.WordBitCode.b11 == 1)
  {
    DeviceState->Errors[errorscount] = PE_NEED_TO_CLOSE_SHIFT;
    DeviceState->SetOutCodes(PE_NEED_TO_CLOSE_SHIFT);
    errorscount ++;
    result = false;
  }

  if ( StatusErrorCode.WordBitCode.b10 == 1)
  {
    //DeviceState->Errors[errorscount] = PE_PREVIOUS_COMMAND_UNRECOGNIZED;
    //errorscount ++;
    DeviceState->Statuses[statusescount] = PE_PREVIOUS_COMMAND_UNRECOGNIZED;
    statusescount ++;
    result = false;
  }

  if ( StatusErrorCode.WordBitCode.b9 == 1)
  {
    //DeviceState->Errors[errorscount] = PE_PREVIOUS_COMMAND_NOT_COMPLETE;
    //errorscount ++;
    DeviceState->Statuses[statusescount] = PE_PREVIOUS_COMMAND_NOT_COMPLETE;
    statusescount ++;
    result = false;
  }

  if ( StatusErrorCode.WordBitCode.b8 == 1)
  {
    DeviceState->Errors[errorscount] = PE_INSPECTOR_PASSWORD_INCORRECT;
    DeviceState->SetOutCodes(PE_INSPECTOR_PASSWORD_INCORRECT);
    errorscount ++;
    result = false;
  }

  if ( StatusErrorCode.WordBitCode.b7 == 1)
  {
    DeviceState->Statuses[statusescount] = PE_SESSION_NOT_CLOSED;
    statusescount ++;
  }

   DeviceState->PrinterMode = UNKNOWN_MODE;

  if ( StatusErrorCode.WordBitCode.b6 == 1)
  {
      DeviceState->SetOutCodes(DSE_NOTSET,PRN_FISCAL_MODE);
      DeviceState->PrinterMode = FISCAL_MODE;
  }
  else
  {
      DeviceState->SetOutCodes(DSE_NOTSET,PRN_NONFISCAL_MODE);
      DeviceState->PrinterMode = TRAIN_MODE;
  }

  if ( StatusErrorCode.WordBitCode.b5 == 1)
  {
    DeviceState->Statuses[statusescount] = PE_DOC_BUFFER_ABOUT_OVERFLOW;
    statusescount ++;
  }

  /*if ( StatusErrorCode.WordBitCode.b4 == 1)
     DeviceState->ShiftState = SHIFT_OPENED;
  else
     DeviceState->ShiftState = SHIFT_CLOSED;*/

  // Анализируем состояние печатающего устройства
  //1-й байт PE_ERROR_OF_STATUS_RECEIVE
   StatusErrorCode.ByteCode = GetBYTE(Buffer,GetPrinterStatusIndex());
  if ( StatusErrorCode.ByteBitCode.b4 == 1)
  {
    DeviceState->Errors[errorscount] = PE_LINK_ERROR;
    errorscount ++;
    DeviceState->SetOutCodes(PRN_SOFTWARE_ERROR);
    //DeviceState->Statuses[statusescount] = PE_LINK_ERROR;
    //statusescount ++;
    result = false;
  }

  if ((DeviceState->ResultCode!=PE_OK)
  &&(DeviceState->ResultCode!=PE_PAPER_ABOUT_OVER)
  //&&(DeviceState->ResultCode!=PE_EKLZ_NOT_FOUND)
  )
  {
    DeviceState->Errors[errorscount] = DeviceState->ResultCode;
    switch(DeviceState->ResultCode)
    {
        case PE_DATE_TIME_ERROR:
            DeviceState->SetOutCodes(DSE_NOTSET, PRN_INCORRECT_DATE_TIME);
            break;
        case PE_PASSWORD_ERROR:
            DeviceState->SetOutCodes(PRN_INCORRECT_PASSWORD);
            break;
        case PE_NEED_COMMAND_BEGIN_SESSION:
            DeviceState->SetOutCodes(DSE_NOTSET, PRN_NEED_OPEN_SESSION);
            break;
        case PE_RESULT_OVERFLOW:
            DeviceState->SetOutCodes(DSE_NOTSET, PRN_TOO_BIG_RESULT);
            break;
        case PE_NO_CASH_ERROR:
            DeviceState->SetOutCodes(DSE_NOTSET, PRN_NO_CASH_FOR_OPERATION);
            break;
        case PE_PRINTER_NOT_SERTIFICATED:
            DeviceState->SetOutCodes(DSE_NOTSET, PRN_NEED_SERTIFICATION);
            break;
        case PE_Z_REPORT_NEEDED:
            DeviceState->SetOutCodes(DSE_NOTSET, PRN_NEED_CLOSE_SHIFT);
            break;
        case PE_FISCAL_PRINTER_FAILURE:
            DeviceState->SetOutCodes(PRN_HARDWARE_ERROR);
            break;
        case PE_PRINTER_NOT_READY:
            DeviceState->SetOutCodes(PRN_PRINTER_NOT_READY);
            break;
        case PE_PAPER_ABOUT_OVER:
            DeviceState->SetOutCodes(DSE_NOTSET, PRN_PAPER_NEAREND);
            break;
        case PE_PRINTER_NOT_FISCAL:
            DeviceState->SetOutCodes(DSE_NOTSET, PRN_NEED_FISCALIZATION);
            break;
        case PE_RECIEVE_ERROR:
            DeviceState->SetOutCodes(PRN_SOFTWARE_ERROR);
            break;
        case PE_PRINTER_STATE_ERROR:
            DeviceState->SetOutCodes(PRN_PRINTER_INCORRECT_STATE);
            break;
        case PE_NONCORRECT_PRINTER_STATE:
            DeviceState->SetOutCodes(PRN_PRINTER_INCORRECT_STATE);
            break;
        case PE_FISCAL_MEMORY_FAILURE:
            DeviceState->SetOutCodes(PRN_FISCAL_MEMORY_ERROR);
            break;
        case PE_FISCAL_MEMORY_NOT_INITIATED:
            DeviceState->SetOutCodes(PRN_FISCAL_MEMORY_INIT_REQUIRED);
            break;
        case PE_FISCAL_MEMORY_OVERFLOW:
            DeviceState->SetOutCodes(PRN_FISCAL_MEMORY_ERROR);
            break;
        case 0x35:
        case 0x36:
        case 0x37:
        case 0x38:
        case 0x39:
        case 0x3A:
        case 0x3B:
        case 0x3C:
        case 0x3D:
        case 0x3E:
        case 0x3F:
        case 0x40:
            DeviceState->SetOutCodes(PRN_EKLZ_ERROR);
            break;
    }
    errorscount ++;
    result = false;
  }


  DeviceState->Errors[errorscount] = END_OF_SEQUENCE;
  DeviceState->Statuses[statusescount] = END_OF_SEQUENCE;

  DeviceState->CriticalErrorsCount = errorscount;
  DeviceState->ErrorsCount = errorscount;
  DeviceState->StatusesCount = statusescount;

  return result;
}

