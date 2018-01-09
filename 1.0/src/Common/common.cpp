//---------------------------------------------------------------------------


#include <algorith.h>
#include <vcl.h>
#include <memory>
#include <vector>
#include <boost\algorithm\string.hpp>
#include <boost\format.hpp>
#include <boost\lexical_cast.hpp>
#pragma hdrstop

#include "common.h"
#include "SevenZipVCL.hpp"
#include "globals.h"

#pragma package(smart_init)

const bool cNoThrow = true;

//---------------------------------------------------------------------------

double ConvertCoinAcceptorValue(unsigned int Value, unsigned int Precession)
{
    std::string strvalue = (boost::format("%016i") % Value).str();
    strvalue.insert(strvalue.length() - Precession, ".");
    return atof(strvalue.c_str());
}

//---------------------------------------------------------------------------
/*
Функция округления.
Принимает 1-й параметр - сумму, 2-й параметр - единицу валюты: рубли (true) или копейки (false), по умолчанию - копейки.
Возвращает округленное значение в той же единице валюты, например:
my_round(1234.5678, true)  -> 1234.57
my_round(1234.5678, false) -> 1234 
*/
double my_round(double tt, bool rs)
{
  if (rs)
    if ((tt*100 - floor(tt*100)) >= 0.5)
      return ceil(tt*100)/100;
    else
      return floor(tt*100)/100;
  else
    if ((tt - floor(tt)) >= 0.5)
      return ceil(tt);
    else
      return floor(tt);

  return 0;
}

//---------------------------------------------------------------------------

AnsiString ShowError(AnsiString Header)
{
LPVOID lpMsgBuf = NULL;
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
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, NULL);
				Temp = "Exception in ShowError: ";
				}
		}
__finally
		{
    if (lpMsgBuf)
		  LocalFree(lpMsgBuf);
		return Temp;
		}
}

//---------------------------------------------------------------------------

std::string StripFileName(std::string FullName)
{
	//char path_buf[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char fext[_MAX_EXT];

//	::GetModuleFileName(NULL,path_buf,sizeof path_buf);
	_splitpath(FullName.c_str(),drive,dir,fname,fext);
//	_makepath (path_buf,0,0,0,0);
	return std::string(fname)+std::string(fext);
}

//---------------------------------------------------------------------------

AnsiString GetFileName(AnsiString FullName)
{
	//char path_buf[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char fext[_MAX_EXT];

	_splitpath(FullName.c_str(),drive,dir,fname,fext);
	return AnsiString(fname);
}

//---------------------------------------------------------------------------

AnsiString GetExtName(AnsiString FullName)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char fext[_MAX_EXT];

	_splitpath(FullName.c_str(),drive,dir,fname,fext);
  return AnsiString(fext).Delete(1, 1);
}

//---------------------------------------------------------------------------

AnsiString GetPath(AnsiString FullName)
{
	//char path_buf[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char fext[_MAX_EXT];

//	::GetModuleFileName(NULL,path_buf,sizeof path_buf);
	_splitpath(FullName.c_str(),drive,dir,fname,fext);
//	_makepath (path_buf,0,0,0,0);
	return AnsiString(drive)+AnsiString(dir);
}

//---------------------------------------------------------------------------

/*AnsiString ChangeChars(AnsiString Source, AnsiString OldChars, AnsiString NewChars)
{
  AnsiString Temp;
  try
  {
    Temp=Source;
    while (Temp.Pos(OldChars)>0)
    {
      Temp=Temp.SubString(0,Temp.Pos(OldChars)-1)+NewChars+Temp.SubString(Temp.Pos(OldChars)+OldChars.Length(),Temp.Length());
    }
  }
  catch (...)
	{
  }
 return Temp;
}*/

//---------------------------------------------------------------------------

AnsiString GetSubString(AnsiString Source, AnsiString StartChars, AnsiString EndChars)
{
  AnsiString Temp;
  try
  {
    if ((Source.Pos(StartChars))&&(Source.Pos(EndChars)))
    {
      Temp = Source.SubString(0,Source.Pos(EndChars)-1);
      Temp = Temp.SubString(Temp.Pos(StartChars)+StartChars.Length(),Temp.Length());
    }
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, NULL);
    }
  return Temp;
}

