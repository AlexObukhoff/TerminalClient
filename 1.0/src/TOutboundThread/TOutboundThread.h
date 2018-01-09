//---------------------------------------------------------------------------

#ifndef TOutboundThreadH
#define TOutboundThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "LogClass.h"
#include "TWConfig.h"
#include "XMLOutbound.h"
#include "TFileMap.h"

const int cnPaymentsOut = 1;
const int cnStatOut = 2;
const int cnCommandsIn = 3;
const int cnEMailOut = 4;

//---------------------------------------------------------------------------
class TOutboundThread : public TThread
{
private:
    TFileMap* FileMap;
    TXMLInfo* m_infoFile;
    TLogClass* Log;
    bool InnerLog;
    TWConfig* Cfg;
    TOutbound *XMLOut;
    int DirType;
    TCriticalSection *CS;
    TDateTime _TimeMark;
    TDateTime ReadThreadTimeMark();// {TDateTime DT; if (CS) {CS->Acquire(); DT = _TimeMark; CS->Release();} else DT=0; return DT;};
    void WriteThreadTimeMark(TDateTime _Src);// {if (CS) {CS->Acquire(); _TimeMark=_Src; CS->Release();}};
    int Interval;
protected:
    void __fastcall Execute();
public:
    __property TDateTime TimeMark = { read=ReadThreadTimeMark, write=WriteThreadTimeMark };
    int Command;
    __fastcall TOutboundThread(int, TWConfig*, TLogClass*, TFileMap*,TXMLInfo* infoFile);
    __fastcall virtual ~TOutboundThread(void);
    int UnprocessedFilesCount;
    TDateTime DTLastFileProcessed;
    TOutbound *Outbound;
    void Stop();
    bool Finished;
    bool IndyError;
    void Process(bool bFilesCountOnly = false);
};
//---------------------------------------------------------------------------
#endif
