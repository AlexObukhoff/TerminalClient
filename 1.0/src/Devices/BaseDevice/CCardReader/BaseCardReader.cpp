//---------------------------------------------------------------------------


#pragma hdrstop

#include "BaseCardReader.h"

//---------------------------------------------------------------------------

CBaseCardReader::CBaseCardReader(unsigned int comPortNum, std::string prefix, TLogClass* log)
    : TDeviceClass(comPortNum, log, prefix.c_str()), IsWaitingData(false)
{
    // from TDeviceClass
    GlobalStop = true;
}

void CBaseCardReader::StartWaitData()
{
    IsWaitingData = true;
    // вот думаю что лучше делать.. StopPooling() или GlobalStop = true
    StopPooling();
    StartPooling();
}

void CBaseCardReader::StopWaitData()
{
    IsWaitingData = false;
    StopPooling();
}

int CBaseCardReader::Initialize()
{
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// implementation of "null object" cardReader
CNullCardReader::CNullCardReader(unsigned int comPortNum, TLogClass* pLog)
    : CBaseCardReader(comPortNum, "NullCardReader", pLog)
{
}

CNullCardReader::~CNullCardReader()
{
}

void CNullCardReader::StartWaitData()
{
}

void CNullCardReader::StopWaitData()
{
}

void CNullCardReader::Start()
{
}
void CNullCardReader::Stop()
{
}
#pragma package(smart_init)