//---------------------------------------------------------------------------
AnsiString RemoveSubStrings(AnsiString Source, AnsiString StartChars, AnsiString EndChars)
{
  AnsiString Temp;
  try
  {
    //Temp=Source;
    while ((Source.Pos(StartChars))&&(Source.Pos(EndChars)))
    {
      Temp += Source.SubString(0, Source.Pos(StartChars)-1);
      Source = Source.SubString(Source.Pos(StartChars)+StartChars.Length(), Source.Length());
      Source = Source.SubString(Source.Pos(EndChars)+EndChars.Length(), Source.Length());
    }
    Temp += Source;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, NULL);
    }
    return Temp;
}

//---------------------------------------------------------------------------

AnsiString ChangeChars(AnsiString Source, AnsiString OldChars, AnsiString NewChars)
{
    AnsiString Temp;
    try {
        while(Source.Pos(OldChars)) {
            Temp += Source.SubString(0, Source.Pos(OldChars) - 1) + NewChars;
            Source = Source.SubString(Source.Pos(OldChars) + OldChars.Length(), Source.Length());
        }
        Temp += Source;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, NULL);
    }
    return Temp;
}

//---------------------------------------------------------------------------


double GetDouble(AnsiString Source)
{
    if(Source=="")
        return 0;
    try {
	    try	{
		    return ChangeChars(Source.c_str(), ".", ",").ToDouble();
		} catch (...) {
    		return ChangeChars(Source.c_str(), ",", ".").ToDouble();
		}
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, NULL);
        return 0;
    }
}

//---------------------------------------------------------------------------

bool IsDouble(AnsiString Source)
{
    try {
    	try {
    		ChangeChars(Source, ".", ",").ToDouble();
	    	return true;
		} catch (...) {
    		ChangeChars(Source, ",", ".").ToDouble();
	    	return true;
		}
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, NULL);
    	return false;
    }
}

//---------------------------------------------------------------------------

TDateTime GetDateTime(AnsiString Source)
{
try
  {
  return TDateTime(Source);
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, NULL);
        return 0;
    }
}

//---------------------------------------------------------------------------

int GetInt(AnsiString Source)
{
try
  {
  if ((Source.LowerCase()=="no")||(Source==""))
    return 0;
    else
    if (Source.LowerCase()=="yes")
      return 1;
      else
      return Source.ToInt();
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, NULL);
        return 0;
    }
}

//---------------------------------------------------------------------------

long GetLong(AnsiString Source)
{
try
  {
  if ((Source.LowerCase()=="no")||(Source==""))
    return 0;
    else
    if (Source.LowerCase()=="yes")
      return 1;
      else
      return StrToInt64(Source);
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, NULL);
        return 0;
    }
}

std::string DateTimeToString(TDateTime source)
{
    try
    {
        return DateTimeToStr(source).c_str();
    }
    catch(...)
    {
        //Log->Write((boost::format("error value: {%1%}") % resStr).str().c_str());
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, NULL);
    }
}

//---------------------------------------------------------------------------

std::string TruncateLocation(AnsiString _fileName)
{
AnsiString Temp;
try
{
  AnsiString Temp = _fileName;

  while ((Temp.Pos("/")!=0)&&(Temp.Pos("/")<Temp.Pos(".htm")))
    Temp.Delete(1,Temp.Pos("/"));
  return std::string(Temp.c_str());
}
catch(...)
{
  ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, NULL);
  return std::string(Temp.c_str());
}
}

bool StoreStringToFile(AnsiString FileName, AnsiString Content, TLogClass* Log)
{
  bool bRes = false;
  try
  {
    DeleteFile(FileName);
    std::auto_ptr<TFileStream> OutFile(new TFileStream(FileName, fmCreate));
    OutFile->Size = 0;
    OutFile->Seek(0, soFromBeginning);
    OutFile->Write(Content.c_str(), Content.Length());
    OutFile.reset(NULL);
    bRes=true;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        if (Log) Log->Write((boost::format("Exception occured while saving %1% file") % FileName.c_str()).str().c_str());
    }
  return bRes;
}

bool GetFileData(AnsiString FileName, AnsiString &Content, TLogClass* Log)
{
  try
  {
    std::auto_ptr<TFileStream> InFile(new TFileStream(FileName, fmOpenReadWrite|fmShareExclusive));
    std::vector<char> Buffer( InFile->Size+1, 0 );
    InFile->Seek(0, soFromBeginning);
    InFile->Read(&*Buffer.begin(), InFile->Size);
    Content = AnsiString(&*Buffer.begin(), InFile->Size);
    InFile.reset(NULL);
    return true;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        if (Log) Log->Write((boost::format("Exception occured while loading %1% file") % FileName.c_str()).str().c_str());
        return false;
  }
}

