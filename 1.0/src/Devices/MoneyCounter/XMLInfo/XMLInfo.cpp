//---------------------------------------------------------------------------

#include <XMLDoc.hpp>
#include <vector>
#pragma hdrstop

#include "XMLInfo.h"
#include "globals.h"
#include "boost/format.hpp"
#pragma package(smart_init)

void TXMLInfo::InitFile()
{
    try
    {
        //CoInitializeEx(NULL, COINIT_MULTITHREADED);
        Enabled = false;
        if (FileName=="")
            FileName="details.xml";
        if (FileExists(FileName)) // details.xml существует
        {
            InfoFile = new TFileStream(FileName, fmOpenReadWrite | fmShareDenyNone);
            if (InfoFile == NULL)
            {
                Log->Write((boost::format("Open file error %1%!") % FileName.c_str()).str().c_str());
                return;
            }
            std::vector<char> Buffer( InfoFile->Size+1, 0 );
            InfoFile->Seek(0, soFromBeginning);
            InfoFile->Read(&*Buffer.begin(), InfoFile->Size);
            AnsiString FileData = AnsiString(&*Buffer.begin(), InfoFile->Size);
            try
            {
                try
                {
                    dXML = LoadXMLData(FileData);
                }
                catch(...)// не смогли распарсить содержимое файла
                {
                    ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                    AnsiString NewFileData = FileData.SubString(0,FileData.Pos("</root>")+6);
                    dXML = LoadXMLData(NewFileData);
                }
            }
            catch(...)// не смогли распарсить содержимое файла
            {
                ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                Log->Write("Exception occured in TXMLInfo::InitFile:LoadXMLData");
                Log->Write((boost::format("Details.xml contents: \n%1%\n") % FileData.c_str()).str().c_str());

                if (InfoFile)
                {
                    delete InfoFile;
                    InfoFile = NULL;
                }
                TDateTime DT = DT.CurrentDateTime();
                if (FileExists(FileName+AnsiString(DT)))
                    DeleteFile(FileName+AnsiString(DT));
                if (RenameFile(FileName,FileName+DT.FormatString("-yymmddhhnnss")))
                {
                    Log->Write((boost::format("%1% renamed to %2%.") % FileName.c_str() % (FileName + DT.FormatString("-yymmddhhnnss")).c_str()).str().c_str());
                }
                else
                {
                    Log->Write((boost::format("Error renaming %1% to %2%!") % FileName.c_str() % (FileName + DT.FormatString("-yymmddhhnnss")).c_str()).str().c_str());
                    if (FileExists(FileName))
                        DeleteFile(FileName);
                }
                InfoFile = new TFileStream(FileName, fmCreate);
                if (InfoFile)
                {
                    delete InfoFile;
                    InfoFile = NULL;
                }
                InfoFile = new TFileStream(FileName, fmOpenReadWrite | fmShareDenyNone);
                Log->Append("OK.");
                dXML = NewXMLDocument ();
                dXML->XML->Clear();
                dXML->Active = true;
                dXML->NodeIndentStr="        ";
                dXML->StandAlone = "yes";
                dXML->Encoding = "windows-1251";
                dXML->Options = dXML->Options << doNodeAutoIndent;
                Root = dXML->AddChild("root");
            }
            //dXML = LoadXMLDocument(FileName);
            dXML->NodeIndentStr="        ";
            dXML->Options = dXML->Options << doNodeAutoIndent;
            Root = dXML->GetDocumentElement();
            if (!Root.Assigned())
                Root = dXML->AddChild("root");
        }
        else
        {                                                                         // details.xml не существует, создаем новый...
            Log->Write((boost::format("File %1% does not exists, trying to create...") % FileName.c_str()).str().c_str());
            InfoFile = new TFileStream(FileName, fmCreate);
            if (InfoFile)
            {
                delete InfoFile;
                InfoFile = NULL;
            }
            InfoFile = new TFileStream(FileName, fmOpenReadWrite | fmShareDenyNone);
            Log->Append("OK.");
            dXML = NewXMLDocument ();
            dXML->XML->Clear();
            dXML->Active = true;
            dXML->NodeIndentStr="        ";
            dXML->StandAlone = "yes";
            dXML->Encoding = "windows-1251";
            dXML->Options = dXML->Options << doNodeAutoIndent;
            Root = dXML->AddChild("root");
        }
        if ((dXML.Assigned())&&(Root.Assigned()))
        {
            Enabled=true;
//        Log->Write("Status file initialized.");
        }
        else
        {
            Enabled=false;
            Log->Write("Error initializing status file.");
        }
        _FileSystemError = false;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Enabled=false;
    }
}

