//---------------------------------------------------------------------------
#pragma hdrstop

#include "TLocationParser.h"
#include "IdURI.hpp"
#include <IdHTTP.hpp>
#include <vector>
#include "boost\algorithm\string.hpp"
#include <boost\regex.hpp>
//#include "boost\any.hpp"

//---------------------------------------------------------------------------

TLocationParser::TLocationParser(const char* _Location)
{
try
{
    std::string tURL = _Location;
    boost::replace_all(tURL, "\\", "/");
    const std::string URLregex = "(.*)/((.*\\.html)\\??(.*))";
    boost::regex URLexpr(URLregex);

    //std::auto_ptr <TIdURI> iduri (new TIdURI(_Location));
    //if (iduri.get())
    if(boost::regex_match(tURL, URLexpr))
    {
      boost::cmatch what_pb;
      boost::regex_search(tURL, what_pb, URLexpr);

      //m_location = URLDecode(iduri->Document).c_str();
      m_location = URLDecode(what_pb[2].str().c_str()).c_str();

      int lastChar = m_location.length() - 1;
      if (m_location[lastChar] == '?')
        m_location.erase(lastChar, 1);

      if(std::string::npos == m_location.find("?"))
      {
          PageName = m_location;
          m_parametrs = "";
      }
      else
      {
          PageName = m_location.substr(0, m_location.find("?"));
          m_parametrs = m_location.substr(m_location.find("?") + 1);
          std::vector<std::string> tokens;
          std::string separator = "&";
          std::string strtmp = m_parametrs;

          while(true)
          {
              if(std::string::npos == strtmp.find(separator))
              {
                  tokens.push_back(strtmp);
                  break;
              }

              tokens.push_back(strtmp.substr(0, strtmp.find(separator)));
              strtmp = strtmp.substr(strtmp.find(separator) + 1,strtmp.length());
          }

          std::vector<std::string> pair;
          separator = "=";
          for(std::size_t i=0;i<tokens.size();i++)
          {
              strtmp = tokens[i];
              if(std::string::npos == strtmp.find(separator))
              {
                  Parameters[strtmp.c_str()]="";
                  continue;
              }

              Parameters[strtmp.substr(0, strtmp.find(separator)).c_str()] = strtmp.substr(strtmp.find(separator) + 1,strtmp.length());
          }
          /* в Visual Studio все компилится
          boost::split(tokens, m_parametrs, boost::is_any_of(separator.c_str()) );
          for(std::size_t i=0;i<tokens.size();i++)
          {
              std::vector<std::string> pair;
              boost::split(pair, tokens[i], boost::is_any_of ("=") );
              if(pair.size() != 2)
                  throw std::invalid_argument(tokens[i].c_str());
              else
                  parameters[pair[0]]=pair[1];
          }
          */
      }
    }
}
catch(...) {}
}

std::string TLocationParser::getURL()
{
    m_parametrs = GetParameters();
    return (m_parametrs == "") ? PageName : (PageName + "?" + m_parametrs);
}

bool TLocationParser::HasParameter(const char* ParamName) const
{
    return (Parameters.find(ParamName) != Parameters.end());
}

std::string TLocationParser::GetParameter(std::string ParamName) const
{
    if(Parameters.find(ParamName) != Parameters.end())
        return Parameters.find(ParamName)->second;
    else
        return "";
}

void TLocationParser::SetParameter(const char* ParamName, const char* ParamValue)
{
    std::string tmpName = ParamName;
    std::string tmpValue = ParamValue;

    std::map<std::string, std::string>::iterator it = Parameters.find(tmpName);
    if(it != Parameters.end())
    {
      it->second = tmpValue;
    }
    else
    {
      Parameters.insert(make_pair<std::string, std::string>(ParamName, ParamValue));
    }
}

std::string TLocationParser::GetParameters() const
{
    m_parametrs = "";
    for(std::map<std::string,std::string>::const_iterator it=Parameters.begin();it!=Parameters.end();++it)
    {
        if(""==m_parametrs)
            m_parametrs=it->first+"="+it->second;
        else
            m_parametrs+="&"+it->first+"="+it->second;
    }

    return m_parametrs;
}
//---------------------------------------------------------------------------

#pragma package(smart_init)
