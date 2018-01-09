//---------------------------------------------------------------------------
#ifndef TModemH
#define TModemH
//---------------------------------------------------------------------------
#include <vcl.h>
#include <algorith.h>
#include <Classes.hpp>
#include <ExtCtrls.hpp>
#include <ras.h>
#include <raserror.h>
#include <idicmpclient.hpp>

#include "LogClass.h"
#include "TWConfig.h"
#include "CWatchDog.h"
#include "ModemSiemensMC35i.h"
#include "TFileMap.h"
#include "SMSOutbound.h"
//---------------------------------------------------------------------------

class TModemClass
{
private:
    // для правильного определения размера структуры RASCONN
    DWORD m_rasConnSize;
    bool m_isInnerLog;
    TLogClass* m_pLog;
    TWConfig* m_pCfg;
    CWatchDog* m_pWatchDog;
    TFileMap* m_pFileMap;
//    HRASCONN m_hRemoteAccessService;
    int m_linkFailures;
    int m_rebootFailures;
    TDateTime m_lastDialTime;

    HRASCONN getConnection();
    RASCONNSTATE getConnectionState(HRASCONN hRasConn);
    std::string getStatusDescription(RASCONNSTATE  rasconnstate);
    bool isPingOK();
    void processFailure();
    void hangUpThis(HRASCONN hRasConn);

public:
    TModemClass(TLogClass* log, TWConfig* cfg, CWatchDog* watchDog, TFileMap* fileMap);
    ~TModemClass();

    void checkConnection();
    void hangUp();
    void dial();
    bool isConnected();

    bool Finished;
};

//---------------------------------------------------------------------------

#endif
