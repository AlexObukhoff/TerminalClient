//---------------------------------------------------------------------------

#ifndef TFileMapH
#define TFileMapH

#include "LogClass.h"
#include <boost\format.hpp>

#pragma pack(push)
#pragma pack(1)

const short cnTerminalErrorMode = 1;
const short cnTerminalForceBlock = 2;
const short cnTerminalWriteError = 4;
const short cnTerminalInternalBlock = 8;
const short cnValidatorError = 16;
const short cnPrinterError = 32;
const short cnCardReaderError = 64;
const short cnCoinAcceptorError = 128;

struct TSFileMap
{
    UINT WCHandle;
    char Session[20];
    TDateTime WCTimeMark;
    TDateTime ConnTimeMark;
    TDateTime NumDBLastUpdatedDT;
    TDateTime LastConnectionDT;//Время последнего удачного HeartBeat
    TDateTime LastSuccessfullConnectionDT;
    TDateTime LastCPConnectionDT;
    TDateTime LastSuccessfullCPConnectionDT;//Время последнего удачного платежа
    UINT Command;
    bool Idle;
    bool CheckStatDir;
    short WCState;                                //
    short DebugState;
    TDateTime LastPaymentReceived;                //
    TDateTime LastPaymentProcessed;               //
    int BillsCount;                               //
    int BillsSum;                                 //
    char ValidatorOutState[255];                  // *
    char CoinAcceptorOutState[255];               // *
    char PrinterOutState[255];                    // *
    int UnprocPaymentsCount;
    int UnprocStatPacketsCount;
    bool CheckPaymentDir;
    int SIMBalance;                               //
    int GSMSignalQuality;                         //
    int ValidatorState;
    int CoinAcceptorState;
    int PrinterState;
    int SW0;
    int SW1;
    int SW2;
    int GSMOperatorID;
    int CID;
    char CIDFileName[255];
    bool NumDBUpdateDone;
    bool WriteErrorFound;
    bool FirstConnected;
    TDateTime LastStartNavigateDT;
    TDateTime LastEndNavigateDT;
    bool ForceBlock;
    bool MonSrvrConnectOK;
    bool WCRestartReq;
    bool GetKeysRQ;
    bool m_incrementMoneyTransferToken;
    long m_MoneyTransferToken;
    long m_ChequeCounter;
};// датчики, код оператора

#pragma pack(pop)

class TFileMap
{
    TLogClass* Log;
    bool InnerLog;
    //void ShowError(AnsiString);
    /*    int ReadInt(int);
    //    double ReadDouble(int);
    long ReadLong(int);
    void WriteInt(int,int);
    void WriteLong(long,int);*/
    //    void WriteDouble(double, int);
    TSFileMap *Map;
    TCriticalSection *CS;
    bool Connected;
    LPVOID lpFileMap;
    HANDLE hFileMapping;
    bool ReadWCIdle();
    void WriteWCIdle(bool);

    TDateTime ReadWCTimeMark(){if (Map) return Map->WCTimeMark; else return 0;};
    void WriteWCTimeMark(TDateTime _Src) {if (Map) Map->WCTimeMark=_Src;};
    TDateTime ReadConnTimeMark(){if (Map) return Map->ConnTimeMark; else return 0;};
    void WriteConnTimeMark(TDateTime _Src) {if (Map) Map->ConnTimeMark=_Src;};

    TDateTime ReadLastConnectionDT() {TDateTime Res=0;if(CS) CS->Acquire();try{if (Map) Res = Map->LastConnectionDT;}__finally{if(CS) CS->Release();return Res;}};
    void WriteLastConnectionDT(TDateTime _Src) {if(CS) CS->Acquire();try{if (Map) Map->LastConnectionDT=_Src;}__finally{if(CS) CS->Release();}};

