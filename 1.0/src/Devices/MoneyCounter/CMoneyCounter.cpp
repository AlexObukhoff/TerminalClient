//---------------------------------------------------------------------------
#pragma hdrstop
#include "CMoneyCounter.h"
#include "globals.h"
#include "boost/format.hpp"
//---------------------------------------------------------------------------
CMoney::CMoney(double nominal)
{
    Nominal = nominal;
    Count = 0;
}

CMoney::~CMoney()
{}

void CMoney::Inc()
{
    Count++;
}

//=============================================================================

CMoneyCounter::CMoneyCounter(int id, AnsiString filename, TLogClass* log, TXMLInfo* _SaveFile)
{
    try
      {
          DeviceName = "Validator"+AnsiString(id);
          Log = log;
          if (Log)
              Log->Write((boost::format("MoneyCounter with ID=%1% created.") % id).str().c_str());
          Elements = new TList();
          LastBill = 0;
          ID = id;
          FileName = filename;
          //SaveFile = new TXMLInfo(filename, log);
          SaveFile = _SaveFile;
          Notes.clear();
          Read();
      }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

CMoneyCounter::~CMoneyCounter()
{
    for(int i=0; i<Elements->Count; i++)
    {
        CMoney* element = (CMoney*)Elements->Items[i];
        delete element;
    }
    delete Elements;
    //if (SaveFile)
    //    delete SaveFile;
}

int CMoneyCounter::GetElementIndex(double Nominal)
{
    int result = -1;
    for(int i=0; i<Elements->Count; i++)
        if ( ((CMoney*)Elements->Items[i])->Nominal == Nominal )
            return i;
    return result;
}

int CMoneyCounter::IncCounter(double Nominal)
{
    int index = GetElementIndex(Nominal);
    if (index >= 0)
        ((CMoney*)Elements->Items[index])->Inc();
    else
    {
        index = AddNominal(Nominal);
        ((CMoney*)Elements->Items[index])->Inc();
    }
    LastBill = Nominal;
    int result = ((CMoney*)Elements->Items[index])->Count;
    Write(Nominal, result);
    return result;
}

int CMoneyCounter::GetCount(double Nominal)
{
    int index = GetElementIndex(Nominal);
    if (index >= 0)
        return ((CMoney*)Elements->Items[index])->Count;
    return 0;
}

void CMoneyCounter::ClearCounters()
{
    for(int i=0; i<Elements->Count; i++)
        ((CMoney*)Elements->Items[i])->Count = 0;
    SaveFile->WriteIncassation(DeviceName);
    ReadIncassation();
}

int CMoneyCounter::AddNominal(double Nominal)
{
    if (GetElementIndex(Nominal) < 0)
        Elements->Add(new CMoney(Nominal));
    return Elements->Count - 1;
}

double CMoneyCounter::TotalMoney()
{
    double result = 0;
    for(int i=0; i<Elements->Count; i++)
        result += ((CMoney*)Elements->Items[i])->Count * ((CMoney*)Elements->Items[i])->Nominal;
    return result;
}

int CMoneyCounter::TotalBill()
{
    int result = 0;
    for(int i=0; i<Elements->Count; i++)
        result += ((CMoney*)Elements->Items[i])->Count;
    return result;
}

void CMoneyCounter::Read()
{
    try
      {
          Notes.clear();
          SaveFile->ReadNotes(DeviceName,Notes);
          for(int i=0; i<Elements->Count; i++)
              delete Elements->Items[i];
          double Nominal, Count;
          for(int i = 0; i < Notes.size(); i++)
          {
              Nominal = Notes[i].Nominal;
              Count = Notes[i].Count;

              int index = GetElementIndex(Nominal);
              if (index >= 0)
              {
                  for(int j=1; j<=Count; j++)
                    ((CMoney*)Elements->Items[index])->Inc();
              }
              else
              {
                  index = AddNominal(Nominal);
                  for(int j=1; j<=Count; j++)
                    ((CMoney*)Elements->Items[index])->Inc();
              }
          }
          ReadIncassation();
      }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CMoneyCounter::Write()
{
    try
      {
          Notes.clear();
          AnsiString str;
          for(int i=0; i<Elements->Count; i++)
          {
              //Log->Write("Write to "+FileName+" Nominal="+AnsiString(((CMoney*)Elements->Items[i])->Nominal)+"; Count="+AnsiString(((CMoney*)Elements->Items[i])->Count));
              Notes.push_back(TNote(((CMoney*)Elements->Items[i])->Nominal, ((CMoney*)Elements->Items[i])->Count));
          }
          SaveFile->WriteNotes(DeviceName,Notes);
      }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CMoneyCounter::Write(double Nominal, int Count)
{
    try
      {
          AnsiString Name = FloatToStr(Nominal);
          AnsiString Value = IntToStr(Count);
          //if (Log)
          //    Log->Write("Write to "+FileName+" Nominal="+Name+"; Count="+Value);
          SaveFile->WriteNote(DeviceName,Name,Value);
      }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

AnsiString CMoneyCounter::ReadIncassation()
{
      //LastIncassation = SaveFile->ReadDateTime(DeviceName,"LastIncassation");;
      return LastIncassation = SaveFile->GetIncassationDT(DeviceName);
}

int CMoneyCounter::GetNominalCount()
{
    return Elements->Count;
}

double CMoneyCounter::GetNominal(int index)
{
    return ((CMoney*)Elements->Items[index])->Nominal;
}

double CMoneyCounter::GetCount(int index)
{
    return ((CMoney*)Elements->Items[index])->Count;
}


#pragma package(smart_init)
