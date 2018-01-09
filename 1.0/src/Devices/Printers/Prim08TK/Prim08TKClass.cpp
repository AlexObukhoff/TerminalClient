//---------------------------------------------------------------------------
#include <math.h>
#include <time.h>
#pragma hdrstop
#include "Prim08TKClass.h"
#include "Prim08TKThread.h"
#include "globals.h"
#include "boost/format.hpp"
//---------------------------------------------------------------------------

#pragma package(smart_init)

CPrim08TKClass::CPrim08TKClass(int ComPort, int BaudRate, TLogClass* _Log) : CPrinter(ComPort,_Log, "Prim08TK")
{
  DataLength = -1;
  DataLengthIndex = -1;
  BeginByte = 0x02;
  EndByte = 0x03;
  CRCLength = 4;
  delete Port;
  COMParameters->Parity = NOPARITY;
  if (BaudRate > 0)
      COMParameters->BaudRate = BaudRate;
  COMParameters->timeout = 600;
  COMParameters->DtrControl = DTR_CONTROL_ENABLE;
  COMParameters->RtsControl = RTS_CONTROL_ENABLE;
  Port = new TComPort(COMParameters,Log,true);
  LoggingErrors = true;
  AutoOpenShift = true;
  //Fiscal = true;

  State = 0x00;
  OldState = 0x00;
  SubState = 0x00;
  OldSubState = 0x00;
  DeviceName = "Prim08TK";
  SendType = RecieveAnswer;

  CurrentCharsInString = 20;
  memset(DeviceState->Errors,0,100);
  DeviceState->ErrorsCount = 0;
  memset(DeviceState->Statuses,0,100);
  DeviceState->StatusesCount = 0;
  CommandNumber = 0x20;
  Position = 0;
  StringType = stDifferentLengthOfString;
  SetUserPassword(UserPassword);
  ExecCode = PE_OK;
  RestOfMoney = 0;
  NumberOfLastCheque = 0;
  ThreadLifeTime = 5*1000;
  //ThreadLifeTime = 500*1000;
  DSE_OK_Sensitive = false;
  DeviceState->ShiftState = 1;
  SessionOpened = true;
  BeginZReportNumber = 0;
  EndZReportNumber = 0;
}

CPrim08TKClass::~CPrim08TKClass()
{

}

void CPrim08TKClass::SendCommand()
{
  if (!Port->PortInit)
  {
      if (DeviceState)
          DeviceState->OutStateCode = DSE_NOTMOUNT;
      Log->Write("Port initialization problem. Can`t perform the command.");
      ChangeDeviceState();
      return;
  }
  DeviceThread = new CPrim08TKThread();

  Start();

  if (DeviceState)
      DeviceState->StateDescription = GetStatusDescription(DeviceState->ResultCode);
  delete DeviceThread;

  DeviceThread = NULL;
  SendEvent();
  if (DeviceState)
     StateExplanation(DeviceState->ResultCode);
  if (DeviceState)
     Log->Write((boost::format("Execution result = %1%") % GetStatusDescription(DeviceState->ResultCode).c_str()).str().c_str());
}

std::string CPrim08TKClass::GetStateDescription()
{
  return GetStatusDescription((BYTE)DeviceState->StateCode);
}

//==============================================================================

void CPrim08TKClass::SetUserPassword(char* password)
{
  tStringType OldType = StringType;

  StringType = stDifferentLengthOfString;
  memset((BYTE*)DOSLetters,0,CharsInString);
  memset((BYTE*)WinLetters,0,CharsInString);

  StringToOem(password);

  StringType = OldType;
  Move((BYTE *)&DOSLetters,(BYTE *)&Pass, strlen(DOSLetters)+1);
  PasswordLength = strlen(Pass);
  return;
}

BYTE CPrim08TKClass::CharToBYTE(BYTE ch)
{
  BYTE result = 0;

  if ((ch>=0x41)&&(ch<=0x46))
    result = ch - (BYTE)0x37;
  if ((ch>=0x30)&&(ch<=0x39))
    result = ch - (BYTE)0x30;
  return result;
}

WORD CPrim08TKClass::GetStatusWORD(BYTE *Buffer, int index)
{
  int i;
  WORD result = 0;

  for(i=0; i<=3; i++)
    result |= ((WORD)CharToBYTE(Buffer[index+i]))<<((3-i)*4);

  return result;
}

BYTE CPrim08TKClass::GetBYTE(BYTE *Buffer, int index)
{
  BYTE result = 0;

  result = (BYTE)CharToBYTE(Buffer[index])<<4;
  result |= ((BYTE)CharToBYTE(Buffer[index+1]));

  return result;
}

WORD CPrim08TKClass::GetWORD(BYTE *Buffer, int index)
{
  WORD result = 0;

  result = (WORD)CharToBYTE(Buffer[index])<<4;
  result |= ((WORD)CharToBYTE(Buffer[index+1]));
  result |= ((WORD)CharToBYTE(Buffer[index+2])<<12);
  result |= ((WORD)CharToBYTE(Buffer[index+3])<<8);

  return result;
}

char *CPrim08TKClass::TrimString(char *string)
{
  char *str;
  int i, len = strlen(string);
  int CountFromBegin = 0, CountFromEnd = 0;

  for (i = 0; i<=len; i++)
  {
    if ((BYTE)string[i] == 0x20)
      CountFromBegin++;
    else
      break;
  }
  for (i = len; i>=0; i--)
  {
    if ((BYTE)string[i] == 0x20)
      CountFromBegin++;
    else
      break;
  }

  len = len-(CountFromBegin+CountFromEnd);
  if (len == (signed)strlen(string))
    return string;
  str = new char[len+1];
  memset(str,0,len+1);
  Move((BYTE *)str, (BYTE *)&string[CountFromBegin], len);
  memset(string,0,strlen(string));
  Move((BYTE *)string, (BYTE *)str, len);
  delete [] str;
  return string;
}

char *CPrim08TKClass::AlignStringToCenter(char *str, int width)
{
  int i, iR, iL;
  char* text;
  int len = 0;

  str = TrimString(str);
  len = strlen(str);

  if (width>(CharsInString))
    width = CharsInString;

  if ( width <= len )
    return str;

  text = new char[len+1];
  memset(text,0, len+1);
  Move((BYTE *)text, (BYTE *)str, len);
  memset(str,0x20,width);
  str[width+1] = 0;

  i = width-len;
  iR = floor((float)(i/2));
  iL = width-len-iR;

  Move((BYTE *)&str[iL], (BYTE *)text, len);

  delete [] text;
  return str;
}

void CPrim08TKClass::SetByteToBuffer(BYTE *Buffer, char* Byte, int index)
{
  Move((BYTE*)Byte, (BYTE*)&Buffer[index], 2);
  return;
}

double CPrim08TKClass::GetCorrectResultR(double Number, int Offset)
{
  int i;
  long mult = 1;

  for (i=1; i<=Offset; i++)
    mult *= 10;

  Number *= mult;

  Number = (double)round(Number,0);

  return Number;
}

float CPrim08TKClass::GetCorrectMoney(float Price, float Quantity)
{
  return (float)round(Price*Quantity,2);
}

void CPrim08TKClass::ConvertFloatToString(double Number, char *Buffer, int Precision)
{
#define NumberLength 30
 //   int i,j = 0;
    int PointPosition, Sign;
    int pos = 0;
    int Width = 0;

    char *String;

    char LeftPart[NumberLength];
    memset(LeftPart,0,NumberLength);

    char RightPart[NumberLength];
    memset(RightPart,0,NumberLength);

    Number = round(Number,Precision);

    Width = Precision;
    String = _fcvt(Number, Width, &PointPosition, &Sign);

    if (strcmp(String,"")==0)
    {
        Buffer = "0\0";
        return;
    }

    if (Sign != 0)
    {
        Buffer[pos] = '-';
        pos++;
    }

    if (PointPosition > 0)
    {
        Move((BYTE*)String, (BYTE*)&Buffer[pos],PointPosition);
        pos += PointPosition;
        Buffer[pos] = '.';
        pos++;
        if ((signed)(strlen(String)-PointPosition)>Precision)
        {
            Move((BYTE*)&String[PointPosition],(BYTE*)&Buffer[pos],Precision);
            pos += Precision;
        }
        else
        {
            Move((BYTE*)&String[PointPosition],(BYTE*)&Buffer[pos],strlen(String)-PointPosition);
            pos += strlen(String)-PointPosition;
        }
    }
    else
    {
        strcpy((char*)&Buffer[pos],"0.");
        pos+=2;
        for(int i=PointPosition; i<0; i++)
        {
            strcpy((char*)&Buffer[pos],"0");
            pos++;
        }
        Move((BYTE*)String,(BYTE*)&Buffer[pos],strlen(String));
        pos += strlen(String);
        Buffer[pos] = 0;
    }
    Buffer[pos] = 0;

    return;
}

void CPrim08TKClass::PrepareFloat(float &Number, BYTE Precision)
{
  char str[NumberLength];
  memset(str, 0, NumberLength);

  ConvertFloatToString(Number, str, Precision);
  Number = (float)atof(str);
  return;
}

float CPrim08TKClass::GetMoneyFromBuffer(BYTE *Buffer, int &index)
{
    float result = 0;
    char ResultString[15];
    int pos = 0;

    memset(ResultString,0,15);
    while ((Buffer[index] != SPR)&&(pos <= 13))
    {
        ResultString[pos] = Buffer[index];
        pos++;
        index++;
    }

    Log->Write((boost::format("money = %1%") % ResultString).str().c_str());
    result = (float)atof(ResultString);
    //new 30-10-2007
    result = (int)result;
    index++;
    return result;
}

double CPrim08TKClass::round(double Number, int Precision)
{
  int i;
  long mult = 1;

  for (i=1; i<=Precision; i++)
    mult *= 10;

  Number *= mult;

  Number = (double)floor(Number);

  Number /= mult;
  return Number;
}

BYTE CPrim08TKClass::GetCommandNumber()
{
  if ( CommandNumber != 0xFF)
    CommandNumber++;
  else
    CommandNumber = 0x20;

  return CommandNumber;
}

void CPrim08TKClass::AttachStandardFrames(BYTE*& Buffer, int ETXIndex, char* CommandCode, bool SetDateTime)
{
  int pos = 0;
  int GlobalPosition = Position;

  Buffer[pos] = STX;
  pos++;

  // ������ ������ ������������
  Move((BYTE*)Pass, &(Buffer[pos]), 4);
  pos+=4;

  //������������� ����
  Buffer[pos] = GetCommandNumber();
  pos++;

  //��� ���������
  SetByteToBuffer(Buffer, CommandCode, pos);
  pos+=2;

  // ����������� �����
  Buffer[pos] = SPR;
  pos++;

  //����������� ���� � �����
  if (SetDateTime)
    SetDateTimeToBuffer(Buffer, pos);

  // ������ ����������� ����������� �����
  Buffer[ETXIndex-1] = SPR;
  // ������ ����� �������
  Buffer[ETXIndex] = ETX;

  SetBCC(Buffer,ETXIndex+1);

  Position = GlobalPosition;
}