    TDateTime ReadLastSuccessfullConnectionDT() {TDateTime Res=0;if(CS) CS->Acquire();try{if (Map) Res = Map->LastSuccessfullConnectionDT;}__finally{if(CS) CS->Release();return Res;}};
    void WriteLastSuccessfullConnectionDT(TDateTime _Src) {if(CS) CS->Acquire();try{if (Map) Map->LastSuccessfullConnectionDT=_Src;}__finally{if(CS) CS->Release();}};

    TDateTime ReadLastCPConnectionDT() {TDateTime Res=0;if(CS) CS->Acquire();try{if (Map) Res = Map->LastCPConnectionDT;}__finally{if(CS) CS->Release();return Res;}};
    void WriteLastCPConnectionDT(TDateTime _Src) {if(CS) CS->Acquire();try{if (Map) Map->LastCPConnectionDT=_Src;}__finally{if(CS) CS->Release();}};

    TDateTime ReadLastSuccessfullCPConnectionDT() {TDateTime Res=0;if(CS) CS->Acquire();try{if (Map) Res = Map->LastSuccessfullCPConnectionDT;}__finally{if(CS) CS->Release();return Res;}};
    void WriteLastSuccessfullCPConnectionDT(TDateTime _Src) {if(CS) CS->Acquire();try{if (Map) Map->LastSuccessfullCPConnectionDT=_Src;}__finally{if(CS) CS->Release();}};

    /*TDateTime ReadLastCPConnectionDT(){if (Map) return Map->LastCPConnectionDT; else return 0;};
    void WriteLastCPConnectionDT(TDateTime _Src) {if (Map) Map->LastCPConnectionDT=_Src;};
    TDateTime ReadLastSuccessfullCPConnectionDT(){if (Map) return Map->LastSuccessfullCPConnectionDT; else return 0;};
    void WriteLastSuccessfullCPConnectionDT(TDateTime _Src) {if (Map) Map->LastSuccessfullCPConnectionDT=_Src;};*/

    /*TDateTime ReadConnSendHeartBeatThreadTimeMark(){if (Map) return Map->ConnSendHeartBeatThreadTimeMark; else return 0;};
    void WriteConnSendHeartBeatThreadTimeMark(TDateTime _Src) {if (Map) Map->ConnSendHeartBeatThreadTimeMark=_Src;};
    TDateTime ReadWCGetCardsInfoThreadTimeMark(){if (Map) return Map->WCGetCardsInfoThreadTimeMark; else return 0;};
    void WriteWCGetCardsInfoThreadTimeMark(TDateTime _Src) {if (Map) Map->WCGetCardsInfoThreadTimeMark=_Src;};*/

    TDateTime ReadLastPaymentReceived(){if (Map) return Map->LastPaymentReceived; else return 0;};
    void WriteLastPaymentReceived(TDateTime _Src) {if (Map) Map->LastPaymentReceived=_Src;};
    TDateTime ReadLastPaymentProcessed(){if (Map) return Map->LastPaymentProcessed; else return 0;};
    void WriteLastPaymentProcessed(TDateTime _Src) {if (Map) Map->LastPaymentProcessed=_Src;};

    int ReadBillsCount(){if (Map) return Map->BillsCount; else return 0;};
    void WriteBillsCount(int _Src) {if (Map) Map->BillsCount=_Src;};
    int ReadBillsSum(){if (Map) return Map->BillsSum; else return 0;};
    void WriteBillsSum(int _Src) {if (Map) Map->BillsSum=_Src;};

    int ReadSW0(){if (Map) return Map->SW0; else return 0;};
    void WriteSW0(int _Src) {if (Map) Map->SW0=_Src;};
    int ReadSW1(){if (Map) return Map->SW1; else return 0;};
    void WriteSW1(int _Src) {if (Map) Map->SW1=_Src;};
    int ReadSW2(){if (Map) return Map->SW2; else return 0;};
    void WriteSW2(int _Src) {if (Map) Map->SW2=_Src;};

