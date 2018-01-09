//---------------------------------------------------------------------------

#ifndef EMailSenderH
#define EMailSenderH
#include "LogClass.h"
#include "TWConfig.h"
#include "XMLPacket.h"
#include "TFileMap.h"


//---------------------------------------------------------------------------
class TEMailSender
{
    TLogClass *Log;
    bool InnerLog;
    TWConfig *Cfg;
		TEMailPacket *MailPacket;
    TFileMap *FileMap;
public:
    bool Enabled;
    void Clear();
		TEMailSender(AnsiString, TWConfig*, TLogClass*, TFileMap* _FileMap);
    ~TEMailSender();
    void StoreMessage(AnsiString Addresses, TDateTime _EventDateTime, AnsiString _Subject, AnsiString _MessageText, AnsiString FileName = "");
    void StoreHeartBeatMessage(TDateTime _EventDateTime, AnsiString _Subject, AnsiString _MessageText);
    bool Process();
    };

#endif
