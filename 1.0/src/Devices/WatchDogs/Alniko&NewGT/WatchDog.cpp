#pragma hdrstop
#include "WatchDog.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

bool AllowThreadSendAlive;
int WatchDogType_for_Thread;

CLptPort* LptPort;

TWatchDog::TWatchDog(int LptPortNum, int WDType, TLogClass* _Log, int _ModemPort) : CWatchDog(0, _Log, "WatchDog")
{
    DeviceName = "Alnico";
    hThread = NULL;
          LptPort = NULL;
                ModemPort = _ModemPort;
                LptPort = new CLptPort(LptPortNum);
                if(!LptPort->DllIsLoaded())
                {
        delete LptPort;
                                LptPort = NULL;
                                Log->Write("LPT port can not be opened: library 'TVicPort.dll' not found.");
                }
    else
    {
        WatchDogType = WDType;

        switch (WatchDogType)
        {
          case WD_TYPE_NEW_GT:  Log->Write(">>> WatchDog type - NewGT <<<");
          break;
          case WD_TYPE_ALNIKO:  Log->Write(">>> WatchDog type - WatchDog_Alniko <<<");
          break;
          default: Log->Write("!!! WatchDog type unknown!!!");
        }
    }
}
TWatchDog::~TWatchDog()
{
        if(AllowThreadSendAlive)        StopTimer();
    if(LptPort)
    {
            delete LptPort;
        LptPort = NULL;
    }
}
//==============================================================================
void TWatchDog::StartTimer()
{
  if (!LptPort)
    {
    Log->Write("Can't start timer - port not opened.");
    return;
    }

        DWORD ThreadID;
        AllowThreadSendAlive = true;
        if(hThread == NULL)
    {
    WatchDogType_for_Thread = WatchDogType;
                hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadSendAlive, 0, NULL, &ThreadID);
    if(hThread != NULL)        Log->Write(">>> StartTimer...");
      else Log->Write("StartTimer error: can't cretae thread!");
    }
}
DWORD WINAPI        ThreadSendAlive()
{
        int count = 0;
                //TLogClass* LogThread = new TLogClass("WatchDog");

        while(AllowThreadSendAlive)
        {
                if(count == 150)
                {
                                                if(WatchDogType_for_Thread == WD_TYPE_ALNIKO)
                                                {
                                                                LptPort->SetPin(2, TRUE);
                                                                Sleep(300);
                                                                LptPort->SetPin(2, FALSE);
                                                }
                                        if(WatchDogType_for_Thread == WD_TYPE_NEW_GT)
                                                {
                                                                //LogThread->Write("GetPin №14: " + IntToStr(LptPort->GetPin(14)));
                                                        LptPort->SetPin(14, TRUE);
                                                                //LogThread->Write("GetPin №14: " + IntToStr(LptPort->GetPin(14)));
                                                                Sleep(300);
                                                                LptPort->SetPin(14, FALSE);
                                                                //LogThread->Write("GetPin №14: " + IntToStr(LptPort->GetPin(14)));
                                                }
                        count = 0;

                                                //LogThread->Write("> Ok");
                }
                count++;
                Sleep(100);
        };
                //delete LogThread;
        return 0;
}
void TWatchDog::StopTimer()
{
        if(hThread != NULL)
        {
                AllowThreadSendAlive = false;
                WaitForSingleObject(hThread, 200);
                CloseHandle(hThread);
                hThread = NULL;
        Log->Write("<<< StopTimer!");
        }
}
void TWatchDog::ClearGSM()
{
  if (!LptPort)
    {
    Log->Write("Can't start ClearGSM - port not opened.");
    return;
    }

    char TypeModem[100] = "";

    Log->WriteInLine(">>> ClearGSM...");

    ModemSiemensMC35i = NULL;

                                if(ModemPort > 0)
                                {
                                                ModemSiemensMC35i = new CModemSiemensMC35i(ModemPort);
                                                if(!ModemSiemensMC35i)  Log->Write("!!! Modem not found !!!");
                                }
                                else  Log->Write("!!! Modem COM port incorrect!!!");

                if(WatchDogType == WD_TYPE_NEW_GT)
    {
      LptPort->SetPin(1, 1);
      Sleep(500);
      LptPort->SetPin(1, 0);
    }
    if(WatchDogType == WD_TYPE_ALNIKO)
    {
        LptPort->SetPin(3, 1);
        Sleep(1000);
        if(ModemSiemensMC35i)
        {
                                                //if(!ModemSiemensMC35i->GetModemType(TypeModem)) Log->Write("Модем отключен.");
            //else Log->Write("Модем не отключен. Тип модема: " + (AnsiString)TypeModem);
        }
        Sleep(4000);
        LptPort->SetPin(3, 0);
    }
    Log->Write("<<< ClearGSM!");
    if(ModemSiemensMC35i)  delete ModemSiemensMC35i;
}
void TWatchDog::ResetPC()
{
        Log->Write("ResetPC...");
}
//==============================================================================
BOOL TWatchDog::ReadLPTPort(int PinNumber)
{
        BOOL b;
    switch (PinNumber)
    {
      case 1: b = LptPort->GetPin(1);        break;
      case 2: b = LptPort->GetPin(2);        break;
      case 3: b = LptPort->GetPin(3);        break;
      case 4: b = LptPort->GetPin(4);         break;
      case 5: b = LptPort->GetPin(5);        break;
      case 6: b = LptPort->GetPin(6);         break;
      case 7: b = LptPort->GetPin(7);        break;
      case 8: b = LptPort->GetPin(8);         break;
      case 9: b = LptPort->GetPin(9);        break;
      case 10: b = LptPort->GetPin(10);        break;
      case 11: b = LptPort->GetPin(11);        break;
      case 12: b = LptPort->GetPin(12);        break;
      case 13: b = LptPort->GetPin(13);        break;
      case 14: b = LptPort->GetPin(14); break;
      case 15: b = LptPort->GetPin(15);        break;
      case 16: b = LptPort->GetPin(16); break;
      case 17: b = LptPort->GetPin(17); break;
      default: ;
    }
        return b;
}
void TWatchDog::WriteLPTPort(int PinNumber, BOOL PinLevel)
{
    switch (PinNumber)
    {
      case 1: LptPort->SetPin(1, PinLevel);        break;
      case 2: LptPort->SetPin(2, PinLevel); break;
      case 3: LptPort->SetPin(3, PinLevel);        break;
      case 4: LptPort->SetPin(4, PinLevel); break;
      case 5: LptPort->SetPin(5, PinLevel);        break;
      case 6: LptPort->SetPin(6, PinLevel); break;
      case 7: LptPort->SetPin(7, PinLevel);        break;
      case 8: LptPort->SetPin(8, PinLevel); break;
      case 9: LptPort->SetPin(9, PinLevel);        break;
      case 14: LptPort->SetPin(14, PinLevel); break;
      case 16: LptPort->SetPin(16, PinLevel); break;
      case 17: LptPort->SetPin(17, PinLevel); break;
      default: ;
    }
}
