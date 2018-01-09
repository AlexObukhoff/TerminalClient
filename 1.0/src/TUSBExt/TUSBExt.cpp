//---------------------------------------------------------------------------

#pragma hdrstop

#include "TUSBExt.h"
#pragma package(smart_init)

TUSBExt::TUSBExt(TLogClass *Log)
{
try
{
InnerLog=false;

if (Log==NULL) {
        log = new TLogClass("TUSBExt");
        InnerLog=true;
        }
        else
        log=Log;

Enabled=false;
int InitResult=FT_OpenEx("CYBERPLAT USB EXTENDER",FT_OPEN_BY_DESCRIPTION,&USBExtHandle);
if (InitResult==FT_OK) {
        log->Write("USB Extender Started Successfully");
        FT_ResetDevice(USBExtHandle);
        FT_Purge(USBExtHandle, FT_PURGE_RX);
        FT_Purge(USBExtHandle, FT_PURGE_TX);
        FT_SetTimeouts(USBExtHandle,30,30);
        Enabled=true;
        WDTTicTac(); // WatchDog Timer Refull
//        Application->ProcessMessages();
//        Sleep(2000);
        UpdStatTimer = new TTimer(0);
        UpdStatTimer->Interval=100;
        UpdStatTimer->OnTimer=UpdStatTimerTimer;
        UpdStatTimer->Enabled=true;


        _Key1Timer=NULL;
        _Key2Timer=NULL;
        _Key3Timer=NULL;
        FilterLength=1;
        _Key1 = new TBoolFilter(FilterLength);
        _Key2 = new TBoolFilter(FilterLength);
        _Key3 = new TBoolFilter(FilterLength);
        _USB1 = new TBoolFilter(FilterLength);
        _USB2 = new TBoolFilter(FilterLength);
        _USB1OverCurrent = new TBoolFilter(FilterLength);
        _USB2OverCurrent = new TBoolFilter(FilterLength);
        _WDT = new TBoolFilter(FilterLength);

        }
        else
        {
        log->Write("USB Extender start error #"+AnsiString(InitResult));
        }
}
catch (Exception &exception)
{
Enabled=false;
log->Write("USB Extender start error due to exception: "+exception.Message);
}
}

//---------------------------------------------------------------------------

TUSBExt::~TUSBExt()
{
try
{
if (UpdStatTimer!=NULL)
        delete UpdStatTimer;
/*if (PingTimer!=NULL)
        delete PingTimer;
if (IdPing!=NULL)
        delete IdPing;*/
if (InnerLog)
        delete log;
if (_Key1!=NULL)
        delete _Key1;
if (_Key2!=NULL)
        delete _Key2;
if (_Key3!=NULL)
        delete _Key3;
if (_USB1!=NULL)
        delete _USB1;
if (_USB2!=NULL)
        delete _USB2;
if (_USB1OverCurrent!=NULL)
        delete _USB1OverCurrent;
if (_USB2OverCurrent!=NULL)
        delete _USB2OverCurrent;
if (_WDT!=NULL)
        delete _WDT;
/*if (Ping!=NULL)
        delete Ping;*/

log->Write("USB Extender Stopped Successfully.");
}
catch (Exception &exception)
{
log->Write("USB Extender Stopped with exception: "+exception.Message);
}
}

//---------------------------------------------------------------------------

/*void __fastcall TUSBExt::PingTimerTimer(TObject *Sender)
{
int i;
if (Ping->Status==cnDisabled) {
        log->Write("Enabling modem...");
        Ping->Status=cnEnabled;
        USB1=true;
        PingTimer->Enabled=false;
        PingTimer->Interval=Ping->EnableTime;
        PingTimer->Enabled=true;
        return;
        }
        else
        if (Ping->Status==cnEnabled) {
                log->Write("Modem Enabled.");
                Ping->Status=cnNormal;
                PingTimer->Enabled=false;
                PingTimer->Interval=Ping->Interval;
                PingTimer->Enabled=true;
                }
for (int i=0; i<5; i++) {
        IdPing->Host=Ping->Hosts[i];
        try
        {
        IdPing->Ping();
        if (IdPing->ReplyStatus.ReplyStatusType==0) {
                log->Write("Ping to "+Ping->Hosts[i]+" - Ok. "+IdPing->ReplyStatus.MsRoundTripTime+" ms.");
                return;
                }
                else
                {
                log->Write("Can' ping "+Ping->Hosts[i]+" - error ¹"+IdPing->ReplyStatus.ReplyStatusType);
                }
        }
        catch (Exception &exception)
        {
        log->Write("Can't ping "+Ping->Hosts[i]+" - an exception occured: "+exception.Message);
        }
        }
Ping->Status=cnDisabled;
log->Write("Disabling modem...");
USB1=false;
PingTimer->Enabled=false;
PingTimer->Interval=Ping->DisableTime;
PingTimer->Enabled=true;
} */

//---------------------------------------------------------------------------

int TUSBExt::UpdateStatus()
{
if ((Enabled)&&(USBExtHandle!=NULL)) {
        char FT_In_Buffer[255];
        unsigned long result=0;
        FT_Read(USBExtHandle, &FT_In_Buffer, sizeof(FT_In_Buffer), &result);
        _WDT->Data=FT_In_Buffer[0] & 0x1;
        _Key3->Data=FT_In_Buffer[0] & 0x2;
        _Key2->Data=FT_In_Buffer[0] & 0x4;
        _Key1->Data=FT_In_Buffer[0] & 0x8;
        _USB2->Data=FT_In_Buffer[0] & 0x10;
        _USB1->Data=FT_In_Buffer[0] & 0x20;
        _USB2OverCurrent->Data=FT_In_Buffer[0] & 0x40;
        _USB1OverCurrent->Data=FT_In_Buffer[0] & 0x80;
        return result;
        }
        else return -1;
}

