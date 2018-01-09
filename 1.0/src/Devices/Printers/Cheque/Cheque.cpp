#include "Cheque.h"
#include "globals.h"

using namespace std;

CCheque::CCheque(const char* FileName, TStringList* StringList_1, TStringList* StringList_2) : m_hFile(0), sFileName(FileName), m_StringList_1(StringList_1), m_StringList_2(StringList_2), do_1_tact(false)
{
    Open();
}
CCheque::~CCheque()
{
    Close();
}

AnsiString CCheque::Replace(bool bWriteToFile)
{
    try
    {
        if(m_hFile != INVALID_HANDLE_VALUE)
        {
            sFileBuffer.assign("");
            Read(sFileBuffer);

            int pos_1 = sFileBuffer.find("%%");
            if(m_StringList_2 && pos_1 != string::npos)
            {
                string Key_1(""), Key_2("");

                int pos_2 = sFileBuffer.find("%%", pos_1+2);
                if(pos_2 != string::npos)
                {
                    int iCheckError_1 = sFileBuffer.find("%", pos_1+2);
                    if(iCheckError_1 != pos_2)
                    {
                        int iCheckError_2 = sFileBuffer.rfind("%", pos_2-1);
                        if(iCheckError_2 != pos_1+1)
                        {
                            int iCheckError = sFileBuffer.find("%", iCheckError_1+1);
                            if(iCheckError < iCheckError_2)
                            {
                                throw Exception("Error! Invalid pattern!");
                            }
                        }
                    }
//---------------------------------- Prefix
                    std::string sPrefix;
                    int pos_prefix = sFileBuffer.rfind("\n", pos_1);
                    sPrefix.assign(sFileBuffer, pos_prefix + 1, pos_1 - pos_prefix - 1);
//----------------------------------
                    string KeyBody("");
                    KeyBody.assign( sFileBuffer, pos_1, (pos_2-pos_1+2) );

                    int Key_1_begin = KeyBody.find("%%"), Key_1_end = 0;
                    if(Key_1_begin != string::npos)
                    {
                        Key_1_end = KeyBody.find("%", Key_1_begin+2);
                        if(Key_1_end != string::npos)   Key_1.assign(KeyBody, Key_1_begin+1, (Key_1_end-Key_1_begin-1));
                    }

                    int Key_2_end = KeyBody.find("%%", Key_1_end+1), Key_2_begin = 0;
                    if(Key_2_end != string::npos)
                    {
                        Key_2_begin = KeyBody.rfind("%", Key_2_end-1);
                        if(Key_2_end != string::npos)   Key_2.assign(KeyBody, Key_2_begin+1, (Key_2_end-Key_2_begin));
                    }

                    for(int i= pos_1 + KeyBody.size(), n=1;  true;   n++, i += KeyBody.size())
                    {
                        if(n < m_StringList_2->Count)
                        {
                            sFileBuffer.insert(i++, "\r");
                            sFileBuffer.insert(i++, "\n");
//---------------------------------- Prefix
                            sFileBuffer.insert(i, sPrefix);
                            i += int(sPrefix.size());
//----------------------------------
                            sFileBuffer.insert(i, KeyBody);
                        }
                        else break;
                    }
                }
                else    throw Exception("Error! Invalid pattern!");

                do_1_tact = true;
                for(int i = 0; i < m_StringList_2->Count; i++)
                {
                    WriteToBuffer( Key_1.c_str(), m_StringList_2->Names[i].c_str() );
                    WriteToBuffer( Key_2.c_str(), m_StringList_2->Values[m_StringList_2->Names[i]].c_str() );
                }
            }

            do_1_tact = false;
            for(int i = 0; i < m_StringList_1->Count; i++)
            {
                WriteToBuffer( m_StringList_1->Names[i].c_str(), m_StringList_1->Values[m_StringList_1->Names[i]].c_str());
            }

            WriteToBuffer( "%", "%");

            WriteToBuffer( "", "");

            if(bWriteToFile) WriteToFile();
            return (AnsiString)sFileBuffer.c_str();
        }
        throw Exception("Error! File of the pattern can`t be read!");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, NULL);
        Close();
        throw;
    }
}

