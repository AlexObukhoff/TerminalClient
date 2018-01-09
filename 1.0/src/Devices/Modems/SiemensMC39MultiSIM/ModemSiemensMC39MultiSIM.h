//---------------------------------------------------------------------------
#ifndef ModemSiemensMC39MultiSIMH
#define ModemSiemensMC39MultiSIMH
//---------------------------------------------------------------------------
#include "Modem.h"
#include "ModemSiemensMC35i.h"
#include "LogClass.h"

class CModemSiemensMC39MultiSIM : public CModemSiemensMC35i
{
protected:
public:
  	CModemSiemensMC39MultiSIM(int ComPort,TLogClass* _Log = NULL);
  	~CModemSiemensMC39MultiSIM();

    virtual bool ResetModem();
    virtual bool ChangeSIM_No(int SIM_No);
    virtual bool ChangeSIM();
    virtual bool IsItYou();
};
#endif