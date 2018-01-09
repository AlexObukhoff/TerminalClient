//---------------------------------------------------------------------------
#pragma hdrstop
#include "TGetCardsInfoThread.h"
#include "globals.h"
#include "boost/format.hpp"
//---------------------------------------------------------------------------

void __fastcall TGetCardsInfoThread::Execute()
{
    try
    {
        try
        {
            Log->Write("GetCardsInfo thread execution started.");
            int StartTimeOut = 10;
            //int StartTimeOut = 3000;
            Log->Write("Storing cards info...");
            StoreJSFile();
            while ((StartTimeOut>0)&&(!Terminated))
            {
                StartTimeOut--;
                Sleep(100);
                if (FileMap)
                    TimeMark=TimeMark.CurrentDateTime();
            }
            FirstCheck=true;
            int CheckCards = 0;
            int CashCards = 0;
            while (!Terminated)
            {
                CheckForIncompletePayments();
                if (CheckCards == 0)
                {
                    CheckCards = 9;
                    for (std::size_t i=0;i<Cfg->OperatorsInfo.size();i++)
                    {
                        AnsiString ProcessorType=AnsiString(Cfg->Operator(Cfg->OperatorByNum(i).Id).ProcessorType.c_str()).LowerCase();
                        if (ProcessorType == "cyberplat_pin" || ProcessorType == "half_pin" || ProcessorType=="cyberplat_pin_trans")
                        {
                            CheckCardsInfoForOperator(Cfg->OperatorByNum(i).Id);
                            TimeMark=TimeMark.CurrentDateTime();
                        }
                    }

                    if (CardsInfoChanged)
                    {
                        Log->Write("Cards info changed!");
                        StoreJSFile();
                    }

                    CardsInfoChanged = false;
                }


                if (CashCards == 0)
                {
                    CashCards = 1;
                    for (std::size_t i=0;i<Cfg->OperatorsInfo.size();i++)
                    {
                        AnsiString ProcessorType=AnsiString(Cfg->Operator(Cfg->OperatorByNum(i).Id).ProcessorType.c_str()).LowerCase();
                        if (ProcessorType == "cyberplat_pin" || ProcessorType == "half_pin" || ProcessorType=="cyberplat_pin_trans")
                        {
                            TimeMark=TimeMark.CurrentDateTime();
                            /*
                            Cfg->OperatorByNum(i).GetCardsAllowed = true;

                            if (Cfg->OperatorByNum(i).GetCardsAllowed)
                            */
                            {
                                if (GetCards(Cfg->OperatorByNum(i).Id)==1) // недостаточно средств на счету дилера
                                {
                                    Log->Write("Money shortage detected, terminating next tries for 10 minutes...");
                                    CashCards = 10;
                                    break;
                                }
                            }
                        }
                    }
                }

                CheckCards--;
                CashCards--;
                int TimeOut = Interval/100;
                //int TimeOut = Interval/1000000;
                while ((!Terminated)&&(TimeOut>0))
                {
                    TimeMark=TDateTime::CurrentDateTime();
                    TimeOut--;
                    Sleep(100);
                }
                FirstCheck=false;
            }
        }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        }
    }
    __finally
    {
        Log->Write("GetCardsInfo thread execution done.");
        Finished=true;
    }
}

//---------------------------------------------------------------------------

