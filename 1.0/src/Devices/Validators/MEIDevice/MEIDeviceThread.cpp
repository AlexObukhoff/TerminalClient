//---------------------------------------------------------------------------


#pragma hdrstop

#include <math.h>
#include "MEIDeviceThread.h"
#include "CValidator.h"
#include "globals.h"
#include "boost/format.hpp"

//---------------------------------------------------------------------------

#pragma package(smart_init)

#define PollingInterval 100

__fastcall TMEIDeviceThread::TMEIDeviceThread(int _mode) : TDeviceThread(true)
{
    mode = _mode;
    ACKAnswered = true;
    LastACK = 1;
    BeginByte = 0x02;
    DataLengthIndex = 1;
    CRCLength = 1;
    PollCounterLimit = 100;
    PollCounter = 0;

    ClearByteFields();

    host_Byte0.ByteCode = 0x7F;
    host_Byte2.ByteCode = 0x00;

    host_Byte2.ByteBitCode.NoPushMode = 1;
}

__fastcall TMEIDeviceThread::~TMEIDeviceThread()
{
}

void TMEIDeviceThread::ClearByteFields()
{
    acc_Byte0.ByteCode = 0x00;
    acc_Byte1.ByteCode = 0x00;
    acc_Byte2.ByteCode = 0x00;
    acc_Byte3.ByteCode = 0x00;

    host_Byte1.ByteCode = 0x00;
    host_Byte1.ByteBitCode.Orientation0 = 1;
    host_Byte1.ByteBitCode.Orientation1 = 1;
    host_Byte1.ByteBitCode.EscrowEnable = 1;
}

unsigned short TMEIDeviceThread::Calc_CRC(BYTE* DataBuf)
{
   BYTE CRC = DataBuf[1];
   for (int i = 2; i <= 5; i++)
      CRC ^= DataBuf[i];
   return CRC;
}

void TMEIDeviceThread::SendPacket(BYTE* _data)
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

    Command[0] = BeginByte;
    Command[1] = 0x08;
    memcpy(&Command[2],_data,4);
    Command[6] = 0x03;
    Command[7] = Calc_CRC(Command);
    CommandSize = 8;
  }
  __finally
  {
  }
}

void TMEIDeviceThread::ParseAnswer(int _mode)
{
  try
  {
    if (DeviceState)
      AnswerSize = DeviceState->AnswerSize;
    if (AnswerSize >= 11)
    {
       ACKAnswered = true;
       PollCounter = 0;

       memset((BYTE*)AnswerData, 0, receiveDataCount[mode]);
       memcpy((BYTE*)AnswerData, &Answer[2], receiveDataCount[mode]);

       BYTE HostACK = LastACK;
       int modeTypeIndex = 0;
       if((AnswerSize - 4) > receiveDataCount[2])
           modeTypeIndex = 1;
       else if ((AnswerSize - 4) > receiveDataCount[0])
           modeTypeIndex = 2;
       else
           modeTypeIndex = 0;
       MEI_MODE::MEI_MODE answerMode = static_cast<MEI_MODE::MEI_MODE>(modeTypeIndex);
       BYTE AcceptorACK = Answer[2] & MEI_MODE::ACK_MASK;
       if ((HostACK != AcceptorACK)&&(LoggingErrors))
       {
           ACKAnswered = false;
           Log->Write((boost::format("Unequal ACK number: HostACK = %1%; AcceptorACK = %2%") % HostACK % AcceptorACK).str().c_str());
       }

       DeviceState->StateCode = 0x00;
       int datashift = 1;
       switch(answerMode)
       {
           case MEI_MODE::Simple:
           break;
           case MEI_MODE::Extended:
               datashift = 2;
           break;
           case MEI_MODE::Version:
               DeviceState->ProjectNumber = GetIntFromBuffer(&AnswerData[2], 5);
               DeviceState->FirmWare = GetDoubleFromBuffer(&AnswerData[7], 3)/100;
           break;
       }
       acc_Byte0.ByteCode = AnswerData[datashift + 0];
       acc_Byte1.ByteCode = AnswerData[datashift + 1];
       acc_Byte2.ByteCode = AnswerData[datashift + 2];
       acc_Byte3.ByteCode = AnswerData[datashift + 3];
       DeviceState->StateCode = GetStateCodeFromByteFields();
       if ((Log)&&(LoggingErrors))
       {
          Log->Write("Answer data:");
          Log->WriteBuffer(AnswerData, receiveDataCount[modeTypeIndex]);
       }
    }
    else
       ACKAnswered = false;
  }
  __finally
  {
  }
}

