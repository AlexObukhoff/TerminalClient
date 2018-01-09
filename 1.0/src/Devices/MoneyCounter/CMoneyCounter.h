//---------------------------------------------------------------------------

#ifndef CMoneyCounterH
#define CMoneyCounterH
//---------------------------------------------------------------------------

#include <Classes.hpp>
#include "XMLInfo.h"
#include "TNote.h"

class CMoney
{
public:
    CMoney(double nominal);
    virtual ~CMoney();

    double  Nominal;
    int     Count;
    void    Inc();
};

class CMoneyCounter
{
private:
protected:
    TLogClass* Log;
    TList* Elements;
    AnsiString FileName;
    AnsiString DeviceName;
    TXMLInfo* SaveFile;

    int GetElementIndex(double Nominal);
    int AddNominal(double Nominal);

    void Read();
    void Write();
    void Write(double Nominal, int Count);
public:
    CMoneyCounter(int id, AnsiString filename, TLogClass* log, TXMLInfo* _SaveFile);
    virtual ~CMoneyCounter();

    int ID;
    int LastBill;
    TDateTime LastIncassation;
    TNotesVector Notes;

    virtual int     IncCounter(double Nominal);
    virtual int     GetCount(double Nominal);
    virtual void    ClearCounters();
    virtual double  TotalMoney();
    virtual int     TotalBill();
    virtual int     GetNominalCount();
    virtual double  GetNominal(int index);
    virtual double  GetCount(int index);
    virtual AnsiString ReadIncassation();
};

#endif
