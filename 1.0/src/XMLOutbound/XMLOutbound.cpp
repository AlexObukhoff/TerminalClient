//---------------------------------------------------------------------------

#include <classes.hpp>
#include <system.hpp>
#include <XMLDoc.hpp>
#include <SysUtils.hpp>
#pragma hdrstop

#include "globals.h"
#include "boost/format.hpp"
#include "XMLOutbound.h"

#include "TAviaPayment.h"
#include "TMoneyTransferPayment.h"
#include "THalfPinPayment.h"
#include "TTaxPayment.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

//---------------------------------------------------------------------------
__fastcall TOutbound::TOutbound(TWConfig *cfg, TLogClass *log, TFileMap *fileMap, TXMLInfo* infoFile)
{
    m_pCfg = cfg;
    m_pFileMap = fileMap;
    m_infoFile = infoFile;

    if(log == 0)
    {
        m_pLog = new TLogClass("TOutbound");
        m_isInnerLog = true;
    }
    else
    {
        m_pLog = log;
        m_isInnerLog = false;
    }

    terminating = false;
    busy = false;
    bFileProcessingStopped = false;
    NoTime = false;
    IndyError = false;
    DTLastFileProcessed = 0;
    UnprocessedFilesCount = 0;
}

//---------------------------------------------------------------------------

TOutbound::~TOutbound()
{
    if(m_isInnerLog)
        delete m_pLog;
}

//---------------------------------------------------------------------------

