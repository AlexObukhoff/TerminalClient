#include <algorith.h>
#include <time.h>
#include <sstream>
#include "globals.h"
#include <boost\format.hpp>
#include <boost\algorithm\string.hpp>
#include "LogClass.h"
#pragma hdrstop
#pragma package(smart_init)

TLogClass::TLogClass(const char *Prefix, const char *_SubFolder)
{
    CriticalSection = new TCriticalSection();
    CriticalSection->Enter();
    alreadyWriteVersion=false;
    Enable = true;
    _Prefix = Prefix;
    Counter = 0;
    LogInit = false;
    LogFile = NULL;
    SubFolder = _SubFolder;
    ErrorCallback = NULL;

    if (!DirectoryExists("logs"))
        CreateDir("logs");

    if (SubFolder != "")
    {
        SubFolder = ".\\logs\\"+SubFolder;
        if (!DirectoryExists(SubFolder.c_str()))
            CreateDir(SubFolder.c_str());
        SubFolder += "\\";
    }
    else
    {
        SubFolder = ".\\logs\\";
    }

    SetLogName();
    //_Write(("Ctor("+_FileName+")").c_str());
    CriticalSection->Leave();
}

void TLogClass::SetLogError(ErrorCallbackFunc callback)
{
    CriticalSection->Enter();
    ErrorCallback = callback;
    CriticalSection->Leave();
}

void TLogClass::SetLogName()
{
    ::time_t tim = time(NULL);
    struct tm* TIME;
    TIME = localtime(&tim);
    std::string strtmp=(boost::format("%04i-%02i-%02i-") % (TIME->tm_year+1900) % (TIME->tm_mon+1) % TIME->tm_mday).str();
    std::string DateLogName = strtmp;

    std::string strtmp1 = SubFolder;
    strtmp1 += DateLogName;
    strtmp1 += _Prefix.c_str();
    strtmp1 += ".log";
    LogName = strtmp1.c_str();

    if (_FileName.c_str() != LogName)
    {
        _FileName = LogName.c_str();
        if (LogFile)
        {
            delete LogFile;
            LogFile = NULL;
            LogInit = false;
        }

        do
        {
            if (Counter != 0)
            {
                Sleep(100);
                std::ostringstream out;
                out << SubFolder.c_str() << DateLogName.c_str() << _Prefix.c_str() << Counter << ".log";
                LogName=out.str();
                _FileName = LogName.c_str();
            }
            if (FileExists(_FileName.c_str()))
            {
                LogFile = new TFileStream(_FileName.c_str(), fmOpenReadWrite | fmShareDenyNone);
            }
            else
            {
                LogFile = new TFileStream(_FileName.c_str(), fmCreate | fmShareDenyNone);
                if (LogFile)
                {
                    delete LogFile;
                    LogFile = NULL;
                    LogInit = false;
                }
                LogFile = new TFileStream(_FileName.c_str(), fmOpenReadWrite | fmShareDenyNone);
            }
            LogInit = true;
            if ((LogFile == NULL)||(LogFile->Handle == 0))
            {
                if (LogFile)
                {
                    delete LogFile;
                    LogFile = NULL;
                }
                LogInit = false;
            }
            Counter++;
        }
        while((LogInit==false)&&(Counter<=5));

        if ((LogFile == NULL)||(LogFile->Handle == 0))
        {
            if (LogFile)
            {
                delete LogFile;
                LogFile = NULL;
            }
            LogInit = false;
            if(ErrorCallback)
                ErrorCallback();
            return;
        }

        _FileName = LogName.c_str();
        Counter = 0;

        if(!alreadyWriteVersion)
        {
            InsertVersionInfo();
            alreadyWriteVersion=true;
        }
    }
}

void TLogClass::_Write(const char* LogString)
{
    if (!Enable || !LogInit)
        return;
    SetLogName();
    LogFile->Seek(0,soFromEnd);
    std::string strtmp=LogString;
    strtmp+="\r\n";
    WriteWithControl(strtmp.c_str(), strtmp.length());
}

void TLogClass::Write(const char* _LogString)
{
    std::string LogString = _LogString;
    if(LogString == "")
        return;
    if(!Enable)
        return;
    try {
        if (!LogInit)
            return;
        if (CriticalSection)
            CriticalSection->Acquire();
        SetLogName();
        ::time_t curtime = time(NULL);
        tm* curtimefmt = localtime(&curtime);
        SYSTEMTIME systime;
        GetSystemTime(&systime);
        std::string mess = (boost::format("%02i.%02i.%i %02i:%02i") % curtimefmt->tm_mday % (curtimefmt->tm_mon + 1) % (curtimefmt->tm_year + 1900) % curtimefmt->tm_hour % curtimefmt->tm_min).str();
        mess += (boost::format(":%02i.%03i ") % curtimefmt->tm_sec % systime.wMilliseconds).str();
//        mess.sprintf("%02i.%02i.%i %02i:%02i:%02i.%03i ",TIME->tm_mday,TIME->tm_mon+1,TIME->tm_year+1900,TIME->tm_hour,TIME->tm_min,TIME->tm_sec, systime.wMilliseconds);
        LogFile->Seek(0, soFromEnd);
        WriteWithControl(mess.c_str(), mess.length());
        DateLineLength = mess.length();
        LogString += "\r\n";
        WriteWithControl(LogString.c_str(), LogString.length());
    }
    __finally
    {
        if(CriticalSection)
            CriticalSection->Release();
    }
}

