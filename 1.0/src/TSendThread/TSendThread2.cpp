//---------------------------------------------------------------------------


#pragma hdrstop

#include "TSendThread2.h"
#include <boost/format.hpp>
#include "SevenZipVCL.hpp"
#include "globals.h"
#include "CryptLib2.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)
__fastcall TSendThread2::TSendThread2(TWConfig* cfg, TStatPacket *packet, TLogClass* log, TFileMap *fileMap)
    : TThread(true)
{
    m_pCfg = cfg;
    m_pPacket = packet;
    m_pFileMap = fileMap;

    if(log == 0)
    {
        m_pLog = new TLogClass("TSendThread");
        m_isInnerLog = true;
    }
    else
    {
        m_pLog = log;
        m_isInnerLog = false;
    }

    m_pTCPClient = 0;
    m_pTCPClient = new TIdTCPClient(Application);
    if (cfg->Connection().HTTPProxy.Type.find("socks") != std::string::npos)
        m_pTCPClient->SocksInfo->Assign(cfg->Connection().Proxy);

    m_pTCPClient->Host = cfg->GetStatServerHost();
    m_pTCPClient->Port = cfg->GetStatServerPort();

    Finished = false;
    ConnectOK = false;
    HeartBeatOK = false;
    Sent = false;

    m_serverAnswer.push_back("OK");
    m_serverAnswer.push_back("terminal_not_exist");
    m_serverAnswer.push_back("server database error");
    m_serverAnswer.push_back("file saving error");
    m_serverAnswer.push_back("unknown packet");
    m_serverAnswer.push_back("session number not found");
}

__fastcall TSendThread2::~TSendThread2()
{
    if(m_isInnerLog)
        delete m_pLog;

    if(m_pTCPClient)
        delete m_pTCPClient;
}

void __fastcall TSendThread2::Execute()
{
    TestConnectionResult = "";
    Finished = false;

    try
    {
        m_pLog->Write((boost::format("connecting to: %1%:%2%") % m_pTCPClient->Host.c_str() % m_pTCPClient->Port).str().c_str());
        m_pTCPClient->Connect();
        ConnectOK = true;

        if(handshake() && prepareSend())
        {
            // отсылаем начальный пакет
            m_pLog->WriteBuffer(m_sendBuffer, m_sendBufferSize);
            m_pTCPClient->WriteBuffer((const void*)m_sendBuffer, m_sendBufferSize, true);

            // при необходимости отсылаем файл
            if(m_pPacket->PacketType == cnFileSend || m_pPacket->PacketType == cnFileSendNew)
            {
                std::string fileName = m_pPacket->PacketFileName.substr(0, m_pPacket->PacketFileName.length() - 3) + "file";
                m_pTCPClient->WriteFile(fileName.c_str(), false);
            }

            m_pLog->Write("receiving server answer... ");
            int cmd = m_pTCPClient->ReadInteger(false);
            m_pLog->Append("done");

            // обрабатываем ответ сервера
            if(m_pPacket->PacketType == cnHeartBeat)
            {
                processHeartBeatAnswer(cmd);
                HeartBeatOK = true;
            }
            else
            {
                processOtherAnswer(cmd);
            }

            Sent = true;
            m_pLog->Write("operation completed");
            TestConnectionResult = "Connected to " + m_pTCPClient->Host + " successfully.";
        }
    }
    catch (Exception &exception)
    {
        TestConnectionResult = "Connect to "+m_pTCPClient->Host+":"+AnsiString(m_pTCPClient->Port)+" error: "+exception.Message;
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
    }
    catch(...)
    {
        TestConnectionResult = "Connect to " + m_pTCPClient->Host + ":" + AnsiString(m_pTCPClient->Port) + " error: ?";
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
    }

    if(m_pTCPClient->Connected())
        m_pTCPClient->Disconnect();
    Finished = true;
}

