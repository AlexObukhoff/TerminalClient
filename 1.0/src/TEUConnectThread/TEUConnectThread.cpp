//---------------------------------------------------------------------------

#include <Classes.hpp>
#include <vcl.h>
#pragma hdrstop

#include "TEUConnectThread.h"
#include "globals.h"
#include "boost/format.hpp"
#pragma package(smart_init)
//---------------------------------------------------------------------------

//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall TConnectThread::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

__fastcall TEUConnectThread::TEUConnectThread(TLogClass *_Log, TWConfig *_Cfg, AnsiString _URL, AnsiString _SignedMessage)
                : TConnectThread(_Log, _Cfg, _URL, _SignedMessage)
{
  ESL = NULL;
}

//---------------------------------------------------------------------------

__fastcall TEUConnectThread::~TEUConnectThread(void)
{
if (ESL)
  delete ESL;
}

//---------------------------------------------------------------------------

void __fastcall TEUConnectThread::Execute()
{
try
        {
        Process();
        Finished=true;
        }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TEUConnectThread::Process()
{
//Log->Write("Message: "+PrepareString(SignedMessage));
AnsiString Result = "";
bool bOK=true;
try
  {
  try
    {
    TransportError = false;
    ConnectResult = "";

    if (!Terminated)
      {
      ESL = new TEvroSetLib(Log);
      //connect
      }

    if(!ESL)
      {
      Log->Write("TEUConnectThread::Process: Error creating ESL.");
      bOK=false;
      ConnectResult += "\r\nError creating ESL.";
      }

    if ((bOK)&&(!Terminated))
      {
      //Log->Write("   Posting data to "+URL+"...");
      int conres = ESL->Connect();
      if (conres==0)
        {
        bOK = true;
        Log->Append("OK");
        ConnectResult += "\r\nConnected...";
        ConnectOK = true;
        }
        else
        {
        Log->Append((boost::format(" Connect error: %1%") % conres).str().c_str());
        bOK=false;
        ConnectResult += "\r\nConnect error: "+AnsiString(conres);
        TransportError = true;
        Result = "RESULT=1\r\nERROR="+AnsiString((conres==0 ? 0 : conres+1100))+"\r\nDATE="+AnsiString(TDateTime::CurrentDateTime())+"\r\n";
        }
      }

    if ((bOK)&&(!Terminated))
      {
      //Log->Write("   Posting data to "+URL+"...");
      int pcres = -1;
      if (URL=="check")
        {
        long long BasketNum;
        try
          {
          BasketNum = StrToInt64(SignedMessage.SubString(0,SignedMessage.Pos("|")-1));
          }
          catch (...)
          {
          Log->Write((boost::format("Error converting %1% to int64!") % SignedMessage.c_str()).str().c_str());
          }
        AnsiString Session = SignedMessage.SubString(SignedMessage.Pos("|")+1,SignedMessage.Length());
        double BasketAmount;
        pcres = ESL->GetAmount(BasketNum, BasketAmount,Session);
        if (pcres == 0)
          {
          Result = "ERROR=0\r\nRESULT=0\r\nDATE="+AnsiString(TDateTime::CurrentDateTime())+"\r\nAMOUNT="+AnsiString(BasketAmount);
          }
          else
          {
          Result = "RESULT=1\r\nERROR="+AnsiString((pcres==0 ? 0 : pcres+1100))+"\r\nDATE="+AnsiString(TDateTime::CurrentDateTime())+"\r\n";
          }
        }
        else
        if (URL=="pay")
          {
          long long BasketNum;
          try
            {
            BasketNum = StrToInt64(SignedMessage.SubString(0,SignedMessage.Pos("|")-1));
            }
            catch (...)
            {
            Log->Write((boost::format("Error converting %1% to int64") % SignedMessage.c_str()).str().c_str());
            }
          AnsiString Session = SignedMessage.SubString(SignedMessage.Pos("|")+1,SignedMessage.Length());
          AnsiString Pin;
          pcres = ESL->GetPin(BasketNum, Pin,Session);
          if (pcres == 0)
            {
            Result = "ERROR=0\r\nRESULT=0\r\nDATE="+AnsiString(TDateTime::CurrentDateTime())+"\r\nPIN="+Pin+"\r\n";
            }
            else
            {
            Result = "RESULT=1\r\nERROR="+AnsiString((pcres==0 ? 0 : pcres+1100))+"\r\nDATE="+AnsiString(TDateTime::CurrentDateTime())+"\r\n";
            }
          }
          else
          Log->Write((boost::format("Unknown command %1%") % URL.c_str()).str().c_str());

      if (pcres==0)
        {
        bOK = true;
        Log->Append("OK");
        ConnectResult += "\r\nPacket sent...";
        ConnectOK = true;
        }
        else
        {
        Log->Append((boost::format(" Send command error: %1%") % pcres).str().c_str());
        bOK=false;
        ConnectResult += "\r\nSend command error: "+AnsiString(pcres);
        TransportError = true;
        }
      }

    if ((bOK)&&(!Terminated))
      {
      int disconres = ESL->Disconnect();
      if (disconres==0)
        {
        bOK = true;
        Log->Append("OK");
        ConnectResult += "\r\nDisconnected...";
        }
        else
        {
        Log->Append((boost::format(" Disconnect error: %1%") % disconres).str().c_str());
        bOK=false;
        ConnectResult += "\r\Disconnect error: "+AnsiString(disconres);
        }
      }
    }
  catch(Exception& error)
    {
    Log->Write((boost::format("Exception in TEUConnectThread::Process: %1%") % error.Message.c_str()).str().c_str());
    ConnectResult += "\r\nConnect exception: "+AnsiString(error.Message);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
  }
  __finally
  {
  AnswerMessage=Result;
  if (ESL)
    {
    delete ESL;
    ESL = NULL;
    }
  Finished = true;
  }
}