void CPrim08TKClass::ClearBuffers()
{
  CommandSize = 0;
  AnswerSize = 0;
  memset(Command,0,DCBufferSize);
  memset(Answer,0,DCBufferSize);
  memset(DeviceState->Errors, 0, 100);
  memset(DeviceState->Statuses, 0, 100);
  memset((BYTE *)DOSLetters,0,CharsInString+1);
  memset((BYTE *)WinLetters,0,CharsInString+1);
  return;
}

void CPrim08TKClass::StringToOem(char *Text)
{
  char str[CharsInString+1];
  int len = strlen(Text);

  memset(str,0,CharsInString+1);

  if (len > CurrentCharsInString)
    len = CurrentCharsInString;

  switch (StringType)
  {
    case stDifferentLengthOfString:
      memset((BYTE *)WinLetters,0,CharsInString+1);
      memset((BYTE *)DOSLetters,0,CharsInString+1);
      break;
    case stEqualLengthOfString:
      memset((BYTE *)WinLetters,0,CharsInString+1);
      memset((BYTE *)DOSLetters,0,CharsInString+1);
      memset(WinLetters,0x20,CurrentCharsInString);
      memset(DOSLetters,0x20,CurrentCharsInString);
      break;
  };

  Move((BYTE *)Text, (BYTE *)&WinLetters, len);
  CharToOem(WinLetters,str);
  Move((BYTE *)str, (BYTE *)DOSLetters, strlen(str));
  return;
}

void CPrim08TKClass::OemToString(char *Text)
{
  char str[CharsInString+1];
  int len = strlen(Text);

  memset(str,0,CharsInString+1);

  if (len > CurrentCharsInString)
    len = CurrentCharsInString;

  switch (StringType)
  {
    case stDifferentLengthOfString:
      memset((BYTE *)WinLetters,0,CharsInString+1);
      memset((BYTE *)DOSLetters,0,CharsInString+1);
      break;
    case stEqualLengthOfString:
      memset((BYTE *)WinLetters,0,CharsInString+1);
      memset((BYTE *)DOSLetters,0,CharsInString+1);
      memset(WinLetters,0x20,CurrentCharsInString);
      memset(DOSLetters,0x20,CurrentCharsInString);
      break;
  };

  Move((BYTE *)Text, (BYTE *)&DOSLetters, len);
  OemToChar(DOSLetters,str);
  Move((BYTE *)str, (BYTE *)WinLetters, strlen(str));
  return;
}

void CPrim08TKClass::SetBCC(BYTE *Buffer, unsigned int index)
{
  WORD BCC = 0;
  char* CalcBCC = new char[5];

  memset(CalcBCC,0,5);
  for(unsigned int i = 0; i<index; i++)
    BCC += Buffer[i];
  sprintf(CalcBCC, "%04X", BCC);

  Buffer[index] = CalcBCC[2];
  Buffer[index+1] = CalcBCC[3];
  Buffer[index+2] = CalcBCC[0];
  Buffer[index+3] = CalcBCC[1];

  delete [] CalcBCC ;

  return;
}

void CPrim08TKClass::WriteErrorsToLog()
{
  AnsiString ErrorString;
  char str[100];

  if (DeviceState == NULL)
      return;

  for(int i=0; i<DeviceState->StatusesCount; i++)
    if (DeviceState->Statuses[i] != END_OF_SEQUENCE)
    {
        ErrorString =GetErrorExplanationString((char*)str, DeviceState->Statuses[i]);
        //ErrorString = str;
        Log->Write((boost::format("������ %1%") % ErrorString.c_str()).str().c_str());
    }
    else
       break;

  for(int i=0; i<DeviceState->CriticalErrorsCount; i++)
    if (DeviceState->Errors[i] != END_OF_SEQUENCE)
    {
        ErrorString =GetErrorExplanationString((char*)str, DeviceState->Errors[i]);
        //ErrorString = str;
        Log->Write((boost::format("������ %1%") % ErrorString.c_str()).str().c_str());
    }
    else
       break;
}


