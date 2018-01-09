//---------------------------------------------------------------------------


#pragma hdrstop

#include "CardsInfo.h"
//#include "TPayment.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

TCardsInfo::TCardsInfo(TLogClass *_Log)
{
try
  {
  InnerLog=false;
  Log=_Log;
  if (_Log==NULL) {
    Log = new TLogClass("TCardsInfo");
    InnerLog=true;
    }
  Cards = NULL;
  Cards = new TList;
  CardsCount = 0;
  }
catch (Exception &ex)
 {
 Log->Write("Exception in TCardsInfo::TCardsInfo: "+ex.Message);
 }
}

//---------------------------------------------------------------------------

bool TCardsInfo::SetCardsInfo(AnsiString Source, TDateTime _ActualDT)
{
PCardInfo Card;
AnsiString ACardInfo;
try
  {
  Clear();
  Source+=":";
  while (Source.Pos(":")!=0) {
    ACardInfo = Source.SubString(0,Source.Pos(":")-1);
    ACardInfo+="=";
    Card = new TCardInfo;

    Card->CardName=ACardInfo.SubString(0,ACardInfo.Pos("=")-1);
    ACardInfo = ACardInfo.SubString(ACardInfo.Pos("=")+1,ACardInfo.Length());

    Card->CardNumber=ACardInfo.SubString(0,ACardInfo.Pos("=")-1).ToInt();
    ACardInfo = ACardInfo.SubString(ACardInfo.Pos("=")+1,ACardInfo.Length());

    try
      {
      Card->Sum=ACardInfo.SubString(0,ACardInfo.Pos("=")-1).ToDouble();
      }
    catch (...)
      {
      Card->Sum=-1;
      }
    ACardInfo = ACardInfo.SubString(ACardInfo.Pos("=")+1,ACardInfo.Length());

    Card->OperatorName=ACardInfo.SubString(0,ACardInfo.Pos("=")-1);
    ACardInfo = ACardInfo.SubString(ACardInfo.Pos("=")+1,ACardInfo.Length());

    Card->OperatorCode=ACardInfo.SubString(0,ACardInfo.Pos("=")-1);
    ACardInfo = ACardInfo.SubString(ACardInfo.Pos("=")+1,ACardInfo.Length());

    Cards->Add(Card);

    Source = Source.SubString(Source.Pos(":")+1,Source.Length());
    CardsCount++;
    }

  if (_ActualDT.Val!=0)
    ActualDT=_ActualDT;
    else
    ActualDT=ActualDT.CurrentDateTime();

  return true;
  }
catch (Exception &ex)
  {
  Log->Write("Exception in ~SetCardsInfo: "+ex.Message);
  return false;
  }
}

//---------------------------------------------------------------------------

TCardsInfo::~TCardsInfo()
{
try
  {
  if (InnerLog)
    delete Log;
  if (Cards) {
    Clear();
    delete Cards;
    Cards=NULL;
    }
  }
catch (Exception &ex)
  {
  Log->Write("Exception in ~TCardsInfo: "+ex.Message);
  }
}

//---------------------------------------------------------------------------

void TCardsInfo::Clear()
{
try
  {
  PCardInfo Card;
  for (int i=0;i<Cards->Count;i++) {
    Card=(PCardInfo)Cards->Items[i];
    delete Card;
    }
  Cards->Clear();
  CardsCount=0;
  }
catch (Exception &ex)
  {
  Log->Write("Exception in TCardsInfo::Clear: "+ex.Message);
  }
}

//---------------------------------------------------------------------------

PCardInfo TCardsInfo::GetCard(int _CardIndex)
{
try
  {
  PCardInfo Card = NULL;
  if (_CardIndex<CardsCount)
    Card=(PCardInfo)Cards->Items[_CardIndex];
    else
    Log->Write("Card "+AnsiString(_CardIndex)+" is out of bounds.");
  return Card;
  }
catch (Exception &ex)
  {
  Log->Write("Exception in TCardsInfo::GetCard: "+ex.Message);
  return NULL;
  }
}


PCardInfo TCardsInfo::GetCardByNumber(AnsiString _CardNumber)
{
try
  {
  PCardInfo Card;
  for (int i=0;i<Cards->Count;i++) {
    Card=GetCard(i);
      if (Card->CardNumber==_CardNumber) {
          return Card;
          }
      }
  Log->Write("Card "+AnsiString(_CardNumber)+" not found.");
  return NULL;
  }
catch (Exception &ex)
  {
  Log->Write("Exception in TCardsInfo::GetCard: "+ex.Message);
  return NULL;
  }
}

//---------------------------------------------------------------------------

AnsiString TCardsInfo::GetCardName(AnsiString _CardNumber)
{
try
  {
  PCardInfo Card=GetCardByNumber(_CardNumber);
  if (Card!=NULL)
    return Card->CardName;
    else
    return "";
  }
catch (Exception &ex)
  {
  Log->Write("Exception in TCardsInfo::GetCardName: "+ex.Message);
  return "";
  }
}

//---------------------------------------------------------------------------

AnsiString TCardsInfo::GetCardNumberByIndex(int _CardIndex)
{
try
  {
  PCardInfo Card=GetCard(_CardIndex);
  if (Card!=NULL)
    return Card->CardNumber;
    else
    return "";
  }
catch (Exception &ex)
  {
  Log->Write("Exception in TCardsInfo::GetCardNumberByIndex: "+ex.Message);
  return "";
  }
}

//---------------------------------------------------------------------------

double TCardsInfo::GetCardSum(AnsiString _CardNumber)
{
try
  {
  PCardInfo Card=GetCardByNumber(_CardNumber);
  if (Card!=NULL)
    return Card->Sum;
    else
    return -1;
  }
catch (Exception &ex)
  {
  Log->Write("Exception in TCardsInfo::GetCardSum: "+ex.Message);
  return -1;
  }
}

//---------------------------------------------------------------------------

AnsiString TCardsInfo::GetOperatorName(AnsiString _CardNumber)
{
try
  {
  PCardInfo Card=GetCardByNumber(_CardNumber);
  if (Card!=NULL)
    return Card->OperatorName;
    else
    return "";
  }
catch (Exception &ex)
  {
  Log->Write("Exception in TCardsInfo::GetOperatorName: "+ex.Message);
  return "";
  }
}

//---------------------------------------------------------------------------

AnsiString TCardsInfo::GetOperatorCode(AnsiString _CardNumber)
{
try
  {
  PCardInfo Card=GetCardByNumber(_CardNumber);
  if (Card!=NULL)
    return Card->OperatorCode;
    else
    return "";
  }
catch (Exception &ex)
  {
  Log->Write("Exception in TCardsInfo::GetOperatorCode: "+ex.Message);
  return "";
  }
}


