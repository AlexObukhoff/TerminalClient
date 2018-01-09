//---------------------------------------------------------------------------

#include <vcl.h>
#include <algorith.h>
#include <Classes.hpp>

#pragma hdrstop

#include "EMailSendThread.h"
#include "globals.h"
#include "boost/format.hpp"
#pragma package(smart_init)

__fastcall TEMailSendThread::TEMailSendThread(TWConfig *_Cfg, TEMailPacket *_EMailPacket, TLogClass *_Log)
		: TThread(true)
{
try
  {
  //CoInitialize(NULL);
  CoInitializeEx(NULL, COINIT_MULTITHREADED);
  Finished=false;
  InnerLog=false;
  if (_Log==NULL) {
    Log = new TLogClass("TEMailSendThread");
    InnerLog=true;
    }
    else
    Log=_Log;
  Cfg=_Cfg;
  EMailPacket=_EMailPacket;
  Sent=false;

  SMTP=NULL;
  SMTP = new TIdSMTP(NULL);
  if (Cfg->EMailInfo.UserId!="")
    SMTP->AuthenticationType = atLogin;
  SMTP->Host = Cfg->EMailInfo.SMailHost.c_str();
  SMTP->Port = 25;
  SMTP->UserId = Cfg->EMailInfo.UserId.c_str();
  SMTP->Password = Cfg->EMailInfo.Password.c_str();
  if (Cfg->Connection().HTTPProxy.Type.find("socks")!=std::string::npos)
    SMTP->SocksInfo->Assign(Cfg->Connection().Proxy);

    AnsiString strtmp=Cfg->EMailInfo.IgnoredErrorsList.c_str();
  IgnoredErrorsList = ","+ChangeChars(strtmp," ","")+",";
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

__fastcall TEMailSendThread::~TEMailSendThread(void)
{
if (SMTP!=NULL) {
	delete SMTP;
	SMTP=NULL;
	}
if (InnerLog) {
	delete Log;
	Log=NULL;
	}
}

//---------------------------------------------------------------------------

void __fastcall TEMailSendThread::Execute()
{
try
	{
	Sent=Process();
	Finished=true;
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}
//---------------------------------------------------------------------------

void TEMailSendThread::SendMessage()
{
try
		{
/*    Log->Write("  Sending CommandProcessed event {TerminalID: "+AnsiString(TerminalID)+"; CommandUID: "+AnsiString(StatPacket->Status)+"}...");
		Write(cnCommandProcessed);
		Write(TerminalID);
		Write(StatPacket->Status);*/
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------


bool TEMailSendThread::Process()
{
TIdMessage* M = NULL;
bool bResult = false;
//int iError;
try
	{
	try
		{
		Log->Write((boost::format("  Connecting to %1%:%2%...") % SMTP->Host.c_str() % SMTP->Port).str().c_str());
		SMTP->Connect();
		if(SMTP->Connected())
			{
			Log->Append((boost::format("OK. Auth. schemes supported: %1%.") % SMTP->AuthSchemesSupported->DelimitedText.c_str()).str().c_str());
			if (Cfg->EMailInfo.UserId!="")
				{
				Log->Write("  Authenticating...");
				if (SMTP->Authenticate())
					Log->Append("OK.");
					else
					Log->Append("Error!");
				}
			M = new TIdMessage(NULL);
			M->From->Text = Cfg->EMailInfo.FromAddress.c_str();
			//M->Recipients->Add();
			//M->Recipients->Items[0]->Text = Cfg->EMailInfo.ToAddress;
      M->Recipients->EMailAddresses = EMailPacket->GetParamValue("Addresses");
      if (M->Recipients->EMailAddresses =="")
        {
  			Log->Append("Address is null!");
        return true;
        }
			Log->Append((boost::format("Sending to: %1%...") % M->Recipients->EMailAddresses.c_str()).str().c_str());
			M->CharSet=" charset=windows-1251";
      M->ContentType="text/plain";
			M->Subject = EMailPacket->GetParamValue("Subject");
			M->Body->Add(EMailPacket->GetParamValue("MessageText"));
			try
				{
			  Log->Write((boost::format("Sending message to: %1%...") % M->Recipients->EMailAddresses.c_str()).str().c_str());
				SMTP->Send(M);
				Log->Append("OK.");
				bResult = true;
				}
			catch(Exception &exception)
				{
				Log->Append((boost::format("Server response: %1%!") % exception.Message.c_str()).str().c_str());
        int ErrorCode = GetInt(exception.Message.SubString(0,3));
        AnsiString CurrentError = ","+AnsiString(ErrorCode)+",";
        if ((Cfg->EMailInfo.IgnoredErrorsList!="")&&(IgnoredErrorsList.Pos(CurrentError)!=0))
          bResult = false;
          else
          bResult = true;
				}
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            bResult = false;
        }

			}
      else
      {
      Log->Append("ERROR!");
      }
		}
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		}
	}
__finally
	{
	if (SMTP->Connected())
		SMTP->Disconnect();
	if (M)
		{
		delete M;
		M = NULL;
		}
	return bResult;
	}
}
//---------------------------------------------------------------------------
