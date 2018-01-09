//---------------------------------------------------------------------------

#ifndef TSendThreadH
#define TSendThreadH
//---------------------------------------------------------------------------
#include "LogClass.h"
#include "TWConfig.h"
#include "XMLPacket.h"
//#include "PacketSender.h"
#include "CommandReceiver.h"

#include "TFileMap.h"
//---------------------------------------------------------------------------
#define StatBufferSize 10240

class TSendThread : public TThread
{
private:
    void Reboot();
    void signBuffer(void* bufer, std::size_t& size); 

protected:
    TFileMap *FileMap;
    BYTE Buffer[StatBufferSize];
    int BufferLength;
    TFileStream *InFile;
    TCommandReceiver *CommandReceiver;
    TFileStream *FileToSend;

    void __fastcall Execute();
    void ShutDown();
    void ForceBlock();
    TLogClass* Log;
    TWConfig* Cfg;
    TStatPacket *StatPacket;
    bool InnerLog;
    TIdTCPClient *IdTCPC;
    int TerminalID;
    AnsiString Host;
    int Port;

    AnsiString FileName;
    AnsiString SendFileName;
    int Command;
    int CommandUID;

    void SendHeartBeat();
    void SendTestConnection();
    void SendPaymentInit();
    void SendIncassation();
    void SendError();
    void SendPaymentStatusChange();
    void SendPaymentComplete();
    void SendCommandProcessed();
    bool SendFileSend();

    int ReceiveCommand(int Command, int CommandUID);

    bool Process();

    void Write(short);
    void Write(int);
    void Write(double);
    void Write(AnsiString);
    void WriteSession(AnsiString);
    void Write(AnsiString, int);
    
    void WriteFile(AnsiString);
    
    void WriteBuffer(void*, int);
    
    void ClearBuffer();
    void AddToBuffer(int);
    void AddToBuffer(short);
    void AddToBuffer(__int8);
    void AddToBuffer(double);
    void AddToBuffer(AnsiString, bool AddZero = true, int Size = 0);
    void WriteBuffer();
    bool TestArchiveFile(AnsiString SourceFileName);
    void __fastcall Init(int _TerminalID, AnsiString _Host, int _Port, TStatPacket *_StatPacket, TLogClass *_Log);

public:
    bool Finished;
    bool ConnectOK;
    bool HeartBeatOK;
    bool Sent;
    AnsiString TestConnectionResult;

    __fastcall TSendThread(TWConfig *_Cfg, TStatPacket *_StatPacket, TLogClass *_Log, TFileMap *FileMap);
    __fastcall virtual ~TSendThread(void);

};
//---------------------------------------------------------------------------
#endif