bool TSendThread2::prepareSend()
{
    m_pLog->Write("preparing message content for sending");

    bool result = true;
    m_sendBufferSize = 0;

    addToSendBuffer(m_pPacket->PacketType);
    addToSendBuffer(AnsiString(m_pCfg->Terminal.Number.c_str()).ToInt());

    switch(m_pPacket->PacketType)
    {
        case cnPaymentInit:
            preparePaymentInit();
            break;
        case cnIncassation:
            prepareIncassation();
            break;
        case cnHeartBeat:
            prepareHeartBeat();
            break;
        case cnError:
            prepareError();
            break;
        case cnPaymentStatusChange:
            preparePaymentStatusChange();
            break;
        case cnPaymentComplete:
            preparePaymentComplete();
            break;
        case cnTestConnection:
            prepareTestConnection();
            break;
        case cnCommandProcessed:
            prepareCommandProcess();
            break;
        case cnFileSend:
        case cnFileSendNew:
            result = prepareFileSend();
            break;
        default:
            m_pLog->Write((boost::format("Unknown packet type: %1%") % m_pPacket->PacketType).str().c_str());
            break;
    }

    return result;
}

void TSendThread2::preparePaymentInit()
{
    addToSendBuffer((double)m_pPacket->EventDateTime);
    addToSendBuffer(m_pPacket->OperatorID);
    addToSendBuffer(m_pPacket->InitialSessionNum, false, 20);
    addToSendBuffer((int)(m_pPacket->Comission * 100));

    std::size_t notesSize = m_pPacket->vNotes.size();
    addToSendBuffer((int)notesSize);

    for(std::size_t i = 0; i < notesSize; i++)
    {
        addToSendBuffer(m_pPacket->vNotes[i].ValidatorID);
        addToSendBuffer(m_pPacket->vNotes[i].CurrencyID.c_str(), false);
        addToSendBuffer((int)m_pPacket->vNotes[i].Nominal);
        addToSendBuffer(m_pPacket->vNotes[i].Count);
    }
    addToSendBuffer(m_pPacket->GetParamValue("params"));
}

void TSendThread2::prepareIncassation()
{

    addToSendBuffer((double)m_pPacket->EventDateTime);
    
    std::size_t notesSize = m_pPacket->vNotes.size();
    addToSendBuffer((int)notesSize);

    for(std::size_t i = 0; i < notesSize; i++)
    {
        addToSendBuffer(m_pPacket->vNotes[i].ValidatorID);
        addToSendBuffer(m_pPacket->vNotes[i].CurrencyID.c_str(), false);
        addToSendBuffer((int)m_pPacket->vNotes[i].Nominal);
        addToSendBuffer(m_pPacket->vNotes[i].Count);
    }
}

void TSendThread2::prepareHeartBeat()
{
    addToSendBuffer(short(m_pPacket->ErrorCode));
    addToSendBuffer(short(m_pPacket->Status));
    addToSendBuffer(int(m_pPacket->BillCount));
    addToSendBuffer(int(m_pPacket->BillSum));
    addToSendBuffer(int(m_pPacket->ErrType));
    addToSendBuffer(__int8(m_pPacket->ErrSubType));
    addToSendBuffer(int(m_pPacket->ChequeCounter));
}

void TSendThread2::prepareError()
{
    addToSendBuffer(double(m_pPacket->EventDateTime));
    addToSendBuffer(m_pPacket->ErrType);
    addToSendBuffer(m_pPacket->ErrSubType);
    addToSendBuffer(m_pPacket->ErrSender + "|" + m_pPacket->ErrDescription + "|" + m_pPacket->ErrSubDescription);
}

void TSendThread2::preparePaymentStatusChange()
{
    addToSendBuffer(double(m_pPacket->EventDateTime));
    addToSendBuffer(m_pPacket->InitialSessionNum, false, 20);
    addToSendBuffer(short(m_pPacket->ErrorCode));
    addToSendBuffer(short(m_pPacket->Status));
}

void TSendThread2::preparePaymentComplete()
{
    addToSendBuffer(double(m_pPacket->EventDateTime));
    addToSendBuffer(m_pPacket->InitialSessionNum, false, 20);
    addToSendBuffer(m_pPacket->SessionNum, false, 20);
    addToSendBuffer(m_pPacket->ErrorCode);
}

