//---------------------------------------------------------------------------
#ifndef TPacketSenderH
#define TPacketSenderH
#include "LogClass.h"
#include "TWConfig.h"
#include "XMLPacket.h"
#include "TFileMap.h"

//---------------------------------------------------------------------------

class TPacketSender
{
protected:
    TFileMap *FileMap;
    TLogClass *Log;
    TWConfig *Cfg;
    TStatPacket *StatPacket;
    bool InnerLog;
//    TIdTCPClient *IdTCPC;
    int TerminalID;
    AnsiString Host;
    int Port;
    AnsiString TestResult;
    bool Enabled;
    bool PacketLoadError;
    bool ListFile(AnsiString SourceFileName);
    void __fastcall SZListFileEvent(System::TObject* Sender, WideString Filename, unsigned Fileindex, unsigned FileSizeU, unsigned FileSizeP, unsigned Fileattr, unsigned Filecrc, WideString Filemethod, double FileTime);
public:
    TPacketSender(AnsiString, TWConfig*, TLogClass*, TFileMap*);
    virtual ~TPacketSender();
    void Clear();
    virtual void StorePaymentInitTemp(TDateTime EventDateTime, int OperatorId, AnsiString InitialSessionNum, float Comission, const std::vector<TNote>& Notes, AnsiString Fields);
    virtual void StorePaymentInitComplete();
    virtual void StoreIncassation(TDateTime EventDateTime, const TNotesVector& _Notes, double _Comission, AnsiString _IncassationNumber);
    virtual void StoreError(TDateTime EventDateTime, AnsiString Sender, int Type, AnsiString Description, int SubType, AnsiString SubDescription);
    virtual void StorePaymentStatusChange(TDateTime EventDateTime, AnsiString InitialSessionNum, int _Status, int _ErrorCode);
    //virtual void StorePaymentComplete(TDateTime EventDateTime, AnsiString InitialSessionNum, AnsiString LastSessionNum, int ErrorCode);
    virtual void StorePaymentComplete(TDateTime _EventDateTime, AnsiString _InitialSessionNum, AnsiString _LastSessionNum, int _ErrorCode, int _LastErrorCode, TDateTime _FirstTryDT, int _OperatorId, double _Sum, double _Comission, AnsiString _Fields);
    virtual void StoreSendCommandProcessedTemp(int _CommandUID);
    virtual void StoreCommandProcessed(int _CommandUID);
    virtual void StoreFileSend(AnsiString FileName);
    virtual void StoreFilesSend(AnsiString, AnsiString);

    virtual bool SendHeartBeat(short, short, int, int, int, int, bool, int);
    virtual AnsiString TestConnection(AnsiString URL="");
    virtual bool Process(bool bProcessMessages = false);
    bool PutFile(AnsiString, AnsiString);
    bool Terminated;
    bool CreatePacket(AnsiString PacketName = "");
    bool CreateTempPacket();
};
#endif
