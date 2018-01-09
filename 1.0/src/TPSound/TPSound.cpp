//---------------------------------------------------------------------------


#include <system.hpp>

#pragma hdrstop

#include "TPSound.h"
#include "common.h"
#include "globals.h"
#include "boost/format.hpp"
#include <io.h>


#pragma package(smart_init)

//---------------------------------------------------------------------------

TPSound::TPSound(AnsiString _SndVolume, TLogClass *_Log)
{
    Log = _Log ? _Log : new TLogClass("Sound");
    InnerLog = !_Log;
    SndVolume = _SndVolume;
    Aliases = new TList;
}

//---------------------------------------------------------------------------

TPSound::~TPSound()
{
    try
    {
        CloseAll();
        delete Aliases;
        if(InnerLog)
            delete Log;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TPSound::Play(AnsiString FileName)
{
    try
    {
        for(int i = 0; i < Aliases->Count; i++)
        {
            TAliasInfo *Alias = (TAliasInfo*)Aliases->Items[i];
            if(Alias->FileName == FileName)
            {
                int StopRes = mciSendString(("stop " + Alias->AliasName).c_str(), 0, 0, NULL);
                if(StopRes != 0)
                    StopRes = ::PlaySound(0, 0, SND_PURGE) ? 0 : 1;
                int PlayRes = mciSendString(("play " + Alias->AliasName + " from 0").c_str(), 0, 0, NULL);
                if(PlayRes != 0)
                    PlayRes = ::PlaySound(Alias->m_pContents.get(), 0, SND_MEMORY | SND_ASYNC | SND_NODEFAULT) ? 0 : 1;
                if(StopRes != 0)
                    Log->Write((boost::format("Error stopping sound %1%, alias %2%, stopRes = %3%") % FileName.c_str() % Alias->AliasName.c_str() % StopRes).str().c_str());
                if(PlayRes != 0)
                    Log->Write((boost::format("Error playing sound %1%, alias %2%, PlayRes %3%") % FileName.c_str() % Alias->AliasName.c_str() % PlayRes).str().c_str());
                return;
            }
        }
        Log->Write((boost::format("Couldn't find alias for the %1% file") % FileName.c_str()).str().c_str());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TPSound::Stop(AnsiString AliasName)
{
    try
    {
        int StopRes = mciSendString(("stop " + AliasName).c_str(), 0, 0, NULL);
        if(StopRes != 0)
            StopRes = ::PlaySound(0, 0, SND_PURGE) ? 0 : 1;
        if(StopRes != 0)
            Log->Write((boost::format("Error stopping alias %1%, stopRes = %2%") % AliasName.c_str() % StopRes).str().c_str());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TPSound::StopAll()
{
    try
    {
        for(int i = 0; i < Aliases->Count; i++)
        {
            TAliasInfo *Alias = (TAliasInfo*)Aliases->Items[i];
            if(mciSendString(("stop " + Alias->AliasName).c_str(), 0, 0, NULL) != 0)
                ::PlaySound(0, 0, SND_PURGE);
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TPSound::CloseAll()
{
    try
    {
        for(int i = 0; i < Aliases->Count; i++)
        {
            TAliasInfo *Alias = (TAliasInfo*)Aliases->Items[i];
            if(mciSendString(("close " + Alias->AliasName).c_str(), 0, 0, NULL) != 0)
                ::PlaySound(0, 0, SND_PURGE);
            delete Alias;
        }
        Aliases->Clear();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TPSound::CashSounds(AnsiString SoundDir)
{
    try
    {
        TSearchRec sr;
        if(FindFirst(SoundDir + "\\*.*", 0, sr) == 0)
        {
            do
            {
                TAliasInfo *Alias = new TAliasInfo;
                Alias->FileName = sr.Name;
                Alias->AliasName = "SND#" + StripFileName(sr.Name.c_str());
                int OpenRes = mciSendString(("open \"" + SoundDir + "\\" + sr.Name + "\" type MPEGVideo alias " + std::string(Alias->AliasName.c_str()).c_str()).c_str(), 0, 0, NULL);
                if(OpenRes != 0)
                {
                    int soundhandle = ::FileOpen(SoundDir + "\\" + sr.Name, fmOpenRead);
                    if(soundhandle >= 0)
                    {
                        int soundsize = ::FileSeek(soundhandle, 0, 2);
                        if(soundsize > 0)
                        {
                            ::FileSeek(soundhandle, 0, 0);
                            Alias->m_pContents.reset(new const char[soundsize]);
                            ::FileRead(soundhandle, (void *)Alias->m_pContents.get(), soundsize);
                            OpenRes = 0;
                        }
                        ::FileClose(soundhandle);
                    }
                }
                if(OpenRes != 0)
                    Log->Write((boost::format("File %1% load error %2%") % StripFileName(sr.Name.c_str()).c_str() % OpenRes).str().c_str());
                Aliases->Add(Alias);
            } while(FindNext(sr) == 0);
            FindClose(sr);
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}


