#ifndef ChequeH
#define ChequeH
//---------------------------------------------------------------------------
#include <Windows.h> 
#include <string>
#include <Classes.hpp>

class CCheque
{
private:
	HANDLE              m_hFile;
    const std::string   sFileName;
    TStringList*        m_StringList_1;
    TStringList*        m_StringList_2;
    std::string         sFileBuffer;
    bool do_1_tact;

	HANDLE	Open();
	bool	Close();

    bool 	Read(std::string &Str);
	void    WriteToBuffer(std::string KeyWord, std::string StrValue);
	bool    WriteToFile();

public:
    CCheque(const char* FileName, TStringList* StringList_1, TStringList* StringList_2 = NULL);
    ~CCheque();

    AnsiString Replace(bool bWriteToFile = false);
};

#endif
 