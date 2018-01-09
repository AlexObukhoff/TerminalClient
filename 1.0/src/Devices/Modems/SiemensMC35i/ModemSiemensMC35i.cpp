//---------------------------------------------------------------------------
#pragma hdrstop
#include <string>
#include "ModemSiemensMC35i.h"
#include "DeviceThread.h"
#include "globals.h"
#include "boost/format.hpp"
#pragma package(smart_init)
#define BufferSize 500

using namespace std;

CModemSiemensMC35i::CModemSiemensMC35i(int ComPort,TLogClass* _Log, AnsiString Prefix) : CModem(ComPort, _Log, Prefix)
{
    DataLength = 1;
    COMParameters->Parity = NOPARITY;
    COMParameters->BaudRate = 115200;
    LoggingErrors = false;
    DeviceName = "ModemSiemensMC35i";
    InitModem();

    OperatorNames = new TStringList();
		OperatorIDs = new TList();
}
CModemSiemensMC35i::~CModemSiemensMC35i()
{
		if (OperatorNames)
				delete OperatorNames;
		if (OperatorIDs)
				delete OperatorIDs;
}

float CModemSiemensMC35i::SignalQuality()
{
float fRes = 0;
try
	{
	AnsiString ReturnString, SignalString;

	if(!SendATCommand((BYTE *)"+CSQ"))
		{
		Log->Write("Send command error!");
		return 0;
		}
	ReturnString = (GetAnswerBuffer(20, "OK")).c_str();
	if (!ReturnString.Pos("CSQ:"))
		{
 		Log->Write((boost::format("Error getting modem signal quality info: %1%.") % ReturnString.c_str()).str().c_str());
		return -1;
		}
	SignalString = ReturnString.SubString(ReturnString.Pos("CSQ:")+5,5);
	try
		{
		try
			{
			fRes = ChangeChars(SignalString, ".", ",").ToDouble();
			}
		catch (...)
			{
			fRes = ChangeChars(SignalString, ",", ".").ToDouble();
			}
		}
	catch (...)
		{
                ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		Log->Write((boost::format("Error getting modem signal quality info: %1%.") % ReturnString.c_str()).str().c_str());
		return -1;
		}

	    Log->Write((boost::format("Modem Signal Quality = %1%%%") % ( 100 - fRes)).str().c_str());
		return (100 - fRes);
	}
    catch (...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
	}
    return -1;
}

