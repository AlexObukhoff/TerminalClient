//---------------------------------------------------------------------------

#ifndef ValidatorInfoH
#define ValidatorInfoH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "CMoneyCounter.h"

class CValidatorInfo
{
protected:
    TList* Validators;
    AnsiString FileName;
    int GetValidatorIndex(int ID);
public:
    CValidatorInfo(AnsiString filename);
    virtual ~CValidatorInfo();

    virtual int     GetLastBill(int ValidatorID);
    virtual int     IncCounter(int ValidatorID, double Nominal);
    virtual int     GetCount(int ValidatorID, double Nominal);
    virtual void    ClearCounters(int ValidatorID);
    virtual double  TotalMoney(int ValidatorID);
    virtual int     TotalBill(int ValidatorID);
};


#endif