void TSendThread2::prepareTestConnection()
{
    addToSendBuffer(m_pPacket->Status);
    if(m_pPacket->ForcedURL != "")
    {
        if(m_pPacket->ForcedURL.Pos(":"))
        {
            m_pTCPClient->Host = m_pPacket->ForcedURL.SubString(0, m_pPacket->ForcedURL.Pos(":") - 1);
            m_pTCPClient->Port = GetInt(m_pPacket->ForcedURL.SubString(m_pPacket->ForcedURL.Pos(":") + 1, m_pPacket->ForcedURL.Length()));
        }
        else
        {
            m_pTCPClient->Host = m_pPacket->ForcedURL;
            m_pTCPClient->Port = 10024;
        }
    }
}

void TSendThread2::prepareCommandProcess()
{
    addToSendBuffer(m_pPacket->Status);
}

bool TSendThread2::prepareFileSend()
{
    bool result = false;
    AnsiString fileName = (m_pPacket->PacketFileName.substr(0, m_pPacket->PacketFileName.length() - 3) + "file").c_str();

    if(!FileExists(fileName))
    {
        m_pLog->Write((boost::format("packet send failed - file does not exists: %1%") % fileName.c_str()).str().c_str());
    }
    else
    {
        addToSendBuffer(m_pPacket->SendFileName.Length());
        addToSendBuffer(m_pPacket->SendFileName, false);
        addToSendBuffer((int)FileSizeByName(fileName));
        result = true;
    }
    
    return result;
}

template<class T>
void TSendThread2::addToSendBuffer(T value)
{
    if(m_sendBufferSize + sizeof(T) < MaxSendBufferSize)
    {
        memcpy(m_sendBuffer + m_sendBufferSize, &value, sizeof(T));
        m_sendBufferSize += sizeof(T);
    }
    else
    {
        m_pLog->Write("buffer overflow. cannot add data");
    }
}

void TSendThread2::addToSendBuffer(AnsiString value, bool isZeroTerminated, std::size_t size)
{
    for (std::size_t i = value.Length(); i < size; i++)
        value += " ";

    size_t bufferEx = isZeroTerminated ? 1 : 0;
    if (m_sendBufferSize + value.Length() + bufferEx < MaxSendBufferSize)
    {
        memcpy(m_sendBuffer+m_sendBufferSize, value.c_str(), value.Length());
        m_sendBufferSize += value.Length();

        if(isZeroTerminated)
        {
            m_sendBuffer[m_sendBufferSize] = '\0';
            m_sendBufferSize++;
        }
    }
    else
    {
        m_pLog->Write("buffer overflow. cannot add data");
    }
}

bool TSendThread2::handshake()
{
    m_pLog->Write("handshake");
    if(!m_pCfg->StatInfo.IsSignStatistics)
    {
        m_pLog->Write("statpackets not require signing - handshake succesfull");
        return true;
    }
    
    bool result = false;

    std::string outMsg = (boost::format("::%1%::") % m_pCfg->Terminal.Number).str();
    AnsiString signOutMsg = crypt::sign(m_pCfg->GetKeysNum(), outMsg.c_str()).c_str();

    m_pTCPClient->WriteInteger(cnHandshake, false);  // признак handshake
    m_pTCPClient->WriteInteger(signOutMsg.Length(), false);
    m_pTCPClient->Write(signOutMsg);

    int replyByte = m_pTCPClient->ReadInteger(false);
    if(replyByte == cnHandshake)
    {
        m_pLog->Write("server accepted signed message. checking server sign");
        int inMsgSize = m_pTCPClient->ReadInteger(false);
        AnsiString inSignMsg = m_pTCPClient->ReadString(inMsgSize);

        AnsiString inMsg = crypt::verify(m_pCfg->GetKeysNum(), inSignMsg.c_str()).c_str();
        if(inMsg == "accepted")
        {
            result = true;
            m_pLog->Write("server sign accepted. handshake succeed");
        }
        else
        {
            m_pLog->Write("server sign rejected. handshake fail");
        }
    }
    else
    {
        m_pLog->Write("server rejected signed message. handshake fail");
    }

    return result;
}

