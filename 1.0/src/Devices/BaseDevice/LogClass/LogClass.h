#ifndef LogClassH
#define LogClassH
#include <vcl.h>
#include <stdio.h>
#include <SyncObjs.hpp>
#include <string>
#include "math.h"

typedef void _fastcall (__closure *ErrorCallbackFunc)();

class TLogClass
{
private:
  TFileStream*  LogFile;
  std::string _FileName;
  std::string LogName;
  std::string _Prefix;
  std::string LogTemplate;
  std::string SubFolder;
  int Counter;
  bool LogInit;
  bool Enable;
  int DateLineLength;
  int LastLineLength;

  TCriticalSection* CriticalSection;

  void InitLog(const char* FileName);
  void _Write(const char* LogString);
  void WriteWithControl(const char *Buffer, int Count);
  void InsertVersionInfo();
  bool alreadyWriteVersion;
  ErrorCallbackFunc ErrorCallback;
  TLogClass(const TLogClass&);
  operator = (const TLogClass&);

  void SetLogName();
  void InsertEmptyString();
public:
  //void _fastcall (__closure *ErrorCallback)();
  void SetLogError(ErrorCallbackFunc callback);

  TLogClass(const char *Prefix = "log", const char *_SubFolder = "");
  ~TLogClass();
  void Write(const char *_LogString);
  void Write(const std::string& str) { Write(str.c_str()); }
  void Write(const AnsiString& str)  { Write(str.c_str()); }

  void Append(const char *_LogString);
  void Append(const std::string& str) { Append(str.c_str()); }
  void Append(const AnsiString& str)  { Append(str.c_str()); }
  
  void WriteLines(const char *Lines);
  void WriteInLine(const char *Lines);
  void WriteBuffer(BYTE* Buffer, unsigned int count);
  void DeleteLastLine();

  void DisableLogging();
  void EnableLogging();
};

#endif
