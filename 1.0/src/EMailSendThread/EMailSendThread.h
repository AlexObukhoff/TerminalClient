//---------------------------------------------------------------------------

#ifndef TEMailSendThreadH
#define TEMailSendThreadH
//---------------------------------------------------------------------------
#include "LogClass.h"
#include "TWConfig.h"
#include "XMLPacket.h"
#include <IdSMTP.hpp>
//---------------------------------------------------------------------------
class TEMailSendThread : public TThread
{
private:
protected:
    void __fastcall Execute();
    TLogClass* Log;
    TWConfig* Cfg;
    TEMailPacket *EMailPacket;
		bool InnerLog;
//    AnsiString FileName;
		TIdSMTP *SMTP;
    AnsiString IgnoredErrorsList;
public:
    __fastcall TEMailSendThread(TWConfig *_Cfg, TEMailPacket *_EMailPacket, TLogClass *_Log);
    __fastcall virtual ~TEMailSendThread(void);

    bool Sent;
    bool Finished;

    void SendMessage();

    bool Process();
};
//---------------------------------------------------------------------------
#endif