void TSendThread2::processHeartBeatAnswer(int cmd)
{
    m_pCfg->StatInfo.DTLastSuccessfullPacketSending = TDateTime::CurrentDateTime();
    do
    {
        m_pLog->Write((boost::format("TSendThread2::processHeartBeatAnswer - cmd == %1%") % cmd).str().c_str());
      //m_pLog->Write((boost::format("failed send packet - unknown answer^ %1%") % cmd).str().c_str());
        if(cmd == cnCmdNone)
        {
            m_pCfg->StatInfo.Inhibit = false;
            break;
        }
        else if(cmd == cnCmdInhibitPktSend)
        {
            m_pCfg->StatInfo.Inhibit = true;
            m_pLog->Write("server does not accept statistics (inhibit = true)");
            break;
        }
        else
        {
            // обрабатываем команду
            if(m_pFileMap && m_pCfg->StatInfo.Inhibit)
                m_pFileMap->CheckStatDir = true;

            m_pCfg->StatInfo.Inhibit = false;
            int cmdUID = m_pTCPClient->ReadInteger(false);
            cmd = receiveCommand(cmd, cmdUID);
        }

    }while(cmd != 0 && cmd != 6 && cmd != -1);
}

void TSendThread2::processOtherAnswer(int cmd)
{
    if(cmd == 0 && m_pPacket->PacketType == cnCommandProcessed)
    {
        // успешная отправка
        AnsiString fileName = AnsiString(m_pCfg->Dirs.CommandsInbound.c_str()) + "\\" + AnsiString(m_pPacket->Status);
        DeleteFile(fileName + ".pkt");
        DeleteFile(fileName + ".ok");
    }

    if(cmd < (int)m_serverAnswer.size() && cmd > -1)
        m_pLog->Write(m_serverAnswer[cmd].c_str());
    else
        m_pLog->Write((boost::format("failed send packet - unknown answer^ %1%") % cmd).str().c_str());
}

int TSendThread2::receiveCommand(int cmd, int cmdUID)
{
    int result = -1;
    std::auto_ptr<TCommandReceiver> cmdRecv(new TCommandReceiver("", m_pCfg, m_pLog, m_pFileMap));

    switch(cmd)
    {
        case cnCmdReboot:
            cmdReboot(cmdRecv.get(), cmdUID);
            break;
        case cnCmdReceiveFileOld:
            cmdReceiveFileOld(cmdRecv.get(), cmdUID);
            break;
        case cnCmdSendConfigOld:
            cmdSendConfigOld(cmdRecv.get(), cmdUID);
            break;
        case cnCmdCancelPayment:
            cmdCancelPayment(cmdRecv.get(), cmdUID);
            break;
        case cnCmdSendFile:
            cmdSendFile(cmdRecv.get(), cmdUID);
            break;
        case cnCmdReceiveFile:
            cmdReceiveFile(cmdRecv.get(), cmdUID);
            break;
        case cnCmdResurrectPayment:
            cmdResurrectPayment(cmdRecv.get(), cmdUID);
            break;
        case cnCmdShutDown:
            cmdShutdown(cmdRecv.get(), cmdUID);
            break;
        case cnCmdBlock:
            cmdBlock(cmdRecv.get(), cmdUID);
            break;
        case cnCmdUnblock:
            cmdUnblock(cmdRecv.get(), cmdUID);
            break;
        case cnCmdGetKeys:
            cmdGetKeys(cmdRecv.get(), cmdUID);
            break;
        default:
            m_pLog->Write((boost::format("unknown server command: %1%") % cmd).str().c_str());
            break;
    }
    return result;
}

void TSendThread2::cmdReboot(TCommandReceiver* cmdRecv, int cmdUID)
{
    m_pLog->Write("receive cmd: reboot");
    if(!cmdRecv->StoreRebootCommand(cmdUID))
    {
        m_pLog->Write("error saving cmd. trying self-reboot...");
        //reboot();
    }
}

