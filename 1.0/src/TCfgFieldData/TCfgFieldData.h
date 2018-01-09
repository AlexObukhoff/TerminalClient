//---------------------------------------------------------------------------

#ifndef TCfgFieldDataH
#define TCfgFieldDataH

#include <Classes.hpp>
//#include "VirtualTrees.hpp"
#include "TWConfig.h"
#include "LogClass.h"

enum field_type {ftNone, ftBool, ftInt, ftString, ftList, ftEnum, ftComPortsEnum, ftSArray, ftOther=255};

class TCfgFieldData
{
  TWConfig *Cfg;
  TLogClass *Log;
  AnsiString Rule;
  int COMPortsScanned;
  AnsiString GetCOMPortsList(int MaxComPortNumber);
  AnsiString ComPortsList;
  TStringList* EnumDataSL;
  TStringList* ListDataSL;
  TStringList* SArraySL;
  AnsiString GetRasConnections();
  AnsiString DisplayName;
  void ParseRule();
  AnsiString readSArrayData(){if (SArraySL) return SArraySL->DelimitedText; else return "";};
  AnsiString CheckStatURL(AnsiString URL);
public:
  TCfgFieldData(TWConfig *_Cfg, TLogClass *_Log, AnsiString _NodeName, AnsiString _Text, bool _isAttribute, bool _isTextNode, AnsiString _Rule);
  ~TCfgFieldData();

  AnsiString Display(int Index);

  AnsiString Name;
  bool isAttribute;
  bool isTextNode;
  AnsiString Text;
  AnsiString Comment;
  AnsiString CheckResult;

  field_type Type;
  AnsiString ASubTypes;

  int IntMin;
  int IntMax;
  int IntDef;

  AnsiString SArrayName;
  AnsiString ListData;
  int ListIndex;
  void Store(AnsiString Data);
  __property AnsiString SArrayData = { read=readSArrayData};
  void Check(AnsiString Text, TLabel *CommentLabel, TLabel* CheckLbl);
  bool CheckInProgress;
  bool isActiveNode;
};

//---------------------------------------------------------------------------
typedef struct
{
 TCfgFieldData *NodeData;
} VTVNODEDATA;
#endif
 