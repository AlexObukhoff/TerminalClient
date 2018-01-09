//---------------------------------------------------------------------------

#include <system.hpp>

#include "TJSMaker.h"
#pragma hdrstop

//---------------------------------------------------------------------------

TJSMaker::TJSMaker()
{
    Body = "";
    Tab = "\t";
    Level = 0;
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

void TJSMaker::AddString(const char* String)
{
    Body+="\n";
    Body+=String;
}

void TJSMaker::AddString(const AnsiString& String)
{
    AddString(String.c_str());
}

//---------------------------------------------------------------------------

void TJSMaker::AddChild(const char* Name)
{
    Body+="\n";
    for (int i=0;i<Level;i++)
      Body+=_Tab;
    Body+="'";
    Body+=Name;
    Body+="' : {";
    Level++;
}

void TJSMaker::AddChild(const AnsiString& Name)
{
    AddChild(Name.c_str());
}
//---------------------------------------------------------------------------
void TJSMaker::CloseChild(bool comma)
{
    Level--;

    if (Body[Body.length()-1]==',')
        Body=Body.substr(0,Body.length()-1);
    Body+="\n";
    for (int i=0;i<Level;i++)
        Body+=_Tab;
    Body+=( comma ? "}," : "}");
}

//---------------------------------------------------------------------------

void TJSMaker::AddStringAttribute(const char* Name, const char* Value)
{
    Body+="\n";
    for (int i=0;i<Level;i++)
        Body+=_Tab;
    Body+="'";
    Body+=Name;
    Body+="' : '";
    Body+=Value;
    Body+="',";
}

void TJSMaker::AddStringAttribute(const char* Name, const AnsiString& Value)
{
    AddStringAttribute(Name,Value.c_str());
}

void TJSMaker::AddStringAttribute(const AnsiString& Name, const AnsiString& Value)
{
    AddStringAttribute(Name.c_str(),Value.c_str());
}
//---------------------------------------------------------------------------

void TJSMaker::AddBoolAttribute(const char* Name, bool Value)
{
    Body+="\n";
    for (int i=0;i<Level;i++)
        Body+=_Tab;
    Body+="'";
    Body+=Name;
    Body+="' : ";
    if (Value)
        Body+="true";
    else
        Body+="false";
    Body+=",";
}

//---------------------------------------------------------------------------

void TJSMaker::AddAttribute(const char* Name, const char* Value)
{
    Body+="\n";
    for (int i=0;i<Level;i++)
        Body+=_Tab;
    Body+="'";
    Body+=Name;
    Body+="' : '";
    Body+=Value;
    Body+="',";
}

//---------------------------------------------------------------------------


#pragma package(smart_init)