void TSendThread2::cmdReceiveFileOld(TCommandReceiver* cmdRecv, int cmdUID)
{
    m_pLog->Write("receive cmd: cmdReceiveFileOld");
    int fileNameSize = m_pTCPClient->ReadInteger(false);
    AnsiString fileName = m_pTCPClient->ReadString(fileNameSize);
    int fileSize = m_pTCPClient->ReadInteger(false);

    m_pLog->Write((boost::format("receiving file_old: %1%, size: %2%") % fileName.c_str() % fileSize).str().c_str());
    AnsiString localFileName = AnsiString(m_pCfg->Dirs.CommandsInbound.c_str()) + "\\" + AnsiString(cmdUID) + ".pkt";
    std::auto_ptr<TFileStream> incomingFile(new TFileStream(localFileName, fmCreate | fmShareDenyNone));
    m_pTCPClient->ReadStream(incomingFile.get(), fileSize, false);
    cmdRecv->StoreReceiveFileCommand(cmdUID, fileName, fileSize, NULL);

    m_pLog->Write((boost::format("file received successfully: %1%") % localFileName.c_str()).str().c_str());
}

void TSendThread2::cmdSendConfigOld(TCommandReceiver* cmdRecv, int cmdUID)
{
    m_pLog->Write("receive cmd: cmdReceiveConfigOld");
    cmdRecv->StoreSendFileCommand(cmdUID, "");
}

void TSendThread2::cmdCancelPayment(TCommandReceiver* cmdRecv, int cmdUID)
{
    m_pLog->Write("receive cmd: cmdCancelPayment");

    AnsiString session = m_pTCPClient->ReadString(20);
    m_pLog->Write((boost::format("cmdUID = %1%, session = %2%") % cmdUID % session.c_str()).str().c_str());
    cmdRecv->StoreCancelPaymentCommand(cmdUID, session);
}

void TSendThread2::cmdSendFile(TCommandReceiver* cmdRecv, int cmdUID)
{
    m_pLog->Write("receive cmd: cmdSendFile");

    AnsiString fileMask = m_pTCPClient->ReadString(20);
    cmdRecv->StoreGetFileByMaskCommand(cmdUID, fileMask);
}

void TSendThread2::cmdReceiveFile(TCommandReceiver* cmdRecv, int cmdUID)
{
    m_pLog->Write("receive cmd: cmdReceiveFile");

    int fileNameSize = m_pTCPClient->ReadInteger(false);
    AnsiString fileName = m_pTCPClient->ReadString(fileNameSize);
    int fileSize = m_pTCPClient->ReadInteger(false);

    m_pLog->Write((boost::format("receiving file: %1%, size: %2%") % fileName.c_str() % fileSize).str().c_str());

    AnsiString localFileName = AnsiString(m_pCfg->Dirs.CommandsInbound.c_str()) + "\\" + fileName + AnsiString(fileSize) + ".part";

    int startOffset = 0;
    WORD streamMode = 0;
    if(FileExists(localFileName))
    {
       startOffset = FileSizeByName(localFileName);
       streamMode = fmOpenWrite | fmShareExclusive;
    }
    else
    {
        streamMode = fmCreate | fmShareExclusive;
    }

    std::auto_ptr<TFileStream> fs(new TFileStream(localFileName, streamMode));
    fs->Seek(startOffset, 0);
    // говорим с какой части надо закачивать
    m_pTCPClient->WriteInteger(startOffset, false);

    int receivedSize = 0;
    while(receivedSize + startOffset < fileSize)
    {
        // получаем части файла через m_sendBuffer
        int partSize = min(fileSize - (receivedSize + startOffset), MaxSendBufferSize);
//        m_pLog->Write((boost::format("received: %1% bytes. next part: %2% bytes") % receivedSize % partSize).str().c_str());
        m_pTCPClient->ReadBuffer(m_sendBuffer, partSize);
        fs->Write(m_sendBuffer, partSize);
        receivedSize += partSize;
    }

    FileClose(fs->Handle);
    fs.release(); // release auto_ptr

    m_pLog->Write((boost::format("received done. total: %1% bytes") % receivedSize).str().c_str());
    int fileResult = 0;
    std::string fileResultDescr = "file recieved OK";
    if(localFileName.LowerCase().Pos(".7z") > 0)
    {
        if(!testArchive(localFileName))
        {
            fileResult = 1;
            fileResultDescr = "corrupted 7z arch. deleting";
            DeleteFile(localFileName);
        }
    }

    m_pLog->Write(fileResultDescr.c_str());
    m_pTCPClient->WriteInteger(fileResult, false);
    if(fileResult == 0)
    {
        RenameFile(localFileName, AnsiString(m_pCfg->Dirs.CommandsInbound.c_str()) + "\\" + AnsiString(cmdUID) + ".file");
        DeleteFile(localFileName);
        cmdRecv->StoreReceiveFileCommand(cmdUID, fileName, fileSize, 0);
    }
}

