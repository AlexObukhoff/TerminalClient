//---------------------------------------------------------------------------


#pragma hdrstop

#include "TNote.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)


void TNote::swap(const TNote& rhs)
{
    this->ValidatorID=rhs.ValidatorID;
    this->CurrencyID=rhs.CurrencyID;
    this->Nominal=rhs.Nominal;
    this->Count=rhs.Count;
}

TNote::TNote(const TNote& rhs)
{
    swap(rhs);
}

TNote::TNote()
{
    this->ValidatorID=-1;
    this->CurrencyID="";
    this->Nominal=0;
    this->Count=0;
}

TNote& TNote::operator = (const TNote& rhs)
{
    TNote tmp(rhs);
    swap(rhs);
    return *this;
}
