#ifndef localizeH
#define localizeH

#include <SysUtils.hpp>
#include <map>
#include <string>

#include "TWConfig.h"
#include "LogClass.h"

typedef std::map<std::string, std::string> TLocaleMap;

class CLocalize
{
private:
    TLogClass* m_log;
protected:
    TLocaleMap m_localization;
    std::string m_locale;
    std::string m_localePath;
public:
    void setLog(TLogClass* log);
    __fastcall CLocalize();
    __fastcall ~CLocalize();
    void __fastcall Load();
    void __fastcall SetLocale(const char* Locale, TWConfig* Cfg);
    const char* operator [](const char* Name);
    std::string __fastcall getLocalePath() const;
    std::string __fastcall getLocale() const;
};

extern CLocalize Localization;

#endif
