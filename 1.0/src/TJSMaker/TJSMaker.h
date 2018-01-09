//---------------------------------------------------------------------------

#ifndef TJSMakerH
#define TJSMakerH

#include <system.hpp>
#include <string>
//---------------------------------------------------------------------------

class TJSMaker
{
    std::string _Tab;
    std::string Body;
    const char* GetBody() {return Body.c_str();};
    void SetBody(const char* _AContent) {Body = _AContent;};
    void SetTab(const char* _ATab) {_Tab = _ATab;};
public:
    TJSMaker();
    void Clear() {Body = "";};
    void AddString(const char* Str);
    void AddString(const AnsiString& Name);
    int Level;
    void AddChild(const char* Name);
    void AddChild(const AnsiString& Name);
    void CloseChild(bool comma = true);
    void AddStringAttribute(const char* Name, const char* Value = "");
    void AddStringAttribute(const char* Name, const AnsiString& Value = "");
    void AddStringAttribute(const AnsiString& Name, const AnsiString& Value = "");
    void AddBoolAttribute(const char* Name, bool Value = false);
    void AddAttribute(const char* Name, const char* Value = "");
    __property const char* Content = { read=GetBody, write=SetBody};
    __property const char* Tab = { write=SetTab};
};


#endif
