//---------------------------------------------------------------------------

#ifndef TSendHeartBeatThreadH
#define TSendHeartBeatThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "LogClass.h"
#include "TWConfig.h"
#include "PacketSender.h"
#include "TFileMap.h"
#include "XMLOutbound.h"
//---------------------------------------------------------------------------
class TSendHeartBeatThread : public TThread
{
private:
    bool m_isInnerLog;
    TLogClass* m_pLog;
    TFileMap *m_pFileMap;
    TWConfig *m_pCfg;

    
    TPacketSender *PS;
    TEMailSender *EMailSender;
    TOutbound *XMLOut;
    TOutbound *XMLStatOut;
    int Interval;
    TCriticalSection *CS;
    TDateTime m_timeMark;

    TDateTime ReadThreadTimeMark();
    void WriteThreadTimeMark(TDateTime _Src);

protected:
    void __fastcall Execute();

public:
    bool Finished;

    __fastcall TSendHeartBeatThread(TFileMap *fileMap, TWConfig *cfg, TLogClass *log);
    __fastcall virtual ~TSendHeartBeatThread(void);
    __property TDateTime TimeMark = { read=ReadThreadTimeMark, write=WriteThreadTimeMark };
};
//---------------------------------------------------------------------------
#endif
