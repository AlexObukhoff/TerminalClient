//---------------------------------------------------------------------------

#ifndef UpdaterFormH
#define UpdaterFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include "LogClass.h"
//#include "TWConfig.h"
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
    TLabel *Label1;
  TTimer *StartTimer;
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall FormDestroy(TObject *Sender);
  void __fastcall StartTimerTimer(TObject *Sender);
private:	// User declarations
    TLogClass *Log;
    //TWConfig *WebClientCfg;
    TDateTime DTStartUpdate;
    HANDLE UpdateHandle;
    DWORD UpdateProcessId;
    bool UpdateStarted;
    bool IsProcessRunning(DWORD ProcessId);
    bool IsProgramWindowExists(AnsiString);
    bool StartUpdate(void);
    bool StartProgram(AnsiString);
    void TerminateUpdateProgram();
    bool BackUp(void);
		bool CopyDir(AnsiString aFrom, AnsiString aTo, bool bClearDir = true);
		bool RollBack(void);
    bool DeleteDir(AnsiString DirName);
    AnsiString ShowError(AnsiString Header);
    bool FinishUpdate(void);
    bool StartWebClient(void);
    AnsiString GetPath(void);
    AnsiString GetDrive(void);
    AnsiString GetPrRoot(void);
    AnsiString Path;
    AnsiString Drive;
    AnsiString PrRoot;
public:		// User declarations
    __fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
