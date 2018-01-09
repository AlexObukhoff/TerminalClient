//---------------------------------------------------------------------------
#pragma hdrstop
#include "CValidator.h"
#include "globals.h"
#include "boost/format.hpp"
//---------------------------------------------------------------------------

#pragma package(smart_init)

CValidator::CValidator(int id, int ComPort,TLogClass* _Log, AnsiString Prefix, int _mode)
: TDeviceClass(ComPort,_Log, Prefix)
{
    ID = id;
    mode = _mode;
    ExchangeRate = 1;
    Currency = "rur";
    MaxCash = 0;
    MinCash = 1;
    DeviceName = "Validator";
    BillsSensitivity.ByteCode = 0x00;
    BillsInhibit.ByteCode = 0x00;
    m_enable = false;
}

bool CValidator::isEnableBill()
{
    return m_enable;
}

CValidator::~CValidator()
{
  try
  {
    StopPooling();
  }
  __finally
  {
  }
}

bool CValidator::IsItYou()
{
    if (DeviceThread)
        return DeviceThread->IsItYou();
    else
        return false;
}

void CValidator::SetMaxCash(double value)
{
    MaxCash = value;
    if (DeviceThread)
    {
        DeviceThread->MaxCash = MaxCash;
        Log->Write((boost::format("SetMaxCash(): MaxCash=%1%") % MaxCash).str().c_str());
    }
}

void CValidator::SetMinCash(double value)
{
    MinCash = value;
    if (DeviceThread)
    {
        DeviceThread->MinCash = MinCash;
        Log->Write((boost::format("SetMinCash(): MinCash=%1%") % MinCash).str().c_str());
    }
}

void CValidator::EnableBill()
{
    m_enable = true;
    ClearMoney();
    DeviceState->BillEnable = true;
    SetExtCommand(oc_EnableBill);
    if ((DeviceState)&&(Log))
        Log->Write((boost::format("Idle=%1%") % DeviceState->Idle).str().c_str());
    /*if (DeviceThread)
    {
        DeviceThread->EnterLoop = true;
        CheckFrozen();
    }*/
}

void CValidator::DisableBill()
{
    m_enable = false;
    DeviceState->BillEnable = false;
    //SetMaxCash(0);
    SetExtCommand(oc_DisableBill);
    if (ErrorMode == DSE_OK)
      while(!DeviceState->Idle)
      {
        Application->ProcessMessages();
        ChangeDeviceState(true);
      }
    if ((DeviceState)&&(Log))
        Log->Write((boost::format("Idle=%1%") % DeviceState->Idle).str().c_str());
    /*if (DeviceThread)
    {
        DeviceThread->EnterLoop = true;
        CheckFrozen();
    }*/
}

void CValidator::getVersion()
{
    std::string version;
    SetExtCommand(oc_getVersion);
}

void CValidator::ClearMoney()
{
  if (DeviceThread)
  {
    DeviceThread->bill_count_old = 0;
    DeviceThread->bill_count = 0;
  }
}

double CValidator::GetMoney()
{
  double bill_count = 0;
  if (DeviceThread)
    bill_count = DeviceThread->bill_count;
  return bill_count;
}

void CValidator::StartDevice()
{
  StartPooling();
}

bool CValidator::IsInitialized()
{
    if (DeviceThread)
        return DeviceThread->IsInitialized();
    else
        return Initialized;
}

bool CValidator::CheckFrozen()
{
    clock_t CurrentTime = clock();
    if((Port != NULL) && (CurrentTime - GetTimeStamp1()) > (clock_t)(FrozenTimeOut * 1000))
    {//поток завис
        if(DeviceState)
        {
           DeviceState->OutStateCode = DSE_NOTMOUNT;
           if (DeviceState->OldOutStateCode != DeviceState->OutStateCode)
           {
              Log->Write("Validator thread is frozen.");;
              ChangeDeviceState();
           }
        }
        return false;
    }
    else
        return true;
}

