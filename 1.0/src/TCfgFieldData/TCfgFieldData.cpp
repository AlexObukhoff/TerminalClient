//---------------------------------------------------------------------------


#pragma hdrstop

#include "TCfgFieldData.h"
#include "common.h"
#include <ras.h>
#include <raserror.h>
#include "PacketSender.h"
#include "CSPacketSender.h"
#include "SSPacketSender.h"
#include "globals.h"
#include "boost/format.hpp"

#pragma package(smart_init)

//---------------------------------------------------------------------------

TCfgFieldData::TCfgFieldData(TWConfig *_Cfg, TLogClass *_Log, AnsiString _NodeName, AnsiString _Text, bool _isAttribute, bool _isTextNode, AnsiString _Rule)
{
Log = NULL;
EnumDataSL = NULL;
ListDataSL = NULL;
SArraySL = NULL;
Name = _NodeName;
Text = _Text;
isAttribute = _isAttribute;
isTextNode = _isTextNode;
Rule = _Rule;
CheckInProgress = false;
isActiveNode = false;
Log = _Log;
Cfg = _Cfg;
COMPortsScanned = IntMin = IntMax = IntDef = 0;
ParseRule();
}

//---------------------------------------------------------------------------

TCfgFieldData::~TCfgFieldData()
{
if (EnumDataSL)
  {
  delete EnumDataSL;
  EnumDataSL = NULL;
  }
if (ListDataSL)
  {
  delete ListDataSL;
  ListDataSL = NULL;
  }
if (SArraySL)
  {
  delete SArraySL;
  SArraySL = NULL;
  }
}

//---------------------------------------------------------------------------

