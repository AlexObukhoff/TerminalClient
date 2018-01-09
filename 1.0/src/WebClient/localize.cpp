#pragma hdrstop
#include "localize.h"
#include <IniFiles.hpp>
#include <boost\algorithm\string.hpp>
#include <boost\format.hpp>
#include <boost\regex.hpp>
#pragma package(smart_init)

__fastcall CLocalize::CLocalize()
{
    m_log = NULL;
}

__fastcall CLocalize::~CLocalize()
{
}

void __fastcall CLocalize::SetLocale(const char* locale, TWConfig* Cfg)
{
    m_locale = locale;
    m_localePath = Cfg->Dirs.InterfaceDir + "/locale/" + m_locale + "/";
    int iFileHandle;

    iFileHandle = FileOpen((Cfg->Dirs.InterfaceDir + "/main.html").c_str(), fmOpenReadWrite);
    if(iFileHandle != -1)
    {
      int iFileLength = FileSeek(iFileHandle, 0, 2);
      FileSeek(iFileHandle, 0, 0);
      char* pszBuffer = new char[iFileLength+1];
      int iBytesRead = FileRead(iFileHandle, pszBuffer, iFileLength);
      pszBuffer[iFileLength] = '\0';
      std::string fileContent = pszBuffer;

      std::string localeRegex = "(.*)URL=locale/([^/]*)/main.html\\?locale=([^\"]*)\"(.*)";
      boost::regex localeExpr(localeRegex);
      if (boost::regex_match(fileContent, localeExpr))
      {
        boost::cmatch what_locale;
        boost::regex_search(fileContent, what_locale, localeExpr);

        std::string old_locale_path = (boost::format("URL=locale/%1%/main.html?locale=") % what_locale[2]).str();
        std::string new_locale_path = (boost::format("URL=locale/%1%/main.html?locale=") % locale).str();

        std::string old_locale_value = (boost::format("/main.html?locale=%1%\"") % what_locale[3]).str();
        std::string new_locale_value = (boost::format("/main.html?locale=%1%\"") % locale).str();

        boost::replace_all(fileContent, old_locale_path,  new_locale_path);
        boost::replace_all(fileContent, old_locale_value, new_locale_value);

        FileSeek(iFileHandle, 0, 0);
        FileWrite(iFileHandle, fileContent.c_str(), fileContent.length());
      }
      else
        m_log->Write(("Locale string not found, locale = " + m_locale + " !!!").c_str());
      delete [] pszBuffer;
    }
    else
      m_log->Write(("Error on opening " + Cfg->Dirs.InterfaceDir + "/main.html, locale = " + m_locale + " !!!").c_str());
    FileClose(iFileHandle);
}

void __fastcall CLocalize::Load()
{
    const char* LocaleNames[] = {"ok", "yes", "no", "pleasewait", "incassmsg", "enterdevnum",
        "performingbalance", "cantgetbalance", "cantprintforincass", "performingincass", "unknown",
        "reloadterminalq", "exitq", "printzreportq", "cantprintzreport1", "cantprintzreport2",
        "printingzreport", "nosavedreceipts", "receiptsaveoff", "generatekey", "enterlogin",
        "enterpassword", "cancelnexttryq", "denumbanknote", "denumcoin", "nominals", "exchange_rate",
        "paperreload", "txtpaperreload", "txtchangeprinter", "incass_no_devices", "choose_incass_device",
        "banknotes_incass", "coins_incass", "back_to_menu"};
    std::string fileName = m_localePath + "translation.ini";
    TIniFile *langdata = new TIniFile(fileName.c_str());
    for(int i = 0; i < sizeof(LocaleNames) / sizeof(LocaleNames[0]); i++)
    {
        std::string localetext = langdata->ReadString("translation", LowerCase(LocaleNames[i]).c_str(), "").c_str();
		boost::replace_all(localetext, "\\n", "\n");
        m_localization[LocaleNames[i]] = localetext;
    }
    delete langdata;
}

const char* CLocalize::operator [](const char* Name)
{
    TLocaleMap::iterator i = m_localization.find(LowerCase(Name).c_str());
    return (i == m_localization.end()) ? "" : i->second.c_str();
}


std::string __fastcall CLocalize::getLocalePath() const
{
    return m_localePath;
}

std::string __fastcall CLocalize::getLocale() const
{
    return m_locale;
}


void CLocalize::setLog(TLogClass* log)
{
    m_log = log;
}

CLocalize Localization;
