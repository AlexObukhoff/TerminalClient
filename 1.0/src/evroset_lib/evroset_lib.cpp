//---------------------------------------------------------------------------

#include <IniFiles.hpp>
#include <Classes.hpp>
#include <time.h>
#pragma hdrstop

#include "evroset_lib.h"
#pragma package(smart_init)

#include "globals.h"
#include "boost/format.hpp"

//---------------------------------------------------------------------------
//TEvroSetLib::TEvroSetLib(AnsiString _publicKeyPath, AnsiString _address, unsigned short _portNum, TLogClass *_Log)
TEvroSetLib::TEvroSetLib(TLogClass *_Log)
{
Log = _Log;
InnerLog = false;
if (_Log==NULL)
  {
  Log = new TLogClass("TEvroSetClient");
  InnerLog=true;
  }
/*publicKeyPath = _publicKeyPath;
address = _address;
portNum = _portNum;*/

TIniFile *ini = NULL;
try
  {
  ini = new TIniFile(ExtractFilePath(Application->ExeName) + "EvrosetClient.ini");
  if (ini)
    {
    if (!FileExists(ExtractFilePath(Application->ExeName) + "EvrosetClient.ini"))
      {
      ini->WriteString( "EvrosetClient", "ServerPublicKeyPath", "server.key" );
      ini->WriteString( "EvrosetClient", "ClientPublicKeyPath", "client.key" );
      ini->WriteString( "EvrosetClient", "ServerAddress", "127.0.0.1" );
      ini->WriteInteger( "EvrosetClient", "ServerPort", 8142 );
      }
    ServerPublicKeyPath = ini->ReadString( "EvrosetClient", "ServerPublicKeyPath", "server.key" );
    ClientPublicKeyPath = ini->ReadString( "EvrosetClient", "ClientPublicKeyPath", "client.key" );
    address = ini->ReadString( "EvrosetClient", "ServerAddress", "127.0.0.1" );
    portNum = ini->ReadInteger( "EvrosetClient", "ServerPort", 8142 );
    }
  }
__finally
  {
  if (ini)
    delete ini;
  }

EvroSet_DLL = NULL;
EvroSet_DLL = LoadLibrary("EvrosetClient.dll");
Log->Write(ShowError("load EvrosetClient.dll").c_str());
if(!EvroSet_DLL)
  {
  Log->Write("EvroSet_DLL: Can't load EvrosetClient.dll!");
  return;
  }
else
  {
  //Init        = (TInit)       GetProcAddress(EvroSet_DLL, "Init");
  _CreateConnection = (TCreateConnection) GetProcAddress(EvroSet_DLL, "?CreateConnection@@YAHPAD0G@Z");
  if (!_CreateConnection)
    Log->Write(ShowError("Error getting CreateConnection from dll!").c_str());

  _CloseConnection = (TCloseConnection) GetProcAddress(EvroSet_DLL, "?CloseConnection@@YAHXZ");
  if (!_CloseConnection)
    Log->Write(ShowError("Error getting CloseConnection from dll!").c_str());

  _ExportPublicKey = (TExportPublicKey) GetProcAddress(EvroSet_DLL, "?ExportPublicKey@@YAHPAD@Z");
  if (!_ExportPublicKey)
    Log->Write(ShowError("Error getting ExportPublicKey from dll!").c_str());
    
  _ProcessCommand = (TProcessCommand) GetProcAddress(EvroSet_DLL, "?ProcessCommand@@YAHJD_JPADFAAJAADAA_JPAPADAAH@Z");
//_ProcessCommand = (TProcessCommand) GetProcAddress(EvroSet_DLL, "?ProcessCommand@@YAH_JD0PADFAA_JAAD2PAPADAAH@Z");
  if (!_ProcessCommand)
    Log->Write(ShowError("Error getting ProcessCommand from dll!").c_str());

  if ((_CreateConnection==NULL)||(_CloseConnection==NULL)||(_ExportPublicKey==NULL)||(_ProcessCommand==NULL))
    {
    Log->Write("EvroSetClient init failed: error getting info from EvrosetClient.dll!");
    FreeLibrary(EvroSet_DLL);
    EvroSet_DLL = NULL;
    return;
    }
/*  if (!FileExists(ClientPublicKeyPath))
    {
    ExportPublicKey(ClientPublicKeyPath);
    }*/
  }
}

//---------------------------------------------------------------------------


TEvroSetLib::~TEvroSetLib()
{
if (InnerLog)
  {
  delete Log;
  Log = NULL;
  }

if(EvroSet_DLL)
  {
  FreeLibrary(EvroSet_DLL);
  EvroSet_DLL = NULL;
  }
}

//---------------------------------------------------------------------------

