//---------------------------------------------------------------------------

#include <IdTCPClient.hpp>
#pragma hdrstop

#pragma package(smart_init)
#include "PacketSender.h"
#include "SevenZipVCL.hpp"
#include "globals.h"
#include "boost/format.hpp"

//---------------------------------------------------------------------------

TPacketSender::TPacketSender(AnsiString _fileName, TWConfig* _Cfg, TLogClass* _Log, TFileMap* _FileMap)
{
try
  {
	PacketLoadError = false;
	if ((_Cfg->GetStatServerHost().LowerCase()=="none")||(_Cfg->GetStatServerHost().LowerCase()==""))
	  {
    Enabled=false;
    }
  //CoInitializeEx(NULL, COINIT_MULTITHREADED);
  Cfg = _Cfg;
  FileMap = _FileMap;
  InnerLog=false;
  if (_Log==NULL) {
    Log = new TLogClass("PacketSender");
    InnerLog=true;
    }
    else
    Log=_Log;
  StatPacket = new TStatPacket(Cfg, Log);
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
	Terminated=false;
  try
    {
    TerminalID=AnsiString(Cfg->Terminal.Number.c_str()).ToInt();
    }
  catch (...)
    {
    Log->Write((boost::format("Terminal number is not valid integer value: '%1%' set to -1") % Cfg->Terminal.Number).str().c_str());
    TerminalID=-1;
    }
	Host=Cfg->GetStatServerHost();
	Port=Cfg->GetStatServerPort();
  Enabled=true;
  }
    catch (...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        PacketLoadError = true;
    }
}

//---------------------------------------------------------------------------

TPacketSender::~TPacketSender()
{
    if (!Enabled)
        return;
    //CoUninitialize();
    if (StatPacket)
    {
        delete StatPacket;
        StatPacket = NULL;
        //  Log->Write("  Packet closed.");
    }
    Clear();
    if (InnerLog)
        delete Log;
}

//---------------------------------------------------------------------------

