//---------------------------------------------------------------------------


//#pragma hdrstop
#include <memory>
#include "EMailSender.h"
#include "EMailSendThread.h"
#include "globals.h"
#include "boost/format.hpp"

TEMailSender::TEMailSender(AnsiString _fileName, TWConfig* _Cfg, TLogClass* _Log, TFileMap* _FileMap)
{
MailPacket = NULL;
Log = NULL;
try
	{
        AnsiString SMailHost=AnsiString(_Cfg->EMailInfo.SMailHost.c_str()).LowerCase();
	if ((SMailHost=="none")||(SMailHost==""))
	  {
		Enabled=false;
		return;
		}
	//CoInitializeEx(NULL, COINIT_MULTITHREADED);
	Cfg = _Cfg;
	FileMap = _FileMap;
	InnerLog=false;
	if (_Log==NULL)
		{
		Log = new TLogClass("EMailSender");
		InnerLog=true;
		}
		else
		Log=_Log;

	MailPacket = new TEMailPacket(Cfg, Log);
	if (_fileName!="")
		{
		MailPacket->LoadFromFile(_fileName);
		}
	Enabled=true;
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

TEMailSender::~TEMailSender()
{
if (!Enabled)
    return;
//CoUninitialize();
Clear();
if (MailPacket)
    delete MailPacket;
if (InnerLog)
    delete Log;
}

//---------------------------------------------------------------------------

void TEMailSender::Clear()
{
try
    {
    if (!Enabled)
        return;
    MailPacket->Clear();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TEMailSender::StoreMessage(AnsiString Addresses, TDateTime _EventDateTime, AnsiString _Subject, AnsiString _MessageText, AnsiString FileName)
{
try
    {
		if (!Enabled)
				return;
//		if (Cfg->EMailInfo.SendMessages)
//			{
			Clear();
			AnsiString PacketFileName;
      if (FileName=="")
        PacketFileName = MailPacket->GetNewFileName("");
        else
        PacketFileName = (Cfg->Dirs.EMailOutbound + "\\").c_str() + FileName +".";
 			MailPacket->AddParam("Addresses",Addresses);
			MailPacket->AddParam("TerminalNumber",AnsiString(Cfg->Terminal.Number.c_str()));
			MailPacket->AddParam("DateTime",AnsiString(_EventDateTime));
			MailPacket->AddParam("Subject",AnsiString(_Subject));
			MailPacket->AddParam("MessageText",AnsiString(_MessageText));
			if ((AnsiString(Cfg->EMailInfo.SMailHost.c_str()).LowerCase()!="external_sender"))
				if (!MailPacket->StoreToFile(PacketFileName+"pkt",false))
					FileMap->WriteErrorFound = true;
			MailPacket->CloseFile();
			if ((Cfg->EMailInfo.Ext)||(AnsiString(Cfg->EMailInfo.SMailHost.c_str()).LowerCase()=="external_sender"))
				{
				if (MailPacket->StoreToFile((Cfg->Dirs.EMailOutboundExt+"\\").c_str()+MailPacket->TruncateFileName(PacketFileName+"pkt"),false))
					{
					Log->Append(" Packet stored to ext folder.");
					}
					else
					{
					Log->Append(" Error storing packet to ext folder!");
					FileMap->WriteErrorFound = true;
					}
				MailPacket->CloseFile();
				}
			Log->WriteInLine((boost::format("Message stored. {TerminalID: %1%; EventDateTime: %2%; Subject: %3%; MessageText: %4%.") % MailPacket->GetParamValue("TerminalNumber").c_str() % MailPacket->GetParamValue("DateTime").c_str() % MailPacket->GetParamValue("Subject").c_str() % MailPacket->GetParamValue("MessageText").c_str()).str().c_str());
			//}
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TEMailSender::StoreHeartBeatMessage(TDateTime _EventDateTime, AnsiString _Subject, AnsiString _MessageText)
{
try
    {
    if (!Enabled)
        return;
//		if (Cfg->EMailInfo.SendMessages)
//			{
			Clear();
			MailPacket->AddParam("TerminalNumber",AnsiString(Cfg->Terminal.Number.c_str()));
			MailPacket->AddParam("DateTime",AnsiString(_EventDateTime));
			MailPacket->AddParam("Subject",AnsiString(_Subject));
			MailPacket->AddParam("MessageText",AnsiString(_MessageText));

			AnsiString FileName = (Cfg->Dirs.EMailOutboundExtTemp+"\\HeartBeat.pkt").c_str();
			MailPacket->StoreToFile(FileName,false);
			MailPacket->CloseFile();
			Log->WriteInLine((boost::format("HeartBeatMessage stored to %1%. {TerminalID: %2%; EventDateTime: %3%; Subject: %4%; MessageText: %5%.") % FileName.c_str() % MailPacket->GetParamValue("TerminalNumber").c_str() % MailPacket->GetParamValue("DateTime").c_str() % MailPacket->GetParamValue("Subject").c_str() % MailPacket->GetParamValue("MessageText").c_str()).str().c_str());
//			}
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

bool TEMailSender::Process()
{
try
    {
    if (!Enabled)
        return false;
    bool Result=false;
    std::auto_ptr <TEMailSendThread> Send ( new TEMailSendThread(Cfg, MailPacket, Log) );
    if (Send.get()) {
        Send->Resume();
        int i=0;
        while ((i<6000)&&(!Send->Finished)) {
            //Application->ProcessMessages();
            Sleep(10);
            i++;
            }
        if (i>=6000) {
            Log->Write("Terminating connection due to timeout...");
            Send->Terminate();
            }
        if (Send->Finished) {
            Result=Send->Sent;
            }
        Sleep(100);
        }
    return Result;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

#pragma package(smart_init)

