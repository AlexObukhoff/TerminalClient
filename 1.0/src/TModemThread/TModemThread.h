//---------------------------------------------------------------------------
#ifndef TModemThreadH
#define TModemThreadH
//---------------------------------------------------------------------------
#include "LogClass.h"
#include "TWConfig.h"
#include "ModemSiemensMC35i.h"
#include "TFileMap.h"
#include "SMSOutbound.h"
#include "TModem.h"

//---------------------------------------------------------------------------

class TModemThread  : public TThread
{
private:
  TLogClass* m_pLog;
  TWConfig* m_pCfg;
  TFileMap* m_pFileMap;
  TModemClass* m_pModem;
  CWatchDog* m_pWatchDog;
  TSMSOutbound* m_pOutbound;

  TDateTime m_timeMark;
  AnsiString m_message;

  AnsiString ReadMessage();
  void WriteMessage(AnsiString msg);
  TDateTime ReadThreadTimeMark();
  void WriteThreadTimeMark(TDateTime timeMark);
  
protected:
  void __fastcall Execute();
  //void SendNotification(float _SignalQuality, float _Balance);

  void processSMS();
  void processInfo();
public:
  __fastcall TModemThread(TWConfig*, CWatchDog*, TFileMap*);
  __fastcall ~TModemThread();
  __property TDateTime TimeMark = { read=ReadThreadTimeMark, write=WriteThreadTimeMark };
  __property AnsiString Message = { read=ReadMessage, write=WriteMessage };
  bool Finished;
};

//---------------------------------------------------------------------------

#endif