char* CPrim08TKClass::GetErrorExplanationString(char *DestPointer, BYTE Code)
{
  switch (Code)
  {
    // ���������� ������
    case PE_OK:
    {
      strcpy(DestPointer,"OK");
      break;
    }
    case PE_ERROR_OF_STATUS_RECEIVE:
    {
      strcpy(DestPointer,"������ ����� ������� ����������� ����������.");
      break;
    }
    case PE_HARDWARE_ERROR:
    {
      strcpy(DestPointer,"���������� ������.");
      break;
    }
    case PE_WORKING_MEMORY_ERROR:
    {
      strcpy(DestPointer,"������ ����������� ������.");
      break;
    }
    case PE_FISCAL_MEMORY_ERROR:
    {
      strcpy(DestPointer,"������ ���������� ������.");
      break;
    }
                case PE_FISCAL_MODE_NOT_SET:
    {
      strcpy(DestPointer,"���������� ����� �� ����������.");
      break;
    }
                case PE_FISCAL_MEMORY_ABOUT_OVERFLOW:
    {
      strcpy(DestPointer,"���������� ������ ������ � ����������.");
      break;
    }
                case PE_FISCAL_MEMORY_OVERFLOW:
    {
      strcpy(DestPointer,"���������� ������ �����������.");
      break;
    }
                case PE_REGISTRATION_COUNT_OVERFLOW:
    {
      strcpy(DestPointer,"���������� ��������������� ���������.");
      break;
    }
                case PE_HARDWARE_SERIAL_NUMBER_NOT_SET:
    {
      strcpy(DestPointer,"�������� �� �������� �������� �����.");
      break;
    }

                case PE_NEED_TO_CLOSE_SHIFT:
    {
      strcpy(DestPointer,"���������� ������� �����.");
      if (DeviceState)
          DeviceState->ShiftState = 0;
      SessionOpened = false;
      break;
    }
                case PE_PREVIOUS_COMMAND_UNRECOGNIZED:
    {
      strcpy(DestPointer,"���������� ������� �� ����������.");
      break;
    }
                case PE_PREVIOUS_COMMAND_NOT_COMPLETE:
    {
      strcpy(DestPointer,"���������� ������� �� ���������.");
      break;
    }
                case PE_INSPECTOR_PASSWORD_INCORRECT:
    {
      strcpy(DestPointer,"�������� ������ ����������.");
      break;
    }
                case PE_SESSION_NOT_CLOSED:
    {
      strcpy(DestPointer,"����� �� ������");
      break;
    }
                case PE_FISCAL_MODE:
    {
      strcpy(DestPointer,"����������� ��������� � ���������� ������.");
      break;
    }
                case PE_TRAIN_MODE:
    {
      strcpy(DestPointer,"����������� ��������� � ��������������� ������.");
      break;
    }
                case PE_DOC_BUFFER_ABOUT_OVERFLOW:
    {
      strcpy(DestPointer,"����� ��������� ������ � �����.");
      break;
    }
                case PE_SHIFT_CLOSED:
    {
      strcpy(DestPointer,"����� �������.");
      break;
    }

                case PE_INCORRECT_COMMAND_FORMAT:
    {
      strcpy(DestPointer,"�������� ������ ���������.");
      break;
    }
                case PE_INCORRECT_COMMAND_FIELD:
    {
      strcpy(DestPointer,"�������� ������ ����.");
      break;
    }
                case PE_DATE_TIME_ERROR:
    {
      strcpy(DestPointer,"�������� ����/�����. ���������� ���������� ���������� ����/�����.");
      break;
    }
                case PE_INCORRECT_COMMAND_NUMBER:
    {
      strcpy(DestPointer,"��� ������� � ����� �������.");
      break;
    }
                case PE_NEED_COMMAND_BEGIN_SESSION:
    {
      strcpy(DestPointer,"���������� ������� ������ ������.");
      break;
    }
                case PE_TIME_OVERFLOW:
    {
      strcpy(DestPointer,"����� ���������� ������ ��� �� 24 ����.");
      break;
    }
                case PE_STRING_FIELD_OVERFLOW:
    {
      strcpy(DestPointer,"��������� ������������ ����� ���������� ����.");
      break;
    }
                case PE_COMMAND_LENGTH_OVERFLOW:
    {
      strcpy(DestPointer,"��������� ������������ ����� ���������.");
      break;
    }
                case PE_INCORRECT_OPERATION:
    {
      strcpy(DestPointer,"������������ ��������.");
      break;
    }
                case PE_FIELD_VALUE_ERROR:
    {
      strcpy(DestPointer,"�������� ���� ��� ���������.");
      break;
    }
                case PE_INCORRECT_COMMAND_WITH_CURRENT_STATE:
    {
      strcpy(DestPointer,"��� ������ ��������� ��������� ��� ������� �� ���������.");
      break;
    }
                case PE_NEEDED_STRING_FIELD_ERROR:
    {
      strcpy(DestPointer,"������������ ��������� ���� ����� ������� �����.");
      break;
    }
                case PE_MONEY_COUNT_ERROR:
    {
      strcpy(DestPointer,"������������ ��������� ��������.");
      break;
    }
                case PE_BACK_OPERATION_ERROR_WITHOUT_DIRECT_OPERATION:
    {
      strcpy(DestPointer,"�������� �������� ���������� ��-�� ���������� ������.");
      break;
    }
                case PE_NO_CASH_ERROR:
    {
      strcpy(DestPointer,"��� ������� �������� ��� ���������� ��������.");
      break;
    }
                case PE_BACK_OPERATION_RESULT_OVERFLOW:
    {
      strcpy(DestPointer,"�������� �������� ��������� ���� �� ������ ��������.");
      break;
    }
                case PE_PRINTER_NOT_SERTIFICATED:
    {
      strcpy(DestPointer,"���������� ��������� ������������ ��������.");
      break;
    }
                case PE_Z_REPORT_NEEDED:
    {
      if (DeviceState)
          DeviceState->ShiftState = 0;
      SessionOpened = false;
      strcpy(DestPointer,"���������� ��������� Z-����� (�������� �����).");
      break;
    }

                case PE_PRINTER_NOT_READY:
    {
      strcpy(DestPointer,"������� �� �����.");
      break;
    }
                case PE_PAPER_ABOUT_OVER:
    {
      strcpy(DestPointer,"������ ������ � �����.");
      break;
    }
                case PE_PRINTER_NOT_FISCAL:
    {
      strcpy(DestPointer,"���������� �������� ������������.");
      break;
    }
                case PE_PAPER_OVER:
    {
      strcpy(DestPointer,"������ �����������.");
      break;
    }
                case PE_FISCAL_COUNT_OVERFLOW:
    {
      strcpy(DestPointer,"���������� ������������ �����������.");
      break;
    }
                case PE_PRINTER_HAS_SERTIFICATED:
    {
      strcpy(DestPointer,"������� ��� ��������������.");
      break;
    }
                case PE_PAYMENT_TYPE_NUMBER_INCORRECT:
    {
      strcpy(DestPointer,"�������� ����� ���� ������.");
      break;
    }
                case PE_TIMEOUT_OF_RECIEVE_ERROR:
    {
      strcpy(DestPointer,"���������� ����-���� �����.");
      break;
    }
                case PE_RECIEVE_ERROR:
    {
      strcpy(DestPointer,"������ �����.");
      break;
    }
                case PE_PRINTER_STATE_ERROR:
    {
      strcpy(DestPointer,"�������� ��������� ������������.");
      break;
    }
                case PE_OPERATION_COUNT_ERROR:
    {
      strcpy(DestPointer,"������� ����� �������� � ���������.");
      break;
    }
                case PE_BEGIN_SHIFT_COMMAND_NEED:
    {
      strcpy(DestPointer,"���������� �������� �����.");
      break;
    }
                case PE_PAYMENT_NUMBER_INCORRECT:
    {
      strcpy(DestPointer,"�������� ����� ���� �������.");
      break;
    }
                case PE_SHIFT_ALREADY_OPENED:
    {
      if (DeviceState)
          DeviceState->ShiftState = 1;
      SessionOpened = true;
      strcpy(DestPointer,"����� ��� �������.");
      break;
    }
                case PE_DATE_ERROR:
    {
      strcpy(DestPointer,"�������� ����.");
      break;
    }
                case PE_FISCAL_MEMORY_FAILURE:
    {
      strcpy(DestPointer,"���� ���������� ������.");
      break;
    }
                case PE_OPERATION_TIME_ERROR:
    {
      strcpy(DestPointer,"�������� ���� ��������.");
      break;
    }
                case PE_FISCAL_MEMORY_NOT_INITIATED:
    {
      strcpy(DestPointer,"���������� ������������� ���������� ������.");
      break;
    }

    // ������ ���������� �����
    case PE_FISCAL_MEMORY_CHECKSUM_ERROR:
    {
      strcpy(DestPointer,"������ ����������� ����� ���������� ������.");
      break;
    }
    case PE_PASSWORD_ERROR:
    {
      strcpy(DestPointer,"�������� ������ ������������.");
      break;
    }
    case PE_WORKING_MEMORY_CHECKSUM_ERROR:
    {
      strcpy(DestPointer,"������ ����������� ����� ������� ������.");
      break;
    }
    case PE_BATTERY:
    {
      strcpy(DestPointer,"��������� ���������, �������� ������� ������ � ������.");
      break;
    }
    case PE_DATA_INCORRECT:
    {
      strcpy(DestPointer,"������ ������ ��� ������.");
      break;
    }
    case PE_INAPPROPRIATE_COMMAND:
    {
      strcpy(DestPointer,"������� ����� ���������, �� ��� ������ ��������� ��������.");
      break;
    }
    case PE_RESULT_OVERFLOW:
    {
      strcpy(DestPointer,"������������ ������.");
      break;
    }
    case PE_DOCUMENT_OPENED:
    {
      strcpy(DestPointer,"������ ���������� ��������.");
      break;
    }
    case PE_RECEIPT_OPENED:
    {
      strcpy(DestPointer,"������ ���������� ���.");
      break;
    }
    case PE_RECEIPT_NOT_OPENED:
    {
      strcpy(DestPointer,"���������� ��� �� ������.");
      break;
    }

    // ������ ����������� ��������
    case PE_FISCAL_PRINTER_FAILURE:
    {
      strcpy(DestPointer,"������ ��� ������������� ����������� ��������.");
      break;
    }
    case 0xFF:
    case PE_LINK_ERROR:
    {
      strcpy(DestPointer,"�������� ����� ���������� � ���������.");
      break;
    }
    case PE_BUFFER_OVERFLOW:
    {
      strcpy(DestPointer,"������������ ������ ��������.");
      break;
    }
    case PE_DOCUMENT_PRESENT:
    {
      strcpy(DestPointer,"��������� ���������� ��������.");
      break;
    }

    // ������ ������
    case PE_WRITEFILE_ERROR:
    case PE_READFILE_ERROR:
    {
      strcpy(DestPointer,"�������� �����.");
      break;
    }
    case PE_DIFFERBYTE_ERROR:
    {
      strcpy(DestPointer,"���������� ����������� �����. �������� �����.");
      break;
    }
    case PE_BCC_ERROR:
    {
      strcpy(DestPointer,"������ ����������� �����.");
      break;
    }

//==============================================================================
    case PE_NONCORRECT_PRINTING_BUFFER:
      strcpy(DestPointer,"PE_NONCORRECT_PRINTING_BUFFER");break;
    case PE_NONCORRECT_G_FIELD:
      strcpy(DestPointer,"PE_NONCORRECT_G_FIELD");break;
    case PE_NOT_SPACE_FOR_ADDING_DEPART:
      strcpy(DestPointer,"PE_NOT_SPACE_FOR_ADDING_DEPART");break;
    case PE_DEPARTS_INDEX_ALREADY_EXIST:
      strcpy(DestPointer,"PE_DEPARTS_INDEX_ALREADY_EXIST");break;
    case PE_DEPART_CANT_DELETED:
      strcpy(DestPointer,"PE_DEPART_CANT_DELETED");break;
    case PE_DEPARTS_INDEX_NOT_FOUND:
      strcpy(DestPointer,"PE_DEPARTS_INDEX_NOT_FOUND");break;
    case PE_NONCORRECT_START_SYMBOL:
      strcpy(DestPointer,"PE_NONCORRECT_START_SYMBOL");break;
    case PE_UNKNOWN_ANSWER_FROM_EKLZ:
      strcpy(DestPointer,"PE_UNKNOWN_ANSWER_FROM_EKLZ");break;
    case PE_UNKNOWN_COMMAND_EKLZ:
      strcpy(DestPointer,"PE_UNKNOWN_COMMAND_EKLZ");break;
    case PE_UNKNOWN_STATE_EKLZ:
      strcpy(DestPointer,"PE_UNKNOWN_STATE_EKLZ");break;
    case PE_TIMEOUT_RECIEVING_EKLZ:
      strcpy(DestPointer,"PE_TIMEOUT_RECIEVING_EKLZ");break;
    case PE_TIMEOUT_TRANSMITING_EKLZ:
      strcpy(DestPointer,"PE_TIMEOUT_TRANSMITING_EKLZ");break;
    case PE_NONCORRECT_CRC_ANSWER_FROM_EKLZ:
      strcpy(DestPointer,"PE_NONCORRECT_CRC_ANSWER_FROM_EKLZ");break;
    case PE_NONCORRECT_STATE_EKLZ:
      strcpy(DestPointer,"PE_NONCORRECT_STATE_EKLZ");break;
    case PE_NO_FREE_SPACE_EKLZ:
      strcpy(DestPointer,"PE_NO_FREE_SPACE_EKLZ");break;
    case PE_NONCORRECT_CRC_COMMAND_EKLZ:
      strcpy(DestPointer,"PE_NONCORRECT_CRC_COMMAND_EKLZ");break;
    case PE_EKLZ_NOT_FOUND:
      strcpy(DestPointer,"PE_EKLZ_NOT_FOUND");break;
    case PE_DATA_EKLZ_NOT_EXIST:
      strcpy(DestPointer,"PE_DATA_EKLZ_NOT_EXIST");break;
    case PE_DATA_EKLZ_UNSYNCHRONIZED:
      strcpy(DestPointer,"PE_DATA_EKLZ_UNSYNCHRONIZED");break;
    case PE_NONCORRECT_PRINTER_STATE:
      strcpy(DestPointer,"�������� ��������� ��������");break;
    /*case PE_ERROR_RIK_STATE:
      strcpy(DestPointer,"PE_ERROR_RIK_STATE");break;
    case PE_NONCORRECT_DATETIME_IN_EKLZ_COMMAND:
      strcpy(DestPointer,"PE_NONCORRECT_DATETIME_IN_EKLZ_COMMAND");break;
    case PE_TIME_OF_EKLZ_EXPIRED:
      strcpy(DestPointer,"PE_TIME_OF_EKLZ_EXPIRED");break;
    case PE_EKLZ_OVERFLOW:
      strcpy(DestPointer,"PE_EKLZ_OVERFLOW");break;
    case PE_ACTIVATION_TIMES_EXPIRED:
      strcpy(DestPointer,"PE_ACTIVATION_TIMES_EXPIRED");break;
    case PE_NEED_PRINTING_SKL:
      strcpy(DestPointer,"PE_NEED_PRINTING_SKL");break;
    case PE_ERROR_SKL_STATE:
      strcpy(DestPointer,"PE_ERROR_SKL_STATE");break;*/
    case PE_ERROR_PRINT_STRING_CREATING:
      strcpy(DestPointer,"PE_ERROR_PRINT_STRING_CREATING");break;

    default:
    {
      AnsiString str = "����������� ��� = "+AnsiString(Code);
      strcpy(DestPointer,str.c_str());
      break;
    }
  }

  return DestPointer;
}


void CPrim08TKClass::SetDateToBuffer(BYTE *Buffer, int pos)
{
  struct tm *DateTime;
  int Year = 0;
  time_t Time;
  char DateBuffer[7] = {NULL};

  time(&Time);
  DateTime = localtime( &Time );
  if (DateTime->tm_year >= 100)
    Year = DateTime->tm_year - 100;
  else
    Year = DateTime->tm_year;

  sprintf(DateBuffer, "%02d%02d%02d", DateTime->tm_mday, DateTime->tm_mon+1, Year);
  Move((BYTE *)&DateBuffer[0], (BYTE *)&Buffer[pos], 6);
  return;
}

void CPrim08TKClass::SetTimeToBuffer(BYTE *Buffer, int pos)
{
  struct tm *DateTime;
  time_t Time;
  char TimeBuffer[5] = {NULL};

  time(&Time);
  DateTime = localtime( &Time );

  sprintf(TimeBuffer, "%02d%02d",DateTime->tm_hour, DateTime->tm_min);
  Move((BYTE *)&TimeBuffer[0], (BYTE *)&Buffer[pos], 4);
  return;
}

char *CPrim08TKClass::GetDateFromBuffer(BYTE *Buffer, int pos)
{
  char temp[3];
  temp[2] = 0;
  memset((BYTE *)DOSLetters,0,CharsInString);
  temp[0]=DOSLetters[0] = Buffer[pos+0];
  temp[1]=DOSLetters[1] = Buffer[pos+1];
  DateTime.Day = atoi(temp);
  DOSLetters[2] = '.';
  temp[0]=DOSLetters[3] = Buffer[pos+2];
  temp[1]=DOSLetters[4] = Buffer[pos+3];
  DateTime.Month = atoi(temp);
  DOSLetters[5] = '.';
  temp[0]=DOSLetters[6] = Buffer[pos+4];
  temp[1]=DOSLetters[7] = Buffer[pos+5];
  DateTime.Year = 2000+atoi(temp);
  DOSLetters[8] = 0;
  return DOSLetters;
}

char *CPrim08TKClass::GetTimeFromBuffer(BYTE *Buffer, int pos)
{
  char temp[3];
  temp[2] = 0;
  memset((BYTE *)WinLetters,0,CharsInString);
  temp[0]=WinLetters[0] = Buffer[pos+0];
  temp[1]=WinLetters[1] = Buffer[pos+1];
  DateTime.Hour = atoi(temp);
  WinLetters[2] = ':';
  temp[0]=WinLetters[3] = Buffer[pos+2];
  temp[1]=WinLetters[4] = Buffer[pos+3];
  DateTime.Min = atoi(temp);
  WinLetters[5] = 0;
  return WinLetters;
}

void CPrim08TKClass::SetDateTimeToBuffer(BYTE *Buffer, int pos)
{
  SetDateToBuffer(Buffer, pos);
  Position = pos+6;
  Buffer[Position] = SPR;
  Position++;
  SetTimeToBuffer(Buffer, Position);
  Position += 4;
  Buffer[Position] = SPR;
  Position++;
}

