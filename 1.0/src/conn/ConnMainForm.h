//---------------------------------------------------------------------------

#ifndef ConnMainFormH
#define ConnMainFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "LogClass.h"
#include "TWConfig.h"
#include "TFileMap.h"
#include "TAviaPayment.h"
#include <ExtCtrls.hpp>
#include "TSendHeartBeatThread.h"
#include "TOutboundThread.h"
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
    TTimer *CheckTimeTimer;
    TTimer *CheckThreadsTimer;
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall CheckTimeTimerTimer(TObject *Sender);
    void __fastcall CheckThreadsTimerTimer(TObject *Sender);
//    void __fastcall FormCreate(TObject *Sender);
private:	// User declarations
    TLogClass *Log;
    TWConfig *Cfg;
    TXMLInfo *InfoFile;
    TFileMap* FileMap;
    void Reboot();
    void ShutDown();
    HANDLE hMutexOneInstance;
    TSendHeartBeatThread *SendHeartBeatThread;
    TOutboundThread *PaymentsThread;
    TOutboundThread *StatThread;
    TOutboundThread *CommandsThread;
    TOutboundThread *EMailThread;
    void CheckThreads();
    void PurgeLogs();
    bool IsProgramWindowExists(AnsiString ApplicationWindowCaption);
    bool StartUpdate();
    bool AlreadyRunning(void);
    bool IsProcessRunning(HANDLE PrHandle);
    HANDLE StartProgram(AnsiString ApplicationName);
    bool TerminateProgram(HANDLE PrHandle);
    bool StartWebClient();
    void TerminateWebClient();
    void RestartWebClient();

    int m_timerTicksCount;    // для удаления неушедших файлов
public:		// User declarations
    __fastcall virtual ~TForm1();
    __fastcall TForm1(TComponent* Owner);
    TDateTime *ConnStartDT;

};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
