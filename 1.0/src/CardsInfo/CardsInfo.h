//---------------------------------------------------------------------------

#ifndef CardsInfoH
#define CardsInfoH
#include "TWConfig.h"
#include "LogClass.h"
//---------------------------------------------------------------------------
typedef struct CardInfo
{
  AnsiString CardName;
  AnsiString CardNumber;
  double Sum;
  AnsiString OperatorName;
  AnsiString OperatorCode;
} TCardInfo;
typedef TCardInfo* PCardInfo;

class TCardsInfo
{
  TList *Cards;
  bool InnerLog;
  TLogClass *Log;
public:
  int CardsCount;
  TDateTime ActualDT;
  TCardsInfo(TLogClass*);
  bool SetCardsInfo(AnsiString,TDateTime);
  ~TCardsInfo();
  void Clear();
  PCardInfo GetCard(int);
  PCardInfo GetCardByNumber(AnsiString);
  AnsiString GetCardName(AnsiString);
  AnsiString GetCardNumberByIndex(int);
  double GetCardSum(AnsiString);
  AnsiString GetOperatorName(AnsiString);
  AnsiString GetOperatorCode(AnsiString);
};

#endif
 