//-------------------------------------------------
HANDLE	CCheque::Open()
{
	if(m_hFile == 0)
	{
		m_hFile = CreateFile(sFileName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		return m_hFile;
	}
	return 0;
}
bool	CCheque::Close()
{
	if(m_hFile != 0)
	{
		if(CloseHandle(m_hFile) == S_OK) return true;
	}
	return false;
}
//-------------------------------------------------
bool CCheque::Read(std::string &Str)
{
	bool bRez = false;
	DWORD nRead;

	DWORD nSize = GetFileSize(m_hFile, 0);
	char* pBuf = new char[nSize + 1];

	if(ReadFile(m_hFile, LPVOID(pBuf), DWORD(nSize), &nRead, NULL) != 0)
	{
		pBuf[nSize] = '\0';
		Str = pBuf;
		bRez = true;
	}
	delete [] pBuf;
	return bRez;
}
void CCheque::WriteToBuffer(std::string KeyWord, std::string StrValue)
{
    while(true)
    {
        if(KeyWord == "%" && StrValue == "%")
        {
            while(true)
            {
                unsigned int ClearPosition_begin = 0, ClearPosition_end = 0;

                ClearPosition_begin = sFileBuffer.find("%%");
                if(ClearPosition_begin != string::npos)
                {
                    ClearPosition_end = sFileBuffer.find("%%", ClearPosition_begin+2);
                    if(ClearPosition_end != string::npos)
                    {
                        sFileBuffer.erase(ClearPosition_begin, ClearPosition_end - ClearPosition_begin + 2);
                    }
                }
                return ;
            }
        }

        unsigned int KeyPosition = 0;
        string Str = StrValue, _KeyWord = "%" + KeyWord, _KeyWord_;

        KeyPosition = sFileBuffer.find(_KeyWord);
        if(KeyPosition != string::npos)
        {
            for(int n=KeyPosition+_KeyWord.size(); sFileBuffer[n] != '%'; n++)  _KeyWord.push_back(sFileBuffer[n]);

            int pos_format = _KeyWord.find(":");
            if(pos_format != string::npos)
            {
                string sDigit("");
                char Format_Simbol = _KeyWord[_KeyWord.size()-1];
                if(Format_Simbol == '%')    Format_Simbol = _KeyWord[_KeyWord.size()-2];

                if(isdigit(Format_Simbol)) sDigit.append( _KeyWord, pos_format+1, (_KeyWord.size()-pos_format-1) );
                else sDigit.append( _KeyWord, pos_format+1, (_KeyWord.size()-pos_format-2) );
                int iDigit = atoi(sDigit.c_str());

                int delta = iDigit - Str.size();
                if( !isdigit(Format_Simbol) && ( (Format_Simbol == 'R') || (Format_Simbol == 'r') ) )
                {
                    if(delta > 0)   for(int i=0; i<delta; i++)  Str.insert(i, " ");
                }
                else
                {
                    if(delta > 0)   for(int i=0; i<delta; i++)                              Str.push_back(' ');
                    else            if( (Format_Simbol == 'S') || (Format_Simbol == 's') )  Str.erase(iDigit, abs(delta));
                }
            }

            _KeyWord_ = _KeyWord + "%";
            while(true)
            {
                KeyPosition = sFileBuffer.find(_KeyWord_);
                if(KeyPosition != string::npos) sFileBuffer.replace(KeyPosition, _KeyWord_.size(), Str);
                else break;
                if(do_1_tact)   return;
            }
        }
        else break;
    }
}
bool CCheque::WriteToFile()
{
    DWORD nWrite;

	if(m_hFile != 0)	CloseHandle(m_hFile);
	m_hFile = CreateFile(sFileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(m_hFile != INVALID_HANDLE_VALUE)
    {
        if(WriteFile(m_hFile, (LPCVOID)sFileBuffer.c_str(), DWORD(sFileBuffer.size()), &nWrite, NULL) != 0) return true;
    }
	return false;
}

