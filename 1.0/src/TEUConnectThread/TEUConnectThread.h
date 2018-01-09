//---------------------------------------------------------------------------

#ifndef TEUConnectThreadH
#define TEUConnectThreadH
//---------------------------------------------------------------------------
/*#include "LogClass.h"
#include "TWConfig.h"*/
#include "TConnectThread.h"
#include "evroset_lib.h"

//---------------------------------------------------------------------------
class TEUConnectThread : public TConnectThread
  {
  private:
  TEvroSetLib *ESL;
  protected:
    void __fastcall Execute();
  public:
    __fastcall TEUConnectThread(TLogClass*, TWConfig*, AnsiString, AnsiString);
    __fastcall virtual ~TEUConnectThread(void);
  //    void Connect();
    void Process();
  };
//---------------------------------------------------------------------------
#endif