void CPrim08TKClass::SetTextToBuffer(BYTE* Buffer, int index, char *Text)
{
  int len = strlen(Text);

  StringToOem(Text);
  len = strlen(DOSLetters);
  if (len != 0)
  {
    Move((BYTE *)DOSLetters,&Buffer[index],len);
    Position = index+len;
  }
  Buffer[Position] = SPR;
  Position++;
}

void CPrim08TKClass::SetQuantityToBuffer(BYTE* Buffer, int index, float Number)
{
  ConvertFloatToString(Number, DOSLetters, 3);
  int len = strlen(DOSLetters);
  if (len>14)
    len = 14;
  Move((BYTE *)DOSLetters,&Buffer[index],len);
  Position+=len;
  Buffer[Position] = SPR;
  Position++;
}

void CPrim08TKClass::SetPriceToBuffer(BYTE* Buffer, int index, float Number)
{
  ConvertFloatToString(Number, DOSLetters, 2);
  int len = strlen(DOSLetters);
  if (len>14)
    len = 14;
  Move((BYTE *)DOSLetters,&Buffer[index],len);
  Position+=len;
  Buffer[Position] = SPR;
  Position++;
}

void CPrim08TKClass::SetMoneyToBuffer(BYTE* Buffer, int index, float Number)
{
  SetPriceToBuffer(Buffer, index, Number);
}
//==============================================================================
std::string CPrim08TKClass::GetStatusDescription(BYTE StatusCode)
{
    std::string result;
    result = GetErrorExplanationString((char *)result.c_str(), StatusCode);
    return StateDescr = result;
}


void CPrim08TKClass::SendPacket(BYTE*& command, char* CommandCode, BYTE* data, int datalen, bool SetDateTime)
{
  if (!Port->PortInit)
    return;
  try
  {
    ClearCommand();
    ClearAnswer();
    CommandSize = 0;
    int pos = 0;

    if (SetDateTime)
        pos = FirstDataByteAfterFrames;
    else
        pos = FirstDataByteAfterFramesWithOutDateTime;

    int count = pos+1;

    if ((data != NULL)&&(datalen != 0))
        Move(data,&command[pos],datalen);
    count += datalen;

    AttachStandardFrames(command, count-1, (char*)CommandCode, SetDateTime);

    CommandSize = count+5-1;
  }
  __finally
  {
  }
}

bool CPrim08TKClass::GetStateBool()
{
    bool result = false;
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return false;
    }

    Log->Write("GetState()");
    //AnsiString CommandCode = "03";
    AnsiString CommandCode = "97";
    SendPacket(Command,CommandCode.c_str());
    bool old = DSE_OK_Sensitive;
    DSE_OK_Sensitive = true;
    SendCommand();

    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;
    else
        if (DeviceState)
            WriteErrorsToLog();

    DSE_OK_Sensitive = old;

    if ((DeviceState)&&(DeviceState->ResultCode == PE_LINK_ERROR))
        return result = false;

    if ((DeviceState)&&(DeviceState->ShiftState == 0))
    {
        if (AutoOpenShift == false)
        {
            //PrinterEnable = false;
            PrinterEnable = true;
            SessionOpened = false;
            //return false;
            return true;
        }

        Log->Write("ShiftState == 0, now is closing shift and opening the new one");
        //PrintZReport();
        //=====new 19-06-2007======
        if (ZReportInBuffer)
        {
            CommandZReportIntoBuffer();
        }
        else
        {
            PrintZReport();
        }
        //=====new 02-05-2007======
        if (DeviceState)
            CommandSetDateTimeOne();
        if (DeviceState)
            CommandSetDocumentsParameters();
        //=========================
        if (!DeviceState)
            return false;

        PrinterEnable = true;
        CommandCode = "97";
        SendPacket(Command,CommandCode.c_str());
        bool old = DSE_OK_Sensitive;
        DSE_OK_Sensitive = true;
        SendCommand();
        if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        {
            if (DeviceState->OutStateCode == DSE_HARDWARE_ERROR)
            {
                DeviceState->OutStateCode == DSE_HARDWARE_ERROR;
                PrinterEnable = result = false;
            }
            else
            {
                //DeviceState->OutStateCode == DSE_OK;
                PrinterEnable = result = true;
            }
        }
        else
        {
            if (DeviceState)
                WriteErrorsToLog();
        }
        DSE_OK_Sensitive = old;
    }

    //SendEvent();
    return result;
}

bool CPrim08TKClass::CommandXReport()
{
    bool result = false;
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }

    Log->Write("CommandXReport()");
    DWORD OldThreadLifeTime = ThreadLifeTime;
    ThreadLifeTime = 50*1000;

    DWORD OldTimeOut = Port->timeout;
    Port->timeout = 60000;

    AnsiString CommandCode = "30";
    SendPacket(Command,CommandCode.c_str());
    SendCommand();
    //if ((DeviceState)&&(DeviceState->AnswerSize != -1))
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;

    if (Port)
        Port->timeout = OldTimeOut;
    ThreadLifeTime = OldThreadLifeTime;
    return result;
}

bool CPrim08TKClass::CommandZReport()
{
    bool result = false;

    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }

    Log->Write("CommandZReport()");
    DWORD OldThreadLifeTime = ThreadLifeTime;
    ThreadLifeTime = 50*1000;

    DWORD OldTimeOut = Port->timeout;
    Port->timeout = 60000;

    AnsiString CommandCode = "31";
    SendPacket(Command,CommandCode.c_str());
    SendCommand();
    //if ((DeviceState)&&(DeviceState->AnswerSize != -1))
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
    {
        if (DeviceState)
            DeviceState->ShiftState = 1;
        SessionOpened = true;
        result = true;
    }

    ThreadLifeTime = OldThreadLifeTime;
    if (Port)
        Port->timeout = OldTimeOut;
    return result;
}

bool CPrim08TKClass::CommandOpenCheque(char* Type, AnsiString text)
{
    bool result = false;
    memset(Data,0,CPC_BufferSize);
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }

    char MessageCode[3] = "10";
    char* Description = " ";

    Position = 0;
    // ��� ���������
    Move((BYTE *)Type,&Data[Position],2);
    Position+=2;
    Data[Position]=SPR;Position++;

    /* ������� �� �����
    Data[Position]=SPR;Position++;
    // ����� ��� ����������
    Data[Position]=SPR;Position++;
    // ����� ����� �� �����
    Data[Position]=SPR;Position++;*/

    //������ ������
    SetTextToBuffer(Data, Position,Description);
    //��� �����
    SetTextToBuffer(Data, Position,Description);
    //����� �����
    SetTextToBuffer(Data, Position,Description);

    //���������� �����
    strcpy((char*)&Data[Position],"01");
    Position+=2;
    Data[Position]=SPR;Position++;

    // ����� ����� ������
    SetTextToBuffer(Data, Position,Description);

    //new 21-12-2006
    if (text != "")
    {
        int Limit = 256;
        int size = text.Length();
        if (size > Limit)
        {
            size = Limit;
            Log->Write((boost::format("����� ������ ��������� 256 ��������, ������ ����� ��������: %1%") % text.SubString(size,text.Length()-1).c_str()).str().c_str());
            text = text.SubString(0,size-1);
        }
        char TextBuffer[CPC_BufferSize];
        memset(TextBuffer,0,CPC_BufferSize);
        //Log->Write("Text to printer: "+ text);
//        TStringList* strings = new TStringList();
        AnsiString delim = "\r\n";
        while(true)
        {
          int pos = text.Pos(delim);
          if (pos == 0)
            break;
          text = text.Delete(pos,2);
          text = text.Insert("|",pos);
        }
        AnsiString _text = "";
        CharToOem(text.c_str(),TextBuffer);
        Move(TextBuffer,&Data[Position],text.Length());
        Position += text.Length();
        Data[Position] = SPR;
        Position++;
    }

    SendPacket(Command,(char*)MessageCode,Data,Position);
    SendCommand();
    //if ((DeviceState)&&(DeviceState->AnswerSize != -1))
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;

    return result;
}

bool CPrim08TKClass::CommandPrintSale(char *Description, float Quantity, float Price, char* Unit, char* Section, AnsiString text)
{
    bool result = false;
    memset(Data,0,CPC_BufferSize);
    Position = 0;

    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }

    char MessageCode[3] = "11";
    char* empty = " ";

    //������������ ������
    SetTextToBuffer(Data,Position,Description);
    //��� ������ ����������
    //Command[Position]=SPR;Position++;
    SetTextToBuffer(Data,Position,empty);
    //����
    SetPriceToBuffer(Data,Position, Price);
    //����������
    SetQuantityToBuffer(Data,Position, Quantity);
    //������� ���������
    SetTextToBuffer(Data,Position,Unit);
    //��� ������
    SetTextToBuffer(Data,Position,"01");
    //������
    SetTextToBuffer(Data,Position,Section);

    //new 21-12-2006
    if (text != "")
    {
        char TextBuffer[255];
        memset(TextBuffer,0,255);
        //Log->Write("Text to printer: "+ text);
//        TStringList* strings = new TStringList();
        AnsiString delim = "\r\n";
        while(true)
        {
          int pos = text.Pos(delim);
          if (pos == 0)
            break;
          text = text.Delete(pos,2);
          text = text.Insert("|",pos);
        }
        AnsiString _text = "";
        CharToOem(text.c_str(),TextBuffer);
        Move(TextBuffer,&Data[Position],text.Length());
        Position += text.Length();
        Data[Position] = SPR;
        Position++;
    }

    SendPacket(Command,(char*)MessageCode,Data,Position,false);
    SendCommand();
    //if ((DeviceState)&&(DeviceState->AnswerSize != -1))
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;

    return result;
}

bool CPrim08TKClass::CommandPrintSummary()
{
    bool result = false;
    memset(Data,0,CPC_BufferSize);
    Position = 0;
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }

    char MessageCode[3] = "12";

    SendPacket(Command,(char*)MessageCode,NULL,0,false);
    SendCommand();
    //if ((DeviceState)&&(DeviceState->AnswerSize != -1))
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;

    return result;
}

bool CPrim08TKClass::CommandPrintTotalOfCheque(float Money, char *PaymentDescriptor, char *CardNumber)
{
    bool result = false;
    memset(Data,0,CPC_BufferSize);
    Position = 0;
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }

    char MessageCode[3] = "13";

    //��� ������
    SetTextToBuffer(Data,Position,PaymentDescriptor);
    //����� �� ����������
    SetMoneyToBuffer(Data,Position,Money);
    //����� �����
    SetTextToBuffer(Data,Position,CardNumber);

    SendPacket(Command,(char*)MessageCode, Data, Position, false);
    SendCommand();
    //if ((DeviceState)&&(DeviceState->AnswerSize != -1))
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;

    return result;
}

bool CPrim08TKClass::CommandCloseCheque()
{
    bool result = false;
    memset(Data,0,CPC_BufferSize);
    Position = 0;
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }

    DWORD OldTimeOut = Port->timeout;
    Port->timeout = 3000;

    char MessageCode[3] = "14";

    SendPacket(Command,(char*)MessageCode, NULL, 0, false);
    SendCommand();
    Port->timeout = OldTimeOut;
    //if ((DeviceState)&&(DeviceState->AnswerSize != -1))
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;

    return result;
}

