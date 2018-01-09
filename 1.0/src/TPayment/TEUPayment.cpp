//---------------------------------------------------------------------------


#pragma hdrstop

#include "TEUPayment.h"
#include "TEUConnectThread.h"
#include "globals.h"
#include "boost/format.hpp"

//---------------------------------------------------------------------------

#pragma package(smart_init)

//---------------------------------------------------------------------------

TEUPayment::TEUPayment(AnsiString _fileName, TWConfig *_Cfg, TLogClass *_Log, TFileMap* _FileMap, TXMLInfo* _InfoFile)
: TPinPayment(_fileName, _Cfg, _Log, _FileMap, _InfoFile)
{
}


//---------------------------------------------------------------------------

void TEUPayment::ParseLocation(AnsiString LocationString)
{
	try
	{

    std::auto_ptr <TLocationParser> Location ( new TLocationParser(LocationString.c_str()));

    if (Location->HasParameter("recepient"))
  		Recepient=GetInt(Location->GetParameter("recepient").c_str());
    else
    {
 			Log->Write("Error: \"recepient\" parameter missing in location string!");
			return;
    }

		std::auto_ptr <TStringList> FieldForChequeList ( new TStringList() );
		std::auto_ptr <TStringList> UnnamedFieldForChequeList ( new TStringList() );

		CardID = Location->GetParameter("field100").c_str();
		CardName = "";
		CardValue = 0;

		XMLP->AddParam("NUMBER",CardID);

		FieldForChequeList->Add("Номер корзины="+CardID);

		for (unsigned int i=0; i<Cfg->Operator(Recepient).Fields.size();i++)
    {
			if (Cfg->Operator(Recepient).Fields[i].Type != "secretword")
				if (Cfg->Operator(Recepient).Fields[i].Type == "enum")
				{
					FieldForChequeList->Add((Cfg->Operator(Recepient).Fields[i].Name+"=").c_str()+Cfg->Operator(Recepient).Fields[i].GetEnumText(Location->GetParameter(("field"+Cfg->Operator(Recepient).Fields[i].Id).c_str()).c_str()));
					UnnamedFieldForChequeList->Add(("FIELD"+Cfg->Operator(Recepient).Fields[i].Id+"=").c_str()+Cfg->Operator(Recepient).Fields[i].GetEnumText(Location->GetParameter(("field"+Cfg->Operator(Recepient).Fields[i].Id).c_str()).c_str()));
				}
				else
				{
					FieldForChequeList->Add(((Cfg->Operator(Recepient).Fields[i].Name+"=")+Location->GetParameter(("field"+Cfg->Operator(Recepient).Fields[i].Id).c_str())).c_str());
					UnnamedFieldForChequeList->Add(("FIELD"+Cfg->Operator(Recepient).Fields[i].Id+"="+Location->GetParameter(("field"+Cfg->Operator(Recepient).Fields[i].Id).c_str())).c_str());
				}
		}

		AFieldsForInterface = Location->GetParameters().c_str();
		AFieldsForCheque = FieldForChequeList->DelimitedText;
		AUnnamedFieldsForCheque = UnnamedFieldForChequeList->DelimitedText;

  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

AnsiString TEUPayment::GetMessageTextForCheck(bool bFirstCheck)
{
UNREFERENCED_PARAMETER(bFirstCheck);
return "";
}

//---------------------------------------------------------------------------

AnsiString TEUPayment::GetMessageTextForPayment()
{
return "";
}

//---------------------------------------------------------------------------

AnsiString TEUPayment::GetMessageText(AnsiString SessionNumber)
{
return SessionNumber;
}

//---------------------------------------------------------------------------

void TEUPayment::PostProcessFirstCheck(int _ResultCode, int _ErrorCode)
{
try
	{
	if ((_ResultCode==0)&&(_ErrorCode==0))
    {
    AnsiString Amount = CheckResult.SubString(CheckResult.Pos("AMOUNT=")+7,CheckResult.Length());
    //Amount = Amount.SubString(0, Amount.Pos("\r"));
    ForcedSum = GetInt(Amount);
    CardValue = ForcedSum;
    
		}
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        ForcedSum = -1;
    }
}

//---------------------------------------------------------------------------

AnsiString TEUPayment::Connect(AnsiString URL, bool bProcessMessages)
{
AnsiString Result="";
		try
				{
				TransportError = false;

				if ((!XMLP)||(!PS))
						return "";
				AnsiString SignedMessage=AnsiString(CardID)+"|"+MessageText;
				if (SignedMessage=="")
					{
          return "ERROR=-2";
					}
				if (SignedMessage!="")
					{
					std::auto_ptr <TEUConnectThread> EUCon ( new TEUConnectThread(Log, Cfg, URL, SignedMessage) );
					if (!EUCon.get()) {
						Log->Write("  TEUConnectThread creating Error. ");
						}
						else
						{
						EUCon->Resume();
						int TimeOut=6000;
						while ((TimeOut>0)&&(!EUCon->Finished)) {
								if (bProcessMessages)
									 Application->ProcessMessages();
								Sleep(10);
								TimeOut--;
								}

						IndyError = EUCon->IndyError;

						if (TimeOut<=0) {
								Log->Append("Timed out.");
								TerminateThread((HANDLE)EUCon->Handle,0);
								}
								else {
								if ((EUCon->Finished)&&(EUCon->AnswerMessage!=""))
                  {
									Result = EUCon->AnswerMessage;
					        Log->Write((boost::format("Result = %1%") % Result.c_str()).str().c_str());
									}
								}
						ConnectResult = EUCon->ConnectResult;
						TransportError = EUCon->TransportError;

            if ((Result!="")&&(URL=="check"))
              {
              CheckResult = Result;
              }
						}
          }
				}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return Result;
}

//---------------------------------------------------------------------------

AnsiString TEUPayment::GetErrorDescr(int val)
{
  AnsiString res = "";
  switch (val)
  {
    case 1101:
			res = "Корзина не найдена";
      break;
    case 1102:
      res = "Корзина уже оплачена";
      break;
    case 1104:
      res = "Ошибка №4";
      break;
		default:
			res = "Неизвестная ошибка #"+AnsiString(val);
		}
	return res;
}