AnsiString CModemSiemensMC35i::FindBalance(AnsiString Src)
{
AnsiString Res = "-1";
bool bDigitAlreadyFound = false;
int BBegin = 0;
int BEnd = 0;
try
  {
  for (int i=3;i<Src.Length();i++)
    {
    //Log->Write("i: "+AnsiString(i)+", bDigitAlreadyFound: "+AnsiString(int(bDigitAlreadyFound))+", char: "+AnsiString(Src.c_str()[i-1])+", BBegin: "+AnsiString(BBegin)+", BEnd: "+AnsiString(BEnd)+", Src: " + Src.SubString(BBegin,BEnd-BBegin+1));
//    char Char = Src.c_str()[i];
    if (isdigit(Src.c_str()[i-1]))
      {
      if (bDigitAlreadyFound)
        {
        BEnd++;
        }
        else
        {
        bDigitAlreadyFound = true;
        BBegin = BEnd = i;
        }
      }
      else
      {
      if (BBegin!=0)
        {
        if ((Src.c_str()[i-1]=='.')||(Src.c_str()[i-1]==','))
          BEnd++;
          else
          {
          if (Src.c_str()[i-1]==':')
            {
            BBegin=BEnd=0;
            bDigitAlreadyFound = false;
            }
            else
            {
            //Log->Write(AnsiString(Src.c_str()[BBegin-3])+" "+Src.SubString(BBegin-1,1)+" "+AnsiString(isdigit(Src.c_str()[BBegin-3])));
            if ((Src.SubString(BBegin-1,1)=="(")||((Src.SubString(BBegin-1,1)==":")&&(isdigit(Src.c_str()[BBegin-3]))))
              {
              BBegin=BEnd=0;
              bDigitAlreadyFound = false;
              }
              else
              {
              if (IsDouble(Src.SubString(BBegin,BEnd-BBegin+1)))
                {
                Res = Src.SubString(BBegin,BEnd-BBegin+1);
                break;
                }
                else
                {
                BBegin=BEnd=0;
                bDigitAlreadyFound = false;
                }
              }
            }
          }
        }
      }
    }
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
return Res;
}

double CModemSiemensMC35i::GetBalance(AnsiString BalanceNumber, int RepeatRequest)
{
try
  {
  string sss("");

  int Repeat = 0;

      while(Repeat < RepeatRequest)
      {
              if( !SendATCommand((BYTE *)BalanceNumber.c_str(), CALL_COMMAND) )
              {
                  Log->Write("Command send error!");
                  return -1;
              }

              sss = GetAnswerBuffer(50, NULL, false);
              if(BalanceNumber.Pos("+7"))    // Звонок на +7...
              {
                  Log->Write((boost::format("---->>> Outgoing call %1%...") % BalanceNumber.c_str()).str().c_str());
                  break;
              }
              else
              {
                  int iCtrl = sss.find("OK");
                  if(iCtrl != string::npos)
                  {
                      Log->Write("Ballance query executed. Answer received...");
                      int cusd_pos = sss.find("+CUSD:");
                      if(cusd_pos == string::npos)
                      {
                          Repeat++;
                          Log->Write((boost::format("Ballance query executed. Answer not received. [try № %1%]") % Repeat).str().c_str());
                          if(Repeat < RepeatRequest)	Sleep(7000);
                      }
                      else
                      {
                      break;
                      }
                  }
                  else
                  {
                      Repeat++;
                      Log->Write((boost::format("Ballance query executed. No access! [try № %1%]") % Repeat).str().c_str());
                      if(Repeat < RepeatRequest)    Sleep(5000);
                  }
              }
      }
  AnsiString Src = AnsiString(sss.c_str());
  AnsiString Res;

  Src = DecodeString(Src);
  Res = FindBalance(Src);
  Log->Write((boost::format("Src: %1%, Res: %2%.") % Src.c_str() % Res.c_str()).str().c_str());

/*  Src = "+CUSD: 2,\"29.05.2007 03:46:34 OCTATOK: 420.08 RUB, MF-Bonus:74.4\",15";
  Src = DecodeString(Src);
  Res = FindBalance(Src);
  Log->Write("Src:" + Src + ", Res: "+Res+".");
  Src = "+CUSD: 2,\"Dolg 501 tenge Istekaet 30.06\",15";
  Src = DecodeString(Src);
  Res = FindBalance(Src);
  Log->Write("Src:" + Src + ", Res: "+Res+".");
  Src = "+CUSD: 2,\"Vash balans = 443,45\",15";
  Src = DecodeString(Src);
  Res = FindBalance(Src);
  Log->Write("Src:" + Src + ", Res: "+Res+".");
  Src = "+CUSD: 2,\"Vash balans = 443,45rub\",15";
  Src = DecodeString(Src);
  Res = FindBalance(Src);
  Log->Write("Src:" + Src + ", Res: "+Res+".");
  Src = "+CUSD: 2,\"Vash balans:413,46 rub\",1";
  Src = DecodeString(Src);
  Res = FindBalance(Src);
  Log->Write("Src:" + Src + ", Res: "+Res+".");
  Src = "+CUSD: 2,\"04110430043B0430043D0441002000330033002C00370020044004430431002E00200020002A002A0020002A0031003100310023003E04250438044200200434043D044F003E0412043B044E0431043B0435043D043D044B043C002D0432044B043804330440043004390020044204430440002004320020041F04400430043304430021\",72";
  Src = DecodeString(Src);
  Res = FindBalance(Src);
  Log->Write("Src:" + Src + ", Res: "+Res+".");
  Src = "+CUSD: 2,\"04110430043B0430043D0441002000310034002E003200340440002E00200411043E043D0443044100200030002E003000300440002E002004210435043A0443043D043400200030002E003000200414043E043F002E002004310430043B0430043D0441044B003A0020002A0031003000360023\",72";
  Src = DecodeString(Src);
  Res = FindBalance(Src);
  Log->Write("Src:" + Src + ", Res: "+Res+".");*/

  /*Src = "+CUSD: 2,\"04110430043B0430043D0441002000320037002E003400340440002E00200411043E043D0443044100200030002E003000300440002E002004210435043A0443043D043400200030002E003000200414043E043F002E002004310430043B0430043D0441044B003A0020002A0031003000360023\",72";
  Src = DecodeString(Src);
  Res = FindBalance(Src);
  Log->Write("Src:" + Src + ", Res: "+Res+".");*/

/*  Src = "+CUSD: 2,\"58,20 rub.Srok dejstviya platezha ne ogranichen. * Obshhajsya i poluchaj prizy' za bally'-Megafon Bonus. Podr.0510\",1";
  Src = DecodeString(Src);
  Res = FindBalance(Src);
  Log->Write("Src:" + Src + ", Res: "+Res+".");*/

  return atof(Res.c_str());
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
  return -1;
}

bool CModemSiemensMC35i::IsItYou()
{
	try
	{
			AnsiString Type = GetModemType();
			if (Log)
					Log->Write((boost::format("CModemSiemensMC35i::IsItYou() found type: %1%") % Type.c_str()).str().c_str());
			std::string Temp(Type.c_str());
			if(Temp.find("MC35i") != string::npos)
        return true;
      return false;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
  return false;
}

float  CModemSiemensMC35i::SignalLevel()
{
    string Signal, Temp;
    AnsiString SignalLevelDescription = "";

    Log->Write(">>> SignalLevel...");
    if(!SendATCommand((BYTE *)"^MONI"))
    {
        Log->Write("Wrong COM port!");
        return -1;
    }

    //if (DeviceState->AnswerSize > 0)
    //{
    string sss("");
    sss = GetAnswerBuffer(20, "Serving Cell");
    bool bFindNeedWord = false;
    int i = 0;
    int f = sss.find("ChMod");
    if(f != string::npos)
    {
        while((f+i) != (int)sss.size())
        {
            Temp.assign(sss, f+i, 1);
            if(isdigit(int(Temp[0])))
            {
                if(!bFindNeedWord)
                {
                    bFindNeedWord = true;
                    f = sss.find(" ", f+i);
                    i = 0;
                }
                else
                {
                    Temp.assign(sss, f+i, 2);
                    Signal.assign(Temp);
                    break;
                }
            }
            i++;
        }
    }
    //}

    if(atof(Signal.c_str()) > 0)
    {
        Log->Write((boost::format("Modem Signal Level = %1%%%") % (100 * atof(Signal.c_str()) / 63)).str().c_str());
        return (100*atof(Signal.c_str()))/63;
    }
    else
    {
        Log->Write("Modem Signal Level = ERROR");
        return 0;
    }
}

void CModemSiemensMC35i::GetOperators()
{
    try
    {
        OperatorNames->Clear();
        OperatorIDs->Clear();

        AnsiString OperatorName = "";
        int OperatorID = 0;

        if (Log)
            Log->Write(">>> GetOperators...");
        if(!SendATCommand((BYTE *)"+COPN",AT_COMMAND))
        {
            if (Log)
                Log->Write("Wrong COM port!");
            return;
        }

        string str("");
        str = GetAnswerBuffer(20, "OK");
        AnsiString Answer = str.c_str();
        if (Log)
            Log->Write((boost::format("Recieved answer: %1%") % Answer.c_str()).str().c_str());
        int index = Answer.Pos(":");
        while(index != 0)
        {
            GetOperatorItems(Answer,index+2,OperatorID,OperatorName);
            OperatorNames->Add(OperatorName);
            OperatorIDs->Add((void*)OperatorID);
            if (Log)
                Log->Write((boost::format("Found operator with ID = %1%; Name = %2%") % OperatorID % OperatorName.c_str()).str().c_str());
            Answer.Delete(index,1);
            index = Answer.Pos(":");
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void CModemSiemensMC35i::GetOperatorItems(AnsiString Answer, int index, int& OperatorID, AnsiString& OperatorName)
{
    UNREFERENCED_PARAMETER(OperatorName);
    try
    {
        int Length =  Answer.Length();
        AnsiString Text = Answer.SubString(index,Length-index+1);
        //take operator id
        int DelimiterIndex = Text.Pos(",");
        AnsiString OperatorID_str = Text.SubString(index,DelimiterIndex-index);
        OperatorID = StrToInt(OperatorID_str);
        //take operator name
        Text.Delete(DelimiterIndex+2,1);
        int DelimiterIndex_1 = Text.Pos("\"");
        AnsiString OperatorName = Text.SubString(DelimiterIndex+2,DelimiterIndex+2-DelimiterIndex_1);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

AnsiString CModemSiemensMC35i::GetOperatorName(int ID)
{
    try
    {
        AnsiString result = "";
        OperatorNames->Clear();
        OperatorIDs->Clear();
        AnsiString OperatorName = "";
        int OperatorID = ID;

        if (Log)
            Log->Write(">>> GetOperatorName...");
        AnsiString command = "+WOPN=0,"+AnsiString(ID);
        if(!SendATCommand((BYTE *)command.c_str(),AT_COMMAND))
        {
            if (Log)
                Log->Write("Wrong COM port!");
            return result;
        }

        string str("");
        str = GetAnswerBuffer(20, "OK");
        AnsiString Answer = str.c_str();
        if (Log)
            Log->Write((boost::format("Recieved answer: %1%") % Answer.c_str()).str().c_str());
        int index = Answer.Pos(":");
        GetOperatorItems(Answer,index+2,OperatorID,OperatorName);
        OperatorNames->Add(OperatorName);
        OperatorIDs->Add((void*)OperatorID);
        if (Log)
            Log->Write((boost::format("Found operator with ID = %1%; Name = %2%") % OperatorID % OperatorName.c_str()).str().c_str());
        return result = OperatorName;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return "";
}

AnsiString CModemSiemensMC35i::GetOperatorName()
{
    try
    {
				AnsiString OperatorName = "";

				if (Log)
						Log->Write(">>> GetOperatorName...");
				AnsiString command = "+COPS?";
				if(!SendATCommand((BYTE *)command.c_str(),AT_COMMAND))
				{
						if (Log)
								Log->Write("Wrong COM port!");
						return "";
				}

				string str("");
				str = GetAnswerBuffer(20, "OK");
				AnsiString Answer = str.c_str();
				if (Log)
						Log->Write((boost::format("Recieved answer: %1%") % Answer.c_str()).str().c_str());
				int index = Answer.Pos("\"");
				Answer.Delete(index,1);
				int index_2 = Answer.Pos("\"");
				OperatorName = Answer.SubString(index,index_2-index);
				if (Log)
						Log->Write((boost::format("Found operator with name %1%") % OperatorName.c_str()).str().c_str());
				return OperatorName;
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

AnsiString CModemSiemensMC35i::DecodeString(AnsiString Src)
{
char* Chr;
AnsiString Res;
try
	{
	try
		{
		AnsiString Prefix = Src.SubString(0, Src.Pos("\""));
		AnsiString Temp = Src.SubString(Src.Pos("\"")+1, Src.Length());
		AnsiString Postfix = Temp.SubString(Temp.Pos("\""), Temp.Length());
		AnsiString Coding = Temp.SubString(Temp.Pos("\",")+2, Temp.Length());
		Temp= Temp.SubString(0,Temp.Pos("\"")-1);
    Res = Temp;

		WideString Dest;
		Chr = new char[Temp.Length() + 1 ];
		strcpy( Chr, Temp.c_str() );
    Log->Write((boost::format("Coding = %1%") % Coding.c_str()).str().c_str());
		if (Coding.Pos("72")>0)
			{
			WideChar cTemp;
			for(int x = 3, i = 0; i<int(Temp.Length()); i++)
        {
        if(x == 3)
          {
          x = 2;
          cTemp = (Chr[i] < 'A' ? Chr[i]-'0' : Chr[i]-'0'-7)*4096;
          }
          else
          {
          if(x == 2)
            {
            x = 1;
            cTemp += (Chr[i] < 'A' ? Chr[i]-'0' : Chr[i]-'0'-7)*256;
            }
            else
            {
            if(x == 1)
              {
              x = 0;
              cTemp += (Chr[i] < 'A' ? Chr[i]-'0' : Chr[i]-'0'-7)*16;
              }
              else
              {
              x = 3;
              cTemp += (Chr[i] < 'A' ? Chr[i]-'0' : Chr[i]-'0'-7);
              Dest+=WideString(cTemp);
              }
            }
          }
        }
			Res = AnsiString(Dest);
			}
		}
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		}
	}
__finally
	{
	delete[] Chr;
	return AnsiString("  ")+Res+AnsiString("  ");
	}
}


