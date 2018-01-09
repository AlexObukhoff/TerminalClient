//---------------------------------------------------------------------------

#include <classes.hpp>
#include <system.hpp>
#include <XMLDoc.hpp>
#include <SysUtils.hpp>
#include <memory>
#include "globals.h"
#include <algorith.h>
#include "boost/format.hpp"
#include <boost\lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost\algorithm\string.hpp>

#pragma hdrstop

#include "XMLPacket.h"
//#include "PacketSender.h"


//---------------------------------------------------------------------------

#pragma package(smart_init)

//---------------------------------------------------------------------------

TXMLPacket::TXMLPacket(TWConfig *Cfg, TLogClass *_Log)
{
    try
    {
        CoInitializeEx(NULL, COINIT_MULTITHREADED);
        randomize();
        cfg = Cfg;
        InnerLog=false;
        if (_Log==NULL)
        {
            Log = new TLogClass("TXMLPacket");
            InnerLog=true;
        }
        else
        {
            Log=_Log;
        }

        OutboundDir = "";
        TempOutboundDir = "";

        Params = NULL;
        PktFile = NULL;

        Params = new TList;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        if (Params)
        {
            delete Params;
            Params = NULL;
        }
        throw;
    }
}

//---------------------------------------------------------------------------

TXMLPacket::~TXMLPacket()
{
    try
    {
        if (PktFile)
        {
            delete PktFile;
            PktFile = NULL;
        }

        if (PacketFileName!="")
        {
            if ((FileExists(PacketFileName.c_str()))&&(FileSizeByName(PacketFileName.c_str())==0))
            {
                //Log->Write("FileSizeByName: "+AnsiString(FileSizeByName(PacketFileName)));
                if (!DeleteFile(PacketFileName.c_str()))
                    Log->Write((boost::format("  Error deleting zero-sized packet file %1% !") % PacketFileName.c_str()).str().c_str());
            }
        }

        //Log->Write("TXMLPacket destructor entered.");
        Clear();
        if (Params)
        {
            delete Params;
            Params = NULL;
        }

//    Log->Write("TXMLPacket done.");
        if (InnerLog)
        {
            delete Log;
            Log=NULL;
        }
        //CoUninitialize();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TXMLPacket::Clear()
{
    try
    {
        PParams Parameter;
        for (int i=0;i<Params->Count;i++)
        {
            Parameter=(PParams)Params->Items[i];
            delete Parameter;
        }
        Params->Clear();

        vNotes.clear();

        PacketFileName="";
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

AnsiString TXMLPacket::TruncateFileName(AnsiString _fileName)
{
    AnsiString Temp;
    Temp=_fileName;
    while (Temp.Pos("\\")!=0)
        Temp.Delete(1,Temp.Pos("\\"));
    return Temp;
}

//---------------------------------------------------------------------------

void TXMLPacket::SetParamValue(AnsiString _ParamName,AnsiString _ParamValue)
{
    try
    {
        if (!Params)
            return;

        PParams Parameter;

        for (int i=0;i<Params->Count;i++)
        {
            Parameter=(PParams)Params->Items[i];
            if (Parameter->Name==_ParamName)
            {
                Parameter->Value=_ParamValue;
                return;
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TXMLPacket::SetParamValue(AnsiString _ParamName,float _ParamValue)
{
    try
    {
        if (!Params)
            return;

        PParams Parameter;
        AnsiString Temp=ChangeChars(FloatToStrF(_ParamValue,ffFixed,18,2),",",".");

        for (int i=0;i<Params->Count;i++)
        {
            Parameter=(PParams)Params->Items[i];
            if (Parameter->Name==_ParamName)
            {
                Parameter->Value=Temp;
                return;
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TXMLPacket::DeleteParam(AnsiString paramName)
{
    try
    {
        if(!Params)
            return;

        int paramIndex = -1;
        for(int i = 0; i < Params->Count; i++)
        {
            PParams Parameter = (PParams)Params->Items[i];
            if(Parameter->Name == paramName)
            {
                paramIndex = i;
                break;
            }
        }

        if(paramIndex != -1)
            Params->Delete(paramIndex);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TXMLPacket::AddParam(AnsiString _ParamName,AnsiString _ParamValue)
{
    try
    {
        if (!Params)
            return;

        for (int i=0;i<Params->Count;i++)
        {
            PParams Parameter=(PParams)Params->Items[i];
            if (Parameter->Name==_ParamName)
            {
                Parameter->Value=_ParamValue;
                return;
            }
        }

        PParams NewParameter = new TPParams;
        NewParameter->Name=_ParamName;
        NewParameter->Value=_ParamValue;
        Params->Add(NewParameter);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TXMLPacket::AddParamI(AnsiString _ParamName,int _ParamValue)
{
    AddParam(_ParamName,AnsiString(_ParamValue));
}

//---------------------------------------------------------------------------

void TXMLPacket::AddParam(AnsiString _ParamName,float _ParamValue)
{
    try
    {
        AnsiString Temp=ChangeChars(FloatToStrF(_ParamValue,ffFixed,18,2),",",".");
        AddParam(_ParamName,Temp);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

AnsiString TXMLPacket::GetParamValue(AnsiString _ParamName)
{
    try
    {
        PParams Parameter;

        for (int i=0;i<Params->Count;i++)
        {
            Parameter=(PParams)Params->Items[i];
            if (Parameter->Name==_ParamName)
            {
                return Parameter->Value;
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return "";
}

//---------------------------------------------------------------------------

AnsiString TXMLPacket::GetParamName(int _index)
{
    try
    {
        PParams Parameter;
        if (_index>Params->Count-1)
            return "";
        Parameter=(PParams)Params->Items[_index];
        return Parameter->Name;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return "";
}

//---------------------------------------------------------------------------

AnsiString TXMLPacket::GetParamValue(int _index)
{
    try
    {
        PParams Parameter;
        if (_index>Params->Count-1)
            return "";
        Parameter=(PParams)Params->Items[_index];
        return Parameter->Value;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return "";
}

//---------------------------------------------------------------------------

bool TXMLPacket::IsExist(AnsiString NodeName,xmlGuard <_di_IXMLNodeList> &xmlNodeList)
{
    try
    {
        xmlGuard <_di_IXMLNode> xmlNode (xmlNodeList->FindNode(NodeName));
        return bool(xmlNode.get());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

//---------------------------------------------------------------------------

AnsiString TXMLPacket::GetString(AnsiString NodeName,xmlGuard <_di_IXMLNodeList> &xmlNodeList)
{
    try
    {
        xmlGuard <_di_IXMLNode> xmlNode (xmlNodeList->FindNode(NodeName.c_str()));
        //_di_IXMLNode xmlNode=xmlNodeList->FindNode(NodeName);
        if ((xmlNode.get())&&(xmlNode->IsTextElement))
            return AnsiString((xmlNode->NodeValue).VOleStr);
        else
            return "";
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return "";
    }
}

//---------------------------------------------------------------------------

TDateTime TXMLPacket::GetDateTime(AnsiString NodeName,xmlGuard <_di_IXMLNodeList> &xmlNodeList)
{
    try
    {
        return TDateTime(GetString(NodeName, xmlNodeList));
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return TDateTime::CurrentDateTime();
}

//---------------------------------------------------------------------------

void TXMLPacket::SetValue(AnsiString NodeName, AnsiString NodeValue, xmlGuard <_di_IXMLNode> &xmlParentNode)
{
    try
    {
        xmlGuard <_di_IXMLNode> FieldNode (xmlParentNode->AddChild(NodeName));
        if (FieldNode.Assigned())
            FieldNode->SetNodeValue(NodeValue);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TXMLPacket::SetValue(AnsiString NodeName, int NodeValue, xmlGuard <_di_IXMLNode> &xmlParentNode)
{
    try
    {
        xmlGuard <_di_IXMLNode> FieldNode (xmlParentNode->AddChild(NodeName));
        if (FieldNode.Assigned())
            FieldNode->SetNodeValue(NodeValue);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TXMLPacket::SetValue(AnsiString NodeName, float NodeValue, xmlGuard <_di_IXMLNode> &xmlParentNode)
{
    try
    {
        xmlGuard <_di_IXMLNode> FieldNode (xmlParentNode->AddChild(NodeName));
        if (FieldNode.Assigned())
            FieldNode->SetNodeValue(NodeValue);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TXMLPacket::RenameTempFile(AnsiString Ext)
{
    try
    {
        if (PktFile)
        {
            delete PktFile;
            PktFile = NULL;
        }

        AnsiString NewFileName=PacketFileName.substr(0,PacketFileName.length()-3).c_str()+Ext;
        if (!RenameFile(PacketFileName.c_str(),NewFileName))
            Log->Append
             (
               (
                boost::format
                ("Error renaming temp file %1% to %2%") %
                 TruncateFileName(PacketFileName.c_str()).c_str() % TruncateFileName(NewFileName).c_str()
               ).str().c_str()
             );
        else
            PacketFileName = NewFileName.c_str();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

bool TXMLPacket::RenamePaketFile(const char* newName)
{
    try
    {
        if (PktFile)
        {
            delete PktFile;
            PktFile = NULL;
        }

        int end=0;
        end=PacketFileName.rfind("\\");

        std::string NewFileName=PacketFileName.substr(0,end)+newName+PacketFileName.substr(PacketFileName.length()-3,4);
        if (!RenameFile(PacketFileName.c_str(),NewFileName.c_str()))
        {
            Log->Append((boost::format("Error renaming temp file %1% to %2%") % TruncateFileName(PacketFileName.c_str()).c_str() % TruncateFileName(NewFileName.c_str()).c_str()).str().c_str());
            return false;
        }
        else
        {
            PacketFileName = NewFileName;
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
    return true;
}

bool TXMLPacket::RenamePaketFileFullPath(const char* newName)
{
    try
    {
        if (PktFile)
        {
            delete PktFile;
            PktFile = NULL;
        }

        if (!RenameFile(PacketFileName.c_str(),newName))
        {
            Log->Append
             (
              (
               boost::format
                ("Error renaming temp file %1% to %2%")
                % TruncateFileName(PacketFileName.c_str()).c_str() % TruncateFileName(newName).c_str()
              ).str().c_str()
             );
             return false;
        }
        else
        {
            PacketFileName = newName;
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------

void TXMLPacket::DeleteTempFile()
{
    try
    {
        if (PktFile)
        {
            delete PktFile;
            PktFile = NULL;
        }
        if (!DeleteFile(PacketFileName.c_str()))
            Log->Append((boost::format("Error deleting temp file %1%") % TruncateFileName(PacketFileName.c_str()).c_str()).str().c_str());
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

bool TXMLPacket::StoreToFile(AnsiString FileName, bool NoUpdate)
{
    UNREFERENCED_PARAMETER(NoUpdate);
    bool bResult = false;
    try
    {
        xmlGuard <_di_IXMLDocument> dXML (NewXMLDocument ());

        dXML->XML->Clear();
        dXML->Active = true;
        dXML->StandAlone = "yes";
        dXML->Encoding = "windows-1251";
        dXML->Options = dXML->Options << doNodeAutoIndent;

        xmlGuard <_di_IXMLNode> Root (dXML->AddChild("root"));
        xmlGuard <_di_IXMLNode> FieldsNode (Root->AddChild("fields"));

        for (int i=0;i<this->Params->Count;i++)
        {
            PParams Parameter=(PParams)Params->Items[i];
            SetValue(Parameter->Name,Parameter->Value,FieldsNode);
        }

        AnsiString Temp;
        dXML->SaveToXML(Temp);
        bResult = SaveFile(FileName,Temp);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return bResult;
}

//---------------------------------------------------------------------------

bool TXMLPacket::SaveFile(AnsiString _FileName, AnsiString SourceData, bool bNoAutoCreate)
{
    bool Res = false;
    try
    {
        if ((PktFile==NULL)&&(!bNoAutoCreate))
        {
            if (FileExists(_FileName))
                LoadFromFile(_FileName);
            else
                CreateFile(_FileName);
        }

        if (PktFile != NULL)
        {
            PktFile->Seek(0, soFromBeginning);
            PktFile->Write(SourceData.c_str(), SourceData.Length());
            PktFile->Size = SourceData.Length();
            Res = true;
        }
        else
        {
            Log->Write((boost::format("Error saving file packet %1% : file not opened!") % TruncateFileName(_FileName).c_str()).str().c_str());
            Res = true;
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Res = false;
    }
    return Res;
}

//---------------------------------------------------------------------------

bool TXMLPacket::CreateFile(AnsiString FileName)
{
    try
    {
        if (PktFile!=NULL)
        {
            delete PktFile;
            PktFile = NULL;
        }

        PacketFileName=FileName.c_str();
        PktFile = new TFileStream(FileName, fmCreate);

        if (PktFile != NULL)
        {
            return true;
        }
        else
        {
            Log->Write((boost::format("Error creating file packet %1%") % FileName.c_str()).str().c_str());
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return false;
}

//---------------------------------------------------------------------------

bool TXMLPacket::CloseFile()
{
    try
    {
        if (PktFile!=NULL)
        {
            delete PktFile;
            PktFile = NULL;
        }
        return true;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

//---------------------------------------------------------------------------

bool TXMLPacket::SaveToFile()
{
    if (PacketFileName!="")
        return StoreToFile(PacketFileName.c_str(), false);
    else
        return StoreToFile(GetNewFileName("pkt"), false);
}

//---------------------------------------------------------------------------

bool TXMLPacket::SaveToTempFile()
{
    if (PacketFileName!="")
        return StoreToFile(PacketFileName.c_str(),true);
    else
        return StoreToFile(GetNewFileName("tmp"),true);
}

//---------------------------------------------------------------------------
AnsiString TXMLPacket::GetNewFileName(AnsiString AFileExt, AnsiString AFilePrefix)
{
    try
    {
        AnsiString AFileDir;
        AnsiString AFileName;
        AFileDir=(OutboundDir+"\\").c_str();
        AFileName=AFilePrefix+GetFileName();
        while(FileExists(AFileDir+AFileName+"."+AFileExt))
        {
            AFileName=AFilePrefix+GetFileName();
            Log->Write((boost::format("AFileName = %1%%2%.%3%") % AFileDir.c_str() % AFileName.c_str() % AFileExt.c_str()).str().c_str());
        }
        return AFileDir+AFileName+"."+AFileExt;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return "";
}

//---------------------------------------------------------------------------

AnsiString TXMLPacket::GetFileName(void)
{
    try
    {
        AnsiString FileName;
        FileName=FormatDateTime("yymmddhhmmsszzz",Now());
        while (FileName.Length()<24)
            FileName+=rand()%10;
        return FileName;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return "";
}

//---------------------------------------------------------------------------
bool TXMLPacket::LoadFromFile(AnsiString FileName)
{
    m_xmlParseError = false;
    Clear();
    PacketFileName=FileName.c_str();
    bool bResult = false;
    try
    {
        xmlGuard <_di_IXMLDocument> dXML(NULL);
        if (FileExists(FileName))
        {
            AnsiString FileData;
            bool bRes = OpenFile(FileName, FileData);
            if (bRes)
            {
                //try
                //{
                    dXML = LoadXMLData(FileData);
                /*
                }
                catch(...) // не смогли распарсить содержимое файла
                {
                    ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                    FileData = FileData.SubString(0,FileData.Pos("</root>")+6);
                    dXML = LoadXMLData(FileData);
                }
                */
                bResult = GetDataFromXML(dXML);
            }
            else
            {
                Log->Write((boost::format("Error getting data from %1%") % FileName.c_str()).str().c_str());
            }
        }
        else
        {
            Log->Write((boost::format("File %1% does not exists") % FileName.c_str()).str().c_str());
        }
    }
    catch(Exception& ex)
    {
        std::string strtmp = ex.Message.c_str();
        if(std::string::npos != strtmp.find("The following tags were not closed") || std::string::npos != strtmp.find("Processing instruction was not closed"))
        {
            m_xmlParseError = true;
            Log->Write((boost::format("XML parse error: %1%") % ex.Message.c_str()).str().c_str());
        }
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    catch(...)
    {
        bResult = false;
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Log->Write((boost::format("FileName: %1%") % FileName.c_str()).str().c_str());
    }
    return bResult;
}

//---------------------------------------------------------------------------
bool TXMLPacket::getXmlParseError()
{
    return m_xmlParseError;
}
//---------------------------------------------------------------------------

bool TXMLPacket::OpenFile(AnsiString FileName, AnsiString &FileData)
{
    try
    {
        if (PktFile!=NULL)
        {
            delete PktFile;
            PktFile = NULL;
        }
        //FileName = GetNewFileName(Extension);
        if (FileExists(FileName))
        {
            PktFile = new TFileStream(FileName, fmOpenReadWrite | fmShareDenyNone);
            if (PktFile)
            {
                std::vector<char> Buffer( static_cast<std::size_t>(PktFile->Size+1), 0 );
                PktFile->Seek(0, soFromBeginning);
                PktFile->Read(&*Buffer.begin(), static_cast<std::size_t>(PktFile->Size));
                FileData = AnsiString(&*Buffer.begin(), static_cast<std::size_t>(PktFile->Size));
                return true;
            }
            else
            {
                Log->Write((boost::format("Open file packet %1% error") % FileName.c_str()).str().c_str());
            }
        }
        else
        {
            Log->Write((boost::format("Open file packet %1% error. File not found") % FileName.c_str()).str().c_str());
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Log->Write((boost::format("file packet %1% ") % FileName.c_str()).str().c_str());
    }
    return false;
}

bool TXMLPacket::GetDataFromXML(xmlGuard <_di_IXMLDocument> &XMLDoc)
{
    try
    {
        xmlGuard <_di_IXMLNode> RootNode (XMLDoc->GetDocumentElement());
        if (RootNode.Assigned())
        {
            if (RootNode->HasChildNodes)
            {
                xmlGuard <_di_IXMLNodeList> xmlRootNodeList (RootNode->GetChildNodes());
                xmlGuard <_di_IXMLNode> xmlSecNode (xmlRootNodeList->FindNode("fields"));
                if ((xmlSecNode.Assigned())&&(xmlSecNode->HasChildNodes))
                {
                    xmlGuard <_di_IXMLNodeList> xmlNodeList (xmlSecNode->GetChildNodes());
                    if (xmlNodeList.Assigned())
                    {
                        for (int i=0; i<xmlNodeList->Count; i++)
                        {
                            xmlGuard <_di_IXMLNode> xmlNode (xmlNodeList->Get(i));
                            if (xmlNode.Assigned())
                            {
                                if (xmlNode->IsTextElement)
                                    AddParam(xmlNode->NodeName,AnsiString(xmlNode->NodeValue));
                                else
                                    AddParam(xmlNode->NodeName,"");
                            }
                        }
                        return true;
                    }
                }
            }
        }
        Log->Write("Error parsing packet!");
        return false;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}

//---------------------------------------------------------------------------

void TXMLPacket::StoreNotes(const TNotesVector& _Notes, xmlGuard <_di_IXMLNode> &xmlParentNode)
{
    try
    {
        xmlGuard <_di_IXMLNode> NotesNode (xmlParentNode->AddChild("notes"));
        if (NotesNode.Assigned())
        {
            for (std::size_t i=0;i<_Notes.size();i++)
            {
                xmlGuard <_di_IXMLNode> NoteNode (NotesNode->AddChild("note"));
                if (NoteNode.Assigned())
                {
                    NoteNode->SetAttribute("validator_id",_Notes[i].ValidatorID);
                    AnsiString strtmp=_Notes[i].CurrencyID.c_str();//Не убирать, нужно
                    NoteNode->SetAttribute("currency_id",strtmp);
                    NoteNode->SetAttribute("nominal",_Notes[i].Nominal);
                    NoteNode->SetAttribute("count",_Notes[i].Count);
                }
                else
                {
                    Log->Write("Error creating <note> node!");
                }
            }
        }
        else
        {
            Log->Write("Error creating <notes> node!");
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TXMLPacket::GetNotes(TNotesVector& _Notes, xmlGuard <_di_IXMLNodeList> &xmlNodeList)
{
    try
    {
        xmlGuard <_di_IXMLNode> xmlNode (xmlNodeList->FindNode("notes"));
        if (xmlNode.Assigned())
        {
            xmlGuard <_di_IXMLNodeList> NotesNDL (xmlNode->GetChildNodes());
            if (NotesNDL.Assigned())
            {
                for (int i=0;i<NotesNDL->GetCount();i++)
                {
                    xmlGuard <_di_IXMLNode> NoteND (NotesNDL->Get(i));
                    TNote Note;
                    Note.ValidatorID=NoteND->GetAttribute("validator_id");
                    AnsiString strtmp=NoteND->GetAttribute("currency_id");
                    Note.CurrencyID=strtmp.c_str();
                    Note.Nominal=NoteND->GetAttribute("nominal");
                    Note.Count=NoteND->GetAttribute("count");
                    _Notes.push_back(Note);
                }
            }
            else
                  Log->Write("Error getting <notes> nodelist!");
        }
        //else
        //    Log->Write("Error getting <notes> node!");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TXMLPacket::AddNote(int _ValidatorID, double _Nominal)
{
    try
    {
        for (std::size_t i=0;i<vNotes.size();i++)
        {
            if ((vNotes[i].ValidatorID==_ValidatorID)&&(vNotes[i].Nominal==_Nominal))
            {
                vNotes[i].Count++;
                return;
            }
        }

        TNote NewNote;
        NewNote.ValidatorID=_ValidatorID;
        NewNote.CurrencyID=cfg->CurrencyInfo.Currency;
        NewNote.Nominal=_Nominal;
        NewNote.Count=1;
        vNotes.push_back(NewNote);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TXMLPacket::AddNotes(const TNotesVector& _Notes)
{
    vNotes=_Notes;
}

//---------------------------------------------------------------------------

TPaymentPacket::TPaymentPacket(TWConfig *Cfg, TLogClass *_Log) : TXMLPacket(Cfg, _Log)
{
    try
    {
        OperatorId=-1;
        OutboundDir=Cfg->Dirs.PaymentsOutbound.c_str();
        TempOutboundDir=Cfg->Dirs.PaymentsOutboundTemp.c_str();
        NumOfTries=0;
        Cancelled=false;
        Status=0;
        ResultExternal=-1;
        LastErrorCode=0;
        //Notes = new TList();
        //Log->Write("TPaymentPacket initialized.");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

TPaymentPacket::~TPaymentPacket()
{
}

//---------------------------------------------------------------------------

void TPaymentPacket::Clear()
{
    try
    {
        TXMLPacket::Clear();
        OperatorId=-1;
        LastSession="";
        InitialSessionNum="";
        Status=0;
        FirstTryDT.Val=0;
        NumOfTries=0;
        LastErrorCode=0;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TPaymentPacket::ClearForResurrect()
{
    try
    {
        //TXMLPacket::Clear();
        PParams Parameter;
        for (int i=0;i<Params->Count;i++)
        {
            Parameter=(PParams)Params->Items[i];
            if (Parameter->Name!="AMOUNT_ALL")
            {
                Log->Write((boost::format("Parameter %1% = %2% deleted from packet") % Parameter->Name.c_str() % Parameter->Value.c_str()).str().c_str());
                delete Parameter;
                Params->Delete(i);
            }
        }
        //LastSession="";
        Status=0;
        FirstTryDT.Val=0;
        NumOfTries=0;
        LastErrorCode=0;
        Cancelled=false;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

bool TPaymentPacket::StoreToFile(AnsiString FileName,bool NoUpdate)
{
    AnsiString ErrInfo;
    bool bResult = false;
    try
    {
        PParams Parameter;

        xmlGuard <_di_IXMLDocument> dXML (NewXMLDocument ());

        ErrInfo+="dXML created...";

        dXML->XML->Clear();
        dXML->Active = true;
        dXML->StandAlone = "yes";
        dXML->Encoding = "windows-1251";
        dXML->Options = dXML->Options << doNodeAutoIndent;

        ErrInfo+="dXML params set...";

        xmlGuard <_di_IXMLNode> Root (dXML->AddChild("root"));
        xmlGuard <_di_IXMLNodeList> RootNL (Root->GetChildNodes());
        xmlGuard <_di_IXMLNode> FieldsNode (Root->AddChild("fields"));

        SetValue("processor_type",ProcessorType,Root);
        SetValue("operator_id",OperatorId,Root);

        SetValue("initial_session_num",InitialSessionNum,Root);

        if ((!NoUpdate)&&(LastSession!=NULL))
            SetValue("last_session",LastSession,Root);

        SetValue("transid",TransId,Root);

        SetValue("status",Status,Root);
        SetValue("last_error_code",LastErrorCode,Root);
        SetValue("cancelled",int(Cancelled),Root);

        SetValue("payment_create_dt",AnsiString(PaymentCreateDT), Root);
        SetValue("first_try_dt",AnsiString(FirstTryDT), Root);
        SetValue("next_try_dt",AnsiString(NextTryDT), Root);

        SetValue("num_of_tries",NumOfTries,Root);

        StoreNotes(vNotes, Root);

        SetValue("saved_data", SavedData, Root);
        SetValue("saved_card_string", SavedCardString, Root);

        if (ResultExternal!=-1)
            SetValue("result_external",ResultExternal,Root);

        SetValue("last_payment_error_code",LastPaymentErrorCode,Root);

        ErrInfo+="XML partially stored...";

        ErrInfo+="PARAMS count = "+AnsiString(Params->Count)+"...";

        for (int i=0;i<this->Params->Count;i++)
        {
            Parameter=(PParams)Params->Items[i];
            SetValue(Parameter->Name,Parameter->Value,FieldsNode);
            ErrInfo+="PARAM "+Parameter->Name+" stored...";
        }

        AnsiString Temp;
        dXML->SaveToXML(Temp);
        ErrInfo+="XML saved to string...";
        bResult = SaveFile(FileName,Temp, NoUpdate);
        ErrInfo+="XML saved to file.";

        PacketFileName=FileName.c_str();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Log->Write((boost::format("Error saving payment file %1%") % TruncateFileName(FileName).c_str()).str().c_str());
        Log->Write((boost::format("ErrInfo: %1%") % ErrInfo.c_str()).str().c_str());
        bResult = false;
    }
    return bResult;
}

//---------------------------------------------------------------------------

bool TPaymentPacket::SaveToFile()
{
    if (PacketFileName!="")
        return StoreToFile(PacketFileName.c_str(), false);
    else
        return StoreToFile(GetNewFileName("pkt"), false);
}

//---------------------------------------------------------------------------

bool TPaymentPacket::SaveToTempFile()
{
    if (PacketFileName!="")
        return StoreToFile(PacketFileName.c_str(),true);
    else
        return StoreToFile(GetNewFileName("tmp"),true);
}

//---------------------------------------------------------------------------

AnsiString TPaymentPacket::GetNewFileName(AnsiString AFileExt, AnsiString AFilePrefix)
{
    AnsiString AFileDir;
    AnsiString AFileName;
    AFileDir=(OutboundDir+"\\").c_str();
    AFileName=AFilePrefix+"-"+InitialSessionNum;
    while(FileExists(AFileDir+AFileName+"."+AFileExt))
    {
        AFileName+="~";
    }
    return AFileDir+AFileName+"."+AFileExt;
}

//---------------------------------------------------------------------------

bool TPaymentPacket::GetDataFromXML(xmlGuard <_di_IXMLDocument> &XMLDoc)
{
    try
    {
        //_di_IXMLNode xmlNode, xmlSecNode;
        //_di_IXMLNodeList xmlRootNodeList, xmlNodeList;

        xmlGuard <_di_IXMLNode> RootNode (XMLDoc->GetDocumentElement());
        if (RootNode->HasChildNodes)
        {
            xmlGuard <_di_IXMLNodeList> xmlRootNodeList (RootNode->GetChildNodes());

            ProcessorType = GetString("processor_type",xmlRootNodeList);

            OperatorId=GetInt(GetString("operator_id",xmlRootNodeList));

            LastSession=GetString("last_session",xmlRootNodeList);

            InitialSessionNum=GetString("initial_session_num",xmlRootNodeList);

            TransId=GetString("transid",xmlRootNodeList);

            Status=GetInt(GetString("status",xmlRootNodeList));
            LastErrorCode=GetInt(GetString("last_error_code",xmlRootNodeList));
            Cancelled=GetInt(GetString("cancelled",xmlRootNodeList));

            FirstTryDT=GetDateTime("first_try_dt",xmlRootNodeList);
            NextTryDT=GetDateTime("next_try_dt",xmlRootNodeList);
            if (IsExist("payment_create_dt",xmlRootNodeList))
                PaymentCreateDT=GetDateTime("payment_create_dt",xmlRootNodeList);
            else
                PaymentCreateDT=FirstTryDT;

            NumOfTries=GetInt(GetString("num_of_tries",xmlRootNodeList));

            GetNotes(vNotes, xmlRootNodeList);

            if (IsExist("result_external",xmlRootNodeList))
                    ResultExternal=GetInt(GetString("result_external",xmlRootNodeList));
                    else
                    ResultExternal=-1;

            xmlGuard <_di_IXMLNode> xmlSecNode (xmlRootNodeList->FindNode("fields"));
            if ((xmlSecNode.Assigned())&&(xmlSecNode->HasChildNodes))
            {
                xmlGuard <_di_IXMLNodeList> xmlNodeList (xmlSecNode->GetChildNodes());
                for (int i=0; i<xmlNodeList->Count; i++)
                {
                    xmlGuard <_di_IXMLNode> xmlNode (xmlNodeList->Get(i));
                    if (xmlNode->IsTextElement)
//          Parameters->Add(xmlNode->NodeName+"="+AnsiString(xmlNode->NodeValue));
                        AddParam(xmlNode->NodeName,AnsiString(xmlNode->NodeValue));
                    else
//          Parameters->Add(xmlNode->NodeName+"=");
                        AddParam(xmlNode->NodeName,"");
                }
            }
            SavedData = GetString("saved_data",xmlRootNodeList);
            SavedCardString = GetString("saved_card_string",xmlRootNodeList);
            if (IsExist("last_payment_error_code",xmlRootNodeList))
                LastPaymentErrorCode = GetInt(GetString("last_payment_error_code",xmlRootNodeList));
            else
                LastPaymentErrorCode = -2;
        }
        else
        {
            Log->Write((boost::format("File %1% does not have root entry") % PacketFileName.c_str()).str().c_str());
            return false;
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------
//                          TStatPacket
//---------------------------------------------------------------------------

TStatPacket::TStatPacket(TWConfig *Cfg, TLogClass *_Log) : TXMLPacket(Cfg, _Log)
{
    OutboundDir=Cfg->Dirs.StatOutbound.c_str();
    TempOutboundDir=Cfg->Dirs.StatOutboundTemp.c_str();
}

TStatPacket::~TStatPacket()
{
}

//---------------------------------------------------------------------------

void TStatPacket::Clear()
{
    try
    {
        TXMLPacket::Clear();
        PacketType=0;
        Status=0;
        TerminalID=0;
        EventDateTime.Val=0;
        InitDT.Val=0;
        OperatorID=-1;
        SessionNum="";
        InitialSessionNum="";
        Comission=0;
        ErrorCode=0;
        ErrSender="";
        ErrType=0;
        ChequeCounter=0;
        ErrDescription="";
        ErrSubType=0;
        ErrSubDescription="";
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

bool TStatPacket::StoreToFile(AnsiString FileName, bool NoUpdate)
{
    UNREFERENCED_PARAMETER(NoUpdate);
    bool bResult = false;
    try
    {
        PParams Parameter;

        xmlGuard <_di_IXMLDocument> dXML (NewXMLDocument ());
        dXML->XML->Clear();
        dXML->Active = true;
        dXML->StandAlone = "yes";
        dXML->Encoding = "windows-1251";
        dXML->Options = dXML->Options << doNodeAutoIndent;
        xmlGuard <_di_IXMLNode> Root (dXML->AddChild("root"));
        xmlGuard <_di_IXMLNode> FieldsNode (Root->AddChild("fields"));
        switch (PacketType)
        {
            case cnPaymentInit:
                SetValue("packet_type", PacketType, Root);
                SetValue("terminal_id", TerminalID, Root);
                SetValue("event_date_time", AnsiString(EventDateTime), Root);
                SetValue("operator_id", OperatorID, Root);
                SetValue("session", SessionNum, Root);
                SetValue("initial_session", InitialSessionNum, Root);
                SetValue("comission", Comission, Root);
                StoreNotes(vNotes, Root);
                for (int i=0;i<Params->Count;i++)
                {
                    Parameter=(PParams)Params->Items[i];
                    SetValue(Parameter->Name,Parameter->Value,FieldsNode);
                }
                break;
            case cnIncassation:
                SetValue("packet_type", PacketType, Root);
                SetValue("terminal_id", TerminalID, Root);
                SetValue("event_date_time", AnsiString(EventDateTime), Root);
                SetValue("comission", Comission, Root);
                SetValue("session", SessionNum, Root);
                StoreNotes(vNotes, Root);
                break;
            case cnError:
                SetValue("packet_type", PacketType, Root);
                SetValue("terminal_id", TerminalID, Root);
                SetValue("event_date_time", AnsiString(EventDateTime), Root);
                SetValue("sender_name", ErrSender, Root);
                SetValue("error_type", ErrType, Root);
                SetValue("error_type_description", ErrDescription, Root);
                SetValue("error_subtype", ErrSubType, Root);
                SetValue("error_subtype_description", ErrSubDescription, Root);
                break;
            case cnPaymentStatusChange:
                SetValue("packet_type", PacketType, Root);
                SetValue("terminal_id", TerminalID, Root);
                SetValue("event_date_time", AnsiString(EventDateTime), Root);
                SetValue("initial_session", InitialSessionNum, Root);
                SetValue("status", Status, Root);
                SetValue("error_code", ErrorCode, Root);
                break;
            case cnPaymentComplete:
                SetValue("packet_type", PacketType, Root);
                SetValue("terminal_id", TerminalID, Root);
                SetValue("event_date_time", AnsiString(EventDateTime).c_str(), Root);
                SetValue("initial_session", InitialSessionNum, Root);
                SetValue("session", SessionNum, Root);
                SetValue("error_code", ErrorCode, Root);
                SetValue("init_dt", AnsiString(InitDT), Root);
                for (int i=0;i<Params->Count;i++)
                {
                    Parameter=(PParams)Params->Items[i];
                    SetValue(Parameter->Name,Parameter->Value,FieldsNode);
                }
                break;
            case cnCommandProcessed:
                SetValue("packet_type", PacketType, Root);
                SetValue("terminal_id", TerminalID, Root);
                SetValue("status", Status, Root);
                break;
            case cnFileSend:
            case cnFileSendNew:
                SetValue("packet_type", PacketType, Root);
                SetValue("terminal_id", TerminalID, Root);
                SetValue("filename", SendFileName, Root);
                break;
            default:
                Log->Write((boost::format("Unknown packet type: %1%") % PacketType).str().c_str());
                break;
        }

        AnsiString Temp;
        dXML->SaveToXML(Temp);
        bResult = SaveFile(FileName,Temp);
        PacketFileName=FileName.c_str();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Log->Write((boost::format("Error saving stat file %1%") % TruncateFileName(FileName).c_str()).str().c_str());
        bResult =  false;
    }
    return bResult;
}

//---------------------------------------------------------------------------

bool TStatPacket::SaveToFile()
{
    if (PacketFileName!="")
        return StoreToFile(PacketFileName.c_str(), false);
    else
        return StoreToFile(GetNewFileName("pkt"), false);
}

//---------------------------------------------------------------------------

bool TStatPacket::SaveToTempFile()
{
    if (PacketFileName!="")
        return StoreToFile(PacketFileName.c_str(),true);
    else
        return StoreToFile(GetNewFileName("tmp"),true);
}

//---------------------------------------------------------------------------

bool TStatPacket::GetDataFromXML(xmlGuard <_di_IXMLDocument> &XMLDoc)
{
    try
    {
        xmlGuard <_di_IXMLNode> RootNode (XMLDoc->GetDocumentElement());
        if (RootNode->HasChildNodes)
        {
            xmlGuard <_di_IXMLNodeList> xmlRootNodeList (RootNode->GetChildNodes());

            PacketType=GetInt(GetString("packet_type",xmlRootNodeList));
            Status=GetInt(GetString("status",xmlRootNodeList));

            TerminalID=GetInt(GetString("terminal_id",xmlRootNodeList));

            try
            {
                EventDateTime=TDateTime(GetString("event_date_time",xmlRootNodeList));
            }
            catch (...)
            {
                EventDateTime=EventDateTime.CurrentDateTime();
            }

            try
            {
                InitDT=TDateTime(GetString("init_dt",xmlRootNodeList));
            }
            catch (...)
            {
                InitDT.Val=0;
            }

            OperatorID=GetInt(GetString("operator_id",xmlRootNodeList));
            SessionNum=GetString("session",xmlRootNodeList);
            InitialSessionNum=GetString("initial_session",xmlRootNodeList);
            Comission=GetDouble(GetString("comission",xmlRootNodeList));

            //Notes = new TList();
            GetNotes(vNotes, xmlRootNodeList);

            ErrorCode=GetInt(GetString("error_code",xmlRootNodeList));
            ErrSender=GetString("sender_name",xmlRootNodeList);

            ErrType=GetInt(GetString("error_type",xmlRootNodeList));
            ErrDescription=GetString("error_type_description",xmlRootNodeList);
            ErrSubType=GetInt(GetString("error_subtype",xmlRootNodeList));
            ErrSubDescription=GetString("error_subtype_description",xmlRootNodeList);

            SendFileName=GetString("filename",xmlRootNodeList);

            xmlGuard <_di_IXMLNode> xmlSecNode (xmlRootNodeList->FindNode("fields"));
            if ((xmlSecNode.Assigned())&&(xmlSecNode->HasChildNodes))
            {
                xmlGuard <_di_IXMLNodeList> xmlNodeList (xmlSecNode->GetChildNodes());
                for (int i=0; i<xmlNodeList->Count; i++)
                {
                    xmlGuard <_di_IXMLNode> xmlNode (xmlNodeList->Get(i));
                    if (xmlNode->IsTextElement)
                        AddParam(xmlNode->NodeName,AnsiString(xmlNode->NodeValue));
                    else
                        AddParam(xmlNode->NodeName,"");
                }
            }
        }
        else
        {
            Log->Write((boost::format("File %1% does not have root entry") % PacketFileName.c_str()).str().c_str());
            return false;
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------

TEMailPacket::TEMailPacket(TWConfig *Cfg, TLogClass *_Log) : TXMLPacket(Cfg, _Log)
{
    try
    {
        OutboundDir=Cfg->Dirs.EMailOutbound.c_str();
        TempOutboundDir=Cfg->Dirs.EMailOutboundTemp.c_str();
        //Log->Write("TPaymentPacket initialized.");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

TEMailPacket::~TEMailPacket()
{
}

//---------------------------------------------------------------------------
//                          TSMSPacket
//---------------------------------------------------------------------------

TSMSPacket::TSMSPacket(TWConfig *Cfg, TLogClass *_Log) : TStatPacket(Cfg, _Log)
{
    try
    {
        OutboundDir=Cfg->Dirs.SMSOutbound.c_str();
        TempOutboundDir=Cfg->Dirs.SMSOutboundTemp.c_str();
        //Log->Write("TStatPacket initialized.");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

TSMSPacket::~TSMSPacket()
{
}
//---------------------------------------------------------------------------

_filesProperties::_filesProperties(int a_packet_type, std::string a_filesTypeName, TWConfig* a_m_pCfg, TLogClass* a_m_pLog):
    filesTypeName(a_filesTypeName)
{
    count = 0;
    undeleted = 0;
    packet_type = a_packet_type;
    m_pCfg = a_m_pCfg;
    m_pLog = a_m_pLog;
}

//---------------------------------------------------------------------------

int _filesProperties::includeNode(std::string a_name, std::string a_value)
{
    boost::trim(a_name);
    boost::trim(a_value);
    boost::to_lower(a_name);
    boost::to_lower(a_value);
    m_nodes.push_back(_spec_packet_node(a_name, a_value));

    return m_nodes.size() - 1;
}

//---------------------------------------------------------------------------

bool _filesProperties::isValidType(int a_packet_type)
{
    return a_packet_type == packet_type;
}

//---------------------------------------------------------------------------

bool _filesProperties::isLastFile(std::string& a_file_name)
{
    BOOST_FOREACH(_spec_packet_node node, m_nodes)
    {
        if (node.lastFileName == a_file_name)
            return true;
    }

    return false;
}

//---------------------------------------------------------------------------

bool _filesProperties::isValidValue(std::string a_value)
{
    boost::trim(a_value);
    boost::to_lower(a_value);

    BOOST_FOREACH(_spec_packet_node node, m_nodes)
    {
        if (a_value.find(node.value) != std::string::npos)
            return true;
    }

    return false;
}

//---------------------------------------------------------------------------

void _filesProperties::getLastFileName(AnsiString a_work_directory, int a_nodeIndex)
{
    if(!m_pCfg)
        return;

    try
    {
        TSearchRec sr;
        int iAttributes = 0;
        TDateTime lastDateTime = 0;

        if (FindFirst(a_work_directory + "\\*.pkt", iAttributes, sr) == 0)
        {
            do
            {
                AnsiString pktFileName = a_work_directory + "\\" + sr.Name;
                std::auto_ptr<TStatPacket> packet (new TStatPacket(m_pCfg, m_pLog));

                xmlGuard <_di_IXMLDocument> XmlDoc;
                xmlGuard <_di_IXMLNode> RootNode;

                std::auto_ptr<TFileStream> pktFile (new TFileStream(pktFileName, fmOpenReadWrite | fmShareDenyNone));
                bool OpenFileResult = false;
                if (pktFile.get())
                {
                    OpenFileResult = true;
                }
                else
                {
                    if(m_pLog)
                        m_pLog->Write((boost::format("Open file error %1%!") % pktFileName.c_str()).str().c_str());
                }

                if (OpenFileResult)
                {
                    std::vector<char> Buffer(static_cast<std::size_t>(pktFile->Size)+1, 0);
                    pktFile->Seek(0, soFromBeginning);
                    pktFile->Read(&*Buffer.begin(), static_cast<int>(pktFile->Size));
                    AnsiString FileData = AnsiString(&*Buffer.begin(), static_cast<int>(pktFile->Size));

                    bool bResult = false;
                    try
                    {
                        try
                        {
                            XmlDoc = LoadXMLData(FileData);
                            bResult = true;
                        }
                        catch(...)
                        {
                            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
                        }
                    }
                    catch (EDOMParseError &e)
                    {
                        if(m_pLog)
                            m_pLog->Write((boost::format("EDOMParseError exception encountered: %1%") % e.Message.c_str()).str().c_str());
                    }
                    catch (...)
                    {
                        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
                    }

                    if (bResult)
                    {
                        RootNode = XmlDoc->GetDocumentElement();
                        int pkt_packet_type = m_pCfg->GetInt(false, "packet_type", RootNode, cnCmdNone);
                        TDateTime modified = FileDateToDateTime(sr.Time);

                        if ((FileDateToDateTime(sr.Time) > lastDateTime) &&
                            (pkt_packet_type == packet_type) &&
                            m_pCfg->IsExist(m_nodes[a_nodeIndex].name.c_str(), RootNode))
                        {
                            std::string current_node_value = m_pCfg->GetChildNodeValue(false, m_nodes[a_nodeIndex].name.c_str(), RootNode, "").c_str();
                            boost::trim(current_node_value);
                            boost::to_lower(current_node_value);
                            if (current_node_value.find(m_nodes[a_nodeIndex].value) != std::string::npos)
                            {
                                m_nodes[a_nodeIndex].lastFileName = sr.Name.c_str();
                                lastDateTime = FileDateToDateTime(sr.Time);
                            }
                        }
                    }
                }
            }
            while (FindNext(sr) == 0);
            FindClose(sr);
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
    }
}

//---------------------------------------------------------------------------

void _filesProperties::prepareToDeletingFile(AnsiString& FileName, TStatPacket* packet, char* unconditionalExtention)
{
    int packet_type = packet ? packet->PacketType : cnNoType;

    if (packet_type || (GetExtName(FileName.c_str()) == unconditionalExtention))
    {
        count++;

        if (!isLastFile(StripFileName(FileName.c_str())))
            m_deletedFiles.push_back(FileName.c_str());
    }
}

//---------------------------------------------------------------------------

int _filesProperties::deletingFiles()
{
    try
    {
        int deletingFilesFailCount = 0;

        for(int i  = 0; i < m_deletedFiles.size(); i++) //если использовать BOOST_FOREACH - не компилится, F1002 - fatal error
        {
            deletingFilesFailCount += static_cast<int>(!DeleteFile(m_deletedFiles[i].c_str()));
        }

        int deletedFilesCount = m_deletedFiles.size() - deletingFilesFailCount;
        undeleted = count - deletedFilesCount;

        return deletedFilesCount;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, m_pLog);
    }
}

//---------------------------------------------------------------------------

std::string getDeleteStatFilesString(_filesProperties* CFilesProperties, int a_count, int a_deleted)
{
    int _count   = CFilesProperties ?  CFilesProperties->count : a_count;
    int _deleted = CFilesProperties ? (CFilesProperties->count - CFilesProperties->undeleted): a_deleted;

    int alignNameValue = 28;    //название файлов
    std::string _filesType = std::string(alignNameValue + 1, ' ');
    if (CFilesProperties)
         _filesType = fill(CFilesProperties->filesTypeName, " ", alignNameValue, SIDE::LEFT) + ":";
    else if (a_count)
         _filesType = fill("Total:", " ", alignNameValue + 1, SIDE::LEFT);

    return (boost::format("\n%1% %2% found, %3% deleted")
        % _filesType
        % fill(_count, " ", 3, SIDE::RIGHT)
        % fill(_deleted, " ", 3, SIDE::RIGHT)).str();
}
