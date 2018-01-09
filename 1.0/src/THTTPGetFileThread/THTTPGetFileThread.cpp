//---------------------------------------------------------------------------

#include <vcl.h>
#include <algorith.h>
#pragma hdrstop

#include "THTTPGetFileThread.h"
#include "SevenZipVCL.hpp"
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
//      void __fastcall THTTPGetFileThread::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------
const cnMaxHTTPPacketSize=10240;

__fastcall THTTPGetFileThread::THTTPGetFileThread(TLogClass *_Log, TWConfig *_Cfg, AnsiString _URL, AnsiString _FileName, AnsiString _CUID, TDateTime _CheckDT)
								: TThread(true)
{
try
	{
	//CoInitialize(NULL);
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	FreeOnTerminate = false;
	Finished=false;
	InnerLog=false;
	if (_Log==NULL) {
		Log = new TLogClass("THTTPGetFileThread");
		InnerLog=true;
		}
		else
		Log=_Log;
	//Log->Write("THTTPGetFileThread.Init started...");
	Cfg=_Cfg;
	URL=_URL;
	CUID=_CUID;
	CheckDT=_CheckDT;
	FileName=_FileName;
	IndyError=false;
	AnswerContentLength=0;
	InFile = NULL;
	LastUpdatedDT.Val = 0;
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

__fastcall THTTPGetFileThread::~THTTPGetFileThread(void)
{
//CoUninitialize();
//Log->Write("THTTPGetFileThread done.");
if (IdHTTPC!=NULL) {
	delete IdHTTPC;
	IdHTTPC = NULL;
	}
if (IdSSL!=NULL) {
	delete IdSSL;
	IdSSL = NULL;
	}
if (Answer!=NULL) {
	delete Answer;
	Answer = NULL;
	}
if (InFile!=NULL) {
	delete InFile;
	InFile = NULL;
	}
if (InnerLog) {
	delete Log;
	}
}

//---------------------------------------------------------------------------

void __fastcall THTTPGetFileThread::Execute()
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

void THTTPGetFileThread::Process()
{
    //int Result = "";
    int IncomingFileSize;
    int ReceivedFileSize=0;
    //char FileBuffer[1024];
    int PacketSize=0;
    int StartOffset=0;
    AnsiString IncomingPartFileName;
    bool bOK=true;
    Result = GetFileError;
    ServerReply = 0;
    try
    {
        if (!Terminated)
        {
            try
            {
                IdHTTPC = new TIdHTTP(Application);
                Answer = new TMemoryStream();
            }
            catch(Exception& error)
            {
                IndyError=true;
                Log->Write((boost::format("THTTPGetFileThread::Process: Init Exception: %1%") % error.Message.c_str()).str().c_str());
                bOK=false;
                //Application->Terminate();
            }
            catch(...)
            {
                bOK=false;
                ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            }
        }
        if(0==URL.Pos("http"))
            URL=Cfg->StatInfo.DownloadURLPrefix.c_str()+URL;
        Log->Write((boost::format("Receiving file %1%, from %2%") % FileName.c_str() % URL.c_str()).str().c_str());

        if ((bOK)&&(!Terminated))
        {
            try
                                {
																if (URL.Pos("https://")!=0) {
                                        IdSSL = new TIdConnectionInterceptOpenSSL(Application);
                                        IdHTTPC->Intercept = IdSSL;
																				IdHTTPC->InterceptEnabled=true;
                                        IdHTTPC->Port=443;
                                        }
                                        else
                                        IdHTTPC->Port=80;
                                if (Cfg->Connection().HTTPProxy.Type=="http")
                                        {
                                        IdHTTPC->Request->ProxyServer=Cfg->Connection().HTTPProxy.Host.c_str();
                                        IdHTTPC->Request->ProxyPort=Cfg->Connection().HTTPProxy.Port;
                                        IdHTTPC->Request->ProxyUsername=Cfg->Connection().HTTPProxy.UserName.c_str();
                                        IdHTTPC->Request->ProxyPassword=Cfg->Connection().HTTPProxy.Password.c_str();
                                        }
                                        else
                                        if (Cfg->Connection().HTTPProxy.Type=="socks")
                                                IdHTTPC->SocksInfo->Assign(Cfg->Connection().Proxy);
                                }
                        catch(Exception& error)
                                {
                                IndyError=true;
                                Log->Write((boost::format("THTTPGetFileThread::Process: ProxyInit Exception: %1%") % error.Message.c_str()).str().c_str());
                                bOK=false;
                                }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            bOK=false;
        }
                        }

								if((IdHTTPC == NULL)||(Answer == NULL)) {
												Log->Write("THTTPGetFileThread::Process: Error creating object(s).");
                        bOK=false;
                        }

								if ((bOK)&&(!Terminated)) {
												try
																{
																IdHTTPC->Head(URL);
																IncomingFileSize=IdHTTPC->Response->ContentLength;
																Log->Append((boost::format(", size: %1%, last modified: %2%, prev. last modified: %3%") % IncomingFileSize % AnsiString(IdHTTPC->Response->LastModified).c_str() % AnsiString(CheckDT).c_str()).str().c_str());
																if ((CheckDT.Val!=0)&&(IdHTTPC->Response->LastModified==CheckDT))
																	{
																	Log->Append(", file not changed, aborting download.");
																	IdHTTPC->Disconnect();
																	Result = GetFileCheckFailed;
																	return;
																	}
																	else
																	LastUpdatedDT = IdHTTPC->Response->LastModified;

																IncomingPartFileName=(Cfg->Dirs.CommandsInbound+"\\").c_str()+FileName+AnsiString(IncomingFileSize)+".part";

																if (FileExists(IncomingPartFileName))
																	{
																	StartOffset = FileSizeByName(IncomingPartFileName);
																	if (InFile!=NULL) {
																		delete InFile;
																		InFile = NULL;
																		}
																	InFile = new TFileStream(IncomingPartFileName, fmOpenWrite | fmShareExclusive);
																	if (StartOffset>0)
																		InFile->Seek(0,soFromEnd);
																	}
																	else
																	InFile = new TFileStream(IncomingPartFileName, fmCreate | fmShareExclusive);
																Log->Append((boost::format(", starting from: %1%.") % StartOffset).str().c_str());
																ReceivedFileSize=0;
																while (ReceivedFileSize+StartOffset<IncomingFileSize) {
																PacketSize=min(IncomingFileSize-(ReceivedFileSize+StartOffset),cnMaxHTTPPacketSize);
																IdHTTPC->Request->ContentLength=IncomingFileSize;
																IdHTTPC->Request->ContentRangeStart=StartOffset+ReceivedFileSize;
																IdHTTPC->Request->ContentRangeEnd=StartOffset+ReceivedFileSize+PacketSize;
																/*Log->Write("Length: "+AnsiString(IdHTTPC->Request->ContentLength));
																Log->Append(" RangeStart: "+AnsiString(IdHTTPC->Request->ContentRangeStart));
																Log->Append(" RangeEnd: "+AnsiString(IdHTTPC->Request->ContentRangeEnd));
																Log->Append(" PacketSize: "+AnsiString(PacketSize));*/
																Answer->Clear();
																Answer->SetSize(PacketSize);
																	try
																		{
																		IdHTTPC->Get(URL,Answer);
																		}
																	catch (Exception &exception)
																		{
																		Log->Write((boost::format("Exception occured in TSendThread::ReceiveFile: %1%") % exception.Message.c_str()).str().c_str());
																		Log->Write((boost::format("File receiving aborted - %1% bytes received.") % ReceivedFileSize).str().c_str());
																		break;
																		}
                catch(...)
                {
                    ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                    Log->Write("Exception occured in TSendThread::ReceiveFile");
                    Log->Write((boost::format("File receiving aborted - %1% bytes received.") % ReceivedFileSize).str().c_str());
                    break;
                }
																	Answer->Position=0;
																	InFile->CopyFrom(Answer, PacketSize);
																	ReceivedFileSize+=PacketSize;
																	}
																if (ReceivedFileSize+StartOffset!=IncomingFileSize) {
																	Log->Write((boost::format("File NOT received. %1% bytes stored.") % ReceivedFileSize).str().c_str());
																	}
																	else
																	{
																	Log->Write((boost::format("File received. %1% bytes stored.") % ReceivedFileSize).str().c_str());
																	if (InFile!=NULL)
																		{
																		delete InFile;
																		InFile = NULL;
																		}
																	DeleteFile((Cfg->Dirs.CommandsInbound+"\\").c_str()+CUID+".file");
																	if (RenameFile(IncomingPartFileName, (Cfg->Dirs.CommandsInbound+"\\").c_str()+CUID+".file"))
																		Log->Write((boost::format("File %1% renamed to %2%\\%3%.file.") % IncomingPartFileName.c_str() % Cfg->Dirs.CommandsInbound.c_str() % CUID.c_str()).str().c_str());
                                                                    else
																		Log->Write((boost::format("Error renaming file %1% to %2%\\%3%.file.") % IncomingPartFileName.c_str() % Cfg->Dirs.CommandsInbound.c_str() % CUID.c_str()).str().c_str());
																	Log->Write((boost::format("Incoming file saved: %1%\\%2%.file") % Cfg->Dirs.CommandsInbound.c_str() % CUID.c_str()).str().c_str());
																	if ((FileName.LowerCase()).Pos(".7z")>0)
																		{
																		if (TestArchiveFile((Cfg->Dirs.CommandsInbound+"\\").c_str()+CUID+".file"))
																			{
																			Result = GetFileOK;
																			}
																			else
																			{
																			if (DeleteFile((Cfg->Dirs.CommandsInbound+"\\").c_str()+CUID+".file"))
																				Log->Write((boost::format("File deleted: %1%\\%2%.file.") % Cfg->Dirs.CommandsInbound.c_str() % CUID.c_str()).str().c_str());
																				else
																				Log->Write((boost::format("Error deleting file: %1%\\%2%.file!") % Cfg->Dirs.CommandsInbound.c_str() % CUID.c_str()).str().c_str());
																			}
																		}
																		else
																		{
																		Result = GetFileOK;
																		}
																	}
																}
												catch (EIdProtocolReplyError &ErrData)
																{
																Log->Append((boost::format(" Error: Server reply: %1%") % ErrData.ReplyErrorCode).str().c_str());
																ServerReply = ErrData.ReplyErrorCode;
																bOK=false;
																}
												catch (Exception &exception)
																{
																Log->Append((boost::format(" THTTPC.Get exception: %1%") % exception.Message.c_str()).str().c_str());
																ServerReply = -1;
																bOK=false;
																}
            catch(...)
            {
                ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                Log->Append(" THTTPC.Get exception: ?");
                ServerReply = -1;
                bOK=false;
            }
        }

        if ((bOK)&&(!Terminated))
        {
            Log->Append((boost::format(", result: %1%.") % IdHTTPC->ResponseText.c_str()).str().c_str());
            try
            {
                IdHTTPC->Disconnect();
            }
            catch(...)
            {
                ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                Log->Write("THTTPGetFileThread::Process: Close connection exception");
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

bool THTTPGetFileThread::TestArchiveFile(AnsiString SourceFileName)
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
      Log->Write((boost::format("Testing archive file %1% error #%2%!") % SourceFileName.c_str() % SZ->ErrCode).str().c_str());
      //Log->Write("Testing archive file "+SourceFileName+" error!");
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

/*				case cnHTTPFileRequest:
						IncomingFileName=XMLP->GetParamValue("FileName");
						Log->Write("Starting receiving file...");
						Log->Append(", file name: "+IncomingFileName);
						Log->Append(", URL: "+XMLP->GetParamValue("URL"));
						GetFile(XMLP->GetParamValue("URL"),IncomingFileName);
						AnsiString Answer = Connect(XMLP->GetParamValue("URL"),"", cnMHead);
						Log->Append(", file size: "+Answer);*/
						/*IncomingFileSize=Answer.ToInt();
						//IncomingFileSize=IdTCPC->ReadInteger(false);
						//Log->Append(", file size: "+AnsiString(IncomingFileSize)+", ");
						IncomingPartFileName=Cfg->Dirs->CommandsInbound+"\\"+IncomingFileName+AnsiString(IncomingFileSize)+".part";

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
						//Write(StartOffset);
						Log->Append("starting offset: "+AnsiString(StartOffset)+".");
						ReceivedFileSize=0;
						//IncomingFileSize+=10000;
						while (ReceivedFileSize+StartOffset<IncomingFileSize) {
						PacketSize=min(IncomingFileSize-(ReceivedFileSize+StartOffset),cnMaxPacketSize);
//            Log->Write("PacketSize = "+PacketSize);
							try
								{
								//IdTCPC->ReadBuffer(FileBuffer, PacketSize);
								}
							catch (Exception &exception)
								{
								Log->Write("Exception occured in TSendThread::ReceiveFile: "+exception.Message);
								Log->Write("File receiving aborted - "+AnsiString(ReceivedFileSize)+" bytes received.");
								break;
								}
							InFile->Write(FileBuffer, PacketSize);
							ReceivedFileSize+=PacketSize;
							//Log->Write("Received "+AnsiString(PacketSize)+" bytes.");
							}
						if (ReceivedFileSize+StartOffset!=IncomingFileSize) {
							Log->Write("File NOT received. "+AnsiString(ReceivedFileSize)+" bytes stored.");
							break;
							}
						Log->Write("File received. "+AnsiString(ReceivedFileSize)+" bytes stored.");
						if (InFile!=NULL) {
							delete InFile;
							InFile = NULL;
							}
						DeleteFile(Cfg->Dirs->CommandsInbound+"\\"+AnsiString(CommandUID)+".file");
						if (RenameFile(IncomingPartFileName, Cfg->Dirs->CommandsInbound+"\\"+AnsiString(CommandUID)+".file"))
							Log->Write("File "+IncomingPartFileName+" renamed to "+Cfg->Dirs->CommandsInbound+"\\"+AnsiString(CommandUID)+".file"+".");
							else
							Log->Write("Error renaming file "+IncomingPartFileName+" to "+Cfg->Dirs->CommandsInbound+"\\"+AnsiString(CommandUID)+".file"+".");
						Log->Write("Incoming file saved: "+Cfg->Dirs->CommandsInbound+"\\"+AnsiString(CommandUID)+".file");
						if ((IncomingFileName.LowerCase()).Pos(".7z")>0) {
							if (TestArchiveFile(Cfg->Dirs->CommandsInbound+"\\"+AnsiString(CommandUID)+".file")) {
								//Write(int(0)); // File received OK.
								}
								else {
								if (DeleteFile(Cfg->Dirs->CommandsInbound+"\\"+AnsiString(CommandUID)+".file"))
									Log->Write("File deleted: "+Cfg->Dirs->CommandsInbound+"\\"+AnsiString(CommandUID)+".file.");
									else
									Log->Write("Error deleting file: "+Cfg->Dirs->CommandsInbound+"\\"+AnsiString(CommandUID)+".file!");
								break;
								}
							}
							else
							{
							//Write(int(0)); // File received OK.
							}
						if (((FileExists(Cfg->Dirs->CommandsInbound+"\\"+AnsiString(CommandUID)+".pkt"))||(FileExists(Cfg->Dirs->CommandsInbound+"\\"+AnsiString(CommandUID)+".ok"))))
							{
							Log->Append(" Command already registered, skipping...");
							break;
							}
						CommandReceiver->StoreReceiveFileCommand(CommandUID, IncomingFileName, IncomingFileSize, Buffer);
						break;*/