void TPacketSender::Clear()
{
try
		{
		if (!Enabled)
				return;
		if (StatPacket)
			StatPacket->Clear();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TPacketSender::StorePaymentInitTemp(TDateTime _EventDateTime, int _OperatorId, AnsiString _InitialSessionNum, float _Comission, const TNotesVector& _Notes, AnsiString _Fields)
{
    try
    {
        if (!Enabled)
            return;
        //Clear();
        StatPacket->PacketType = cnPaymentInit;
        StatPacket->TerminalID = TerminalID;
        StatPacket->EventDateTime = _EventDateTime;
        StatPacket->OperatorID = _OperatorId;
        StatPacket->InitialSessionNum = _InitialSessionNum;
        if (1000*fabs(ceil(_Comission) - _Comission) < 1)
            StatPacket->Comission = my_round(_Comission, true);
        else
            StatPacket->Comission = _Comission;
        StatPacket->AddNotes(_Notes);
        StatPacket->AddParam("params",_Fields);
        //StatPacket->SaveToTempFile();
        if (!StatPacket->SaveToTempFile())
            if (FileMap)
                FileMap->WriteErrorFound = true;
        //StatPacket->RenameTempFile();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TPacketSender::StorePaymentInitComplete()
{
if (!Enabled)
  return;
try
  {
  StatPacket->RenameTempFile();
  StatPacket->CloseFile();
  AnsiString AToLog="Payment init stored to "+StatPacket->TruncateFileName(StatPacket->PacketFileName.c_str())+". {TID: "+AnsiString(TerminalID)+"; EventDT: "+StatPacket->EventDateTime+"; OperatorID: "+StatPacket->OperatorID+"; InitialSessionNum: "+StatPacket->InitialSessionNum+"; Comission: "+StatPacket->Comission+"; Notes: { ";
  for (std::size_t i=0;i<StatPacket->vNotes.size();i++) {
      AToLog += (boost::format("{%1% %2% %3% %4%} ") % StatPacket->vNotes[i].ValidatorID % StatPacket->vNotes[i].CurrencyID % StatPacket->vNotes[i].Nominal % StatPacket->vNotes[i].Count).str().c_str();
      }
  AToLog+="}; Payment Info: {"+StatPacket->GetParamValue("params")+"}}.";
  Log->Write(AToLog.c_str());
  if (FileMap) FileMap->CheckStatDir=true;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TPacketSender::StoreIncassation(TDateTime _EventDateTime, const TNotesVector& _Notes, double _Comission, AnsiString _IncassationNumber)
{
try
	{
	if (!Enabled)
			return;
	Clear();
  //CreatePacket();
	StatPacket->PacketType=cnIncassation;
	StatPacket->TerminalID=TerminalID;
	StatPacket->EventDateTime=_EventDateTime;
	StatPacket->AddNotes(_Notes);
	StatPacket->Comission=_Comission;
	StatPacket->SessionNum=_IncassationNumber;
	if (!StatPacket->SaveToFile())
		if (FileMap) FileMap->WriteErrorFound = true;
	StatPacket->CloseFile();

	AnsiString AToLog="Incassation stored to "+StatPacket->TruncateFileName(StatPacket->PacketFileName.c_str())+". {Incass.Number: "+StatPacket->SessionNum+"; Comission: "+FloatToStrF(StatPacket->Comission,ffFixed,18,2)+"; Notes: { ";
	for (std::size_t i=0;i<StatPacket->vNotes.size();i++)
    {
		AToLog += (boost::format("{%1% %2% %3% %4%} ") % StatPacket->vNotes[i].ValidatorID % StatPacket->vNotes[i].CurrencyID % StatPacket->vNotes[i].Nominal % StatPacket->vNotes[i].Count).str().c_str();
		}
	AToLog+="}}.";
	Log->Write(AToLog.c_str());
	if (FileMap) FileMap->CheckStatDir=true;
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TPacketSender::StoreError(TDateTime _EventDateTime, AnsiString _Sender, int _Type, AnsiString _Description, int _SubType, AnsiString _SubDescription)
{
try
	{
	if (!Enabled)
			return;
  if (StatPacket->PacketFileName=="")
  	Clear();
	StatPacket->PacketType=cnError;
	StatPacket->TerminalID=TerminalID;
	StatPacket->EventDateTime=_EventDateTime;
	StatPacket->ErrSender=_Sender;
	StatPacket->ErrType=_Type;
	StatPacket->ErrDescription=_Description;
	StatPacket->ErrSubType=_SubType;
	StatPacket->ErrSubDescription=_SubDescription;
	if (!StatPacket->SaveToFile())
		if (FileMap) FileMap->WriteErrorFound = true;
	StatPacket->CloseFile();

	AnsiString AToLog="Error stored to "+StatPacket->TruncateFileName(StatPacket->PacketFileName.c_str())+". {Sender: "+StatPacket->ErrSender+"; Type: "+StatPacket->ErrType+": "+StatPacket->ErrDescription;
	if (StatPacket->ErrSubType!=0)
		AToLog+="; SubType: "+AnsiString(StatPacket->ErrSubType)+": "+StatPacket->ErrSubDescription;
	AToLog+="}.";
	Log->Write(AToLog.c_str());
	if (FileMap) FileMap->CheckStatDir=true;
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TPacketSender::StorePaymentStatusChange(TDateTime _EventDateTime, AnsiString _InitialSessionNum, int _Status, int _ErrorCode)
{
try
  {
  if (!Enabled)
    return;
  Clear();
  StatPacket->PacketType=cnPaymentStatusChange;
  StatPacket->TerminalID=TerminalID;
  StatPacket->EventDateTime=_EventDateTime;
  StatPacket->InitialSessionNum=_InitialSessionNum;
  StatPacket->Status=_Status;
  StatPacket->ErrorCode=_ErrorCode;
  if (!StatPacket->SaveToFile())
    if (FileMap) FileMap->WriteErrorFound = true;
  StatPacket->CloseFile();

  Log->Write((boost::format(" Payment status change stored to %1%. {Initial session: %2%; Status: %3%; Error: %4%}.") % StatPacket->TruncateFileName(StatPacket->PacketFileName.c_str()).c_str() % StatPacket->InitialSessionNum.c_str() % StatPacket->Status % StatPacket->ErrorCode).str().c_str());
  FileMap->CheckStatDir=true;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TPacketSender::StorePaymentComplete(TDateTime _EventDateTime, AnsiString _InitialSessionNum, AnsiString _LastSessionNum, int _ErrorCode, int _LastErrorCode, TDateTime _FirstTryDT, int _OperatorId, double _Sum, double _Comission, AnsiString _Fields)
{
UNREFERENCED_PARAMETER(_LastErrorCode);
UNREFERENCED_PARAMETER(_OperatorId);
UNREFERENCED_PARAMETER(_Sum);
UNREFERENCED_PARAMETER(_Comission);
try
  {
  if (!Enabled)
      return;
  Clear();
  StatPacket->PacketType=cnPaymentComplete;
  StatPacket->TerminalID=TerminalID;
  StatPacket->EventDateTime=_EventDateTime;
  StatPacket->InitialSessionNum=_InitialSessionNum;
  StatPacket->SessionNum=_LastSessionNum;
  StatPacket->ErrorCode=_ErrorCode;
  StatPacket->InitDT = _FirstTryDT;
  if (!StatPacket->SaveToFile())
    if (FileMap) FileMap->WriteErrorFound = true;
  StatPacket->CloseFile();

  Log->Write((boost::format(" PaymentComplete event stored to %1%. {Initial session: %2%; Last session: %3%; ErrorCode: %4%}.") % StatPacket->TruncateFileName(StatPacket->PacketFileName.c_str()).c_str() % StatPacket->InitialSessionNum.c_str() % StatPacket->SessionNum.c_str() % StatPacket->ErrorCode).str().c_str());
  FileMap->CheckStatDir=true;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TPacketSender::StoreSendCommandProcessedTemp(int _CommandUID)
{
try
  {
  if (!Enabled)
    return;
  Clear();
  StatPacket->PacketType=cnCommandProcessed;
  StatPacket->TerminalID=TerminalID;
  StatPacket->Status=_CommandUID;
  //StatPacket->SaveToTempFile();
  if (!StatPacket->SaveToTempFile())
    if (FileMap) FileMap->WriteErrorFound = true;
  StatPacket->CloseFile();

  Log->Write((boost::format("  Command processed temp stored to %1%. {CID: %2%}.") % StatPacket->TruncateFileName(StatPacket->PacketFileName.c_str()).c_str() % StatPacket->Status).str().c_str());
  FileMap->CheckStatDir=true;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TPacketSender::StoreCommandProcessed(int _CommandUID)
{
try
  {
  if (!Enabled)
    return;
  if (_CommandUID<0)
    {
    Log->Write((boost::format("Packet not created - CID %1% < 0.") % _CommandUID).str().c_str());
    return;
    }
  Clear();
  StatPacket->PacketType=cnCommandProcessed;
  StatPacket->TerminalID=TerminalID;
  StatPacket->Status=_CommandUID;
  if (!StatPacket->SaveToFile())
    if (FileMap) FileMap->WriteErrorFound = true;
  StatPacket->CloseFile();

  Log->Write((boost::format("  Command processed event stored to %1%. {CID: %2%}.") % StatPacket->TruncateFileName(StatPacket->PacketFileName.c_str()).c_str() % StatPacket->Status).str().c_str());
  if (FileMap) FileMap->CheckStatDir=true;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TPacketSender::StoreFileSend(AnsiString _FileName)
{
try
  {
  if (!Enabled)
      return;
  if (!FileExists(_FileName)) {
      Log->Write((boost::format("Can't store file - %1% does not exist!") % _FileName.c_str()).str().c_str());
      return;
      }
  Clear();
  StatPacket->SendFileName=StatPacket->TruncateFileName(_FileName);
  StatPacket->PacketFileName=StatPacket->GetNewFileName("").c_str();
  if (!PutFile("\""+_FileName+"\"",(StatPacket->PacketFileName+"file").c_str())) {
    Log->Write((boost::format("Can't store file - %1% can not be copied to %2%file!") % _FileName.c_str() % StatPacket->PacketFileName.c_str()).str().c_str());
    return;
    }
  StatPacket->PacketType=cnFileSend;
  StatPacket->TerminalID=TerminalID;
  if (!StatPacket->StoreToFile((StatPacket->PacketFileName+"pkt").c_str(),false))
    if (FileMap) FileMap->WriteErrorFound = true;
  StatPacket->CloseFile();
  Log->Write((boost::format("  File %1% stored to %2% file.") % StatPacket->SendFileName.c_str() % StatPacket->TruncateFileName(StatPacket->PacketFileName.c_str()).c_str()).str().c_str());
  if (FileMap) FileMap->CheckStatDir=true;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TPacketSender::StoreFilesSend(AnsiString _FileNames, AnsiString _ArchiveFileName)
{
try
	{
	if (!Enabled)
		return;
	Clear();
	StatPacket->SendFileName=_ArchiveFileName;
	AnsiString PacketFileName=StatPacket->GetNewFileName("");
	if (!PutFile(_FileNames,PacketFileName+"file")) {
		Log->Write((boost::format("  Can't store FileSend event - file(s) %1% can not be copied to %2%file.") % _FileNames.c_str() % PacketFileName.c_str()).str().c_str());
		return;
		}
	StatPacket->PacketType=cnFileSendNew;
	StatPacket->TerminalID=TerminalID;
	if (!StatPacket->StoreToFile(PacketFileName+"pkt",false))
		if (FileMap) FileMap->WriteErrorFound = true;
	StatPacket->CloseFile();

	Log->Write((boost::format("  File(s) %1% stored to %2%file.") % StatPacket->SendFileName.c_str() % StatPacket->TruncateFileName(PacketFileName).c_str()).str().c_str());
	ListFile(PacketFileName+"file");
	if (FileMap) FileMap->CheckStatDir=true;
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

bool TPacketSender::PutFile(AnsiString SourceFileName, AnsiString TargetFileName)
{
  bool bResult = false;
  try
  {
    if (!Enabled)
        return false;
		if (Cfg->StatInfo.CompressFiles)
		{
			std::auto_ptr <TStringList> slNames (new TStringList() );
			slNames->Clear();
			slNames->DelimitedText=SourceFileName;
			std::auto_ptr <TSevenZip> SZ ( new TSevenZip(NULL) );
                        WideString wsTargetFileName = TargetFileName;
			SZ->SZFileName=wsTargetFileName;
			SZ->LZMACompressStrength=ULTRA;
			SZ->Files->Clear();
			SZ->AddOptions=SZ->AddOptions << AddStoreOnlyFilename;
			SourceFileName+=" ";
			for (int i=0;i<slNames->Count;i++)
			{
				SZ->Files->AddString(WideString(slNames->Strings[i]));
			}
			SZ->Add();
			if (SZ->ErrCode==0)
			{
				if (StatPacket->SendFileName.Pos(".")!=0)
					StatPacket->SendFileName=StatPacket->SendFileName.SubString(0,StatPacket->SendFileName.Pos("."))+"7z";
				else
					StatPacket->SendFileName+=".7z";
				bResult=true;
			}
			else
				Log->Write((boost::format("Error archiving file %1% into %2%. error code = %3%") % SourceFileName.c_str() % TargetFileName.c_str() % SZ->ErrCode).str().c_str());
    }
		else
			bResult = CopyFileTo(SourceFileName,TargetFileName);
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        bResult = false;
    }
    return bResult;
}

//---------------------------------------------------------------------------

bool TPacketSender::SendHeartBeat(short _Version, short _Status, int _BillCount, int _BillSum, int _SimBalance, int _SignalLevel, bool bFirstTry, int ChequeCounter)
{
UNREFERENCED_PARAMETER(_Version);
UNREFERENCED_PARAMETER(_Status);
UNREFERENCED_PARAMETER(_BillCount);
UNREFERENCED_PARAMETER(_BillSum);
UNREFERENCED_PARAMETER(_SimBalance);
UNREFERENCED_PARAMETER(_SignalLevel);
UNREFERENCED_PARAMETER(bFirstTry);
UNREFERENCED_PARAMETER(ChequeCounter);
return false;
}

//---------------------------------------------------------------------------

AnsiString TPacketSender::TestConnection(AnsiString URL)
{
return "Not implemented.";
}

//---------------------------------------------------------------------------

bool TPacketSender::Process(bool bProcessMessages)
{
UNREFERENCED_PARAMETER(bProcessMessages);
if (!Enabled)
	return false;
if (PacketLoadError)
	{
	Log->Write("Error! Packet not loaded.");
	return false;
	}
return false;
}

//---------------------------------------------------------------------------

bool TPacketSender::ListFile(AnsiString SourceFileName)
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
			Log->Write((boost::format("Error listing file %1%.") % SourceFileName.c_str()).str().c_str());
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        bResult = false;
    }
    return bResult;
}

//---------------------------------------------------------------------------

void __fastcall TPacketSender::SZListFileEvent(System::TObject* Sender, WideString Filename, unsigned Fileindex, unsigned FileSizeU, unsigned FileSizeP, unsigned Fileattr, unsigned Filecrc, WideString Filemethod, double FileTime)
{
    UNREFERENCED_PARAMETER(Sender);
    UNREFERENCED_PARAMETER(Fileindex);
    UNREFERENCED_PARAMETER(FileSizeP);
    UNREFERENCED_PARAMETER(Fileattr);
    Log->Write((boost::format("    %1%, %2%, %3%, CRC: %4%.") % AnsiString(Filename).c_str() % FileSizeU % AnsiString(TDateTime(FileTime)).c_str() % Filecrc).str().c_str());
}

//---------------------------------------------------------------------------

bool TPacketSender::CreatePacket(AnsiString PacketName)
{
try
	{
	if (!Enabled)
		return true;
  Clear();
  if (PacketName=="")
  	StatPacket->PacketFileName = StatPacket->GetNewFileName("pkt").c_str();
    else
    StatPacket->PacketFileName = ((Cfg->Dirs.StatOutbound + "\\").c_str() + PacketName +".pkt").c_str();
	return StatPacket->CreateFile(StatPacket->PacketFileName.c_str());
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

//---------------------------------------------------------------------------

bool TPacketSender::CreateTempPacket()
{
    try
    {
        if (!Enabled)
            return true;
        StatPacket->PacketFileName = StatPacket->GetNewFileName("tmp").c_str();
        return StatPacket->CreateFile(StatPacket->PacketFileName.c_str());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

