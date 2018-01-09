//---------------------------------------------------------------------------
#ifndef TEvroSetLibH
#define TEvroSetLibH
//---------------------------------------------------------------------------
#include "LogClass.h"
//#include "TWConfig.h"
//---------------------------------------------------------------------------
typedef __stdcall int (*TCreateConnection) (char* publicKeyPath, char* address, unsigned short portNum);
typedef __stdcall int (*TCloseConnection) ();
typedef __stdcall int (*TExportPublicKey) (char* path);
typedef __stdcall int (*TProcessCommand) (const long timeStamp, const char code, const long long body, char* additional, short additionalSize, long &resultTimeStamp, char &resultCode, long long &result, char** serverData, int &serverDataLength);

const int esOK = 0;

const int esRetOK = 1;
const int esRetError = 0;

//enum esAnswer { esError , esOk };
enum esErrorCode { esErrorBasketNotFound = 1, esErrorBasketAlreadyPaid = 2, esErrorOtherError = 4 };

//---------------------------------------------------------------------------
class TEvroSetLib
{
  HINSTANCE EvroSet_DLL;

  TCreateConnection _CreateConnection;
  TCloseConnection _CloseConnection;
  TExportPublicKey _ExportPublicKey;
  TProcessCommand _ProcessCommand;

  TLogClass* Log;
  bool InnerLog;
  AnsiString ShowError(AnsiString Header);
  AnsiString ServerPublicKeyPath;
  AnsiString ClientPublicKeyPath;
  AnsiString address;
  unsigned short portNum;

public:
  TEvroSetLib(TLogClass *_Log);
//  TEvroSetLib(AnsiString _publicKeyPath, AnsiString _address, unsigned short _portNum, TLogClass *_Log);
  ~TEvroSetLib();
  int Connect();
  int Disconnect();
  int ExportPublicKey(AnsiString publicKeyPath);
//  int ProcessCommand();
  int GetAmount(long long BasketNum, double &BasketAmount, AnsiString Comment);
  int GetPin(long long BasketNum, AnsiString &PinData, AnsiString Comment);
  int ProcessCommand(const char code, const long long body, AnsiString additional, char &resultCode, long long &result, AnsiString &serverData);
};
//---------------------------------------------------------------------------
#endif