bool CPrim08TKClass::CommandSetDateTimeOne()
{
    bool result = false;
    memset(Data,0,CPC_BufferSize);
    Position = 0;
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }
    Log->Write("CommandSetDateTimeOne()");
    char MessageCode[3] = "42";

    SendPacket(Command,(char*)MessageCode,NULL,0,true);
    SendCommand();
    //if ((DeviceState)&&(DeviceState->AnswerSize != -1))
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;

    return result;
}

bool CPrim08TKClass::CommandGetDateTime()
{
    bool result = false;
    memset(Data,0,CPC_BufferSize);
    Position = 0;
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }
    Log->Write("CommandGetDateTime()");
    char MessageCode[3] = "43";

    SendPacket(Command,(char*)MessageCode);
    SendCommand();
    //if ((DeviceState)&&(DeviceState->AnswerSize != -1))
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;

    memset((BYTE *)DOSLetters,0,CharsInString);
    memset((BYTE *)WinLetters,0,CharsInString);

    if (!result)
    {
      strncpy(DOSLetters,"00.00.00",8);
      strncpy(WinLetters,"00:00",5);
      return result;
    }

    Position = 29;
    GetDateFromBuffer(Answer,Position);// result in DOSLetters

    Position = 36;
    GetTimeFromBuffer(Answer,Position);// result in WinLetters

    return result;
}

bool CPrim08TKClass::CommandGetDateTime(char *Date, char *Time)
{
    bool result = false;
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }

    result = CommandGetDateTime();

    if (!result)
      return false;

    Move((BYTE *)&DOSLetters, (BYTE *)Date, strlen(DOSLetters));
    Move((BYTE *)&WinLetters, (BYTE *)Time, strlen(WinLetters));

    return result;
}


bool CPrim08TKClass::IsItYou()
{
    if(Port)
        Port->ClearCOMPort();
    GetState();
    if ((DeviceState)&&(DeviceState->AnswerSize > 1))
        return true;
    else
        return false;
}

bool CPrim08TKClass::CommandGetLastChequeNumber()
{
    bool result = false;
    memset(Data,0,CPC_BufferSize);
    Position = 0;
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }
    Log->Write("CommandGetLastChequeNumber()");
    char MessageCode[3] = "35";

    SendPacket(Command,(char*)MessageCode);
    SendCommand();
    //if ((DeviceState)&&(DeviceState->AnswerSize != -1))
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;

    if (!result)
      return result;

    // ��������� ����� ���������� ����
    NumberOfLastCheque = GetWORD(Answer, FirstDataByteAfterFramesInAnswer);

    return result;
}

bool CPrim08TKClass::CommandCancelCheque()
{
    bool result = false;
    memset(Data,0,CPC_BufferSize);
    Position = 0;
    if (Port == NULL)
        return result;
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }
    Log->Write("CommandCancelCheque()");
    char MessageCode[3] = "17";

    SendPacket(Command,(char*)MessageCode);
    SendCommand();
    //if ((DeviceState)&&(DeviceState->AnswerSize != -1))
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;

    return result;
}

bool CPrim08TKClass::CommandAddMoneyToCash(float Money)
{
    bool result = false;
    memset(Data,0,CPC_BufferSize);
    Position = 0;
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }
    Log->Write("CommandAddMoneyToCash");
    char MessageCode[3] = "32";

    //����� ����� � �����
    SetMoneyToBuffer(Data,Position,Money);

    SendPacket(Command,(char*)MessageCode,Data,Position);
    SendCommand();
    //if ((DeviceState)&&(DeviceState->AnswerSize != -1))
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;

    return result;
}

bool CPrim08TKClass::CommandTakeMoneyFromCash(float Money)
{
    bool result = false;
    memset(Data,0,CPC_BufferSize);
    Position = 0;
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }
    Log->Write("CommandTakeMoneyFromCash");
    char MessageCode[3] = "33";

    //����� ����� � �����
    SetMoneyToBuffer(Data,Position,Money);

    SendPacket(Command,(char*)MessageCode,Data,Position);
    SendCommand();
    //if ((DeviceState)&&(DeviceState->AnswerSize != -1))
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;

    return result;
}

bool CPrim08TKClass::CommandOpenSession()
{
    bool result = false;
    memset(Data,0,CPC_BufferSize);
    Position = 0;
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }
    Log->Write("CommandOpenSession");
    char MessageCode[3] = "01";

    SendPacket(Command,(char*)MessageCode);
    SendCommand();
    //if ((DeviceState)&&(DeviceState->AnswerSize != -1))
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;

    return result;
}

bool CPrim08TKClass::CommandOpenShift()
{
    bool result = false;
    memset(Data,0,CPC_BufferSize);
    Position = 0;
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }
    Log->Write("CommandOpenShift");
    char MessageCode[3] = "02";

    SendPacket(Command,(char*)MessageCode);
    SendCommand();
    //if ((DeviceState)&&(DeviceState->AnswerSize != -1))
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;

    return result;
}

bool CPrim08TKClass::CommandSetDocumentsParameters()
{
    bool result = false;
    memset(Data,0,CPC_BufferSize);
    Position = 0;
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }
    Log->Write("CommandSetDocumentsParameters");
    char MessageCode[3] = "4C";

    //SetTextToBuffer(Data,Position,"8700");
    //SetTextToBuffer(Data,Position,"C600");
    //new 30-07-2007
    SetTextToBuffer(Data,Position,"E601");
    SetTextToBuffer(Data,Position,"0000");
    SetTextToBuffer(Data,Position,"0000");

    SendPacket(Command,(char*)MessageCode,Data,Position,false);

    DWORD OldTimeOut = Port->timeout;
    Port->timeout = 30000;
    SendCommand();
    Port->timeout = OldTimeOut;

    //if ((DeviceState)&&(DeviceState->AnswerSize != -1))
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;
}

bool CPrim08TKClass::CommandSetPrintersParameters()
{
    bool result = false;
    memset(Data,0,CPC_BufferSize);
    Position = 0;
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }
    Log->Write("CommandSetPrintersParameters");
    char MessageCode[3] = "94";

    //����� ������
    SetTextToBuffer(Data,Position,"00");
    //������ �� 5-�� ��������� ����� 00-��� 01-��
    SetTextToBuffer(Data,Position,"01");
    //����� ������
    SetTextToBuffer(Data,Position,"00");
    //����� ������
    SetTextToBuffer(Data,Position,"00");
    //���� � ����� ���������� � ������� 00-��� 01-��
    SetTextToBuffer(Data,Position,"01");
    //����� ������
    SetTextToBuffer(Data,Position,"00");

    SendPacket(Command,(char*)MessageCode,Data,Position);
    SendCommand();
    //if ((DeviceState)&&(DeviceState->AnswerSize != -1))
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;
}

bool CPrim08TKClass::CommandGetXReportData()
{
    bool result = false;
    memset(Data,0,CPC_BufferSize);
    Position = 0;
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }
    Log->Write("CommandGetXReportData");
    char MessageCode[3] = "34";

    SendPacket(Command,(char*)MessageCode,Data,Position);

    bool old = DSE_OK_Sensitive;
    DSE_OK_Sensitive = false;
    SendCommand();
    DSE_OK_Sensitive = old;

    //if ((DeviceState)&&(DeviceState->AnswerSize != -1))
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;

    if (!result)
    {
        if (DeviceState)
          WriteErrorsToLog();
        //return result;
    }

    // ��������� ������� ����� � ��������
    Position = FirstDataByteAfterFramesInAnswer;
    // ���������� ��������
    for(int i=0; i<18; i++)
        GetMoneyFromBuffer(Answer,Position);

    // ��������� ��� �������
    RestOfMoney = GetMoneyFromBuffer(Answer,Position);
    Log->Write((boost::format("Money in printer = %1%") % RestOfMoney).str().c_str());

    return result;
}

bool CPrim08TKClass::PrintSaleTicket(BYTE ucDocumentType, char *pcWareName, char *pcMeasure,float fPrice, BYTE ucPaymentMethod)
{
  bool result = false;

  switch (ucDocumentType)
  {
    case SDOC:
      result = CommandOpenCheque(SALE, "");
            break;
    case RDOC:
      result = CommandOpenCheque(RETURN);
            break;
  }
  if (!result)
    return result;

  //�������� ������ �������
  result = CommandPrintSale(pcWareName,1, fPrice, pcMeasure, "", "");
  if (!result)
  {
    CommandCancelCheque();
    return result;
  }

  //�������� ����
  result = CommandPrintSummary();
  if (!result)
  {
    CommandCancelCheque();
    return result;
  }

  // ������
  switch (ucPaymentMethod)
  {
    case MONEY:
      result = CommandPrintTotalOfCheque(fPrice,CASH,"");
            break;
    case _CREDIT:
      result = CommandPrintTotalOfCheque(fPrice,CREDIT,"");
            break;
    case DEBET:
      result = CommandPrintTotalOfCheque(fPrice,CARD,"");
            break;
    case SB:
      result = CommandPrintTotalOfCheque(fPrice,PAYMENT4,"");
            break;
  }
  if (!result)
  {
    CommandCancelCheque();
    return result;
  }

  // ��������� ���
  result = CommandCloseCheque();
  if (!result)
  {
    CommandCancelCheque();
    return result;
  }

  return result;
}