//---------------------------------------------------------------------------

void __fastcall TUSBExt::UpdStatTimerTimer(TObject *Sender)
{
if (Enabled) {
        UpdateStatus();

        if ((_Key1->Changed)&&(_Key1->Data)&&(_Key1Timer!=NULL)) {
                log->Write("USB Extender: Key1 Pressed.");
                _Key1Timer->Enabled=true;
                _Key1->Clear_Changed();
                }

        if ((_Key2->Changed)&&(_Key2->Data)&&(_Key2Timer!=NULL)) {
                log->Write("USB Extender: Key2 Pressed.");
                _Key2Timer->Enabled=true;
                _Key2->Clear_Changed();
                }

        if ((_Key3->Changed)&&(_Key3->Data)&&(_Key3Timer!=NULL)) {
                log->Write("USB Extender: Key3 Pressed.");
                _Key3Timer->Enabled=true;
                _Key3->Clear_Changed();
                }

        if ((_USB1OverCurrent->Changed)&&(_USB1OverCurrent->Data)) {
                log->Write("USB Extender: USB1 OverCurrent.");
                _USB1OverCurrent->Clear_Changed();
                }

        if ((_USB2OverCurrent->Changed)&&(_USB2OverCurrent->Data)) {
                log->Write("USB Extender: USB2 OverCurrent.");
                _USB2OverCurrent->Clear_Changed();
                }

        if (_USB1->Changed) {
                if (USB1)
                        log->Write("USB Extender: USB1 Enabled.");
                        else
                        log->Write("USB Extender: USB1 Disabled.");
                _USB1->Clear_Changed();
                }

        if (_USB2->Changed) {
                if (USB2)
                        log->Write("USB Extender: USB2 Enabled.");
                        else
                        log->Write("USB Extender: USB2 Disabled.");
                _USB2->Clear_Changed();
                }

        if (_WDT->Changed) {
                if (WDT)
                        log->Write("USB Extender: WDT Enabled.");
                        else
                        log->Write("USB Extender: WDT Disabled.");
                _WDT->Clear_Changed();
                }
        }
}

//---------------------------------------------------------------------------

int TUSBExt::SendData(char Output)
{
if ((Enabled)&&(USBExtHandle!=NULL)) {
        char FT_Out_Buffer[256];
        unsigned long result=0;
        FT_Out_Buffer[0]=Output;
        FT_Write(USBExtHandle,&FT_Out_Buffer,1,&result);
        return result;
        }
        else return -1;
}

//---------------------------------------------------------------------------

void TUSBExt::SetUSB1(bool _NewUSB1)
{
if (Enabled)
        if (_NewUSB1) {
                log->Write("USB Extender: Trying to enable USB1...");
                SendData(33);
                }
                else {
                log->Write("USB Extender: Trying to disable USB1...");
                SendData(32);
                }
};

//---------------------------------------------------------------------------

void TUSBExt::SetUSB2(bool _NewUSB2)
{
if (Enabled)
        if (_NewUSB2) {
                log->Write("USB Extender: Trying to enable USB2...");
                SendData(65);
                }
                else {
                log->Write("USB Extender: Trying to disable USB2...");
                SendData(64);
                }
};

//---------------------------------------------------------------------------

void TUSBExt::SetKey1Timer(TTimer* _TimerHandle)
{
if (Enabled) {
        try
        {
        if (_TimerHandle!=NULL) {
                _Key1Timer=_TimerHandle;
                _TimerHandle->Interval=50;
                _TimerHandle->Enabled=false;
                }
        }
        catch (Exception &exception)
        {
        log->Write("Can't set timer for Key1 due to exception: "+exception.Message);
        }
        }
}

//---------------------------------------------------------------------------

void TUSBExt::SetKey2Timer(TTimer* _TimerHandle)
{
if (Enabled) {
        try
        {
        if (_TimerHandle!=NULL) {
                _Key2Timer=_TimerHandle;
                _TimerHandle->Interval=50;
                _TimerHandle->Enabled=false;
                }
        }
        catch (Exception &exception)
        {
        log->Write("Can't set timer for Key2 due to exception: "+exception.Message);
        }
        }
}

//---------------------------------------------------------------------------

void TUSBExt::SetKey3Timer(TTimer* _TimerHandle)
{
if (Enabled) {
        try
        {
        if (_TimerHandle!=NULL) {
                _Key3Timer=_TimerHandle;
                _TimerHandle->Interval=50;
                _TimerHandle->Enabled=false;
                }
        }
        catch (Exception &exception)
        {
        log->Write("Can't set timer for Key3 due to exception: "+exception.Message);
        }
        }
}

//---------------------------------------------------------------------------

TBoolFilter::TBoolFilter(int _Length = 3)
{
Length=_Length;
Top=0;
DataCurrent=false;
Changed=false;
}

//---------------------------------------------------------------------------

void TBoolFilter::Add(bool DataNew)
{
if (DataNew==DataCurrent)
        Top=0;
        else {
        Top++;
        if (Top>=Length) {
                DataCurrent=DataNew;
                Top=0;
                Changed=true;
                }
        }
}

//---------------------------------------------------------------------------