int FileCount(AnsiString FileMask)
{
int FCount = 0;
try
  {
  TSearchRec sr;
  int iAttributes = 0;
  if (FindFirst(FileMask, iAttributes, sr) == 0)
    {
    do
      {
      FCount++;
      } while (FindNext(sr) == 0);
    FindClose(sr);
    }
  return FCount;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, NULL);
  return -1;
  }
}

AnsiString TransLit(AnsiString InString)
{
static char *s1[] = {
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
" ", "!", "\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/",
"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?",
"@", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
"P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "[", "\\", "]", "^", "_",
"`", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
"p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "{", "|", "}", "~", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "JO", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "jo", "", "", "", "", "", "", "",
"A", "B", "V", "G", "D", "E", "ZH", "Z", "I", "JJ", "K", "L", "M", "N", "O", "P",
"R", "S", "T", "U", "F", "KH", "C", "CH", "SH", "SHH", "", "Y", "'", "JE", "JU", "JA",
"a", "b", "v", "g", "d", "e", "zh", "z", "i", "jj", "k", "l", "m", "n", "o", "p",
"r", "s", "t", "u", "f", "kh", "c", "ch", "sh", "shh", "", "y", "'", "je", "ju", "ja"
};

AnsiString Res;
try
  {
  for(int i=0;i<InString.Length();i++)
    Res += s1[(unsigned char)InString.c_str()[i]];
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, NULL);
    }
    return Res;
}


AnsiString GetUTCDateTimeString(TDateTime DT)
{
TIME_ZONE_INFORMATION TZInfo;
int TZRes = GetTimeZoneInformation(&TZInfo);
long Bias;
switch (TZRes)
  {
  case TIME_ZONE_ID_UNKNOWN:   //The operating system cannot determine the current time zone. This is usually because a previous call to the SetTimeZoneInformation function supplied only the bias (and no transition dates).
    Bias = TZInfo.Bias;
    break;
  case TIME_ZONE_ID_STANDARD:  //The operating system is operating in the range covered by the StandardDate member of the structure pointed to by the lpTimeZoneInformation parameter.
    Bias = TZInfo.Bias + TZInfo.StandardBias;
    break;
  case TIME_ZONE_ID_DAYLIGHT:
    Bias = TZInfo.Bias + TZInfo.DaylightBias;
    break;
  }
//DT = DT+float(Bias)/24/60;
enum TStringFloatFormat {sffGeneral, sffExponent, sffFixed, sffNumber, sffCurrency };
AnsiString GMTstring = AnsiString(" GMT")+(Bias<=0 ? "+" : "-")+AnsiString(float((abs(Bias)))/60);
//AnsiString UTC = AnsiString("GMT")+(Bias<=0 ? "+" : "-")+AnsiString::FloatToStrF(float(abs(Bias)/60),sffFixed,1,1);
return DT.FormatString("dd.mm.yyyy hh:nn:ss") + GMTstring;
}

bool AddTextNode(xmlGuard <_di_IXMLNode> &Parent, AnsiString Name, AnsiString Text)
{
  xmlGuard<_di_IXMLNode>NewNode(Parent->AddChild(Name));
  if (!NewNode.Assigned())
    throw Exception((AnsiString)"Error creating <"+Name+"> node!");

  NewNode->Text = Text;

  return true;
}

bool GetNodeText(xmlGuard <_di_IXMLNode> &Parent, AnsiString Name, AnsiString &Text, bool bNoThrow)
{
  Text = "";

  xmlGuard <_di_IXMLNodeList> ParentNDL (Parent->GetChildNodes());
  if (!ParentNDL.Assigned())
  {
    //if (bNoThrow)
      return false;
    //else
    //  throw Exception((AnsiString)"Error getting parent nodelist for <"+Name+"> node!");
  }


  xmlGuard <_di_IXMLNode> Node (ParentNDL->FindNode(Name));
  if (!Node.Assigned())
  {
    //if (bNoThrow)
      return false;
    //else
    //  throw Exception((AnsiString)"Node <"+Name+"> not found!");
  }

  if (Node->IsTextElement)
  {
    Text = Node->NodeValue.VOleStr;
  }

  return true;
}

