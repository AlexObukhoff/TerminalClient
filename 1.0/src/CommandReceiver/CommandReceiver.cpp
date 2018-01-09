//---------------------------------------------------------------------------


//#pragma hdrstop

#include "CommandReceiver.h"
#include "PacketSender.h"
#include "CSPacketSender.h"
#include "SSPacketSender.h"
#include "TPayment.h"
#include "SevenZipVCL.hpp"
#include "common.h"
#include "globals.h"
#include "boost/format.hpp"

//---------------------------------------------------------------------------

const bool bNoClearDir = false;
const bool bClearDir = true;

#pragma package(smart_init)

TCommandReceiver::TCommandReceiver(AnsiString _fileName, TWConfig* _Cfg, TLogClass* _Log, TFileMap* _FileMap)
{
try
    {
		PacketLoadError = false;

/*		if (_Cfg->StatInfo->Host.LowerCase()=="none") {
				Enabled=false;
				return;
				}*/
		//CoInitializeEx(NULL, COINIT_MULTITHREADED);
    Cfg = _Cfg;
		FileMap = _FileMap;
    InnerLog=false;
    if (_Log==NULL) {
            Log = new TLogClass("TCommandReceiver");
            InnerLog=true;
            }
						else
						Log=_Log;

		XMLP = NULL;
		XMLP = new TXMLPacket(Cfg, Log);
		if (_fileName!="") {
				if (!XMLP->LoadFromFile(_fileName))
					PacketLoadError = true;
				}
		LastUpdatedDT.Val = 0;
		Enabled=true;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		PacketLoadError = true;
		}
}

//---------------------------------------------------------------------------

TCommandReceiver::~TCommandReceiver()
{
if (!Enabled)
    return;
//CoUninitialize();
Clear();
if (XMLP)
		delete XMLP;
if (InnerLog)
    delete Log;
}

//---------------------------------------------------------------------------

