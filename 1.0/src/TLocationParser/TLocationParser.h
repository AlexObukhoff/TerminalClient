//---------------------------------------------------------------------------

#ifndef TLocationParserH
#define TLocationParserH

#include <string>
#include <map>
#include <map.h>

class TLocationParser
{
private:
    std::string m_location;
    mutable std::string m_parametrs;
public:
    std::string getURL();
    TLocationParser(const char* _Location);
    bool HasParameter(const char* ParamName) const;
    std::string GetParameter(std::string ParamName) const;
    void SetParameter(const char* ParamName, const char* ParamValue);
    std::string GetParameters() const;
    std::map<std::string, std::string> Parameters;
    __property std::string URL = {read = getURL};
    std::string PageName;
};

//---------------------------------------------------------------------------
#endif