void TXMLInfo::reopen()
{
    if(InfoFile)
    {
        delete InfoFile;
        InfoFile = NULL;
    }
    InitFile();
}

//---------------------------------------------------------------------------

TXMLInfo::TXMLInfo(AnsiString _FileName, TLogClass *_Log)
{
try
	{
        //CoInitializeEx(NULL, COINIT_MULTITHREADED);
	InnerLog=false;
	if (_Log==NULL)
		{
		Log = new TLogClass("XMLInfo");
		InnerLog=true;
		}
		else
		Log=_Log;
	FileName=_FileName;
	CS = NULL;
	CS = new TCriticalSection();
	InitFile();
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Enabled=false;
    }
}

//---------------------------------------------------------------------------

TXMLInfo::~TXMLInfo()
{
try
	{
//    Log->Write("TXMLInfo done.");
	if (CS!=NULL)
		{
		CS->Acquire();
		CS->Release();
		delete CS;
		CS = NULL;
		}
//	if (dXML!=NULL)
//		dXML.Release();
	if (InfoFile)
		{
		delete InfoFile;
		InfoFile = NULL;
		}
	if (InnerLog)
        {
            delete Log;
            Log=NULL;
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

bool TXMLInfo::Write(AnsiString ObjectName, AnsiString KeyName,AnsiString KeyValue)
{
bool bRes = false;
CS->Acquire();
//Log->Write("CS Acquired Write");
try
	{
	try
		{
		if (Enabled)
			{
			xmlGuard <_di_IXMLNode> KeyNode (GetNode(ObjectName, KeyName));
			if (KeyNode.Assigned())
					KeyNode->Text=KeyValue;
		//		dXML->SaveToFile(WideString(FileName));
			AnsiString Temp;
			dXML->SaveToXML(Temp);
			if (!SaveFile(FileName,Temp))
				{
				Log->Write("Error writing InfoFileto disk!");
				_FileSystemError = true;
				}
				else
				bRes = true;
			}
		}
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		_FileSystemError = true;
		bRes = false;
		}
	}
__finally
	{
	CS->Release();
	//Log->Write("CS Released Write");
	return bRes;
	}
}

//---------------------------------------------------------------------------

AnsiString TXMLInfo::Read(AnsiString ObjectName, AnsiString KeyName)
{
try
		{
		if (!Enabled)
				return NULL;
                _di_IXMLNode tmpNode=GetNode(ObjectName, KeyName);
		xmlGuard <_di_IXMLNode> KeyNode (tmpNode);
		if ((KeyNode.Assigned())&&(KeyNode->IsTextElement))
				return KeyNode->Text;
		return "";
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		return "";
		}
}

//---------------------------------------------------------------------------

double TXMLInfo::ReadDouble(AnsiString ObjectName, AnsiString KeyName)
{
try
		{
		if (!Enabled)
				return 0;
		return AnsiString(Read(ObjectName, KeyName)).ToDouble();
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		return 0;
		}
}

//---------------------------------------------------------------------------

TDateTime TXMLInfo::ReadDateTime(AnsiString ObjectName, AnsiString KeyName)
{
try
		{
		if (!Enabled)
				return 0;
		AnsiString Data = Read(ObjectName, KeyName);
		if (Data!="")
		  return TDateTime(Data);
        else
          return 0;
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		return 0;
		}
}

//---------------------------------------------------------------------------

_di_IXMLNode TXMLInfo::GetNode(AnsiString ObjectName, AnsiString KeyName)
{
  try
  {
		if (!Enabled)
			return NULL;

    xmlGuard <_di_IXMLNode> ObjectND;

		xmlGuard <_di_IXMLNodeList> RootNDL (Root->GetChildNodes());
		ObjectND = RootNDL->FindNode(ObjectName);
		if (!ObjectND.Assigned())
    {
			Log->Write((boost::format("Can't find %1% node, creating...") % ObjectName.c_str()).str().c_str());
		  ObjectND=Root->AddChild(ObjectName);
		}

    _di_IXMLNode KeyND = NULL;
    xmlGuard <_di_IXMLNodeList> ObjectNDL (ObjectND->GetChildNodes());
    KeyND = ObjectNDL->FindNode(KeyName);
    if (KeyND==NULL)
    {
      Log->Write((boost::format("Can't find %1% node, creating...") % KeyName.c_str()).str().c_str());
      KeyND = ObjectND->AddChild(KeyName);
    }

//    dXML->SaveToFile(WideString(FileName));
		return KeyND;
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
	return NULL;
	}
}

//---------------------------------------------------------------------------

void TXMLInfo::ReadNotes(AnsiString ObjectName, TNotesVector& Notes)
{
    try
    {
        if (!Enabled)
            return;
        Notes.clear();
        if (Read(ObjectName,"LastIncassation")=="")
            Write(ObjectName,"LastIncassation",TDateTime::CurrentDateTime());

        xmlGuard <_di_IXMLNode> NotesNode (GetNode(ObjectName, "Notes"));
        if ((NotesNode.Assigned())&&(NotesNode->HasChildNodes))
        {
            xmlGuard <_di_IXMLNodeList> NotesChildNDL (NotesNode->GetChildNodes());
            for (int i=0;i<NotesChildNDL->Count;i++)
            {
                xmlGuard <_di_IXMLNode> NoteNode (NotesChildNDL->Get(i));
                if (NoteNode->NodeName==AnsiString("note"))
                {
                    int nominal = WCharToInt((NoteNode->GetAttribute("nominal")).VOleStr);
                    int count = WCharToInt((NoteNode->GetAttribute("count")).VOleStr);
                    Log->Write((boost::format("Got note info: %1% %2% %3%") % i % nominal % count).str().c_str());
                    Notes.push_back(TNote(nominal, count));
                }
            }
        }
        // dXML->SaveToFile(WideString(FileName));
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return;
    }
}

//---------------------------------------------------------------------------

void TXMLInfo::WriteNotes(AnsiString ObjectName, TNotesVector& Notes)
{
    CS->Acquire();
    //Log->Write("CS Acquired WriteNotes");
    try
    {
        try
        {
            if (Enabled)
            {
                if (Read(ObjectName,"LastIncassation")=="")
                    Write(ObjectName,"LastIncassation",TDateTime::CurrentDateTime());

                xmlGuard <_di_IXMLNode> NotesNode (GetNode(ObjectName, "Notes"));
                if ((NotesNode.Assigned())&&(NotesNode->HasChildNodes))
                {
                    xmlGuard <_di_IXMLNodeList> NotesChildNDL (NotesNode->GetChildNodes());
                    while (NotesChildNDL->Count>0)
                        NotesChildNDL->Delete(0);
                    /*        NotesChildNDL->BeginUpdate();
                            NotesChildNDL->Clear();
                            NotesChildNDL->EndUpdate();*/
                }
                else
                {
                    Log->Write("Error deleting notes!");
                }

                for (int i = 0; i < Notes.size(); i++)
                {
                    _di_IXMLNode NoteNode = NotesNode->AddChild("note");
                    int _nominal = Notes[i].Nominal;
                    int _count = Notes[i].Count;
                    NoteNode->SetAttribute("nominal", _nominal);
                    NoteNode->SetAttribute("count", _count);
                    Log->Write((boost::format("Nominal written: %1% : %2%") % _nominal % _count).str().c_str());
                }
                AnsiString Temp;
                dXML->SaveToXML(Temp);
                Log->Write((boost::format("Tgt: {%1%}.") % Temp.c_str()).str().c_str());
                if (!SaveFile(FileName,Temp))
                {
                    Log->Write("Error writing InfoFileto disk!");
                    _FileSystemError = true;
                }
            }
        }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            _FileSystemError = true;
        }
    }
    __finally
    {
        CS->Release();
        //Log->Write("CS Released WriteNotes");
    }
}

//---------------------------------------------------------------------------

void TXMLInfo::WriteNote(AnsiString ObjectName, AnsiString Nominal, AnsiString Count)
{
    CS->Acquire();
    //Log->Write("CS Acquired WriteNote");
    try
    {
        try
        {
            if (Enabled)
            {
                bool bFound = false;
                if (Read(ObjectName,"LastIncassation")=="")
                    Write(ObjectName,"LastIncassation",TDateTime::CurrentDateTime());

                _di_IXMLNode tmpNode=GetNode(ObjectName, "Notes");
                xmlGuard <_di_IXMLNode> NotesNode=tmpNode;
                if ((NotesNode.Assigned())&&(NotesNode->HasChildNodes))
                {
                    xmlGuard <_di_IXMLNodeList> NotesChildNDL (NotesNode->GetChildNodes());
                    for (int i=0;i<NotesChildNDL->Count;i++)
                    {
                        xmlGuard <_di_IXMLNode> NoteNode (NotesChildNDL->Get(i));
                        if (NoteNode->HasAttribute("nominal"))
                        {
                            if (NoteNode->GetAttribute("nominal")==Nominal)
                            {
                                NoteNode->SetAttribute("count",Count);
                                bFound = true;
                                break;
                            }
                        }
                    }
                }
                if (!bFound)
                {
                    xmlGuard <_di_IXMLNode> NewNoteNode (NotesNode->AddChild("note"));
                    NewNoteNode->SetAttribute("nominal",Nominal);
                    NewNoteNode->SetAttribute("count",Count);
                }
                AnsiString Temp;
                dXML->SaveToXML(Temp);
                if (!SaveFile(FileName,Temp))
                {
                    Log->Write("Error writing InfoFile to disk!");
                    _FileSystemError = true;
                }
            }
        }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            _FileSystemError = true;
        }
    }
    __finally
    {
        CS->Release();
	//Log->Write("CS Released WriteNote");
    }
}