bool isNodeExists(xmlGuard <_di_IXMLNode> &Parent, AnsiString Name)
{
  xmlGuard <_di_IXMLNodeList> ParentNDL (Parent->GetChildNodes());
  if (!ParentNDL.Assigned())
    return false;

  xmlGuard <_di_IXMLNode> Node (ParentNDL->FindNode(Name));
  if (!Node.Assigned())
    return false;
    
  return true;
}

void CheckNode(xmlGuard <_di_IXMLNode> &Node, AnsiString Name)
{
  if (!Node.Assigned())
    throw Exception((AnsiString)"Node <"+Name+"> not found!");
}

AnsiString GetHEXString(AnsiString S, AnsiString Delimiter)
{
AnsiString AHex;
for (int i = 1; i <= S.Length(); i++){
    AHex += IntToHex((unsigned char)S[i], 2) + Delimiter;
		}
return AHex;
}

bool TestArchiveFile(AnsiString SourceFileName, TLogClass *Log)
{
    try
    {
        std::auto_ptr <TSevenZip> SZ ( new TSevenZip(NULL) );
        SZ->SZFileName=WideString(SourceFileName);
        SZ->Files->Clear();
        SZ->Extract(true);
        if (SZ->ErrCode==0)
        {
            return true;
        }
        else
        {
            if (Log) Log->Write((boost::format("Testing archive file %1% error# %2%") % SourceFileName.c_str() % SZ->ErrCode).str().c_str());
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return false;
}

bool DeleteDir(AnsiString DirName, TLogClass *Log)
{
  if (Log) Log->Write((boost::format("Deleting %1%") % DirName.c_str()).str().c_str());
  char From[MAX_PATH];
  ZeroMemory(From, sizeof(From));
  strcat(From,DirName.c_str());
  strcat(From,"\0\0");
  SHFILEOPSTRUCT op;
  ZeroMemory(&op, sizeof(op));
  op.hwnd = NULL;
  op.wFunc = FO_DELETE;
  op.pFrom = From;
  op.fFlags = FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_NOERRORUI|FOF_SILENT;
  if (!SHFileOperation(&op))
  {
    if (Log) Log->Append(" OK.");
    return true;
  }
  else
  {
    if (Log) Log->Append(ShowError(" Error! ").c_str());
    return false;
  }
}

//---------------------------------------------------------------------------

std::string mytrim(std::string source)
{
  if(!source.empty())
  {
    int left = source.find_first_not_of(" ");
    int right = source.find_last_not_of(" ");
    return source.substr(left, right - left + 1);
  }
  else
    return "";
}

//---------------------------------------------------------------------------

char* GetTextFromBuffer(BYTE* fromAddr, int countBytes)
{
    char* result;
    memset(result, 0, countBytes + 1);
    memcpy(result, fromAddr, countBytes);
    return result;    
}

//---------------------------------------------------------------------------

int GetIntFromBuffer(BYTE* fromAddr, int countBytes,  int defaultValue)
{
    return StringToInt(GetStrFromBuffer(fromAddr, countBytes), defaultValue);
}

//---------------------------------------------------------------------------

double GetDoubleFromBuffer(BYTE* fromAddr, int countBytes, double defaultValue)
{
    return StringToDouble(GetStrFromBuffer(fromAddr, countBytes), defaultValue);
}

//---------------------------------------------------------------------------

std::string GetStrFromBuffer(BYTE* fromAddr, int countBytes)
{
    return std::string(GetTextFromBuffer(fromAddr, countBytes));
}

//---------------------------------------------------------------------------

void activeSleep(int timeout, int step)
{
    if (step < 1)
        step = 1; 
    for(int i = 0; i < int(timeout/step); i++)
    {
        Application->ProcessMessages();
        Sleep(step);
    }
}

//---------------------------------------------------------------------------

std::string getUpdaterFullPathName(std::string defaultValue)
{
    return getUpdaterDirectory() + "\\updater.exe";
}

//---------------------------------------------------------------------------

std::string getProjectFullPathName(std::string defaultValue)
{
    std::string currentDirectory = GetCurrentDir().c_str();
    currentDirectory = currentDirectory.substr(0, currentDirectory.find_last_of("\\"));
    /*
    #ifdef __CONN__
    currentDirectory = currentDirectory.substr(0, currentDirectory.find_last_of("\\"));
    #endif
    */
    return currentDirectory;
}

//---------------------------------------------------------------------------

std::string getWCDirectory(std::string defaultValue)
{
    std::string result = defaultValue;

    #ifdef __WC__
        result = GetCurrentDir().c_str();
    #else
        result = getRegistryTextValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\CyberPlat\\WebClient", "InstallPath");
        if(result.empty())
        {
            result = defaultValue;
        }
    #endif
    return result;
}

//---------------------------------------------------------------------------

std::string getUpdaterDirectory(std::string defaultValue)
{
    return getProjectFullPathName() + "\\WebUpdate";
}

//---------------------------------------------------------------------------

HKEY getRegistryKey(std::string pathStr)
{
    HKEY result = NULL;
    std::string key = pathStr;
    
    key = mytrim(key.substr(0, key.find_first_of("\\")));
    //boost::to_upper(key); //- не компилится в Borland-е

         if(key == "HKEY_LOCAL_MACHINE")
        result = HKEY_LOCAL_MACHINE;
    else if(key == "HKEY_CLASSES_ROOT")
        result = HKEY_LOCAL_MACHINE;
    else if(key == "HKEY_CURRENT_CONFIG")
        result = HKEY_LOCAL_MACHINE;
    else if(key == "HKEY_CURRENT_USER")
        result = HKEY_LOCAL_MACHINE;
    else if(key == "HKEY_USERS")
        result = HKEY_LOCAL_MACHINE;
    else if(key == "HKEY_DYN_DATA")
        result = HKEY_LOCAL_MACHINE;

    return result;
}

//---------------------------------------------------------------------------

std::string getRegistryTextValue(char* pathStr, char* paramStr, char* defaultStr)
{
    std::string result = defaultStr;
    std::string fullPath = mytrim(pathStr);
    int root_pos = fullPath.find_first_of("\\");
    std::string localPath = fullPath.substr(root_pos + 1, fullPath.length() - root_pos - 1);

    HKEY h;
    HKEY rootKey = getRegistryKey(fullPath);

    if(!rootKey)
        return result;
    if(RegOpenKeyEx(rootKey, localPath.c_str(), 0, KEY_READ, &h) != ERROR_SUCCESS)
        return result;

    DWORD dataType, charCount = 1500;
    char* result_buffer = new char[charCount];
    if(RegQueryValueEx(h, paramStr, NULL, &dataType, result_buffer, &charCount) != ERROR_SUCCESS)
        return result;
    result = result_buffer;
    delete [] result_buffer;

    return result;
}

//---------------------------------------------------------------------------

int WCharToInt(wchar_t* str, int defaultValue)
{
    return StringToInt(WCharToString(str), defaultValue);
}

//---------------------------------------------------------------------------

double WCharToDouble(wchar_t* str, double defaultValue)
{
    return StringToInt(WCharToString(str), defaultValue);
}

//---------------------------------------------------------------------------

bool WCharToBool(wchar_t* str, bool defaultValue)
{
    return StringToInt(WCharToString(str), defaultValue);
}

//---------------------------------------------------------------------------

std::string WCharToString(wchar_t* str)
{
    return std::string(WideCharToString(str).c_str());
}

//---------------------------------------------------------------------------

int StringToInt(std::string str, int defaultValue)
{
    return my_round(StringToDouble(str, defaultValue));
}

//---------------------------------------------------------------------------

double StringToDouble(std::string str, double defaultValue)
{
    double result = defaultValue;

    try
    {
        boost::replace_all(str, ",", ".");
        result = boost::lexical_cast<double>(mytrim(str));
    }
    catch(...)
    {
        //Log->Write((boost::format("error value: {%1%}") % str).str().c_str());
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, NULL);
    }
    return result;
}

//---------------------------------------------------------------------------

bool StringToBool(std::string str, bool defaultValue)
{
    bool result = defaultValue;
    std::string resStr = mytrim(str);

    try
    {
        result = static_cast<bool>(boost::lexical_cast<double>(mytrim(str)));
    }
    catch(...)
    {
        if (resStr == "false")
            result = false;
        else if (resStr == "true")
            result = true;
        else
        {
            //Log->Write((boost::format("error value: {%1%}") % resStr).str().c_str());
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, NULL);
        }
    }
    return result;
}

