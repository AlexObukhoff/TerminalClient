//---------------------------------------------------------------------------


#pragma hdrstop

#include "JSONDocument.h"
#include "common.h"

//---------------------------------------------------------------------------

bool JSONItem::HasNotOnlyComments(unsigned start)
{
  for (unsigned i = start; i<ChildItems.size();i++)
  {
    if (!ChildItems[i].isComment)
      return true;
  }
  return false;
}


JSONDocument::JSONDocument(TLogClass* _Log)
{
Log = _Log;
MarginChar = "\t";
//Log->Write("JSONDocument created.");
}

JSONDocument::~JSONDocument()
{
//Log->Write("JSONDocument done.");
}

bool JSONDocument::OpenFile (AnsiString _FileName)
{
  GetFileData(_FileName, FileData, Log);
  FileData+="\r\n";
  FileData = RemoveSubStrings(FileData,"/*","*/");
  FileData = RemoveSubStrings(FileData,"//","\n");

  //Log->Write(FileData);
//  Log->Write("Start...");
  ParseItem("root",FileData,RootItem, "", true);
//  Log->Write("All done.");
  return true;
}

/*AnsiString JSONDocument::GetNextString()
{
  long MaxChars = FileData.Length();
  CurrStrEnd = CurrStrBegin;
  while ((CurrStrEnd<MaxChars-1)&&(FileData.c_str()[CurrStrEnd]!='\r')&&(FileData.c_str()[CurrStrEnd+1]!='\n'))
    CurrStrEnd++;
  AnsiString Temp = AnsiString(FileData.c_str()+CurrStrBegin,CurrStrEnd-CurrStrBegin);
  CurrStrBegin = CurrStrEnd+2;
  return Temp;
}*/

JSONDocument::GetObject(AnsiString Data, unsigned long& StartOffset, AnsiString& ObjName, AnsiString& ObjBody)
{
  unsigned long MaxChars = Data.Length();
  unsigned long Pos = StartOffset;
  int Level = 0;
  unsigned long NameValueDelimiter = 0;
  bool bOpened = false;
//  bool bMultiStringCommentStarted = false;
//  bool bOneStringCommentStarted = false;
  ObjName = "";
  ObjBody = "";

  while (Pos < MaxChars)
  {
/*    if ((!bMultiStringCommentStarted)&&(!bOneStringCommentStarted)&&(Data.c_str()[Pos]=='/')&&(Data.c_str()[Pos-1]=='/'))
    {
      Log->Write("comment entered at "+AnsiString(Pos)+ " " + AnsiString(Data.c_str()[Pos+1]));
      bOneStringCommentStarted = true;
    }

    if ((!bMultiStringCommentStarted)&&(Data.c_str()[Pos]=='*')&&(Data.c_str()[Pos-1]=='/'))
    {
      Log->Write("multistring comment entered at "+AnsiString(Pos)+ " " + AnsiString(Data.c_str()[Pos+1]));
      bMultiStringCommentStarted = true;
    }

    if (bMultiStringCommentStarted)
    {
      if ((Data.c_str()[Pos]=='/')&&(Data.c_str()[Pos-1]=='*'))
      {
      Log->Write("multistring comment leaved at "+AnsiString(Pos)+ " " + AnsiString(Data.c_str()[Pos+1]));
        bMultiStringCommentStarted = false;
      }
    }
    else
    {
      if (bOneStringCommentStarted)
      {
        if (Data.c_str()[Pos]=='\n')
        {
        Log->Write("comment leaved at "+AnsiString(Pos)+ " " + AnsiString(Data.c_str()[Pos+1]));
          bOneStringCommentStarted = false;
        }
      }
    }

    if ((!bMultiStringCommentStarted)&&(!bOneStringCommentStarted))
    {*/
      if ((!bOpened)&&(Data.c_str()[Pos]=='}'))
      {
        Level--;
        //if (Level < 0)
        //  throw Exception("Level error!");
      }

      if ((Data.c_str()[Pos]=='\'')&&(Data.c_str()[Pos-1]!='\\'))
        bOpened = 1 - bOpened;

      if ((!bOpened)&&(Level==0)&&((Data.c_str()[Pos]==',')||(Data.c_str()[Pos]=='}')||(Data.c_str()[Pos]==';')||(Data.c_str()[Pos]=='\n')))
      {
        break;
      }

      if ((!bOpened)&&(Data.c_str()[Pos]=='{'))
        Level++;

      if ((!bOpened)&&((Data.c_str()[Pos]==':')||(Data.c_str()[Pos]=='='))&&(NameValueDelimiter == 0))
        NameValueDelimiter = Pos;
    //}
    Pos++;
  }
  if ( NameValueDelimiter >= StartOffset )
  {
//    ObjName = RemoveSubStrings(RemoveSubStrings(ChangeChars(AnsiString(Data.c_str()+StartOffset,NameValueDelimiter - StartOffset).Trim(),"'",""),"/*","*/"),"//","\n").Trim();
    ObjName = ChangeChars(AnsiString(Data.c_str()+StartOffset,NameValueDelimiter - StartOffset).Trim(),"'","").Trim();
    ObjBody = AnsiString(Data.c_str()+NameValueDelimiter+1,Pos - NameValueDelimiter-1).Trim();
    if (ObjBody.c_str()[0]=='\'')
    {
      ObjBody = ObjBody.SubString(2,ObjBody.Length()-2);
      ObjBody = ChangeChars(ObjBody,"\\'","\'");
    }
  }
  StartOffset = Pos+1;
}

