//---------------------------------------------------------------------------

#include <vcl.h>
#include <algorith.h>
#include <Classes.hpp>
#include <IdTCPClient.hpp>
#pragma hdrstop

#include "TSendThread.h"
#include "SevenZipVCL.hpp"
#include "globals.h"
#include <boost/format.hpp>
#include "CryptLib2.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------

//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall TSendThread::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

const cnMaxPacketSize=1024;

void __fastcall TSendThread::Init(int _TerminalID, AnsiString _Host, int _Port, TStatPacket *_StatPacket, TLogClass *_Log)
{
try
		{
		CoInitializeEx(NULL, COINIT_MULTITHREADED);
		Finished=false;
		InnerLog=false;
		if (_Log==NULL) {
				Log = new TLogClass("TSendThread");
				InnerLog=true;
				}
				else
				Log=_Log;
		    StatPacket=_StatPacket;
		TerminalID=_TerminalID;
		//FileMap=_FileMap;
		Host=_Host;
		Port=_Port;
		InFile = NULL;
		FileToSend = NULL;
		CommandReceiver = NULL;
		IdTCPC = NULL;
		IdTCPC = new TIdTCPClient(Application);
		if (Cfg->Connection().HTTPProxy.Type.find("socks")!=std::string::npos)
				IdTCPC->SocksInfo->Assign(Cfg->Connection().Proxy);
		IdTCPC->Host=Host;
		IdTCPC->Port=Port;
		Sent=false;
		ConnectOK = false;
    HeartBeatOK = false;
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}


/*__fastcall TSendThread::TSendThread(int _TerminalID, AnsiString _Host, int _Port, TStatPacket *_StatPacket, TLogClass *_Log, TFileMap *_FileMap)
		: TThread(true)
{
FileMap=_FileMap;
Init((_TerminalID, _Host, _Port, _StatPacket, _Log);
}*/

__fastcall TSendThread::TSendThread(TWConfig *_Cfg, TStatPacket *_StatPacket, TLogClass *_Log, TFileMap *_FileMap)
		: TThread(true)
{
Cfg=_Cfg;
//m_cryptLib = crypt;
FileMap=_FileMap;
Init((AnsiString(Cfg->Terminal.Number.c_str())).ToInt(), Cfg->GetStatServerHost(), Cfg->GetStatServerPort(), _StatPacket, _Log);
}

//---------------------------------------------------------------------------

__fastcall TSendThread::~TSendThread(void)
{
//CoUninitialize();
//Log->Write("~TSendThread started.");
if (InFile!=NULL) {
	Log->Write("InFile...");
	delete InFile;
	Log->Append("done.");
	InFile = NULL;
	}
if (FileToSend!=NULL) {
	Log->Write("FileToSend...");
	delete FileToSend;
	Log->Append("done.");
	FileToSend = NULL;
	}
if (IdTCPC!=NULL) {
//	Log->Write("IdTCPC...");
	IdTCPC->Binding->CloseSocket();
//	IdTCPC->CancelWriteBuffer();
	if (IdTCPC->Connected())
		{
		IdTCPC->Disconnect();
		Log->Append("disconnected...");
		}
	delete IdTCPC;
//	Log->Append("done.");
	IdTCPC = NULL;
	}
if (CommandReceiver) {
//	Log->Write("CommandReceiver...");
	delete CommandReceiver;
//	Log->Append("done.");
	CommandReceiver = NULL;
	}
//Log->Write("~TSendThread done.");
if (InnerLog)
	delete Log;
}

//---------------------------------------------------------------------------

void __fastcall TSendThread::Execute()
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

//---------------------------------------------------------------------------

void TSendThread::SendPaymentInit()
{
try
		{
		AnsiString AToLog="  Sending PaymentInit event {TID: "+AnsiString(TerminalID)+"; EventDT: "+StatPacket->EventDateTime+"; OpID: "+StatPacket->OperatorID+"; InitSess: "+StatPacket->InitialSessionNum+"; Comiss.: "+StatPacket->Comission+"; Notes: { ";
		AddToBuffer(cnPaymentInit);
		AddToBuffer(TerminalID);
		AddToBuffer(double(StatPacket->EventDateTime));
		AddToBuffer(StatPacket->OperatorID);
		AddToBuffer(StatPacket->InitialSessionNum,false, 20);
		AddToBuffer(int(StatPacket->Comission*100));
		AddToBuffer((int)StatPacket->vNotes.size());
		for (std::size_t i=0;i<StatPacket->vNotes.size();i++) {
				AddToBuffer(StatPacket->vNotes[i].ValidatorID);
				AddToBuffer(StatPacket->vNotes[i].CurrencyID.c_str(),false);
				AddToBuffer(StatPacket->vNotes[i].Nominal);
				AddToBuffer(StatPacket->vNotes[i].Count);
				AToLog += (boost::format("{%1% %2% %3% %4%} ") % StatPacket->vNotes[i].ValidatorID % StatPacket->vNotes[i].CurrencyID.c_str() % StatPacket->vNotes[i].Nominal % StatPacket->vNotes[i].Count).str().c_str();
				}
		AToLog+="}; Payment Info: {"+StatPacket->GetParamValue("params")+"}}...";
		Log->Write(AToLog.c_str());
		AddToBuffer(StatPacket->GetParamValue("params"));
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TSendThread::SendIncassation()
{
try
		{
		AnsiString AToLog="  Sending Incassation event {TID: "+AnsiString(TerminalID)+"; EventDT: "+StatPacket->EventDateTime+"; Notes: { ";
		AddToBuffer(cnIncassation);
		AddToBuffer(TerminalID);
		AddToBuffer(double(StatPacket->EventDateTime));
		AddToBuffer((int)StatPacket->vNotes.size());
		for (std::size_t i=0;i<StatPacket->vNotes.size();i++) {
				AddToBuffer(StatPacket->vNotes[i].ValidatorID);
				AddToBuffer(StatPacket->vNotes[i].CurrencyID.c_str(), false);
				AddToBuffer(StatPacket->vNotes[i].Nominal);
				AddToBuffer(StatPacket->vNotes[i].Count);
				AToLog += (boost::format("{%1% %2% %3% %4%} ") % StatPacket->vNotes[i].ValidatorID % StatPacket->vNotes[i].CurrencyID.c_str() % StatPacket->vNotes[i].Nominal % StatPacket->vNotes[i].Count).str().c_str();
				}
		AToLog+="};}...";
		Log->Write(AToLog.c_str());
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TSendThread::SendError()
{
try
		{
		AnsiString AToLog="  Sending Error event {TID: "+AnsiString(TerminalID)+"; EventDT: "+StatPacket->EventDateTime+"; ErrorSender: "+StatPacket->ErrSender+"; ErrorType: "+StatPacket->ErrType+"; ErrorDescription: "+StatPacket->ErrDescription+"}...";
    if (StatPacket->ErrSubType!=0)
        AToLog+="; ErrorSubType: "+AnsiString(StatPacket->ErrSubType)+"; ErrorSubDescription: "+StatPacket->ErrSubDescription;
    AToLog+="}...";
    Log->Write(AToLog.c_str());

    AddToBuffer(cnError);
		AddToBuffer(TerminalID);
    AddToBuffer(double(StatPacket->EventDateTime));
    AddToBuffer(StatPacket->ErrType);
    AddToBuffer(StatPacket->ErrSubType);
    AddToBuffer(StatPacket->ErrSender+"|"+StatPacket->ErrDescription+"|"+StatPacket->ErrSubDescription);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TSendThread::SendPaymentStatusChange()
{
try
		{
		Log->Write((boost::format("  Sending PaymentStatusChange event {TID: %1%; EventDT: %2%; InitSess: %3%; Status: %4%; Error: %5%}...") % TerminalID % AnsiString(StatPacket->EventDateTime).c_str() % StatPacket->InitialSessionNum.c_str() % StatPacket->Status % StatPacket->ErrorCode).str().c_str());

		AddToBuffer(cnPaymentStatusChange);
		AddToBuffer(TerminalID);
		AddToBuffer(double(StatPacket->EventDateTime));
		AddToBuffer(StatPacket->InitialSessionNum, false, 20);
		AddToBuffer(short(StatPacket->ErrorCode));
		AddToBuffer(short(StatPacket->Status));
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}


//---------------------------------------------------------------------------

void TSendThread::SendPaymentComplete()
{
try
		{
		Log->Write((boost::format("  Sending PaymentComplete event {TID: %1%; EventDT: %2%; InitSess.: %3%; Sess.: %4%; Error: %5%}...") % TerminalID % AnsiString(StatPacket->EventDateTime).c_str() % StatPacket->InitialSessionNum.c_str() % StatPacket->SessionNum.c_str() % StatPacket->ErrorCode).str().c_str());
        AddToBuffer(cnPaymentComplete);
        AddToBuffer(TerminalID);
        AddToBuffer(double(StatPacket->EventDateTime));
		AddToBuffer(StatPacket->InitialSessionNum, false, 20);
		AddToBuffer(StatPacket->SessionNum, false, 20);
        AddToBuffer(StatPacket->ErrorCode);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TSendThread::SendHeartBeat()
{
    try {

        Log->Write((boost::format(" Sending HeartBeat {TID: %1%; Ver.: %2%; St.: %3%; BillCount: %4%; BillSum: %5%; SIM balance: %6%; Signal level: %7%; ChequeCounter: %8%}...") % TerminalID % StatPacket->ErrorCode % StatPacket->Status % StatPacket->BillCount % StatPacket->BillSum % StatPacket->ErrType % StatPacket->ErrSubType % StatPacket->ChequeCounter).str().c_str());
        AddToBuffer(cnHeartBeat);
        AddToBuffer(TerminalID);
        AddToBuffer(short(StatPacket->ErrorCode));
        AddToBuffer(short(StatPacket->Status));
        AddToBuffer(int(StatPacket->BillCount));
        AddToBuffer(int(StatPacket->BillSum));
        AddToBuffer(int(StatPacket->ErrType));
        AddToBuffer(__int8(StatPacket->ErrSubType));
        AddToBuffer(int(StatPacket->ChequeCounter));
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TSendThread::SendCommandProcessed()
{
try
    {
		Log->Write((boost::format("  Sending CommandProcessed event {TID: %1%; CID: %2%}...") % TerminalID % StatPacket->Status).str().c_str());
		AddToBuffer(cnCommandProcessed);
		AddToBuffer(TerminalID);
		AddToBuffer(StatPacket->Status);
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TSendThread::SendTestConnection()
{
try
		{
		Log->Write((boost::format("  Testing connection {TID: %1%; Status: %2%}...") % TerminalID % StatPacket->Status).str().c_str());

    AddToBuffer(cnTestConnection);
    AddToBuffer(TerminalID);
    AddToBuffer(StatPacket->Status);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

bool TSendThread::SendFileSend()
{
try
		{
		AnsiString TempFileName = (StatPacket->PacketFileName.substr(0,StatPacket->PacketFileName.length()-3)+"file").c_str();
		int iFileSize = FileSizeByName(TempFileName);
		Log->Write((boost::format("  Sending file: %1%, size: %2%...") % StatPacket->SendFileName.c_str() % iFileSize).str().c_str());
		AddToBuffer(StatPacket->PacketType);
		AddToBuffer(TerminalID);
		AddToBuffer(StatPacket->SendFileName.Length());
		AddToBuffer(StatPacket->SendFileName, false);
		AddToBuffer(iFileSize);
		return true;
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

bool TSendThread::Process()
{
/*
    if(Cfg->StatInfo.IsSignStatistics && m_cryptLib == NULL)
    {
        Log->Write("Невозможно подписать пакет, т.к. CryptLib не инициализированна");
        return false;
    }
*/

    char FileBuffer[cnMaxPacketSize];
    bool bResult=false;
    int SentFileSize=0;
    int PacketSize=0;
    bool bFileSendError=false;
    try
    {
        try
        {
            if (!Terminated)
            {
                Command=-1;
                ClearBuffer();
                AnsiString TempFileName = (StatPacket->PacketFileName.substr(0,StatPacket->PacketFileName.length()-3)+"file").c_str();
                switch (StatPacket->PacketType)
                {
                    case cnPaymentInit:
                        SendPaymentInit();
                        break;
                    case cnIncassation:
                        SendIncassation();
                        break;
                    case cnHeartBeat:
                        SendHeartBeat();
                        break;
                    case cnError:
                        SendError();
                        break;
                    case cnPaymentStatusChange:
                        SendPaymentStatusChange();
                        break;
                    case cnPaymentComplete:
                        SendPaymentComplete();
                        break;
                    case cnTestConnection:
                        if (StatPacket->ForcedURL!="")
                          {
                          if (StatPacket->ForcedURL.Pos(":"))
                            {
                            IdTCPC->Host = StatPacket->ForcedURL.SubString(0,StatPacket->ForcedURL.Pos(":")-1);
                            IdTCPC->Port = GetInt(StatPacket->ForcedURL.SubString(StatPacket->ForcedURL.Pos(":")+1,StatPacket->ForcedURL.Length()));
                            }
                            else
                            {
                            IdTCPC->Host = StatPacket->ForcedURL;
                            IdTCPC->Port = 10024;
                            }
                          }
                        SendTestConnection();
                        break;
                    case cnCommandProcessed:
                        SendCommandProcessed();
                        break;
                    case cnFileSend:
                    case cnFileSendNew:
                        if (!FileExists(TempFileName))
                        {
                            Log->Append((boost::format("  Error sending packet - attached file %1% not found.") % TempFileName.c_str()).str().c_str());
                            bResult = true;
                        }
                        else
                        {
                            if (!SendFileSend())
                            {
                                Log->Write("Error accessing attached file - cancelling file sending...");
                                return false;
                            }
                        }
                        break;
                    default:
                        Log->Write((boost::format("  Unknown packet type: %1%") % StatPacket->PacketType).str().c_str());
                        bResult = true;
                }
                if (Terminated)
                    return false;

                if (!bResult)
                {
                    Log->Write((boost::format("   Connecting to %1%:%2%...") % IdTCPC->Host.c_str() % IdTCPC->Port).str().c_str());

                    IdTCPC->Connect();
                    Log->Append("OK.");
                    ConnectOK = true;
                    if (IdTCPC->Connected())
                    {
                        WriteBuffer();
                        if (StatPacket->PacketType==cnFileSend)
                        {
                            WriteFile(TempFileName);
                        }
                        if (StatPacket->PacketType==cnFileSendNew)
                        {
                            AnsiString TempFileName = (StatPacket->PacketFileName.substr(0,StatPacket->PacketFileName.length()-3)+"file").c_str();
                            int iFileSize = FileSizeByName(TempFileName);
                            int Offset=IdTCPC->ReadInteger(false);  // получаем offset...
                            Log->Append((boost::format(" Sending file from %1%...") % Offset).str().c_str());
                            FileToSend = new TFileStream(TempFileName,fmOpenRead);
                            FileToSend->Seek(Offset,soFromBeginning);
                            SentFileSize=0;
                            while (SentFileSize+Offset<iFileSize)
                            {
                                PacketSize=min(iFileSize-(SentFileSize+Offset),cnMaxPacketSize);
                                FileToSend->Read(FileBuffer, PacketSize);
                                try
                                {
                                    IdTCPC->WriteBuffer(FileBuffer, PacketSize, true);
                                    if (Terminated)
                                    {
                                        Log->Write((boost::format("Sent %1% bytes.") % SentFileSize).str().c_str());
                                        Log->Write("Thread terminated, exiting...");
                                        IdTCPC->CancelWriteBuffer();
                                        IdTCPC->Disconnect();
                                        return false;
                                    }
                                }
                                catch (...)
                                {
                                    ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                                    Log->Write("Exception occured in TSendThread::Process.SendFileNew");
                                    Log->Write((boost::format("File sending aborted - %1% bytes sent.") % SentFileSize).str().c_str());
                                    IdTCPC->CancelWriteBuffer();
                                    bFileSendError=true;
                                    break;
                                }
                                SentFileSize+=PacketSize;
                                //Log->Write("Sent "+AnsiString(SentFileSize)+" bytes.");
                            }
                        }
                        Log->Append("Done.");
                        if (!bFileSendError)
                        {
                            if ((StatPacket->PacketType!=cnHeartBeat)&&(!bFileSendError))
                            {  // все пакеты кроме heartbeat
                                try
                                {
                                    Log->Append(" Getting server  answer...");
                                    Command=IdTCPC->ReadInteger(false);  // получаем ответ сервера
                                }
                                catch (...)
                                {
                                    ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                                    Log->Append("Error getting answer from server, setting to -1... ");
                                    Command=-1;
                                }
                                switch (Command)
                                {
                                    case 0:
                                        Log->Append("OK.");
                                        Cfg->StatInfo.DTLastSuccessfullPacketSending=TDateTime::CurrentDateTime();
                                        bResult = true;
                                        if (StatPacket->PacketType==cnCommandProcessed)
                                        {
                                            if (FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(StatPacket->Status)+".pkt"))
                                            {
                                                if (DeleteFile((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(StatPacket->Status)+".pkt"))
                                                    Log->Write((boost::format("Command with CommandUID = %1% deleted.") % StatPacket->Status).str().c_str());
                                                else
                                                    Log->Write((boost::format("Can't delete file %1%\\%2%.pkt.") % Cfg->Dirs.CommandsInbound.c_str() % StatPacket->Status).str().c_str());
                                            }
                                            if (FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(StatPacket->Status)+".ok"))
                                            {
                                                if (DeleteFile((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(StatPacket->Status)+".ok"))
                                                    Log->Write((boost::format("Command with CommandUID = %1% processed and deleted.") % StatPacket->Status).str().c_str());
                                                else
                                                    Log->Write((boost::format("Can't delete file %1%\\%2%.ok.") % Cfg->Dirs.CommandsInbound.c_str() % StatPacket->Status).str().c_str());
                                            }
                                        }
                                        break;
                                        /*case 1:
                                            Log->Append("Error sending packet to server - terminal does not exist on server.");
                                            IdTCPC->Disconnect();
                                            Cfg->StatInfo->Inhibit=true;
                                            return false;*/
                                    case 2:
                                        Log->Append("Error sending packet to server - server database error.");
                                        break;
                                    case 3:
                                        Log->Append("Error sending packet to server - file saving error.");
                                        break;
                                    case 4:
                                        Log->Append("Error sending packet to server - unknown packet.");
                                        break;
                                    case 5:
                                        Log->Append("Error sending packet to server - session number not found.");
                                        break;
                                    default:
                                        Log->Append((boost::format("Error sending packet to server - unknown answer: #%1%.") % Command).str().c_str());
                                }
                            }
                            else
                            { // heartbeat
                                try
                                {
                                    Log->Append(" Getting  server answer...");
                                    Command=IdTCPC->ReadInteger(false);  // получаем ответ сервера
                                    Cfg->StatInfo.DTLastSuccessfullPacketSending=TDateTime::CurrentDateTime();
                                    do
                                    {
                                        if (Command == cnCmdNone)
                                        {
                                            HeartBeatOK = true;
                                            Cfg->StatInfo.Inhibit=false;  //Если прошел HeartBeat, включаем отправку платежей.
                                            Log->Append("OK.");
                                            bResult = true;
                                            break;
                                        }
                                        else
                                        {
                                            if (Command == cnCmdInhibitPktSend)
                                            {
                                                HeartBeatOK = true;
                                                Log->Append(" Server inhibits sending stat packets.");
                                                Cfg->StatInfo.Inhibit=true;
                                                break;
                                            }
                                            else
                                            {
                                                HeartBeatOK = true;
                                                int CommandUID=0;
                                                Log->Append("OK..");
                                                if (Cfg->StatInfo.Inhibit)
                                                    if (FileMap)
                                                        FileMap->CheckStatDir=true;
                                                Cfg->StatInfo.Inhibit=false;
                                                CommandUID=IdTCPC->ReadInteger(false);
                                                Command = ReceiveCommand(Command, CommandUID);
                                            }
                                        }
                                    }
                                    while ((Command!=0)&&(Command!=6)&&(Command!=-1));
                                }
                                catch (Exception &e)
                                {
                                    Log->Append((boost::format("Error getting answer from server: %1%, setting to -1... ") % e.Message.c_str()).str().c_str());
                                    Command=-1;
                                }
                                catch (...)
                                {
                                    ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                                    Log->Append("Error getting answer from server, setting to -1... ");
                                    Command=-1;
                                }
                            }
                        }
                    }
                    else
                    {
                        if (StatPacket->PacketType!=cnHeartBeat)
                            Log->Write("  Error getting answer from server - packet NOT sent.");
                    }
                }
                if (StatPacket->PacketType==cnTestConnection)
                {
                    TestConnectionResult="Connected to "+Host+" successfully.";
                    bResult = true;
                }
            }
        }
        catch (Exception &exception)
        {
            Log->WriteInLine((boost::format("Exception in TSendThread::Process: %1%") % exception.Message.c_str()).str().c_str());
            if (StatPacket->PacketType==cnTestConnection)
            {
                TestConnectionResult="Connect to "+IdTCPC->Host+":"+AnsiString(IdTCPC->Port)+" error: "+exception.Message;
            }
            else
            {
                if ((Cfg->StatInfo.DTLastSuccessfullPacketSending+Cfg->StatInfo.DTServerTimeOutDiff<TDateTime::CurrentDateTime())&&(!Cfg->StatInfo.Inhibit))
                {
                    Log->Write((boost::format("Last successfull packet sending time: %1%, entering inhibit mode...") % AnsiString(Cfg->StatInfo.DTLastSuccessfullPacketSending).c_str()).str().c_str());
                    Cfg->StatInfo.Inhibit=true;
                }
            }
            bResult = false;
        }
        catch (...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            Log->WriteInLine("Exception in TSendThread::Process");
            if (StatPacket->PacketType==cnTestConnection)
            {
                TestConnectionResult="Connect to "+IdTCPC->Host+":"+AnsiString(IdTCPC->Port)+" error: ?";
            }
            else
            {
                if ((Cfg->StatInfo.DTLastSuccessfullPacketSending+Cfg->StatInfo.DTServerTimeOutDiff<TDateTime::CurrentDateTime())&&(!Cfg->StatInfo.Inhibit))
                {
                    Log->Write((boost::format("Last successfull packet sending time: %1%, entering inhibit mode...") % AnsiString(Cfg->StatInfo.DTLastSuccessfullPacketSending).c_str()).str().c_str());
                    Cfg->StatInfo.Inhibit=true;
                }
            }
            bResult = false;
        }
    }
    __finally
    {
        if (IdTCPC->Connected())
        {
            IdTCPC->Disconnect();
            Log->Append(" Disconnected.");
        }
        if (FileToSend!=NULL)
        {
            delete FileToSend;
            FileToSend = NULL;
        }
    }
    return bResult;
}

//---------------------------------------------------------------------------

void TSendThread::Write(short _src)
{
    try
    {
        //IdTCPC->WriteInteger(_src);
        IdTCPC->WriteBuffer((const void*)&_src,sizeof(_src));
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TSendThread::Write(int _src)
{
    try
    {
        //IdTCPC->WriteInteger(_src);
        IdTCPC->WriteBuffer((const void*)&_src,sizeof(_src));
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TSendThread::Write(double _src)
{
    try
    {
        IdTCPC->WriteBuffer((const void*)&_src,sizeof(_src));
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TSendThread::Write(AnsiString _src)
{
    try
    {
        IdTCPC->WriteBuffer((const void*)_src.c_str(),_src.Length());
        IdTCPC->WriteSmallInt(0);
        //IdTCPC->WriteBuffer((const void*)_src.c_str(),_src.Length());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TSendThread::WriteSession(AnsiString _src)
{
    try
    {
        IdTCPC->WriteBuffer((const void*)_src.c_str(),20);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TSendThread::Write(AnsiString _src, int _BytesCount)
{
try
    {
        if (_BytesCount>_src.Length())
            IdTCPC->WriteBuffer((const void*)_src.c_str(), _BytesCount);
        else
            IdTCPC->WriteBuffer((const void*)_src.c_str(), _src.Length());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TSendThread::WriteBuffer(void* _src, int _BytesCount)
{
    try
    {
        if(Cfg->StatInfo.IsSignStatistics)
            signBuffer(_src, _BytesCount);

        IdTCPC->WriteBuffer((const void*)_src, _BytesCount, true);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TSendThread::WriteFile(AnsiString FileName)
{
    try
    {
        IdTCPC->WriteFile(FileName,false);
        //IdTCPC->WriteBuffer((const void*)_src.c_str(), _BytesCount);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

int TSendThread::ReceiveCommand(int Command, int CommandUID)
{
//TCommandReceiver *CommandReceiver = NULL;
//TPacketSender *PFS = NULL;
int IncomingFileSize;
int IncomingFileNameSize;
AnsiString IncomingFileName;
AnsiString IncomingPartFileName;
int ParametersSize;
AnsiString Parameters;
AnsiString FileMask;
//double Temp;
TDateTime BeginDT;
TDateTime EndDT;
AnsiString SessionNumber;
int ReceivedFileSize=0;
char FileBuffer[1024];
int PacketSize=0;
int StartOffset=0;
bool FileReceived = false;
int ReturnCommand = -1;
int KeysId;
bool DisconnectOrdered = false;
try
	{
	try
		{
		if (Terminated)
				return -1;
		if (CommandReceiver) {
			delete CommandReceiver;
			CommandReceiver = NULL;
			}
		CommandReceiver = new TCommandReceiver("",Cfg,Log,FileMap);
		switch (Command)
			{
			case cnCmdReboot:
				Log->Write((boost::format("Reboot command received, CID: %1%") % CommandUID).str().c_str());
				if (((FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".pkt"))||(FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".ok"))))
					{
					Log->Append(" Command already registered, skipping...");
					break;
					}
				if (!CommandReceiver->StoreRebootCommand(CommandUID))
					{
					Log->Write("Error saving packet file, trying to reboot...");
					Reboot();
					}
				break;
			case cnCmdReceiveFileOld:  //old file receive command
				Log->Write((boost::format("Starting receiving file, CID: %1%") % CommandUID).str().c_str());
				IncomingFileNameSize=IdTCPC->ReadInteger(false);
				IncomingFileName=IdTCPC->ReadString(IncomingFileNameSize);
				Log->Write((boost::format("File name: %1%.") % IncomingFileName.c_str()).str().c_str());
				IncomingFileSize=IdTCPC->ReadInteger(false);
				Log->Write((boost::format("File size: %1%.") % IncomingFileSize).str().c_str());
				if (InFile!=NULL) {
					delete InFile;
					InFile = NULL;
					}
				InFile = new TFileStream((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".file", fmCreate | fmShareDenyNone);
				IdTCPC->ReadStream(InFile, IncomingFileSize, False);
				Log->Write("File received.");
				if (((FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".pkt"))||(FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".ok"))))
					{
					Log->Append(" Command already registered, skipping...");
					break;
					}
				CommandReceiver->StoreReceiveFileCommand(CommandUID, IncomingFileName, IncomingFileSize, NULL);
				Log->Write((boost::format("Incoming file saved: %1%\\%2%.file") % Cfg->Dirs.CommandsInbound.c_str() % CommandUID).str().c_str());
				break;
			case cnCmdSendConfigOld:
				Log->Write((boost::format("Old send config file(s) command received, CID: %1%") % CommandUID).str().c_str());
				if (((FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".pkt"))||(FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".ok"))))
					{
					Log->Append(" Command already registered, skipping...");
					break;
					}
				CommandReceiver->StoreSendFileCommand(CommandUID, "");
				break;
			case cnCmdCancelPayment:
				Log->Write((boost::format("CancelPayment(s) cmd, CID: %1%") % CommandUID).str().c_str());
				SessionNumber=IdTCPC->ReadString(20);
				Log->Append((boost::format(", SessionNumber: %1%.") % SessionNumber.c_str()).str().c_str());
				if (Terminated)
						return -1;
				if (((FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".pkt"))||(FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".ok"))))
					{
					Log->Append(" Command already registered, skipping...");
					break;
					}
				CommandReceiver->StoreCancelPaymentCommand(CommandUID, SessionNumber);
				break;
			case cnCmdSendFile:
				Log->Write("File requested: ");
				FileMask=IdTCPC->ReadString(20);
				FileMask=FileMask.Trim();
				Log->Append(FileMask.c_str());
				if (((FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".pkt"))||(FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".ok"))))
					{
					Log->Append(" Command already registered, skipping...");
					break;
					}
				CommandReceiver->StoreGetFileByMaskCommand(CommandUID,FileMask);
				break;
/*			case cnCmdInhibitPktSend:
				Log->Append(" Server inhibits sending stat packets.");
				Cfg->StatInfo->Inhibit=true;
				break;*/
			case cnCmdReceiveFile: //new Receive File Command
				Log->Write((boost::format("Starting receiving file, CID: %1%") % CommandUID).str().c_str());
				IncomingFileNameSize=IdTCPC->ReadInteger(false);
				IncomingFileName=IdTCPC->ReadString(IncomingFileNameSize);
				Log->Append((boost::format(", file name: %1%") % IncomingFileName.c_str()).str().c_str());
				IncomingFileSize=IdTCPC->ReadInteger(false);
				Log->Append((boost::format(", file size: %1%, ") % IncomingFileSize).str().c_str());
				IncomingPartFileName=(Cfg->Dirs.CommandsInbound+"\\").c_str()+IncomingFileName+AnsiString(IncomingFileSize)+".part";

				if (FileExists(IncomingPartFileName)) {
					StartOffset = FileSizeByName(IncomingPartFileName);
					if (InFile!=NULL) {
						delete InFile;
						InFile = NULL;
						}
					InFile = new TFileStream(IncomingPartFileName, fmOpenWrite | fmShareExclusive);
					//StartOffset = FileSizeByName(IncomingPartFileName);
					if (StartOffset>0)
						InFile->Seek(0,soFromEnd);
					//StartOffset = InFile->Seek(0,soFromEnd);
					}
					else
					InFile = new TFileStream(IncomingPartFileName, fmCreate | fmShareExclusive);
				Write(StartOffset);
				Log->Append((boost::format(", starting offset: %1%.") % StartOffset).str().c_str());
				ReceivedFileSize=0;
				//IncomingFileSize+=10000;
				while (ReceivedFileSize+StartOffset<IncomingFileSize) {
				PacketSize=min(IncomingFileSize-(ReceivedFileSize+StartOffset),cnMaxPacketSize);
//            Log->Write("PacketSize = "+PacketSize);
					try
						{
						IdTCPC->ReadBuffer(FileBuffer, PacketSize);
						}
					catch (Exception &exception)
						{
						Log->Write((boost::format("Exception occured in TSendThread::ReceiveFile: %1%") % exception.Message.c_str()).str().c_str());
						Log->Write((boost::format("File receiving aborted - %1% bytes received.") % ReceivedFileSize).str().c_str());
						break;
						}
                                        catch (...)
                                        {
                                            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                                            Log->Write("Exception occured in TSendThread::ReceiveFile");
                                            Log->Write((boost::format("File receiving aborted - %1% bytes received.") % ReceivedFileSize).str().c_str());
                                            break;
                                        }
					InFile->Write(FileBuffer, PacketSize);
					ReceivedFileSize+=PacketSize;
					//Log->Write("Received "+AnsiString(PacketSize)+" bytes.");
					}
				if (ReceivedFileSize+StartOffset!=IncomingFileSize) {
					Log->Write((boost::format("File NOT received. %1% bytes stored.") % ReceivedFileSize).str().c_str());
					break;
					}
				Log->Write((boost::format("File received. %1% bytes stored.") % ReceivedFileSize).str().c_str());
				if (InFile!=NULL) {
					delete InFile;
					InFile = NULL;
					}
				Log->Write((boost::format("Incoming file saved to %1%.") % IncomingPartFileName.c_str()).str().c_str());
				if ((IncomingFileName.LowerCase()).Pos(".7z")>0) {
					if (TestArchiveFile(IncomingPartFileName))
						{
						Write(int(0)); // File received OK.
						FileReceived = true;
						}
						else {
						Write(int(1)); // Bad archive.
						if (DeleteFile(IncomingPartFileName))
							Log->Write((boost::format("File deleted: %1%.") % IncomingPartFileName.c_str()).str().c_str());
							else
							Log->Write((boost::format("Error deleting file: %1%!") % IncomingPartFileName.c_str()).str().c_str());
						break;
						}
					}
					else
					{
					Write(int(0)); // File received OK.
					FileReceived = true;
					}
				break;
			case cnCmdResurrectPayment:
				Log->Write((boost::format("Resurrect payment cmd, CID: %1%") % CommandUID).str().c_str());
				SessionNumber=IdTCPC->ReadString(20);
				Log->Append((boost::format(", SessionNumber: %1%") % SessionNumber.c_str()).str().c_str());
				ParametersSize=IdTCPC->ReadInteger(false);
				Parameters=IdTCPC->ReadString(ParametersSize);
				Log->Append((boost::format(", Parameters: %1%.") % Parameters.c_str()).str().c_str());

				if (Terminated)
						return -1;
				if (((FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".pkt"))||(FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".ok"))))
					{
					Log->Append(" Command already registered, skipping...");
					break;
					}
				CommandReceiver->StoreResurrectPaymentCommand(CommandUID, SessionNumber,Parameters);
				break;
			case cnCmdShutDown:
				Log->Write((boost::format("ShutDown command received, CID: %1%") % CommandUID).str().c_str());
				if (((FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".pkt"))||(FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".ok"))))
					{
					Log->Append(" Command already registered, skipping...");
					break;
					}
				if (!CommandReceiver->StoreShutDownCommand(CommandUID))
					{
					Log->Write("Error saving packet file, trying to shutdown system...");
					ShutDown();
					}
				break;
			case cnCmdBlock:
				Log->Write((boost::format("Block command received, CID: ") % CommandUID).str().c_str());
				if (((FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".pkt"))||(FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".ok"))))
					{
					Log->Append(" Command already registered, skipping...");
					break;
					}
				if (!CommandReceiver->StoreBlockCommand(CommandUID))
					{
					Log->Write("Error saving packet file, trying to block system...");
					ForceBlock();
					}
				break;
			case cnCmdUnblock:
				Log->Write((boost::format("Unblock command received, CID: %1%") % CommandUID).str().c_str());
				if (((FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".pkt"))||(FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".ok"))))
					{
					Log->Append(" Command already registered, skipping...");
					break;
					}
				if (!CommandReceiver->StoreUnblockCommand(CommandUID))
					{
					Log->Write("Error saving packet file!");
					}
				break;
			case cnCmdGetKeys:
				Log->Write((boost::format("GetKeys cmd, CID: %1%") % CommandUID).str().c_str());
				ParametersSize=IdTCPC->ReadInteger(false);
				Log->Append((boost::format(", KeysId: %1%") % KeysId).str().c_str());
 				ParametersSize=IdTCPC->ReadInteger(false);
				Parameters=IdTCPC->ReadString(ParametersSize);
				Log->Append((boost::format(", Parameters: %1%.") % Parameters.c_str()).str().c_str());

				if (Terminated)
						return -1;
				if (((FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".pkt"))||(FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".ok"))))
					{
					Log->Append(" Command already registered, skipping...");
					break;
					}
				CommandReceiver->StoreGetKeysCommand(CommandUID, KeysId,Parameters);
				break;
      default:
  			Log->Append((boost::format(" Unknown command #%1% received, disconnecting...") % Command).str().c_str());
        DisconnectOrdered = true;
        break;
			}

    if (!DisconnectOrdered)
      {
      try                                                                         // Получаем следующую команду...
        {
        Log->Write(" Getting server answer...");
        ReturnCommand=IdTCPC->ReadInteger(false);  // получаем ответ сервера
        Log->Append((boost::format("%1%.") % ReturnCommand).str().c_str());

        switch (Command)                                                          // Пост-обработка команды после ответа сервера
          {
          case cnCmdReceiveFile: //new Receive File Command
            if (FileReceived)
              {
              DeleteFile((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".file");
              if (RenameFile(IncomingPartFileName, (Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".file"))
                Log->Write((boost::format("File %1% saved to %2%\\%3%.file.") % IncomingPartFileName.c_str() % Cfg->Dirs.CommandsInbound.c_str() % CommandUID).str().c_str());
                else
                Log->Write((boost::format("Error renaming file %1% to %2%\\%3%.file!") % IncomingPartFileName.c_str() % Cfg->Dirs.CommandsInbound.c_str() % CommandUID).str().c_str());
              if (((FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".pkt"))||(FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".ok"))))
                {
                Log->Append(" Command already registered, skipping...");
                break;
                }
              CommandReceiver->StoreReceiveFileCommand(CommandUID, IncomingFileName, IncomingFileSize, Buffer);
              }
            break;
          }

        }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            Log->Append("setting to -1... ");
            ReturnCommand=-1;
        }
      }
      else
      ReturnCommand = -1;
		}
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        }
	}
__finally
	{
	if (CommandReceiver) {
		delete CommandReceiver;
		CommandReceiver = NULL;
		}
	if (InFile!=NULL) {
		delete InFile;
		InFile = NULL;
		}
	return ReturnCommand;
	}
}

//---------------------------------------------------------------------------

void TSendThread::ClearBuffer()
{
    try
    {
        ZeroMemory(Buffer,StatBufferSize);
        BufferLength=0;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TSendThread::AddToBuffer(__int8 Value)
{
    try
    {
        if (BufferLength+sizeof(__int8)<StatBufferSize)
        {
            memcpy(Buffer+BufferLength,&Value,sizeof(__int8));
            BufferLength+=sizeof(__int8);
        }
        else
        {
            Log->Write((boost::format("Can't add {%1%} to the buffer - limit exceeded.") % Value).str().c_str());
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TSendThread::AddToBuffer(int Value)
{
    try
    {
        if (BufferLength+sizeof(int)<StatBufferSize)
        {
            memcpy(Buffer+BufferLength,&Value,sizeof(int));
            BufferLength+=sizeof(int);
        }
        else
        {
            Log->Write((boost::format("Can't add {%1%} to the buffer - limit exceeded.") % Value).str().c_str());
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TSendThread::AddToBuffer(short Value)
{
    try
    {
        if (BufferLength+sizeof(short)<StatBufferSize)
        {
            memcpy(Buffer+BufferLength,&Value,sizeof(short));
            BufferLength+=sizeof(short);
        }
        else
        {
            Log->Write((boost::format("Can't add {%1%} to the buffer - limit exceeded.") % Value).str().c_str());
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TSendThread::AddToBuffer(double Value)
{
    try
    {
        if (BufferLength+sizeof(double)<StatBufferSize)
        {
            memcpy(Buffer+BufferLength,&Value,sizeof(double));
            BufferLength+=sizeof(double);
        }
        else
        {
            Log->Write((boost::format("Can't add {%1%} to the buffer - limit exceeded.") % Value).str().c_str());
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TSendThread::AddToBuffer(AnsiString Value, bool AddZero, int Size)
{
    try
    {
        for (int i=Value.Length();i<Size;i++)
            Value+=" ";
        if (BufferLength+Value.Length()+1<StatBufferSize)
        {
            memcpy(Buffer+BufferLength,Value.c_str(),Value.Length());
            BufferLength+=Value.Length();
            if (AddZero)
               BufferLength++;
        }
        else
        {
            Log->Write((boost::format("Can't add {%1%} to the buffer - limit exceeded.") % Value.c_str()).str().c_str());
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TSendThread::WriteBuffer()
{
    try
    {
        WriteBuffer(Buffer,BufferLength);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

bool TSendThread::TestArchiveFile(AnsiString SourceFileName)
{
TSevenZip *SZ = NULL;
bool bResult = false;
try
  {
  try
    {
    TSevenZip *SZ = new TSevenZip(NULL);
    SZ->SZFileName=WideString(SourceFileName);
    //SZ->ExtrBaseDir = TargetDirName;
    //SZ->ExtractOptions = SZ->ExtractOptions << ExtractOverwrite;
    SZ->Files->Clear();
    SZ->Extract(true);
    if (SZ->ErrCode==0) {
      bResult=true;
      }
      else
      Log->Write((boost::format("Testing archive file %1% error #%1%!") % SourceFileName.c_str() % SZ->ErrCode).str().c_str());
    }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            bResult = false;
        }
  }
__finally
  {
  if (SZ!=NULL) {
    delete SZ;
    SZ = NULL;
    }
  return bResult;
  }
}

void TSendThread::Reboot()
{
Log->Write("Reboot sequence started...");
HANDLE hToken;
TOKEN_PRIVILEGES tkp;

OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken); // Get a token for this process.

LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); // Get the LUID for the shutdown privilege.

tkp.PrivilegeCount = 1;  // one privilege to set
tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); // Get the shutdown privilege for this process.
																																					 // Cannot test the return value of AdjustTokenPrivileges.
ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0); //Shut down the system and force all applications to close.

Log->Write("ExitWindows done...");
Application->Terminate();
}

void TSendThread::ShutDown()
{
Log->Write("Shutdown sequence started...");
HANDLE hToken;
TOKEN_PRIVILEGES tkp;

OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken); // Get a token for this process.

LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); // Get the LUID for the shutdown privilege.

tkp.PrivilegeCount = 1;  // one privilege to set
tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); // Get the shutdown privilege for this process.
																																					 // Cannot test the return value of AdjustTokenPrivileges.
ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0); //Shut down the system and force all applications to close.

Log->Write("ExitWindows done...");
Application->Terminate();
}

void TSendThread::ForceBlock()
{
if (FileMap) FileMap->ForceBlock = true;
}

void TSendThread::signBuffer(void* buffer, std::size_t& size)
{
    BYTE* b = (BYTE*)buffer;

    // преобразуем буфер в строку
    std::string strBuf = "";
    for(std::size_t i = 0; i < size; i++)
        strBuf += (boost::format("%02X") % b[i]).str();

    // подписываем
    //AnsiString signedBuffer = m_cryptLib->Sign(Cfg->GetKeysNum(), strBuf).c_str();
    AnsiString signedBuffer = crypt::sign(Cfg->GetKeysNum(), strBuf).c_str();

    // переписываем буфер
    if(signedBuffer.Length() > StatBufferSize)
    {
        Log->Write("Rreceive buffer size more than transmittion buffer size");
        //return false;
    }
    memcpy(buffer, signedBuffer.c_str(), signedBuffer.Length());
    size = signedBuffer.Length();
}

