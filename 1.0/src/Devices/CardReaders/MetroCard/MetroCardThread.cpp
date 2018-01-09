#include <math.h>
//---------------------------------------------------------------------------
#pragma hdrstop
#include "MetroCardThread.h"
#include "globals.h"
#include "boost/format.hpp"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#define TIMEOUT 250

__fastcall TMetroCardDeviceThread::TMetroCardDeviceThread() : TDeviceThread(true)
{
  FreeOnTerminate = false;
  CardFound = false;
  StopOperation = true;
  ExtCommand = EXTC_Free;
  _PollingInterval = 500;

  ASKOPMInit = NULL;
  ASKOPMGetMenu = NULL;
  ASKOPMFindCard = NULL;
  ASKOPMWriteCard = NULL;

  InitInfo = NULL;
  FindCardInfo = NULL;
  GetMenuInfo = NULL;
  WriteCardInfo = NULL;
}

__fastcall TMetroCardDeviceThread::~TMetroCardDeviceThread()
{
  Log->Write("~TMetroCardDeviceThread()");
}

void TMetroCardDeviceThread::PollingLoop()
{
  if (ASKOPMInit == NULL)
  {
      Log->Write("Device not mount.");
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      ChangeDeviceState();
      return;
  }

  while(!Terminated)
  {
      DWORD interval = _PollingInterval;
      int ticks = (int)ceill(interval/10);
      for(int i = 1; i<=ticks; i++)
      {
        Sleep(10);
        ProcessOutCommand();
        if (Terminated)
          break;
      }

      if (StopOperation == false)
          SetExtCommand(EC_ASKOPMFindCard);
      ProcessOutCommand();
  }
  Log->Write("Exit from PollingLoop().");
}

void __fastcall TMetroCardDeviceThread::ProcessLoopCommand()
{
  PollingLoop();
}