void CPrim08TKClass::PrintCheck(double money, AnsiString text)
{
    try
    {
        if (!GetStateBool())
            return;

        if (DeviceState)
        {
            Log->Write("CommandOpenCheque");
            //��������� ���
            if (!CommandOpenCheque(SALE, text))
            {
                if ((LoggingErrors)&&(DeviceState))
                    WriteErrorsToLog();
                int result_code = DeviceState->ResultCode;
                //��� ������ ���������� ���
                CommandCancelCheque();
                //� ���� ��������� ������� �����, �������� � ������� � ����� ������� ���
                if (result_code == PE_Z_REPORT_NEEDED)
                {
                    Log->Write("Closing shift after CommandOpenCheque...");
                    //��������� ��������� � �������� ������� ����� � ������� �����
                    if (!GetStateBool())
                        return;
                    //����� �������� ������� ���
                    if (!CommandOpenCheque(SALE, text))
                    {
                        //���� �� ����������, �� �������
                        if ((LoggingErrors)&&(DeviceState))
                            WriteErrorsToLog();
                        //� ���������� ���
                        CommandCancelCheque();
                        return;
                    }
                }
                else
                    return;
            }
        }
        else
            return;

        //new 23-01-2007
        /*if (DeviceState)
        {
            char TextBuffer[10000];
            memset(TextBuffer,0,10000);
            TStringList* strings = new TStringList();
            AnsiString delim = "\r\n";
            AnsiString Text;
            while(true)
            {
              int pos = text.Pos(delim);
              if (pos == 0)
                break;
              text = text.Delete(pos,2);
              text = text.Insert("|",pos);
            }
            delim = "|";
            char _subtext[1024];
            LinesCount = 0;
            while(true)
            {
                AnsiString subtext;
                int pos = text.Pos(delim);
                if (pos == 0)
                {
                    if (!CommandPrintString(text))
                        if ((LoggingErrors)&&(DeviceState))
                            WriteErrorsToLog();
                    break;
                }
                subtext = text.SubString(0,pos-1);
                text = text.SubString(pos+1,text.Length()-pos);
                if (!CommandPrintString(subtext))
                    if ((LoggingErrors)&&(DeviceState))
                        WriteErrorsToLog();
            }
        }
        else
            return;*/

        if (DeviceState)
        {
            Log->Write("CommandPrintSale");
            if (!CommandPrintSale("������",1,money," "," "))
            {
                if ((LoggingErrors)&&(DeviceState))
                    WriteErrorsToLog();
                CommandCancelCheque();
                return;
            }
        }
        else
            return;


        if (DeviceState)
        {
            Log->Write("CommandPrintSummary");
            if (!CommandPrintSummary())
            {
                if ((LoggingErrors)&&(DeviceState))
                    WriteErrorsToLog();
                CommandCancelCheque();
                return;
            }
        }
        else
            return;

        if (DeviceState)
        {
            Log->Write("CommandPrintTotalOfCheque");
            if (!CommandPrintTotalOfCheque(money,CASH,""))
            {
                if ((LoggingErrors)&&(DeviceState))
                    WriteErrorsToLog();
                CommandCancelCheque();
                return;
            }
        }
        else
            return;

        if (DeviceState)
        {
            Log->Write("CommandCloseCheque");
            if (!CommandCloseCheque())
            {
                if ((LoggingErrors)&&(DeviceState))
                    WriteErrorsToLog();
                CommandCancelCheque();
                return;
            }
        }
        else
            return;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

bool CPrim08TKClass::CommandPrintString(AnsiString text)
{
    bool result = false;
    memset(Data,0,1000);
    Position = 0;
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }
    Log->Write("CommandPrintString");
    char MessageCode[3] = "1C";
    char TextBuffer[255];
    memset(TextBuffer,0,255);

    //Log->Write("Text to printer: "+ text);
//    TStringList* strings = new TStringList();
    AnsiString delim = "\r\n";
    while(true)
    {
      int pos = text.Pos(delim);
      if (pos == 0)
        break;
      text = text.Delete(pos,2);
      text = text.Insert("|",pos);
    }
    AnsiString _text = "";
    CharToOem(text.c_str(),TextBuffer);
    Move(TextBuffer,Data,text.Length());
    Position += text.Length();
    Data[Position] = SPR;
    Position++;

    SendPacket(Command,(char*)MessageCode,Data,Position,false);
    SendCommand();
    //if ((DeviceState)&&(DeviceState->AnswerSize != -1))
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;
    return result;
}

void CPrim08TKClass::PrintCheck(AnsiString text, std::string barcode)
{
    try
    {
        if (DeviceState)
        {
            Log->Write("CommandOpenNFCheque");
            if (!CommandOpenNFCheque())
            {
                if ((LoggingErrors)&&(DeviceState))
                    WriteErrorsToLog();
                CommandCloseNFCheque();
                return;
            }
        }
        else
            return;

        //new 23-01-2007
        if (DeviceState)
        {
            char TextBuffer[10000];
            memset(TextBuffer,0,10000);
//            TStringList* strings = new TStringList();
            AnsiString delim = "\r\n";
            AnsiString Text;
            while(true)
            {
              int pos = text.Pos(delim);
              if (pos == 0)
                break;
              text = text.Delete(pos,2);
              text = text.Insert("|",pos);
            }
            delim = "|";
//            char _subtext[1024];
            while(true)
            {
                AnsiString subtext;
                int pos = text.Pos(delim);
                if (pos == 0)
                {
                    if (!CommandPrintNFString(text))
                        if ((LoggingErrors)&&(DeviceState))
                            WriteErrorsToLog();
                    break;
                }
                subtext = text.SubString(0,pos-1);
                text = text.SubString(pos+1,text.Length()-pos);
                if (!CommandPrintNFString(subtext))
                    if ((LoggingErrors)&&(DeviceState))
                        WriteErrorsToLog();
            }
        }
        else
            return;
        /*if (DeviceState)
        {
            Log->Write("CommandPrintNFString");
            if (!CommandPrintNFString(text))
            {
                if ((LoggingErrors)&&(DeviceState))
                    WriteErrorsToLog();
                CommandCloseNFCheque();
                return;
            }
        }
        else
            return;*/

        if (DeviceState)
        {
            Log->Write("CommandCloseNFCheque");
            if (!CommandCloseNFCheque())
            {
                if ((LoggingErrors)&&(DeviceState))
                    WriteErrorsToLog();
                return;
            }
        }
        else
            return;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

bool CPrim08TKClass::CommandOpenNFCheque()
{
    bool result = false;
    memset(Data,0,CPC_BufferSize);
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }

    char MessageCode[3] = "50";
    SendPacket(Command,(char*)MessageCode);
    SendCommand();
    //if ((DeviceState)&&(DeviceState->AnswerSize != -1))
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;

    return result;
}

bool CPrim08TKClass::CommandPrintNFString(AnsiString text)
{
    bool result = false;
    memset(Data,0,CPC_BufferSize);
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }

    char MessageCode[3] = "56";
    Position = 0;
    char TextBuffer[10000];
    memset(TextBuffer,0,10000);

    //Log->Write("Text to printer: "+ text);
//    TStringList* strings = new TStringList();
    AnsiString delim = "\r\n";
    while(true)
    {
      int pos = text.Pos(delim);
      if (pos == 0)
        break;
      text = text.Delete(pos,2);
      text = text.Insert("|",pos);
    }
    delim = "|";
 //   char _subtext[1024];
    LinesCount = 0;
    while(true)
    {
      int pos = text.Pos(delim);
      if (pos == 0)
      {
          if (!text.IsEmpty())
          {
            CharToOem(text.c_str(),TextBuffer);
            Move(TextBuffer,&Data[Position],text.Length());
            Position += text.Length();
            Data[Position] = SPR;
            Position++;
            LinesCount++;
          }
          break;
      }
      AnsiString subtext = text.SubString(0,pos-1);
      text = text.SubString(pos+1,text.Length()-pos);
      if (!subtext.IsEmpty())
      {
        CharToOem(subtext.c_str(),TextBuffer);
        Move(TextBuffer,&Data[Position],subtext.Length());
        Position += subtext.Length();
        Data[Position] = SPR;
        Position++;
        LinesCount++;
      }
    }

    SendPacket(Command,(char*)MessageCode,Data,Position,false);
    SendCommand();
    //if ((DeviceState)&&(DeviceState->AnswerSize != -1))
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;

    return result;
}

bool CPrim08TKClass::CommandCloseNFCheque()
{
    bool result = false;
    memset(Data,0,CPC_BufferSize);
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }

    DWORD OldTimeOut = Port->timeout;
    Port->timeout = 3000;
    char MessageCode[3] = "52";
    SendPacket(Command,(char*)MessageCode,NULL,0,false);
    SendCommand();
    Port->timeout = OldTimeOut;
    //if ((DeviceState)&&(DeviceState->AnswerSize != -1))
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;

    return result;
}

void CPrim08TKClass::PrintXReport(AnsiString Text)
{
    if (Text == "")
        CommandXReport();
    else
        PrintCheck(Text);
}

void CPrim08TKClass::CashIncassation(double Money)
{
    UNREFERENCED_PARAMETER(Money);
    if (Port == NULL)
        return;
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        return;
    }

    RestOfMoney = 0;
    if (DeviceState == NULL)
    {
        Log->Write("Breaking operation CashIncassation");
        return;
    }
    bool result = CommandGetXReportData();

    if (DeviceState == NULL)
    {
        Log->Write("Breaking operation CashIncassation");
        return;
    }

    if (RestOfMoney > 0)
    {
        Log->Write((boost::format("Taking %1% cash money from printer.") % RestOfMoney).str().c_str());

        result = CommandTakeMoneyFromCash(RestOfMoney);

        if (!result)
        {
            if (DeviceState)
                WriteErrorsToLog();
        }
    }
    else
    {
        Log->Write("No cash money for take off the printer.");
    }
}

void CPrim08TKClass::PrintZReport(AnsiString Text)
{
    if (Port == NULL)
        return;
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        return;
    }

    if (ZReportInBuffer)
    {
        CommandZReportIntoBuffer();
        PrintZReportsFromBuffer();
        return;
    }

    RestOfMoney = 0;
    if (DeviceState == NULL)
    {
        Log->Write("Breaking operation PrintZReport");
        return;
    }
    bool result = CommandGetXReportData();

    /*if (!result)
    {
        //if ((LoggingErrors)&&(DeviceState))
        if (DeviceState)
            WriteErrorsToLog();
        return;
    }*/

    if (DeviceState == NULL)
    {
        Log->Write("Breaking operation PrintZReport");
        return;
    }

    if (RestOfMoney > 0)
    {
        result = CommandTakeMoneyFromCash(RestOfMoney);

        if (!result)
        {
            //if ((LoggingErrors)&&(DeviceState))
            if (DeviceState)
                WriteErrorsToLog();
            //return;
        }
    }

    if (DeviceState == NULL)
    {
        Log->Write("Breaking operation PrintZReport");
        return;
    }
    if (Text == "")
    {
        if (DeviceState == NULL)
        {
            Log->Write("Breaking operation PrintZReport");
            return;
        }
        result = CommandZReport();
        if (!result)
        {
            if (DeviceState)
                WriteErrorsToLog();
        }
    }
    else
    {
        PrintCheck(Text);
        if (DeviceState == NULL)
        {
            Log->Write("Breaking operation PrintZReport");
            return;
        }
        result = CommandZReport();
        if (!result)
        {
            if (DeviceState)
                WriteErrorsToLog();
        }
    }
}

void CPrim08TKClass::GetState()
{
    bool result = GetStateBool();
    if (!result)
       PrinterEnable = false;
}

bool CPrim08TKClass::IsPrinterEnable()
{
    GetState();
    if ((DeviceState)&&(DeviceState->OutStateCode == DSE_HARDWARE_ERROR))
       PrinterEnable = false;
    return PrinterEnable;
}

int CPrim08TKClass::FindByteInBuffer(BYTE *Buffer, int size, BYTE Value, int Position)
{
  int result = -1;

  for (int i = Position; i<size; i++)
  {
    if (Buffer[i] == Value)
    {
      result = i;
      return result;
    }
  }

  return result;
}