    int ReadSIMBalance(){if (Map) return Map->SIMBalance; else return 0;};
    void WriteSIMBalance(int _Src) {if (Map) Map->SIMBalance=_Src;};
    int ReadGSMSignalQuality(){if (Map) return Map->GSMSignalQuality; else return 0;};
    void WriteGSMSignalQuality(int _Src) {if (Map) Map->GSMSignalQuality=_Src;};
    int ReadGSMOperatorID(){if (Map) return Map->GSMOperatorID; else return 0;};
    void WriteGSMOperatorID(int _Src) {if (Map) Map->GSMOperatorID=_Src;};

    AnsiString ReadValidatorOutState(){if (Map) return AnsiString(Map->ValidatorOutState); else return 0;};
    void WriteValidatorOutState(AnsiString _Src);
    AnsiString ReadPrinterOutState(){if (Map) return AnsiString(Map->PrinterOutState); else return 0;};
    void WritePrinterOutState(AnsiString _Src);
    AnsiString ReadCoinAcceptorOutState(){if (Map) return AnsiString(Map->CoinAcceptorOutState); else return 0;};
    void WriteCoinAcceptorOutState(AnsiString _Src);

    int ReadValidatorState(){if (Map) return Map->ValidatorState; else return 0;};
    void WriteValidatorState(int _Src) {if (Map) Map->ValidatorState=_Src;};
    int ReadPrinterState(){if (Map) return Map->PrinterState; else return 0;};
    void WritePrinterState(int _Src) {if (Map) Map->PrinterState=_Src;};
    int ReadCoinAcceptorState(){if (Map) return Map->CoinAcceptorState; else return 0;};
    void WriteCoinAcceptorState(int _Src) {if (Map) Map->CoinAcceptorState=_Src;};

    int ReadUnprocPaymentsCount(){if (Map) return Map->UnprocPaymentsCount; else return 0;};
    void WriteUnprocPaymentsCount(int _Src) {if (Map) Map->UnprocPaymentsCount=_Src;};
    int ReadUnprocStatPacketsCount(){if (Map) return Map->UnprocStatPacketsCount; else return 0;};
    void WriteUnprocStatPacketsCount(int _Src) {if (Map) Map->UnprocStatPacketsCount=_Src;};

    int ReadCID(){if (Map) return Map->CID; else return 0;};
    void WriteCID(int _Src) {if (Map) Map->CID=_Src;};
    AnsiString ReadCIDFileName(){if (Map) return AnsiString(Map->CIDFileName); else return 0;};
    void WriteCIDFileName(AnsiString _Src);

    bool ReadCheckStatDir(){if (Map) return Map->CheckStatDir; else return 0;};
    void WriteCheckStatDir(bool _Src) {if (Map) Map->CheckStatDir=_Src;};

    bool ReadCheckPaymentDir(){if (Map) return Map->CheckPaymentDir; else return 0;};
    void WriteCheckPaymentDir(bool _Src) {if (Map) Map->CheckPaymentDir=_Src;};

    short ReadWCState();
    void WriteWCState(short);

    short ReadDebugState();
    void WriteDebugState(short);

    bool ReadNumDBUpdateDone(){if (Map) return Map->NumDBUpdateDone; else return 0;};
    void WriteNumDBUpdateDone(bool _Src) {if (Map) Map->NumDBUpdateDone=_Src;};
    TDateTime ReadNumDBLastUpdatedDT(){if (Map) return Map->NumDBLastUpdatedDT; else return 0;};
    void WriteNumDBLastUpdatedDT(TDateTime _Src) {if (Map) Map->NumDBLastUpdatedDT=_Src;};

    bool ReadWriteErrorFound(){if (Map) return Map->WriteErrorFound; else return 0;};
    void WriteWriteErrorFound(bool _Src) {if (Map) Map->WriteErrorFound=_Src;};
    bool ReadFirstConnected(){if (Map) return Map->FirstConnected; else return 0;};
    void WriteFirstConnected(bool _Src) {if (Map) Map->FirstConnected=_Src;};

