//---------------------------------------------------------------------------

#ifndef TSendThread2H
#define TSendThread2H
//---------------------------------------------------------------------------
#include <vector>
#include "LogClass.h"
#include "TWConfig.h"
#include "TFileMap.h"

#include "XMLPacket.h"
#include "CommandReceiver.h"

const std::size_t MaxSendBufferSize = 1024;

class TSendThread2 : public TThread
{
private:
    char* xcode(const char* src, int srcCP, int dstCP);
    
    std::vector<std::string> m_serverAnswer;
    TWConfig* m_pCfg;
    TLogClass* m_pLog;
    TStatPacket* m_pPacket;
    TFileMap* m_pFileMap;
    BYTE m_sendBuffer[MaxSendBufferSize];
    std::size_t m_sendBufferSize;
    bool m_isInnerLog;
    TIdTCPClient *m_pTCPClient;

    void __fastcall Execute();
    bool handshake();
    void processHeartBeatAnswer(int cmd);
    void processOtherAnswer(int cmd);
    int receiveCommand(int cmd, int cmdUID);
    bool testArchive(AnsiString fileName);
    void shutdown(bool reboot = false);

    bool prepareSend();
    void prepareSendHeader();
    void preparePaymentInit();
    void prepareIncassation();
    void prepareHeartBeat();
    void prepareError();
    void preparePaymentStatusChange();
    void preparePaymentComplete();
    void prepareTestConnection();
    void prepareCommandProcess();
    bool prepareFileSend();

    void cmdReboot(TCommandReceiver* cmdRecv, int cmdUID);
    void cmdReceiveFileOld(TCommandReceiver* cmdRecv, int cmdUID);
    void cmdSendConfigOld(TCommandReceiver* cmdRecv, int cmdUID);
    void cmdCancelPayment(TCommandReceiver* cmdRecv, int cmdUID);
    void cmdSendFile(TCommandReceiver* cmdRecv, int cmdUID);
    void cmdReceiveFile(TCommandReceiver* cmdRecv, int cmdUID);
    void cmdResurrectPayment(TCommandReceiver* cmdRecv, int cmdUID);
    void cmdShutdown(TCommandReceiver* cmdRecv, int cmdUID);
    void cmdBlock(TCommandReceiver* cmdRecv, int cmdUID);
    void cmdUnblock(TCommandReceiver* cmdRecv, int cmdUID);
    void cmdGetKeys(TCommandReceiver* cmdRecv, int cmdUID);
    
    template<class T>
    void addToSendBuffer(T value);
    void addToSendBuffer(AnsiString value, bool isZeroTerminated = true, std::size_t size = 0);

public:
    bool Finished;
    bool ConnectOK;
    bool HeartBeatOK;
    bool Sent;
    AnsiString TestConnectionResult;

    __fastcall TSendThread2(TWConfig* cfg, TStatPacket *packet, TLogClass* log, TFileMap *fileMap);
    __fastcall virtual ~TSendThread2();
};

#endif