TDateTime StringToDateTime(std::string source)
{
    try
    {
        return StrToDate(source.c_str());
    }
    catch(...)
    {
        //Log->Write((boost::format("error value: {%1%}") % resStr).str().c_str());
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, NULL);
    }
}

//---------------------------------------------------------------------------

std::string fill(std::string src, const std::string filler, int count, SIDE::SIDE align, bool isAllowTrim)
{
    if(filler.length() <= 1)
    {
        if(count == src.length())
            return src;
        if(count > src.length())
        {
            std::string alignment = "";
            switch(align)
            {
                case SIDE::LEFT:
                    alignment = "-";
                break;
                case SIDE::CENTER:
                    alignment = "=";
                break;
                case SIDE::RIGHT:
                    alignment = " ";
                break;
                default:
                    alignment = " ";
                break;
            }
            std::string format = (boost::format("%%%1%%2%d") % alignment % count).str();
            std::string result = (boost::format(format) % src).str();
            boost::replace_all(result, " ", filler);
            return result;
        }
        else
        {
            if(isAllowTrim)
            {
                switch(align)
                {
                    case SIDE::LEFT:
                        return src.substr(0, count);
                    break;
                    case SIDE::CENTER:
                        return src.substr((src.length() - count)/2, count);
                    break;
                    case SIDE::RIGHT:
                        return src.substr(count + 1);
                    break;
                }
            }
            else
                return src;
        }
    }
    else
        return src;
}