    TDateTime ReadLastStartNavigateDT() {TDateTime Res=0;if(CS) CS->Acquire();try{if (Map) Res = Map->LastStartNavigateDT;}__finally{if(CS) CS->Release();return Res;}};
    void WriteLastStartNavigateDT(TDateTime _Src) {if(CS) CS->Acquire();try{if (Map) Map->LastStartNavigateDT=_Src;}__finally{if(CS) CS->Release();}};

    TDateTime ReadLastEndNavigateDT() {TDateTime Res=0;if(CS) CS->Acquire();try{if (Map) Res = Map->LastEndNavigateDT;}__finally{if(CS) CS->Release();return Res;}};
    void WriteLastEndNavigateDT(TDateTime _Src) {if(CS) CS->Acquire();try{if (Map) Map->LastEndNavigateDT=_Src;}__finally{if(CS) CS->Release();}};

    bool ReadForceBlock(){if (Map) return Map->ForceBlock; else return 0;};
    void WriteForceBlock(bool _Src) {if (Map) Map->ForceBlock=_Src;};

    bool ReadMonSrvrConnectOK(){if (Map) return Map->MonSrvrConnectOK; else return 0;};
    void WriteMonSrvrConnectOK(bool _Src) {if (Map) Map->MonSrvrConnectOK=_Src;};

    bool ReadWCRestartReq(){if (Map) return Map->WCRestartReq; else return 0;};
    void WriteWCRestartReq(bool _Src) {if (Map) Map->WCRestartReq=_Src;};
    bool ReadGetKeysRQ(){if (Map) return Map->GetKeysRQ; else return 0;};
    void WriteGetKeysRQ(bool _Src) {if (Map) Map->GetKeysRQ=_Src;};

    bool getIncrementMoneyTransferToken()
    {
                    #ifdef __CONN__
                        Log->Write((boost::format("CONN, read token flag: %1%") % Map->m_incrementMoneyTransferToken).str().c_str());
                    #else
                        Log->Write((boost::format("WC, read token flag: %1%") % Map->m_incrementMoneyTransferToken).str().c_str());
                    #endif
    if (Map) return Map->m_incrementMoneyTransferToken;
    else Log->Write("FileMap not exist");
    return false;
    };
    void setIncrementMoneyTransferToken(bool src){
                    #ifdef __CONN__
                        Log->Write((boost::format("CONN, set token flag: %1% to %2%") % Map->m_incrementMoneyTransferToken % src).str().c_str());
                    #else
                        Log->Write((boost::format("WC, set token flag: %1% to %2%") % Map->m_incrementMoneyTransferToken % src).str().c_str());
                    #endif
        if (Map) Map->m_incrementMoneyTransferToken = src;
        else Log->Write("FileMap not exist");
    };

    long getMoneyTransferToken()
    {
                    #ifdef __CONN__
                        Log->Write((boost::format("CONN, read token: %1%") % Map->m_MoneyTransferToken).str().c_str());
                    #else
                        Log->Write((boost::format("WC, read token: %1%") % Map->m_MoneyTransferToken).str().c_str());
                    #endif
    if (Map) return Map->m_MoneyTransferToken;
    else Log->Write("FileMap not exist");
    return false;
    };
    void setMoneyTransferToken(long src){
                    #ifdef __CONN__
                        Log->Write((boost::format("CONN, set token: %1% to %2%") % Map->m_MoneyTransferToken % src).str().c_str());
                    #else
                        Log->Write((boost::format("WC, set token: %1% to %2%") % Map->m_MoneyTransferToken % src).str().c_str());
                    #endif
        if (Map && src > Map->m_MoneyTransferToken ) Map->m_MoneyTransferToken = src;
        else Log->Write("FileMap not exist");
    };

    long getChequeCounter()
    {
        if(Map)
            return Map->m_ChequeCounter;
        else
            return -1;
    };
    void setChequeCounter(long src){if(Map) Map->m_ChequeCounter = src;};

public:
    TFileMap(TLogClass*);
    ~TFileMap();
    void Create(AnsiString);
    void Open(AnsiString);
    void Clear();
    void Close();
    HANDLE ReadWCHandle();
    void WriteWCHandle(int);
    //ULONG ReadConnTimeMark();
    void PutWCTimeMark() {WCTimeMark=GetCurrentTime();};
    void PutConnTimeMark() {/*Log->Write("PutConnTimeMark()");*/ ConnTimeMark=GetCurrentTime();};