void TLogClass::Append(const char* _LogString)
{
    std::string LogString = _LogString;
    if(LogString == "")
        return;
    if(!Enable)
        return;
    if(CriticalSection)
        CriticalSection->Acquire();
    try {
        if(!LogInit)
            return;
        SetLogName();
        LogFile->Seek(0,soFromEnd);
        LogFile->Seek(-2,1);
        LogString += "\r\n";
        WriteWithControl(LogString.c_str(), LogString.length());
    }
    __finally
    {
        if (CriticalSection)
            CriticalSection->Release();
    }
}

void TLogClass::InsertEmptyString()
{
    if (!LogInit || !Enable)
        return;
    _Write("");
}

void TLogClass::WriteBuffer(BYTE* Buffer, unsigned int count)
{
    if (Buffer == NULL)
        return;
    if (!Enable || !LogInit)
        return;
    CriticalSection->Enter();
    SetLogName();
    LogFile->Seek(0,soFromEnd);
    std::string mess;

    /*
    for(int i = 0; i < count; i++)
    {
        mess = (boost::format("%02X ") % (unsigned int)Buffer[i]).str();
        WriteWithControl(mess.c_str(), mess.length());
    }
    */
	bool done = false;
	unsigned int indexCounter = 0;

	while(!done)
	{
		std::string msgHex = "";
		std::string msgChr = "";
		for(unsigned int i = indexCounter; i < count && (i < indexCounter + 16); i++)
		{
			msgHex += (boost::format("%02X ") % (short)Buffer[i]).str();
			if(::isalnum(Buffer[i]))
	 			msgChr += (boost::format("%1%") % Buffer[i]).str();
			else
				msgChr += ".";
		}

		indexCounter += 16;
		std::string filler(48 - msgHex.length(), ' ');
		
			
		mess = (boost::format("%1%%2% | %3%\r\n") % msgHex % filler % msgChr).str();
                WriteWithControl(mess.c_str(), mess.length());

		if(indexCounter > count)
			done = true;
	}


    InsertEmptyString();
    CriticalSection->Leave();
}

void TLogClass::WriteLines(const char *Lines)
{
    CriticalSection->Enter();
    _Write(Lines);
    CriticalSection->Leave();
}

void TLogClass::WriteInLine(const char *Lines)
{
    std::string answer = Lines;
    CriticalSection->Enter();
    boost::replace_all(answer,"\n"," ");
    boost::erase_all(answer,"\r");
    _Write(answer.c_str());
    CriticalSection->Leave();
}

void TLogClass::DisableLogging()
{
  Enable = false;
}

void TLogClass::EnableLogging()
{
  Enable = true;
}

void TLogClass::InsertVersionInfo()
{
    _Write(("Version = "+FileVersion).c_str());
}

void TLogClass::WriteWithControl(const char *Buffer, int Count)
{
    if ((Count <= 0)||(Buffer == NULL)) return;
    int result = LogFile->Write(Buffer, Count);
    if (result == 0)
    {
        if(ErrorCallback)
            ErrorCallback();
    }
    else
        LastLineLength = DateLineLength + Count;
}

void TLogClass::DeleteLastLine()
{
    if (!LogInit)
        return;
    CriticalSection->Enter();
    std::string delim = "\r\n";
    LogFile->Seek(0, soFromEnd);
    LogFile->Seek(-LastLineLength, soFromEnd);
    LogFile->Size = LogFile->Position;
    CriticalSection->Leave();
}

TLogClass::~TLogClass()
{
    //Write("Dtor("+_FileName+")");
    InsertEmptyString();
  //если поток не успел завершиться, принудительно его завершаем
  try
  {
    if (CriticalSection)
    CriticalSection->Acquire();
    try
    {
        if (LogFile)
        {
            delete LogFile;
            LogFile = NULL;
            LogInit = false;
        }
    }
    __finally
    {
        if (CriticalSection)
        {
            CriticalSection->Release();
            delete CriticalSection;
            CriticalSection=NULL;
        }
    }
  }
    catch(...)
    {
    }
}