BYTE TMEIDeviceThread::GetACKNumber()
{
    if (ACKAnswered)
    {
        switch (LastACK)
        {
            case 0:
                LastACK = 1;
                break;
            case 1:
                LastACK = 0;
                break;
            default :
                LastACK = 0;
                break;
        }
    }
    return LastACK;
}

void TMEIDeviceThread::Poll(int mode)
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            return;
        }

        SendData[0] = 0x10 + ACK;
        SendData[1] = host_Byte0.ByteCode;
        SendData[2] = host_Byte1.ByteCode;
        SendData[3] = host_Byte2.ByteCode;

        SendType = RecieveAnswer;
        SendPacket(SendData);
        ProcessCommand(mode);

        //checking device activity
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
        CheckDeviceActivity(DeviceState->AnswerSize);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TMEIDeviceThread::Stack()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            return;
        }

        ClearByteFields();
        host_Byte1.ByteBitCode.Stack = 1;

        SendData[0] = 0x10 + ACK;
        SendData[1] = host_Byte0.ByteCode;
        SendData[2] = host_Byte1.ByteCode;
        SendData[3] = host_Byte2.ByteCode;

        SendType = RecieveAnswer;
        SendPacket(SendData);
        ProcessCommand();
        Log->Write("Command Stack()");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TMEIDeviceThread::Escrow()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            return;
        }

        ClearByteFields();
        host_Byte1.ByteBitCode.EscrowEnable = 1;

        SendData[0] = 0x10 + ACK;
        SendData[1] = host_Byte0.ByteCode;
        SendData[2] = host_Byte1.ByteCode;
        SendData[3] = host_Byte2.ByteCode;

        SendType = RecieveAnswer;
        SendPacket(SendData);
        ProcessCommand();
        Log->Write("Command Escrow()");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TMEIDeviceThread::Return()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            return;
        }

        ClearByteFields();
        host_Byte1.ByteBitCode.Return = 1;

        SendData[0] = 0x10 + ACK;
        SendData[1] = host_Byte0.ByteCode;
        SendData[2] = host_Byte1.ByteCode;
        SendData[3] = host_Byte2.ByteCode;

        SendType = RecieveAnswer;
        SendPacket(SendData);
        ProcessCommand();
        Log->Write("Command Return()");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TMEIDeviceThread::Enable()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            return;
        }

        ClearByteFields();
        //host_Byte1.ByteBitCode.EscrowEnable = 1;

        if (mode = MEI_MODE::Extended)
            host_Byte2.ByteBitCode.Mode = 1;
        else if (mode = MEI_MODE::Simple)
            host_Byte2.ByteBitCode.Mode = 0;
        SendData[0] = 0x10 + ACK;
        SendData[1] = host_Byte0.ByteCode = 0x7F;
        SendData[2] = host_Byte1.ByteCode;
        SendData[3] = host_Byte2.ByteCode;

        SendType = RecieveAnswer;
        SendPacket(SendData);
        ProcessCommand();
        Log->Write("Command Enable()");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TMEIDeviceThread::Disable()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            return;
        }

        ClearByteFields();

        host_Byte2.ByteBitCode.EnableDecodBarCode = 0;
        SendData[0] = 0x10 + ACK;
        SendData[1] = host_Byte0.ByteCode = 0x00;
        SendData[2] = host_Byte1.ByteCode;
        SendData[3] = host_Byte2.ByteCode;

        SendType = RecieveAnswer;
        SendPacket(SendData);
        ProcessCommand();
        Log->Write("Command Disable()");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TMEIDeviceThread::Reset()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            return;
        }

        ClearByteFields();

        SendData[0] = 0x60 + ACK;
        SendData[1] = 0x7F;
        SendData[2] = 0x7F;
        SendData[3] = 0x7F;

        SendType = NotRecieveAnswer;
        SendPacket(SendData);
        ProcessCommand();
        SendType = RecieveAnswer;
        Log->Write("Command Reset()");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TMEIDeviceThread::getVersion()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            return;
        }

        ClearByteFields();

        SendData[0] = 0x60 + ACK;
        SendData[1] = 0x00;
        SendData[2] = 0x00;
        SendData[3] = 0x07;

        SendType = RecieveAnswer;
        SendPacket(SendData);
        ProcessCommand(MEI_MODE::Version);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

int TMEIDeviceThread::GetStateCodeFromByteFields()
{
    int result = acc_Byte0.ByteCode;
    result += acc_Byte1.ByteCode<<8;
    result += acc_Byte2.ByteCode<<16;
    result += acc_Byte3.ByteCode<<24;
    return result;
}