int CPrim08TKClass::SendEvent()
{
    char str[100];
    BYTE Value;
    std::string ErrorString;

    if ((ChangeEvent == NULL)||(DeviceState == NULL))
        return 0;

    if((DeviceState)&&(DeviceState->ResultCode == PE_LINK_ERROR))
    {
        StateExplanation(DeviceState->ResultCode);
        DeviceState->StateDescription = GetErrorExplanationString((char*)str, DeviceState->ResultCode);
        DeviceState->OutStateCode == DSE_NOTMOUNT;
        ChangeDeviceState();
        return 0;
    }

    if ((DeviceState)&&(DeviceState->CriticalErrorsCount > 0))
        WriteErrorsToLog();

    if((DeviceState)&&(DeviceState->ResultCode != 0x00)&&(DeviceState->ResultCode != 0xFF))
    {
        StateExplanation(DeviceState->ResultCode);
        DeviceState->StateDescription = GetErrorExplanationString((char*)str, DeviceState->ResultCode);
        if ((DeviceState->OutStateCode == DSE_HARDWARE_ERROR)||(DeviceState->OutStateCode == DSE_NOTPAPER))
        {
            if (ChangeDeviceState())
                return 0;
            if (DeviceState == NULL)
                return 0;
        }
    }

    //������� ������
    for(int i=0; i<DeviceState->CriticalErrorsCount; i++)
    if (DeviceState->Errors[i] != END_OF_SEQUENCE)
    {
        Value = DeviceState->Errors[i];
        ErrorString = GetErrorExplanationString((char*)str, Value);
        DeviceState->StateDescription = ErrorString;
        StateExplanation(Value);
        if (ChangeDeviceState())
            return 0;
        if (DeviceState == NULL)
            return 0;
    }
    else
       break;

    if (DeviceState->ErrorsCount > 0)
        return 0;

    //������� �������
    for(int i=0; i<DeviceState->StatusesCount; i++)
    if (DeviceState->Statuses[i] != END_OF_SEQUENCE)
    {
        Value = DeviceState->Statuses[i];
        if (Value != PE_PAPER_ABOUT_OVER)
            continue;
        ErrorString =GetErrorExplanationString((char*)str, Value);
        DeviceState->StateDescription = ErrorString;
        StateExplanation(Value);
        if (ChangeDeviceState())
            return 0;
        if (DeviceState == NULL)
            return 0;
    }
    else
       break;

    if (DeviceState->ErrorsCount > 0)
        return 0;

    if (DeviceState == NULL)
        return 0;

    if (DeviceState->ResultCode == 0xFF)
    {//������ �� ���������� ���
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        if ((DeviceState)&&(DeviceState->OldOutStateCode != DeviceState->OutStateCode))
        {
            Log->Write("OutStateCode = DSE_NOTMOUNT");
            PrinterEnable = false;
            ChangeDeviceState();
        }
    }
    //���� �� ������, ��� DSE_OK
    else
    {
        if ((DSE_OK_Sensitive)||(DeviceState->OutStateCode == DSE_NOTMOUNT))
        {
            DeviceState->OutStateCode = DSE_OK;
            if ((DeviceState)&&(DeviceState->OldOutStateCode != DeviceState->OutStateCode))
            {
                Log->Write("OutStateCode = DSE_OK");
                PrinterEnable = true;
                ChangeDeviceState();
            }
        }
    }

    return 1;
}

int CPrim08TKClass::StateExplanation(BYTE Value)
{
    switch(Value)
    {
        case PE_PAPER_OVER:
            DeviceState->SetOutCodes(PRN_NO_PAPER);
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
        case PE_LINK_ERROR:
        case PE_RECIEVE_ERROR:
            DeviceState->SetOutCodes(DSE_OFFLINE);
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

    switch (Value)
    {
          case PE_PAPER_ABOUT_OVER:
              DeviceState->OutStateCode = DSE_NEARENDPAPER;
              PrinterEnable = true;
              break;
          case PE_PAPER_OVER:
              DeviceState->OutStateCode = DSE_NOTPAPER;
              PrinterEnable = false;
              break;
          case PE_NONCORRECT_PRINTER_STATE:
          case PE_FISCAL_PRINTER_FAILURE:
          case PE_PRINTER_NOT_READY:
          case PE_HARDWARE_ERROR:
          case PE_WORKING_MEMORY_ERROR:
          case PE_TIME_OVERFLOW:
          case PE_PRINTER_STATE_ERROR:
          case PE_WORKING_MEMORY_CHECKSUM_ERROR:
              DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
              PrinterEnable = false;
              break;

          case PE_LINK_ERROR:
              DeviceState->OutStateCode = DSE_NOTMOUNT;
              PrinterEnable = false;
              break;

          case PE_ERROR_OF_STATUS_RECEIVE:
          case PE_OPERATION_TIME_ERROR:
          case PE_DATE_ERROR:
              //DeviceState->OutStateCode = DSE_MAINERROR;
              PrinterEnable = false;
              break;
          case PE_NEED_TO_CLOSE_SHIFT:
          case PE_Z_REPORT_NEEDED:
               DeviceState->ShiftState = 0;
               SessionOpened = false;
               break;
          case 0xFF:
              DeviceState->OutStateCode = DSE_NOTMOUNT;
              PrinterEnable = false;
              break;
    }


    if (Fiscal)
    {
      switch (Value)
      {
          case PE_FISCAL_PRINTER_FAILURE:
          case PE_PRINTER_NOT_READY:
          case PE_HARDWARE_ERROR:
          case PE_WORKING_MEMORY_ERROR:
          case PE_FISCAL_MEMORY_ERROR:
          case PE_REGISTRATION_COUNT_OVERFLOW:
          case PE_TIME_OVERFLOW:
          case PE_FISCAL_COUNT_OVERFLOW:
          case PE_PRINTER_STATE_ERROR:
          case PE_FISCAL_MEMORY_FAILURE:
          case PE_FISCAL_MEMORY_NOT_INITIATED:
          case PE_FISCAL_MEMORY_OVERFLOW:
          case PE_UNKNOWN_STATE_EKLZ:
          case PE_NONCORRECT_STATE_EKLZ:
          case PE_NO_FREE_SPACE_EKLZ:
          case PE_EKLZ_NOT_FOUND:
          case PE_FISCAL_MEMORY_CHECKSUM_ERROR:
          case PE_WORKING_MEMORY_CHECKSUM_ERROR:
              DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
              PrinterEnable = false;
              break;
          //new 29-10-2007
          case PE_OK:
              if (DeviceState->OutStateCode == DSE_HARDWARE_ERROR)
              {
                  DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
                  PrinterEnable = false;
              }
              break;
      }
    }
    else
    {
      switch (Value)
      {
          case PE_UNKNOWN_ANSWER_FROM_EKLZ :
          case PE_UNKNOWN_COMMAND_EKLZ :
          case PE_TIMEOUT_RECIEVING_EKLZ :
          case PE_TIMEOUT_TRANSMITING_EKLZ :
          case PE_NONCORRECT_CRC_ANSWER_FROM_EKLZ :
          case PE_NONCORRECT_CRC_COMMAND_EKLZ :
          case PE_DATA_EKLZ_NOT_EXIST :
          case PE_DATA_EKLZ_UNSYNCHRONIZED :
          //case PE_FISCAL_PRINTER_FAILURE:
          case PE_PRINTER_NOT_READY:
          case PE_HARDWARE_ERROR:
          case PE_WORKING_MEMORY_ERROR:
          case PE_FISCAL_MEMORY_ERROR:
          case PE_REGISTRATION_COUNT_OVERFLOW:
          case PE_TIME_OVERFLOW:
          case PE_FISCAL_COUNT_OVERFLOW:
          case PE_PRINTER_STATE_ERROR:
          case PE_FISCAL_MEMORY_FAILURE:
          case PE_FISCAL_MEMORY_NOT_INITIATED:
          case PE_FISCAL_MEMORY_OVERFLOW:
          case PE_UNKNOWN_STATE_EKLZ:
          case PE_NONCORRECT_STATE_EKLZ:
          case PE_NO_FREE_SPACE_EKLZ:
          case PE_EKLZ_NOT_FOUND:
          case PE_FISCAL_MEMORY_CHECKSUM_ERROR:
          case PE_WORKING_MEMORY_CHECKSUM_ERROR:
              DeviceState->OutStateCode = DSE_OK;
              DeviceState->ResultCode = PE_OK;
              PrinterEnable = true;
              break;
      }
    }
    return 0;
}

int CPrim08TKClass::Initialize()
{
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        if (DeviceState)
        {
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            ChangeDeviceState();
        }
        return false;
    }
    if (DeviceState)
    {
       if(CommandOpenSession())
           SetInitialized();
       //CommandSetDocumentsParameters();
    }
    if (DeviceState)
       GetState();
}

bool CPrim08TKClass::CommandZReportIntoBuffer()
{
    bool result = false;

    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }

    memset(Data,0,CPC_BufferSize);
    Log->Write("CommandZReportIntoBuffer()");
    DWORD OldThreadLifeTime = ThreadLifeTime;
    ThreadLifeTime = 50*1000;

    DWORD OldTimeOut = Port->timeout;
    Port->timeout = 60000;

    AnsiString CommandCode = "31";

    Position = 0;
    strcpy((char*)&Data[Position],"00");
    Position+=2;
    Data[Position]=SPR;Position++;

    SendPacket(Command,CommandCode.c_str(),Data,Position);
    SendCommand();
    //if ((DeviceState)&&(DeviceState->AnswerSize != -1))
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
    {
        if (DeviceState)
            DeviceState->ShiftState = 1;
        SessionOpened = true;
        result = true;
    }

    ThreadLifeTime = OldThreadLifeTime;
    if (Port)
        Port->timeout = OldTimeOut;
    return result;
}

bool CPrim08TKClass::CommandGetResources()
{
    bool result = false;
    memset(Data,0,CPC_BufferSize);
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }

    Log->Write("CommandGetResources()");
    AnsiString CommandCode = "03";
    SendPacket(Command,CommandCode.c_str());

    DWORD OldThreadLifeTime = ThreadLifeTime;
    ThreadLifeTime = 50*1000;
    DWORD OldTimeOut = Port->timeout;
    Port->timeout = 30000;

    SendCommand();

    Port->timeout = OldTimeOut;
    ThreadLifeTime = OldThreadLifeTime;

    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;
    else
    {
        if (DeviceState)
            WriteErrorsToLog();
    }

    try
    {
        EndZReportNumber = GetZReportFromBuffer(Answer,37);
        Log->Write((boost::format("CommandGetResources(): EndZReportNumber = %1%") % EndZReportNumber).str().c_str());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return result;
}

bool CPrim08TKClass::CommandZReport(int SessionNumber)
{
    bool result = false;

    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }

    memset(Data,0,CPC_BufferSize);
    Log->Write((boost::format("CommandZReport() with SessionNumber = %1%") % SessionNumber).str().c_str());
    DWORD OldThreadLifeTime = ThreadLifeTime;
    ThreadLifeTime = 50*1000;

    DWORD OldTimeOut = Port->timeout;
    Port->timeout = 60000;

    AnsiString CommandCode = "7C";

    try
    {
        Position = 0;

        AnsiString number;
        number.sprintf("%04X",SessionNumber);
        BYTE SessionNumberStr[4];
        Log->Write((boost::format("CommandZReport hex = %1%; int = %2%") % number.c_str() % SessionNumber).str().c_str());
        SessionNumberStr[0] = (BYTE)number.c_str()[2];
        SessionNumberStr[1] = (BYTE)number.c_str()[3];
        SessionNumberStr[2] = (BYTE)number.c_str()[0];
        SessionNumberStr[3] = (BYTE)number.c_str()[1];
        Move(SessionNumberStr,&Data[Position],4);
        Position += 4;
        Data[Position] = SPR;
        Position++;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Log->Write("CommandZReport(int SessionNumber) Exception");
    }

    SendPacket(Command,CommandCode.c_str(),Data,Position,false);
    SendCommand();
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
    {
        if (DeviceState)
            DeviceState->ShiftState = 1;
        SessionOpened = true;
        result = true;
    }

    ThreadLifeTime = OldThreadLifeTime;
    if (Port)
        Port->timeout = OldTimeOut;
    return result;
}

