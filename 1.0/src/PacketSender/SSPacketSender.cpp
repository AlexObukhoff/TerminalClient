//---------------------------------------------------------------------------

#include "SevenZipVCL.hpp"
#include <IdTCPClient.hpp>
#pragma hdrstop
#include "globals.h"
#include "boost/format.hpp"

#pragma package(smart_init)
#include "SSPacketSender.h"
#include "TSendThread2.h"

//---------------------------------------------------------------------------

TSSPacketSender::TSSPacketSender(AnsiString fileName, TWConfig* cfg, TLogClass* log, TFileMap* fileMap)
: TPacketSender(fileName, cfg, log, fileMap)
{
}

//---------------------------------------------------------------------------

/*TSSPacketSender::~TSSPacketSender()
{
//Log->Write("~TSSPacketSender started.");
if (Send)
	{
	//Log->Write("Send...");
	Send->Terminate();
	Sleep(200);
	TerminateThread((HANDLE)Send->Handle,ExitCode);
	//Send->Terminate();
	delete Send;
	//Log->Append("Done.");
	Send=NULL;
	}
//Log->Write("~TSSPacketSender done.");
}*/

//---------------------------------------------------------------------------

bool TSSPacketSender::SendHeartBeat(short _Version, short _Status, int _BillCount, int _BillSum, int _SimBalance, int _SignalLevel, bool bFirstTry, int _ChequeCounter)
{
    UNREFERENCED_PARAMETER(bFirstTry);
    if (!Enabled)
        return false;

    bool result = true;

    try
    {
        Clear();
        StatPacket->PacketType=cnHeartBeat;
        StatPacket->TerminalID=TerminalID;
        StatPacket->ErrorCode=_Version;
        StatPacket->Status=_Status;
        StatPacket->BillCount=_BillCount;
        StatPacket->BillSum=_BillSum;
        StatPacket->ErrType=_SimBalance;
        StatPacket->ErrSubType=_SignalLevel;
        StatPacket->ChequeCounter=_ChequeCounter;
        Log->Write((boost::format(" Sending HeartBeat {TID: %1%; Ver.: %2%; St.: %3%; BillCount: %4%; BillSum: %5%; SIM balance: %6%; Signal level: %7%; ChequeCounter: %8%}...")
          % StatPacket->TerminalID
          % StatPacket->ErrorCode
          % StatPacket->Status
          % StatPacket->BillCount
          % StatPacket->BillSum
          % StatPacket->ErrType
          % StatPacket->ErrSubType
          % StatPacket->ChequeCounter).str().c_str());
        Process();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        result = false;
    }

    return result;
}

//---------------------------------------------------------------------------

AnsiString TSSPacketSender::TestConnection(AnsiString URL)
{
try
    {
    if (!Enabled)
        return "";
    Clear();
    StatPacket->PacketType=cnTestConnection;
    StatPacket->TerminalID=TerminalID;
    StatPacket->Status=0;
    StatPacket->ForcedURL=URL;
    Process(true);
    return TestResult;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		return "Exception occured in TSSPacketSender::TestConnection: ";
		}
}

//---------------------------------------------------------------------------

bool TSSPacketSender::Process(bool bProcessMessages)
{
try
	{
	if (!Enabled)
		return false;

	if (PacketLoadError)
		{
		Log->Write("Error! Packet not loaded.");
		return false;
		}

	if (((StatPacket->PacketType==cnPaymentInit)||(StatPacket->PacketType==cnPaymentStatusChange)||(StatPacket->PacketType==cnPaymentComplete))&&(StatPacket->InitialSessionNum==""))
		{
		Log->Write("Error! Initial session number not found - deleting packet...");
		return true;
		}

	bool Result=false;
	//Command=-1;
	std::auto_ptr <TSendThread2> Send ( new TSendThread2(Cfg, StatPacket, Log, FileMap) );
	if (Send.get())
		{
		Send->Resume();
		int TimeOut=12000;
		if ((StatPacket->PacketType==cnFileSend)||(StatPacket->PacketType==cnFileSendNew))
			TimeOut=54000;
		if (StatPacket->PacketType==cnHeartBeat)
			TimeOut=90000;
		while ((TimeOut>0)&&(!Send->Finished)&&(!this->Terminated))
			{
			if (bProcessMessages) Application->ProcessMessages();
			Sleep(10);
			TimeOut--;
			}
		if (Terminated)
			{
			Log->Write("Terminating connection due to the object termination...");
			Send->Terminate();
			Sleep(5000);
			TerminateThread((HANDLE)Send->Handle,0);
			}
		else {
			if (TimeOut<=0) {
				Log->Write("Terminating connection due to timeout...");
				Send->Terminate();
				Sleep(5000);
				TerminateThread((HANDLE)Send->Handle,0);
				}
			if (StatPacket->PacketType==cnTestConnection)
				TestResult = Send->TestConnectionResult;
			if (Send->Finished) {
				Result=Send->Sent;
				}
			if ((!Result)&&(StatPacket->PacketType!=cnTestConnection))
        {
        Cfg->ChangeStatServer();
				}

      if (FileMap) FileMap->PutLastConnectionDT();
			//Log->Write("LastConnection time mark updated.");

      HeartBeatOK = Send->HeartBeatOK;
			if (Send->ConnectOK)
				{
        if (FileMap) FileMap->PutLastSuccessfullConnectionDT();
				//Log->Write("LastSuccessfullConnection time mark updated.");
				}
			}
		}
	return Result;
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
	return false;
	}
}

//---------------------------------------------------------------------------

