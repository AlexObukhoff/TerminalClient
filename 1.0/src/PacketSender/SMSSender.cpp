//---------------------------------------------------------------------------
#pragma package(smart_init)
#include "SMSSender.h"
#include "globals.h"
#include "boost/format.hpp"
//---------------------------------------------------------------------------

TSMSSender::TSMSSender(AnsiString _fileName, TWConfig* _Cfg, TLogClass* _Log, TFileMap* _FileMap) : TPacketSender("", _Cfg, _Log, _FileMap)
{
try
	{
  if (_Log)
    _Log->Write((boost::format("TSMSSender(%1%,%2%,%3%,%4%)") % _fileName.c_str() % (int(_Cfg)) % (int(_Log)) % (int(_FileMap))).str().c_str());
	if (_Cfg->SMSInfo.PhoneNumber == "")
	  {
		Enabled=false;
  if (_Log)
    _Log->Write((boost::format("TSMSSender done.. Enabled = %1%.") % Enabled).str().c_str());
		return;
		}
	//CoInitializeEx(NULL, COINIT_MULTITHREADED);
	//Modem = NULL;

  if (!_Log)
    {
    delete Log;
    Log = new TLogClass("SMSSender");
    }

  if (StatPacket)
    delete StatPacket;
  StatPacket = NULL;
	StatPacket = new TSMSPacket(Cfg, Log);
	if (_fileName!="")
		{
		Log->Write("  Loading packet...");
		if (StatPacket->LoadFromFile(_fileName))
			{
			Log->Append("OK.");
			}
			else
			{
			Log->Append("Error!");
			PacketLoadError = true;
			}
		}
  if (_Log)
    _Log->Write((boost::format("TSMSSender done. Enabled = %1%.") % Enabled).str().c_str());
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        PacketLoadError = true;
        Enabled=false;
    }
}
//---------------------------------------------------------------------------
TSMSSender::~TSMSSender()
{
/*if (Send!=NULL) {
				int TimeOut=6000;
				//TerminateThread((HANDLE)Send->Handle,ExitCode);
				Send->Terminate();
				while ((Send!=NULL)&&(!Send->Finished)&&(TimeOut>0)) {
								Sleep(10);
								TimeOut--;
		}
	delete Send;
	Send=NULL;
	}*/
	/*if (Modem)
		{
		delete Modem;
		Modem = NULL;
		}*/
}
//---------------------------------------------------------------------------
void TSMSSender::StorePaymentInitTemp(TDateTime _EventDateTime, int _OperatorId, AnsiString _InitialSessionNum, float _Comission, const TNotesVector& _Notes, AnsiString _Fields)
{
    UNREFERENCED_PARAMETER(_EventDateTime);
    UNREFERENCED_PARAMETER(_OperatorId);
    UNREFERENCED_PARAMETER(_InitialSessionNum);
    UNREFERENCED_PARAMETER(_Comission);
    UNREFERENCED_PARAMETER(_Notes);
    UNREFERENCED_PARAMETER(_Fields);
}
//---------------------------------------------------------------------------
void TSMSSender::StorePaymentInitComplete()
{
}
//---------------------------------------------------------------------------
void TSMSSender::StoreError(TDateTime _EventDateTime, AnsiString _Sender, int _Type, AnsiString _Description, int _SubType, AnsiString _SubDescription)
{
    UNREFERENCED_PARAMETER(_EventDateTime);
    UNREFERENCED_PARAMETER(_Sender);
    UNREFERENCED_PARAMETER(_Type);
    UNREFERENCED_PARAMETER(_Description);
    UNREFERENCED_PARAMETER(_SubType);
    UNREFERENCED_PARAMETER(_SubDescription);
}
//---------------------------------------------------------------------------
void TSMSSender::StoreMessage(AnsiString Message)
{
    try {
        if(!Enabled) {
            if(Log)
                Log->Write((boost::format("StoreMessage cancelled. Enabled = %1%.") % Enabled).str().c_str());
            return;
        }
        Clear();
        StatPacket->PacketType=cnError;
        StatPacket->ErrDescription=Message;
  //StatPacket->SaveToFile();
        if(!StatPacket->SaveToFile())
            FileMap->WriteErrorFound = true;
        StatPacket->CloseFile();
        Log->Write((boost::format("SMS Message stored to %1%.") % StatPacket->TruncateFileName(StatPacket->PacketFileName.c_str()).c_str()).str().c_str());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}
//---------------------------------------------------------------------------
void TSMSSender::SendMessage(AnsiString Message)
{
    try	{
        if(!Enabled)
		    return;
		Clear();
		StatPacket->PacketType=cnError;
		StatPacket->ErrDescription=Message;
		//StatPacket->SaveToFile();
		Process();
		Log->Write("SMS Message sent.");
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TSMSSender::StorePaymentStatusChange(TDateTime _EventDateTime, AnsiString _InitialSessionNum, int _Status, int _ErrorCode)
{
    UNREFERENCED_PARAMETER(_EventDateTime);
    UNREFERENCED_PARAMETER(_InitialSessionNum);
    UNREFERENCED_PARAMETER(_Status);
    UNREFERENCED_PARAMETER(_ErrorCode);
}

//---------------------------------------------------------------------------

void TSMSSender::StorePaymentComplete(TDateTime _EventDateTime, AnsiString _InitialSessionNum, AnsiString _LastSessionNum, int _ErrorCode, int _LastErrorCode, TDateTime _FirstTryDT, int _OperatorId, double _Sum, double _Comission, AnsiString _Fields)
{
    UNREFERENCED_PARAMETER(_EventDateTime);
    UNREFERENCED_PARAMETER(_InitialSessionNum);
    UNREFERENCED_PARAMETER(_LastSessionNum);
    UNREFERENCED_PARAMETER(_ErrorCode);
    UNREFERENCED_PARAMETER(_LastErrorCode);
    UNREFERENCED_PARAMETER(_FirstTryDT);
    UNREFERENCED_PARAMETER(_OperatorId);
    UNREFERENCED_PARAMETER(_Sum);
    UNREFERENCED_PARAMETER(_Comission);
    UNREFERENCED_PARAMETER(_Fields);
}

//---------------------------------------------------------------------------

void TSMSSender::StoreSendCommandProcessedTemp(int _CommandUID)
{
    UNREFERENCED_PARAMETER(_CommandUID);
}

//---------------------------------------------------------------------------

void TSMSSender::StoreCommandProcessed(int _CommandUID)
{
    UNREFERENCED_PARAMETER(_CommandUID);
}

//---------------------------------------------------------------------------

void TSMSSender::StoreFileSend(AnsiString _FileName)
{
    UNREFERENCED_PARAMETER(_FileName);
}

//---------------------------------------------------------------------------

void TSMSSender::StoreFilesSend(AnsiString _FileNames, AnsiString _ArchiveFileName)
{
    UNREFERENCED_PARAMETER(_FileNames);
    UNREFERENCED_PARAMETER(_ArchiveFileName);
}

//---------------------------------------------------------------------------

bool TSMSSender::SendHeartBeat(short _Version, short _Status, int _BillCount, int _BillSum, int _SimBalance, int _SignalLevel, bool bFirstTry)
{
    UNREFERENCED_PARAMETER(_Version);
    UNREFERENCED_PARAMETER(_Status);
    UNREFERENCED_PARAMETER(_BillCount);
    UNREFERENCED_PARAMETER(_BillSum);
    UNREFERENCED_PARAMETER(_SimBalance);
    UNREFERENCED_PARAMETER(_SignalLevel);
    UNREFERENCED_PARAMETER(bFirstTry);
    return false;
}

//---------------------------------------------------------------------------

AnsiString TSMSSender::TestConnection(AnsiString URL)
{
    UNREFERENCED_PARAMETER(URL);
    return "Check not implemented, try to check connection to Cyberplat.";
}

//---------------------------------------------------------------------------

bool TSMSSender::Connect(AnsiString PhoneNum, AnsiString MessageText)
{
    bool bResult;
    try {
		if (!StatPacket)
			return "";
        std::auto_ptr <CModemSiemensMC35i> Modem(new CModemSiemensMC35i(Cfg->Peripherals.Modem.ServiceInfoPort, Log));
		if(Modem.get())
			bResult = Modem->SendSMS(Cfg->SMSInfo.PhoneNumber.c_str(),MessageText.c_str());
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return bResult;
}
//---------------------------------------------------------------------------
bool TSMSSender::Process(bool bProcessMessages)
{
    UNREFERENCED_PARAMETER(bProcessMessages);
    bool bResult = false;
	try {
		if(PacketLoadError)	{
			Log->Write("Error! Packet not loaded.");
			return false;
        }
		if (!StatPacket)
			return false;
		switch (StatPacket->PacketType) {
			case cnError:
				Log->WriteInLine((boost::format("  Sending SMS message: {%1%}...") % StatPacket->ErrDescription.c_str()).str().c_str());
				bResult = Connect(Cfg->SMSInfo.PhoneNumber.c_str(), StatPacket->ErrDescription);
                Log->Append((bResult ? "OK." : "ERROR!"));
				break;
			default:
				Log->Write((boost::format("  Unknown packet type: %1%") % StatPacket->PacketType).str().c_str());
				return false;
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
	return bResult;
}

