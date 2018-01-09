//---------------------------------------------------------------------------

#ifndef THTTPGetFileThreadH
#define THTTPGetFileThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "LogClass.h"
#include "TWConfig.h"
#include <IdTCPClient.hpp>
#include <IdHTTP.hpp>
#include <IdIntercept.hpp>
#include <IdSSLIntercept.hpp>
#include <IdSSLOpenSSL.hpp>
//---------------------------------------------------------------------------
const int GetFileOK    				= 0;
const int GetFileError 				= 1;
const int GetFileCheckFailed 	= 2;

class THTTPGetFileThread : public TThread
{
private:
        TIdHTTP *IdHTTPC;
        TIdConnectionInterceptOpenSSL *IdSSL;
//        TMemoryStream* Data;
        TMemoryStream* Answer;
        TFileStream* InFile;
        int Method;
protected:
        void __fastcall Execute();
        bool InnerLog;
        TLogClass *Log;
        TWConfig *Cfg;
        AnsiString URL;
				AnsiString FileName;
				AnsiString CUID;
				TDateTime CheckDT;
				bool TestArchiveFile(AnsiString SourceFileName);
public:
				__fastcall THTTPGetFileThread(TLogClass*, TWConfig*, AnsiString, AnsiString, AnsiString, TDateTime);
        __fastcall virtual ~THTTPGetFileThread(void);
//    void Connect();
        void Process();
//        AnsiString SignedMessage;
//        AnsiString AnswerMessage;
				int Result;
				int ServerReply;
        int AnswerContentLength;
        bool Finished;
				bool IndyError;
				TDateTime LastUpdatedDT;
				};
//---------------------------------------------------------------------------
#endif
