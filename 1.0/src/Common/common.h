//---------------------------------------------------------------------------

#ifndef commonH
#define commonH
#include <vcl.h>
#include <XMLDoc.hpp>

#include "LogClass.h"

template <class X> class xmlGuard
{
  X obj;
public:
  xmlGuard(const X& _object = NULL) : obj(_object) {};
  ~xmlGuard() { /*if (obj) obj.Release();*/ };
  X operator->() { return obj; };
  X get() { return obj; };
  xmlGuard& operator= (const X& C)
    {
    /*if (Assigned())
      obj.Release();*/
    obj = C;
    return *this;
    };
  bool Assigned() { return (obj != NULL); };
};

namespace SIDE
{
    enum SIDE
    {
        LEFT = -1,
        CENTER = 0,
        RIGHT = 1
    };
}

class XLog
{
  bool InnerLog;
  TLogClass* Log;
public:
  XLog() {InnerLog = false; Log = NULL;};
  ~XLog() {if (InnerLog) { delete Log; Log = NULL;} };
  void SetUp(TLogClass* _Log, AnsiString LogCaption) {if (_Log) { Log = new TLogClass(LogCaption.c_str()); InnerLog = true;} };
  TLogClass* operator ->() { return Log; };
};

  double ConvertCoinAcceptorValue(unsigned int Value, unsigned int Precession);
  AnsiString ShowError(AnsiString Header);
  std::string StripFileName(std::string FullName);
  AnsiString GetFileName(AnsiString FullName);
  AnsiString GetPath(AnsiString FullName);
  AnsiString GetExtName(AnsiString FullName);
  std::string getProjectFullPathName(std::string defaultValue = "");
  std::string getWCDirectory(std::string defaultValue = "");
  std::string getUpdaterDirectory(std::string defaultValue = "");
  std::string getUpdaterFullPathName(std::string defaultValue = "");

  std::string getRegistryTextValue(char*, char*, char* = "");
  HKEY getRegistryKey(std::string);

  double GetDouble(AnsiString);
  bool IsDouble(AnsiString);
  int GetInt(AnsiString);
  long GetLong(AnsiString);
  TDateTime GetDateTime(AnsiString);
  AnsiString ChangeChars(AnsiString Source, AnsiString OldChars, AnsiString NewChars);
  AnsiString RemoveSubStrings(AnsiString Source, AnsiString StartChars, AnsiString EndChars);
  AnsiString GetSubString(AnsiString Source, AnsiString StartChars, AnsiString EndChars);

  //AnsiString ChangeChars2(AnsiString Source, AnsiString OldChars, AnsiString NewChars);
  std::string TruncateLocation(AnsiString _fileName);

  bool StoreStringToFile(AnsiString FileName, AnsiString Content, TLogClass* Log = NULL);
  bool GetFileData(AnsiString FileName, AnsiString &Content, TLogClass* Log = NULL);

  int FileCount(AnsiString FileMask);
  AnsiString TransLit(AnsiString InString);
  AnsiString GetUTCDateTimeString(TDateTime DT);
  AnsiString GetHEXString(AnsiString S, AnsiString Delimiter);
  std::string mytrim(std::string);
  std::string fill(std::string src, const std::string filler, int count, SIDE::SIDE align = SIDE::RIGHT, bool isAllowTrim = true);
  std::string fill(int src,          const std::string filler, int count, SIDE::SIDE align = SIDE::RIGHT, bool isAllowTrim = true);

  bool AddTextNode(xmlGuard <_di_IXMLNode> &Parent, AnsiString Name, AnsiString Text);
  bool GetNodeText(xmlGuard <_di_IXMLNode> &Parent, AnsiString Name, AnsiString &Text, bool bNoThrow = false);
  bool isNodeExists(xmlGuard <_di_IXMLNode> &Parent, AnsiString Name);
  void CheckNode(xmlGuard <_di_IXMLNode> &Node, AnsiString Name);
  double my_round(double tt, bool rs = false);
	bool TestArchiveFile(AnsiString SourceFileName, TLogClass* Log = NULL);
  bool DeleteDir(AnsiString DirName, TLogClass *Log = NULL);

  int GetIntFromBuffer(BYTE* fromAddr, int countBytes,  int defaultValue = -1);
  double GetDoubleFromBuffer(BYTE* fromAddr, int countBytes, double defaultValue = -1);
  char* GetTextFromBuffer(BYTE* fromAddr, int countBytes);
  void GetTextFromDWORDHalfBuffer(DWORD*, int, std::string&, bool isNeedTrim = true);
  char* flipAndCodeBytes(const char* str, int pos, int flip, char* buf);

  std::string WCharToString(wchar_t* str);
  std::string DateTimeToString(TDateTime source);

  int StringToInt(std::string str, int defaultValue = -1);
  double StringToDouble(std::string str, double defaultValue = -1);
  bool StringToBool(std::string str, bool defaultValue = false);
  TDateTime StringToDateTime(std::string source);

  int WCharToInt(wchar_t* str, int defaultValue = -1);
  double WCharToDouble(wchar_t* str, double defaultValue = -1);
  bool WCharToBool(wchar_t* str, bool defaultValue = false);

  std::string GetStrFromBuffer(BYTE* fromAddr, int countBytes);
  void activeSleep(int timeout, int step = 1);

//---------------------------------------------------------------------------
#endif