int TEvroSetLib::Connect()
{
int Res = 0;
try
  {
  if((EvroSet_DLL)&&(_CreateConnection))
    {
    Res = _CreateConnection(ServerPublicKeyPath.c_str(), address.c_str(), portNum);
    Log->Write((boost::format("Connect result: %1%") % Res).str().c_str());
    }
    else
    Log->Write("Connect error: EvroSet.dll not loaded!");
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
return Res;
}

//---------------------------------------------------------------------------

int TEvroSetLib::Disconnect()
{
int Res = 0;
try
  {
  if((EvroSet_DLL)&&(_CloseConnection))
    {
    Res = _CloseConnection();
    Log->Write((boost::format("Disconnect result: %1%") % Res).str().c_str());
    }
    else
    Log->Write("Disconnect error: EvroSet.dll not loaded!");
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
return Res;
}

//---------------------------------------------------------------------------

int TEvroSetLib::ExportPublicKey(AnsiString publicKeyPath)
{
int Res = 0;
try
  {
  if((EvroSet_DLL)&&(_ExportPublicKey))
    {
    Res = _ExportPublicKey(publicKeyPath.c_str());
    Log->Write((boost::format("ExportPublicKey result: %1%") % Res).str().c_str());
    }
    else
    Log->Write("ExportPublicKey error: EvroSet.dll not loaded!");
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
return Res;
}

//---------------------------------------------------------------------------

int TEvroSetLib::ProcessCommand(const char code, const long long body, AnsiString additional, char &_resultCode, long long &_result, AnsiString &serverData)
{
int Res = 0;
try
  {
  if((EvroSet_DLL)&&(_ProcessCommand))
    {
    time_t now;
    time(&now);
    time_t resultTime;
    char* resultAdditional = NULL;
    int resultAdditionalSize;
    Res = _ProcessCommand(now, code, body, additional.c_str(), additional.Length(), resultTime, _resultCode, _result, &resultAdditional, resultAdditionalSize);

    serverData = AnsiString(resultAdditional, resultAdditionalSize);

    Log->Write
       (
        (boost::format("ProcessCommand result: %1% Result: code %2%, body %3%, additional data %4%")
         % Res % _resultCode % _result % serverData.c_str()
         ).str().c_str()
       );
    }
    else
    Log->Write("ProcessCommand error: EvroSet.dll not loaded!");
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
return Res;
}

//---------------------------------------------------------------------------

AnsiString TEvroSetLib::ShowError(AnsiString Header)
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
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        }
	}
__finally
	{
	LocalFree(lpMsgBuf);
	return Temp;
	}
}

//---------------------------------------------------------------------------

int TEvroSetLib::GetAmount(long long BasketNum, double &BasketAmount, AnsiString Comment)
{
try
  {
  AnsiString serverData;
  char retCode;
  long long retBody;
  int res = ProcessCommand(2, BasketNum, Comment, retCode, retBody, serverData);
  if (res==esOK)
    {
    if (retCode==esRetOK)
      {
      BasketAmount = double(retBody);
      return esOK;
      }
      else
      {
      BasketAmount = 0;
      switch (retBody)
        {
        case esErrorBasketNotFound:
          Log->Write((boost::format("Get amount error: basket %1% not found") % BasketNum).str().c_str());
          break;
        case esErrorBasketAlreadyPaid:
          Log->Write((boost::format("Get amount error: basket %1% has already been paid") % BasketNum).str().c_str());
          break;
        default:
          Log->Write((boost::format("Get amount error #%1%") % retBody).str().c_str());
          break;
        }
      return retBody;
      }
    }
    else
    {
    Log->Write((boost::format("Process command error #%1%") % res).str().c_str());
    return res;
    }
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

int TEvroSetLib::GetPin(long long BasketNum, AnsiString &PinData, AnsiString Comment)
{
try
  {
  AnsiString serverData;
  char retCode;
  long long retBody;
  int res = ProcessCommand(4, BasketNum, Comment, retCode, retBody, serverData);
  if (res==esOK)
    {
    if (retCode==esRetOK)
      {
      char sBody[8];
      memcpy(sBody, &retBody, 8);
      PinData = AnsiString(sBody,8);
      return esOK;
      }
      else
      {
      PinData = "";
      switch (retBody)
        {
        case esErrorBasketNotFound:
          Log->Write((boost::format("Get pin error: basket %1% not found") % BasketNum).str().c_str());
          break;
        case esErrorBasketAlreadyPaid:
          Log->Write((boost::format("Get pin error: basket %1% has already been paid!") % BasketNum).str().c_str());
          break;
        default:
          Log->Write((boost::format("Get pin error #%1%") % retBody).str().c_str());
          break;
        }
      return retBody;
      }
    }
    else
    {
    Log->Write((boost::format("Process command error #%1%") % res).str().c_str());
    return res;
    }
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}


/*

�������� ���������
������ ��������� ������� �� ���� �������� ������:
1.     ���� � ����� (8 ����).
2.     ����������� ��� (1 ����).
3.     ������ (8 ����).
4.     ��������� ���������� (?).
������ ��������� �������
������:
����������, ����� ���������� ������ ��� ������� �� �������.
����������� ���: 2
������: 8 �����. �������� ����� �������.

�����:
� ������ ��������� �������:
����������� ���: 1
������: 8 ����. �������� ��������� �������.

� ������ ������������� ������ (������� �� ���������� ��� ��� ��������):
����������� ���: 0
������: 8 ����. �������� 1, ���� ������� �� ����������; 2, ���� ��� ��� ��������; 4, ���� �������� ������ ������.
 -----------------------------------------------------------------------------
������������� ������
������:
����������, ����� ���������� ������� �������.
����������� ���: 4
������: 8 �����. �������� ����� �������.

�����:
� ������ ��������� �������:
����������� ���: 1
������: 8 ����. �������� ���������������� ��� ��������� (����� + ��������)

� ������ ������������� ������ (������� �� ���������� ��� ��� ��������):
����������� ���: 0
������: 8 ����. �������� 1, ���� ������� �� ����������; 2, ���� ��� ��� ��������; 4, ���� �������� ������ ������.

*/