int TMEIDeviceThread::GetCurrentNominal(MEI_MODE::MEI_MODE modeType)
{
    int result = 0;
    switch(modeType)
    {
        case MEI_MODE::Simple:
        {
            if (acc_Byte2.ByteBitCode.NoteValue0)
                result |= 0x01;
            if (acc_Byte2.ByteBitCode.NoteValue1)
                result |= 0x02;
            if (acc_Byte2.ByteBitCode.NoteValue2)
                result |= 0x04;
            switch (result)
            {
                case 0:
                    result = 0;
                    break;
                case 1:
                    result = 10;
                    break;
                case 2:
                    result = 50;
                    break;
                case 3:
                    result = 100;
                    break;
                case 4:
                    result = 500;
                    break;
                case 5:
                    result = 1000;
                    break;
                case 6:
                    result = 5000;
                    break;
                case 7:
                    result = 0;
                    break;
            }
        }
        case MEI_MODE::Extended:
        {
            int base = GetIntFromBuffer(&AnswerData[12], 3);
            int exp = GetIntFromBuffer(&AnswerData[15], 3);
            result = base*pow(10, exp);
        }
    }

    return result;
}

void TMEIDeviceThread::PollingLoop()
{
    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0x0000;
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      return;
    }

    if (Terminated) return;
    ProcessOutCommand();
    if (Terminated) return;

    int OldState = DeviceState->StateCode;
    Poll(mode);
    int State = DeviceState->StateCode;

    if (Terminated) return;

    try
    {
        DeviceState->Enabling = false;

        if ((OldState == State)&&(EnterLoop == false)&&(!((acc_Byte0.ByteCode == 0x01)&&(acc_Byte1.ByteCode == 0x10))))
            return;

        if(State == 0)
            return;

        EnterLoop = false;
        DeviceState->Billing = false;

        //===============================================
        if (acc_Byte0.ByteBitCode.Stacked)
        {
            if (DeviceState->Stacking)
            {
                DeviceState->Stacked = true;
                DeviceState->Stacking = false;
                DeviceState->Billing = true;
                DeviceState->OutStateCode = DSE_OK;
                DeviceState->StateDescription = "STACKED";
                Log->Write(DeviceState->StateDescription.c_str());
                if (mode = MEI_MODE::Extended)
                {
                    Log->Write("Extended MEI mode! InfoBuffer:");
                    Log->WriteBuffer(AnswerData, receiveDataCount[1]);
                }
                DeviceState->Nominal = GetCurrentNominal(mode);
                Log->Write((boost::format("Stacked nominal = %1%") % DeviceState->Nominal).str().c_str());
                DeviceState->Global += DeviceState->Nominal;
                DeviceState->Count++;
            }
            else
            {
                Log->Write((boost::format("Error! Stacking nominal = %1% without payment opertaion") % DeviceState->Nominal).str().c_str());
                DeviceState->Nominal = 0;
                return;
            }
        }
        //===============================================
        if (acc_Byte1.ByteBitCode.Jammed)
        {
            DeviceState->Stacking = false;
            DeviceState->Nominal = 0;
            DeviceState->OutStateCode = DSE_BILLJAM;
            DeviceState->StateDescription = "JAMMED";
            Log->Write(DeviceState->StateDescription.c_str());
        }
        //===============================================
        if (acc_Byte1.ByteBitCode.Rejected)
        {
            DeviceState->Stacking = false;
            DeviceState->Nominal = 0;
            DeviceState->OutStateCode = DSE_BILLREJECT;
            DeviceState->StateDescription = "REJECTED";
            Log->Write(DeviceState->StateDescription.c_str());
        }
        //===============================================
        if (acc_Byte2.ByteBitCode.Failure)
        {
            DeviceState->Stacking = false;
            DeviceState->Nominal = 0;
            if (DeviceState->OutStateCode != DSE_HARDWARE_ERROR)
                Reset();
            DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
            DeviceState->StateDescription = "FAILURE";
            Log->Write(DeviceState->StateDescription.c_str());
        }
        //===============================================
        if (acc_Byte1.ByteBitCode.CassetteFull)
        {
            DeviceState->OutStateCode = DSE_STACKERFULL;
            DeviceState->Idle = true;
            DeviceState->StateDescription = "CASSETTE FULL";
            Log->Write(DeviceState->StateDescription.c_str());
        }
        //===============================================
        if (acc_Byte1.ByteBitCode.Cheated)
        {
            DeviceState->Stacking = false;
            DeviceState->Nominal = 0;
            DeviceState->StateDescription = "CHEATED";
            Log->Write(DeviceState->StateDescription.c_str());
        }
        //===============================================
        if (acc_Byte0.ByteBitCode.Returned)
        {
            DeviceState->Stacking = false;
            DeviceState->Nominal = 0;
            DeviceState->OutStateCode = DSE_BILLREJECT;
            DeviceState->StateDescription = "RETURNED";
            Log->Write(DeviceState->StateDescription.c_str());
        }
        //===============================================
        if (acc_Byte0.ByteBitCode.Stacking)
        {
            DeviceState->OutStateCode = DSE_OK;
            DeviceState->Idle = false;
            DeviceState->StateDescription = "STACKING";
            Log->Write(DeviceState->StateDescription.c_str());
        }
        //===============================================
        if (acc_Byte0.ByteBitCode.Escrowed)
        {
            DeviceState->Stacking = true;
            DeviceState->Idle = false;
            DeviceState->OutStateCode = DSE_OK;
            DeviceState->StateDescription = "ESCROWED";
            Log->Write(DeviceState->StateDescription.c_str());
            DeviceState->Nominal = GetCurrentNominal(mode);
            if((DeviceState->Nominal <= MaxCash)&&(DeviceState->Nominal >= MinCash))
            {
                Stack();
                Log->Write((boost::format("Escrowed nominal = %1%") % DeviceState->Nominal).str().c_str());
            }
            else
            {
                Log->Write((boost::format("Returning nominal = %1%") % DeviceState->Nominal).str().c_str());
                DeviceState->Nominal = 0;
                Return();
            }
        }
        //===============================================
        if (acc_Byte0.ByteBitCode.Returning)
        {
            DeviceState->Stacking = false;
            DeviceState->Nominal = 0;
            DeviceState->StateDescription = "RETURNING";
            Log->Write(DeviceState->StateDescription.c_str());
        }
        //===============================================
        if (acc_Byte0.ByteBitCode.Accepting)
        {
            DeviceState->Processing = true;
            DeviceState->Stacking = true;
            DeviceState->StateDescription = "ACCEPTING";
            Log->Write(DeviceState->StateDescription.c_str());
        }
        //===============================================
        if (acc_Byte2.ByteBitCode.PowerUp)
        {
            DeviceState->Nominal = 0;
            DeviceState->StateDescription = "POWER UP";
            Log->Write(DeviceState->StateDescription.c_str());
        }
        //===============================================
        if (acc_Byte2.ByteBitCode.InvalidCommand)
        {
            DeviceState->StateDescription = "INVALID COMMAND";
            Log->Write(DeviceState->StateDescription.c_str());
        }
        //===============================================
        if (acc_Byte1.ByteBitCode.LRC_Status == 0)
        {
            DeviceState->OutStateCode = DSE_STACKEROPEN;
            DeviceState->StateDescription = "CASSETTE REMOVED";
            Log->Write(DeviceState->StateDescription.c_str());
        }
        //===============================================
        if (acc_Byte0.ByteBitCode.Idling)
        {
            if ((acc_Byte0.ByteCode == 0x01)&&(acc_Byte1.ByteCode == 0x10))
            {
                if (DeviceState->Stacked)
                  DeviceState->Processing = false;
                DeviceState->Stacked = false;
                DeviceState->Enabling = true;

                DeviceState->Stacking = false;
                DeviceState->Idle = true;
                DeviceState->Nominal = 0;
                DeviceState->StateDescription = "IDLING";
                //Log->Write(DeviceState->StateDescription.c_str());
                if (DeviceState->OutStateCode == DSE_STACKEROPEN)
                {
                   Log->Write("CASSETTE RETURNED");
                }
                DeviceState->OutStateCode = DSE_OK;
            }
        }

        if (Terminated)  return;
        if(!DeviceState->Idle)
            Log->Write((boost::format("StateCode = %1%; OldStateCode = %2%") % DeviceState->StateCode % DeviceState->OldStateCode).str().c_str());
        DeviceState->OldStateCode = DeviceState->StateCode;

        //Send Notification
        ChangeDeviceState();
    }
    __finally
    {
    }
}

bool TMEIDeviceThread::IsItYou()
{
    return IsInitialized();
}

void __fastcall TMEIDeviceThread::ProcessLoopCommand()
{
  if (Terminated) return;
  ThreadTerminated = false;
  try
  {
      Poll();

      if (Terminated) return;

      if ((DeviceState->StateCode != 0x00)&&(DeviceState->StateCode != 0xFF))
      {
          DeviceState->OutStateCode = DSE_OK;
          if (Log) Log->Write("MEI Device has found.");
          SetInitialized();
          ChangeDeviceState();
      }
      else
      {
          DeviceState->OutStateCode = DSE_NOTMOUNT;
          if (Log) Log->Write("No MEI Device Present...");
          ChangeDeviceState();
      }

      if (Terminated) return;

      Disable();

      LoopOfCommands();
  }
  __finally
  {
  }
}

void TMEIDeviceThread::ProcessOutCommand()
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
            Enable();
            DeviceState->Idle = false;
          break;
          case oc_getVersion:
            Log->Write("ExtCommand getVersion");
            getVersion();
          break;
        }
    }
    __finally
    {
    }
}

