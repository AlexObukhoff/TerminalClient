//---------------------------------------------------------------------------


#pragma hdrstop

#include "CSankyoCardReader.h"
#include "CSankyoCardReaderThread.h"
#include <boost/format.hpp>

//---------------------------------------------------------------------------

// команда инициализация. посылается после поднятия RTS
const BYTE cmdInit[] = {'C', 0x30, 0x40};

CSankyoCardReader::CSankyoCardReader(unsigned int comPortNum, TLogClass* pLog)
    : CBaseCardReader(comPortNum, "SankyoCardReader", pLog)
{
    Port->ReopenPort();
    // переинициалищируем COM
    ::SetupComm(Port->Port, 1200, 1200);
    DCB dcb;
    memset(&dcb, 0, sizeof(dcb));
    ::GetCommState(Port->Port, &dcb);
    dcb.BaudRate = CBR_9600;
    dcb.fParity = true;
    dcb.Parity = EVENPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.ByteSize = 8;
    ::SetCommState(Port->Port, &dcb);

    COMMTIMEOUTS cto;
    cto.ReadIntervalTimeout = MAXDWORD;
    cto.ReadTotalTimeoutMultiplier = MAXDWORD;
    cto.ReadTotalTimeoutConstant = 20;
    cto.WriteTotalTimeoutMultiplier = MAXDWORD;
    cto.WriteTotalTimeoutConstant = MAXDWORD;
    ::SetCommTimeouts(Port->Port, &cto);
}

CSankyoCardReader::~CSankyoCardReader()
{
    Stop();
}

int CSankyoCardReader::Initialize()
{
    Log->Write("initialize");
    int result = 0;

    memcpy(Command, cmdInit, sizeof(cmdInit));
    CommandSize = sizeof(cmdInit);

    ExecuteCommand(NotRecieveAnswer);

    // ответа не ждем (в случае успешного выполнения команды) но все-же проверим
    if(AnswerSize > 0)
    {
       // TODO: analyse cmdInitResult
        result = -1;
    }

//    GlobalStop = true;
    Log->Write((boost::format("initialize result: %1%") % result).str().c_str());
    return result;
}

void CSankyoCardReader::Start()
{
  DeviceState->State = Wait;
  DeviceState->Scanner = true;  // показываем что не надо анализировать предыдущее состояние для того чтобы сгенерировать событие
  DeviceThread = new CSankyoCardReaderThread();
  DeviceThread->Log = Log;
  DeviceThread->DeviceState = DeviceState;
  DeviceThread->CommandParameters = CommandParameters;
  DeviceThread->SendType = SendType;
  DeviceThread->Port = Port;
  DeviceThread->Command = Command;
  DeviceThread->CommandSize = CommandSize;
  DeviceThread->Answer = Answer;
  DeviceThread->AnswerSize = AnswerSize;
  DeviceThread->data = data;
  DeviceThread->len_data = &len_data;

  DeviceThread->CommandCriticalSection = CommandCriticalSection;

  DeviceThread->DataLengthIndex = DataLengthIndex;
  DeviceThread->BeginByte = BeginByte;
  DeviceThread->LoggingErrors = LoggingErrors;
  DeviceThread->LastError = LastError;
  DeviceThread->EndByte = EndByte;
  DeviceThread->CRCLength = CRCLength;
  DeviceThread->DataLength = DataLength;

  DeviceThread->DeviceStarted = DeviceStarted;
  DeviceThread->DeviceStopped = DeviceStopped;
  DeviceThread->DevicePaused = DevicePaused;
  DeviceThread->DeviceStateChanged = DeviceStateChanged;
  DeviceThread->CommandStarted = CommandStarted;
  DeviceThread->CommandPaused = CommandPaused;
  DeviceThread->CommandFinished = CommandFinished;

  DeviceThread->ChangeEvent = ChangeEvent;
}

#pragma package(smart_init)