bool JSONDocument::ParseItem (AnsiString ItemName, AnsiString ItemString, JSONItem& ItemToParse, AnsiString Margin, bool isRootItem )
{
  //Log->Write(Margin+"parsing "+ItemName+"...");
  AnsiString SObjName, SObjValue;

  unsigned long offset = 0;

  if (ItemString.c_str()[0]=='{')
    offset = 3;

  ULONG bc = ItemString.Length();
  ItemToParse.Name = ItemName;
  while (offset<bc)
  {
    //Log->Write(Margin + MarginChar + " lookup from " + AnsiString(offset) + " of " + AnsiString(ItemString.Length()));
    GetObject(ItemString, offset, SObjName, SObjValue);

    if ((isRootItem)&&(SObjName.Pos("var")==1))
      SObjName = SObjName.SubString(4,SObjName.Length()).Trim();

    //Log->Write(Margin + MarginChar + " found "+SObjName+"... {"+SObjValue+"} in {" + ItemString + "}" );
    if (SObjName != "")
    {
      if (SObjValue.c_str()[0]!='{')
      {
        JSONItem NewItem;
        NewItem.Name = SObjName;
        NewItem.Value = SObjValue;
        ItemToParse.ChildItems.push_back(NewItem);
        //Log->Write(Margin+" {" +NewItem.Name+" -+- "+NewItem.Value+" }");
      }
      else
      {
        if (SObjName!="//")
        {
          JSONItem NewItem;
          ParseItem(SObjName, SObjValue, NewItem, Margin + MarginChar);
          ItemToParse.ChildItems.push_back(NewItem);
        }
        else
        {
          JSONItem NewItem;
          NewItem.Name = SObjName;
          NewItem.Value = SObjValue;
          NewItem.isComment = true;
          ItemToParse.ChildItems.push_back(NewItem);
        }
      }
    }
  }
  return true;
}

AnsiString JSONDocument::GetItemJSONString(JSONItem& ItemToParse, AnsiString Margin, bool isRootItem )
{
  AnsiString Result;
  if (ItemToParse.isComment)
    Result = Margin + "// " + ItemToParse.Name;
  else
  {
    Result = Margin + (isRootItem ? "var " : "\'") + ItemToParse.Name + (isRootItem ? " = " : "' : ");

    if (ItemToParse.ChildItems.size()==0)
    {
      Result+= "\'"+ChangeChars(ItemToParse.Value,"\'","\\'")+"\'";
    }
    else
    {
      Result+= "{\r\n";
      for (unsigned i=0;i<ItemToParse.ChildItems.size();i++)
      {
        Result += GetItemJSONString(ItemToParse.ChildItems[i], Margin + MarginChar) +
          (((i==ItemToParse.ChildItems.size()-1)||(ItemToParse.ChildItems[i].isComment)||(!ItemToParse.HasNotOnlyComments(i+1))) ? "\r\n" : ",\r\n");
      }
      Result+= Margin + "}";
    }
  }
  return Result;
}

AnsiString JSONDocument::GetJSONString()
{
  AnsiString Result;
  for (unsigned i=0;i<RootItem.ChildItems.size();i++)
  {
    Result += GetItemJSONString(RootItem.ChildItems[i],"", true) + ";\r\n";
  }
  return Result;
}

#pragma package(smart_init)
