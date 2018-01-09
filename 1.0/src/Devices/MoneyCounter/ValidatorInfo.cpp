//---------------------------------------------------------------------------


#pragma hdrstop

#include "ValidatorInfo.h"

//---------------------------------------------------------------------------
CValidatorInfo::CValidatorInfo(AnsiString filename)
{
    FileName = filename;
    Validators = new TList();
}

CValidatorInfo::~CValidatorInfo()
{
    for(int i=0; i<Validators->Count; i++)
        delete (CMoneyCounter*)(Validators->Items[i]);
}

int CValidatorInfo::GetValidatorIndex(int ID)
{
    for(int i=0; i<Validators->Count; i++)
        if (((CMoneyCounter*)Validators->Items[i])->ID == ID)
            return i;
    return -1;
}

int CValidatorInfo::GetLastBill(int ValidatorID)
{
    int index = GetValidatorIndex(ValidatorID);
    if (index >= 0)
        return ((CMoneyCounter*)Validators->Items[index])->LastBill;
    return -1;
}

int CValidatorInfo::IncCounter(int ValidatorID, double Nominal)
{
    int index = GetValidatorIndex(ValidatorID);
    if (index >= 0)
        return ((CMoneyCounter*)Validators->Items[index])->IncCounter(Nominal);
    return -1;
}

int CValidatorInfo::GetCount(int ValidatorID, double Nominal)
{
    int index = GetValidatorIndex(ValidatorID);
    if (index >= 0)
        return ((CMoneyCounter*)Validators->Items[index])->GetCount(Nominal);
    return -1;
}

#include "Unit1.h"
#include "boost/format.hpp"

void CValidatorInfo::ClearCounters(int ValidatorID)
{
    Form1->Log->Write((boost::format("ValidatorID %1%") % ValidatorID).str().c_str());
    int index = GetValidatorIndex(ValidatorID);
    if (index >= 0)
        ((CMoneyCounter*)Validators->Items[index])->ClearCounters();
}

double CValidatorInfo::TotalMoney(int ValidatorID)
{
    int index = GetValidatorIndex(ValidatorID);
    if (index >= 0)
        return ((CMoneyCounter*)Validators->Items[index])->TotalMoney();
    return -1;
}

int CValidatorInfo::TotalBill(int ValidatorID)
{
    int index = GetValidatorIndex(ValidatorID);
    if (index >= 0)
        return ((CMoneyCounter*)Validators->Items[index])->TotalBill();
    return -1;
}

#pragma package(smart_init)