//---------------------------------------------------------------------------

void TXMLInfo::WriteIncassation(AnsiString ObjectName)
{
    CS->Acquire();
    //Log->Write("CS Acquired WriteIncassation");
    try
    {
        if (Enabled)
        {
            TDateTime DT;
            Write(ObjectName,"LastIncassation",DT.CurrentDateTime());
            xmlGuard <_di_IXMLNode> NotesNode (GetNode(ObjectName, "Notes"));
            if ((NotesNode.Assigned())&&(NotesNode->HasChildNodes))
            {
                xmlGuard <_di_IXMLNodeList> NotesChildNDL (NotesNode->GetChildNodes());
                while (NotesChildNDL->Count>0)
                    NotesChildNDL->Delete(0);
            }
            else
            {
                Log->Write("Error deleting notes!");
            }
            AnsiString Temp;
            dXML->SaveToXML(Temp);
            if (!SaveFile(FileName,Temp))
            {
                Log->Write("Error writing InfoFileto disk!");
                _FileSystemError = true;
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        _FileSystemError = true;
    }
    CS->Release();
    //Log->Write("CS Released WriteIncassation");
}

//---------------------------------------------------------------------------

TDateTime TXMLInfo::GetIncassationDT(AnsiString ObjectName)
{
try
		{
		if (!Enabled)
				return 0;
		AnsiString Temp = Read(ObjectName, "LastIncassation");
		if (Temp=="") {
				TDateTime DT = TDateTime::CurrentDateTime();
				Write(ObjectName,"LastIncassation",DT);
				return DT;
				}
		return TDateTime(Temp);
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return 0;
    }
}

//---------------------------------------------------------------------------
int TXMLInfo::GetPaymentsInfoCount(AnsiString ObjectName)
{
//Log->Write("GetPaymentsInfoCount("+ObjectName+")");
try
  {
  if (!Enabled)
      return 0;
  xmlGuard <_di_IXMLNode> PaymentsNode (GetNode(ObjectName, "PaymentsInfo"));
  if ((PaymentsNode.Assigned())&&(PaymentsNode->HasChildNodes)) {
     xmlGuard <_di_IXMLNodeList> PaymentsChildNDL (PaymentsNode->GetChildNodes());
     return PaymentsChildNDL->Count;
     }
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return 0;
}

void TXMLInfo::ReadPaymentsInfo(AnsiString ObjectName, TStringList *PaymentsInfo)
{
//Log->Write("ReadPaymentsInfo("+ObjectName+")");
try
		{
		if (!Enabled)
				return;
		PaymentsInfo->Clear();
		xmlGuard <_di_IXMLNode> PaymentsNode (GetNode(ObjectName, "PaymentsInfo"));
		if ((PaymentsNode.Assigned())&&(PaymentsNode->HasChildNodes)) {
				xmlGuard <_di_IXMLNodeList> PaymentsChildNDL (PaymentsNode->GetChildNodes());
				for (int i=0;i<PaymentsChildNDL->Count;i++) {
						xmlGuard <_di_IXMLNode> PaymentNode (PaymentsChildNDL->Get(i));
						if (PaymentNode->NodeName==AnsiString("PaymentInfo"))
              {
							AnsiString Sum=(PaymentNode->GetAttribute("sum")).VOleStr;
							AnsiString Info=(PaymentNode->GetAttribute("info")).VOleStr;
							PaymentsInfo->Add(AnsiString(Info+"="+Sum));
							}
						}
				}
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------
void TXMLInfo::ClearPaymentInfo(AnsiString ObjectName,AnsiString Info, AnsiString Sum)
{
try
  {
  if (!Enabled)
    return;
  xmlGuard <_di_IXMLNode> PaymentsNode (GetNode(ObjectName, "PaymentsInfo"));
  if ((PaymentsNode.Assigned())&&(PaymentsNode->HasChildNodes))
    {
    xmlGuard <_di_IXMLNodeList> PaymentsChildNDL (PaymentsNode->GetChildNodes());
    for (int i=0;i<PaymentsChildNDL->Count;i++)
      {
      xmlGuard <_di_IXMLNode> PaymentNode (PaymentsChildNDL->Get(i));
      if (PaymentNode->NodeName==AnsiString("PaymentInfo"))
        {
        if ((Info==(PaymentNode->GetAttribute("info")).VOleStr)&&(Sum==(PaymentNode->GetAttribute("sum")).VOleStr))
          {
//          Log->Write("Got payment info: "+AnsiString(i)+" "+Info);
          PaymentsChildNDL->Delete(i);
          AnsiString Temp;
          dXML->SaveToXML(Temp);
          if (SaveFile(FileName,Temp))
            {
            Log->WriteInLine((boost::format("Payment info deleted: %1% %2% %3%") % i % Info.c_str() % Sum.c_str()).str().c_str());
            }
            else
            {
            Log->Write("Error writing InfoFileto disk!");
            _FileSystemError = true;
            }

          break;
          }
        }
//        else
//        Log->Write("Not a PaymentInfo node found: "+PaymentNode->NodeName+" | "+PaymentNode->NodeValue+".");
      }
    Log->Write((boost::format("Error deleting payment info: %1% %2% - can't find!") % Sum.c_str() % Info.c_str()).str().c_str());
    }
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TXMLInfo::ClearPaymentsInfo(AnsiString ObjectName)
{
    CS->Acquire();
    Log->Write((boost::format("ClearPaymentsInfo(%1%)") % ObjectName.c_str()).str().c_str());
    try
    {
        if (Enabled)
        {
            xmlGuard <_di_IXMLNode> PaymentsNode (GetNode(ObjectName, "PaymentsInfo"));
            if ((PaymentsNode.Assigned())&&(PaymentsNode->HasChildNodes))
            {
                xmlGuard <_di_IXMLNodeList> PaymentsChildNDL (PaymentsNode->GetChildNodes());
                while (PaymentsChildNDL->Count>0)
                    PaymentsChildNDL->Delete(0);
                AnsiString Temp;
                dXML->SaveToXML(Temp);
                if (!SaveFile(FileName,Temp))
                {
                    Log->Write("Error writing InfoFileto disk!");
                    _FileSystemError = true;
                }
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        _FileSystemError = true;
    }
    CS->Release();
}

//---------------------------------------------------------------------------

void TXMLInfo::AddPaymentInfo(AnsiString ObjectName, double Sum, AnsiString Info)
{
    CS->Acquire();
    try
    {
        if (Enabled)
        {
            xmlGuard <_di_IXMLNode> PaymentsNode (GetNode(ObjectName, "PaymentsInfo"));
            if (PaymentsNode.Assigned())
            {
                xmlGuard <_di_IXMLNode> NewPaymentInfoNode (PaymentsNode->AddChild("PaymentInfo"));
                NewPaymentInfoNode->SetAttribute("sum",AnsiString(Sum));
                NewPaymentInfoNode->SetAttribute("info",Info);
            }
            else
            {
                Log->Write("Error getting PaymentsInfo node!");
            }

            //dXML->SaveToFile(WideString(FileName));
            AnsiString Temp;
            dXML->SaveToXML(Temp);
            if (!SaveFile(FileName,Temp))
            {
                Log->Write("Error writing InfoFileto disk!");
                _FileSystemError = true;
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        _FileSystemError = true;
    }
    CS->Release();
}

//---------------------------------------------------------------------------

bool TXMLInfo::SaveFile(AnsiString _FileName, AnsiString SourceData)
{
    try
    {
        if (!Enabled)
            return false;
        if (SourceData=="")
        {
            Log->Write("Error! Zero data.");
            return false;
        }

        if (InfoFile != NULL)
        {
            InfoFile->Seek(0, soFromBeginning);
            InfoFile->Write(SourceData.c_str(), SourceData.Length());
            InfoFile->Size = SourceData.Length();
            return true;
        }
        else
        {
            Log->Write("File not opened!");
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return false;
}


