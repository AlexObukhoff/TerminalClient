//---------------------------------------------------------------------------


#pragma hdrstop

#include "CardReader.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

CCardReader::CCardReader(int ComPort,TLogClass* _Log, AnsiString Prefix)
: TDeviceClass(ComPort,_Log, Prefix)
{
  LoggingErrors = true;

  InitInfo = new _init_info();
  FindCardInfo = new _findcard_info();
  GetMenuInfo = new _getmenu_info();
  WriteCardInfo = new _writecard_info();
}

CCardReader::~CCardReader()
{
    if (InitInfo)
        delete InitInfo;
    if (FindCardInfo)
        delete FindCardInfo;
    if (GetMenuInfo)
        delete GetMenuInfo;
    if (WriteCardInfo)
        delete WriteCardInfo;
}

int CCardReader::Init()
{
    return 0;
}

int CCardReader::FindCard()
{
    return 0;
}

int CCardReader::GetMenu()
{
    return 0;
}

int CCardReader::WriteCard()
{
    return 0;
}

void CCardReader::SetServerStatus(bool value)
{
    _ServerConnected = value;
}

bool CCardReader::GetServerStatus()
{
    return _ServerConnected;
}