void TCommandReceiver::Clear()
{
try
    {
    if (!Enabled)
        return;
    XMLP->Clear();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TCommandReceiver::StoreCommand(int Command, int CommandUID)
{
try
    {
    if (!Enabled)
        return;
    XMLP->AddParamI("Command",Command);
    XMLP->AddParamI("CommandUID",CommandUID);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

bool TCommandReceiver::StoreRebootCommand(int CommandUID)
{
try
    {
    if (!Enabled)
				return false;
		if (CommandAlreadyRegistered(CommandUID))
        return true;
    StoreCommand(cnCmdReboot, CommandUID);
		return Store();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		return false;
		}
}

//---------------------------------------------------------------------------

void TCommandReceiver::StoreInhibitSendingCommand(int CommandUID)
{
try
		{
    if (!Enabled)
        return;
    if (CommandAlreadyRegistered(CommandUID))
        return;
		StoreCommand(cnCmdInhibitPktSend, CommandUID);
    Store();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TCommandReceiver::StoreReceiveFileCommand(int CommandUID, AnsiString FileName, int FileSize, void* Buffer)
{
    UNREFERENCED_PARAMETER(FileSize);
    UNREFERENCED_PARAMETER(Buffer);
try
  {
  if (!Enabled)
      return;
  if (CommandAlreadyRegistered(CommandUID))
      return;
  StoreCommand(cnCmdReceiveFile, CommandUID);
  XMLP->AddParam("FileName",FileName);
  Store();
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TCommandReceiver::StoreSendFileCommand(int CommandUID, AnsiString FileName)
{
try
    {
    if (!Enabled)
        return;
    if (CommandAlreadyRegistered(CommandUID))
        return;
    StoreCommand(cnCmdSendConfigOld, CommandUID);
		XMLP->AddParam("FileName",FileName);
    Store();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------
//    void StoreCancelPaymentCommand(int CommandUID, AnsiString SessionNumber);

void TCommandReceiver::StoreCancelPaymentCommand(int CommandUID, AnsiString SessionNumber)
{
try
    {
    if (!Enabled)
        return;
    if (CommandAlreadyRegistered(CommandUID))
        return;
    StoreCommand(cnCmdCancelPayment, CommandUID);
		XMLP->AddParam("SessionNumber",SessionNumber);
    Store();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TCommandReceiver::StoreGetFileByMaskCommand(int CommandUID, AnsiString _Mask)
{
    if (!Enabled)
        return;
    if (CommandAlreadyRegistered(CommandUID))
        return;

    try
    {
        AnsiString typeParam = _Mask.SubString(0, _Mask.Pos(":") - 1).LowerCase().Trim();
        AnsiString maskParam = _Mask.SubString(_Mask.Pos(":") + 1, _Mask.Length()).LowerCase().Trim();

        StoreCommand(cnCmdSendFile, CommandUID);
        //XMLP->AddParam("Type", (_Mask.SubString(0,_Mask.Pos(":")-1)).LowerCase());
        //XMLP->AddParam("Mask", (_Mask.SubString(_Mask.Pos(":")+1,_Mask.Length())).LowerCase());
        XMLP->AddParam("Type", typeParam);
        XMLP->AddParam("Mask", maskParam);
        Log->Write((boost::format("  File request stored. { %1% | %2% }") % typeParam.c_str() % maskParam.c_str()).str().c_str());
        Store();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------
//    void StoreCancelPaymentCommand(int CommandUID, AnsiString SessionNumber);

void TCommandReceiver::StoreResurrectPaymentCommand(int CommandUID, AnsiString SessionNumber, AnsiString Parameters)
{
try
    {
    if (!Enabled)
        return;
    if (CommandAlreadyRegistered(CommandUID))
        return;
		StoreCommand(cnCmdResurrectPayment, CommandUID);
    XMLP->AddParam("SessionNumber",SessionNumber);
		XMLP->AddParam("Parameters",Parameters);
		Store();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

bool TCommandReceiver::CommandAlreadyRegistered(int CommandUID)
{
try
    {
		if (!Enabled)
        return false;
    if (FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".pkt")) {
				Log->Append((boost::format(" Command with CommandUID = %1% already registered") % CommandUID).str().c_str());
        return true;
        }
    if (FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".gk")) {
				Log->Append((boost::format(" Command with CommandUID = %1% already registered, waiting for the WebClient to process it...") % CommandUID).str().c_str());
        return true;
        }
    if (FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".ok")) {
        Log->Append((boost::format(" Command with CommandUID = %1% already registered and processed.") % CommandUID).str().c_str());
        return true;
        }
		return false;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		return false;
    }
}

//---------------------------------------------------------------------------

bool TCommandReceiver::StoreHTTPFileRequestCommand(int CommandUID, AnsiString _Mask, TDateTime _FileCheckDT)
{
try
		{
		if (!Enabled)
				return false;
		if (CommandAlreadyRegistered(CommandUID))
				return false;
		StoreCommand(cnHTTPFileRequest, CommandUID);
		XMLP->AddParam("FileName", (_Mask.SubString(0,_Mask.Pos("|")-1)).LowerCase());
		XMLP->AddParam("URL", (_Mask.SubString(_Mask.Pos("|")+1,_Mask.Length())).LowerCase());
		XMLP->AddParam("FileCheckDT", AnsiString(_FileCheckDT));
		XMLP->AddParam("FirstTryDT",TDateTime::CurrentDateTime());
		Log->Write((boost::format("  HTTP file request stored. { %1% | %2% }") % XMLP->GetParamValue("FileName").c_str() % XMLP->GetParamValue("URL").c_str()).str().c_str());
		return Store();
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		return false;
		}
}

//---------------------------------------------------------------------------

bool TCommandReceiver::StoreFullURLHTTPFileRequestCommand(int CommandUID, AnsiString _FileName, AnsiString _URL)
{
try
		{
		if (!Enabled)
				return false;
		if (CommandAlreadyRegistered(CommandUID))
				return false;
		StoreCommand(cnFullURLHTTPFileRequest, CommandUID);
		XMLP->AddParam("FileName", _FileName);
		XMLP->AddParam("URL", _URL);
		XMLP->AddParam("FirstTryDT",TDateTime::CurrentDateTime());
		Log->Write((boost::format("  Full URL HTTP file request stored. { %1% | %2% }") % XMLP->GetParamValue("FileName").c_str() % XMLP->GetParamValue("URL").c_str()).str().c_str());
		return Store();
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		return false;
		}
}

//---------------------------------------------------------------------------

bool TCommandReceiver::StoreShutDownCommand(int CommandUID)
{
try
    {
    if (!Enabled)
				return false;
		if (CommandAlreadyRegistered(CommandUID))
        return true;
    StoreCommand(cnCmdShutDown, CommandUID);
		return Store();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		return false;
		}
}

//---------------------------------------------------------------------------

bool TCommandReceiver::StoreBlockCommand(int CommandUID)
{
try
    {
    if (!Enabled)
				return false;
		if (CommandAlreadyRegistered(CommandUID))
        return true;
    StoreCommand(cnCmdBlock, CommandUID);
		return Store();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		return false;
		}
}

//---------------------------------------------------------------------------

bool TCommandReceiver::StoreUnblockCommand(int CommandUID)
{
try
    {
    if (!Enabled)
				return false;
		if (CommandAlreadyRegistered(CommandUID))
        return true;
    StoreCommand(cnCmdUnblock, CommandUID);
		return Store();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		return false;
		}
}

//---------------------------------------------------------------------------

void TCommandReceiver::StoreGetKeysCommand(int CommandUID, int KeysId, AnsiString Parameters)
{
try
    {
    if (!Enabled)
        return;
    if (CommandAlreadyRegistered(CommandUID))
        return;
		StoreCommand(cnCmdGetKeys, CommandUID);
    XMLP->AddParam("KeysId",AnsiString(KeysId));
		XMLP->AddParam("Parameters",Parameters);
		Store("gk");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

bool TCommandReceiver::Store(AnsiString Ext)
{
bool bRes = false;
try
  {
  if (!Enabled)
    return false;
  bRes = XMLP->StoreToFile((Cfg->Dirs.CommandsInbound+"\\").c_str()+XMLP->GetParamValue("CommandUID")+"."+Ext, false);
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  bRes = false;
  }
return bRes;
}

//---------------------------------------------------------------------------

bool TCommandReceiver::UpdateFile(AnsiString SourceFileName, AnsiString TargetFileName)
{
try
  {
  AnsiString OldFileName=TargetFileName.SubString(0,TargetFileName.Length()-3)+"~"+TargetFileName.SubString(TargetFileName.Length()-2,3);
  if (FileExists(OldFileName))
     if (!DeleteFile(OldFileName))
        Log->Write((boost::format("Error deleting %1%") % OldFileName.c_str()).str().c_str());
  if (!RenameFile(TargetFileName, OldFileName))
    Log->Write((boost::format("Error renaming %1% to %2%") % TargetFileName.c_str() % OldFileName.c_str()).str().c_str());
  if (!RenameFile(SourceFileName,TargetFileName)) {
    Log->Write((boost::format("Error renaming %1% to %2%") % SourceFileName.c_str() % TargetFileName.c_str()).str().c_str());
    return false;
    }
    else {
		Log->Write((boost::format("File %1% updated by %2%") % TargetFileName.c_str() % SourceFileName.c_str()).str().c_str());
		return true;
    }
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  return false;
	}
}
//---------------------------------------------------------------------------

bool TCommandReceiver::StoreFile(AnsiString SourceFileName, AnsiString TargetFileName)
{
try
  {
  if (FileExists(TargetFileName))
     if (!DeleteFile(TargetFileName))
        Log->Write((boost::format("Error deleting %1%") % TargetFileName.c_str()).str().c_str());
  if (!RenameFile(SourceFileName,TargetFileName)) {
    Log->Write((boost::format("Error renaming %1% to %2%") % SourceFileName.c_str() % TargetFileName.c_str()).str().c_str());
    return false;
    }
		else {
		Log->Write((boost::format("File %1% stored to %2%") % SourceFileName.c_str() % TargetFileName.c_str()).str().c_str());
    return true;
    }
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
	return false;
	}
}

//---------------------------------------------------------------------------

bool TCommandReceiver::Process()
{
    TPacketSender *PS = NULL;
    bool bResult = false;
    TPayment *Payment = NULL;
    AnsiString PaymentFileName;
    AnsiString IncomingFileName;
    int GetFileResult;
    TDateTime FirstTryDT;
    try
    {
        try
        {
            if (PacketLoadError)
            {
                Log->Write("Error! Packet not loaded.");
                return false;
            }

            if (!Enabled)
                return false;
            int Command=GetInt(XMLP->GetParamValue("Command"));
            int CommandUID=GetInt(XMLP->GetParamValue("CommandUID"));

            if (Cfg->StatInfo.ProcessorType==cnCyberPlatServer)
                PS = new TCSPacketSender("", Cfg, Log, FileMap);
            else
                PS = new TSSPacketSender("", Cfg, Log, FileMap);

            switch (Command)
            {
                case cnCmdReboot:
                    Log->Write("  Reboot requested.");
                    Cfg->StatInfo.Inhibit=false;
                    PS->StoreSendCommandProcessedTemp(CommandUID);
                    bResult = true;
                    break;
                case cnHTTPFileRequest:
                case cnFullURLHTTPFileRequest:
                    Log->Write((boost::format("  File download requested, File name: %1%, URL: %2%") % XMLP->GetParamValue("FileName").c_str() % XMLP->GetParamValue("URL").c_str()).str().c_str());
                    try
                    {
                        FirstTryDT = TDateTime(XMLP->GetParamValue("FirstTryDT"));
                    }
                    catch(...)
                    {
                        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                    }
                    if (FirstTryDT+1<TDateTime::CurrentDateTime())
                    {
                        Log->Write((boost::format("Download file timeout. First try: %1%") % AnsiString(FirstTryDT).c_str()).str().c_str());
                        XMLP->AddParamI("Command",cnCmdNone);
                        bResult = true;
                        break;
                    }

                    IncomingFileName=XMLP->GetParamValue("FileName");
                    GetFileResult = GetFile(((Command == cnHTTPFileRequest ? Cfg->StatInfo.DownloadURLPrefix.c_str() : "") + XMLP->GetParamValue("URL")).c_str(),IncomingFileName,AnsiString(CommandUID),TDateTime(GetDateTime(XMLP->GetParamValue("FileCheckDT"))));
                    if (GetFileResult == GetFileError)
                    {
                        bResult = false;
                        break;
                    }
                    else
                    {
                        if (GetFileResult == GetFileCheckFailed)
                        {
                            XMLP->AddParamI("Command",cnCmdNone);
                            bResult = true;
                            break;
                        }
                    }

                case cnCmdReceiveFileOld:
                case cnCmdReceiveFile:
                        Log->Write((boost::format("  File '%1%'") % XMLP->GetParamValue("FileName").c_str()).str().c_str());
                        Cfg->StatInfo.Inhibit=false;

                        if ((XMLP->GetParamValue("FileName").LowerCase()).Pos(".7z")>0) {
                            ListFile((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".file");
                        if (ExtractFile((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".file",(Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)))
                            Log->Write((boost::format("%1%\\%2% file extracted ") % Cfg->Dirs.CommandsInbound.c_str() % CommandUID).str().c_str());

                        if (XMLP->GetParamValue("FileName").LowerCase()=="update.7z")
                        {
                            CopyDir((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID), GetDrive()+"\\webupdate\\update", bClearDir);
                        }
                        else
                        {
                            if (XMLP->GetParamValue("FileName").LowerCase()=="updater.7z")
                            {
                                CopyDir((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID), GetDrive()+"\\webupdate", bNoClearDir);
                            }
                            else
                            {
                                if (XMLP->GetParamValue("FileName").LowerCase()=="interface.7z")
                                {
                                    std::string strtmp=((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)).c_str();
                                    CopyDir(strtmp.c_str(), Cfg->Dirs.InterfaceDir.c_str(), bNoClearDir);
                                    if (FileExists((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(-1)+"\\db_numcapacity.js"))
                                    {
                                        FileMap->NumDBUpdateDone = true;
                                        FileMap->NumDBLastUpdatedDT = LastUpdatedDT;
                                        XMLP->AddParamI("Command",cnCmdNone);
                                    }
                                }
                                else
                                {
                                    if (XMLP->GetParamValue("FileName").LowerCase()=="operators.7z")
                                    {
                                        CopyDir((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID), (Cfg->Dirs.WorkDir+"config").c_str(), bNoClearDir);
                                    }
                                    else
                                    {
                                        TSearchRec sr;
                                        int iAttributes = 0;
                                        if (FindFirst((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+"\\*.*", iAttributes, sr) == 0)
                                        {
                                            do
                                            {
                                                ProcessFile((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+"\\"+sr.Name,sr.Name);
                                            }
                                            while (FindNext(sr) == 0);
                                            FindClose(sr);
                                        }
                                    }
                                }
                            }
                        }
                        DeleteDir((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID));
                        DeleteFile((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".file");
                    }
                    else
                    {
                        ProcessFile((Cfg->Dirs.CommandsInbound+"\\").c_str()+AnsiString(CommandUID)+".file",XMLP->GetParamValue("FileName"));
                    }
                    PS->StoreCommandProcessed(CommandUID);
                    bResult = true;
                    break;
                case cnCmdSendConfigOld:
                    Log->Write("  Config files requested from server.");
                    Cfg->StatInfo.Inhibit=false;
                    //PS->StoreFilesSend(Cfg->Dirs->WorkDir+"config\\config.xml","config");
                    //PS->StoreFilesSend(Cfg->Dirs->WorkDir+"config\\config.xml","operators");
                    PS->StoreFileSend((Cfg->Dirs.WorkDir+"config\\config.xml").c_str());
                    PS->StoreFileSend((Cfg->Dirs.WorkDir+"config\\operators.xml").c_str());
                    PS->StoreFileSend((Cfg->Dirs.WorkDir+"config\\menu.xml").c_str());
                    PS->StoreCommandProcessed(CommandUID);
                    bResult = true;
                    break;
                case cnCmdCancelPayment:
                    Log->Write("  Cancel payment command received.");
                    Cfg->StatInfo.Inhibit=false;
                    PaymentFileName=(Cfg->Dirs.PaymentsOutboundTemp+"\\out-").c_str()+XMLP->GetParamValue("SessionNumber")+".pkt";
                    if (!FileExists(PaymentFileName))
                    {
                        PaymentFileName=(Cfg->Dirs.PaymentsOutbound+"\\out-").c_str()+XMLP->GetParamValue("SessionNumber")+".pkt";
                        if (!FileExists(PaymentFileName))
                        {
                            PaymentFileName=(Cfg->Dirs.PaymentsOutbound+"\\out-").c_str()+XMLP->GetParamValue("SessionNumber")+".tmp";
                            if (!FileExists(PaymentFileName))
                            {
                                PaymentFileName=(Cfg->Dirs.PaymentsOutboundTemp+"\\ext-").c_str()+XMLP->GetParamValue("SessionNumber")+".pkt";
                                if (!FileExists(PaymentFileName))
                                {
                                    PaymentFileName=(Cfg->Dirs.PaymentsOutbound+"\\ext-").c_str()+XMLP->GetParamValue("SessionNumber")+".pkt";
                                    if (!FileExists(PaymentFileName))
                                    {
                                        PaymentFileName=(Cfg->Dirs.PaymentsOutbound+"\\ext-").c_str()+XMLP->GetParamValue("SessionNumber")+".tmp";
                                        if (!FileExists(PaymentFileName))
                                        {
                                            Log->Write("Can't find payment packet.");
                                            PS->StoreCommandProcessed(CommandUID);
                                            bResult = true;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    Log->Write((boost::format("Payment file found: %1%") % PaymentFileName.c_str()).str().c_str());
                    Payment = NULL;
                    Payment = new TPayment(PaymentFileName, Cfg, Log, FileMap, NULL);
                    if (Payment)
                    {
                        Payment->CancelPayment();
                        PS->StoreCommandProcessed(CommandUID);
                        bResult = true;
                    }
                    break;
                case cnCmdSendFile:
                    Log->Append((boost::format("  Send file request: { %1% | %2% }") % XMLP->GetParamValue("Type").c_str() % XMLP->GetParamValue("Mask").c_str()).str().c_str());
                    Cfg->StatInfo.Inhibit=false;
                    if (XMLP->GetParamValue("Type")=="log")
                    {
                        if (CopyDir((Cfg->Dirs.WorkDir+"logs").c_str(), (Cfg->Dirs.WorkDir+"logs.temp").c_str(), bClearDir))
                        {
                            CopyDir("\\webupdate\\logs", (Cfg->Dirs.WorkDir+"logs.temp").c_str(), bNoClearDir);
                            PS->StoreFilesSend(GetFileNames(XMLP->GetParamValue("Mask")),"log-"+XMLP->GetParamValue("Mask"));
                            DeleteDir((Cfg->Dirs.WorkDir+"logs.temp").c_str());
                        }
                        else
                        {
                            Log->Write("  Error copying log directory!");
                        }
                    }
                    else
                    {
                        if (XMLP->GetParamValue("Type")=="cfg")
                        {
                            if (CopyDir((Cfg->Dirs.WorkDir+"config").c_str(), (Cfg->Dirs.WorkDir+"config.temp").c_str(), bNoClearDir))
                            {
                                PS->StoreFilesSend(GetFileNames(XMLP->GetParamValue("Mask")),"log-"+XMLP->GetParamValue("Mask"));
                                PS->StoreFileSend((Cfg->Dirs.WorkDir+"config.temp\\config.xml").c_str());
                                PS->StoreFileSend((Cfg->Dirs.WorkDir+"config.temp\\operators.xml").c_str());
                                DeleteDir((Cfg->Dirs.WorkDir+"config.temp").c_str());
                            }
                            else
                            {
                                Log->Write("  Error copying cfg directory!");
                            }
                        }
                    }
                    bResult = true;
                    PS->StoreCommandProcessed(CommandUID);
                    break;
                case cnCmdResurrectPayment:
                    Log->Write("  Resurrect payment command received.");
                    Cfg->StatInfo.Inhibit=false;
                    PaymentFileName=(Cfg->Dirs.PaymentsUnprocessed+"\\out-").c_str()+XMLP->GetParamValue("SessionNumber")+".pkt";
                    if (!FileExists(PaymentFileName))
                    {
                        Log->Write("Can't find payment packet in the unprocessed folder, command failed.");
                        PS->StoreCommandProcessed(CommandUID);
                        bResult = true;
                        break;
                    }

                    Log->Write((boost::format("Payment file found: %1%") % PaymentFileName.c_str()).str().c_str());
                    Payment = NULL;
                    Payment = new TPayment(PaymentFileName, Cfg, Log, FileMap, NULL);
                    if (Payment)
                    {
                        if (Payment->ResurrectPayment(AnsiString("?")+XMLP->GetParamValue("Parameters")))
                        {
                            delete Payment;
                            Payment = NULL;
                            if (!RenameFile(PaymentFileName,(Cfg->Dirs.PaymentsOutboundTemp+"\\out-").c_str()+XMLP->GetParamValue("SessionNumber")+".pkt"))
                                Log->Write("Can't rename payment packet, command failed.");
                        }
                        else
                        {
                            Log->Write("Can't resurrect payment, command failed.");
                        }
                        PS->StoreCommandProcessed(CommandUID);
                        bResult = true;
                    }
                    break;

                case cnCmdShutDown:
                    Log->Write("  ShutDown requested.");
                    Cfg->StatInfo.Inhibit=false;
                    PS->StoreCommandProcessed(CommandUID);
                    bResult = true;
                    break;

                case cnCmdBlock:
                    Log->Write("  Block requested.");
                    Cfg->StatInfo.Inhibit=false;
                    PS->StoreCommandProcessed(CommandUID);
                    bResult = true;
                    break;

                case cnCmdUnblock:
                    Log->Write("  Unblock requested.");
                    Cfg->StatInfo.Inhibit=false;
                    PS->StoreCommandProcessed(CommandUID);
                    bResult = true;
                    break;
                case cnCmdGetKeys:
                    Log->Write("  GetKey command processed.");
                    PS->StoreCommandProcessed(CommandUID);
                    bResult = true;
                    break;

                default:
                    Log->Write((boost::format("  Unknown command # %1% recieved") % Command).str().c_str());
            }
        }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            if (PS!=NULL)
            {
                delete PS;
                PS=NULL;
            }
            if (Payment!=NULL)
            {
                delete Payment;
                Payment = NULL;
            }
            return bResult;
        }
    }
    __finally
    {
        if (PS!=NULL)
        {
            delete PS;
            PS=NULL;
        }
        if (Payment)
        {
            delete Payment;
            Payment = NULL;
        }
        return bResult;
    }
}

//---------------------------------------------------------------------------

AnsiString TCommandReceiver::GetFileNames(AnsiString _Mask)
{
AnsiString Result;
TStringList *slNames = NULL;
try
  {
  try
    {
		slNames = new TStringList();
    slNames->Clear();
    TSearchRec sr;
    int iAttributes = 0;
    AnsiString FindDir=(Cfg->Dirs.WorkDir+"logs.temp\\*.*").c_str();
    if (FindFirst(FindDir, iAttributes, sr) == 0) {
				do {
           if (sr.Name.Pos(_Mask)!=0)
             {
						 slNames->Add((Cfg->Dirs.WorkDir+"logs.temp\\").c_str()+sr.Name);
             }
            } while (FindNext(sr) == 0);
        FindClose(sr);
        }
    Result = slNames->DelimitedText;
		}
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        }
  }
__finally
  {
  if (slNames!=NULL) {
    delete slNames;
		slNames = NULL;
    }
	return Result;
  }
}

//---------------------------------------------------------------------------

void TCommandReceiver::ProcessFile(AnsiString RealFileName, AnsiString FileName)
{
try
	{
	Log->Write((boost::format("Processing %1% from %2%") % FileName.c_str() % RealFileName.c_str()).str().c_str());
	if (XMLP->GetParamValue("FileName").LowerCase()=="updater.7z")
		{
		if (!DirectoryExists("\\webupdate\\"))
			if (!ForceDirectories("\\webupdate\\")) {
				Log->Append(" Error creating \\webupdate\\ directory!");
				}
		StoreFile(RealFileName,"\\webupdate\\"+FileName);
		}
		else
		{
		if (XMLP->GetParamValue("FileName").LowerCase()=="interface.7z")
			{
			StoreFile(RealFileName,(Cfg->Dirs.InterfaceDir+"\\").c_str()+FileName);
			}
			else
			{
			if ((FileName=="config.xml")||(FileName=="operators.xml"))
				UpdateFile(RealFileName,(Cfg->Dirs.WorkDir+"config\\").c_str()+FileName);
				else
				{
				if ((FileName=="update.exe")||(FileName=="updater.exe"))
					StoreFile(RealFileName,"\\webupdate\\"+FileName);
					else
					{
					Log->Write((boost::format("Unknown file found: %1%") % FileName.c_str()).str().c_str());
					}
				}
			}
		}
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

int TCommandReceiver::GetCommand()
{
try
    {
    if (!Enabled)
        return 0;
		return (XMLP->GetParamValue("Command")).ToInt();
//return Command;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return 0;
    }
}

//---------------------------------------------------------------------------

bool TCommandReceiver::ExtractFile(AnsiString SourceFileName, AnsiString TargetDirName)
{
bool bResult = false;
	try
		{
		if (!Enabled)
			return false;
    std::auto_ptr <TSevenZip> SZ ( new TSevenZip(NULL) );
		SZ->SZFileName=WideString(SourceFileName);
		SZ->ExtrBaseDir = TargetDirName;
		SZ->ExtractOptions = SZ->ExtractOptions << ExtractOverwrite;
		SZ->Files->Clear();
		SZ->Extract();
		if (SZ->ErrCode==0) {
			bResult=true;
			}
			else
			Log->Write((boost::format("Error extracting file %1% into %2%") % SourceFileName.c_str() % TargetDirName.c_str()).str().c_str());
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        bResult = false;
    }
    return bResult;
}

//---------------------------------------------------------------------------

bool TCommandReceiver::ListFile(AnsiString SourceFileName)
{
  bool bResult = false;
	try
		{
		if (!Enabled)
			return false;
		Log->Write("  Archive contents:");
    std::auto_ptr <TSevenZip> SZ ( new TSevenZip(NULL) );
 		SZ->OnListfile = SZListFileEvent;
		SZ->SZFileName=WideString(SourceFileName);
		SZ->List();
		if (SZ->ErrCode==0) {
			bResult=true;
			}
			else
			Log->Write((boost::format("Error listing file %1%") % SourceFileName.c_str()).str().c_str());
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        bResult = false;
    }
    return bResult;
}

//---------------------------------------------------------------------------

void __fastcall TCommandReceiver::SZListFileEvent(System::TObject* Sender, WideString Filename, unsigned Fileindex, unsigned FileSizeU, unsigned FileSizeP, unsigned Fileattr, unsigned Filecrc, WideString Filemethod, double FileTime)
{
    UNREFERENCED_PARAMETER(Sender);
    UNREFERENCED_PARAMETER(Fileindex);
    UNREFERENCED_PARAMETER(FileSizeP);
    UNREFERENCED_PARAMETER(Fileattr);
    Log->Write((boost::format("%1%, %2%, %3%, CRC: %4%") % AnsiString(Filename).c_str() % FileSizeU % AnsiString(TDateTime(FileTime)).c_str() % Filecrc).str().c_str());
}

//---------------------------------------------------------------------------

bool TCommandReceiver::DeleteDir(AnsiString DirName)
{
try
	{
	Log->Write((boost::format("Deleting %1%") % DirName.c_str()).str().c_str());
	char From[MAX_PATH];
	ZeroMemory(From, sizeof(From));
	strcat(From,DirName.c_str());
	strcat(From,"\0\0");
	SHFILEOPSTRUCT op;
  ZeroMemory(&op, sizeof(op));
  op.hwnd = NULL;//Application->Handle;
  op.wFunc = FO_DELETE;
  op.pFrom = From;
  op.fFlags = FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_NOERRORUI|FOF_SILENT;
  if (!SHFileOperation(&op))
    {
    Log->Append(" OK.");
    return true;
    }
    else {
    Log->Append(ShowError("Error deleting dir").c_str());
    return false;
    }
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

//---------------------------------------------------------------------------

/*AnsiString TCommandReceiver::ShowError(AnsiString Header)
{
LPVOID lpMsgBuf;
AnsiString Temp;
try
	{
	try
		{
		int ErrorCode = GetLastError();
		if (ErrorCode!=0) {
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,NULL,ErrorCode,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0,NULL);
			Temp=Header+": "+AnsiString(ErrorCode)+" "+AnsiString((char*)lpMsgBuf);
			}
		}
	catch (Exception &exception)
		{
		Log->WriteInLine("Exception in TCommandReceiver.ShowError: "+exception.Message);
		}
	}
__finally
	{
	LocalFree(lpMsgBuf);
	return Temp;
	}
}*/

//---------------------------------------------------------------------------

bool TCommandReceiver::CopyDir(AnsiString aFrom, AnsiString aTo, bool bClearDir)
{
try
  {
	if ((bClearDir)&&(DirectoryExists(aTo)))
		if (!DeleteDir(aTo)) {
				Log->Append((boost::format(" Error deleting directory %1%") % aTo.c_str()).str().c_str());
				return false;
				}
				else
				Log->Append((boost::format(" Directory %1% deleted") % aTo.c_str()).str().c_str());

	if (!ForceDirectories(aTo)) {
			Log->Append((boost::format(" Error creating %1% directory") % aTo.c_str()).str().c_str());
			return false;
			}

  Log->Write((boost::format("  Copying dir %1% to %2%") % aFrom.c_str() % aTo.c_str()).str().c_str());
  aFrom+="\\*.*";
  char From[MAX_PATH];
	ZeroMemory(From, sizeof(From));
  strcat(From,aFrom.c_str());
  strcat(From,"\0\0");

  char To[MAX_PATH];
  ZeroMemory(To, sizeof(To));
  strcat(To,aTo.c_str());
  strcat(To,"\0\0");

  SHFILEOPSTRUCT op;
  ZeroMemory(&op, sizeof(op));
  op.hwnd = Application->Handle;
  op.wFunc = FO_COPY;
  op.pFrom = From;
  op.pTo = To;
  op.fFlags = FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_NOERRORUI|FOF_SILENT;
  if (!SHFileOperation( &op))
      {
      Log->Append(" OK.");
      return true;
      }
			else {
      Log->Append(ShowError(" Error copying files: ").c_str());
      return false;
      }
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

//---------------------------------------------------------------------------

AnsiString TCommandReceiver::GetDrive(void)
{
  char path_buf[_MAX_PATH];
  char drive[_MAX_DRIVE];
  char dir[_MAX_DIR];
	char fname[_MAX_FNAME];

	::GetModuleFileName(NULL,path_buf,sizeof path_buf);
	_splitpath(path_buf,drive,dir,fname,0);
	_makepath (path_buf,drive,0,0,0);
	return AnsiString(path_buf);
}

//---------------------------------------------------------------------------

int TCommandReceiver::GetFile(AnsiString URL, AnsiString FileName, AnsiString CUID, TDateTime CheckDT)
{
int Result;
	try
		{
		std::auto_ptr <THTTPGetFileThread> HTTPGetFile ( new THTTPGetFileThread(Log, Cfg, URL, FileName, CUID, CheckDT) );
		if (!HTTPGetFile.get()) {
			Log->Write("  THTTPGetFileThread creating Error. ");
			}
			else
			{
			HTTPGetFile->Resume();
			int TimeOut=54000;
			while ((TimeOut>0)&&(!HTTPGetFile->Finished)) {
					Sleep(10);
					TimeOut--;
					}

			if (TimeOut<=0) {
				Log->Append(" Timed out...");
				TerminateThread((HANDLE)HTTPGetFile->Handle,0);
				Result = false;
				}
				else {
				if (HTTPGetFile->Finished) {
					Result = HTTPGetFile->Result;
					LastUpdatedDT = HTTPGetFile->LastUpdatedDT;
					if ( HTTPGetFile->ServerReply == 404 )
						{
						XMLP->AddParam("NextTryDT",TDateTime::CurrentDateTime()+float(15)/24/60);
						if (!XMLP->SaveToFile())
							FileMap->WriteErrorFound = true;
						//XMLP->SaveToFile();
						}
						else
						if ( HTTPGetFile->ServerReply == -1 )
							{
							XMLP->AddParam("NextTryDT",TDateTime::CurrentDateTime()+float(5)/24/60);
							if (!XMLP->SaveToFile())
								FileMap->WriteErrorFound = true;
							//XMLP->SaveToFile();
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

bool TCommandReceiver::IsOnTime()
{
    if (!XMLP)
        return false;
    TDateTime NextTryDT;
    try
    {
        NextTryDT = TDateTime(XMLP->GetParamValue("NextTryDT"));
    }
    catch(...)
    {
        NextTryDT = TDateTime::CurrentDateTime();
    }
    if (NextTryDT <= TDateTime::CurrentDateTime())
        return true;
    return false;
}