    void PutLastConnectionDT() {LastConnectionDT=GetCurrentTime();};
    void PutLastSuccessfullConnectionDT() {LastSuccessfullConnectionDT=GetCurrentTime();};
    void PutLastCPConnectionDT() {LastCPConnectionDT=GetCurrentTime();};
    void PutLastSuccessfullCPConnectionDT() {LastSuccessfullCPConnectionDT=GetCurrentTime();};

    void PutLastStartNavigate() {LastStartNavigate=GetCurrentTime();};
    void PutLastEndNavigate() {LastEndNavigate=GetCurrentTime();};
    /*void PutConnSendHeartBeatThreadTimeMark() {ConnSendHeartBeatThreadTimeMark=GetCurrentTime();};
    void PutWCGetCardsInfoThreadTimeMark() {WCGetCardsInfoThreadTimeMark=GetCurrentTime();};*/
    /*void PutConnPaymentsThreadTimeMark() {Log->Write("PutConnPaymentsThreadTimeMark()"); ConnPaymentsThreadTimeMark=GetCurrentTime();};
    void PutConnStatThreadTimeMark() {Log->Write("PutConnStatThreadTimeMark()"); ConnStatThreadTimeMark=GetCurrentTime();};
    void PutConnEMailThreadTimeMark() {Log->Write("PutConnEMailThreadTimeMark()"); ConnEMailThreadTimeMark=GetCurrentTime();};
    void PutConnCommandThreadTimeMark() {Log->Write("PutConnCommandThreadTimeMark()"); ConnCommandThreadTimeMark=GetCurrentTime();};
    void PutConnSendHeartBeatThreadTimeMark() {Log->Write("PutConnSendHeartBeatThreadTimeMark()"); ConnSendHeartBeatThreadTimeMark=GetCurrentTime();};
    void PutWCGetCardsInfoThreadTimeMark() {Log->Write("PutWCGetCardsInfoThreadTimeMark()"); WCGetCardsInfoThreadTimeMark=GetCurrentTime();};*/
    //ULONG ReadWCTimeMark();
    int ReadCommand();
    void WriteCommand(int);

    __property bool WCIdle = { read=ReadWCIdle, write=WriteWCIdle };
    __property short WCState = { read=ReadWCState, write=WriteWCState };

    void SetWCState(short Bit);
    void ClearWCState(short Bit);
    bool CheckWCState(short Bit);

    __property short DebugState = { read=ReadDebugState, write=WriteDebugState };
    void SetDebugState(short Bit);
    void ClearDebugState(short Bit);
    bool CheckDebugState(short Bit);

    __property TDateTime WCTimeMark = { read=ReadWCTimeMark, write=WriteWCTimeMark };
    __property TDateTime ConnTimeMark = { read=ReadConnTimeMark, write=WriteConnTimeMark };

    __property TDateTime LastConnectionDT = { read=ReadLastConnectionDT, write=WriteLastConnectionDT };
    __property TDateTime LastSuccessfullConnectionDT = { read=ReadLastSuccessfullConnectionDT, write=WriteLastSuccessfullConnectionDT };
    __property TDateTime LastCPConnectionDT = { read=ReadLastCPConnectionDT, write=WriteLastCPConnectionDT };
    __property TDateTime LastSuccessfullCPConnectionDT = { read=ReadLastSuccessfullCPConnectionDT, write=WriteLastSuccessfullCPConnectionDT };

    /*__property TDateTime ConnSendHeartBeatThreadTimeMark = { read=ReadConnSendHeartBeatThreadTimeMark, write=WriteConnSendHeartBeatThreadTimeMark };
    __property TDateTime WCGetCardsInfoThreadTimeMark = { read=ReadWCGetCardsInfoThreadTimeMark, write=WriteWCGetCardsInfoThreadTimeMark };*/

