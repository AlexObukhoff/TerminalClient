//---------------------------------------------------------------------------

#include <IdSMTP.hpp>
//#include <SevenZipVCL.hpp>
#include <memory>
#pragma hdrstop

#pragma package(smart_init)
#include "CSPacketSender.h"
#include "CommandReceiver.h"
#include "globals.h"
#include <boost/format.hpp>
#include "CryptLib2.h"

//---------------------------------------------------------------------------

TCSPacketSender::TCSPacketSender(AnsiString _fileName, TWConfig* _Cfg, TLogClass* _Log, TFileMap* _FileMap)
: TPacketSender(_fileName, _Cfg, _Log, _FileMap)
{
try
  {
  Connected = false;

	WCState=-1;
	LastPaymentReceived.Val=0;
	LastPaymentProcessed.Val=0;
	BillsCount=-1;
	BillsSum=-1;
	SIMBalance=-1;
	GSMSignalQuality=-1;
	ValidatorState=-1;
	PrinterState=-1;
	SW0=-1;
	SW1=-1;
	SW2=-1;
	GSMOperatorID=-1;

	tmpWCState=-1;
	tmpLastPaymentReceived.Val=0;
	tmpLastPaymentProcessed.Val=0;
	tmpBillsCount=-1;
	tmpBillsSum=-1;
	tmpSIMBalance=-1;
	tmpGSMSignalQuality=-1;
	tmpValidatorState=-1;
	tmpPrinterState=-1;
	tmpSW0=-1;
	tmpSW1=-1;
	tmpSW2=-1;
	tmpGSMOperatorID=-1;
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

TCSPacketSender::~TCSPacketSender()
{
}

//---------------------------------------------------------------------------

/*void TCSPacketSender::StorePaymentInitTemp(TDateTime _EventDateTime, int _OperatorId, AnsiString _InitialSessionNum, float _Comission, const TNotesVector& _Notes, AnsiString _Fields)
{
}

//---------------------------------------------------------------------------

void TCSPacketSender::StorePaymentInitComplete()
{
}

//---------------------------------------------------------------------------

void TCSPacketSender::StoreError(TDateTime _EventDateTime, AnsiString _Sender, int _Type, AnsiString _Description, int _SubType, AnsiString _SubDescription)
{
}

//---------------------------------------------------------------------------

void TCSPacketSender::StorePaymentStatusChange(TDateTime _EventDateTime, AnsiString _InitialSessionNum, int _Status, int _ErrorCode)
{
}*/

//---------------------------------------------------------------------------

void TCSPacketSender::StorePaymentComplete(TDateTime _EventDateTime, AnsiString _InitialSessionNum, AnsiString _LastSessionNum, int _ErrorCode, int _LastErrorCode, TDateTime _FirstTryDT, int _OperatorId, double _Sum, double _Comission, AnsiString _Fields)
{
    UNREFERENCED_PARAMETER(_LastErrorCode);
    UNREFERENCED_PARAMETER(_Sum);
    UNREFERENCED_PARAMETER(_Comission);

    if (!Enabled)
        return;
    if ((_ErrorCode!=254) && (_ErrorCode!=255))
        return;

    try
    {
        AnsiString AToLog="TCSPacketSender -> PaymentComplete event stored. {TID: "+AnsiString(TerminalID)+"; EventDT: "+StatPacket->EventDateTime+"; OperatorID: "+StatPacket->OperatorID+"; InitialSessionNum: "+StatPacket->InitialSessionNum+"; Comission: "+StatPacket->Comission+"; Notes: { ";
        Log->Write(AToLog.c_str());

        StatPacket->PacketType=cnPaymentComplete;
        StatPacket->TerminalID=TerminalID;
        StatPacket->EventDateTime=_EventDateTime;
        StatPacket->OperatorID=_OperatorId;
        StatPacket->InitialSessionNum=_InitialSessionNum;
        StatPacket->AddParam("params",_Fields);
        StatPacket->SessionNum=_LastSessionNum;
        StatPacket->ErrorCode=_ErrorCode;
        StatPacket->InitDT=_FirstTryDT;
        
        if (!StatPacket->SaveToFile())
            if (FileMap)
                FileMap->WriteErrorFound = true;

        StatPacket->CloseFile();
        if (FileMap)
            FileMap->CheckStatDir=true;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

/*void TCSPacketSender::StoreSendCommandProcessedTemp(int _CommandUID)
{
}*/

//---------------------------------------------------------------------------

/*void TCSPacketSender::StoreCommandProcessed(int _CommandUID)
{
Log->Write("Processed CID put to FileMap: "+AnsiString(_CommandUID)+".");
FileMap->CID = _CommandUID;
}*/

//---------------------------------------------------------------------------

/*void TCSPacketSender::StoreFileSend(AnsiString _FileName)
{
} */

//---------------------------------------------------------------------------

/*void TCSPacketSender::StoreFilesSend(AnsiString _FileNames, AnsiString _ArchiveFileName)
{
Log->Write("Sending Files {FileNames: "+_FileNames+"; ArchiveFileName: "+_ArchiveFileName);
}*/

//---------------------------------------------------------------------------

bool TCSPacketSender::SendHeartBeat(short _Version, short _Status, int _BillCount, int _BillSum, int _SimBalance, int _SignalLevel, bool bFirstTry, int _ChequeCounter)
{
    UNREFERENCED_PARAMETER(_Version);
    UNREFERENCED_PARAMETER(_Status);
    UNREFERENCED_PARAMETER(_BillCount);
    UNREFERENCED_PARAMETER(_BillSum);
    UNREFERENCED_PARAMETER(_SimBalance);
    UNREFERENCED_PARAMETER(_SignalLevel);

if (!FileMap)
  return false;
AnsiString MessageText;
//long NotesSum = 0;
int iError;
int iCommand;
int iCID;
	try
		{
		tmpWCState=WCState;
		tmpLastPaymentReceived.Val=LastPaymentReceived.Val;
		tmpLastPaymentProcessed.Val=LastPaymentProcessed.Val;
		tmpBillsCount=BillsCount;
		tmpBillsSum=BillsSum;
		tmpSIMBalance=SIMBalance;
		tmpGSMSignalQuality=GSMSignalQuality;
		tmpValidatorState=ValidatorState;
		tmpPrinterState=PrinterState;
		tmpSW0=SW0;
		tmpSW1=SW1;
		tmpSW2=SW2;
		tmpGSMOperatorID=GSMOperatorID;

		AnsiString AToLog="  Sending HeartBeat event {TID: "+AnsiString(TerminalID)+"; EventDateTime: "+StatPacket->EventDateTime;//+"; Notes: { ";
		MessageText+=("AP="+Cfg->Keys[Cfg->GetKeysNum()].AP+"\r\nDT="+(TDateTime::CurrentDateTime()).FormatString("dd.mm.yyyy hh:nn:ss").c_str()).c_str();
//		bool bFirstTry = true;

		if (((bFirstTry)||(FileMap->WCState!=WCState))&&(FileMap->WCState!=-1))
			{
			WCState = FileMap->WCState;
			MessageText+="\r\nTST="+AnsiString(WCState);
			}

		if (((bFirstTry)||(FileMap->LastPaymentReceived!=LastPaymentReceived))&&(FileMap->LastPaymentReceived.Val!=0))
			{
			LastPaymentReceived = FileMap->LastPaymentReceived;
			MessageText+="\r\nLPIN="+LastPaymentReceived.FormatString("dd.mm.yyyy hh:nn:ss");
			}

		if (((bFirstTry)||(FileMap->LastPaymentProcessed!=LastPaymentProcessed))&&(FileMap->LastPaymentProcessed.Val!=0))
			{
			LastPaymentProcessed = FileMap->LastPaymentProcessed;
			MessageText+="\r\nLPPR="+LastPaymentProcessed.FormatString("dd.mm.yyyy hh:nn:ss");
			}

		if (((bFirstTry)||(FileMap->BillsCount!=BillsCount))&&(FileMap->BillsCount!=-1))
			{
			BillsCount = FileMap->BillsCount;
			MessageText+="\r\nBC="+AnsiString(BillsCount);
			}

		if (((bFirstTry)||(FileMap->BillsSum!=BillsSum))&&(FileMap->BillsSum!=-1))
			{
			BillsSum = FileMap->BillsSum;
			MessageText+="\r\nBSUM="+AnsiString(BillsSum);
			}

		if (((bFirstTry)||(FileMap->SIMBalance!=SIMBalance))&&(FileMap->SIMBalance!=-1))
			{
			SIMBalance = FileMap->SIMBalance;
			MessageText+="\r\nSIMBL="+ChangeChars(FloatToStrF(double(SIMBalance)/100,ffFixed,18,2), ",", ".");
			}

		if (((bFirstTry)||(FileMap->GSMSignalQuality!=GSMSignalQuality))&&(FileMap->GSMSignalQuality!=-1))
			{
			GSMSignalQuality = FileMap->GSMSignalQuality;
			MessageText+="\r\nGSMLVL="+AnsiString(GSMSignalQuality);
			}

		if (((bFirstTry)||(FileMap->ValidatorState!=ValidatorState))&&(FileMap->ValidatorState!=-1))
			{
			ValidatorState = FileMap->ValidatorState;
			MessageText+="\r\nVST="+AnsiString(ValidatorState);
			}

		if (((bFirstTry)||(FileMap->PrinterState!=PrinterState))&&(FileMap->PrinterState!=-1))
			{
			PrinterState = FileMap->PrinterState;
			MessageText+="\r\nPST="+AnsiString(PrinterState);
			}

		if (((bFirstTry)||(FileMap->SW0!=SW0))&&(FileMap->SW0!=-1))
			{
			SW0 = FileMap->SW0;
			MessageText+="\r\nSW1="+AnsiString(SW0);
			}

		if (((bFirstTry)||(FileMap->SW1!=SW1))&&(FileMap->SW1!=-1))
			{
			SW1 = FileMap->SW1;
			MessageText+="\r\nSW2="+AnsiString(SW1);
			}

		if (((bFirstTry)||(FileMap->SW2!=SW2))&&(FileMap->SW2!=-1))
			{
			SW2 = FileMap->SW2;
			MessageText+="\r\nSW3="+AnsiString(SW2);
			}

		if (((bFirstTry)||(FileMap->GSMOperatorID!=GSMOperatorID))&&(FileMap->GSMOperatorID!=-1))
			{
			GSMOperatorID = FileMap->GSMOperatorID;
			MessageText+="\r\nOPER_ID="+AnsiString(GSMOperatorID);
			}

		if (FileMap->CID!=-1)
			{
			MessageText+="\r\nCID="+AnsiString(FileMap->CID);
			MessageText+="\r\nCRESULT=0";
			}

		AToLog+="}...";
		Log->Write(AToLog.c_str());
		AnsiString Answer = Connect(Cfg->StatInfo.HeartBeatURL.c_str(),MessageText);
		std::auto_ptr <TStringList> slResult ( new TStringList() );
		if (!slResult.get())
      {
			Log->Write("  slResult creating Error. ");
			//bResult = false;
			}
			else {
			PrepareAnswer(Answer,slResult.get());

			if (HasAnswerValue(slResult.get(),"ERROR"))
				{
				iError = GetAnswerIntegerValue(slResult.get(),"ERROR");
				Log->Write((boost::format("Error: %1% { %2% }") % iError % GetErrorDescr(iError).c_str()).str().c_str());
				if ((iError==0))
					{
					//bResult=true;
					if (Cfg->StatInfo.Inhibit)
						{
						Cfg->StatInfo.Inhibit=false;
						FileMap->CheckStatDir=true;
						}
						if (FileMap->CID!=-1) {
							if (FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(FileMap->CID)+".pkt")) {
								if (DeleteFile((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(FileMap->CID)+".pkt"))
									Log->Write((boost::format("Command with CID = %1% deleted") % FileMap->CID).str().c_str());
									else
									Log->Write((boost::format("Can't delete file %1%\\%2%.pkt") % Cfg->Dirs.CommandsInbound.c_str() % FileMap->CID).str().c_str());
								}
							if (FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(FileMap->CID)+".ok")) {
								if (DeleteFile((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(FileMap->CID)+".ok"))
									Log->Write((boost::format("Command with CID = %1% processed and deleted") % FileMap->CID).str().c_str());
									else
									Log->Write((boost::format("Can't delete file %1%\\%2%") % Cfg->Dirs.CommandsInbound.c_str() % FileMap->CID).str().c_str());
								}

							if (FileExists(FileMap->CIDFileName)) {
								if (DeleteFile(FileMap->CIDFileName))
									Log->Write((boost::format("File packet %1% processed and deleted") % FileMap->CIDFileName.c_str()).str().c_str());
									else
									Log->Write((boost::format("Can't delete file %1%") % FileMap->CIDFileName.c_str()).str().c_str());
								}
							}
					FileMap->CID=-1;

					//bResult = true;
					AnsiString AParam;
					if (HasAnswerValue(slResult.get(),"COMMAND"))
						{
						iCommand = GetAnswerIntegerValue(slResult.get(),"COMMAND");
						if (iCommand!=-1)
							{
							iCID = GetAnswerIntegerValue(slResult.get(),"CID");
							if (HasAnswerValue(slResult.get(),"PARAMS"))
								{
								AParam = GetAnswerValue(slResult.get(),"PARAMS");
								}
							ReceiveCommand(iCommand, iCID, AParam);
							}
						}
					}
					else                                                                  //Пакет не прошел, откатываем изменения назад.
					{
					WCState = tmpWCState;
					LastPaymentReceived.Val = tmpLastPaymentReceived.Val;
					LastPaymentProcessed.Val = tmpLastPaymentProcessed.Val;
					BillsCount = tmpBillsCount;
					BillsSum = tmpBillsSum;
					SIMBalance = tmpSIMBalance;
					GSMSignalQuality = tmpGSMSignalQuality;
					ValidatorState = tmpValidatorState;
					PrinterState = tmpPrinterState;
					SW0 = tmpSW0;
					SW1 = tmpSW1;
					SW2 = tmpSW2;
					GSMOperatorID = tmpGSMOperatorID;
					}
				}
				else
				{
				Log->Write("Missing error value...");
				}
			}
		}
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		}
	if (Cfg->StatInfo.Inhibit)
		{
		Cfg->StatInfo.Inhibit=false;
		FileMap->CheckStatDir=true;
		}
	return Connected;
}

//---------------------------------------------------------------------------

AnsiString TCSPacketSender::TestConnection(AnsiString URL)
{
return "Check not implemented.";
}

//---------------------------------------------------------------------------

bool TCSPacketSender::SendIncassationMessage()
{
    if (!FileMap)
        return false;
    bool bResult = false;
    AnsiString MessageText;
    long NotesSum = 0;
    int iError;
    if(!StatPacket)
    {
        Log->Write("!StatPacket");
        StatPacket=new TStatPacket(Cfg,Log);
    }
	try
		{
		AnsiString AToLog="  Sending Incassation event {TID: "+AnsiString(TerminalID)+"; EventDateTime: "+StatPacket->EventDateTime+"; Notes: { ";
		MessageText+=("AP="+Cfg->Keys[Cfg->GetKeysNum()].AP+"\r\nDT=").c_str()+(StatPacket->EventDateTime).FormatString("dd.mm.yyyy hh:nn:ss")+"\r\n";
		MessageText+="NOTES=";
		for (std::size_t i=0;i<StatPacket->vNotes.size();i++) {
			MessageText+=AnsiString(StatPacket->vNotes[i].Nominal)+":"+AnsiString(StatPacket->vNotes[i].Count);
			NotesSum+=StatPacket->vNotes[i].Nominal*StatPacket->vNotes[i].Count;
			if(i<StatPacket->vNotes.size()-1)
				MessageText+="|";
			AToLog += (boost::format("{%1% %2% %3% %4%} ") % StatPacket->vNotes[i].ValidatorID % StatPacket->vNotes[i].CurrencyID % StatPacket->vNotes[i].Nominal % StatPacket->vNotes[i].Count).str().c_str();
			}
		MessageText+="\r\nINUM="+StatPacket->SessionNum;
		MessageText+="\r\nSUM="+ChangeChars(FloatToStrF(NotesSum,ffFixed,18,2), ",", ".");
		MessageText+="\r\nCOMM="+ChangeChars(FloatToStrF(StatPacket->Comission,ffFixed,18,2), ",", ".");
		AToLog+="};}...";
		Log->Write(AToLog.c_str());
		AnsiString Answer = Connect(Cfg->StatInfo.IncassationURL.c_str(),MessageText);
		std::auto_ptr <TStringList> slResult ( new TStringList() );
		if (!slResult.get())
			{
			Log->Write("  slResult creating Error. ");
			bResult = false;
			}
			else {
			PrepareAnswer(Answer,slResult.get());

			if (HasAnswerValue(slResult.get(),"ERROR"))
				{
				iError = GetAnswerIntegerValue(slResult.get(),"ERROR");
				Log->Write((boost::format("Error: %1% { %2% }") % iError% GetErrorDescr(iError).c_str()).str().c_str());
				if ((iError<0)||(iError==24)||(iError==30))
          {
					bResult = false;
					}
          else
          {
          bResult = true;
  				Log->Append(" Packet sending completed.");
          }
				}
				else
				{
				Log->Write("Missing error value...");
        bResult = false;
				}
			}
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return bResult;
}

//---------------------------------------------------------------------------

bool TCSPacketSender::SendPaymentCompleteMessage()
{
if (!FileMap)
  return false;
bool bResult = false;
AnsiString MessageText;
//long NotesSum = 0;
int iError;
	try
		{
		AnsiString AToLog="  Sending PaymentComplete event {TID: "+AnsiString(TerminalID)+"; EventDateTime: "+StatPacket->EventDateTime+"; Notes: { ";
		MessageText+=("AP="+Cfg->Keys[Cfg->GetKeysNum()].AP+"\r\nDT=").c_str()+(StatPacket->EventDateTime).FormatString("dd.mm.yyyy hh:nn:ss");
		MessageText+=ChangeChars(ChangeChars(StatPacket->GetParamValue("params"),"\n","\t"),"\t","\r\n");
		MessageText+="\r\nOPERATOR="+AnsiString(StatPacket->OperatorID);
		MessageText+="\r\nERROR="+AnsiString(StatPacket->ErrorCode);
		MessageText+="\r\nSESSION="+StatPacket->InitialSessionNum;
		MessageText+="\r\nPAYMENTINITDT="+StatPacket->InitDT.FormatString("dd.mm.yyyy hh:nn:ss");

		AToLog+="};}...";
		Log->Write(AToLog.c_str());
		AnsiString Answer = Connect(Cfg->StatInfo.PaymentCompleteURL.c_str(),MessageText);
    std::auto_ptr <TStringList> slResult ( new TStringList() );
		if (!slResult.get())
			{
			Log->Write("  slResult creating Error. ");
			bResult = false;
			}
			else {
			PrepareAnswer(Answer,slResult.get());

			if (HasAnswerValue(slResult.get(),"ERROR"))
				{
				iError = GetAnswerIntegerValue(slResult.get(),"ERROR");
				Log->Write((boost::format("Error: %1% { %2% }") % iError % GetErrorDescr(iError).c_str()).str().c_str());
				if ((iError<0)||(iError==24)||(iError==30))
          {
					bResult=false;
					}
          else
          {
          bResult = true;
  				Log->Append(" Packet sending completed.");
          }
				}
				else
				{
				Log->Write("Missing error value...");
				}
			}
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return bResult;
}

//---------------------------------------------------------------------------

bool TCSPacketSender::SendFileViaEMailMessage()
{
if (!FileMap)
  return false;
bool bResult = false;
AnsiString MessageText;
AnsiString TempFileName;
AnsiString RealFileName;
//int iError;
	try
		{
		RealFileName = (StatPacket->PacketFileName.substr(0,StatPacket->PacketFileName.length()-3)+"file").c_str();
		if (!FileExists(RealFileName))
			{
			Log->Append((boost::format("File %1% not found") % RealFileName.c_str()).str().c_str());
			return true;
			}
		std::auto_ptr <TIdSMTP> SMTP ( new TIdSMTP(NULL) );
		if (Cfg->EMailInfo.UserId!="")
			SMTP->AuthenticationType = atLogin;
		SMTP->Host = Cfg->EMailInfo.SMailHost.c_str();
		SMTP->Port = 25;
		SMTP->UserId = Cfg->EMailInfo.UserId.c_str();
		SMTP->Password = Cfg->EMailInfo.Password.c_str();
		if (Cfg->Connection().HTTPProxy.Type.find("socks")!=std::string::npos)
			SMTP->SocksInfo->Assign(Cfg->Connection().Proxy);

		Log->Write((boost::format("  Connecting to %1%:%2%") % SMTP->Host.c_str() % SMTP->Port).str().c_str());
		SMTP->Connect();
		if(SMTP->Connected())
			{
			Log->Append((boost::format("OK. Auth. schemes supported: %1% ") % SMTP->AuthSchemesSupported->DelimitedText.c_str()).str().c_str());
			if (Cfg->EMailInfo.UserId!="")
				{
				Log->Write("  Authenticating...");
				if (SMTP->Authenticate())
					Log->Append("OK.");
					else
					Log->Append("Error!");
				}

			std::auto_ptr <TIdMessage> M ( new TIdMessage(NULL) );
			M->From->Text = Cfg->EMailInfo.FromAddress.c_str();
			M->Recipients->Add();
			M->Recipients->Items[0]->Text = Cfg->EMailInfo.ToFileAddr.c_str();
			M->Subject = StatPacket->SendFileName;
			M->Body->Add("See the attachment file(s)");
			TempFileName = (StatPacket->PacketFileName.substr(0,StatPacket->PacketFileName.length()-3)+"file").c_str();
			RealFileName = (Cfg->Dirs.StatOutboundTemp+"\\").c_str()+StatPacket->SendFileName;
			CopyFile(TempFileName.c_str(), RealFileName.c_str(), false);
			std::auto_ptr <TIdAttachment> att ( new TIdAttachment(M->MessageParts, RealFileName) );
			M->MessageParts->Add();
			M->MessageParts->Items[0] = att.get();
			try
				{
				Log->Write("  Sending message...");
				SMTP->Send(M.get());
				Log->Append("OK.");
				bResult = true;
				}
            catch(...)
            {
                ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                bResult = false;
            }
			DeleteFile(RealFileName);
			}
    if (SMTP->Connected())
      SMTP->Disconnect();
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return bResult;
}

//---------------------------------------------------------------------------

AnsiString TCSPacketSender::Connect(AnsiString URL, AnsiString MessageText)
{
AnsiString Result;
//bool IndyError;
//long NotesSum = 0;
//bool bResult = false;
	try
		{
		Log->Write((boost::format("MessageText: %1%") % PrepareString(MessageText).c_str()).str().c_str());
		//Log->Write("URL: "+URL);

		if (!StatPacket)
			return "";

		AnsiString SignedMessage="inputmessage=" + AnsiString(crypt::sign(Cfg->GetKeysNum(), MessageText.c_str()).c_str());
    if (SignedMessage!="")
      {
  		std::auto_ptr <TConnectThread> Con ( new TConnectThread(Log, Cfg, URL, SignedMessage, 0) );
	  	if (!Con.get())
        {
				Log->Write("  TConnectThread creating Error. ");
				}
				else
				{
				Con->Resume();
				int TimeOut=6000;
				while ((TimeOut>0)&&(!Con->Finished))
          {
					Sleep(10);
					TimeOut--;
					}

				/*IndyError = Con->IndyError;
				if (Con->IndyError)
					{
					Log->Write("Error in Indy library!");
					}*/
        Connected = Con->ConnectOK;
				if (TimeOut<=0)
          {
					Log->Append("Timed out.");
					TerminateThread((HANDLE)Con->Handle,0);
					}
					else
          {
					if (Con->Finished)
            {
						Result = crypt::verify(Cfg->GetKeysNum(),Con->AnswerMessage.c_str()).c_str();
						Log->Write((boost::format("Result = %1%") % PrepareString(Result).c_str()).str().c_str());
						}
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

bool TCSPacketSender::Process(bool bProcessMessages)
{
    UNREFERENCED_PARAMETER(bProcessMessages);
//AnsiString Result;
//AnsiString MessageText;
AnsiString URL;
//bool IndyError;
//long NotesSum = 0;
bool bResult = false;
try
	{
	try
		{

		if (PacketLoadError)
			{
			Log->Write("Error! Packet not loaded.");
			return false;
			}

		if (!StatPacket)
			return false;
		switch (StatPacket->PacketType) {
			case cnIncassation:
				Log->Write("  Sending Incassation message...");
				bResult = SendIncassationMessage();
				break;
			case cnHeartBeat:
				Log->Write("  Sending HeartBeat message...");
				break;
			case cnPaymentComplete:
				Log->Write("  Sending PaymentComplete message...");
				bResult = SendPaymentCompleteMessage();
				break;
			case cnFileSendNew:
				if (Cfg->EMailInfo.GetRecipientAddress(cnETFile)!="")
					{
					Log->Write("  Sending file(s) via e-mail...");
					bResult = SendFileViaEMailMessage();
					}
					else
					{
					Log->Write("  Sending file(s) via e-mail prohibited in config.xml.");
					bResult = true;
					}
				break;
			case cnCommandProcessed:
				Log->Write((boost::format("Processed CID put to FileMap: %1%") % StatPacket->Status).str().c_str());
				FileMap->CID = StatPacket->Status;
				Log->Write((boost::format("Processed CID file name put to FileMap: %1%") % StatPacket->PacketFileName.c_str()).str().c_str());
				FileMap->CIDFileName = StatPacket->PacketFileName.c_str();
				//bResult = true;
				break;
			/*			case cnTestConnection:
				SendTestConnection();
				break;*/
			default:
				Log->Write((boost::format("  Unknown packet type: %1%") % StatPacket->PacketType).str().c_str());
				return false;
			}
		}
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		}
	}
__finally
	{
	return bResult;
	}
}

//---------------------------------------------------------------------------

void TCSPacketSender::PrepareAnswer(AnsiString &ASrc, TStringList *slTrgt)
{
try
    {
    ASrc=PrepareString(ASrc);
    slTrgt->DelimitedText=ASrc;
    //Log->Write(ASrc);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

AnsiString TCSPacketSender::PrepareString(AnsiString ASrc)
{
try
    {
    ASrc=ASrc.TrimRight();
    while (ASrc.Pos("\r\n")>0)
        ASrc=ASrc.SubString(0,ASrc.Pos("\r\n")-1)+"\",\""+ASrc.SubString(ASrc.Pos("\r\n")+2,ASrc.Length());
		ASrc="\""+ASrc+"\"";
		return ASrc;
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return "";
    }
}

//---------------------------------------------------------------------------

AnsiString TCSPacketSender::GetAnswerValue(TStringList *slSrc, AnsiString AName)
{
    try
    {
        return slSrc->Values[AName];
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return "";
    }
}

//---------------------------------------------------------------------------

bool TCSPacketSender::HasAnswerValue(TStringList *slSrc, AnsiString AName)
{
try
		{
		if (slSrc->IndexOfName(AName)==-1)
			 return false;
			 else
			 return true;
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

//---------------------------------------------------------------------------

int TCSPacketSender::GetAnswerIntegerValue(TStringList *slSrc, AnsiString AName)
{
    int result=-1;
    try
    {
        result = GetAnswerValue(slSrc,AName).ToInt();
    }
    catch (EConvertError &ex)
    {
        //Log->Write("Can't get "+AName+" value: {"+ex.Message+"}, setting to -1.");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return result;
}

//---------------------------------------------------------------------------

void TCSPacketSender::ReceiveCommand(int Command, int CID, AnsiString  CmdParam)
{
  AnsiString FileMask;
	try
  {
		std::auto_ptr <TCommandReceiver> CommandReceiver ( new TCommandReceiver("",Cfg,Log,FileMap) );
		switch (Command)
    {
      case cnCmdReboot:
        Log->Write((boost::format("Reboot requested, CID = %1%") % CID).str().c_str());
        CommandReceiver->StoreRebootCommand(CID);
        break;
      case cnCmdSendFile:
        Log->Write("File requested: ");
        FileMask=CmdParam.Trim();
        Log->Append((boost::format("%1%, CID = %2%") % FileMask.c_str() % CID).str().c_str());
        CommandReceiver->StoreGetFileByMaskCommand(CID,FileMask);
        break;
      case cnHTTPFileRequest:
        Log->Write("File receiving requested via HTTP: ");
        FileMask=CmdParam.Trim();
        Log->Append((boost::format("%1%, CID = %2%") % FileMask.c_str() % CID).str().c_str());
        CommandReceiver->StoreHTTPFileRequestCommand(CID,FileMask);
        break;
      default:
        Log->Write((boost::format("Unknown command received. Command: %1%; Param %2%") % Command % CmdParam.c_str()).str().c_str());
		}
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

AnsiString TCSPacketSender::GetErrorDescr(int val)
{
  AnsiString res = "";
  switch (val)
  {
    case 1:
      res = "Сессия с таким номером уже существует";
      break;
    case 2:
      res = "Неправильный код дилера";
      break;
    case 3:
      res = "Неправильный код точки приема";
      break;
    case 4:
      res = "Неправильный код оператора";
      break;
    case 5:
      res = "Неправильный формат кода сессии";
      break;
    case 6:
      res = "Неправильная ЭЦП";
      break;
    case 7:
      res = "Неправильный формат суммы или значение суммы вне допустимого диапазона";
      break;
    case 8:
      res = "Неправильный формат номера телефона";
      break;
    case 9:
      res = "Неправильный формат номера лицевого счета";
      break;
    case 10:
      res = "Неправильный формат документа";
      break;
    case 11:
      res = "Сессия с таким номером не существует";
      break;
    case 12:
      res = "Запрос сделан с другого IP";
      break;
    case 21:
      res = "Не хватает средств на счете дилера для проведения платежа";
      break;
    case 22:
      res = "Не прошел CyberCheck (списание средств со счета дилера)";
      break;
    case 23:
      res = "Не прошел платеж у оператора связи (нет такого телефона)";
      break;
    case 24:
      res = "Невозможно связаться с сервером  оператора связи(технологический перерыв)";
      break;
    case 26:
      res = "Невозможно связаться с сервером оператора связи (технологический перерыв)";
      break;
    case 30:
      res = "Общая ошибка системы (CyberPlat)";
      break;
    case 32:
      res = "Повторный платеж в течение 60 минут с момента окончания платежа (CyberPlat)";
      break;
  }
  return res;
}

//---------------------------------------------------------------------------