//---------------------------------------------------------------------------

std::string fill(int src, const std::string filler, int count, SIDE::SIDE align, bool isAllowTrim)
{
    std::string strtmp = boost::lexical_cast<std::string>(src);
    return fill(strtmp, filler, count, align, isAllowTrim);
}

//-----------------------------------------------------------------------------------

char * flipAndCodeBytes (const char * str,
						 int pos,
						 int flip,
						 char * buf)
{
	int i;
	int j = 0;
	int k = 0;

	buf [0] = '\0';
	if (pos <= 0)
		return buf;

	if ( ! j)
	{
		char p = 0;

		// First try to gather all characters representing hex digits only.
		j = 1;
		k = 0;
		buf[k] = 0;
		for (i = pos; j && str[i] != '\0'; ++i)
		{
			int c = tolower(str[i]);

			if (isspace(c))
				c = '0';

			++p;
			buf[k] <<= 4;

			if (c >= '0' && c <= '9')
				buf[k] |= (unsigned char) (c - '0');
			else if (c >= 'a' && c <= 'f')
				buf[k] |= (unsigned char) (c - 'a' + 10);
			else
			{
				j = 0;
				break;
			}

			if (p == 2)
			{
				if (buf[k] != '\0' && ! isprint(buf[k]))
				{
					j = 0;
					break;
				}
				++k;
				p = 0;
				buf[k] = 0;
			}

		}
	}

	if ( ! j)
	{
		// There are non-digit characters, gather them as is.
		j = 1;
		k = 0;
		for (i = pos; j && str[i] != '\0'; ++i)
		{
			char c = str[i];

			if ( ! isprint(c))
			{
				j = 0;
				break;
			}

			buf[k++] = c;
		}
	}

	if ( ! j)
	{
		// The characters are not there or are not printable.
		k = 0;
	}

	buf[k] = '\0';

	if (flip)
		// Flip adjacent characters
		for (j = 0; j < k; j += 2)
		{
			char t = buf[j];
			buf[j] = buf[j + 1];
			buf[j + 1] = t;
		}

		// Trim any beginning and end space
		i = j = -1;
		for (k = 0; buf[k] != '\0'; ++k)
		{
			if (! isspace(buf[k]))
			{
				if (i < 0)
					i = k;
				j = k;
			}
		}

		if ((i >= 0) && (j >= 0))
		{
			for (k = i; (k <= j) && (buf[k] != '\0'); ++k)
				buf[k - i] = buf[k];
			buf[k - i] = '\0';
		}

		return buf;
}

//-----------------------------------------------------------------------------------

void GetTextFromDWORDHalfBuffer(DWORD* fromAddr, int countBytes, std::string& result, bool isNeedTrim)
{
    char* buffer = new char[countBytes*2 + 1];
    memset(buffer, 0, countBytes*2 + 1);

	for(int i = 0; i < countBytes; i++)
    {
        buffer[i*2]     = (BYTE)(fromAddr[i] >> 8);
        buffer[i*2 + 1] = (BYTE)(fromAddr[i]);
    }

    result = buffer;
    if (isNeedTrim)
        boost::trim(result);
    delete [] buffer;
}