AnsiString TCfgFieldData::Display(int Index)
{
AnsiString TextToDisplay;

if ((!this->isTextNode)&&(Type!=ftSArray))
  if (Index==0)
    return DisplayName;

try
  {
  switch (Type)
    {
    case ftInt:
    case ftString:
    case ftList:
      TextToDisplay = Text;
      break;
    case ftBool:
      TextToDisplay = (bool(GetInt(Text)) ? "Да" : "Нет");
      break;
    case ftEnum:
    case ftComPortsEnum:
      TextToDisplay = Text;
      for (int i=0;i<EnumDataSL->Count;i++)
        if (EnumDataSL->Values[EnumDataSL->Names[i]]==Text)
          {
          TextToDisplay = EnumDataSL->Names[i];
          break;
          }                                         
      break;
    case ftSArray:
      TextToDisplay = SArraySL->DelimitedText;
      /*
      SArrayName = Rule;
      if (SArrayName.Pos("["))
        {
        SArrayName = SArrayName.SubString(SArrayName.Pos("[")+1, SArrayName.Length());
        if (SArrayName.Pos("]"))
          SArrayName = SArrayName.SubString(0, SArrayName.Pos("]")-1);
        }*/
      break;
    }
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
if (Index==0)
  return DisplayName;
  else
  return TextToDisplay;
}

//---------------------------------------------------------------------------

void TCfgFieldData::ParseRule()
{
  try
    {
    Comment = Rule;
    if (Comment.Pos("//"))
      {
      Comment = Comment.SubString(Comment.Pos("//")+2,Comment.Length());
      if (Comment.Pos("||"))
        {
        Comment = Comment.SubString(0,Comment.Pos("||")-1);
        }
      }
      else
      Comment = "";

    DisplayName = Rule;
    if (DisplayName.Pos("||"))
      {
      DisplayName = DisplayName.SubString(DisplayName.Pos("||")+2,DisplayName.Length());
      }
      else
      {
      DisplayName = (isAttribute ? "[" : "") + Name + (isAttribute ? "]" : "");
      }

    AnsiString AType = Rule;
    if (AType.Pos("//"))
      {
      AType = AType.SubString(0,AType.Pos("//")-1);
      if (AType.Pos("("))
        {
        AType = AType.SubString(0,AType.Pos("(")-1);
        }
      }

    Type = (isTextNode ? ftString : ftNone);

    if (AType.Pos("."))
      {
      ASubTypes = (AType.SubString(AType.Pos("."),AType.Length())+".").LowerCase();
      AType = AType.SubString(0,AType.Pos(".")-1).LowerCase();
      }

    if (AType=="int")
      Type = ftInt;
      else
      if (AType=="bool")
        Type = ftBool;
        else
        if (AType=="string")
          Type = ftString;
          else
          if (AType=="list")
            Type = ftList;
            else
            if (AType=="enum")
              {
              Type = ftEnum;
              if (ASubTypes.Pos(".comports."))
                Type = ftComPortsEnum;
              }
              else
              if (AType=="sarray")
                Type = ftSArray;

    AnsiString FieldData = Rule;
    if (FieldData.Pos("//"))
      {
      FieldData = FieldData.SubString(0,FieldData.Pos("//")-1);
      if (FieldData.Pos("("))
        {
        FieldData = FieldData.SubString(FieldData.Pos("(")+1,FieldData.Length());
        if (FieldData.Pos(")"))
          {
          FieldData = FieldData.SetLength(FieldData.Length()-1);
          }
        }
      }

  AnsiString Temp = FieldData;
  AnsiString Temp2;

    switch (Type)
      {
      case ftOther:
        break;
      case ftInt:
        IntMin = IntMax = IntDef = 0;
        if (Temp.Pos(","))
          {
          Temp2 = Temp.SubString(0,Temp.Pos(",")-1);
          Temp = Temp.SubString(Temp.Pos(",")+1,Temp.Length());
          IntMin = GetInt(Temp2);
          if (Temp.Pos(","))
            {
            Temp2 = Temp.SubString(0,Temp.Pos(",")-1);
            Temp = Temp.SubString(Temp.Pos(",")+1,Temp.Length());
            IntMax = GetInt(Temp2);
            IntDef = GetInt(Temp);
            }
          }
        break;
      case ftList:
        ListData = ChangeChars(FieldData,"'","\"");
        ListDataSL = new TStringList();
        ListDataSL->DelimitedText = ChangeChars(FieldData,"'","\"");
        if (ASubTypes.Pos(".rasconn."))
          ListData = GetRasConnections();
          else
          ListData = ListDataSL->DelimitedText;
        ListIndex = ListDataSL->IndexOf(Text);
        break;
      case ftComPortsEnum:
        FieldData = "\"Отсутствует=0\","+GetCOMPortsList(GetInt(FieldData));
      case ftEnum:
        ListIndex = -1;
        EnumDataSL = new TStringList();
        if (ASubTypes.Pos(".oplist."))
          {
          for (std::size_t i=0;i<Cfg->OperatorsInfo.size();i++)
          //for (int i=0;i<Cfg->OperatorsInfo->Count;i++)
            EnumDataSL->Add((Cfg->OperatorByNum(i).Name+"=").c_str()+AnsiString(Cfg->OperatorByNum(i).Id));
          }
          else
          EnumDataSL->DelimitedText = ChangeChars(FieldData,"'","\"");
        ListDataSL = new TStringList();
        ListData = "";
        for (int i=0;i<EnumDataSL->Count;i++)
          {
          ListDataSL->Add(EnumDataSL->Names[i]);
          if (EnumDataSL->Values[EnumDataSL->Names[i]]==Text)
            ListIndex = i;
          }
        ListData = ListDataSL->DelimitedText;
        break;
      case ftSArray:
        SArraySL = new TStringList();
        SArrayName = Rule;
        if (SArrayName.Pos("["))
          {
          SArrayName = SArrayName.SubString(SArrayName.Pos("[")+1, SArrayName.Length());
          if (SArrayName.Pos("]"))
            SArrayName = SArrayName.SubString(0, SArrayName.Pos("]")-1);
          }
        break;
      }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TCfgFieldData::Store(AnsiString Data)
{
    try
    {
    switch (Type)
      {
      case ftInt:
      case ftString:
      case ftBool:
        Text = Data;
        break;
      case ftSArray:
        SArraySL->DelimitedText = Data;
        break;
      case ftList:
        Text = Data;
        //ListIndex =
        break;
      case ftEnum:
      case ftComPortsEnum:
        Text = EnumDataSL->Values[Data];
        ListIndex = EnumDataSL->IndexOfName(Data);
        break;
      }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

AnsiString TCfgFieldData::GetCOMPortsList(int MaxComPortNumber)
{
AnsiString result = ((COMPortsScanned>0) ? "," : "");
for(int i=COMPortsScanned+1; i<=MaxComPortNumber; i++)
{
  AnsiString PortName = "\\\\.\\COM"+AnsiString(i);
  //открываем порт
  HANDLE Port = CreateFile(PortName.c_str(),
                    GENERIC_READ | GENERIC_WRITE,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
  // если ошибка - выходим
  if (Port == INVALID_HANDLE_VALUE)
      continue;

  if ((Port != NULL)&&(Port != INVALID_HANDLE_VALUE))
  CloseHandle(Port);
  result += "COM"+AnsiString(i)+"="+AnsiString(i)+",";
  Application->ProcessMessages();
}
result = result.SetLength(result.Length()-1);
COMPortsScanned = MaxComPortNumber;
ComPortsList += result;
return ComPortsList;
}

AnsiString TCfgFieldData::GetRasConnections()
{
TStringList *SL = NULL;
AnsiString ARes;
//log->Write("GetActiveConnection");
try
  {
  try
    {
    SL = new TStringList();
    const ULONG MaxEnt = 100;
    RASENTRYNAME  RasEntryNames[MaxEnt];

    ULONG BufSize;
    ULONG ConnectionsNum;

    BufSize = sizeof(RASENTRYNAME)*MaxEnt;
    ZeroMemory(RasEntryNames, BufSize);
    RasEntryNames[0].dwSize = sizeof(RASENTRYNAME);

    int res = RasEnumEntries(NULL,NULL,&RasEntryNames[0],&BufSize,&ConnectionsNum);
    if(res == 0)
      {
      for(ULONG i=0; i<ConnectionsNum; i++)
        {
        SL->Add(AnsiString(RasEntryNames[i].szEntryName));
        }
      }
      else
      if (Log)
        Log->Write(ShowError("RasEnumEntries error.").c_str());
    }
  __finally
    {
    if (SL)
      {
      ARes = SL->DelimitedText;
      delete SL;
      }
    }
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
return ARes;
}

void TCfgFieldData::Check(AnsiString Text, TLabel *CommentLabel, TLabel* CheckLbl)
{
try
  {
  try
    {
    AnsiString Res;
    CheckInProgress = true;
    switch (Type)
      {
      case ftString:
        if (this->ASubTypes.Pos(".checkstaturl."))
          Res = CheckStatURL(Text);
        break;
      }
    CheckResult = "\n\nРезультат проверки: "+Res+".";
    if (isActiveNode)
      {
      CommentLabel->Caption = Comment + CheckResult;
      if (CheckLbl)
        CheckLbl->Enabled=true;
      }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
  }
__finally
  {
  CheckInProgress = false;
  }
}


AnsiString TCfgFieldData::CheckStatURL(AnsiString URL)
{
    TPacketSender *TestPS = NULL;
    AnsiString ARes;
    try
    {
        try
        {
            if (URL=="")
                return "URL error";
            if (Cfg->StatInfo.ProcessorType==cnCyberPlatServer)
                TestPS = new TCSPacketSender("", Cfg, Log, NULL);
            else
                TestPS = new TSSPacketSender("", Cfg, Log, NULL);

            ARes=TestPS->TestConnection(URL);
            ARes=ChangeChars(ChangeChars(ARes,"/","$#@"),"$#@","/ ");
            if (Log) Log->WriteInLine((boost::format("Проверка связи с сервером мониторинга: %1%") % ARes.c_str()).str().c_str());
        }
        catch (Exception &exception)
        {
            if (Log) Log->WriteInLine((boost::format("Exception in TForm1::CheckConnectionToMonitoringServer: %1%") % exception.Message.c_str()).str().c_str());
                ARes += "\nПроверка связи с сервером мониторинга:  "+exception.Message;
        }
        catch (...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        }
    }
    __finally
    {
        if (TestPS)
        {
            delete TestPS;
            TestPS=NULL;
        }
    }
    return ARes;
}