    __property TDateTime LastPaymentReceived = { read=ReadLastPaymentReceived, write=WriteLastPaymentReceived };
    __property TDateTime LastPaymentProcessed = { read=ReadLastPaymentProcessed, write=WriteLastPaymentProcessed };

    __property int BillsCount = { read=ReadBillsCount, write=WriteBillsCount };
    __property int BillsSum = { read=ReadBillsSum, write=WriteBillsSum };

    __property int SW0 = { read=ReadSW0, write=WriteSW0 };
    __property int SW1 = { read=ReadSW1, write=WriteSW1 };
    __property int SW2 = { read=ReadSW2, write=WriteSW2 };

    __property int SIMBalance = { read=ReadSIMBalance, write=WriteSIMBalance };
    __property int GSMSignalQuality = { read=ReadGSMSignalQuality, write=WriteGSMSignalQuality };
    __property int GSMOperatorID = { read=ReadGSMOperatorID, write=WriteGSMOperatorID };

    __property AnsiString    ValidatorOutState = { read  = ReadValidatorOutState, write  = WriteValidatorOutState };
    __property AnsiString      PrinterOutState = { read  =   ReadPrinterOutState, write  =   WritePrinterOutState };
    __property AnsiString CoinAcceptorOutState = { read=ReadCoinAcceptorOutState, write=WriteCoinAcceptorOutState };
    __property int    ValidatorState = { read  = ReadValidatorState, write  = WriteValidatorState };
    __property int      PrinterState = { read   =  ReadPrinterState, write  =   WritePrinterState };
    __property int CoinAcceptorState = { read=ReadCoinAcceptorState, write=WriteCoinAcceptorState };

    __property int UnprocPaymentsCount = { read=ReadUnprocPaymentsCount, write=WriteUnprocPaymentsCount };
    __property int UnprocStatPacketsCount = { read=ReadUnprocStatPacketsCount, write=WriteUnprocStatPacketsCount };

    __property bool CheckStatDir = { read=ReadCheckStatDir, write=WriteCheckStatDir};
    __property bool CheckPaymentDir = { read=ReadCheckPaymentDir, write=WriteCheckPaymentDir};

    __property int CID = { read=ReadCID, write=WriteCID};
    __property AnsiString CIDFileName = { read=ReadCIDFileName, write=WriteCIDFileName};

    __property bool NumDBUpdateDone = { read=ReadNumDBUpdateDone, write=WriteNumDBUpdateDone};
    __property TDateTime NumDBLastUpdatedDT = { read=ReadNumDBLastUpdatedDT, write=WriteNumDBLastUpdatedDT };

    __property bool WriteErrorFound = { read=ReadWriteErrorFound, write=WriteWriteErrorFound};
    __property bool FirstConnected = { read=ReadFirstConnected, write=WriteFirstConnected};

    __property TDateTime LastStartNavigate = { read=ReadLastStartNavigateDT, write=WriteLastStartNavigateDT };
    __property TDateTime LastEndNavigate = { read=ReadLastEndNavigateDT, write=WriteLastEndNavigateDT };

    __property bool ForceBlock = { read=ReadForceBlock, write=WriteForceBlock};
    __property bool MonSrvrConnectOK = { read=ReadMonSrvrConnectOK, write=WriteMonSrvrConnectOK};
    __property bool WCRestartReq = { read=ReadWCRestartReq, write=WriteWCRestartReq};
    __property bool GetKeysRQ = { read=ReadGetKeysRQ, write=WriteGetKeysRQ};
    __property bool incrementMoneyTransferToken = { read=getIncrementMoneyTransferToken, write=setIncrementMoneyTransferToken};
    __property long moneyTransferToken = { read=getMoneyTransferToken, write=setMoneyTransferToken};

    __property long ChequeCounter = {read=getChequeCounter, write=setChequeCounter};

    TDateTime GetCurrentTime();
};
//---------------------------------------------------------------------------
#endif
