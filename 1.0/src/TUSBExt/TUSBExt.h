//---------------------------------------------------------------------------
#ifndef TUSBExtH
#define TUSBExtH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <ExtCtrls.hpp>
#include <ftd2xx.h>
#include <ras.h>
#include <idicmpclient.hpp>
#include "..\LogClass\LogClass.h"
//---------------------------------------------------------------------------

class TBoolFilter
{
        bool DataCurrent;
        int Top;
        int Length;
public:
        bool Changed;
        TBoolFilter(int);
        void Add(bool);
        bool Get() {return DataCurrent;};
        void Clear_Changed() {Changed=false;};
        __property bool Data = {read=Get, write=Add};
};

//---------------------------------------------------------------------------

class TUSBExt
{
        UINT FilterLength;
        TTimer *UpdStatTimer;
        TIdIcmpClient *IdPing;
        FT_HANDLE USBExtHandle;
        TBoolFilter *_USB1;
        TBoolFilter *_USB2;
        TBoolFilter *_USB1OverCurrent;
        TBoolFilter *_USB2OverCurrent;
        TBoolFilter *_Key1;
        TTimer *_Key1Timer;
        TBoolFilter *_Key2;
        TTimer *_Key2Timer;
        TBoolFilter *_Key3;
        TTimer *_Key3Timer;
        bool _Relay;
        bool _Reset;
        TBoolFilter *_WDT;
        int SendData(char);
        TLogClass* log;
        bool InnerLog;
        int USB1Enable() {if (Enabled) return SendData(33);else return false;};
        int USB1Disable() {if (Enabled) return SendData(32);else return false;};
        int USB2Enable() {if (Enabled) return SendData(65);else return false;};
        int USB2Disable() {if (Enabled) return SendData(64);else return false;};
        int RelayEnable() {if (Enabled) return SendData(97);else return false;};
        int RelayDisable() {if (Enabled) return SendData(96);else return false;};
        int ResetEnable() {if (Enabled) return SendData(241);else return false;};
        int ResetDisable() {if (Enabled) return SendData(240);else return false;};
        int WDTEnable() {if (Enabled) return SendData(193);else return false;};
        int WDTDisable() {if (Enabled) return SendData(192);else return false;};
        void __fastcall UpdStatTimerTimer(TObject *Sender);
        void __fastcall PingTimerTimer(TObject *Sender);
        bool GetUSB1() {if (Enabled) return _USB1->Data;else return true;};
        void SetUSB1(bool);
        bool GetUSB2() {if (Enabled) return _USB2->Data;else return true;};
        void SetUSB2(bool);
        bool GetUSB1OverCurrent() {if (Enabled) return _USB1OverCurrent->Data;else return false;};
        bool GetUSB2OverCurrent() {if (Enabled) return _USB2OverCurrent->Data;else return false;};
        bool GetKey1() {if (Enabled) return _Key1->Data;else return false;};
        bool GetKey2() {if (Enabled) return _Key2->Data;else return false;};
        bool GetKey3() {if (Enabled) return _Key3->Data;else return false;};
        void SetRelay(bool _New) {if (Enabled) {_Relay=_New;if (_Relay) SendData(97); else SendData(96);}};
        void SetReset(bool _New) {if (Enabled) {_Reset=_New;if (_Reset) SendData(241); else SendData(240);}};
        bool GetWDT() {if (Enabled) return _WDT->Data;else return false;};
        void SetWDT(bool _New) {if (Enabled) {if (_New) SendData(193); else SendData(192);}};
public:
        TUSBExt(TLogClass*);
        ~TUSBExt();
        __property bool USB1 = {read=GetUSB1, write=SetUSB1};
        __property bool USB2 = {read=GetUSB2, write=SetUSB2};
        __property bool USB1OverCurrent = {read=GetUSB1OverCurrent};
        __property bool USB2OverCurrent = {read=GetUSB2OverCurrent};
        __property bool Key1 = {read=GetKey1};
        __property bool Key2 = {read=GetKey2};
        __property bool Key3 = {read=GetKey3};
        __property bool Relay = {write=SetRelay};
        __property bool Reset = {write=SetReset};
        __property bool WDT = {read=GetWDT, write=SetWDT};
        bool Enabled;
        int UpdateStatus();
        int WDTTicTac() {if (Enabled) return SendData(194);else return false;};
        void SetKey1Timer(TTimer*);
        void SetKey2Timer(TTimer*);
        void SetKey3Timer(TTimer*);
        void DropKey1Timer() { _Key1Timer=NULL;};
        void DropKey2Timer() { _Key2Timer=NULL;};
        void DropKey3Timer() { _Key3Timer=NULL;};
};
//---------------------------------------------------------------------------

#endif