void TSendThread2::cmdResurrectPayment(TCommandReceiver* cmdRecv, int cmdUID)
{
    m_pLog->Write("receive cmd: cmdResurrectPayment");

    AnsiString session = m_pTCPClient->ReadString(20);
    int paramsSize = m_pTCPClient->ReadInteger(false);
    AnsiString paramsValues = m_pTCPClient->ReadString(paramsSize);

    char* res = xcode(paramsValues.c_str(), 1251, 1252);
    AnsiString decodedValues = res;
    delete[] res;

    m_pLog->Write((boost::format("cmdUID = %1%, session = %2%, paramsValues = %3%") % cmdUID % session.c_str() % decodedValues.c_str()).str().c_str());
    cmdRecv->StoreResurrectPaymentCommand(cmdUID, session, decodedValues);
}

void TSendThread2::cmdShutdown(TCommandReceiver* cmdRecv, int cmdUID)
{
    m_pLog->Write("receive cmd: cmdShutdown");

    if(!cmdRecv->StoreShutDownCommand(cmdUID))
    {
        m_pLog->Write("error saving cmd. trying self-shutdown...");
        //shutdown();
    }
}

void TSendThread2::cmdBlock(TCommandReceiver* cmdRecv, int cmdUID)
{
    m_pLog->Write("receive cmd: cmdBlock");
    cmdRecv->StoreBlockCommand(cmdUID);
}

void TSendThread2::cmdUnblock(TCommandReceiver* cmdRecv, int cmdUID)
{
    m_pLog->Write("receive cmd: cmdUnblock");
    cmdRecv->StoreUnblockCommand(cmdUID);
}

void TSendThread2::cmdGetKeys(TCommandReceiver* cmdRecv, int cmdUID)
{
    m_pLog->Write("receive cmd: cmdGetKeys");
    int keysID = m_pTCPClient->ReadInteger(false);
    int paramsSize = m_pTCPClient->ReadInteger(false);
    AnsiString paramValues = m_pTCPClient->ReadString(paramsSize);

    m_pLog->Write((boost::format("keysId = %1%, paramValues = %2%") % keysID % paramValues.c_str()).str().c_str());
    cmdRecv->StoreGetKeysCommand(cmdUID, keysID, paramValues);
}


// P.S. - накуя тут эти методы...
bool TSendThread2::testArchive(AnsiString fileName)
{
    bool result = false;
    std::auto_ptr<TSevenZip> sz(new TSevenZip(NULL));

    sz->SZFileName = WideString(fileName);
    sz->Files->Clear();
    sz->Extract(true);
    if(sz->ErrCode == 0)
        result = true;

    return result;
}

void TSendThread2::shutdown(bool reboot)
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); // Get the LUID for the shutdown privilege.

    tkp.PrivilegeCount = 1;  // one privilege to set
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); // Get the shutdown privilege for this process.

    long mode = 0;
    if(reboot)
        mode = EWX_REBOOT | EWX_FORCE;
    else
        mode = EWX_SHUTDOWN | EWX_FORCE;                                                                                                                                                        // Cannot test the return value of AdjustTokenPrivileges.
    ExitWindowsEx(mode, 0);

    Application->Terminate();
}

char* TSendThread2::xcode(const char* src, int srcCP, int dstCP)
{
    int wsize = MultiByteToWideChar(srcCP, 0, src, -1, NULL, 0);
    LPWSTR wbuf = new WCHAR[wsize];
    MultiByteToWideChar(srcCP, 0, src, -1, wbuf, wsize);
    int size = WideCharToMultiByte(dstCP, 0, wbuf, -1, NULL, 0, NULL, NULL);
    char* buf = new char[size];
    WideCharToMultiByte(dstCP, 0, wbuf, -1, buf, size, NULL, NULL);
    delete[] wbuf;

    return buf;
}

