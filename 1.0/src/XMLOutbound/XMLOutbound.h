#ifndef XMLOutboundH
#define XMLOutboundH

#include "TWConfig.h"
#include "LogClass.h"
#include "XMLPacket.h"
#include "TFileMap.h"
#include "EMailSender.h"
#include "PacketSender.h"
#include "TPayment.h"
#include "CommandReceiver.h"

#include <map>
#include <set>

enum _OutboundDirType {cnWork, cnTemp};

namespace fileAction
{
    enum fileAction
    {
        bad = 0,
        send = 1,
        not_send = 2
    };
}

typedef std::pair<std::string, int> StatElement;
typedef std::set<StatElement> StatElements;

//---------------------------------------------------------------------------

class TOutbound
{
protected:
    TFileMap *m_pFileMap;
    TWConfig* m_pCfg;
    TLogClass* m_pLog;
    TXMLInfo* m_infoFile;
    bool m_isInnerLog;
    bool NoTime;
    bool bFileProcessingStopped;
    AnsiString Dir;
    AnsiString DirTemp;
    AnsiString Name;
    AnsiString TruncateFileName(AnsiString);
    StatElements DevicePackets;
    fileAction::fileAction isNeedPacketSend(AnsiString FileName);

    void ProcessOutbound(_OutboundDirType);
    void RenameTempFiles();
    virtual bool ProcessFile(AnsiString);
    virtual void FilePostProcess(AnsiString);

public:
    TDateTime DTLastFileProcessed;
    bool busy;
    bool terminating;
    bool IndyError;
    int Command;
    int UnprocessedFilesCount;
    int FilesCount();
    int WorkDirFilesCount();
    int TempDirFilesCount();

    __fastcall TOutbound(TWConfig* cfg, TLogClass* log, TFileMap* fileMap, TXMLInfo* infoFile);
    virtual ~TOutbound();

    virtual void Process();
};

//---------------------------------------------------------------------------

class TPaymentsOutbound : public TOutbound
{
private:
    TLogClass* PaymentsLog;
    TPayment *Payment;

    bool ProcessFile(AnsiString);
    void FilePostProcess(AnsiString);
public:
    __fastcall TPaymentsOutbound(TWConfig *cfg, TLogClass *log, TFileMap *fileMap, TXMLInfo* infoFile);
    ~TPaymentsOutbound();
};

//---------------------------------------------------------------------------

class TStatisticsOutbound : public TOutbound
{
private:
    TPacketSender *PacketSender;

    bool ProcessFile(AnsiString);
public:
    __fastcall TStatisticsOutbound(TWConfig *cfg, TLogClass *log, TFileMap *fileMap, TXMLInfo* infoFile = NULL);
    ~TStatisticsOutbound();
};

//---------------------------------------------------------------------------

class TCommandsInbound : public TOutbound
{
private:
    TCommandReceiver *CommandReceiver;

    bool ProcessFile(AnsiString);
public:
    __fastcall TCommandsInbound(TWConfig *cfg, TLogClass *log, TFileMap *fileMap, TXMLInfo* infoFile = NULL);
    ~TCommandsInbound();

    void Process();
    void FilePostProcess(AnsiString);
};

//---------------------------------------------------------------------------

class TEMailOutbound : public TOutbound
{
private:
        TEMailSender *EMailSender;

        bool ProcessFile(AnsiString);

public:
        __fastcall TEMailOutbound(TWConfig *cfg, TLogClass *log, TFileMap *fileMap, TXMLInfo* infoFile = NULL);
        ~TEMailOutbound();
};

//---------------------------------------------------------------------------

#endif
