//---------------------------------------------------------------------------
#ifndef ModemSiemensMC35iH
#define ModemSiemensMC35iH
//---------------------------------------------------------------------------
#include "Modem.h"
#include "common.h"
#include "LogClass.h"

class CModemSiemensMC35i : public CModem
{
protected:
    TStringList*  OperatorNames;
    TList*        OperatorIDs;
    void GetOperatorItems(AnsiString Answer, int index, int& OperatorID, AnsiString& OperatorName);
    virtual void GetOperators();
    AnsiString FindBalance(AnsiString Src);
public:
  	CModemSiemensMC35i(int ComPort,TLogClass* _Log = NULL, AnsiString Prefix = "SiemensMC35i");
  	virtual ~CModemSiemensMC35i();

	  virtual float SignalQuality();
    virtual float SignalLevel();
	  virtual double GetBalance(AnsiString BalanceNumber, int RepeatRequest = 5);
    virtual bool IsItYou();
    virtual AnsiString GetOperatorName(int ID = 0);
    virtual AnsiString GetOperatorName();
		AnsiString DecodeString(AnsiString Src);
};
#endif