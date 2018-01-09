//---------------------------------------------------------------------------


#pragma hdrstop

#include "V2EDeviceThread.h"
#include "globals.h"
#include "common.h"
#include "CValidator.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

v2e::CStates   currentState;
v2e::CCommands currentCommand;

__fastcall TV2EDeviceThread::TV2EDeviceThread() : TDeviceThread(true)
{
    currentState = v2e::CStates();
    BillNotes.clear();
    noPoll = false;
}

__fastcall TV2EDeviceThread::~TV2EDeviceThread()
{
}

BYTE TV2EDeviceThread::getCheckByte(BYTE* DataBuf, int end)
{
   BYTE check = 0;

   for (int i = 0; i < end - 1; i++)
      check += DataBuf[i];
      
   return ~check;
}

void TV2EDeviceThread::SendPacket(BYTE command, BYTE* _data, BYTE len_packet)
{
  if (!Port->PortInit)
  {
    DeviceState->StateCode = 0xff;
    return;
  }
  try
  {
    if((command != v2e::ENABLE) && (command != v2e::POLL))
        Log->Write((boost::format("Send %1%...") % currentCommand.getDescription(command)).str().c_str());
    activeSleep(_PollingInterval);

    ClearCommand();
    ClearAnswer();

    Command[0] = BeginByte;
    Command[1] = 0x00;
    Command[2] = 0x00;
    Command[3] = len_packet;
    Command[4] = command;
    if((data) && (len_packet != v2e::PACKET_SIZE))
    {
        memcpy(&Command[v2e::PACKET_SIZE - 2], _data, len_packet - v2e::PACKET_SIZE);
    }
    Command[len_packet - 2] = 0xFF;
    Command[len_packet - 1] = getCheckByte(Command, len_packet);

    CommandSize = len_packet;
  }
  __finally
  {
  }
}

void TV2EDeviceThread::Poll()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            return;
        }

        SendPacket(v2e::POLL);
        SendType = RecieveAnswer;
        ProcessCommand();

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