void TGetCardsInfoThread::StoreJSFile()
{
    try
    {
        JSMaker->Tab="\t";
        JSMaker->Clear();
        JSMaker->AddString("$ga_jcfg['details'] = {");
        JSMaker->Level++;
        JSMaker->AddChild("pin_info");
        for (std::size_t i=0;i<Cfg->OperatorsInfo.size();i++)
        {
            AnsiString ProcessorType=AnsiString(Cfg->Operator(Cfg->OperatorByNum(i).Id).ProcessorType.c_str()).LowerCase();
            if (ProcessorType == "cyberplat_pin" || ProcessorType == "half_pin" || ProcessorType == "cyberplat_pin_trans")
            {
                AnsiString Temp = GetCardsString(Cfg->OperatorByNum(i).Id, InfoFile->Read("CardsInfo","Op"+AnsiString(Cfg->OperatorByNum(i).Id)));
                JSMaker->AddChild(AnsiString(Cfg->OperatorByNum(i).Id));
                JSMaker->AddStringAttribute("data", Temp);
                JSMaker->AddStringAttribute("imgs", CheckPinImgsExistance(Cfg->OperatorByNum(i).Id,InfoFile->Read("CardsInfo","Op"+AnsiString(Cfg->OperatorByNum(i).Id))));
                JSMaker->CloseChild();
            }
        }
        JSMaker->CloseChild();
        JSMaker->CloseChild(false);
        JSMaker->AddString("//  - - - - - всегда в true, для динамического include файла\n$iface_details_js = true;");
        if (StoreFile((Cfg->Dirs.InterfaceDir+"\\iface_details.js").c_str(),JSMaker->Content))
            Log->Write("iface_details.js stored...");
        else
            Log->Write("Error saving iface_details.js!");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

AnsiString TGetCardsInfoThread::CheckPinImgsExistance(int OpId, AnsiString PinString)
{
    AnsiString ImgFileName, Res;
    PinString+=":";
    while (PinString.Pos(":"))
    {
        AnsiString PinString2;
        PinString2 = PinString.SubString(0,PinString.Pos(":"));

        AnsiString PinString3 = PinString2;
        PinString3 = PinString3.SubString(PinString2.Pos("=")+1,PinString2.Length());
        //Temp3 = Temp3.SubString(Temp3.Pos("=")+1,Temp3.Length());
        PinString3 = PinString3.SubString(0,PinString3.Pos("=")-1);
        ImgFileName = "pin"+AnsiString(OpId)+"_"+PinString3+".gif";
        AnsiString FName = ChangeChars(Cfg->Dirs.InterfaceDir.c_str(),"/","\\")+"\\skins\\"+(Cfg->Dirs.InterfaceSkinName+"\\i\\pin\\").c_str()+ImgFileName;
        if (!FileExists(FName))
        {
            ImgFileName="";
        }

        Res+=ImgFileName+":";
        PinString=PinString.SubString(PinString.Pos(":")+1,PinString.Length());
    }
    Res.SetLength(Res.Length()-1);
    return Res;
}

//---------------------------------------------------------------------------

__fastcall TGetCardsInfoThread::TGetCardsInfoThread(TXMLInfo *_InfoFile, TWConfig *_Cfg, TLogClass *_Log, TFileMap *_FileMap): TThread(true)
{
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    FreeOnTerminate = false;
    CS = NULL;
    CS = new TCriticalSection();
    JSMaker = NULL;
    JSMaker = new TJSMaker();
    Finished = false;
    InnerLog=false;
    if (_Log==NULL)
    {
        Log = new TLogClass("GetCardsInfoThread");
        InnerLog=true;
    }
    else
    {
        Log=_Log;
    }
    //Log->Write("TGetCardsInfoThread() started.");
    CardsInfoChanged = false;
    Cfg = _Cfg;
//    Crypt = _Crypt;
    InfoFile = _InfoFile;
    FileMap=_FileMap;
    DetailsFile = NULL;
    TimeMark=TDateTime::CurrentDateTime();
    Interval=60000; // 10 минут.
    DiffDT=double(Cfg->Payments.UpdateCardsInfo)/24/60;

    for (std::size_t i=0;i<Cfg->OperatorsInfo.size();i++)
    {
          AnsiString ProcessorType=AnsiString(Cfg->Operator(Cfg->OperatorByNum(i).Id).ProcessorType.c_str()).LowerCase();
          if (ProcessorType=="cyberplat_pin" || ProcessorType=="half_pin" || ProcessorType=="cyberplat_pin_trans")
            Cfg->SetCardsInfo(Cfg->OperatorByNum(i).Id,InfoFile->Read("CardsInfo","Op"+AnsiString(Cfg->OperatorByNum(i).Id)),InfoFile->ReadDateTime("CardsInfo","Op"+AnsiString(Cfg->OperatorByNum(i).Id)+"DT"));
    }

    Payment = NULL;
    Log->Write((boost::format("Get cards info thread initialized, interval = %1% s, diff = %2% min.") % (Interval/1000) % Cfg->Payments.UpdateCardsInfo).str().c_str());
}

__fastcall TGetCardsInfoThread::~TGetCardsInfoThread()
{
    //Log->Write("~TGetCardsInfoThread() started.");
    if (Payment!=NULL)
    {
        delete Payment;
        Payment=NULL;
    }
    if (JSMaker!=NULL)
    {
        delete JSMaker;
        JSMaker = NULL;
    }
    if (CS!=NULL)
    {
        delete CS;
        CS = NULL;
    }
    Log->Write("~TGetCardsInfoThread() done.");
    if (InnerLog)
        delete Log;
}

//---------------------------------------------------------------------------

void TGetCardsInfoThread::CheckCardsInfoForOperator(int _OpId)
{
    try
    {
        if ((AnsiString(Cfg->Operator(_OpId).DTCardsInfo.c_str()) < TDateTime::CurrentDateTime())||(FirstCheck))
        {
            if (FirstCheck)
                Log->Write((boost::format("Getting cards info for operator %1% for the first time...") % _OpId).str().c_str());
            else
                Log->Write((boost::format("Cards info for operator %1% is out of date, trying to get new one...") % _OpId).str().c_str());
            GetNewCardsInfo(_OpId);
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

int TGetCardsInfoThread::GetCards(int _OpId)
{
    TStringList* CardsCount = NULL;
    TPinPayment* Payment = NULL;
    int iRes = 0;
    try
    {
        try
        {
            CardsCount = new TStringList();
            CardsCount->Delimiter=';';
            CardsCount->DelimitedText = ChangeChars(Cfg->Operator(_OpId).MaxSavedCardsCount.c_str()," ","");
            AnsiString Cards = Cfg->Operator(_OpId).ACardsInfo.c_str();
            if (Cards!="")
            {
                Cards+=":";
                while (Cards.Pos(":"))
                {
                    AnsiString CardInfo = Cards.SubString(0,Cards.Pos(":")-1);
                    AnsiString CardName = CardInfo.SubString(0,CardInfo.Pos("=")-1);
                    AnsiString CardId = CardInfo.SubString(CardInfo.Pos("=")+1,CardInfo.Length());
                    AnsiString CardSum = CardId.SubString(CardId.Pos("=")+1,CardId.Length());;
                    CardId = CardId.SubString(0,CardId.Pos("=")-1);
                    CardSum = CardSum.SubString(0,CardSum.Pos("=")-1);
                    AnsiString Count = CardsCount->Values[CardId];
                    int iCount = GetInt(Count);
                    Log->Write((boost::format("Count of %1%: %2%") % CardId.c_str() % iCount).str().c_str());
                    if (iCount)
                    {
                        int FCount = FileCount((Cfg->Dirs.PaymentsOutbound+"\\out-pin_").c_str()+AnsiString(_OpId)+"_"+CardId+"_*.pin");
                        int FIPCount = FileCount((Cfg->Dirs.PaymentsOutbound+"\\out-pin_").c_str()+AnsiString(_OpId)+"_"+CardId+"_*.get");
                        Log->Write((boost::format("FCount of %1%: %2%") % CardId.c_str() % FCount).str().c_str());
                        Log->Write((boost::format("FIPCount of %1%: %2%") % CardId.c_str() % FIPCount).str().c_str());
                        if (GetDouble(CardSum)>0)
                        {
                            for (int i=0;i<iCount-FCount-FIPCount;i++)
                            {
                              try
                              {
                                  Payment = new TPinPayment("", Cfg, Log, FileMap, InfoFile);
                                  AnsiString Temp = Payment->AutoGetCard(_OpId, CardId, CardName, CardSum, CardInfo);
                                  Log->Write((boost::format("Res: %1%") % Temp.c_str()).str().c_str());
                                  if ((Payment->CheckErrorCode==21)||(Payment->PaymentErrorCode==22))
                                  {
                                    iRes = 1;
                                    throw Exception("Account error!");
                                  }
                              }
                              __finally
                              {
                                  if (Payment)
                                  {
                                      delete Payment;
                                      Payment = NULL;
                                  }
                              }
                            }
                        }
                    }
                    Cards = Cards.SubString(Cards.Pos(":")+1,Cards.Length());
                }
            }
        }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        }
    }
    __finally
    {
        if (CardsCount)
            delete CardsCount;
    }
    return iRes;
}

//---------------------------------------------------------------------------

AnsiString TGetCardsInfoThread::GetNewCardsInfo(int _OpId)
{
AnsiString AResult;
try
	{
	try
		{
		//Log->Write("GetNewCardsInfo("+AnsiString(_OpId)+") started.");
		if (Payment!=NULL) {
			delete Payment;
			Payment=NULL;
			}
		Payment = new TPinPayment("", Cfg, Log, FileMap, InfoFile);
		Payment->Recepient=_OpId;
		AResult = Payment->GetCardsInfo(_OpId);
		TDateTime NextTryDT;
		if (Payment->TransportError)
			{
			NextTryDT = TDateTime::CurrentDateTime()+float(10)/24/60;
      Cfg->SetCardsInfo(_OpId, Cfg->Operator(_OpId).ACardsInfo.c_str(), NextTryDT);
			}
			else
			{
			NextTryDT = TDateTime::CurrentDateTime()+DiffDT;
			if (AResult=="NULL")
				{
				Cfg->SetCardsInfo(_OpId, Cfg->Operator(_OpId).ACardsInfo.c_str(), NextTryDT);
				}
				else
				{
                                                                                                                      				Cfg->SetCardsInfo(_OpId, AResult, NextTryDT);
				InfoFile->Write("CardsInfo","Op"+AnsiString(_OpId),AResult);
				//InfoFile->Write("CardsInfo","Op"+AnsiString(_OpId)+"DT",AnsiString(TDateTime::CurrentDateTime()));
				CardsInfoChanged = true;
				}
			}

		Log->Write((boost::format("Next try: %1%") % AnsiString(NextTryDT).c_str()).str().c_str());
		InfoFile->Write("CardsInfo","Op"+AnsiString(_OpId)+"DT",AnsiString(NextTryDT));

		}
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		}
	}
__finally
	{
	if (Payment!=NULL) {
		delete Payment;
		Payment=NULL;
		}
		//Log->Write("GetNewCardsInfo("+AnsiString(_OpId)+") done.");
	return AResult;
	}
}

//---------------------------------------------------------------------------

TDateTime TGetCardsInfoThread::ReadThreadTimeMark()
{
TDateTime DT;
CS->Acquire();
try
	{
	DT = _TimeMark;
	}
__finally
	{
	CS->Release();
	}
return DT;
}

//---------------------------------------------------------------------------

void TGetCardsInfoThread::WriteThreadTimeMark(TDateTime _Src)
{
CS->Acquire();
try
	{
	_TimeMark=_Src;
	}
__finally
	{
	CS->Release();
	}
}

//---------------------------------------------------------------------------


bool TGetCardsInfoThread::StoreFile(AnsiString FileName, AnsiString Content)
{
bool bRes = false;
try
  {
  try
    {
    //Log->Write(Content);
    if (DetailsFile)
      {
      delete DetailsFile;
      DetailsFile = NULL;
      }
    DeleteFile(FileName);
    //MetroMenuFile = new TFileStream(".\\interface\\details.js");
    DetailsFile = new TFileStream(FileName, fmCreate);
    if (DetailsFile)
      {
      delete DetailsFile;
      DetailsFile = NULL;
      }
    DetailsFile = new TFileStream(FileName, fmOpenReadWrite | fmShareDenyNone);
    DetailsFile->Size = 0;
    DetailsFile->Seek(0, soFromBeginning);
    DetailsFile->Write(Content.c_str(), Content.Length());
    }
  __finally
    {
    if (DetailsFile)
      {
      delete DetailsFile;
      DetailsFile = NULL;
      }
    bRes=true;
    }
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Log->Write((boost::format("Exception occured while saving %1% file") % FileName.c_str()).str().c_str());
    }
    return bRes;
}

void TGetCardsInfoThread::CheckForIncompletePayments()
{
try
  {
  TSearchRec sr;
  int iAttributes = 0;
  if (FindFirst((Cfg->Dirs.PaymentsOutbound+"\\out-pin_*.get").c_str(), iAttributes, sr) == 0) {
    do
      {
      try
        {
        try
          {
          Payment = new TPinPayment((Cfg->Dirs.PaymentsOutbound+"\\").c_str()+sr.Name, Cfg, Log, FileMap, InfoFile);
          AnsiString Res = Payment->CompletePayment();
          }
          catch(...)
          {
              ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
              Log->Write((boost::format("Error processing %1%") % sr.Name.c_str()).str().c_str());
          }
        }
      __finally
        {
        if (Payment)
          {
          delete Payment;
          Payment = NULL;
          }
        }
      } while (FindNext(sr) == 0);
    FindClose(sr);
    }
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
}

AnsiString TGetCardsInfoThread::GetCardsString(int OpId, AnsiString InfoFromDetails)
{
AnsiString Res;
TStringList *SL = NULL;
try
  {
  try
    {
    SL = new TStringList();
    AnsiString CardList;
    TSearchRec sr;
    int iAttributes = 0;
    if (FindFirst((Cfg->Dirs.PaymentsOutbound+"\\out-pin_").c_str()+AnsiString(OpId)+"*.pin", iAttributes, sr) == 0)
      {
      do
        {
        try
          {
          try
            {
            AnsiString CardId = sr.Name;
            AnsiString NamePrefix = "out-pin_"+AnsiString(OpId)+"_";
            CardId = CardId.SubString(CardId.Pos(NamePrefix)+NamePrefix.Length(),CardId.Length());
            CardId = CardId.SubString(0,CardId.Pos("_")-1);
            Log->Write((boost::format("Got payment CardId: %1%") % CardId.c_str()).str().c_str());
            if (SL->IndexOf(CardId)==-1)
              {
              SL->Add(CardId);
              Payment = new TPinPayment((Cfg->Dirs.PaymentsOutbound+"\\").c_str()+sr.Name, Cfg, Log, FileMap, InfoFile);
              Res += Payment->GetPaymentData(OpId, CardId)+":";
              Log->Write((boost::format("Got payment data: %1%") % Res.c_str()).str().c_str());
              }
              else
              Log->Append(" Already processed.");
            }
            catch(...)
            {
                ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            }
          }
        __finally
          {
          if (Payment)
            {
            delete Payment;
            Payment = NULL;
            }
          }
        } while (FindNext(sr) == 0);
      FindClose(sr);
      }
    Res = Res.SetLength(Res.Length()-1);
    Res = MakeFullCardString(InfoFromDetails, Res);
    }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        }
  }
__finally
  {
  if (SL)
    delete SL;
  }
return Res;
}

AnsiString TGetCardsInfoThread::MakeFullCardString(AnsiString FromDetails, AnsiString FromFiles)
{
AnsiString ARes;
TStringList *SL = NULL;
try
	{
	try
		{
    SL = new TStringList();
    Log->Write((boost::format("FromDetails: %1%") % FromDetails.c_str()).str().c_str());
    Log->Write((boost::format("FromFiles: %1%") % FromFiles.c_str()).str().c_str());

    AnsiString AllCards = FromFiles + ":" + FromDetails+":";
    while (AllCards.Pos(":"))
      {
      AnsiString CardInfo = AllCards.SubString(0,AllCards.Pos(":")-1);
      AnsiString CardName = CardInfo.SubString(0,CardInfo.Pos("=")-1);
      AnsiString CardId = CardInfo.SubString(CardInfo.Pos("=")+1,CardInfo.Length());
      CardId = CardId.SubString(0,CardId.Pos("=")-1);

      if ((SL->IndexOf(CardId)==-1)&&(CardId!="0"))
        {
        SL->Add(CardId);
        ARes += CardInfo + ":";
        Log->Write((boost::format("%1% copied...") % CardInfo.c_str()).str().c_str());
        }
        else
        Log->Append(" Already processed.");

      AllCards = AllCards.SubString(AllCards.Pos(":")+1,AllCards.Length());
      }
    ARes = ARes.SetLength(ARes.Length()-1);
    Log->Write((boost::format("ARes: %1%") % ARes.c_str()).str().c_str());
		}
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		}
	}
__finally
	{
  if (SL)
    delete SL;
	}
return ARes;
}

#pragma package(smart_init)

