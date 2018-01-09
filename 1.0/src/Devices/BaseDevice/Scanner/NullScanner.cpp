//---------------------------------------------------------------------------


#pragma hdrstop

#include "NullScanner.h"

//---------------------------------------------------------------------------
CNullScanner::CNullScanner(TLogClass* logClass)
: CScanner(0, "NullScanner", logClass)
{
}

CNullScanner::~CNullScanner()
{
}

void CNullScanner::Start()
{
 Log->Write("NullScanner - Start");
}

void CNullScanner::Stop()
{
 Log->Write("NullScanner - Stop");
}

void CNullScanner::StartDevice()
{
}

#pragma package(smart_init)