void TV2EDeviceThread::Reset()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            return;
        }

        SendType = RecieveAnswer;
        SendPacket(v2e::RESET);
        ProcessCommand();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TV2EDeviceThread::Stack()
{
    try
    {
        noPoll = true;
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            return;
        }

        SendType = RecieveAnswer;
        SendPacket(v2e::STACK);
        ProcessCommand();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TV2EDeviceThread::SetMode()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            return;
        }

        BYTE* _data = new BYTE[1];
        _data[0] = 0x01;   //poll mode, escrow enable

        SendType = RecieveAnswer;
        SendPacket(v2e::SET_MODE, _data, 8);
        ProcessCommand();
        delete [] _data;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TV2EDeviceThread::Return()
{
    try
    {
        noPoll = true;
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            return;
        }

        SendType = RecieveAnswer;
        SendPacket(v2e::RETURN);
        ProcessCommand();
        noPoll = false;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TV2EDeviceThread::Hold()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            return;
        }

        SendType = RecieveAnswer;
        SendPacket(v2e::HOLD);
        ProcessCommand();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TV2EDeviceThread::GetNominalTable()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            return;
        }
        BYTE* _data = new BYTE[v2e::protocolID_BytesCount];
        _data[0] = 0x01;

        SendType = RecieveAnswer;
        SendPacket(v2e::GET_NOMINAL_TABLE, _data, v2e::PACKET_SIZE + v2e::protocolID_BytesCount);
        ProcessCommand(v2e::MODE::INIT);

        delete [] _data;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TV2EDeviceThread::changeAccept(bool enable)
{
    try
    {
        if (!Port->PortInit)
        {
          DeviceState->StateCode = 0xff;
          DeviceState->OutStateCode = DSE_NOTMOUNT;
          return;
        }

        BYTE* _data = new BYTE[v2e::NominalBytesCount];
        _data[0] = enable ? 0x7F : 0x00;
        for(int i = 1; i < v2e::NominalBytesCount; i++)
        {
            _data[i] = 0x00;
        }

        SendPacket(v2e::ENABLE, _data, v2e::NominalBytesCount + v2e::PACKET_SIZE);
        ProcessCommand();
        Log->Write((boost::format("command %1%") % (enable ? "Enable()" : "Disable()")).str().c_str());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TV2EDeviceThread::PollingLoop()
{
    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0;
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      return;
    }

    if (Terminated) return;
    ProcessOutCommand();
    if (Terminated) return;

    int OldState = DeviceState->StateCode;
    Poll();
    int State = DeviceState->StateCode;

    if (Terminated) return;

    try
    {
        DeviceState->Enabling = false;
        DeviceState->Nominal = 0;
        DeviceState->OutStateCode = DSE_OK;

        if (((OldState == State) || (State == 0)) && (EnterLoop == false))
            return;

        if (Answer[4] != v2e::POLL)
            return;
        //*************************************************************************************************
        switch (DeviceState->StateCode & v2e::IDLE_MASK)
        {
            case v2e::ENABLING:
                DeviceState->Enabling = true;
                DeviceState->Stacked = false;

                if (DeviceState->Stacked)
                    DeviceState->Processing = false;

                if (DeviceState->OutStateCode == DSE_STACKEROPEN)
                {
                   Log->Write("CASSETTE RETURNED");
                }
            break;
            case v2e::DISABLING:
                DeviceState->Idle = true;
                DeviceState->Enabling = false;
                DeviceState->Stacked = false;
            break;
        }
        //*************************************************************************************************
        switch (DeviceState->StateCode & v2e::STATE_MASK)
        {
            case v2e::ACCEPTING:
                DeviceState->Processing = true;
                DeviceState->Stacking = true;
            break;
            //----------------------------------------------------------------
            case v2e::ESCROW:
            {
                    DeviceState->Stacking = true;
                    int nominal = GetCurrentNominal();

                    if((nominal <= MaxCash)&&(nominal >= MinCash))
                    {
                        Stack();
                        Log->Write((boost::format("Escrowed nominal = %1%") % nominal).str());
                    }
                    else
                    {
                        Log->Write((boost::format("Returning nominal = %1%") % nominal).str());
                        Return();
                    }
            }
            break;
            //----------------------------------------------------------------
            case v2e::BAD_ESCROW:
                //все, что надо - делается до этого
            break;
            //----------------------------------------------------------------
            case v2e::STACKING:
                //все, что надо - делается до этого
            break;
            //----------------------------------------------------------------
            case v2e::STACKED:
                if (DeviceState->Stacking)
                {
                    DeviceState->Stacked = true;
                    DeviceState->Stacking = false;
                    DeviceState->Billing = true;

                    DeviceState->Nominal = GetCurrentNominal();
                    Log->Write((boost::format("Stacked nominal = %1%")
                      % DeviceState->Nominal).str().c_str());
                    DeviceState->Global += DeviceState->Nominal;
                    DeviceState->Count++;
                }
                else
                {
                    Log->Write((boost::format("Error! Stacking nominal = %1% without payment operation")
                      % GetCurrentNominal()).str().c_str());
                    return;
                }
            break;
            //----------------------------------------------------------------
            case v2e::REJECTING:
            case v2e::RETURNING:
                DeviceState->Stacking = false;
                DeviceState->OutStateCode = DSE_BILLREJECT;
            break;
            //----------------------------------------------------------------
            case v2e::POWER_ON:
            case v2e::POWER_ON_BILL:
                //Reset();
            break;
            //----------------------------------------------------------------
            case 0:
                DeviceState->Stacking = false;
            break;
            //----------------------------------------------------------------
            default:
                Log->Write((boost::format("Unknown state: 0x%08X") % DeviceState->StateCode).str());
            break;
        }
        //*************************************************************************************************
        if(DeviceState->StateCode & v2e::HARDWARE_ERROR)
        {
            DeviceState->Stacking = false;
            switch(DeviceState->StateCode & v2e::ERROR_MASK)
            {
                case v2e::STACKER_OPEN:
                    DeviceState->OutStateCode = DSE_STACKEROPEN;
                    DeviceState->Idle = true;
                break;
                //------------------------------------------------------------
                case v2e::STACKER_FULL:
                    DeviceState->OutStateCode = DSE_STACKERFULL;
                    DeviceState->Idle = true;
                break;
                //------------------------------------------------------------
                case v2e::JAM_ON_ACCEPTOR:
                case v2e::JAM_ON_STACKER:
                    DeviceState->Stacking = false;
                    DeviceState->OutStateCode = DSE_BILLJAM;
                break;
                //------------------------------------------------------------
                case v2e::CHEATED:
                    DeviceState->Stacking = false;
                    DeviceState->OutStateCode = DSE_CHEATED;
                break;
                //------------------------------------------------------------
                case v2e::REJECTING:
                case v2e::RETURNING:
                    DeviceState->Stacking = false;
                    DeviceState->OutStateCode = DSE_BILLREJECT;
                break;
                //------------------------------------------------------------
                case 0:
                    //если имеет место какая-то другая ошибка, резетим купюрник
                    if (DeviceState->OutStateCode != DSE_HARDWARE_ERROR)
                    {
                        Reset();
                    }
                    DeviceState->OutStateCode  = DSE_HARDWARE_ERROR;
                break;
                default:
                    //DeviceState->OutStateCode  = DSE_HARDWARE_ERROR;
                break;
            }
        }

        if (Terminated)  return;
        if(!DeviceState->Idle)
            Log->Write((boost::format("StateCode = %1%; OldStateCode = %2%")
              % DeviceState->StateCode
              % DeviceState->OldStateCode).str().c_str());

        DWORD OldStateCode = DeviceState->OldStateCode;
        DeviceState->OldStateCode = DeviceState->StateCode;

        //Send Notification

        if((DeviceState->StateCode & v2e::STATE_MASK) == v2e::STACKED)
            Log->Write((boost::format("isMiddleProcessing = %1%; isWork = %2%; StateCode = %3%; OldStateCode = %4%")
              % (isMiddleProcessing() ? "true" : "false")
              % (isWork()             ? "true" : "false")
              % DeviceState->StateCode
              % OldStateCode).str().c_str());
        if((!isMiddleProcessing() || !isWork()) && (DeviceState->StateCode != OldStateCode))
        {
            ChangeDeviceState();
        }
    }
    catch(...) {}
}

void TV2EDeviceThread::ParseAnswer(int _mode)
{
    try
    {
        ClearBuffer(data);
        if (DeviceState)
            AnswerSize = DeviceState->AnswerSize;

        //Log->Write("Answer from GPT...");
        //Log->WriteBuffer(Answer, AnswerSize);
        switch(_mode)
        {
            case v2e::MODE::POLL:
            {
                if(Answer[4] == v2e::POLL)
                {
                    DeviceState->StateCode = 0;
                    memcpy(&DeviceState->StateCode, &Answer[5], 3);
                    DeviceState->Idle = false;  //true в PollingLoop

                    DeviceState->StateDescription = currentState.getDescription(DeviceState->StateCode);
                    Log->Write((boost::format("StateCode = 0x%08X, Description = %s")
                      % DeviceState->StateCode
                      % DeviceState->StateDescription).str());
                }     
            }
            break;
            case v2e::MODE::INIT:
                InitNominalTable();
            break;
            default:
                Log->Write("Unknown mode for v2e!!!");
            break;
        }
        noPoll = false;
    }
  __finally
  {
  }

}

bool TV2EDeviceThread::IsItYou()
{
    Poll();
    if (!isWork())
        return false;

    changeAccept(false);
    Poll();
    if (!isWork())
        return false;

    return true;
}

void TV2EDeviceThread::ProcessOutCommand()
{
    try
    {
        BYTE Command = GetExtCommand();
        switch (Command)
        {
          case oc_DisableBill:
            Log->Write("ExtCommand DisableBill");
            changeAccept(false);
          break;
          case oc_EnableBill:
            Log->Write("ExtCommand EnableBill");
            changeAccept(true);
          break;
        }
    }
    __finally
    {
    }
}

bool TV2EDeviceThread::isWork()
{
    DWORD state = DeviceState->StateCode & v2e::STATE_MASK;
    DWORD error = DeviceState->StateCode & v2e::ERROR_MASK;

    return !(((state & v2e::HARDWARE_ERROR) == v2e::HARDWARE_ERROR) ||
             (error == v2e::STACKER_OPEN) ||
             (error == v2e::STACKER_FULL) ||
             (error == v2e::JAM_ON_ACCEPTOR) ||
             (error == v2e::JAM_ON_STACKER) ||
             (error == v2e::CHEATED) ||
             (state == v2e::POWER_ON) ||
             (state == v2e::POWER_ON_BILL));
}

bool TV2EDeviceThread::isMiddleProcessing()
{
    DWORD state = DeviceState->StateCode & v2e::STATE_MASK;
    return ((state == v2e::ESCROW) ||
            (state == v2e::BAD_ESCROW) ||
            (state == v2e::STACKING) ||
            (state == v2e::REJECTING) ||
            (state == v2e::RETURNING));
}

void __fastcall TV2EDeviceThread::ProcessLoopCommand()
{
    ThreadTerminated = false;
    try
    {
        if (Terminated) return;
        Reset();
        GetNominalTable();
        SetMode();
        SetOrientation();
        SetSecurity();
        if (DeviceState->StateCode != 0xff)
        {
            DeviceState->OutStateCode = DSE_OK;
            Log->Write("Device is mount.");
        }
        else
        {
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            Log->Write("No Device Present.");
        }
        while(!Terminated)
        {
            if (Terminated)
              return;
            if (!noPoll)
                PollingLoop();
            else
            {
                activeSleep(_PollingInterval);
            }
        }
    }
    __finally
    {
    }
}

int TV2EDeviceThread::GetCurrentNominal()
{
    try
    {
        int position = Answer[8] - 1;
        return BillNotes[position].Nominal;

    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return 0;
    }
}

void TV2EDeviceThread::InitNominalTable()
{
    try
    {
        int nominal_count = (AnswerSize - v2e::PACKET_SIZE - v2e::protocolID_BytesCount)/v2e::NOTE_SIZE;

        Log->Write("Available nominals = ");
        for(int i = 0; i < nominal_count;i++)
        {
            int j = v2e::PACKET_SIZE - 1 + i*v2e::NOTE_SIZE;

            int _nominal = Answer[j + 4]*pow(10, Answer[j + 5]);
            std::string _ISOcode = GetStrFromBuffer(&Answer[j + 1], 3);
            BillNotes.push_back(TNote(_nominal, 0, 0, _ISOcode));
            Log->Append(((i ? ", " : "") + IntToStr(_nominal)).c_str());
        }
    }
    catch (...)
    {
        Log->Write("Fatal Error! it is impossible to get available nominals! ");
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        if (BillNotes.empty())
        {
            Log->Append("Exiting from thread...");
            exit(0);        
        }
    }
}

void TV2EDeviceThread::SetOrientation()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            return;
        }

        BYTE* _data = new BYTE[1];
        _data[0] = 0x00;   //whatever orientation    0x00 - правильно

        SendType = RecieveAnswer;
        SendPacket(v2e::SET_ORIENTATION, _data, 8);
        ProcessCommand();
        delete [] _data;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TV2EDeviceThread::SetSecurity()
{
    try
    {
        if (!Port->PortInit)
        {
            DeviceState->StateCode = 0xff;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            return;
        }

        BYTE* _data = new BYTE[2];
        _data[0] = 0xFF;
        _data[1] = 0xFF;   //high security level for all bills

        SendType = RecieveAnswer;
        SendPacket(v2e::SET_SECURITY, _data, 9);
        ProcessCommand();
        delete [] _data;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}