bool CPrim08TKClass::CommandClearSKL()
{
    bool result = false;
    memset(Data,0,CPC_BufferSize);
    Position = 0;
    if (Port == NULL)
        return result;
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }
    Log->Write("CommandClearSKL()");
    char MessageCode[3] = "85";

    SendPacket(Command,(char*)MessageCode);
    SendCommand();
    //sending command twice for complete operation
    SendPacket(Command,(char*)MessageCode);
    SendCommand();
    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;

    return result;
}

AnsiString CPrim08TKClass::GetTextFromBuffer(BYTE *Buffer, int& index)
{
    AnsiString result = "";
    char ResultString[40];
    int pos = 0;

    memset(ResultString,0,40);
    while ((Buffer[index] != SPR)&&(pos < 40))
    {
        ResultString[pos] = Buffer[index];
        pos++;
        index++;
    }

    result = AnsiString(ResultString);
    index++;
    return result;
}

int CPrim08TKClass::GetZReportFromBuffer(BYTE *Buffer, int &index)
{
    int result = 0;
    char ResultString[5];
//    int pos = 0;
    int _index = index;

    memset(ResultString,0,4);

    ResultString[0] = Buffer[index+2];
    ResultString[1] = Buffer[index+3];
    ResultString[2] = Buffer[index+0];
    ResultString[3] = Buffer[index+1];
    ResultString[4] = 0;

    AnsiString str =  "0x"+AnsiString(ResultString);
    result = StrToInt(str);
    if (LoggingErrors)
    {
        Log->Write((boost::format("GetZReportFromBuffer: ZReportNumber = %1%; int = %2%") % str.c_str() % result).str().c_str());
        AnsiString str = GetTextFromBuffer(Buffer,index);
        Log->Write((boost::format("Text in buffer from current position = %1%") % str.c_str()).str().c_str());
        OemToString(str.c_str());
        Log->Write((boost::format("OEM string from buffer = %1%") % WinLetters).str().c_str());
    }

    //result = atoi(ResultString); StrToInt
    index = _index+5;
    return result;
}

int CPrim08TKClass::GetFirstZReportFromBuffer(BYTE *Buffer, int &index)
{
    int result = 0;
    try
    {
        char ResultString[5];
//        int pos = 0;
        int _index = index;

        memset(ResultString,0,4);

        ResultString[0] = Buffer[index+2];
        ResultString[1] = Buffer[index+3];
        ResultString[2] = Buffer[index+0];
        ResultString[3] = Buffer[index+1];
        ResultString[4] = 0;

        AnsiString str =  AnsiString(ResultString);
        result = StrToInt(str);
        if (LoggingErrors)
        {
            Log->Write((boost::format("GetZReportFromBuffer: FirstZReportNumber = %1%; int = %2%") % str.c_str() % result).str().c_str());
            AnsiString str = GetTextFromBuffer(Buffer,index);
            Log->Write((boost::format("Text in buffer from current position = %1%") % str.c_str()).str().c_str());
            OemToString(str.c_str());
            Log->Write((boost::format("OEM string from buffer = %1%") % WinLetters).str().c_str());
        }

        //result = atoi(ResultString); StrToInt
        index = _index+5;
        return result;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return result;
    }
}

bool CPrim08TKClass::PrintZReportsFromBuffer(int BeginSessionNumber, int EndSessionNumber)
{
    if (Port == NULL)
        return false;
    if (!Port->PortInit)
    {
        PrinterEnable = false;
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        return false;
    }

    CommandGetFirstZReportNumber();
    BeginSessionNumber = BeginZReportNumber;

    if (BeginSessionNumber == 0)
        BeginSessionNumber = 1;

    Log->Write((boost::format("PrintZReportsFromBuffer(): from CommandGetFirstZReportNumber() -> BeginSessionNumber = %1%") % BeginZReportNumber).str().c_str());
    if (EndSessionNumber == 0)
    {
        CommandGetResources();
        EndSessionNumber = EndZReportNumber;
        if (BeginSessionNumber > EndSessionNumber)
            BeginSessionNumber = EndSessionNumber;
    }
    else
    if (BeginSessionNumber > EndSessionNumber)
        EndSessionNumber = BeginSessionNumber;
    Log->Write((boost::format("PrintZReportsFromBuffer(): BeginSessionNumber = %1%; EndSessionNumber = %2%") % BeginSessionNumber % EndSessionNumber).str().c_str());

    if (EndSessionNumber == 0) return false;

    int ReportsCountRest, ReportsCount;
    ReportsCount = ReportsCountRest = EndSessionNumber-BeginSessionNumber + 1;
    bool result = true;
    int _BeginSessionNumber = BeginSessionNumber;
    for(int i=BeginSessionNumber; i<=EndSessionNumber; i++)
    {
        for(int j=0;j<200;j++)
        {
            Application->ProcessMessages();
            Sleep(10);
        }
        if ( CommandZReport(i) == false )
        {
            result = false;
            Log->Write((boost::format("Can't print Z-report %1%") % i).str().c_str());
        }
        else
        {
            _BeginSessionNumber = i+1;
            Log->Write((boost::format("BeginSessionNumber = %1%") % _BeginSessionNumber).str().c_str());
            ReportsCountRest--;
        }
    }

    if (ReportsCountRest == 0)
        Log->Write("All Z-reports printed successfuly.");
    else
        Log->Write((boost::format("Can't print Z-reports, total %1%; printed %2%") % ReportsCount % (ReportsCount - ReportsCountRest)).str().c_str());

    /*if (CommandClearSKL())
        Log->Write("��� ������� �������.");
    else
        Log->Write("�� ������� �������� ���.");*/

    CommandZReportIntoBuffer();

    if (CommandSaveFirstZReportNumber(_BeginSessionNumber))
        Log->Write((boost::format("First Z-report number = %1%") % _BeginSessionNumber).str().c_str());
    else
        Log->Write((boost::format("Error! Can't write first Z-report number = %1%") % _BeginSessionNumber).str().c_str());

    return result;
}

void CPrim08TKClass::SetZReportNumberIntoBuffer(BYTE* Buffer, int& ind, int ZReportNumber)
{
    AnsiString number;
    number.sprintf("%04d",ZReportNumber);
    BYTE SessionNumber[5];
    SessionNumber[0] = (BYTE)number.c_str()[2];
    SessionNumber[1] = (BYTE)number.c_str()[3];
    SessionNumber[2] = (BYTE)number.c_str()[0];
    SessionNumber[3] = (BYTE)number.c_str()[1];
    SessionNumber[4] = 0;
    Move(SessionNumber,&Buffer[ind],4);
    ind += 4;

    number = "";
    for(int i=1; i<=14; i++)
        number += " ";
    number += ":";
    Move(number.c_str(),&Buffer[ind],15);
    ind += 15;

    Data[ind] = SPR;
    ind++;
    number = AnsiString((char*)SessionNumber) + number;
    if (LoggingErrors)
        Log->Write((boost::format("Saving first ZReportNumber = %1%; command data = %2%") % ZReportNumber % number.c_str()).str().c_str());
}

/*bool CPrim08TKClass::CommandSaveFirstZReportNumber(int SessionNumber)
{
    bool result = false;

    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }

    memset(Data,0,CPC_BufferSize);
    Log->Write("CommandSaveFirstZReportNumber()");

    DWORD OldThreadLifeTime = ThreadLifeTime;
    ThreadLifeTime = 50*1000;
    DWORD OldTimeOut = Port->timeout;
    Port->timeout = 60000;

    AnsiString CommandCode = "47";
    AnsiString Text;

    Position = 0;
    Text = "���Ҩ�";
    SetTextToBuffer(Data, Position, Text.c_str());
    Text = "�������";
    SetTextToBuffer(Data, Position, Text.c_str());

    SetZReportNumberIntoBuffer(Data, Position, SessionNumber);

    //strcpy((char*)&Data[Position],"00");

    SendPacket(Command,CommandCode.c_str(),Data,Position,false);
    SendCommand();

    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;

    ThreadLifeTime = OldThreadLifeTime;
    if (Port)
        Port->timeout = OldTimeOut;
    return result;
}*/

bool CPrim08TKClass::CommandSaveFirstZReportNumber(int SessionNumber)
{
    bool result = false;

    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }

    memset(Data,0,CPC_BufferSize);
    Log->Write("CommandSaveFirstZReportNumber()");

    DWORD OldThreadLifeTime = ThreadLifeTime;
    ThreadLifeTime = 50*1000;
    DWORD OldTimeOut = Port->timeout;
    Port->timeout = 60000;

    AnsiString CommandCode = "4A";
    AnsiString Text;

    Position = 0;
    //index of payment
    Text = "05";
    SetTextToBuffer(Data, Position, Text.c_str());
    //z report number
    SetZReportNumberIntoBuffer(Data, Position, SessionNumber);

    Text = "00";
    SetTextToBuffer(Data, Position, Text.c_str());
    Text = "00";
    SetTextToBuffer(Data, Position, Text.c_str());
    Text = "00";
    SetTextToBuffer(Data, Position, Text.c_str());
    Text = "00";
    SetTextToBuffer(Data, Position, Text.c_str());

    /*Data[Position] = 0x00; Position++;
    Data[Position] = 0x00; Position++;
    Data[Position] = 0x1C; Position++;*/

    Text = "1.00";
    SetTextToBuffer(Data, Position, Text.c_str());
    //strcpy((char*)&Data[Position],"00");

    SendPacket(Command,CommandCode.c_str(),Data,Position,false);
    SendCommand();

    if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        result = true;

    ThreadLifeTime = OldThreadLifeTime;
    if (Port)
        Port->timeout = OldTimeOut;
    return result;
}

int CPrim08TKClass::CommandGetFirstZReportNumber()
{
    bool result = 1;

    if (!Port->PortInit)
    {
        PrinterEnable = false;
        return result;
    }

    memset(Data,0,CPC_BufferSize);
    Log->Write("CommandGetFirstZReportNumber()");

    AnsiString CommandCode = "4B";
    AnsiString Text;

    Position = 0;
    Text = "05";//index of name of payment
    SetTextToBuffer(Data, Position, Text.c_str());

    SendPacket(Command,CommandCode.c_str(),Data,Position,false);
    SendCommand();


    try
    {
        BeginZReportNumber = GetFirstZReportFromBuffer(Answer,32);
        Log->Write((boost::format("CommandGetFirstZReportNumber(): BeginZReportNumber = %1%") % BeginZReportNumber).str().c_str());
        //if ((DeviceState)&&(DeviceState->ResultCode == PE_OK))
        return result = BeginZReportNumber;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }

    return result;
}

/*
Subject: ������������ ������������� ������� �� ���� ��� " ���� "


1.        � ���������� ���������� ��� ��� 13 ������� ����� ���������� ( ����������� ��� ) ���������� � 0 �
2.        ������� 31h ( �������� ����� ) ������ ��������� �������������� ���� ���� BYTE ( �������� �00� ). ��� ���������� ���� ����� ����� ������� � ����������� Z-������.
3.        ����� �����������, ��� �� ��������, ���������� ���� ����������� � ���
4.        ��� ���������� ���� �������� ����� � ��� ������ ������� 7Ch ( ������ Z-������ �� ���� ) c ��������� ������ �����.

������ ���, � ������� ���������� ���� �������� :
�        ����-21� ( ����� ������ 1)
�        ����-22��
�        ��� ���� � ������������ LPC
*/
