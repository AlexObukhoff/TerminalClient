//---------------------------------------------------------------------------

#ifndef JSONTreeH
#define JSONTreeH
#include <system.hpp>
#include <vector>
#include "LogClass.h"

//const bool RootItem = true;

class JSONItem
{
public:
  AnsiString Name;
  AnsiString Value;
  bool isComment;
  std::vector <JSONItem> ChildItems;
  JSONItem() { isComment = false; };
  JSONItem(AnsiString _Name, AnsiString _Value, bool _isComment = false) { Name = _Name; Value = _Value; isComment = _isComment; };
  bool HasNotOnlyComments(unsigned start);
};

class JSONDocument
{
  TLogClass* Log;
  AnsiString FileData;
  GetObject(AnsiString Data, unsigned long& StartOffset, AnsiString& ObjName, AnsiString& ObjBody);
  AnsiString JSONDocument::GetItemJSONString(JSONItem& ItemToParse, AnsiString Margin, bool isRootItem = false);
  bool ParseItem (AnsiString ItemName, AnsiString ItemString, JSONItem& ItemToParse, AnsiString Margin, bool isRootItem = false);
public:
  JSONDocument(TLogClass* _Log);
  ~JSONDocument();
  AnsiString MarginChar;
  JSONItem RootItem;
  bool OpenFile (AnsiString _FileName);
  AnsiString GetJSONString();
};


//---------------------------------------------------------------------------
#endif
 