void TOutbound::RenameTempFiles()
{
    try
    {
        TSearchRec sr;
        int iAttributes = 0;
        if (FindFirst(Dir + "\\*.tmp", iAttributes, sr) == 0)
        {
            do
            {
                AnsiString TempFileName = Dir + "\\" + sr.Name;
                AnsiString NewFileName = TempFileName.SubString(0, TempFileName.Length() - 3) + "pkt";
                if (RenameFile(TempFileName,NewFileName))
                {
                    m_pLog->Write((boost::format("Temporary file %1% renamed to %2%.") % TempFileName.c_str() % NewFileName.c_str()).str().c_str());
                }
                else
                {
                    m_pLog->Write((boost::format("Cannot rename temporary file %1% to %2%.") % TempFileName.c_str() % NewFileName.c_str()).str().c_str());
                    continue;
                }
            }while (FindNext(sr) == 0);

            FindClose(sr);
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
    }
}

//---------------------------------------------------------------------------

bool TOutbound::ProcessFile(AnsiString _fileName)
{
    return false;
}

//---------------------------------------------------------------------------

AnsiString TOutbound::TruncateFileName(AnsiString _fileName)
{
    AnsiString result;
    try
    {
        result = _fileName;
        while(result.Pos("\\") != 0)
            result.Delete(1, result.Pos("\\"));
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
    }

    return result;
}

//---------------------------------------------------------------------------

void TOutbound::FilePostProcess(AnsiString _fileName)
{
    try
    {
        if(FileExists(_fileName.SubString(0,_fileName.Length() - 3) + "file"))
            DeleteFile(_fileName.SubString(0,_fileName.Length() - 3) + "file");

        if(DeleteFile(_fileName))
        {
            m_pLog->Append(" and deleted.");
        }
        else
        {
            m_pLog->Append((boost::format(". Can't delete file: %1%, trying to rename...") % TruncateFileName(_fileName).c_str()).str().c_str());

            if (RenameFile(_fileName,_fileName + ".processed"))
                m_pLog->Append("OK.");
            else
                m_pLog->Append("Error!");
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
    }
}

//---------------------------------------------------------------------------

void TOutbound::ProcessOutbound(_OutboundDirType _DirType)
{
    try
    {
        TSearchRec sr;
        int iAttributes = 0;
        AnsiString FindDir;
        if (_DirType==cnWork)
            FindDir=Dir;
        else
            FindDir=DirTemp;

        DevicePackets.clear();
        if (FindFirst(FindDir+"\\*.pkt", iAttributes, sr) == 0)
        {
            do
            {

                if ((m_pFileMap->WriteErrorFound)&&(!bFileProcessingStopped))
                {
                    m_pLog->Write("Flag WriteError found -> file processing is stopped.");
                    bFileProcessingStopped = true;
                }

                if (_DirType == cnWork)
                {
                    if (!RenameFile((const AnsiString)(Dir + "\\" + sr.Name), (const AnsiString) (DirTemp + "\\" + sr.Name)))
                    {
                        m_pLog->Write((boost::format(" Cannot rename file %1%\\%2% to %3%\\%4%.") % Dir.c_str() % sr.Name.c_str() % DirTemp.c_str() % sr.Name.c_str()).str().c_str());
                        continue;
                    }
                    AnsiString TempFileName = sr.Name.SubString(0,sr.Name.Length()-3)+"file";
                    if (FileExists(Dir + "\\" + TempFileName))
                    {
                        if (!RenameFile((Dir+"\\"+TempFileName), (DirTemp+"\\"+TempFileName)))
                        {
                            m_pLog->Write((boost::format(" Cannot rename file %1%\\%2% to %3%\\%4%.") % Dir.c_str() % TempFileName.c_str() % DirTemp.c_str() % TempFileName.c_str()).str().c_str());
                            continue;
                        }
                    }
                    else
                    {
                        m_pLog->Write((" Cannot find renamed file: " + Dir + "\\" + TempFileName).c_str());
                    }
                }
                if (FileSizeByName(DirTemp+"\\"+sr.Name)==0)
                {
                    m_pLog->Write((boost::format("Null-sized file %1%\\%2% found, trying to delete...") % DirTemp.c_str() % sr.Name.c_str()).str().c_str());
                    if (!DeleteFile(DirTemp+"\\"+sr.Name))
                        m_pLog->Append("Error!");
                    else
                        m_pLog->Append("OK.");
                }

                if (!bFileProcessingStopped)
                {
                    AnsiString fullFileName = DirTemp + "\\" + sr.Name;
                    fileAction::fileAction action = isNeedPacketSend(fullFileName);
                    switch(action)
                    {
                        case fileAction::send:
                            if (!ProcessFile(fullFileName))
                            {
                                if (!NoTime)
                                    m_pLog->Write((boost::format(" File %1% NOT processed.") % sr.Name.c_str()).str().c_str());
                            }
                            else
                            {
                                UnprocessedFilesCount--;
                                DTLastFileProcessed = TDateTime::CurrentDateTime();
                                m_pLog->Write((boost::format(" File %1% processed") % sr.Name.c_str()).str().c_str());
                                FilePostProcess(fullFileName);
                            }
                        break;
                        case fileAction::not_send:
                            DeleteFile(fullFileName);
                            m_pLog->Write((boost::format(" File deleted (even found): %1%%2%")
                                % (fullFileName.Pos("stat\\temp") ? "temp\\" : "")
                                % sr.Name.c_str()).str().c_str());
                        break;
                        case fileAction::bad:
                        {
                            AnsiString DestinationDirectory = m_pCfg->Dirs.PaymentsBad.c_str();
                            if (Name == "Statistics")
                                DestinationDirectory = m_pCfg->Dirs.StatOutboundBad.c_str();
                            RenameFile(fullFileName, (DestinationDirectory + "\\" + sr.Name).c_str());
                        }
                        break;
                        default:
                            m_pLog->Write((boost::format("   Unknown action: %1% for file {%2%}")
                                % action
                                % fullFileName.c_str()).str().c_str());
                        break;
                    }
                }
            }
            while ((FindNext(sr) == 0)&&(!terminating));
            FindClose(sr);
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
    }
}

//---------------------------------------------------------------------------

void TOutbound::Process()
{
    try
    {
        FilesCount();
        ProcessOutbound(cnTemp);
        ProcessOutbound(cnWork);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
    }
}

//---------------------------------------------------------------------------

int TOutbound::FilesCount()
{
    UnprocessedFilesCount = WorkDirFilesCount() + TempDirFilesCount();
    return UnprocessedFilesCount;
}

int TOutbound::WorkDirFilesCount()
{
    int result = 0;
    int iAttributes = 0;
    TSearchRec sr;

    if (FindFirst(Dir+"\\*.pkt", iAttributes, sr) == 0)
    {
        do
        {
            result++;
        }while (FindNext(sr) == 0);

        FindClose(sr);
    }

    return result;
}

int TOutbound::TempDirFilesCount()
{

    int result = 0;
    int iAttributes = 0;
    TSearchRec sr;

    if (FindFirst(DirTemp+"\\*.pkt", iAttributes, sr) == 0)
    {
        do
        {
            result++;
        }while (FindNext(sr) == 0);

        FindClose(sr);
    }

    return result;
}

//---------------------------------------------------------------------------

fileAction::fileAction TOutbound::isNeedPacketSend(AnsiString FileName)
{
    fileAction::fileAction result = fileAction::bad;
    try
    {
        if (FileExists(FileName))
        {
            std::auto_ptr<TXMLPacket> packet (new TXMLPacket(m_pCfg, m_pLog));

            AnsiString FileData;
            bool bRes = packet->OpenFile(FileName, FileData);
            if (bRes)
            {
                xmlGuard <_di_IXMLDocument> XmlDoc(NULL);
                XmlDoc = LoadXMLData(FileData);
                xmlGuard <_di_IXMLNode> RootNode (XmlDoc->GetDocumentElement());

                result = fileAction::send;
                int packetType = m_pCfg->GetInt(false, "packet_type", RootNode, -1);
                int code = -1;
                std::string deviceName = "";
                if (packetType == cnError)
                {
                    code = m_pCfg->GetInt(false, "error_type", RootNode, -1);
                    deviceName = m_pCfg->GetChildNodeValue(false, "sender_name", RootNode, "").c_str();
                }

                if (packetType == cnError)
                {
                    StatElement device_stat_element = make_pair<std::string, int>(deviceName, code);
                    result = (DevicePackets.find(device_stat_element) == DevicePackets.end()) ? fileAction::send : fileAction::not_send;
                    if (result == fileAction::send)
                        DevicePackets.insert(device_stat_element);
                }

                FileClose(bRes);
                return result;
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
        return result;
    }
}

//---------------------------------------------------------------------------

__fastcall TPaymentsOutbound::TPaymentsOutbound(TWConfig *cfg, TLogClass *log, TFileMap *fileMap,TXMLInfo* infoFile)
: TOutbound(cfg, log, fileMap, infoFile)
{
    Name="Payments";
    if (cfg)
    {
        Dir = m_pCfg->Dirs.PaymentsOutbound.c_str();
        DirTemp = m_pCfg->Dirs.PaymentsOutboundTemp.c_str();
    }
    PaymentsLog = new TLogClass("Payments");
    Payment = NULL;
}

//---------------------------------------------------------------------------

TPaymentsOutbound::~TPaymentsOutbound()
{
    try
    {
        if (PaymentsLog)
            delete PaymentsLog;

        if (Payment)
            delete Payment;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
    }
}

//---------------------------------------------------------------------------

bool TPaymentsOutbound::ProcessFile(AnsiString _fileName)
{
    bool bResult = true;
    bool bPaymentTimedOut = false;
    bool bPaymentCancelled = false;
    bool bUnsupportedPaymentType = false;
    int Recepient;

    try
    {
        if (Payment)
        {
            Recepient = Payment->Recepient;
            delete Payment;
            Payment = NULL;
        }
        else
        {
            Payment = new TPayment(_fileName, m_pCfg, m_pLog, m_pFileMap, NULL);
            if(Payment->getXmlParseError())
            {
                bResult = false;
            }
            else
            {
                Recepient = Payment->Recepient;
                delete Payment;
                Payment = NULL;
            }
        }

        if (!terminating && bResult)
        {
            AnsiString ProcessorType = m_pCfg->Operator(Recepient).ProcessorType.c_str();
            ProcessorType=ProcessorType.LowerCase();
            if(ProcessorType == "avia_center")
            {
                Payment = new TAviaPayment(_fileName, m_pCfg, m_pLog, m_pFileMap, NULL);
                #ifdef __CONN__
                ((TAviaPayment*)Payment)->SetAnswer();
                #endif
            }
            else if(ProcessorType == "cyberplat_mt")
            {
                Payment = new TMoneyTransferPayment(_fileName, m_pCfg, m_pLog, m_pFileMap, m_infoFile);
            }
            else if(ProcessorType == "half_pin")
            {
                Payment = new THalfPinPayment(_fileName, m_pCfg, m_pLog, m_pFileMap, NULL);
            }
            else if (ProcessorType == "cyberplat_taxes")
            {
                Payment = new TTaxPayment(_fileName, m_pCfg, m_pLog, m_pFileMap, NULL);
            }
            else
            {
                Payment = new TPayment(_fileName, m_pCfg, m_pLog, m_pFileMap, NULL);
            }

            if (!Payment->IsOnTime())
            {
                NoTime=true;
                bResult = false;
            }
            else
            {
                m_pLog->Write((boost::format(" Processing file: %1%...") % TruncateFileName(_fileName).c_str()).str().c_str());
                bPaymentCancelled = Payment->IsCancelled();
                if (bPaymentCancelled)
                {
                    bool CanCancel = Payment->CanCancel();
                    if (CanCancel)
                    {
                        PaymentsLog->Write((boost::format(" Not Processed - Cancelled from the monitoring server. %1%") % Payment->StringForPaymentsLog().c_str()).str().c_str());
                        bResult = true;
                    }
                    else
                    {
                        m_pLog->Write(" Can't cancel payment - it's being processed.");
                        bPaymentCancelled = false;
                    }
                }
                else
                {
                    bPaymentTimedOut = Payment->IsTimedOut();
                    if (bPaymentTimedOut)
                    {
                        PaymentsLog->Write((boost::format(" Not Processed - Timed out. %1%") % Payment->StringForPaymentsLog().c_str()).str().c_str());
                        bResult = true;
                    }
                    else
                    {
                        AnsiString ProcessorType=AnsiString(m_pCfg->Operator(Recepient).ProcessorType.c_str()).LowerCase();
                        if ((ProcessorType=="cyberplat_euroset")||(ProcessorType=="cyberplat_pin")||(ProcessorType=="cyberplat_metro")||(ProcessorType=="cyberplat_pin_trans"))
                        {
                            PaymentsLog->Write((boost::format(" Not processed by CONN - unsupported type (must have been processed before): %1% | %2%") % m_pCfg->Operator(Recepient).ProcessorType.c_str() % Payment->StringForPaymentsLog().c_str()).str().c_str());
                            bUnsupportedPaymentType = true;
                            bResult = true;
                        }
                        else
                        {
                            NoTime=false;
                            //log->Write(" Processing file: "+TruncateFileName(_fileName));
                            busy=true;
                            bResult = Payment->Process();
                            if (bResult)
                            {
                                if (Payment->StatusError==0 || ProcessorType=="cyberplat_mt")
                                    PaymentsLog->Write(Payment->StringForPaymentsLog().c_str());
                                else
                                    PaymentsLog->Write((boost::format(" Not Processed - Error: %1%. %2%.") % Payment->StatusError % Payment->StringForPaymentsLog().c_str()).str().c_str());
                            }

                            IndyError = Payment->IndyError;
                            if (IndyError)
                              m_pLog->Write(" Outbound: IndyError detected...");
                            Payment->Update();
                            busy=false;
                        }
                    }
                }
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
    }
    bool xmlParseError = Payment->getXmlParseError();
    if (Payment)
    {
        delete Payment;
        Payment = NULL;
    }

    if ((m_pCfg->LogsDelete.UnprocessedPayments) && (bPaymentTimedOut || bPaymentCancelled || bUnsupportedPaymentType || xmlParseError))
    {
        m_pLog->Write(" Trying to copy payment packet to the unprocessed payments folder...");
        std::string strtmp = (boost::format("%1%\\%2%") % m_pCfg->Dirs.PaymentsUnprocessed % TruncateFileName(_fileName).c_str()).str();
        if (CopyFile(_fileName.c_str(), strtmp.c_str(), TRUE))
            m_pLog->Append("OK.");
        else
            m_pLog->Append("Error!");
        bResult = true;
    }
    return bResult;
}

//---------------------------------------------------------------------------

void TPaymentsOutbound::FilePostProcess(AnsiString _fileName)
{
    try
    {
        if (DeleteFile(_fileName))
            m_pLog->Append(", stored to Payments log and deleted.");
        else
        {
          m_pLog->Append((boost::format(". Can't delete file: %1%, trying to rename...") % TruncateFileName(_fileName).c_str()).str().c_str());
          RenameFile(_fileName,_fileName+".processed");
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
    }
}

//---------------------------------------------------------------------------

__fastcall TStatisticsOutbound::TStatisticsOutbound(TWConfig *cfg, TLogClass *log, TFileMap *fileMap, TXMLInfo* infoFile)
: TOutbound(cfg, log, fileMap, infoFile)
{
    if (cfg)
    {
        Dir = cfg->Dirs.StatOutbound.c_str();
        DirTemp = cfg->Dirs.StatOutboundTemp.c_str();
    }
    
    PacketSender=NULL;
    Name="Statistics";
}

//---------------------------------------------------------------------------

TStatisticsOutbound::~TStatisticsOutbound()
{
    try
    {
        if (PacketSender)
            delete PacketSender;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
    }
}

//---------------------------------------------------------------------------

bool TStatisticsOutbound::ProcessFile(AnsiString _fileName)
{
    bool bResult=false;

    if(!FileExists(_fileName))
    {
        m_pLog->Write((boost::format(" File: %1%, no exist") % TruncateFileName(_fileName).c_str()).str().c_str());
        return true;
    }

    try
    {
        try
        {
            if (PacketSender)
            {
                delete PacketSender;
                PacketSender=NULL;
            }

            m_pLog->Write((boost::format(" Processing file: %1%") % TruncateFileName(_fileName).c_str()).str().c_str());
            if (!terminating)
            {
                if (m_pCfg->StatInfo.ProcessorType == cnCyberPlatServer)
                    PacketSender = new TCSPacketSender(_fileName, m_pCfg, m_pLog, m_pFileMap);
                else
                    PacketSender = new TSSPacketSender(_fileName, m_pCfg, m_pLog, m_pFileMap);

                if (PacketSender)
                {
                    busy=true;
                    bResult = PacketSender->Process();
                    busy=false;
                }
            }
        }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
        }
    }
    __finally
    {
        if (PacketSender)
        {
            delete PacketSender;
            PacketSender=NULL;
        }
    }

    return bResult;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

__fastcall TCommandsInbound::TCommandsInbound(TWConfig *cfg, TLogClass *log, TFileMap *fileMap, TXMLInfo* infoFile)
: TOutbound(cfg, log, fileMap, infoFile)
{
    Name="CommandsInbound";
    CommandReceiver = NULL;

    if(m_pCfg)
    {
        Dir = "";
        DirTemp = m_pCfg->Dirs.CommandsInbound.c_str();
    }
}

//---------------------------------------------------------------------------

TCommandsInbound::~TCommandsInbound()
{
    if (CommandReceiver)
        delete CommandReceiver;
}

//---------------------------------------------------------------------------

bool TCommandsInbound::ProcessFile(AnsiString _fileName)
{
    bool bResult=false;
    if(!FileExists(_fileName))
    {
        m_pLog->Write((boost::format(" File: %1%, no exist") % TruncateFileName(_fileName).c_str()).str().c_str());
        return true;
    }
    try
    {
        try
        {
            if (CommandReceiver)
            {
                delete CommandReceiver;
                CommandReceiver=NULL;
            }
            if (!terminating)
            {
                CommandReceiver  = new TCommandReceiver(_fileName, m_pCfg, m_pLog, m_pFileMap);
                if (CommandReceiver)
                {
                    busy=true;
                    NoTime = false;
                    if (CommandReceiver->IsOnTime())
                    {
                        m_pLog->Write((boost::format(" Processing file: %1%.") % TruncateFileName(_fileName).c_str()).str().c_str());
                        bResult = CommandReceiver->Process();
                        if (bResult)
                            Command = CommandReceiver->GetCommand();
                        else
                        Command=-1;
                    }
                    else
                    {
                        NoTime = true;
                    }
                    busy=false;
                }
            }
        }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
        }
    }
    __finally
    {
        if (CommandReceiver)
        {
            delete CommandReceiver;
            CommandReceiver=NULL;
        }
    }
    return bResult;
}

//---------------------------------------------------------------------------

void TCommandsInbound::Process()
{
    try
    {
        ProcessOutbound(cnTemp);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
    }
}

//---------------------------------------------------------------------------

void TCommandsInbound::FilePostProcess(AnsiString _fileName)
{
    try
    {
        AnsiString NewFileName=_fileName.SubString(0,_fileName.Length()-3)+"ok";
        if (FileExists(NewFileName))
        {
            if (DeleteFile(NewFileName))
                m_pLog->Append((boost::format("{ can't delete %1%!} ") % TruncateFileName(NewFileName).c_str()).str().c_str());
        }

        if (RenameFile(_fileName, NewFileName))
        {
            m_pLog->Append((boost::format(" and renamed to %1%") % TruncateFileName(NewFileName).c_str()).str().c_str());
        }
        else
        {
            m_pLog->Append((boost::format(". Can't rename file: %1% to %2%, deleting...") % TruncateFileName(_fileName).c_str() % TruncateFileName(NewFileName).c_str()).str().c_str());
            if (FileExists(_fileName))
            {
                if (DeleteFile(_fileName))
                    m_pLog->Append((boost::format("{ can't delete %1%.} ") % TruncateFileName(NewFileName).c_str()).str().c_str());
                else
                    m_pLog->Append("OK.");
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
    }
}

//---------------------------------------------------------------------------

__fastcall TEMailOutbound::TEMailOutbound(TWConfig *cfg, TLogClass *log, TFileMap *fileMap, TXMLInfo* infoFile)
: TOutbound(cfg, log, fileMap, infoFile)
{
    Name="EMail";
    EMailSender=NULL;

    if (cfg)
    {
        Dir = m_pCfg->Dirs.EMailOutbound.c_str();
        DirTemp = m_pCfg->Dirs.EMailOutboundTemp.c_str();
    }
    m_pLog->Write("TEMailOutbound initialized.");
}

//---------------------------------------------------------------------------

TEMailOutbound::~TEMailOutbound()
{
    if (EMailSender)
        delete EMailSender;
}

//---------------------------------------------------------------------------

bool TEMailOutbound::ProcessFile(AnsiString _fileName)
{
    bool bResult=false;
    if(!FileExists(_fileName))
    {
        m_pLog->Write((boost::format(" File: %1%, no exist") % TruncateFileName(_fileName).c_str()).str().c_str());
        return true;
    }
    try
    {
        try
        {
            if (EMailSender)
            {
                delete EMailSender;
                EMailSender=NULL;
            }
            m_pLog->Write((boost::format(" Processing file: %1%") % TruncateFileName(_fileName).c_str()).str().c_str());
            if (!terminating)
            {
                EMailSender  = new TEMailSender(_fileName, m_pCfg, m_pLog, m_pFileMap);
                if (EMailSender)
                {
                    busy=true;
                    bResult = EMailSender->Process();
                    busy=false;
                }
            }
        }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
        }
    }
    __finally
    {
        if (EMailSender)
        {
            delete EMailSender;
            EMailSender=NULL;
        }
    }
    return bResult;
}

//---------------------------------------------------------------------------