bool CValidator::CheckState()
{
    if (DeviceThread)
    {
        if (CheckFrozen())
            return true;
        else
            return false;
    }
    else
        return false;
}

bool CValidator::SetBillsSensitivity(AnsiString value)
{
    BillsSensitivity.ByteCode = 0x00;
    BillsInhibit.ByteCode = 0x00;
    try
    {
        //1
        int bit = StrToInt(value[1]);
        switch (bit)
        {
            case 0:
                BillsSensitivity.ByteBitCode.b6 = 0;
                BillsInhibit.ByteBitCode.b6 = 1;
                break;
            case 1:
                BillsSensitivity.ByteBitCode.b6 = 0;
                BillsInhibit.ByteBitCode.b6 = 0;
                break;
            case 2:
                BillsSensitivity.ByteBitCode.b6 = 1;
                BillsInhibit.ByteBitCode.b6 = 0;
                break;
        }
        //2
        bit = StrToInt(value[2]);
        switch (bit)
        {
            case 0:
                BillsSensitivity.ByteBitCode.b5 = 0;
                BillsInhibit.ByteBitCode.b5 = 1;
                break;
            case 1:
                BillsSensitivity.ByteBitCode.b5 = 0;
                BillsInhibit.ByteBitCode.b5 = 0;
                break;
            case 2:
                BillsSensitivity.ByteBitCode.b5 = 1;
                BillsInhibit.ByteBitCode.b5 = 0;
                break;
        }
        //3
        bit = StrToInt(value[3]);
        switch (bit)
        {
            case 0:
                BillsSensitivity.ByteBitCode.b4 = 0;
                BillsInhibit.ByteBitCode.b4 = 1;
                break;
            case 1:
                BillsSensitivity.ByteBitCode.b4 = 0;
                BillsInhibit.ByteBitCode.b4 = 0;
                break;
            case 2:
                BillsSensitivity.ByteBitCode.b4 = 1;
                BillsInhibit.ByteBitCode.b4 = 0;
                break;
        }
        //4
        bit = StrToInt(value[4]);
        switch (bit)
        {
            case 0:
                BillsSensitivity.ByteBitCode.b3 = 0;
                BillsInhibit.ByteBitCode.b3 = 1;
                break;
            case 1:
                BillsSensitivity.ByteBitCode.b3 = 0;
                BillsInhibit.ByteBitCode.b3 = 0;
                break;
            case 2:
                BillsSensitivity.ByteBitCode.b3 = 1;
                BillsInhibit.ByteBitCode.b3 = 0;
                break;
        }
        //5
        bit = StrToInt(value[5]);
        switch (bit)
        {
            case 0:
                BillsSensitivity.ByteBitCode.b2 = 0;
                BillsInhibit.ByteBitCode.b2 = 1;
                break;
            case 1:
                BillsSensitivity.ByteBitCode.b2 = 0;
                BillsInhibit.ByteBitCode.b2 = 0;
                break;
            case 2:
                BillsSensitivity.ByteBitCode.b2 = 1;
                BillsInhibit.ByteBitCode.b2 = 0;
                break;
        }
        //6
        bit = StrToInt(value[6]);
        switch (bit)
        {
            case 0:
                BillsSensitivity.ByteBitCode.b0 = 0;
                BillsInhibit.ByteBitCode.b0 = 1;
                break;
            case 1:
                BillsSensitivity.ByteBitCode.b0 = 0;
                BillsInhibit.ByteBitCode.b0 = 0;
                break;
            case 2:
                BillsSensitivity.ByteBitCode.b0 = 1;
                BillsInhibit.ByteBitCode.b0 = 0;
                break;
        }

        if (DeviceThread)
        {
            DeviceThread->BillsSensitivity = BillsSensitivity.ByteCode;
            DeviceThread->BillsInhibit = BillsInhibit.ByteCode;
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return true;
}

