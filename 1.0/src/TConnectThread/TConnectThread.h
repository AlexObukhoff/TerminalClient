//---------------------------------------------------------------------------

#ifndef TConnectThreadH
#define TConnectThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "LogClass.h"
#include "TWConfig.h"
#include "XMLPacket.h"
#include <IdTCPClient.hpp>
#include <IdHTTP.hpp>
#include <IdIntercept.hpp>
#include <IdSSLIntercept.hpp>
#include <IdSSLOpenSSL.hpp>
//---------------------------------------------------------------------------
class TConnectThread : public TThread
{
private:
    int Method;
    int m_keyID;
protected:
    void __fastcall Execute();
    bool InnerLog;
    TLogClass *Log;
    TWConfig *Cfg;
    AnsiString URL;
    AnsiString SignedMessage;
    int inf(TMemoryStream *source, TMemoryStream *dest);
public:
    __fastcall TConnectThread(TLogClass *_Log, TWConfig *_Cfg, AnsiString _URL, AnsiString _SignedMessage, int keyID, AnsiString _DetachedSignature = "");
    __fastcall virtual ~TConnectThread(void);
    void Process();
    AnsiString AnswerMessage;
    int AnswerContentLength;
    bool Finished;
    bool IndyError;
    AnsiString ConnectResult;
    bool ConnectOK;
    bool TransportError;
    AnsiString DetachedSignature;
};
//---------------------------------------------------------------------------

std::string encodeLocaleString(std::string, std::string exceptionChars = "");
#endif
