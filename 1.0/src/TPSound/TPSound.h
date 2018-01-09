//---------------------------------------------------------------------------

#ifndef TPSoundH
#define TPSoundH

#include <system.hpp>
#include <Classes.hpp>
#include "LogClass.h"
#include <string>
#include <map>

struct TAliasInfo
{
  AnsiString FileName;
  std::string AliasName;
  std::auto_ptr<const char> m_pContents;
};

class TPSound
{
  AnsiString SndVolume;
  TLogClass* Log;
  bool InnerLog;
  TList *Aliases;
  void Stop(AnsiString AliasName);
public:
  TPSound(AnsiString _SndVolume, TLogClass* _Log = NULL);
  ~TPSound();
  void Play(AnsiString);
  void StopAll();
  void CloseAll();
  void CashSounds(AnsiString SoundDir);
};

//---------------------------------------------------------------------------
#endif
