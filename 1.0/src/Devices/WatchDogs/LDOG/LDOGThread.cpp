#include <math.h>
//---------------------------------------------------------------------------


#pragma hdrstop

#include "LDOGThread.h"
#include "globals.h"
#include "boost/format.hpp"

//---------------------------------------------------------------------------

#pragma package(smart_init)

__fastcall TLDOGDeviceThread::TLDOGDeviceThread() : TDeviceThread(true)
{
    FreeOnTerminate = false;
    SendType = NotRecieveAnswer;
    DataLength = 1;
    ExtCommand = EXTC_Free;
    _PollingInterval = 30*1000;
    SendType = RecieveAnswer;
    ID = 0;
    StartPCTimeOut = 0;
    LifeTimeOut = 0;
    ResetTimeOut = 0;
    ModemTimeOut = 0;
    Door_1 = Door_2 = 0;
    version = "";
}

__fastcall TLDOGDeviceThread::~TLDOGDeviceThread()
{
  Log->Write("~TLDOGDeviceThread()");
}

void TLDOGDeviceThread::SendPacket(BYTE command, BYTE* _data, int datalen)
{
  if (!Port->PortInit)
    return;
  try
  {
    //здесь в буфер пишем команду для выполнения
    ClearCommand();
    ClearAnswer();

    Command[0] = 0x10;
    Command[1] = command;
    memcpy(&Command[2], _data, datalen);
    int ind = 2+datalen;

    BYTE crc = Command[0];
    for(int i = 1; i<ind; i++)
      crc += Command[i];
    crc = 256 - crc;

    Command[ind] = crc; ind++;
    Command[ind] = 0x0D;

    CommandSize = ind+1;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

int TLDOGDeviceThread::GetInt16(BYTE* _data, int ind)
{
    WORD low = 0, hi = 0;
    low = _data[ind];
    hi  = _data[ind+1];
    hi = hi << 8;
    int result = hi + low;
    return result;
}

long TLDOGDeviceThread::GetInt32(BYTE* _data, int ind)
{
    long result = 0;

    for(int i = 0; i<4; i++)
        result = result + (_data[ind + i] << (8*i));

    return result;
}

void TLDOGDeviceThread::ParseAnswer()
{
  try
  {
    if (DeviceState)
      AnswerSize = DeviceState->AnswerSize;
    ClearBuffer(data);
    if (AnswerSize <= 4)
    {
         Log->Write((boost::format("Invalid answer size = %1%") % AnswerSize).str().c_str());
         AnswerDataSize = 0;
         if (Log)
         {
            Log->Write("Answer is:");
            Log->WriteBuffer(Answer, AnswerSize);
         }
    }
    else
    if (AnswerSize > 4)
    {
         int count = AnswerDataSize = AnswerSize - 4;
         memcpy((BYTE*)data,&Answer[2],count);
         if ((Log)&&(LoggingErrors))
         {
            Log->Write("Answer is:");
            Log->WriteBuffer(data, AnswerDataSize);
         }
    }
  }
  __finally
  {
  }
}

void TLDOGDeviceThread::GetVersion()
{
    if (!Port->PortInit)
      return;

    SendPacket(0x11);
    ProcessCommand();
    AnswerDataSize = COMToBuffer(data, AnswerDataSize);

    if (AnswerDataSize == 4)
    {
        ID = GetInt32(data,0);
        version = AnsiString((char*)data);
        Log->Write((boost::format("device version = %1%") % version.c_str()).str().c_str());
    }
    else
    {
         if ((Log)&&(LoggingErrors))
         {
            Log->Write("Incorrect data length of answer of device while ID request.");
         }
    }
}

int TLDOGDeviceThread::BufferToCOM(BYTE* _data, int& length)
{
    BYTE NewBuffer[100];
    memset(NewBuffer, 0, 100);
    if (length > 100) length = 100;

    int NewPosition = 0;
    for(int i = 0; i<length; i++)
    {
        switch(_data[i])
        {
            case 0x40:
                NewBuffer[NewPosition] = 0x40; NewPosition++;
                NewBuffer[NewPosition] = 0x00;
                break;

            case 0x0D:
                NewBuffer[NewPosition] = 0x40; NewPosition++;
                NewBuffer[NewPosition] = 0xCD;
                break;

            default:
                NewBuffer[NewPosition] = _data[i];
        }
        NewPosition++;
    }

    length = NewPosition;
    memcpy(_data,NewBuffer,length);
    return length;
}

int TLDOGDeviceThread::COMToBuffer(BYTE* _data, int& length)
{
    BYTE NewBuffer[100];
    memset(NewBuffer, 0, 100);
    if (length > 100) length = 100;

    int NewPosition = 0;
    int i = 0;
    while(i<length)
    {
        if (_data[i] == 0x40)
        {
            switch(_data[i+1])
            {
                case 0x00:
                    NewBuffer[NewPosition] = 0x40;
                    break;

                case 0xCD:
                    NewBuffer[NewPosition] = 0x0D;
                    break;
            }
            i++;
        }
        else
            NewBuffer[NewPosition] = _data[i];

        NewPosition++;
        i++;
    }

    memset(_data,0,length);
    length = NewPosition;
    memcpy(_data,NewBuffer,length);
    return length;
}

void TLDOGDeviceThread::GetTimeOuts()
{
    if (!Port->PortInit)
      return;

    StartPCTimeOut = 0;
    LifeTimeOut = 0;
    ResetTimeOut = 0;
    ModemTimeOut = 0;

    SendPacket(0x04);
    ProcessCommand();
    AnswerDataSize = COMToBuffer(data, AnswerDataSize);

    if (AnswerDataSize == 8)
    {
        Log->Write("GetTimeOuts() has successful done.");
        int ind = 0;
        StartPCTimeOut = GetInt16(data,ind); ind += 2;
        Log->Write((boost::format("StartPCTimeOut = %1%") % StartPCTimeOut).str().c_str());
        LifeTimeOut = GetInt16(data,ind); ind += 2;
        Log->Write((boost::format("LifeTimeOut = %1%") % LifeTimeOut).str().c_str());
        ResetTimeOut = GetInt16(data,ind); ind += 2;
        Log->Write((boost::format("ResetTimeOut = %1%") % ResetTimeOut).str().c_str());
        ModemTimeOut = GetInt16(data,ind); ind += 2;
        Log->Write((boost::format("ModemTimeOut = %1%") % ModemTimeOut).str().c_str());
    }
    else
    {
         Log->Write("Incorrect data length of answer of device while TimeOuts request.");
    }
}

void TLDOGDeviceThread::SetInt16ToBuffer(int Value, BYTE* Buffer, int ind)
{
    BYTE low = (BYTE)((WORD)Value&0x00ff);
    BYTE hi = (BYTE)(((WORD)Value&0xff00) >> 8);
    Buffer[ind] = low;
    Buffer[ind+1] = hi;
}

void TLDOGDeviceThread::SetTimeOuts()
{
    if (!Port->PortInit)
      return;

    StartPCTimeOut = 5*60;
    LifeTimeOut = 2*60;
    ResetTimeOut = 2000;
    ModemTimeOut = 2000;

    int ind = 0;
    SetInt16ToBuffer(StartPCTimeOut,data,ind); ind += 2;
    SetInt16ToBuffer(LifeTimeOut,data,ind); ind += 2;
    SetInt16ToBuffer(ResetTimeOut,data,ind); ind += 2;
    SetInt16ToBuffer(ModemTimeOut,data,ind); ind += 2;
    int len = 8;
    len = BufferToCOM(data,len);

    SendPacket(0x05, data, len);
    ProcessCommand();
    AnswerDataSize = COMToBuffer(data, AnswerDataSize);

    if (AnswerSize == 5)
    {
        if (Answer[2] != 0x50)
            Log->Write((boost::format("SetTimeOuts() faild. Answer = %1%") % GetByteExp(Answer[2]).c_str()).str().c_str());
        else
            Log->Write("SetTimeOuts() has successful done.");
    }
    else
    {
         if ((Log)&&(LoggingErrors))
         {
            Log->Write("Incorrect data length of answer of device while TimeOuts set.");
         }
    }
}

void TLDOGDeviceThread::SetID()
{
    if (!Port->PortInit)
      return;

    memset(data,0x31,4);
    SendPacket(0x12, data, 4);
    ProcessCommand();
    AnswerDataSize = COMToBuffer(data, AnswerDataSize);

    if (AnswerSize == 5)
    {
        if (Answer[2] != 0x50)
            Log->Write((boost::format("SetID() faild. Answer = %1%") % GetByteExp(Answer[2]).c_str()).str().c_str());
        else
            Log->Write("SetID() has successful done.");
    }
    else
    {
         if ((Log)&&(LoggingErrors))
         {
            Log->Write("Incorrect data length of answer of device while device ID set.");
         }
    }
}

void TLDOGDeviceThread::Life()
{
    if (!Port->PortInit)
      return;

    SendPacket(0x03);
    ProcessCommand();

    if (AnswerSize == 5)
    {
        if (Answer[2] != 0x50)
            Log->Write((boost::format("Life() faild. Answer = %1%") % GetByteExp(Answer[2]).c_str()).str().c_str());
        //else
            //Log->Write("Life() has successful done.");
    }
    else
    {
        Log->Write((boost::format("Life() faild. AnswerDataSize = %1%; AnswerSize = %2%; data = ") % AnswerDataSize % AnswerSize).str().c_str());
        Log->WriteBuffer(data, AnswerDataSize);
    }
}

void TLDOGDeviceThread::ClearGSM()
{
    if (!Port->PortInit)
      return;

    SendPacket(0x01);
    ProcessCommand();

    if (AnswerSize == 5)
    {
        if (Answer[2] != 0x50)
            Log->Write((boost::format("ClearGSM() faild. Answer = %1%") % GetByteExp(Answer[2]).c_str()).str().c_str());
        else
            Log->Write("ClearGSM() has successful done.");
    }
    else
    {
        Log->Write("ClearGSM() faild.");
    }
}

void TLDOGDeviceThread::ResetPC()
{
    if (!Port->PortInit)
      return;

    SendPacket(0x00);
    ProcessCommand();

    if (AnswerSize == 5)
    {
        if (Answer[2] != 0x50)
            Log->Write((boost::format("ResetPC() faild. Answer = %1%") % GetByteExp(Answer[2]).c_str()).str().c_str());
        else
            Log->Write("ResetPC() has successful done.");
    }
    else
    {
        Log->Write("ResetPC() faild.");
    }
}

void TLDOGDeviceThread::GetDoorSensors()
{
    if (!Port->PortInit)
      return;

    SendPacket(0x07);
    ProcessCommand();
    AnswerDataSize = COMToBuffer(data, AnswerDataSize);

    Door_1 = Door_2 = 0;
    if (AnswerDataSize == 3)
    {
        Door_1 = data[0];
        Door_2 = data[1];
    }
    else
    {
         if ((Log)&&(LoggingErrors))
         {
            Log->Write("Incorrect data length of answer of device while ID request.");
         }
    }
}

void TLDOGDeviceThread::PollingLoop()
{
  if (!Port->PortInit)
    return;

  if (Terminated) return;

  SetID();
  GetVersion();
  if (OnlyPnP == false)
  {
      GetDoorSensors();
      SetTimeOuts();
      Sleep(1000);
      GetTimeOuts();
  }

  while(!Terminated)
  {
    if (OnlyPnP == false)
        Life();
    ProcessOutCommand();
    int ticks = (int)ceill(_PollingInterval/10);
    for(int i = 1; i<=ticks; i++)
    {
      Sleep(10);
      ProcessOutCommand();
      if (Terminated)
        break;
    }
  }
  Log->Write("Exit from PollingLoop().");
  ProcessOutCommand();
}

void TLDOGDeviceThread::ProcessOutCommand()
{
    //обновим метку времени, типа ещё жив поток
    CreationTime = clock();
    int command = GetExtCommand();
    switch (command)
    {
      case WD2_CLEARGSM:
        Log->Write("Reset Modem...");
        ClearGSM();
        break;
      case WD2_RESET:
        Log->Write("Reset PC...");
        ResetPC();
        break;
      case WD2_VERSION:
        Log->Write("Get Version");
        GetVersion();
        break;
      case WD2_GETSENSORS:
        Log->Write("Get Door Sensors");
        GetDoorSensors();
        break;
    }
}

void __fastcall TLDOGDeviceThread::ProcessLoopCommand()
{
  PollingLoop();
}

