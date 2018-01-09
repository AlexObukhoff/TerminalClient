//---------------------------------------------------------------------------

#ifndef TGetCardsInfoThreadH
#define TGetCardsInfoThreadH
#include <Classes.hpp>
#include "LogClass.h"
#include "TWConfig.h"
#include "XMLInfo.h"
#include "TPinPayment.h"
#include "TFileMap.h"
#include "TJSMaker.h"

class TGetCardsInfoThread  : public TThread
{
private:
    TJSMaker *JSMaker;
    TFileMap* FileMap;
    bool InnerLog;
    TLogClass *Log;
    TWConfig *Cfg;
    TXMLInfo *InfoFile;
    TFileStream *DetailsFile;
    TPinPayment *Payment;
    void CheckCardsInfoForOperator(int _OpId);
    int GetCards(int _OpId);
    AnsiString GetNewCardsInfo(int _OpId);
    TCriticalSection *CS;
    TDateTime _TimeMark;
    TDateTime ReadThreadTimeMark();
    void WriteThreadTimeMark(TDateTime _Src);
    bool CardsInfoChanged;
    bool FirstCheck;
    bool StoreFile(AnsiString FileName, AnsiString Content);
    AnsiString CheckPinImgsExistance(int OpId, AnsiString PinString);
    void StoreJSFile();
    void CheckForIncompletePayments();
    AnsiString GetCardsString(int OpId, AnsiString InfoFromDetails);
    AnsiString MakeFullCardString(AnsiString FromDetails, AnsiString FromFiles);
protected:
    void __fastcall Execute();
public:
    __property TDateTime TimeMark = { read=ReadThreadTimeMark, write=WriteThreadTimeMark };
    double DiffDT;
    int Interval;

    __fastcall TGetCardsInfoThread(TXMLInfo*, TWConfig*, TLogClass*, TFileMap*);
    __fastcall ~TGetCardsInfoThread();

    bool Finished;
};

//---------------------------------------------------------------------------
#endif
