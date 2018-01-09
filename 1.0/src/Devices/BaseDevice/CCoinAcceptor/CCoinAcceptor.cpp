//---------------------------------------------------------------------------
#pragma hdrstop
#include "CCoinAcceptor.h"
#include "boost/format.hpp"
//---------------------------------------------------------------------------
#pragma package(smart_init)

CCoinAcceptor::CCoinAcceptor(int id, int ComPort,TLogClass* _Log, AnsiString Prefix)
: TDeviceClass(ComPort,_Log, Prefix)
{
    ID = id;
    ExchangeRate = 1;
    Currency = "rur";
    MaxCash = 0;
    MinCash = 1;
    DeviceName = "CoinAcceptor";
    m_enable = false;
}

CCoinAcceptor::~CCoinAcceptor()
{
  try
  {
    StopPooling();
  }
  __finally
  {
  }
}

bool CCoinAcceptor::isEnableBill()
{
    return m_enable;
}

bool CCoinAcceptor::IsItYou()
{
    if (DeviceThread)
        return DeviceThread->IsItYou();
    else
        return false;
}

void CCoinAcceptor::SetMaxCash(double value)
{
    MaxCash = value;
    if (DeviceThread)
    {
        DeviceThread->MaxCash = MaxCash;
        Log->Write((boost::format("SetMaxCash(): MaxCash = %1%") % MaxCash).str().c_str());
    }
}

void CCoinAcceptor::SetMinCash(double value)
{
    MinCash = value;
    if (DeviceThread)
    {
        DeviceThread->MinCash = MinCash;
        Log->Write((boost::format("SetMinCash(): MinCash = %1%") % MinCash).str().c_str());
    }
}

void CCoinAcceptor::Enable()
{
    m_enable = true;
    if ((DeviceState)&&(Log))
        Log->Write((boost::format("Idle = %1%") % DeviceState->Idle).str().c_str());
    ClearMoney();
    DeviceState->BillEnable = true;
    SetExtCommand(oc_Enable);
    /*if (DeviceThread)
    {
        DeviceThread->EnterLoop = true;
        CheckFrozen();
    }*/
}

void CCoinAcceptor::Disable()
{
    m_enable = false;
    if ((DeviceState)&&(Log))
        Log->Write((boost::format("Idle = %1%") % DeviceState->Idle).str().c_str());
    DeviceState->BillEnable = false;
    SetMaxCash(0);
    SetExtCommand(oc_Disable);
    /*if (DeviceThread)
    {
        DeviceThread->EnterLoop = true;
        CheckFrozen();
    }*/
}

void CCoinAcceptor::ClearMoney()
{
  if (DeviceThread)
  {
    DeviceThread->bill_count_old = 0;
    DeviceThread->bill_count = 0;
    DeviceState->Count = 0;
  }
}

double CCoinAcceptor::GetMoney()
{
  double bill_count = 0;
  if (DeviceThread)
    bill_count = DeviceThread->bill_count = DeviceState->Count;
  return bill_count;
}

void CCoinAcceptor::StartDevice()
{
  StartPooling();
}

bool CCoinAcceptor::IsInitialized()
{
    if (DeviceThread)
        return DeviceThread->IsInitialized();
    else
        return Initialized;
}

bool CCoinAcceptor::CheckFrozen()
{
    clock_t CurrentTime = clock();
    if((Port != NULL)&&(CurrentTime - GetTimeStamp1()) > (FrozenTimeOut*1000))
    {//поток завис
        if(DeviceState)
        {
           DeviceState->OutStateCode = DSE_NOTMOUNT;
           if (DeviceState->OldOutStateCode != DeviceState->OutStateCode)
           {
              Log->Write("CoinAcceptor thread is frozen.");;
              ChangeDeviceState();
           }
        }
        return false;
    }
    else
        return true;
}

bool CCoinAcceptor::CheckState()
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