void TMetroCardDeviceThread::ProcessOutCommand()
{
    //обновим метку времени, типа ещё жив поток
    CreationTime = clock();
    int Command = GetExtCommand();
    if (Command <= 0) return;

    int result = 0;
    switch (Command)
    {
        case EC_ASKOPMInit:
          try
          {
              Log->Write("EC_ASKOPMInit");
              if ((ASKOPMInit)&&(InitInfo))
              {
                  InitInfo->Status = InitInfo->cs_Processing;
                  InitInfo->Result = result =ASKOPMInit(InitInfo->nSystemCode,InitInfo->nDeviceCode,InitInfo->nCom);
                  InitInfo->Status = InitInfo->cs_Done;
                  if (result == 0)
                      SetInitialized();
              }
          }
          catch(...)
          {
              ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
              Log->Write("ASKOPMInit() exception");
          }
          break;

        case EC_ASKOPMFindCard:
          try
          {
              FindCardInfo->CardFound = CardFound = false;
              StopOperation = false;
              if (!IsInitialized())
                  return;
              DeviceState->StateDescription = "";
              //Log->Write("EC_ASKOPMFindCard");
              if ((ASKOPMFindCard)&&(FindCardInfo))
              {
                  FindCardInfo->ClearMembers();
                  FindCardInfo->Status = FindCardInfo->cs_Processing;
                  FindCardInfo->Result = result = ASKOPMFindCard(FindCardInfo->psCardNum);
                  if (FindCardInfo->Result == 0)
                  {
                      //send notification to webclient that has been found the card
                      DeviceState->StateDescription = FindCardInfo->psCardNum;
                      if ((!CardFound)&&(DeviceStateChanged))
                      {
                          //DeviceStateChanged(DeviceState);
                          Log->Write((boost::format("Has been found the card %1%") % FindCardInfo->psCardNum).str().c_str());
                      }
                      CardFound = true;
                      StopOperation = true;
                  }

                  FindCardInfo->Status = FindCardInfo->cs_Done;
                  FindCardInfo->CardFound = CardFound;
              }
          }
          catch(...)
          {
              ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
              Log->Write("ASKOPMFindCard() exception");
          }
          break;

        case EC_ASKOPMGetMenu:
          try
          {
              if (!IsInitialized())
              {
                  Log->Write("Card Reader still has not been initialized.");
                  return;
              }
              Log->Write("EC_ASKOPMGetMenu");
              if ((ASKOPMGetMenu)&&(GetMenuInfo))
              {
                  GetMenuInfo->Status = GetMenuInfo->cs_Processing;
                  long OrderId = GetMenuInfo->pnOrderId;
                  long* p_OrderId = &OrderId;
                  short ItemsNum = GetMenuInfo->pnItemsNum;
                  short* p_ItemsNum = &ItemsNum;
                  GetMenuInfo->ClearMembers();
                  GetMenuInfo->Result = result = ASKOPMGetMenu(p_OrderId,
                                                      GetMenuInfo->psCardNum,
                                                      GetMenuInfo->pAMenu,
                                                      p_ItemsNum,
                                                      GetMenuInfo->psCardStatus);

                  /*GetMenuInfo->ClearMembers();
                  GetMenuInfo->Result = result = ASKOPMGetMenu(&(GetMenuInfo->pnOrderId),
                                                      GetMenuInfo->psCardNum,
                                                      GetMenuInfo->pAMenu,
                                                      &(GetMenuInfo->pnItemsNum),
                                                      GetMenuInfo->psCardStatus);*/
                  GetMenuInfo->Status = GetMenuInfo->cs_Done;
                  Log->Write((boost::format("ASKOPMGetMenu(): card number = %1%; state = %2%") % GetMenuInfo->psCardNum % GetMenuInfo->psCardStatus).str().c_str());
                  GetMenuInfo->pnOrderId = OrderId;
                  GetMenuInfo->pnItemsNum = ItemsNum;
              }
          }
          catch(...)
          {
              ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
              Log->Write("ASKOPMGetMenu() exception");
          }
          break;

        case EC_ASKOPMWriteCard:
          try
          {
              if (!IsInitialized())
              {
                  Log->Write("Card Reader still has not been initialized.");
                  return;
              }
              Log->Write("EC_ASKOPMWriteCard");
              if ((ASKOPMWriteCard)&&(WriteCardInfo))
              {
                  WriteCardInfo->Status = WriteCardInfo->cs_Processing;
                  WriteCardInfo->Result = result = ASKOPMWriteCard(WriteCardInfo->pnOrderId,
                                                          WriteCardInfo->psCardNum,
                                                          WriteCardInfo->nMenuItemID,
                                                          WriteCardInfo->nAcceptedInRub,
                                                          WriteCardInfo->nAuthoriseCode,
                                                          WriteCardInfo->psCardStatus);
                  Log->Write((boost::format("ASKOPMWriteCard(): card number = %1%; state = ") % WriteCardInfo->psCardNum % WriteCardInfo->psCardStatus).str().c_str());
                  WriteCardInfo->Status = WriteCardInfo->cs_Done;
              }
          }
          catch(...)
          {
              ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
              Log->Write("ASKOPMWriteCard() exception");
          }
          break;

        case EC_StopOperation:
            StopOperation = true;
            return;
    }

    if (!DeviceState) return;

    switch (result)
    {
        case 0:
            ServerConnected = true;
            DeviceState->OutStateCode = DSE_OK;
            ChangeDeviceState();
            break;

        case 1:
            ServerConnected = false;
            DeviceState->OutStateCode = DSE_NOTMOUNT;
            ChangeDeviceState();
            break;

        case 2:
            if (Command != EC_ASKOPMFindCard)
                ServerConnected = false;
            break;
    }
}

void TMetroCardDeviceThread::SetServerStatus(bool value)
{
    _ServerConnected = value;
}

bool TMetroCardDeviceThread::GetServerStatus()
{
    return _ServerConnected;
}

