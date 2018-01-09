//---------------------------------------------------------------------------

#include <system.hpp>
#include <registry.hpp>
#include <algorith.h>
#include <XMLDoc.hpp>
#include <Classes.hpp>
#include <ComCtrls.hpp>
#include <memory>
#include <vector>
#include <string>
#include "globals.h"
#include <boost\algorithm\string.hpp>
#include <boost\format.hpp>
#include <boost\lexical_cast.hpp>

#include <IdBaseComponent.hpp>
#include <IdCoder.hpp>
#include <Math.hpp>
#include <IdCoderMessageDigest.hpp>
#include "TWConfig.h"
//#include "TCfgFieldData.h"
#include "JSONDocument.h"
#include "XMLInfo.h"

#include <fstream>
#include <sstream>

#pragma hdrstop
#pragma package(smart_init)
std::vector<std::string> image999;
bool append999 = false;
//---------------------------------------------------------------------------

__fastcall TWConfig::TWConfig(AnsiString _ConfigFileName, AnsiString _OperatorsFileName, TLogClass *_Log)
{
    recepientMT = -1;
    try
    {
        CS=NULL;
        CS=new TCriticalSection();
        CS->Acquire();
        SetDefaultValues();
        JSMaker = NULL;
        Prepared = false;
        InternalVersion = 7;
        CfgFile = NULL;
        OperFile = NULL;
        Menu = NULL;

        ActiveConnection=-1;

        JSMaker = new TJSMaker();
        InnerLog=false;
        Log=_Log;
        if (!Log)
        {
              Log = new TLogClass("WConfig");
              InnerLog=true;
        }

        SetOperDefaults(BlankOper);

        for (int i=0;i<MaxStatServersAdresses;i++)
        {
            StatInfo.Host[i] = "";
            StatInfo.Port[i] = 0;
        }
        StatInfo.StatServersAdressesCount=0;
        StatInfo.CurrentStatServersAdressNum=0;

        ConfigFileName = _ConfigFileName;
        OperatorsFileName = _OperatorsFileName;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TWConfig::SetOperDefaults(TOpInfo& OperatorInfo)
{
    try
    {
        OperatorInfo.Id=-1;
        OperatorInfo.Name="";
        OperatorInfo.Image="";
        OperatorInfo.RootMenuImage="";
        OperatorInfo.LimMin=0;
        OperatorInfo.LimMax=0;
        OperatorInfo.fix = false;
        OperatorInfo.ChequeFileName="default.xml";
        OperatorInfo.ErrorChequeFileName="payment-error.xml";
        OperatorInfo.ErrorMoneyChequeFileName="payment-error.xml";
        OperatorInfo.ProcessorType = "Cyberplat";
        OperatorInfo.KeysId = 0;
        OperatorInfo.Offline = 0;
        OperatorInfo.showOnLineComment = 1;
        OperatorInfo.RoundAmount = RNone;
        OperatorInfo.CheckAmount = 200;
        OperatorInfo.CheckURL="";
        OperatorInfo.PaymentBeforeChequePrinting = false;
        OperatorInfo.PaymentURL="";
        OperatorInfo.StatusURL="";
        OperatorInfo.GetCardsURL="";
        OperatorInfo.CheckAmountFieldId="";
        OperatorInfo.ACardsInfo="";
        OperatorInfo.DTCardsInfo="";
        OperatorInfo.PrinterOkOnly=false;
        OperatorInfo.GetCardsAllowed = false;
        OperatorInfo.SignatureType = 0;
        OperatorInfo.NameForCheque = "-";
        OperatorInfo.INNForCheque = "-";
        OperatorInfo.CurrencyId = "810";
        OperatorInfo.ServiceGuid = "";
        OperatorInfo.fix = false;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void __fastcall TWConfig::CheckDir(AnsiString DirName)
{
    try
    {
        if ((!DirectoryExists(DirName))&&(DirName!= ""))
        {
            Log->Write((boost::format("Directory %1% does not exist, creating...") % DirName.c_str()).str().c_str());
            if (!ForceDirectories(DirName))
            {
                Log->Append("...Error!.");
                return;
            }
            Log->Append("...OK.");
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

__fastcall TWConfig::~TWConfig()
{
    try
    {
        CS->Acquire();
        ClearOperatorsInfo();

        if (CfgFile)
        {
            Log->Write("Config file closed.");
            delete CfgFile;
            CfgFile = NULL;
        }

        if (OperFile)
        {
            Log->Write("Operators file closed.");
            delete OperFile;
            OperFile = NULL;
        }

        if (JSMaker)
        {
            delete JSMaker;
            JSMaker = NULL;
        }
        CountryCodes.clear();
        RASConnections.clear();
        ChequeCaption.clear();
        Keys.clear();

        if (InnerLog)
        {
            delete Log;
            Log=NULL;
        }
        CS->Release();
        delete CS;
        CS=NULL;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TWConfig::ClearOperatorsInfo()
{
    OperatorsInfo.clear();
}
//---------------------------------------------------------------------------

AnsiString TWConfig::GetHostFromRegistry(int HostNum)
{
	try
	{
		std::auto_ptr <TRegistry> Reg ( new TRegistry() );
		Reg->RootKey = HKEY_LOCAL_MACHINE;
		Reg->OpenKeyReadOnly("SOFTWARE\\CYBERPLAT");
		if (HostNum == 0)
			return Reg->ReadString("IP_CHECK");
		else
			return Reg->ReadString("IP_CHECK2");
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
  return "";
}

bool TWConfig::GetOperatorsInfo()
{
bool bResult = true;
try
	{
	try
		{
    xmlGuard <_di_IXMLDocument> XmlDoc(NULL);

		if (OperatorsInfo.size()>0)
			{
			Log->Write((boost::format("File %1% already read, trying to clear...") % OperatorsFileName.c_str()).str().c_str());
            ClearOperatorsInfo();
			}

else
      {

			if (FileExists(OperatorsFileName))
				{
				AnsiString FileData;
				bool bRes = OpenFile(OperFile, OperatorsFileName, FileData);
				if (bRes)
					{
          try
            {
            try
              {
              XmlDoc = LoadXMLData(FileData);
              }
              catch(...)
              {
                  ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
              FileData = FileData.SubString(0,FileData.Pos("</root>")+6);
              XmlDoc = LoadXMLData(FileData);
              }
            }
          catch (EDOMParseError &e)
            {
            Log->Write((boost::format("Operators cfg EDOMParseError exception encountered: %1%") % e.Message.c_str()).str().c_str());
            bResult = false;
            }
          catch(...)
          {
              ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
              bResult = false;
          }
          if (bResult)
          {
            XmlDoc->NodeIndentStr = "        ";
            XmlDoc->Options = XmlDoc->Options << doNodeAutoIndent;
            xmlGuard <_di_IXMLNode> RootNode (XmlDoc->GetDocumentElement());
            if (RootNode->HasChildNodes)
            {
              xmlGuard <_di_IXMLNodeList> xmlRootNodeList (RootNode->GetChildNodes());
              xmlGuard <_di_IXMLNode> xmlSecNode (xmlRootNodeList->FindNode("operators"));
              if ((xmlSecNode.Assigned())&&(xmlSecNode->HasChildNodes))
              {
                JSMaker->AddChild("operators");
                xmlGuard <_di_IXMLNodeList> xmlNodeList (xmlSecNode->GetChildNodes());
                for (int i = 0; (i<xmlNodeList->Count); i++)
                  {
                    xmlGuard <_di_IXMLNode> xmlNode (xmlNodeList->Get(i));
                    if ((xmlNode.Assigned())&&(xmlNode->NodeName == WideString("operator")))
                    {
                    ProcessOperatorNode(xmlNode);
                    }
                  }
                }
                JSMaker->CloseChild();
              }
            }//JSMaker->CloseChild();
					}
					else
					{
					Log->Write((boost::format("Error getting data from %1%!") % OperatorsFileName.c_str()).str().c_str());
					bResult = false;
					}
				}
				else
				{
				Log->Write((boost::format("File %1% does not exists.") % OperatorsFileName.c_str()).str().c_str());
				bResult = false;
				}
			}
		}
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            Prepared = false;
            bResult = false;
        }
    }
__finally
	{
	if (OperFile)
		{
		Log->Write("Operators file closed.");
		delete OperFile;
		OperFile = NULL;
		}
	return bResult;
	}
}

bool TWConfig::readMenu(const std::string& menuFileName)
{
    if (!FileExists(menuFileName.c_str()))
    {
        try
        {
            std::ifstream ifs(OperatorsFileName.c_str(), std::ios::in | std::ios::binary);
            std::ostringstream out;
            out << ifs.rdbuf();
            std::string str_data = out.str();
            std::string menu;

            int startMenu = str_data.find("<menu");
            int endMenu = str_data.find("</menu>");
            if(startMenu != str_data.npos && endMenu != str_data.npos)
            {
                int slnpos = str_data.find("\n"); // Узнаем где кончается шапка <?xml version="1.0" encoding="Windows-1251"?>
                if(slnpos != str_data.npos)
                    slnpos++; // Добавим первод строки
                menu = str_data.substr(0, slnpos) + str_data.substr(startMenu, endMenu - startMenu + strlen("</menu>"));
                std::ofstream ofs(menuFileName.c_str());
                ofs << menu;
            }
            else
            {
                Log->Write("Can not find <menu...</menu> in operators.xml");
                return false;
            }
        }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            return false;
        }
    }

    xmlGuard <_di_IXMLDocument> XmlDoc(NULL);
    AnsiString FileData;
    bool bRes = OpenFile(OperFile, menuFileName.c_str(), FileData);

    if(OperFile)
    {
        delete OperFile;
        OperFile = NULL;
    }

    if(!bRes)
    {
        Log->Write((boost::format("Can not open file: %1%") % menuFileName).str().c_str());
        return false;
    }

    try
    {
        XmlDoc = LoadXMLData(FileData);
    }
    catch (EDOMParseError &e)
    {
        Log->Write((boost::format("File:%1% EDOMParseError exception encountered: %2%") % menuFileName % e.Message.c_str()).str().c_str());
        return false;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }

    XmlDoc->NodeIndentStr = "        ";
    XmlDoc->Options = XmlDoc->Options << doNodeAutoIndent;
    xmlGuard <_di_IXMLNode> RootNode (XmlDoc->GetDocumentElement());
    xmlGuard <_di_IXMLNodeList> xmlRootNodeList (RootNode->GetChildNodes());
      //zh_mk508
      _groups_info tempGroup;
      //xmlGuard <_di_IXMLNode> MenuNode (xmlRootNodeList->FindNode("menu"));
      if(RootNode.Assigned())
        if(RootNode->HasChildNodes)
        {
          //xmlGuard <_di_IXMLNodeList> MenuNodeList (RootNode->GetChildNodes());
          for (int izz = 0; (izz<xmlRootNodeList->Count); izz++)
          {
            xmlGuard <_di_IXMLNode> groupNode (xmlRootNodeList->Get(izz));
            if (groupNode.Assigned())
              if ((groupNode->NodeName == WideString("group")) && (groupNode->HasAttribute("id")))
              {
                int tGroupId = AnsiString(groupNode->GetAttribute("id").VOleStr).ToInt();
                if(tGroupId == 100)
                {
                  if(groupNode->HasChildNodes)
                  {
                    xmlGuard <_di_IXMLNodeList> groupNodeList (groupNode->GetChildNodes());
                    for (int jzz = 0; (jzz<groupNodeList->Count); jzz++)
                    {
                      xmlGuard <_di_IXMLNode> groupNodeItem (groupNodeList->Get(jzz));
                      if (groupNodeItem.Assigned())
                        if ((groupNodeItem->NodeName == WideString("operator_id")) && (groupNodeItem->HasAttribute("id")))
                          if(AnsiString(groupNodeItem->GetAttribute("id").VOleStr) == "999")
                            if(groupNodeItem->HasAttribute("image"))
                              image999.push_back(AnsiString(groupNodeItem->GetAttribute("image")).c_str());
                            else
                              image999.push_back("---");
                    }
                  }
                }
                else
                {
                  tempGroup.Id = tGroupId;
                  if(groupNode->HasAttribute("title"))
                    tempGroup.title = AnsiString(groupNode->GetAttribute("title").VOleStr).c_str();
                  tempGroup.OperatorIds.clear();
                  if(groupNode->HasChildNodes)
                  {
                    xmlGuard <_di_IXMLNodeList> groupNodeList (groupNode->GetChildNodes());
                    for (int jzz = 0; (jzz<groupNodeList->Count); jzz++)
                    {
                      xmlGuard <_di_IXMLNode> groupNodeItem (groupNodeList->Get(jzz));
                      if (groupNodeItem.Assigned())
                      {
                        if ((groupNodeItem->NodeName == WideString("operator_id")) && (groupNodeItem->HasAttribute("id")))
                        {
                          int iOpID = AnsiString(groupNodeItem->GetAttribute("id").VOleStr).ToInt();
                          m_allOperatorsInMenu.insert(iOpID);
                          tempGroup.OperatorIds.insert(AnsiString(groupNodeItem->GetAttribute("id").VOleStr).ToInt());
                        }
                        if(groupNodeItem->HasChildNodes)
                        {
                          xmlGuard <_di_IXMLNodeList> groupSecondNodeList (groupNodeItem->GetChildNodes());
                          for (int kzz = 0; (kzz<groupSecondNodeList->Count); kzz++)
                          {
                            xmlGuard <_di_IXMLNode> groupSecondNodeItem (groupSecondNodeList->Get(kzz));
                            if (groupSecondNodeItem.Assigned())
                            {
                              if ((groupSecondNodeItem->NodeName == WideString("operator_id")) && (groupSecondNodeItem->HasAttribute("id")))
                              {
                                int iOpID = AnsiString(groupSecondNodeItem->GetAttribute("id").VOleStr).ToInt();
                                m_allOperatorsInMenu.insert(iOpID);
                                tempGroup.OperatorIds.insert(iOpID);
                              }
                              if(groupSecondNodeItem->HasChildNodes)
                              {
                                xmlGuard <_di_IXMLNodeList> groupThirdNodeList (groupSecondNodeItem->GetChildNodes());
                                for (int nzz = 0; (nzz < groupThirdNodeList->Count); nzz++)
                                {
                                  xmlGuard <_di_IXMLNode> groupThirdNodeItem (groupThirdNodeList->Get(nzz));
                                  if (groupThirdNodeItem.Assigned())
                                  {
                                    if ((groupThirdNodeItem->NodeName == WideString("operator_id")) && (groupThirdNodeItem->HasAttribute("id")))
                                    {
                                      int iOpID = AnsiString(groupThirdNodeItem->GetAttribute("id").VOleStr).ToInt();
                                      m_allOperatorsInMenu.insert(iOpID);
                                      tempGroup.OperatorIds.insert(iOpID);
                                    }
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                  GroupsInfo.push_back(tempGroup);
                }
              }
          }
        }
    if (!Menu)
    {
        JSMaker->AddChild("menu");
        JSMaker->AddChild("0");
    }
    ProcessMenuGroup(RootNode);
    if (!Menu)
    {
        JSMaker->CloseChild();
        JSMaker->CloseChild();
    }
    return true;
}

void TWConfig::ProcessOperatorNode(xmlGuard <_di_IXMLNode>& xmlNode)
{
    TOpInfo OperatorInfo;
    SetOperDefaults(OperatorInfo);
    std::vector<std::string>::iterator itzz = image999.begin();

    if (xmlNode->HasAttribute("id"))
        OperatorInfo.Id = AnsiString((xmlNode->GetAttribute("id")).VOleStr).ToInt();
    else
        return;

    m_allOperators.insert(OperatorInfo.Id);

    if (xmlNode->HasAttribute("printer_ok_only"))
    {
        OperatorInfo.PrinterOkOnly = AnsiString((xmlNode->GetAttribute("printer_ok_only")).VOleStr).ToInt();
    }

    if (xmlNode->HasChildNodes)
    {
        int has_fix = 0;

        xmlGuard <_di_IXMLNodeList> xmlOpParamsNodeList (xmlNode->GetChildNodes());
        for (int i = 0; (i<xmlOpParamsNodeList->Count); i++)
        {
          xmlGuard <_di_IXMLNode> xmlOpParamNode (xmlOpParamsNodeList->Get(i));
          AnsiString ParamName = xmlOpParamNode->NodeName;
        //      Log->Write(ParamName);
          if (xmlOpParamNode->IsTextElement)
          {
            if (ParamName == "name")
            {
              OperatorInfo.Name = AnsiString(xmlOpParamNode->NodeValue).c_str();
              continue;
            }

            if (ParamName == "name_for_cheque")
            {
              OperatorInfo.NameForCheque = AnsiString(xmlOpParamNode->NodeValue).c_str();
              continue;
            }

            if (ParamName == "inn_for_cheque")
            {
              OperatorInfo.INNForCheque = AnsiString(xmlOpParamNode->NodeValue).c_str();
              continue;
            }

            ////////////////////////////////////////////////////////////////////////
            // тип услуги
            if(ParamName == "serviceGuid")
            {
             OperatorInfo.ServiceGuid = AnsiString(xmlOpParamNode->NodeValue).c_str();
            }
            ////////////////////////////////////////////////////////////////////////
        if (ParamName == "image")
        {
          OperatorInfo.Image = AnsiString(xmlOpParamNode->NodeValue).c_str();
          continue;
        }

            if (ParamName == "rootmenuimage")
            {
              OperatorInfo.RootMenuImage = AnsiString(xmlOpParamNode->NodeValue).c_str();
              continue;
            }

          continue;
          }

          if (ParamName == "limit")
          {
            if (xmlOpParamNode->HasAttribute("min"))
              OperatorInfo.LimMin = ::GetDouble((xmlOpParamNode->GetAttribute("min")).VOleStr);

            if (xmlOpParamNode->HasAttribute("max"))
              OperatorInfo.LimMax = ::GetDouble((xmlOpParamNode->GetAttribute("max")).VOleStr);

            OperatorInfo.fix = (OperatorInfo.LimMin == OperatorInfo.LimMax);
            if (xmlOpParamNode->HasAttribute("fix"))
              has_fix = ::GetInt((xmlOpParamNode->GetAttribute("fix")).VOleStr);
            continue;
          }

          if (ParamName == "cheque")
          {
            if (xmlOpParamNode->HasAttribute("filename"))
              OperatorInfo.ChequeFileName = AnsiString(xmlOpParamNode->GetAttribute("filename")).c_str();

            if (xmlOpParamNode->HasAttribute("payment_error_filename"))
              OperatorInfo.ErrorChequeFileName = AnsiString(xmlOpParamNode->GetAttribute("payment_error_filename")).c_str();
            if (xmlOpParamNode->HasAttribute("payment_notenoughmoney_filename"))
              OperatorInfo.ErrorMoneyChequeFileName = AnsiString(xmlOpParamNode->GetAttribute("payment_notenoughmoney_filename")).c_str();
            else
              OperatorInfo.ErrorMoneyChequeFileName=OperatorInfo.ErrorChequeFileName;
            continue;
          }

          if (ParamName == "comission")
          {
            GetComission(OperatorInfo,xmlOpParamNode);
            continue;
          }

          if (ParamName == "fields")
          {
            OperatorInfo.SalonField = 0;
            ProcessFieldsNode(OperatorInfo, xmlOpParamNode);
            continue;
          }

          if (ParamName == "marketing")
          {
            if (xmlOpParamNode->HasAttribute("operator_id"))
              OperatorInfo.MarketingOperatorId = AnsiString(xmlOpParamNode->GetAttribute("operator_id")).c_str();
            continue;
          }

          if (ParamName == "processor")
          {
            ProcessProcessorNode(OperatorInfo, xmlOpParamNode);
            continue;
          } //processor
        }

        for(int i = 0; i < GroupsInfo.size(); i++)
          if (GroupsInfo[i].OperatorIds.find(OperatorInfo.Id) != GroupsInfo[i].OperatorIds.end())
            OperatorInfo.GroupId = GroupsInfo[i].Id;

        if (has_fix)
        {
          if (OperatorInfo.ProcessorType == "cyberplat_mt")
            OperatorInfo.fix = static_cast<int>(has_fix == 1);
          for(int i = 0; i < OperatorInfo.Fields.size(); i++)
            if ((boost::lexical_cast<int>(OperatorInfo.Fields[i].Id) == has_fix) &&
                (OperatorInfo.Fields[i].Type == "integer") &&
                (OperatorInfo.GroupId != 101))
              OperatorInfo.fix = has_fix;
        }
        if (OperatorInfo.ProcessorType == "Cyberplat_PIN")
           OperatorInfo.PrinterOkOnly = 1;

        OperatorInfo.ChequeFileName = (Dirs.WorkDir+"config\\templates\\"+OperatorInfo.ChequeFileName.c_str()).c_str();
        OperatorInfo.ErrorChequeFileName = (Dirs.WorkDir+"config\\templates\\"+OperatorInfo.ErrorChequeFileName.c_str()).c_str();
        OperatorInfo.ErrorMoneyChequeFileName = (Dirs.WorkDir+"config\\templates\\"+OperatorInfo.ErrorMoneyChequeFileName.c_str()).c_str();
    }

    // write js
    postProcessOperatorNode(xmlNode, OperatorInfo);
    OperatorsInfo.push_back(OperatorInfo);
}

void TWConfig::postProcessOperatorNode(xmlGuard <_di_IXMLNode>& xmlNode, TOpInfo& OperatorInfo)
{
    JSMaker->AddChild(AnsiString(OperatorInfo.Id).c_str());
    JSMaker->AddStringAttribute("printer_ok_only", AnsiString(int(OperatorInfo.PrinterOkOnly)).c_str());

    //ищем тег комиссии, если его - прописываем в iface_config.js 0 комиссию
    bool comissionExist = false;
    if (xmlNode->HasChildNodes)
    {
        xmlGuard <_di_IXMLNodeList> xmlOpParamsNodeList (xmlNode->GetChildNodes());
        for (int i = 0; (i < xmlOpParamsNodeList->Count); i++)
        {
            xmlGuard <_di_IXMLNode> xmlOpParamNode (xmlOpParamsNodeList->Get(i));
            std::string paramName = WCharToString(xmlOpParamNode->NodeName);
            boost::to_lower(paramName);
            if(paramName == "comission")
            {
                comissionExist = true;
                break;
            }
        }
    }
    if(!comissionExist)
    {
        JSMaker->AddChild("comission");
            JSMaker->AddChild("part0");
                JSMaker->AddStringAttribute("min", "0");
                JSMaker->AddStringAttribute("value", "0%");
                JSMaker->AddStringAttribute("min_time", "0");
                JSMaker->AddStringAttribute("min_day", "0");
            JSMaker->CloseChild();
        JSMaker->CloseChild();
    }

    if (xmlNode->HasChildNodes)
    {
        int has_fix = 0;

        xmlGuard <_di_IXMLNodeList> xmlOpParamsNodeList (xmlNode->GetChildNodes());
        for (int i = 0; (i<xmlOpParamsNodeList->Count); i++)
        {
          xmlGuard <_di_IXMLNode> xmlOpParamNode (xmlOpParamsNodeList->Get(i));
          AnsiString ParamName = xmlOpParamNode->NodeName;
          if (xmlOpParamNode->IsTextElement)
          {
            if ((ParamName != "image")&&(ParamName!= "rootmenuimage"))
            {
              JSMaker->AddStringAttribute(ParamName.c_str(), (xmlOpParamNode->NodeValue.operator AnsiString()).c_str());
            }
            ////////////////////////////////////////////////////////////////////////
            if (ParamName == "image")
            {
              if ((OperatorInfo.Id == 999) && !image999.empty())
              {
                JSMaker->AddChild("image");
                int tempCounter = 0;
                for(std::vector<std::string>::iterator it = image999.begin(); it != image999.end(); it++)
                  JSMaker->AddStringAttribute(AnsiString(tempCounter++).c_str(), (*it).c_str());
                JSMaker->CloseChild();
              }
              else
                JSMaker->AddStringAttribute("image", OperatorInfo.Image.c_str());
              continue;
            }

            if (ParamName == "rootmenuimage")
            {
              JSMaker->AddStringAttribute("rootmenuimage", OperatorInfo.RootMenuImage.c_str());
              continue;
            }

          continue;
          }

          if (ParamName == "limit")
          {
            JSMaker->AddChild("limit");
            JSMaker->AddAttribute("min", AnsiString(OperatorInfo.LimMin).c_str());
            JSMaker->AddAttribute("max", AnsiString(OperatorInfo.LimMax).c_str());
            JSMaker->CloseChild();
            continue;
          }

          if (ParamName == "comission")
          {
            JSMaker->AddChild("comission");
            for (std::size_t i=0;i < OperatorInfo.CommissionInfo.size();i++)
            {
                if(OperatorInfo.ProcessorType == "avia_center")
                {
                    OperatorInfo.CommissionInfo[i].Value = 0;
                }

                JSMaker->AddChild(("part"+AnsiString(i)).c_str());
                JSMaker->AddStringAttribute("min", AnsiString(OperatorInfo.CommissionInfo[i].Min).c_str());
                if(OperatorInfo.CommissionInfo[i].Relative)
                    JSMaker->AddStringAttribute("value",(AnsiString(OperatorInfo.CommissionInfo[i].Value)+"%").c_str());
                else
                    JSMaker->AddStringAttribute("value",AnsiString(OperatorInfo.CommissionInfo[i].Value).c_str());
                JSMaker->AddStringAttribute("min_time", AnsiString(OperatorInfo.CommissionInfo[i].MinTime).c_str());
                JSMaker->AddStringAttribute("min_day", AnsiString(OperatorInfo.CommissionInfo[i].MinDay).c_str());
                JSMaker->CloseChild();
            }
            JSMaker->CloseChild();
            continue;
          }

          if (ParamName == "fields")
          {
            postProcessFieldsNode(xmlOpParamNode);
            continue;
          }

          if (ParamName == "marketing")
          {
            JSMaker->AddAttribute("marketing_operator_id", OperatorInfo.MarketingOperatorId.c_str());
            continue;
          }

          if (ParamName == "processor")
          {
            postProcessProcessorNode(OperatorInfo, xmlOpParamNode);
            continue;
          } //processor
        }

        JSMaker->AddAttribute("fix", AnsiString((int)OperatorInfo.fix).c_str());
        JSMaker->CloseChild();  //operator
    }
}

void TWConfig::ProcessProcessorNode(TOpInfo& OperatorInfo, xmlGuard <_di_IXMLNode>& xmlOpParamNode)
{
  if (xmlOpParamNode->HasAttribute("type"))
    OperatorInfo.ProcessorType = AnsiString(xmlOpParamNode->GetAttribute("type")).c_str();
  else
    OperatorInfo.ProcessorType = "Cyberplat";

  if (xmlOpParamNode->HasAttribute("keys_id"))
    OperatorInfo.KeysId = ::GetInt(xmlOpParamNode->GetAttribute("keys_id"));
  else
    OperatorInfo.KeysId = 0;

  if (xmlOpParamNode->HasAttribute("offline"))
    OperatorInfo.Offline = xmlOpParamNode->GetAttribute("offline");
  else
    OperatorInfo.Offline = Payments.Offline;

  std::string processorTypeLowerCase = OperatorInfo.ProcessorType;
  boost::to_lower(processorTypeLowerCase);
  if((processorTypeLowerCase == "cyberplat_taxes") ||
     (processorTypeLowerCase == "cyberplat_mt") ||
     (processorTypeLowerCase == "cyberplat_pin") ||
     (processorTypeLowerCase == "cyberplat_pin_trans") ||
     (processorTypeLowerCase == "half_pin") ||
     (processorTypeLowerCase == "avia_center") ||
     (processorTypeLowerCase == "cyberplat_mt") ||
     (processorTypeLowerCase == "cyberplat_metro"))
    OperatorInfo.Offline = 0;

  OperatorInfo.showOnLineComment = 1;
  if (xmlOpParamNode->HasAttribute("showOnLineComment"))
    OperatorInfo.showOnLineComment = xmlOpParamNode->GetAttribute("showOnLineComment");

  if (xmlOpParamNode->HasAttribute("showAddInfo"))
  {
      if(xmlOpParamNode->GetAttribute("showAddInfo")!=0 && false==OperatorInfo.Offline)
          OperatorInfo.ShowAddInfo = true;
      else
          OperatorInfo.ShowAddInfo = false;
  }
  else
  {
    OperatorInfo.ShowAddInfo = false;
  }

  OperatorInfo.RoundAmount = RNone;
  if (xmlOpParamNode->HasAttribute("round-amount"))
  {
    AnsiString RoundTemp = xmlOpParamNode->GetAttribute("round-amount");
    if (RoundTemp.LowerCase() == "down")
    {
      OperatorInfo.RoundAmount = RDown;
    }
    else
    {
      if (RoundTemp.LowerCase() == "up")
      {
        OperatorInfo.RoundAmount = RUp;
      }
      else
      {
        if (RoundTemp.LowerCase() == "bankers")
        {
          OperatorInfo.RoundAmount = RBankers;
        }
      }
    }
  }

  if (xmlOpParamNode->HasAttribute("signature_type"))
    OperatorInfo.SignatureType = ::GetInt(xmlOpParamNode->GetAttribute("signature_type"));
    else
    OperatorInfo.SignatureType = 0;

  //if (((AnsiString)(OperatorInfo.ProcessorType.c_str())).LowerCase() == "cyberplat_mt")
    //OperatorInfo.SignatureType = 1;

  if (xmlOpParamNode->HasAttribute("currency_id"))
    OperatorInfo.CurrencyId = AnsiString(xmlOpParamNode->GetAttribute("currency_id")).c_str();
    else
    OperatorInfo.CurrencyId = "810";

  OperatorInfo.PaymentBeforeChequePrinting = false;
  AnsiString ProcessorType=((AnsiString)OperatorInfo.ProcessorType.c_str()).LowerCase();
  if(ProcessorType == "cyberplat_mt")
  {
      recepientMT = OperatorInfo.Id;
  }

  if (
    (ProcessorType == "cyberplat")
    ||(ProcessorType == "cyberplat_pin")
    ||(ProcessorType == "cyberplat_pin_trans")
    ||(ProcessorType == "cyberplat_euroset")
    ||(ProcessorType == "cyberplat_metro")
    ||(ProcessorType == "cyberplat_mt")
    ||(ProcessorType == "half_pin")
    ||(ProcessorType == "avia_center")
    ||(ProcessorType == "cyberplat_taxes")
    )
  {
    xmlGuard <_di_IXMLNodeList> xmlProcNodeList (xmlOpParamNode->GetChildNodes());
    for (int i = 0;i<xmlProcNodeList->GetCount();i++)
    {
      xmlGuard <_di_IXMLNode> xmlProcParamNode (xmlProcNodeList->Get(i));

      if (xmlProcParamNode->NodeName==(AnsiString)"check")
      {
        AnsiString CheckAmountTemp = "0";
        try
        {
          if (xmlProcParamNode->HasAttribute("amount-value"))
          {
            CheckAmountTemp = xmlProcParamNode->GetAttribute("amount-value");
            if (CheckAmountTemp.Pos("%"))
            {
              OperatorInfo.CheckAmount = -1;
              OperatorInfo.CheckAmountFieldId = ChangeChars(CheckAmountTemp,"%","").c_str();
            }
            else
            {
              OperatorInfo.CheckAmount = ::GetInt(CheckAmountTemp);
            }
          }
          else
          {
            OperatorInfo.CheckAmount = 200;
          }
        }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            Log->Write("Error getting check amount, set to 200.");
            OperatorInfo.CheckAmount = 200;
        }

        xmlGuard <_di_IXMLNodeList> xmlCheckNodeList (xmlProcParamNode->GetChildNodes());
        xmlGuard <_di_IXMLNode> xmlCheckURLNode (xmlCheckNodeList->FindNode("url"));
        if (xmlCheckURLNode.Assigned())
          OperatorInfo.CheckURL = AnsiString(xmlCheckURLNode->GetNodeValue()).c_str();

        //int k = 0;
        for (int i = 0;i<xmlCheckNodeList->GetCount();i++)
        {
          xmlGuard <_di_IXMLNode> ParamNode (xmlCheckNodeList->Get(i));
          if ((ParamNode.Assigned())&&(ParamNode->NodeName == AnsiString("request-property"))&&(ParamNode->HasAttribute("name")))
          {
            AnsiString Name,FieldId;
            //OperatorInfo.PrName[k] = ParamNode->GetAttribute("name");
            Name = ParamNode->GetAttribute("name");
            if(ParamNode->HasAttribute("field-rule"))
            {
              //OperatorInfo.PrFieldID[k] = ParamNode->GetAttribute("field-rule");
              FieldId = ParamNode->GetAttribute("field-rule");
            }
            else
            {
              if(ParamNode->HasAttribute("field-id"))
              {
                //OperatorInfo.PrFieldID[k] = "[#"+ParamNode->GetAttribute("field-id")+"]";
                FieldId = "[#"+ParamNode->GetAttribute("field-id")+"]";
              }
              else
              {
                if (ParamNode->HasChildNodes)
                {
                  //OperatorInfo.PrFieldID[k] = "";
                  xmlGuard <_di_IXMLNodeList> ParamNDL (ParamNode->GetChildNodes());
                  for (int i = 0;i<ParamNDL->GetCount();i++)
                  {
                    xmlGuard <_di_IXMLNode> FieldIdNode (ParamNDL->Get(i));
                    if (FieldIdNode->NodeName == AnsiString("field-ref"))
                    {
                      //OperatorInfo.PrFieldID[k]+= "[#";
                      FieldId += "[#";
                    }
                    if ((FieldIdNode.Assigned())&&(FieldIdNode->HasAttribute("id")))
                    {
                      //OperatorInfo.PrFieldID[k]+= FieldIdNode->GetAttribute("id");
                      FieldId += FieldIdNode->GetAttribute("id");
                    }
                    if (FieldIdNode->NodeName == AnsiString("field-ref"))
                    {
                      //OperatorInfo.PrFieldID[k]+= "]";
                      FieldId += "]";
                    }
                  }
                }
              }
              //k++;
            }
            OperatorInfo.Properties.push_back(property(Name.c_str(),FieldId.c_str()));
          }
          else
          {
            if ((ParamNode.Assigned())&&(ParamNode->NodeName == AnsiString("receive-property"))&&(ParamNode->HasAttribute("name")))
            {
              AnsiString Description;
              std::string Name;
              bool Encrypted;

              Name = ((AnsiString)ParamNode->GetAttribute("name")).c_str();

              if(ParamNode->HasAttribute("description"))
              {
                Description = ParamNode->GetAttribute("description");
              }

              if(ParamNode->HasAttribute("encrypted"))
              {
                Encrypted = ::GetInt(ParamNode->GetAttribute("encrypted"));
              }
              else
              {
                Encrypted = false;
              }

              OperatorInfo.ReceiveProperties[Name] = receive_property(Name.c_str(),Description.c_str(),Encrypted);
            }
          }
        }
        //OperatorInfo.PrCount = k;
        continue;
      } //check

      if (xmlProcParamNode->NodeName==(AnsiString)"payment")
      {
        if (xmlProcParamNode->HasAttribute("before_cheque_printing"))
          OperatorInfo.PaymentBeforeChequePrinting = AnsiString((xmlProcParamNode->GetAttribute("before_cheque_printing")).VOleStr).ToInt();
        else
        {
          ProcessorType=((AnsiString)OperatorInfo.ProcessorType.c_str()).LowerCase();
          if ((ProcessorType == "cyberplat_pin")||(ProcessorType == "cyberplat_pin_trans")||(ProcessorType == "half_pin")||(ProcessorType == "cyberplat_euroset")||(ProcessorType == "cyberplat_metro")||(ProcessorType == "cyberplat_mt")||(ProcessorType == "avia_center"))
            OperatorInfo.PaymentBeforeChequePrinting = true;
          else
            OperatorInfo.PaymentBeforeChequePrinting = false;
        }

        xmlGuard <_di_IXMLNodeList> xmlPaymentNodeList (xmlProcParamNode->GetChildNodes());
        xmlGuard <_di_IXMLNode>  xmlPaymentURLNode (xmlPaymentNodeList->FindNode("url"));
        if (xmlPaymentURLNode.Assigned())
          OperatorInfo.PaymentURL = AnsiString(xmlPaymentURLNode->GetNodeValue()).c_str();
        continue;
      } //payment

      if (xmlProcParamNode->NodeName==(AnsiString)"status")
      {
        xmlGuard <_di_IXMLNodeList> xmlStatusNodeList (xmlProcParamNode->GetChildNodes());
        xmlGuard <_di_IXMLNode>  xmlStatusURLNode (xmlStatusNodeList->FindNode("url"));
        if (xmlStatusURLNode.Assigned())
          OperatorInfo.StatusURL = AnsiString(xmlStatusURLNode->GetNodeValue()).c_str();
        continue;
      } //status

      if (xmlProcParamNode->NodeName==(AnsiString)"getcards")
      {
        if (xmlProcParamNode->HasAttribute("before_cheque_printing"))
          OperatorInfo.PaymentBeforeChequePrinting = AnsiString((xmlProcParamNode->GetAttribute("before_cheque_printing")).VOleStr).ToInt();
        else
        {
          ProcessorType=((AnsiString)OperatorInfo.ProcessorType.c_str()).LowerCase();
          if ((ProcessorType == "cyberplat_pin")||(ProcessorType == "cyberplat_pin_trans")||(ProcessorType == "half_pin")||(ProcessorType == "cyberplat_euroset")||(ProcessorType == "cyberplat_metro")||(ProcessorType == "cyberplat_mt")||(ProcessorType == "avia_center"))
            OperatorInfo.PaymentBeforeChequePrinting = true;
          else
            OperatorInfo.PaymentBeforeChequePrinting = false;
        }
        if (xmlProcParamNode->HasAttribute("mnvo_id"))
          OperatorInfo.MNVO_ID = AnsiString((xmlProcParamNode->GetAttribute("mnvo_id")).VOleStr).c_str();
        xmlGuard <_di_IXMLNodeList> xmlGetCardsNodeList (xmlProcParamNode->GetChildNodes());
        xmlGuard <_di_IXMLNode>  xmlGetCardsURLNode (xmlGetCardsNodeList->FindNode("url"));
        if (xmlGetCardsURLNode.Assigned())
          OperatorInfo.GetCardsURL = AnsiString(xmlGetCardsURLNode->GetNodeValue()).c_str();

        OperatorInfo.GetCardsAllowed = false;
        OperatorInfo.MaxSavedCardsCount = "";
        xmlGuard <_di_IXMLNode>  xmlGetCardsMSCCNode (xmlGetCardsNodeList->FindNode("max_saved_cards_count"));
        if (xmlGetCardsMSCCNode.Assigned())
          OperatorInfo.MaxSavedCardsCount = AnsiString(xmlGetCardsMSCCNode->GetNodeValue()).c_str();
        continue;
      } //getcards

      if (xmlProcParamNode->NodeName==(AnsiString)"login")
      {
        xmlGuard <_di_IXMLNodeList> xmlLoginNodeList (xmlProcParamNode->GetChildNodes());
        xmlGuard <_di_IXMLNode>  xmlLoginURLNode (xmlLoginNodeList->FindNode("url"));
        if (xmlLoginURLNode.Assigned())
          OperatorInfo.LoginURL = AnsiString(xmlLoginURLNode->GetNodeValue()).c_str();
        continue;
      } //login
    }
  }
}

void TWConfig::postProcessProcessorNode(TOpInfo& OperatorInfo, xmlGuard <_di_IXMLNode>& xmlOpParamNode)
{
    JSMaker->AddChild("processor");
    JSMaker->AddStringAttribute("type", OperatorInfo.ProcessorType.c_str());
    JSMaker->AddStringAttribute("offline", AnsiString(OperatorInfo.Offline).c_str());
    JSMaker->AddStringAttribute("showOnLineComment", OperatorInfo.showOnLineComment);
    if(OperatorInfo.ShowAddInfo)
        JSMaker->AddStringAttribute("showAddInfo", "1");
    else
        JSMaker->AddStringAttribute("showAddInfo", "0");

    JSMaker->AddChild("properties");
    for(int i = 0; i < OperatorInfo.Properties.size(); i++)
    {
        JSMaker->AddChild(i);
            JSMaker->AddStringAttribute("name", OperatorInfo.Properties[i].Name.c_str());
            JSMaker->AddStringAttribute("field_property", OperatorInfo.Properties[i].FieldId.c_str());
        JSMaker->CloseChild();
    }
    JSMaker->CloseChild();

    AnsiString ProcessorType=((AnsiString)OperatorInfo.ProcessorType.c_str()).LowerCase();

    if (
        (ProcessorType == "cyberplat")
        ||(ProcessorType == "cyberplat_pin")
        ||(ProcessorType == "cyberplat_pin_trans")
        ||(ProcessorType == "cyberplat_euroset")
        ||(ProcessorType == "cyberplat_metro")
        ||(ProcessorType == "cyberplat_mt")
        ||(ProcessorType == "half_pin")
        ||(ProcessorType == "avia_center")
        ||(ProcessorType == "cyberplat_taxes")
        )
    {
        xmlGuard <_di_IXMLNodeList> xmlProcNodeList (xmlOpParamNode->GetChildNodes());
        for (int i = 0;i<xmlProcNodeList->GetCount();i++)
        {
          xmlGuard <_di_IXMLNode> xmlProcParamNode (xmlProcNodeList->Get(i));

          if (xmlProcParamNode->NodeName==(AnsiString)"check")
          {
            JSMaker->AddChild("ret_properties");
            AnsiString CheckAmountTemp = "0";
            xmlGuard <_di_IXMLNodeList> xmlCheckNodeList (xmlProcParamNode->GetChildNodes());
            xmlGuard <_di_IXMLNode> xmlCheckURLNode (xmlCheckNodeList->FindNode("url"));

            //int k = 0;
            for (int i = 0;i<xmlCheckNodeList->GetCount();i++)
            {
                xmlGuard <_di_IXMLNode> ParamNode (xmlCheckNodeList->Get(i));
                if ((ParamNode.Assigned())&&(ParamNode->NodeName == AnsiString("receive-property"))&&(ParamNode->HasAttribute("name")))
                {
                  AnsiString Description;
                  std::string Name;

                  Name = ((AnsiString)ParamNode->GetAttribute("name")).c_str();

                  if(ParamNode->HasAttribute("description"))
                  {
                    Description = ParamNode->GetAttribute("description");
                  }

                  JSMaker->AddStringAttribute(Name.c_str(), Description.c_str());
                }
            }
            //OperatorInfo.PrCount = k;
            JSMaker->CloseChild(); //ret_properties
            continue;
          } //check
        }
    }
    JSMaker->CloseChild();  //processor
}

void TWConfig::ProcessFieldsNode(TOpInfo& OperatorInfo, xmlGuard <_di_IXMLNode>& xmlOpParamNode)
{
  if (xmlOpParamNode->HasChildNodes)
  {
    xmlGuard <_di_IXMLNodeList> xmlFieldsNDL (xmlOpParamNode->GetChildNodes());
    for (int i = 0;i<xmlFieldsNDL->GetCount();i++)
    {
        int siblingEnumCounter = 0;

      xmlGuard <_di_IXMLNode> xmlFieldNode (xmlFieldsNDL->Get(i));
      if (xmlFieldNode->NodeName==(AnsiString)"field")
      {
        AnsiString Id;
        AnsiString Name;
        AnsiString URL1 = "";
        AnsiString Mask;
        bool SendUnmasked = false;
        AnsiString Type;
        std::map <std::string, std::string> Enum;
        AnsiString Regexp,Format;

        if (xmlFieldNode->HasAttribute("id"))
          {
          //OperatorInfo.FieldID[OperatorInfo.FieldsCount] = xmlFieldNode->GetAttribute("id");
          Id = xmlFieldNode->GetAttribute("id");
          }

        if (xmlFieldNode->HasAttribute("type"))
          {
          //OperatorInfo.FieldType[OperatorInfo.FieldsCount] = xmlFieldNode->GetAttribute("type");
          Type = xmlFieldNode->GetAttribute("type");
          }

        if (xmlFieldNode->HasChildNodes)
        {

          xmlGuard <_di_IXMLNodeList> xmlFieldNDL (xmlFieldNode->GetChildNodes());
            bool closeChild = false;
          for (int i = 0;i<xmlFieldNDL->Count;i++)
          {
            xmlGuard <_di_IXMLNode> xmlFieldParamNode (xmlFieldNDL->Get(i));

            if (xmlFieldParamNode->NodeName==(AnsiString)"name")
            {
              Name = xmlFieldParamNode->GetNodeValue();
              continue;
            }

            if (xmlFieldParamNode->NodeName == AnsiString("url_svyaznoy"))
            {
              URL1 = xmlFieldParamNode->GetNodeValue();
              OperatorInfo.SalonField = boost::lexical_cast<int>(OperatorInfo.Fields.size());
              continue;
            }

            if(xmlFieldParamNode->NodeName == (AnsiString)"cheque_mask")
            {
                if (xmlFieldParamNode->HasAttribute("regexp"))
                    Regexp = xmlFieldParamNode->GetAttribute("regexp");
                if (xmlFieldParamNode->HasAttribute("format"))
                    Format = xmlFieldParamNode->GetAttribute("format");
            }

            if (xmlFieldParamNode->NodeName==(AnsiString)"mask")
            {
              Mask = xmlFieldParamNode->GetNodeValue();
              if (xmlFieldParamNode->HasAttribute("send_unmasked"))
              {
                AnsiString MaskFlag = (xmlFieldParamNode->GetAttribute("send_unmasked")).VOleStr;
                SendUnmasked = ((MaskFlag == "1")||(MaskFlag.LowerCase() == "yes"));
              }
              continue;
            }
            if ((xmlFieldParamNode->NodeName==(AnsiString)"enum")&&(Type == AnsiString("enum")))
            {
              // смотрим есть ли parentId parentValue

              xmlGuard <_di_IXMLNodeList> EnumFieldNDL (xmlFieldParamNode->GetChildNodes());

              int nestedEnumCounter = 0;
              for (int i = 0;i<EnumFieldNDL->Count;i++)
              {
                xmlGuard <_di_IXMLNode> EnumValueFieldNode (EnumFieldNDL->Get(i));
                if (EnumValueFieldNode->HasAttribute("value"))
                {
                  std::string Value = AnsiString(EnumValueFieldNode->GetAttribute("value")).c_str();
                  std::string Text = AnsiString(EnumValueFieldNode->Text).c_str();
                  Enum[Value] = Text;
                }
              }
              continue;
            }
          }
        }

        OperatorInfo.Fields.push_back(field(Id.c_str(),Name.c_str(),Mask.c_str(),SendUnmasked,Type.c_str(),Enum,Regexp.c_str(),Format.c_str(),URL1.c_str()));
      }
    }
  }// fields
}


void TWConfig::postProcessFieldsNode(xmlGuard <_di_IXMLNode>& xmlOpParamNode)
{
  JSMaker->AddChild("fields");
  if (xmlOpParamNode->HasChildNodes)
  {
    xmlGuard <_di_IXMLNodeList> xmlFieldsNDL (xmlOpParamNode->GetChildNodes());
    for (int i = 0;i<xmlFieldsNDL->GetCount();i++)
    {
        int siblingEnumCounter = 0;

      xmlGuard <_di_IXMLNode> xmlFieldNode (xmlFieldsNDL->Get(i));
      if (xmlFieldNode->NodeName==(AnsiString)"field")
      {
        AnsiString Id;
        AnsiString Name;
        AnsiString URL1 = "";
        AnsiString Mask;
        bool SendUnmasked = false;
        AnsiString Type;
        std::map <std::string, std::string> Enum;
        AnsiString Regexp,Format;

        if (xmlFieldNode->HasAttribute("id"))
          {
          Id = xmlFieldNode->GetAttribute("id");
          }
        JSMaker->AddChild(AnsiString(Id).c_str());

        if (xmlFieldNode->HasAttribute("type"))
          {
          Type = xmlFieldNode->GetAttribute("type");
          }

        xmlGuard <_di_IXMLNodeList> FAttrNDL = xmlFieldNode->AttributeNodes;
        for (int k = 0;k<FAttrNDL->GetCount();k++)
        {
          xmlGuard <_di_IXMLNode> FAttrNode = FAttrNDL->Get(k);
          if ((FAttrNode.Assigned())&&(FAttrNode->IsTextElement)&&(FAttrNode->NodeName!= AnsiString("id")))
          {
            JSMaker->AddStringAttribute(AnsiString(FAttrNode->NodeName).c_str(), (FAttrNode->NodeValue.operator AnsiString()).c_str());
          }
        }

        if (xmlFieldNode->HasChildNodes)
        {

          xmlGuard <_di_IXMLNodeList> xmlFieldNDL (xmlFieldNode->GetChildNodes());
            bool closeChild = false;
          for (int i = 0;i<xmlFieldNDL->Count;i++)
          {
            xmlGuard <_di_IXMLNode> xmlFieldParamNode (xmlFieldNDL->Get(i));
            if (xmlFieldParamNode->IsTextElement)
              JSMaker->AddStringAttribute(AnsiString(xmlFieldParamNode->NodeName).c_str(), (xmlFieldParamNode->NodeValue.operator AnsiString()).c_str());

            if (xmlFieldParamNode->NodeName==(AnsiString)"name")
            {
              Name = xmlFieldParamNode->GetNodeValue();
              continue;
            }

            if (xmlFieldParamNode->NodeName == AnsiString("url_svyaznoy"))
            {
              URL1 = xmlFieldParamNode->GetNodeValue();
              continue;
            }

            if(xmlFieldParamNode->NodeName == (AnsiString)"cheque_mask")
            {
                if (xmlFieldParamNode->HasAttribute("regexp"))
                    Regexp = xmlFieldParamNode->GetAttribute("regexp");
                if (xmlFieldParamNode->HasAttribute("format"))
                    Format = xmlFieldParamNode->GetAttribute("format");
            }

            if (xmlFieldParamNode->NodeName==(AnsiString)"mask")
            {
              Mask = xmlFieldParamNode->GetNodeValue();
              if (xmlFieldParamNode->HasAttribute("send_unmasked"))
              {
                AnsiString MaskFlag = (xmlFieldParamNode->GetAttribute("send_unmasked")).VOleStr;
                SendUnmasked = ((MaskFlag == "1")||(MaskFlag.LowerCase() == "yes"));
              }
              continue;
            }
            if ((xmlFieldParamNode->NodeName==(AnsiString)"enum")&&(Type == AnsiString("enum")))
            {
              // смотрим есть ли parentId parentValue
              JSMaker->AddChild("enum");

              xmlGuard <_di_IXMLNodeList> EnumFieldNDL (xmlFieldParamNode->GetChildNodes());

              int nestedEnumCounter = 0;
              for (int i = 0;i<EnumFieldNDL->Count;i++)
              {
                xmlGuard <_di_IXMLNode> EnumValueFieldNode (EnumFieldNDL->Get(i));

                // обрабатываем обычное пречисление
                if (EnumValueFieldNode->HasAttribute("value"))
                {
                  std::string Value = AnsiString(EnumValueFieldNode->GetAttribute("value")).c_str();
                  std::string Text = AnsiString(EnumValueFieldNode->Text).c_str();
                  JSMaker->AddStringAttribute(Value.c_str(), Text.c_str());
                }
                else
                {
                    // проверим, есть ли вложенные элементы
                    if(EnumValueFieldNode->HasChildNodes)
                    {
                        std::string branchName = "";
                        /*
                        if(EnumValueFieldNode->HasAttribute("parentValue"))
                            branchName = AnsiString(EnumValueFieldNode->GetAttribute("parentValue")).c_str();
                        else
                            branchName = (boost::format("enum%1%") % ++nestedEnumCounter).str();
                        */
                        branchName = (boost::format("enum%1%") % ++nestedEnumCounter).str();
                        processNestedEnum(EnumValueFieldNode, Enum, branchName);
                    }
                }
              }
              JSMaker->CloseChild();    // enum
              continue;
            }
          }
        }
        JSMaker->CloseChild();
      }
    }
  }// fields
  JSMaker->CloseChild();
}

void TWConfig::processNestedEnum(xmlGuard <_di_IXMLNode>& enumNode, std::map<std::string, std::string>& Enum, const std::string &branchName)
{
    JSMaker->AddChild(branchName.c_str());
    for (int nodeIndex = 0; nodeIndex < enumNode->GetChildNodes()->GetCount(); nodeIndex++)
    {
        xmlGuard <_di_IXMLNode> nestedNode(enumNode->GetChildNodes()->Get(nodeIndex));

        if(nestedNode->NodeName == (AnsiString)"enum")
        {
            JSMaker->AddStringAttribute("type", nestedNode->NodeName);
            JSMaker->AddChild("values");

            xmlGuard<_di_IXMLNodeList> nestedEnumList(nestedNode->GetChildNodes());

            // enum делаем по-старому. через жопу
            if(nestedNode->NodeName == AnsiString("enum"))
            {
                for(int enumIndex = 0; enumIndex < nestedEnumList->GetCount(); ++enumIndex)
                {
                    xmlGuard<_di_IXMLNode> nestedEnumNode(nestedEnumList->Get(enumIndex));
                    if (nestedEnumNode->HasAttribute("value"))
                    {
                      std::string value = AnsiString(nestedEnumNode->GetAttribute("value")).c_str();
                      std::string text = AnsiString(nestedEnumNode->Text).c_str();
                      JSMaker->AddStringAttribute(value.c_str(), text.c_str());
                      Enum[value] = text;
                    }
                }
            }
            // остальные поля нормально
            else
            {
                for(int enumIndex = 0; enumIndex < nestedEnumList->GetCount(); ++enumIndex)
                {
                    xmlGuard<_di_IXMLNode> nestedEnumNode(nestedEnumList->Get(enumIndex));
                    JSMaker->AddStringAttribute(nestedEnumNode->NodeName, nestedEnumNode->Text);
                }
            }

            JSMaker->CloseChild();
        }
        else
        {
            JSMaker->AddStringAttribute(nestedNode->NodeName, AnsiString(nestedNode->Text).c_str());
        }
/*
        if(nestedNode->NodeName == (AnsiString)"name")
        {
            JSMaker->AddStringAttribute("name", AnsiString(nestedNode->Text).c_str());
            continue;
        }
        else
        {
            JSMaker->AddStringAttribute("type", nestedNode->NodeName);
            JSMaker->AddChild("values");

            xmlGuard<_di_IXMLNodeList> nestedEnumList(nestedNode->GetChildNodes());

            // enum делаем по-старому. через жопу
            if(nestedNode->NodeName == AnsiString("enum"))
            {
                for(int enumIndex = 0; enumIndex < nestedEnumList->GetCount(); ++enumIndex)
                {
                    xmlGuard<_di_IXMLNode> nestedEnumNode(nestedEnumList->Get(enumIndex));
                    if (nestedEnumNode->HasAttribute("value"))
                    {
                      std::string value = AnsiString(nestedEnumNode->GetAttribute("value")).c_str();
                      std::string text = AnsiString(nestedEnumNode->Text).c_str();
                      JSMaker->AddStringAttribute(value.c_str(), text.c_str());
                      Enum[value] = text;
                    }
                }
            }
            // остальные поля нормально
            else
            {
                for(int enumIndex = 0; enumIndex < nestedEnumList->GetCount(); ++enumIndex)
                {
                    xmlGuard<_di_IXMLNode> nestedEnumNode(nestedEnumList->Get(enumIndex));
                    JSMaker->AddStringAttribute(nestedEnumNode->NodeName, nestedEnumNode->Text);
                }
            }

            JSMaker->CloseChild();
        }
        */
    }
    JSMaker->CloseChild();
}
//---------------------------------------------------------------------------

void TWConfig::GetComission(TOpInfo& OperatorInfo, xmlGuard <_di_IXMLNode>& xmlComissionNode)
{
    try
    {
        if (!xmlComissionNode.Assigned())
        {
            OperatorInfo.CommissionInfo.push_back(comm_info(false,0,0,0,0));
        }
        else
        {
            if (xmlComissionNode->HasChildNodes)
            {
                xmlGuard <_di_IXMLNodeList> PartsNDL (xmlComissionNode->GetChildNodes());

                if (PartsNDL->GetCount() == 0)
                {
                    OperatorInfo.CommissionInfo.push_back(comm_info(false,0,0,0,0));
                }
                else
                {
                    for (int i = 0;((i<PartsNDL->GetCount())&&(i<200));i++)
                    {
                        xmlGuard <_di_IXMLNode> PartNode (PartsNDL->Get(i));

                        if ((PartNode.Assigned())&&(PartNode->NodeName == AnsiString("part")))
                        {
                            int iMin      = 0;
                            double dValue = 0;
                            int iMinTime  = 0;
                            int iMinDay   = 0;
                            bool bRelative= false;

                            if (PartNode->HasAttribute("min")) {
                                iMin = ::GetDouble((PartNode->GetAttribute("min")).VOleStr);
                            }

                            if (PartNode->HasAttribute("time")) {
                                iMinTime = ::GetDouble((PartNode->GetAttribute("time")).VOleStr);
                            }

                            if (PartNode->HasAttribute("day")) {
                                iMinDay = ::GetDouble((PartNode->GetAttribute("day")).VOleStr);
                            }

                            if (PartNode->HasAttribute("value")) {
                                AnsiString Temp = (PartNode->GetAttribute("value")).VOleStr;

                                if (Temp.Pos("%")!= 0) {
                                    dValue = ::GetDouble(Temp.SubString(0,Temp.Pos("%")-1));
                                    bRelative = true;
                                } else {
                                    dValue = ::GetDouble(Temp);
                                    bRelative = false;
                                }

                                // если в конфиге отрицательное значение комиссии то делаем ее нулевой
                                if(dValue < 0)
                                   dValue = 0;
                            }

                            OperatorInfo.CommissionInfo.push_back(comm_info(bRelative,dValue,iMin,iMinTime,iMinDay));
                        } //if
                    } //for
                    SortComission(OperatorInfo);
                } //else
            }
            else
            {
                int iMin       = 0;
                double dValue  = 0;
                int iMinTime   = 0;
                int iMinDay    = 0;
                bool bRelative = false;

                if (xmlComissionNode.Assigned())
                {
                    if (xmlComissionNode->HasAttribute("amount"))
                    {
                        dValue = ::GetDouble((xmlComissionNode->GetAttribute("amount")).VOleStr);
                        bRelative = true;

                    }
                    else if ((xmlComissionNode.Assigned())&&(xmlComissionNode->HasAttribute("relative")))
                    {
                        dValue = ::GetDouble((xmlComissionNode->GetAttribute("relative")).VOleStr);
                        bRelative = true;

                    }
                    else if ((xmlComissionNode.Assigned())&&(xmlComissionNode->HasAttribute("absolute")))
                    {
                        dValue = ::GetDouble((xmlComissionNode->GetAttribute("absolute")).VOleStr);
                        bRelative = false;

                    }
                    else if ((xmlComissionNode.Assigned())&&(xmlComissionNode->HasAttribute("value")))
                    {
                        AnsiString Temp = (xmlComissionNode->GetAttribute("value")).VOleStr;

                        if (Temp.Pos("%")!= 0)
                        {
                            dValue = ::GetDouble(Temp.SubString(0,Temp.Pos("%")-1));
                            bRelative = true;
                        }
                        else
                        {
                            dValue = ::GetDouble(Temp);
                            bRelative = true;
                        }

                    }
                    // если в конфиге отрицательное значение комиссии то делаем ее нулевой
                    if(dValue < 0.0)
                        dValue = 0;
                }
                OperatorInfo.CommissionInfo.push_back(comm_info(bRelative,dValue,iMin,iMinTime,iMinDay));
            } //else
        }
    /*Log->Write("op #"+AnsiString(OperatorInfo.Id)+": size:"+AnsiString(OperatorInfo.CommissionInfo.size()));
    for (unsigned int i=0;i < OperatorInfo.CommissionInfo.size();i++)
    {
      Log->Write("\ti:"+AnsiString(i) + "\t" + OperatorInfo.CommissionInfo[i].GetString());
    }*/
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TWConfig::SortComission(TOpInfo& OperatorInfo)
{
try
  {
  for (unsigned int i = 1;i <= OperatorInfo.CommissionInfo.size();i++)
    {
    for (unsigned int j = 1;j <= OperatorInfo.CommissionInfo.size()-i;j++)
      {
 			if (OperatorInfo.CommissionInfo[j].MinDay < OperatorInfo.CommissionInfo[j-1].MinDay)
        {
        comm_info Temp;
        Temp = OperatorInfo.CommissionInfo[j];
        OperatorInfo.CommissionInfo[j] = OperatorInfo.CommissionInfo[j-1];
        OperatorInfo.CommissionInfo[j-1] = Temp;
        }
        else
        {
        if (OperatorInfo.CommissionInfo[j].MinDay == OperatorInfo.CommissionInfo[j-1].MinDay)
          {
          if (OperatorInfo.CommissionInfo[j].MinTime < OperatorInfo.CommissionInfo[j-1].MinTime)
            {
            comm_info Temp;
            Temp = OperatorInfo.CommissionInfo[j];
            OperatorInfo.CommissionInfo[j] = OperatorInfo.CommissionInfo[j-1];
            OperatorInfo.CommissionInfo[j-1] = Temp;
            }
            else
            {
            if (OperatorInfo.CommissionInfo[j].MinTime == OperatorInfo.CommissionInfo[j-1].MinTime)
              {
              if (OperatorInfo.CommissionInfo[j].Min < OperatorInfo.CommissionInfo[j-1].Min)
                {
                comm_info Temp;
                Temp = OperatorInfo.CommissionInfo[j];
                OperatorInfo.CommissionInfo[j] = OperatorInfo.CommissionInfo[j-1];
                OperatorInfo.CommissionInfo[j-1] = Temp;
                }
              }
            }
          }
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

double TWConfig::GetComission(int _OpId, double Sum, TDateTime DT)
{

    TOpInfo OperatorInfo = Operator(_OpId);

    double Temp = 0;
    unsigned short h,m,s,ms;

    DT.DecodeTime(&h,&m,&s,&ms);

    for (int i = OperatorInfo.CommissionInfo.size()-1;i >= 0;i--) {
        int Day = ((DT.DayOfWeek()!=1) ? DT.DayOfWeek()-1 : 7);
        if ((Sum >= OperatorInfo.CommissionInfo[i].Min)&&(Day >= OperatorInfo.CommissionInfo[i].MinDay)&&((h*60+m) >= OperatorInfo.CommissionInfo[i].MinTime))
        {
            if (OperatorInfo.CommissionInfo[i].Relative)
                Temp = RoundTo((OperatorInfo.CommissionInfo[i].Value/100.0)*Sum,-2);
            else
                Temp = RoundTo(OperatorInfo.CommissionInfo[i].Value,-2);
            break;
         }
    }

    double Amount = Sum-Temp;

    if (Amount<0)
        Amount = 0;

    switch (OperatorInfo.RoundAmount)
    {
        case RUp:
            Amount = Ceil(Amount);
        break;
        case RDown:
            Amount = Floor(Amount);
        break;
        case RBankers:
            Amount = RoundTo(Amount,-2);
        break;
    }
    Temp = RoundTo(Sum-Amount,-2);
    return Temp;
}

//---------------------------------------------------------------------------

double TWConfig::GetPaymentMinSum(int _OpId, double MinSum,const TDateTime& DT)
{
    double DesiredMinSum;
    if (MinSum == 0)
        DesiredMinSum = Operator(_OpId).LimMin;
    else
        DesiredMinSum = MinSum;
    return DesiredMinSum;
    /*
    int i=1;
    while(true)
    {
        if (i*Terminal.NoteMinAmount-GetComission(_OpId, double(i*Terminal.NoteMinAmount), DT) >= DesiredMinSum)
            return i*Terminal.NoteMinAmount;
        i++;
    }
    */
    //return Operator(_OpId).LimMin;
}

//---------------------------------------------------------------------------

AnsiString TWConfig::GetPath(AnsiString _URL)
{
AnsiString Temp;
Temp = _URL;
if (Temp.Pos("//")!= 0)
		Temp.Delete(1,Temp.Pos("//")+1);
return "http://"+Temp;
}

//---------------------------------------------------------------------------

AnsiString TWConfig::GetHost(AnsiString _URL)
{
AnsiString Temp;
Temp = _URL;
if (Temp.Pos("//")!= 0)
    Temp.Delete(1,Temp.Pos("//")+1);
if (Temp.Pos("/")!= 0)
    Temp.Delete(Temp.Pos("/"),Temp.Length());
return Temp;
}

//---------------------------------------------------------------------------

AnsiString __fastcall TWConfig::Strip(AnsiString S)
{
return S.Delete(1, 2);
}

//---------------------------------------------------------------------------

AnsiString TWConfig::md5ToString(AnsiString S)
{
AnsiString AHex;
for (int i = 1; i <= S.Length(); i++){
    AHex += IntToHex((unsigned char)S[i], 2);
		}
return AHex;
}

//---------------------------------------------------------------------------

AnsiString TWConfig::Md5Str(AnsiString src)
{
    try
    {
        std::auto_ptr <TIdCoderMD5> md5 (new TIdCoderMD5(NULL));
        md5->Reset();
        md5->AutoCompleteInput = true;
        return md5ToString(Strip(md5->CodeString(src)));
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return "";
}

//---------------------------------------------------------------------------

bool TWConfig::IsOffline(int _OpNum, AnsiString& OfflinePeriodMessage)
{
    TOpInfo OperatorInfo = Operator(_OpNum);
    if(OperatorInfo.Offline)
        return true;
    if ((Payments.OfflinePeriodBegin.Val!=0)&&(Payments.OfflinePeriodEnd.Val!=0))
    {
        OfflinePeriodMessage = " [" + Payments.OfflinePeriodBegin.FormatString("hh:mm") + "-" + Payments.OfflinePeriodEnd.FormatString("hh:mm") + "]";
        TDateTime DT;
        DT=TDateTime::CurrentTime();
        if ((Payments.OfflinePeriodBegin<=Payments.OfflinePeriodEnd)&&(DT>Payments.OfflinePeriodBegin)&&(DT<Payments.OfflinePeriodEnd))
            return true;
        else
            if ((Payments.OfflinePeriodBegin > Payments.OfflinePeriodEnd)&&((DT>Payments.OfflinePeriodBegin)||(DT<Payments.OfflinePeriodEnd)))
                return true;
    }
    return false;
}

//---------------------------------------------------------------------------

bool TWConfig::IsMoneyOffline()
{
if ((Payments.OfflineMoneyPeriodBegin.Val!= 0)&&(Payments.OfflineMoneyPeriodEnd.Val!= 0)) {
		if ((Payments.OfflineMoneyPeriodBegin <= Payments.OfflineMoneyPeriodEnd)&&(TDateTime::CurrentDateTime()>Payments.OfflineMoneyPeriodBegin)&&(TDateTime::CurrentDateTime()<Payments.OfflineMoneyPeriodEnd))
				return true;
				else
				if ((Payments.OfflineMoneyPeriodBegin>Payments.OfflineMoneyPeriodEnd)&&(((TDateTime::CurrentDateTime()>Payments.OfflineMoneyPeriodBegin)&&(TDateTime::CurrentDateTime().Val<1))||((TDateTime::CurrentDateTime().Val>0)&&(TDateTime::CurrentDateTime()<Payments.OfflineMoneyPeriodEnd))))
						return true;
		}
return false;
}

//---------------------------------------------------------------------------

AnsiString TWConfig::GetPath(void)
{
char path_buf[_MAX_PATH];
char drive[_MAX_DRIVE];
char dir[_MAX_DIR];
char fname[_MAX_FNAME];

::GetModuleFileName(NULL,path_buf,sizeof path_buf);
_splitpath(path_buf,drive,dir,fname,0);
_makepath (path_buf,drive,dir,0,0);
return AnsiString(path_buf);
}

//---------------------------------------------------------------------------

bool TWConfig::IsExist(AnsiString NodeName, xmlGuard <_di_IXMLNode>& xmlParentNode)
{
  try
  {
		if (xmlParentNode.Assigned())
    {
      if (xmlParentNode->HasChildNodes)
      {
				xmlGuard <_di_IXMLNodeList> xmlNodeList (xmlParentNode->GetChildNodes());
  			xmlGuard <_di_IXMLNode> xmlNode (xmlNodeList->FindNode(NodeName));
    		if (xmlNode.Assigned())
          return true;
      }
    }
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
	}
  return false;
}

//---------------------------------------------------------------------------

int TWConfig::GetInt(bool bOverWrite, const char* NodeName, xmlGuard <_di_IXMLNode>& xmlParentNode, int Default)
{
    try
    {
        return StringToInt(GetChildNodeValue(bOverWrite, NodeName, xmlParentNode, boost::lexical_cast<std::string>(Default).c_str()), Default);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Log->Write((boost::format("Can't get integer value from %1%, setting it to %2%") % NodeName % Default).str().c_str());
        return Default;
    }
}

//---------------------------------------------------------------------------

double TWConfig::GetDouble(bool bOverWrite, const char* NodeName, xmlGuard <_di_IXMLNode>& xmlParentNode, double Default)
{
    std::string Temp;
    try
    {
        Temp = GetChildNodeValue(bOverWrite, NodeName, xmlParentNode, boost::lexical_cast<std::string>(Default).c_str());
        return StringToDouble(Temp);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return Default;
    }
}

//---------------------------------------------------------------------------

std::string TWConfig::GetChildNodeValue(bool bOverWrite, const char* NodeName, xmlGuard <_di_IXMLNode>& xmlParentNode, const char* Default, bool isNeedTrim)
{
    std::string default_str = Default;
    try
    {
  	    if (xmlParentNode.Assigned())
        {
            xmlGuard <_di_IXMLNodeList> xmlNodeList (xmlParentNode->GetChildNodes());
            xmlGuard <_di_IXMLNode> xmlNode (xmlNodeList->FindNode(NodeName));
            if (xmlNode.Assigned())
            {
                if (bOverWrite)
                    SetNodeValue(xmlNode, Default);
                return GetNodeValue(xmlNode, default_str, isNeedTrim);
            }
            else
            {
                if (bOverWrite)
                {
                    CreateNode(NodeName, xmlParentNode, Default);
                    xmlGuard <_di_IXMLNode> xmlNode (xmlNodeList->FindNode(NodeName));
                }
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return default_str;
}

//---------------------------------------------------------------------------

std::string TWConfig::GetChildNodeValue(bool bOverWrite, const char* NodeName, xmlGuard <_di_IXMLNode>& xmlParentNode, int Default, bool isNeedTrim)
{
  return GetChildNodeValue(bOverWrite, NodeName, xmlParentNode, boost::lexical_cast<std::string>(Default), isNeedTrim);
}

//---------------------------------------------------------------------------

std::string TWConfig::GetChildNodeValue(bool bOverWrite, const char* NodeName, xmlGuard <_di_IXMLNode>& xmlParentNode, double Default, bool isNeedTrim)
{
  return GetChildNodeValue(bOverWrite, NodeName, xmlParentNode, boost::lexical_cast<std::string>(Default), isNeedTrim);
}

//---------------------------------------------------------------------------

std::string TWConfig::GetChildNodeValue(bool bOverWrite, const char* NodeName, xmlGuard <_di_IXMLNode>& xmlParentNode, std::string Default, bool isNeedTrim)
{
  return GetChildNodeValue(bOverWrite, NodeName, xmlParentNode, Default.c_str(), isNeedTrim);
}

//---------------------------------------------------------------------------

std::string TWConfig::GetNodeValue(xmlGuard <_di_IXMLNode>& xmlNode, const std::string& defaultValue, bool isNeedTrim)
{
    try
    {
        std::string result = WCharToString((xmlNode->NodeValue).VOleStr);
        if(isNeedTrim)
            result = mytrim(result);
        return result;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

int TWConfig::GetNodeValue(xmlGuard <_di_IXMLNode>& xmlNode, const int defaultValue, bool isNeedTrim)
{
    return StringToInt(GetNodeValue(xmlNode, boost::lexical_cast<std::string>(defaultValue)), isNeedTrim);
}

//---------------------------------------------------------------------------

double TWConfig::GetNodeValue(xmlGuard <_di_IXMLNode>& xmlNode, const double defaultValue, bool isNeedTrim)
{
    return StringToInt(GetNodeValue(xmlNode, boost::lexical_cast<std::string>(defaultValue)), isNeedTrim);
}

//---------------------------------------------------------------------------

bool TWConfig::GetNodeValue(xmlGuard <_di_IXMLNode>& xmlNode, const bool defaultValue, bool isNeedTrim)
{
    return StringToInt(GetNodeValue(xmlNode, boost::lexical_cast<std::string>(static_cast<int>(defaultValue))), isNeedTrim);
}

//---------------------------------------------------------------------------
_di_IXMLNode TWConfig::CreateNode(AnsiString NodeName,xmlGuard <_di_IXMLNode>& xmlParentNode, AnsiString Value)
{
_di_IXMLNode xmlNode;
try
		{
		Log->Write((boost::format("Creating node: %1% = %2%") % NodeName.c_str() % Value.c_str()).str().c_str());
		xmlNode = xmlParentNode->AddChild(NodeName);
    if (Value!= "")
       xmlNode->Text = Value;
    Log->Append("OK.");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return xmlNode;
}

//---------------------------------------------------------------------------

void TWConfig::DeleteNode(AnsiString NodeName, xmlGuard <_di_IXMLNode>& xmlParentNode)
{
try
    {
		xmlGuard <_di_IXMLNodeList> NDL (xmlParentNode->GetChildNodes());
		int res = NDL->Delete(NodeName);
		if (res!= -1)
  		Log->Write((boost::format("Node %1% deleted.") % NodeName.c_str()).str().c_str());
		}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

bool TWConfig::SetNodeValue(xmlGuard <_di_IXMLNode>& xmlNode, AnsiString Value)
{
  try
 	{
    if (xmlNode.Assigned())
    {
      if ((xmlNode->Text == (AnsiString)"")&&(xmlNode->Text != Value))
        Log->Write((boost::format("Setting '%1%' to '%2%'...") % AnsiString(xmlNode->NodeName).c_str() % Value.c_str()).str().c_str());
      else
      {
        if (xmlNode->Text != Value)
          Log->Write((boost::format("Changing '%1%' from '%2%' to '%3%'...") % AnsiString(xmlNode->NodeName).c_str() % AnsiString(xmlNode->Text).c_str() % Value.c_str()).str().c_str());
        else
          return true;
      }

      xmlNode->Text = Value;
      Log->Append("OK.");
      return true;
    }
    else
    {
      Log->Append("Error: Node not assigned!");
    }
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return false;
}

//---------------------------------------------------------------------------

_di_IXMLNode TWConfig::GetNode(AnsiString NodeName, xmlGuard <_di_IXMLNode>& xmlParentNode, bool bNoCreate)
{
  _di_IXMLNode xmlNode = NULL;
  try
  {
    if (xmlParentNode.Assigned())
    {
      xmlGuard <_di_IXMLNodeList> xmlNodeList (xmlParentNode->GetChildNodes());
      xmlNode = xmlNodeList->FindNode(NodeName);
      if ((xmlNode==NULL)&&(!bNoCreate))
      {
        CreateNode(NodeName, xmlParentNode,"");
        xmlNode = xmlNodeList->FindNode(NodeName);
      }
    }
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return xmlNode;
}

//---------------------------------------------------------------------------

bool TWConfig::SetAttribute(AnsiString AttribName, xmlGuard <_di_IXMLNode>& xmlNode, AnsiString Value)
{
  try
  {
    if (xmlNode->HasAttribute(AttribName))
    {
      if (xmlNode->GetAttribute(AttribName)!=Value)
        Log->Write((boost::format("Changing attribute '%1%' from '%2%' to '%3%'...") % AttribName.c_str() % AnsiString(xmlNode->GetAttribute(AttribName)).c_str() % Value.c_str()).str().c_str());
      else
        return true;
    }
    else
      Log->Write((boost::format("Setting attribute '%1%' to '%2%'...") % AttribName.c_str() % Value.c_str()).str().c_str());


		if (xmlNode.Assigned())
    {
			xmlNode->SetAttribute(AttribName,Value);
      Log->Append("OK.");
      return true;
    }
		else
    {
      Log->Append("Error: Node not assigned.");
    }
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return false;
}

bool TWConfig::DeleteAttribute(AnsiString AttribName, xmlGuard <_di_IXMLNode>& xmlNode)
{
    bool bResult = false;
    try
    {
        Log->Write((boost::format("Deleting attribute: %1%...") % AttribName.c_str()).str().c_str());
        if (xmlNode.Assigned())
        {
            xmlGuard <_di_IXMLNodeList> Attributes = xmlNode->AttributeNodes;
            bResult = Attributes->Delete(AttribName);
            Log->Append("OK.");
        }
        else
        {
            Log->Append("Error: Node == NULL.");
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return bResult;
}

//---------------------------------------------------------------------------
/*
AnsiString TWConfig::GetStringAttribute(bool bOverWrite, const std::string AttribName&, xmlGuard <_di_IXMLNode>& xmlParentNode, const std::string& Default, bool bNoCreate)
{
    return GetStringAttribute(bOverWrite,AttribName.c_str(),xmlParentNode,Default.c_str(),bNoCreate);
}

AnsiString TWConfig::GetStringAttribute(bool bOverWrite, const AnsiString& AttribName, xmlGuard <_di_IXMLNode>& xmlParentNode, const AnsiString& Default, bool bNoCreate)
{
    return GetStringAttribute(bOverWrite,AttribName.c_str(),xmlParentNode,Default.c_str(),bNoCreate);
}
*/
std::string TWConfig::GetStringAttribute(bool bOverWrite, const char* AttribName, xmlGuard <_di_IXMLNode>& xmlParentNode, const char* Default, bool bNoCreate)
{
    try
    {
        if (xmlParentNode.Assigned())
        {
            if (bOverWrite)
            {
                SetAttribute(AttribName,xmlParentNode,Default);
            }
            else
            {
                if (xmlParentNode->HasAttribute(AttribName)) {
                    AnsiString p = AnsiString(xmlParentNode->GetAttribute(AttribName));
                    std::string retdata = AnsiString(xmlParentNode->GetAttribute(AttribName)).c_str();
                    return retdata;
                } else {
                    if (!bNoCreate)
                        SetAttribute(AttribName,xmlParentNode,Default);
                }
            }
        }
        return Default;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return Default;
    }
}
//---------------------------------------------------------------------------
bool TWConfig::IsAttributeExists(AnsiString AttribName, xmlGuard <_di_IXMLNode>& xmlParentNode)
{
try
  {
  if (xmlParentNode.Assigned())
    {
    return xmlParentNode->HasAttribute(AttribName);
    }
  return false;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
      return false;
    }
}
//---------------------------------------------------------------------------
int TWConfig::GetIntAttribute(bool bOverWrite, const char* AttribName, xmlGuard <_di_IXMLNode>& xmlParentNode, int Default, bool bNoCreate)
{
    try {
        std::string Temp = GetStringAttribute(bOverWrite, AttribName, xmlParentNode, (boost::format("%1%") % Default).str().c_str(), bNoCreate);
        if(Temp != "")
            return boost::lexical_cast<int>(Temp);
        else
            return Default;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Log->Write((boost::format("Can't get integer value from attribute %1%, node %2%, setting it to %3%") % AttribName % AnsiString(xmlParentNode->NodeName).c_str() % Default).str().c_str());
        return Default;
    }
}
//---------------------------------------------------------------------------
long TWConfig::GetLongAttribute(bool bOverWrite, const char* AttribName, xmlGuard <_di_IXMLNode>& xmlParentNode, long Default, bool bNoCreate)
{
    try
    {
        std::string Temp = GetStringAttribute(bOverWrite, AttribName, xmlParentNode, (boost::format("%1%") % Default).str().c_str(), bNoCreate);
        if(Temp != "")
            return boost::lexical_cast<long>(Temp);
        else
            return Default;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Log->Write((boost::format("Can't get integer value from attribute %1%, node %2%, setting it to %3%") % AttribName % AnsiString(xmlParentNode->NodeName).c_str() % Default).str().c_str());
        return Default;
    }
}
//---------------------------------------------------------------------------
double TWConfig::GetDoubleAttribute(bool bOverWrite, const char* AttribName, xmlGuard <_di_IXMLNode>& xmlParentNode, double Default, bool bNoCreate)
{
    std::string Temp;
    try {
        Temp = GetStringAttribute(bOverWrite, AttribName, xmlParentNode, (boost::format("%1%") % Default).str().c_str(), bNoCreate);
        Temp = ChangeChars(Temp.c_str(), ",", ".").c_str();
        return boost::lexical_cast<double>(Temp);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Log->Write((boost::format("Can't get double value from attribute %1%, node %2%, setting it to %3%") % AttribName % AnsiString(xmlParentNode->NodeName).c_str() % Default).str().c_str());
        return Default;
    }
}
//---------------------------------------------------------------------------
TDateTime TWConfig::GetTimeAttribute(bool bOverWrite, const char* AttribName, xmlGuard <_di_IXMLNode>& xmlParentNode, TDateTime Default, bool bNoCreate)
{
    AnsiString Temp;
    try {
        Temp = GetStringAttribute(bOverWrite, AttribName, xmlParentNode, Default.FormatString("hh:mm").c_str(), bNoCreate).c_str();
        return TDateTime(Temp);
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        Log->Write((boost::format("Can't get time from attribute %1%: %2%, node %3%, setting it to %4%") % AttribName % Temp.c_str() % AnsiString(xmlParentNode->NodeName).c_str() % AnsiString(Default).c_str()).str().c_str());
        return Default;
    }
}
//---------------------------------------------------------------------------
void TWConfig::SetDefaultValues()
{
try
  {
	StatInfo.StatServerPollInterval = 10;
//	StatInfo->StatServerSrc = "none";
	//StatInfo->Host = "none";
	//StatInfo->Port = 10024;
	for (int i = 0;i<MaxStatServersAdresses;i++)
		{
		StatInfo.Host[i] = "";
		StatInfo.Port[i] = 10024;
		}
	StatInfo.Host[0] = "none";
	StatInfo.Port[0] = 10024;
	StatInfo.StatServersAdressesCount = 0;
	StatInfo.CurrentStatServersAdressNum = 0;

  StatInfo.Inhibit = true;
  StatInfo.NoFileSending = false;
  StatInfo.CompressFiles = true;
  StatInfo.InhibitHeartBeats = false;
  //int MaxMin = max(30,(RequestTime*5));
	//StatInfo->DTServerTimeOutDiff = double(MaxMin)/24/60;
	StatInfo.DTServerTimeOutDiff = 0;
	StatInfo.ProcessorType = cnMonitoringServer;
	StatInfo.HeartBeatURL = "http://monitor.cyberplat.com/cgi-bin/tm/send_heartbeat.cgi";
	StatInfo.PaymentCompleteURL = "http://monitor.cyberplat.com/cgi-bin/tm/send_badpayments.cgi";
	StatInfo.IncassationURL = "http://monitor.cyberplat.com/cgi-bin/tm/send_collect.cgi";
	StatInfo.DownloadURLPrefix = "http://monitor.cyberplat.com";
  ServiceInfo.PaymentBookURL = "http://mon_api3.cyberplat.com/cgi-bin/nb/index.pl";
    StatInfo.IsSignStatistics = false;

  m_LocaleName = "ru";

  GetKeysURL = "http://card.cyberplat.ru/cgi-bin/java_keys.cgi";
  Keys.push_back(_keys_info(0,0,"","","",".\\keys\\pubkeys.key",904291,".\\keys\\secret.key","0"));

	Payments.Offline = false;
  Payments.Rest = 0; //остаток после платежа
  Payments.ErrorPaymentsDeleteHours = 72;

  Payments.OfflinePeriodBegin = 0;
  Payments.OfflinePeriodEnd = 0;

  Payments.OfflineMoneyPeriodBegin = 0;
  Payments.OfflineMoneyPeriodEnd = 0;
	Payments.UpdateCardsInfo = 0;

        Peripherals.Keyboard.Type = "";
        Peripherals.Keyboard.Port = 0;
//        Peripherals.Keyboard.ortBaudRate = 0;

        Peripherals.CardReader.Type = "";
        Peripherals.CardReader.Port = 0;
        Peripherals.CardReader.PortBaudRate = 0;
        Peripherals.CardReader.SystemCode = 0;
        Peripherals.CardReader.DeviceCode = 0;
        Peripherals.CardReader.FreeseOnError = false;

        ServiceInfo.Number = "9991234567";
        ServiceInfo.OperatorId = 0;
        ServiceInfo.ServiceMenuPasswordMaskSrc = "";
        ServiceInfo.ServiceMenuPasswordMask = "";
        ServiceInfo.ServiceMenuItems = "";
        ServiceInfo.ServiceShortMenuPasswordMaskSrc = "";
        ServiceInfo.ServiceShortMenuPasswordMask = "";
        ServiceInfo.ServiceShortMenuItems = "";
        ServiceInfo.IncassationNumberMaskSrc = "";
        ServiceInfo.IncassationNumberMask = "";
        ServiceInfo.OutOfOrderPassword = "";
        ChequeCaption.clear();
        ChequeCaption.push_back("       Телефон службы поддержки");
        ChequeCaption.push_back("           (495)***-**-**");
        ChequeCaption.push_back("           ООО ** *******");

        Terminal.Number = Keys[0].AP;

        Dirs.WorkDir = (boost::format("%1%\\") % GetCurrentDir().c_str()).str();
        Dirs.PaymentsOutbound = Dirs.WorkDir+"outbound\\payments";
        Dirs.PaymentsOutboundTemp = Dirs.WorkDir+"outbound\\payments\\temp";
        Dirs.PaymentsOutboundCanceled = Dirs.WorkDir+"outbound\\payments\\canceled";
        Dirs.StatOutbound = Dirs.WorkDir+"outbound\\stat";
        Dirs.StatOutboundTemp = Dirs.WorkDir+"outbound\\stat\\temp";
        Dirs.EMailOutbound = Dirs.WorkDir+"outbound\\email";
        Dirs.EMailOutboundTemp = Dirs.WorkDir+"outbound\\email\\temp";
        Dirs.EMailOutboundExt = Dirs.WorkDir+"outbound\\email\\ext";
        Dirs.EMailOutboundExtTemp = Dirs.WorkDir+"outbound\\email\\ext\\temp";
        Dirs.SMSOutbound = Dirs.WorkDir+"outbound\\sms";
        Dirs.SMSOutboundTemp = Dirs.WorkDir+"outbound\\sms\\temp";
        Dirs.StatusFileName = Dirs.WorkDir+"config\\Details.xml";
        Dirs.CommandsInbound = Dirs.WorkDir+"inbound";
        Dirs.InterfaceDir = ".\\interface";
        Dirs.InterfaceSkinName = "default";
        Dirs.DBNumCapacityUpdateAddress = "/update/db_numcapacity.7z";
        Dirs.MTRoot = Dirs.WorkDir+"mt_data";

	CurrencyInfo.Currency = "RUR";
	CurrencyInfo.ExchangeRate = 1;
	CurrencyInfo.CurrencyName = "руб";
	CurrencyInfo.currencyId = 810;

	Peripherals.Validator.Type = "none";
	Peripherals.Validator.Protocol = "CCNet";
	Peripherals.Validator.Port = 1;
    Peripherals.Validator.PortBaudRate = 0;
	Peripherals.Validator.StackerOpenCommand = cnSOIncassation;
	Peripherals.Validator.BillsSensitivity = "222222";
    Peripherals.Validator.ReportBillCount = "";

	Peripherals.Modem.AutoDial = 0;
	//Peripherals.Modem.ConnectionName = "gprs";
	//Peripherals.Modem.Interval = 60;
  //Peripherals.Modem.ConnectionCheckTimeOutDT = float(15)/24/60;
	//Peripherals.Modem.Hosts[0] = "";
	//Peripherals.Modem.Hosts[1] = "";
	Peripherals.Modem.Port = 0;
	Peripherals.Modem.ServiceInfoPort = 0;
  Peripherals.Modem.PortBaudRate = 0;
	Peripherals.Modem.ServiceNumber = "*102#";
	Peripherals.Modem.GetServiceInfoInterval = 1440;
	//Peripherals.Modem.DisconnectTime = 0;
	Peripherals.Modem.FailuresBeforeModemReset = 0;
	Peripherals.Modem.FailuresBeforeTerminalReboot = 10;
	Peripherals.Modem.SendSMSInterval = 0;
  Peripherals.Modem.ServiceAnswerMask = "%ANSWER%|-%ANSWER%";
	//Peripherals.Modem.InitCmd = "";
	//Peripherals.Modem.Login = "";
	//Peripherals.Modem.Password = "";
	//Peripherals.Modem.InitString[0] = "";
	//Peripherals.Modem.InitString[1] = "";

	Peripherals.WatchDog.Type = "none";
	Peripherals.WatchDog.Port = 0;
  Peripherals.WatchDog.PortBaudRate = 0;

	Peripherals.Keyboard.Type = "";
	Peripherals.Keyboard.Port = 0;
  Peripherals.Keyboard.PortBaudRate = 0;

	Peripherals.CardReader.Type = "";
	Peripherals.CardReader.Port = 0;
  Peripherals.CardReader.PortBaudRate = 0;
  Peripherals.CardReader.SystemCode = 0;
  Peripherals.CardReader.DeviceCode = 0;
  Peripherals.CardReader.FreeseOnError = false;

  ServiceInfo.Number = "9991234567";
	ServiceInfo.OperatorId = 0;
  ServiceInfo.ServiceMenuPasswordMaskSrc = "";
  ServiceInfo.ServiceMenuPasswordMask = "";
  ServiceInfo.ServiceMenuItems = "";
  ServiceInfo.ServiceShortMenuPasswordMaskSrc = "";
  ServiceInfo.ServiceShortMenuPasswordMask = "";
  ServiceInfo.ServiceShortMenuItems = "";
	ServiceInfo.IncassationNumberMaskSrc = "";
	ServiceInfo.IncassationNumberMask = "";
	ServiceInfo.OutOfOrderPassword = "";

	//CheckCaption->Clear();
	//CheckCaption->Add("       Телефон службы поддержки");
  //CheckCaption->Add("           (495)***-**-**");
	//CheckCaption->Add("           ООО ** *******");

  ChequeCaption.clear();
  ChequeCaption.push_back("       Телефон службы поддержки");
  ChequeCaption.push_back("           (495)***-**-**");
  ChequeCaption.push_back("           ООО ** *******");


	Terminal.Number = Keys[0].AP;
	/*Terminal->SrcNumber = "";

  Terminal->SupportString = "Tелефон техподдержки: [b]***-****[/b][br]e-mail: [b]support@****.ru[/b]";
	Terminal->MainMenuMarqueeString = "За проведение платежа взимается комиссия.";
	Terminal->NightModePeriodBegin = 0;
	Terminal->NightModePeriodEnd = 0;
	//Terminal->DetectWriteErrors = true;
	Terminal->SetWebclientHighPriority = true;
  Terminal->StayOnTop = true;*/

	Dirs.WorkDir = (GetCurrentDir()+"\\").c_str();
	//Dirs.WorkDir = (Dirs.WorkDir).SubString(0,(Dirs.WorkDir).Length()-NumOfCharsToStrip);
  //Log->Write("WorkDir : "+Dirs.WorkDir);

  Dirs.PaymentsOutbound = Dirs.WorkDir+"outbound\\payments";
  Dirs.PaymentsOutboundTemp = Dirs.WorkDir+"outbound\\payments\\temp";
	Dirs.StatOutbound = Dirs.WorkDir+"outbound\\stat";
  Dirs.StatOutboundTemp = Dirs.WorkDir+"outbound\\stat\\temp";
	Dirs.EMailOutbound = Dirs.WorkDir+"outbound\\email";
	Dirs.EMailOutboundTemp = Dirs.WorkDir+"outbound\\email\\temp";
	Dirs.EMailOutboundExt = Dirs.WorkDir+"outbound\\email\\ext";
	Dirs.EMailOutboundExtTemp = Dirs.WorkDir+"outbound\\email\\ext\\temp";
	Dirs.SMSOutbound = Dirs.WorkDir+"outbound\\sms";
	Dirs.SMSOutboundTemp = Dirs.WorkDir+"outbound\\sms\\temp";
	Dirs.StatusFileName = Dirs.WorkDir+"config\\Details.xml";
	Dirs.CommandsInbound = Dirs.WorkDir+"inbound";
  Dirs.InterfaceDir = ".\\interface";
	Dirs.InterfaceSkinName = "default";
	Dirs.DBNumCapacityUpdateAddress = "/update/db_numcapacity.7z";
	Dirs.MTRoot = Dirs.WorkDir+"mt_data";
	ServiceInfo.BalanceReportFileName = Dirs.WorkDir + "config\\templates\\balance.xml";
   ServiceInfo.IncassReportFileName = Dirs.WorkDir + "config\\templates\\incass.xml";
     ServiceInfo.TestChequeFileName = Dirs.WorkDir + "config\\templates\\testprinter.xml";
	ServiceInfo.IncassReportCount = 1;

/*  DealerInfo.DealerName = "-";
  DealerInfo.DealerAddress = "-";
  DealerInfo.DealerINN = "-";
  DealerInfo.DealerPhone = "-";
  DealerInfo.PointAddress = "-";
  DealerInfo.ContractNumber = "-";
  DealerInfo.BankName = "-";
  DealerInfo.BankBIK = "-";
  DealerInfo.BankPhone = "-";*/

	/*HTTPProxy.CfgType = "none";
  Proxy->Host = "";
	Proxy->Port = 0;
  Proxy->UserID = "";
  Proxy->Password = "";
  Proxy->Version = svNoSocks;
  HTTPProxy.Type = "none";
  HTTPProxy.Host = "";
	HTTPProxy.Port = 0;
  HTTPProxy.UserName = "";
  HTTPProxy.Password = "";*/
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
	}
}

//---------------------------------------------------------------------------

bool TWConfig::ProcessConfigFile(bool bOverWrite, bool bCreateDirs, bool bSaveFile)
{
    bool bResult = true;
    bool bUpdateOldStatServerInfo = false;
    CheckDir(Dirs.PaymentsOutboundCanceled.c_str());
    try
    {
        CS->Acquire();
        try
        {
            if (CfgFile)
            {
                Log->Write("Config file pre-closed.");
                delete CfgFile;
                CfgFile = NULL;
            }

            //_di_IXMLNode xmlNode, xmlSecNode, RootNode;
            //_di_IXMLNodeList xmlRootNodeList, xmlNodeList;
            xmlGuard <_di_IXMLDocument> XmlDoc;
            xmlGuard <_di_IXMLNode> RootNode;
            if (FileExists(ConfigFileName))
            {
                CfgFile = new TFileStream(ConfigFileName, fmOpenReadWrite | fmShareDenyNone);
                if (CfgFile != NULL)
                {
                    Log->Write((boost::format("  File %1% successfully opened.") % ConfigFileName.c_str()).str().c_str());
                }
                else
                {
                    Log->Write((boost::format("Open file error %1%!") % ConfigFileName.c_str()).str().c_str());
                    bResult = false;
                }

                std::vector<char> Buffer(static_cast<std::size_t>(CfgFile->Size)+1, 0);
                CfgFile->Seek(0, soFromBeginning);
                CfgFile->Read(&*Buffer.begin(), static_cast<int>(CfgFile->Size));
                AnsiString FileData = AnsiString(&*Buffer.begin(), static_cast<int>(CfgFile->Size));

                try
                {
                    try
                    {
                        XmlDoc = LoadXMLData(FileData);
                    }
                    catch(...)
                    {
                        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                        FileData = FileData.SubString(0,FileData.Pos("</root>")+6);
                        XmlDoc = LoadXMLData(FileData);
                    }//XmlDoc = LoadXMLData(FileData);
                }
                catch (EDOMParseError &e)
                {
                    Log->Write((boost::format("EDOMParseError exception encountered: %1%") % e.Message.c_str()).str().c_str());
                    bResult = false;
                }
                catch (...)
                {
                    ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                    bResult = false;
                }
                if (bResult)
                {
                    XmlDoc->NodeIndentStr = "        ";
                    XmlDoc->Options = XmlDoc->Options << doNodeAutoIndent;
                    RootNode = XmlDoc->GetDocumentElement();
                    if (!RootNode.Assigned())
                        RootNode = XmlDoc->AddChild("root");
                }
            }
            else
            {
                Log->Write((boost::format("File %1% does not exists, trying to create...") % ConfigFileName.c_str()).str().c_str());
                CfgFile = new TFileStream(ConfigFileName, fmCreate);
                if (CfgFile)
                {
                    delete CfgFile;
                    CfgFile = NULL;
                }
                //Log->Write("Config file closed.");
                CfgFile = new TFileStream(ConfigFileName, fmOpenReadWrite | fmShareDenyNone);
                Log->Append("OK.");
                XmlDoc = NewXMLDocument ();
                XmlDoc->XML->Clear();
                XmlDoc->Active = true;
                XmlDoc->StandAlone = "yes";
                XmlDoc->Encoding = "windows-1251";
                XmlDoc->Options = XmlDoc->Options << doNodeAutoIndent;
                RootNode = XmlDoc->AddChild("root");
            }

    if (bResult)
      {
      JSMaker->AddChild("config");

      //SL = new TStringList();
      //SL->AddStrings(XmlDoc->GetXML());
      //ConfigMD5 = Md5Str(SL->DelimitedText);
      ConfigMD5 = Md5Str(AnsiString(FileDateToDateTime(FileAge(ConfigFileName))) + " " + AnsiString(FileSizeByName(ConfigFileName))).c_str();

      if (!RootNode.Assigned())
         RootNode = XmlDoc->AddChild("root");

      xmlGuard <_di_IXMLNode> DebugNode (GetNode("debug", RootNode));
      if (DebugNode.Assigned())
      {
        if(DebugNode->HasChildNodes)
        {
          xmlGuard <_di_IXMLNodeList> DebugNodeList (DebugNode->GetChildNodes());
          if (DebugNodeList.Assigned())
          {
            for (int i = 0; i < DebugNodeList->Count; i++)
            {
              xmlGuard <_di_IXMLNode> DebugNodeItem (DebugNodeList->Get(i));
              if (DebugNodeItem.Assigned())
              {
                std::string DebugNodeItemName = WideCharToString(DebugNodeItem->NodeName).c_str();
                std::string DebugNodeItemName_LowCase = DebugNodeItemName;
                boost::to_lower(DebugNodeItemName_LowCase);

                if((DebugNodeItemName_LowCase == "validator") &&
                    GetNodeValue(DebugNodeItem, (CDebug.PeripheralsState & cnValidatorError)))
                  CDebug.PeripheralsState |= cnValidatorError;

                if((DebugNodeItemName_LowCase == "printer") &&
                    GetNodeValue(DebugNodeItem, (CDebug.PeripheralsState & cnPrinterError)))
                  CDebug.PeripheralsState |= cnPrinterError;

                if (DebugNodeItemName_LowCase == "sound")
                  CDebug.sound = GetNodeValue(DebugNodeItem, CDebug.sound);

                if (DebugNodeItemName_LowCase == "process")
                {
                  if (DebugNodeItem->HasAttribute("explorer"))
                    CDebug.explorerProcess = WCharToBool(DebugNodeItem->GetAttribute("explorer").VOleStr);

                  if (DebugNodeItem->HasAttribute("conn"))
                    CDebug.connProcess = WCharToBool(DebugNodeItem->GetAttribute("conn").VOleStr);
                }
                if (DebugNodeItemName_LowCase == "log")
                {
                    bool DebugNodeItemValue = static_cast<bool>(GetInt(false, DebugNodeItemName.c_str(), DebugNode, CDebug.Logs.url));
                    CDebug.Logs = DebugNodeItemValue;
                    if (!DebugNodeItemValue)
                    {
                        if (DebugNodeItem->HasAttribute("url"))
                        {
                            CDebug.Logs.url = WCharToBool(DebugNodeItem->GetAttribute("url").VOleStr, CDebug.Logs.url);
                        }
                        if (DebugNodeItem->HasAttribute("sound"))
                        {
                            CDebug.Logs.sound = WCharToBool(DebugNodeItem->GetAttribute("sound").VOleStr, CDebug.Logs.sound);
                        }
                        if (DebugNodeItem->HasAttribute("full"))
                        {
                            CDebug.Logs.full = WCharToBool(DebugNodeItem->GetAttribute("full").VOleStr, CDebug.Logs.full);
                        }
                    }
                }
              }
            }
          }
        }
      }

      xmlGuard <_di_IXMLNode> xmlSecNode (GetNode("parameters",RootNode));
      if (xmlSecNode.Assigned())
        {
        StatInfo.StatServerPollInterval = max(GetInt(bOverWrite,"request_time",xmlSecNode,StatInfo.StatServerPollInterval),1);

        StatInfo.StatServerSrc = StatInfo.Host[0];

        AnsiString Host0=AnsiString(StatInfo.Host[0].c_str()).LowerCase();
        if ((Host0!= "none")&&(Host0!= "cyberplat")&&(StatInfo.Port[0]!= 0))
          StatInfo.StatServerSrc+= (":"+AnsiString(StatInfo.Port[0])).c_str();

        xmlGuard <_di_IXMLNode> StatServerNode (GetNode("server_ip",xmlSecNode));
        StatInfo.StatServersAdressesCount = 0;

        if ((!bOverWrite)&&(!StatServerNode->HasAttribute("alt_addr1")))
          {
          StatInfo.StatServerSrc = GetChildNodeValue(bOverWrite,"server_ip",xmlSecNode,StatInfo.StatServerSrc.c_str()).c_str();
          AnsiString StatServerAdresses = (StatInfo.StatServerSrc+" ").c_str();
          while ((StatServerAdresses.Pos(" "))&&(StatInfo.StatServersAdressesCount<MaxStatServersAdresses))
            {
            AnsiString StatServerAdress = StatServerAdresses.SubString(0,StatServerAdresses.Pos(" ")-1);
            if (StatServerAdress.Pos(":"))
              {
              TStringVector vURL;
              boost::split(vURL, StatServerAdress.c_str(), boost::is_any_of(":"));

              StatInfo.Host[StatInfo.StatServersAdressesCount] = vURL[0];
              StatInfo.Port[StatInfo.StatServersAdressesCount] = StringToInt(vURL[1]);
              }
              else
              {
              StatInfo.Host[StatInfo.StatServersAdressesCount] = StatServerAdress.c_str();
              StatInfo.Port[StatInfo.StatServersAdressesCount] = 10024;
              }
            StatInfo.StatServersAdressesCount++;
            StatServerAdresses = StatServerAdresses.SubString(StatServerAdresses.Pos(" ")+1,StatServerAdresses.Length());
            }
          bUpdateOldStatServerInfo = true;
          }

        bUpdateOldStatServerInfo = bUpdateOldStatServerInfo|bOverWrite;
        std::string Addr;
        for (int StatSrvAddr = 0; StatSrvAddr <= 2; StatSrvAddr++)
          {
          Addr = "";
          if ((StatInfo.Host[StatSrvAddr]!= "")&&(StatInfo.Host[StatSrvAddr]!= "none"))
            Addr = (boost::format("%1%:%2%") % StatInfo.Host[StatSrvAddr] % StatInfo.Port[StatSrvAddr]).str();

            if (StatSrvAddr == 0)
            {
                Addr = GetChildNodeValue(bUpdateOldStatServerInfo, "server_ip", xmlSecNode, Addr.c_str());
            }
            else
            {
                Addr = GetStringAttribute(bUpdateOldStatServerInfo, (boost::format("alt_addr%1%") % StatSrvAddr).str().c_str(), StatServerNode, Addr.c_str());
                Addr = mytrim(Addr);
            }

          int defaultPort = 10024;
          if (Addr.find(":") == std::string::npos)
            Addr += ":" + boost::lexical_cast<std::string>(defaultPort);

          TStringVector ulr_parameters;
          boost::split(ulr_parameters, Addr, boost::is_any_of(":"));
          StatInfo.Host[StatSrvAddr] = ulr_parameters[0];
          StatInfo.Port[StatSrvAddr] = StringToInt(ulr_parameters[1], defaultPort);
          }

        StatInfo.CurrentStatServersAdressNum = 0;

        StatInfo.StatServersAdressesCount++;
        if (StatInfo.Host[2]!= "")
          {
          StatInfo.StatServersAdressesCount+= 2;
          }
          else
          if (StatInfo.Host[1]!= "")
            {
            StatInfo.StatServersAdressesCount++;
            }


        if ((AnsiString(StatInfo.Host[0].c_str()).LowerCase()) == "cyberplat")
          {
          StatInfo.ProcessorType = cnCyberPlatServer;
          }
          StatInfo.HeartBeatURL = GetStringAttribute(bOverWrite,"heart_beat_url",StatServerNode,StatInfo.HeartBeatURL.c_str()).c_str();
          StatInfo.PaymentCompleteURL = GetStringAttribute(bOverWrite,"payment_complete_url",StatServerNode,StatInfo.PaymentCompleteURL.c_str()).c_str();
          StatInfo.IncassationURL = GetStringAttribute(bOverWrite,"incassation_url",StatServerNode,StatInfo.IncassationURL.c_str()).c_str();
//          }
        StatInfo.DownloadURLPrefix = GetStringAttribute(bOverWrite,"download_url_prefix",StatServerNode,StatInfo.DownloadURLPrefix.c_str()).c_str();
        ServiceInfo.PaymentBookURL = GetStringAttribute(bOverWrite,"payment_book_url",StatServerNode,StatInfo.DownloadURLPrefix.c_str()).c_str();

        StatInfo.DTLastSuccessfullPacketSending = TDateTime::CurrentDateTime();
        int MaxMin = max(30,(StatInfo.StatServerPollInterval*5));
        StatInfo.DTServerTimeOutDiff = double(MaxMin)/24/60;

        Payments.Offline = GetInt(bOverWrite,"offline_payment",xmlSecNode,Payments.Offline);
        Terminal.ShowPB = static_cast<bool>(GetInt(bOverWrite,"show_payment_book",xmlSecNode,Terminal.ShowPB));
        StatInfo.IsSignStatistics = GetInt(bOverWrite, "sign_statistics", xmlSecNode, StatInfo.IsSignStatistics);
        m_LocaleName = GetChildNodeValue(bOverWrite, "locale", xmlSecNode, "").c_str();

        if (!bOverWrite)
          {
          if (IsExist("KEYS",RootNode))
            {
            Keys.clear();
            _keys_info NewKey;
//            Keys.push_back(_keys_info(0));
/*            Keys[GetKeysNum()].Hasp = GetInt(bOverWrite,"hasp",xmlSecNode,Keys[GetKeysNum()].Hasp);
            Keys[GetKeysNum()].SD = GetString(bOverWrite,"SD",xmlSecNode,Keys[GetKeysNum()].SD);
            Keys[GetKeysNum()].AP = GetString(bOverWrite,"AP",xmlSecNode,Keys[GetKeysNum()].AP);
            Keys[GetKeysNum()].OP = GetString(bOverWrite,"OP",xmlSecNode,Keys[GetKeysNum()].OP);

            xmlGuard <_di_IXMLNode> KeysNode (GetNode("KEYS",RootNode));
            Keys[GetKeysNum()].PubKeySerial = GetInt(bOverWrite,"serial_no", KeysNode, Keys[GetKeysNum()].PubKeySerial);
            Keys[GetKeysNum()].PubKeyPath = GetString(bOverWrite,"pub_key", KeysNode, Keys[GetKeysNum()].PubKeyPath);
            Keys[GetKeysNum()].SecKeyPassword = GetString(bOverWrite,"sec_password", KeysNode, Keys[GetKeysNum()].SecKeyPassword);
            Keys[GetKeysNum()].SecKeyPath = GetString(bOverWrite,"sec_key", KeysNode, Keys[GetKeysNum()].SecKeyPath);*/

            NewKey.Id = 0;

            NewKey.Hasp = GetInt(bOverWrite,"hasp",xmlSecNode,NewKey.Hasp);
            NewKey.SD = GetChildNodeValue(bOverWrite,"SD",xmlSecNode,NewKey.SD.c_str()).c_str();
            NewKey.AP = GetChildNodeValue(bOverWrite,"AP",xmlSecNode,NewKey.AP.c_str()).c_str();
            NewKey.OP = GetChildNodeValue(bOverWrite,"OP",xmlSecNode,NewKey.OP.c_str()).c_str();

            xmlGuard <_di_IXMLNode> KeysNode (GetNode("KEYS",RootNode));
            NewKey.PubKeySerial = GetInt(bOverWrite,"serial_no", KeysNode, NewKey.PubKeySerial);
            NewKey.PubKeyPath = GetChildNodeValue(bOverWrite,"pub_key", KeysNode, NewKey.PubKeyPath.c_str()).c_str();
            NewKey.SecKeyPassword = GetChildNodeValue(bOverWrite,"sec_password", KeysNode, NewKey.SecKeyPassword.c_str()).c_str();
            NewKey.SecKeyPath = GetChildNodeValue(bOverWrite,"sec_key", KeysNode, NewKey.SecKeyPath.c_str()).c_str();

            Keys.push_back(NewKey);
            }
            else
            {
            xmlGuard <_di_IXMLNode> KeysInfoNode (GetNode("keys_info",RootNode));
            GetKeysURL = GetStringAttribute(bOverWrite,"get_keys_url",KeysInfoNode,GetKeysURL.c_str()).c_str();

            if ((KeysInfoNode.Assigned())&&(KeysInfoNode->HasChildNodes))
              {
              xmlGuard <_di_IXMLNodeList> KeysInfoNodeList (KeysInfoNode->GetChildNodes());
              //ClearKeys();
              Keys.clear();
              for (int i = 0; (i<KeysInfoNodeList->Count); i++)
                {
                xmlGuard <_di_IXMLNode> KeysNode (KeysInfoNodeList->Get(i));
                if (KeysNode->NodeName == WideString("keys"))
                  {
                  _keys_info NewKey;
                  NewKey.Id = GetIntAttribute(false,"id",KeysNode,0);
                  NewKey.Hasp = GetInt(false,"hasp",KeysNode,NewKey.Hasp);
                  NewKey.SD = GetChildNodeValue(false,"SD",KeysNode,NewKey.SD.c_str()).c_str();
                  NewKey.AP = GetChildNodeValue(false,"AP",KeysNode,NewKey.AP.c_str()).c_str();
                  NewKey.OP = GetChildNodeValue(false,"OP",KeysNode,NewKey.OP.c_str()).c_str();
                  NewKey.PubKeySerial = GetInt(false,"serial_no", KeysNode, NewKey.PubKeySerial);
                  NewKey.PubKeyPath = GetChildNodeValue(false,"pub_key", KeysNode, NewKey.PubKeyPath.c_str()).c_str();
                  NewKey.SecKeyPassword = GetChildNodeValue(false,"sec_password", KeysNode, NewKey.SecKeyPassword.c_str()).c_str();
                  NewKey.SecKeyPath = GetChildNodeValue(false,"sec_key", KeysNode, NewKey.SecKeyPath.c_str()).c_str();
                  Keys.push_back(NewKey);
                  }
                }
              SortKeys();
              }
              else
              {
              xmlGuard <_di_IXMLNode> KeysInfoNode (GetNode("keys_info",RootNode));
              xmlGuard <_di_IXMLNode> KeysNode (GetNode("keys",KeysInfoNode));
              GetIntAttribute(bOverWrite,"id",KeysNode,Keys[GetKeysNum()].Id);
              GetInt(bOverWrite,"hasp",KeysNode,Keys[GetKeysNum()].Hasp);
              GetChildNodeValue(bOverWrite,"SD",KeysNode,Keys[GetKeysNum()].SD.c_str()).c_str();
              GetChildNodeValue(bOverWrite,"AP",KeysNode,Keys[GetKeysNum()].AP.c_str()).c_str();
              GetChildNodeValue(bOverWrite,"OP",KeysNode,Keys[GetKeysNum()].OP.c_str()).c_str();
              GetInt(bOverWrite,"serial_no", KeysNode, Keys[GetKeysNum()].PubKeySerial);
              GetChildNodeValue(bOverWrite,"pub_key", KeysNode, Keys[GetKeysNum()].PubKeyPath.c_str()).c_str();
              GetChildNodeValue(bOverWrite,"sec_password", KeysNode, Keys[GetKeysNum()].SecKeyPassword.c_str()).c_str();
              GetChildNodeValue(bOverWrite,"sec_key", KeysNode, Keys[GetKeysNum()].SecKeyPath.c_str()).c_str();
              }
            }
          }
          else
          {
          DeleteNode("hasp",xmlSecNode);
          DeleteNode("SD",xmlSecNode);
          DeleteNode("AP",xmlSecNode);
          DeleteNode("OP",xmlSecNode);
          DeleteNode("KEYS",RootNode);
          xmlGuard <_di_IXMLNode> KeysInfoNode (GetNode("keys_info",RootNode));
          xmlGuard <_di_IXMLNodeList> KeysInfoNodeList (KeysInfoNode->GetChildNodes());
          for (int i = KeysInfoNodeList->Count-1; i >= 0; i--)
            {
            xmlGuard <_di_IXMLNode> KeysNode (KeysInfoNodeList->Get(i));
            if (KeysNode->NodeName == WideString("keys"))
              KeysInfoNodeList->Delete(i);
            }

          for(unsigned i = 0;i<Keys.size();i++)
            {
            xmlGuard <_di_IXMLNode> KeysNode (CreateNode("keys",KeysInfoNode,""));
            GetIntAttribute(bOverWrite,"id",KeysNode,Keys[i].Id);
            GetInt(bOverWrite,"hasp",KeysNode,Keys[i].Hasp);
            GetChildNodeValue(bOverWrite,"SD",KeysNode,Keys[i].SD.c_str()).c_str();
            GetChildNodeValue(bOverWrite,"AP",KeysNode,Keys[i].AP.c_str()).c_str();
            GetChildNodeValue(bOverWrite,"OP",KeysNode,Keys[i].OP.c_str()).c_str();
            GetInt(bOverWrite,"serial_no", KeysNode, Keys[i].PubKeySerial);
            GetChildNodeValue(bOverWrite,"pub_key", KeysNode, Keys[i].PubKeyPath.c_str()).c_str();
            GetChildNodeValue(bOverWrite,"sec_password", KeysNode, Keys[i].SecKeyPassword.c_str()).c_str();
            GetChildNodeValue(bOverWrite,"sec_key", KeysNode, Keys[i].SecKeyPath.c_str()).c_str();
            }
          }
        Terminal.Number = Keys[GetKeysNum()].AP;



        xmlGuard <_di_IXMLNode> PaymentsNode (GetNode("payments",xmlSecNode));
        Payments.ErrorPaymentsDeleteHours = GetDoubleAttribute(bOverWrite,"error_payments_delete",PaymentsNode,Payments.ErrorPaymentsDeleteHours);
        AnsiString SavedOffline;
        Payments.rState = eNo;
        if ((Payments.OfflinePeriodBegin.Val!=0)&&(Payments.OfflinePeriodEnd.Val!=0))
          SavedOffline = Payments.OfflinePeriodBegin.FormatString("hh:mm")+"-"+Payments.OfflinePeriodEnd.FormatString("hh:mm");

        AnsiString Temp=GetStringAttribute(bOverWrite,"offline_period",PaymentsNode,SavedOffline.c_str()).c_str();
        if (Temp.Pos("-")!= 0) {
          try
            {
            Payments.OfflinePeriodBegin = Temp.SubString(0,Temp.Pos("-")-1);
            Payments.OfflinePeriodEnd = Temp.SubString(Temp.Pos("-")+1,Temp.Length());
            }
            catch(...)
            {
                ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            }
          }
        Temp = GetStringAttribute(bOverWrite,"money_offline_period",PaymentsNode,"").c_str();
        if (Temp.Pos("-")!= 0) {
          try
            {
            Payments.OfflineMoneyPeriodBegin = Temp.SubString(0,Temp.Pos("-")-1);
            Payments.OfflineMoneyPeriodEnd = Temp.SubString(Temp.Pos("-")+1,Temp.Length());
            }
            catch(...)
            {
                ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            }
          }

        if (Payments.UpdateCardsInfo!= 0)
          Payments.UpdateCardsInfo = max(Payments.UpdateCardsInfo,1440);
        Payments.UpdateCardsInfo = GetIntAttribute(bOverWrite,"update_cards_info",PaymentsNode,Payments.UpdateCardsInfo);
        if (Payments.UpdateCardsInfo!= 0)
          Payments.UpdateCardsInfo = max(Payments.UpdateCardsInfo,1440);
        //Log->Write("UpdateCardsInfo: "+AnsiString(Payments.UpdateCardsInfo));

        LogsDelete.UnprocessedPayments = GetIntAttribute(bOverWrite,"delete_unprocessed_payments",PaymentsNode,LogsDelete.UnprocessedPayments);

        std::string IgnoredCheckErrors = GetStringAttribute(bOverWrite,"ignored_check_errors_list",PaymentsNode,"").c_str();
        TIntVector ignoredErrorsVector;
        IgnoredCheckErrors = strToIntVector(IgnoredCheckErrors, ignoredErrorsVector, "ignored errors");
        if(!IgnoredCheckErrors.empty())
        {
            for(int i = 0; i < ignoredErrorsVector.size(); i++)
            {
                if (ignoredErrorsVector[i])
                {
                    Payments.IgnoredCheckErrorsList.insert(ignoredErrorsVector[i]);
                }
            }
            JSMaker->AddStringAttribute("IgnoredCheckErrorsList", IgnoredCheckErrors.c_str());
            Log->Write(("Ignored errors: " + IgnoredCheckErrors).c_str());
        }

       //	_di_IXMLNode CommandsNode = GetNode("commands",xmlSecNode);
       //	Commands->CommandDeleteTime = GetIntAttribute(bOverWrite,"commands_delete_time",CommandsNode,Commands->CommandDeleteTime);
        Payments.nextTry = -1;
        Payments.nextTry = GetDoubleAttribute(false,"next_try", PaymentsNode, -1, true);

        xmlGuard <_di_IXMLNode> LogsDeleteNode (GetNode("logs_delete",xmlSecNode));
        //_di_IXMLNode LogsDeleteNode = GetNode("logs_delete",xmlSecNode);
        LogsDelete.Payments = GetIntAttribute(bOverWrite,"payments",LogsDeleteNode,LogsDelete.Payments);
        LogsDelete.Main = GetIntAttribute(bOverWrite,"main",LogsDeleteNode,LogsDelete.Main);
        LogsDelete.Other = GetIntAttribute(bOverWrite,"other",LogsDeleteNode,LogsDelete.Other);

        xmlGuard <_di_IXMLNode> MTNode (GetNode("money_transfers",xmlSecNode));
        MoneyTransfers.MTInfoUpdateTime = GetTimeAttribute(bOverWrite,"mt_info_update_time",MTNode,MoneyTransfers.MTInfoUpdateTime);
// currency
        xmlGuard<_di_IXMLNode> CurrencyNode(GetNode("currency", xmlSecNode));
        CurrencyInfo.Currency = GetStringAttribute(bOverWrite, "stdname", CurrencyNode, CurrencyInfo.Currency.c_str()).c_str();
        int tExchange_rate = GetDoubleAttribute(bOverWrite, "exchange_rate", CurrencyNode, CurrencyInfo.ExchangeRate);
        CurrencyInfo.ExchangeRate = (tExchange_rate > 0) ? tExchange_rate : CurrencyInfo.ExchangeRate;
        CurrencyInfo.CurrencyName = GetStringAttribute(bOverWrite, "currency_name", CurrencyNode, CurrencyInfo.CurrencyName.c_str()).c_str();
        CurrencyInfo.currencyId = GetIntAttribute(bOverWrite, "currency_iso_id", CurrencyNode, CurrencyInfo.currencyId);
// currency
        }

      xmlGuard <_di_IXMLNode> EmailSenderNode (GetNode("email_sender",RootNode));

      EMailInfo.SendInterval = GetIntAttribute(bOverWrite,"send_interval",EmailSenderNode,EMailInfo.SendInterval);
      EMailInfo.SMailHost = GetStringAttribute(bOverWrite,"smailhost",EmailSenderNode,EMailInfo.SMailHost.c_str()).c_str();
      EMailInfo.FromAddress = GetStringAttribute(bOverWrite,"from",EmailSenderNode,EMailInfo.FromAddress.c_str()).c_str();
      EMailInfo.UserId = GetStringAttribute(bOverWrite,"userid",EmailSenderNode,EMailInfo.UserId.c_str()).c_str();
      EMailInfo.Password = GetStringAttribute(bOverWrite,"password",EmailSenderNode,EMailInfo.Password.c_str()).c_str();
      //bool bSendFiles = GetIntAttribute(bOverWrite,"send_files",EmailSenderNode,EMailInfo.SendFiles);
      //bool bSendMessages = GetIntAttribute(bOverWrite,"send_messages",EmailSenderNode,EMailInfo.SendMessages);
      EMailInfo.Ext = GetIntAttribute(bOverWrite,"ext",EmailSenderNode,EMailInfo.Ext);
      //EMailInfo.ToAddress = GetStringAttribute(bOverWrite,"to",EmailSenderNode, EMailInfo.ToAddress);

      if (!IsAttributeExists("hw_err_to", EmailSenderNode))
        EMailInfo.ToHWErrAddr = GetStringAttribute(bOverWrite,"to",EmailSenderNode, EMailInfo.ToHWErrAddr.c_str()).c_str();
      if (!IsAttributeExists("hw_ok_to", EmailSenderNode))
        EMailInfo.ToHWOKAddr = GetStringAttribute(bOverWrite,"to",EmailSenderNode,EMailInfo.ToHWOKAddr.c_str()).c_str();
      if (!IsAttributeExists("val_full_to", EmailSenderNode))
        EMailInfo.ToValFullAddr = GetStringAttribute(bOverWrite,"to",EmailSenderNode,EMailInfo.ToValFullAddr.c_str()).c_str();
      if (!IsAttributeExists("incass_to", EmailSenderNode))
        EMailInfo.ToIncassAddr = GetStringAttribute(bOverWrite,"to",EmailSenderNode,EMailInfo.ToIncassAddr.c_str()).c_str();
      if (!IsAttributeExists("prog_msg_to", EmailSenderNode))
        EMailInfo.ToProgMsgAddr = GetStringAttribute(bOverWrite,"to",EmailSenderNode,EMailInfo.ToProgMsgAddr.c_str()).c_str();
      if (!IsAttributeExists("mdm_msg_to", EmailSenderNode))
        EMailInfo.ToMdmMsgAddr = GetStringAttribute(bOverWrite,"to",EmailSenderNode,EMailInfo.ToMdmMsgAddr.c_str()).c_str();
      if (!IsAttributeExists("file_to", EmailSenderNode))
        EMailInfo.ToFileAddr = GetStringAttribute(bOverWrite,"to",EmailSenderNode,EMailInfo.ToFileAddr.c_str()).c_str();

      EMailInfo.IgnoredErrorsList = GetStringAttribute(bOverWrite,"ignored_errors",EmailSenderNode,EMailInfo.IgnoredErrorsList.c_str()).c_str();

      if (IsAttributeExists("send_messages", EmailSenderNode))
        {
        if (!GetIntAttribute(bOverWrite,"send_messages",EmailSenderNode,false))
          {
          EMailInfo.ToHWErrAddr = "";
          EMailInfo.ToHWOKAddr = "";
          EMailInfo.ToValFullAddr = "";
          EMailInfo.ToIncassAddr = "";
          EMailInfo.ToProgMsgAddr = "";
          EMailInfo.ToMdmMsgAddr = "";
          }
        }
      if (IsAttributeExists("send_files", EmailSenderNode))
        {
        if (!GetIntAttribute(bOverWrite,"send_files",EmailSenderNode,true))
          {
          EMailInfo.ToFileAddr = "";
          }
        }

      EMailInfo.ToHWErrAddr = GetStringAttribute(bOverWrite,"hw_err_to",EmailSenderNode,EMailInfo.ToHWErrAddr.c_str()).c_str();
      EMailInfo.ToHWOKAddr = GetStringAttribute(bOverWrite,"hw_ok_to",EmailSenderNode,EMailInfo.ToHWOKAddr.c_str()).c_str();
      EMailInfo.ToValFullAddr = GetStringAttribute(bOverWrite,"val_full_to",EmailSenderNode,EMailInfo.ToValFullAddr.c_str()).c_str();
      EMailInfo.ToIncassAddr = GetStringAttribute(bOverWrite,"incass_to",EmailSenderNode,EMailInfo.ToIncassAddr.c_str()).c_str();
      EMailInfo.ToProgMsgAddr = GetStringAttribute(bOverWrite,"prog_msg_to",EmailSenderNode,EMailInfo.ToProgMsgAddr.c_str()).c_str();
      EMailInfo.ToMdmMsgAddr = GetStringAttribute(bOverWrite,"mdm_msg_to",EmailSenderNode,EMailInfo.ToMdmMsgAddr.c_str()).c_str();
      EMailInfo.ToFileAddr = GetStringAttribute(bOverWrite,"file_to",EmailSenderNode,EMailInfo.ToFileAddr.c_str()).c_str();

      if (IsAttributeExists("to", EmailSenderNode))
        DeleteAttribute("to", EmailSenderNode);
      if (IsAttributeExists("send_messages", EmailSenderNode))
        DeleteAttribute("send_messages", EmailSenderNode);
      if (IsAttributeExists("send_files", EmailSenderNode))
        DeleteAttribute("send_files", EmailSenderNode);

      xmlGuard <_di_IXMLNode> SMSSenderNode (GetNode("sms_sender",RootNode));
      SMSInfo.PhoneNumber = GetStringAttribute(bOverWrite,"phone_number",SMSSenderNode,SMSInfo.PhoneNumber.c_str()).c_str();
      SMSInfo.Comment = GetStringAttribute(bOverWrite,"comment",SMSSenderNode,SMSInfo.Comment.c_str()).c_str();
      SMSInfo.SendStartUpSMS = GetIntAttribute(bOverWrite,"send_startup_sms",SMSSenderNode,SMSInfo.SendStartUpSMS);

      ConnectionTimeOut=60;
      xmlGuard <_di_IXMLNode> ConnectionNode (GetNode("connection",RootNode));
      if(GetStringAttribute(bOverWrite,"timeout",ConnectionNode,(boost::format("%1%") % ConnectionTimeOut).str().c_str()) != "")
          ConnectionTimeOut=atoi(GetStringAttribute(bOverWrite,"timeout",ConnectionNode,(boost::format("%1%") % ConnectionTimeOut).str().c_str()).c_str());

      xmlSecNode = GetNode("peripherals",RootNode);
      if (xmlSecNode.Assigned())
        {
        /* astafiev */
        xmlGuard <_di_IXMLNode> ScannerNode (GetNode("scanner", xmlSecNode));
        Peripherals.Scanner.Type = GetStringAttribute(bOverWrite,"type",ScannerNode,Peripherals.Scanner.Type.c_str());
        boost::to_lower(Peripherals.Scanner.Type);
        Peripherals.Scanner.Port = GetIntAttribute(bOverWrite,"port",ScannerNode,Peripherals.Scanner.Port);
        //Peripherals.Scanner.PortBaudRate = GetIntAttribute(bOverWrite,"baudrate",ScannerNode,Peripherals.Scanner.PortBaudRate);
        if (!(Peripherals.Scanner.Port))
          Peripherals.Scanner.Type = "";
        /************/
        xmlGuard <_di_IXMLNode> PrinterNode (GetNode("printer",xmlSecNode));
        Peripherals.Printer.Type = GetStringAttribute(bOverWrite,"type",PrinterNode,Peripherals.Printer.Type.c_str());
        boost::to_lower(Peripherals.Printer.Type);
        Peripherals.Printer.Port = GetIntAttribute(bOverWrite,"port",PrinterNode,Peripherals.Printer.Port);
        Peripherals.Printer.PortBaudRate = GetIntAttribute(bOverWrite,"baudrate",PrinterNode,Peripherals.Printer.PortBaudRate);
        Peripherals.Printer.AutoOpenShift = GetIntAttribute(bOverWrite,"auto_open_shift",PrinterNode,Peripherals.Printer.AutoOpenShift);
        Peripherals.Printer.SaveUnprintedCheques = GetIntAttribute(bOverWrite,"save_unprinted_cheques",PrinterNode,Peripherals.Printer.SaveUnprintedCheques);
        Peripherals.Printer.PrintUnprintedCheques = GetIntAttribute(bOverWrite,"print_unprinted_cheques",PrinterNode,Peripherals.Printer.PrintUnprintedCheques);
        Peripherals.Printer.FreeseOnError = GetIntAttribute(bOverWrite,"terminal_freese_on_error",PrinterNode,Peripherals.Printer.FreeseOnError);
        Peripherals.Printer.IncassBarCode = static_cast<bool>(GetIntAttribute(bOverWrite,"incass_barcode",PrinterNode,Peripherals.Printer.IncassBarCode));
        Peripherals.Printer.ShowMessageOnError = GetIntAttribute(bOverWrite,"show_message_on_error",PrinterNode,Peripherals.Printer.ShowMessageOnError);
        Peripherals.Printer.MinLinesCount = GetIntAttribute(bOverWrite,"min_lines_count",PrinterNode,Peripherals.Printer.MinLinesCount);
        Peripherals.Printer.Font = GetStringAttribute(bOverWrite,"font",PrinterNode,Peripherals.Printer.Font.c_str());
        Peripherals.Printer.PresenterCommand = GetIntAttribute(bOverWrite,"presenter_command",PrinterNode,Peripherals.Printer.PresenterCommand);
        Peripherals.Printer.ZReportWithIncassation = GetIntAttribute(bOverWrite,"zreport_with_incassation",PrinterNode,Peripherals.Printer.ZReportWithIncassation);
        Peripherals.Printer.TapeLength = GetIntAttribute(bOverWrite,"tape_length",PrinterNode,Peripherals.Printer.TapeLength);

        if (!(Peripherals.Printer.Port) && Peripherals.Printer.Type != "windows")
          Peripherals.Printer.Type = "";
        
        xmlGuard <_di_IXMLNode> ValidatorNode (GetNode("validator",xmlSecNode));
        Peripherals.Validator.Type = GetStringAttribute(bOverWrite,"type",ValidatorNode,Peripherals.Validator.Type.c_str());
/*zh*/  boost::to_lower(Peripherals.Validator.Type);
          AnsiString tempStr = AnsiString(Peripherals.Validator.Type.c_str());
          tempStr = tempStr.Trim();
          Peripherals.Validator.Type = tempStr.c_str();
        Peripherals.Validator.Protocol = GetStringAttribute(bOverWrite,"protocol",ValidatorNode,Peripherals.Validator.Protocol.c_str());
/*zh*/  boost::to_lower(Peripherals.Validator.Protocol);
          tempStr = AnsiString(Peripherals.Validator.Protocol.c_str());
          tempStr = tempStr.Trim();
          Peripherals.Validator.Protocol = tempStr.c_str();

        Peripherals.Validator.Port = GetIntAttribute(bOverWrite,"port",ValidatorNode,Peripherals.Validator.Port);
        //Peripherals->Validator->PortBaudRate = GetIntAttribute(bOverWrite,"baudrate",ValidatorNode,Peripherals->Validator->PortBaudRate);
        Peripherals.Validator.StackerOpenCommand = GetIntAttribute(bOverWrite,"stacker_open_command",ValidatorNode,Peripherals.Validator.StackerOpenCommand);
/*zh*/  Peripherals.Validator.IncassOpenStacker = GetIntAttribute(bOverWrite,"incass_open_stacker",ValidatorNode,Peripherals.Validator.IncassOpenStacker);
        Peripherals.Validator.BillsSensitivity = GetStringAttribute(bOverWrite,"bills_sensitivity",ValidatorNode,Peripherals.Validator.BillsSensitivity.c_str());
        Peripherals.Validator.ReportBillCount = GetStringAttribute(bOverWrite,"report_bill_count",ValidatorNode,Peripherals.Validator.ReportBillCount.c_str());
        if (!(Peripherals.Validator.Port))
          Peripherals.Validator.Type = "";


        xmlGuard <_di_IXMLNode> CoinAcceptorNode (GetNode("coin_acceptor",xmlSecNode));
        Peripherals.CoinAcceptor.Type = GetStringAttribute(bOverWrite,"type",CoinAcceptorNode,Peripherals.CoinAcceptor.Type.c_str());
        boost::to_lower(Peripherals.CoinAcceptor.Type);
        Peripherals.CoinAcceptor.Port = GetIntAttribute(bOverWrite,"port",CoinAcceptorNode,Peripherals.CoinAcceptor.Port);
        //Peripherals->CoinAcceptor->PortBaudRate = GetIntAttribute(bOverWrite,"baudrate",CoinAcceptorNode,Peripherals->CoinAcceptor->Port);
        Peripherals.CoinAcceptor.ReportCoinCount = GetStringAttribute(bOverWrite,"report_coin_count",CoinAcceptorNode,Peripherals.CoinAcceptor.ReportCoinCount.c_str());
        Peripherals.CoinAcceptor.MinCash = GetDoubleAttribute(bOverWrite,"min_coin_nominal",CoinAcceptorNode,Peripherals.CoinAcceptor.MinCash);
        Peripherals.CoinAcceptor.nominalMultiply = GetDoubleAttribute(bOverWrite,"nominal_multiply",CoinAcceptorNode,Peripherals.CoinAcceptor.nominalMultiply);
        if (!(Peripherals.CoinAcceptor.Port))
          Peripherals.CoinAcceptor.Type = "";

        JSMaker->AddStringAttribute("CurrencyName", CurrencyInfo.CurrencyName.c_str());
        JSMaker->AddStringAttribute("CurrencyType", CurrencyInfo.Currency.c_str());

  /*			_di_IXMLNode ModemNode = GetNode("modem",xmlSecNode);
        Peripherals.Modem.AutoDial = GetIntAttribute(bOverWrite,"auto_dial",ModemNode,Peripherals.Modem.AutoDial);
        Peripherals.Modem.ConnectionName = GetStringAttribute(bOverWrite,"connection_name",ModemNode,Peripherals.Modem.ConnectionName);
        Peripherals.Modem.Interval = max(60,Peripherals.Modem.Interval);
        Peripherals.Modem.Interval = GetIntAttribute(bOverWrite,"interval",ModemNode,Peripherals.Modem.Interval);
        Peripherals.Modem.Interval = max(60,Peripherals.Modem.Interval);
        Peripherals.Modem.ConnectionCheckTimeOutDT = float(GetIntAttribute(bOverWrite,"connection_check_time_out",ModemNode,int(Peripherals.Modem.ConnectionCheckTimeOutDT.Val*24*60)))/24/60;
        Peripherals.Modem.Hosts[0] = GetStringAttribute(bOverWrite,"host0",ModemNode,Peripherals.Modem.Hosts[0]);
        Peripherals.Modem.Hosts[1] = GetStringAttribute(bOverWrite,"host1",ModemNode,Peripherals.Modem.Hosts[1]);
        Peripherals.Modem.FailuresBeforeModemReset = GetIntAttribute(bOverWrite,"failures_before_modem_reset",ModemNode,Peripherals.Modem.FailuresBeforeModemReset);
        Peripherals.Modem.FailuresBeforeTerminalReboot = GetIntAttribute(bOverWrite,"failures_before_reboot",ModemNode,Peripherals.Modem.FailuresBeforeTerminalReboot);
        Peripherals.Modem.Port = GetIntAttribute(bOverWrite,"port",ModemNode,Peripherals.Modem.Port);
        Peripherals.Modem.ServiceNumber = GetStringAttribute(bOverWrite,"service_number",ModemNode,Peripherals.Modem.ServiceNumber);
        Peripherals.Modem.GetServiceInfoInterval = GetIntAttribute(bOverWrite,"get_service_info_interval",ModemNode,Peripherals.Modem.GetServiceInfoInterval);
        Peripherals.Modem.DisconnectTime = GetIntAttribute(bOverWrite,"disconnect_time",ModemNode,Peripherals.Modem.DisconnectTime);
        Peripherals.Modem.InitCmd = GetStringAttribute(bOverWrite,"init_cmd",ModemNode,Peripherals.Modem.InitCmd);
        Peripherals.Modem.Login = GetStringAttribute(bOverWrite,"login",ModemNode,Peripherals.Modem.Login);
        Peripherals.Modem.Password = GetStringAttribute(bOverWrite,"password",ModemNode,Peripherals.Modem.Password);
        Peripherals.Modem.InitString[0] = GetStringAttribute(bOverWrite,"init_string",ModemNode,Peripherals.Modem.InitString[0]);
        Peripherals.Modem.InitString[1] = GetStringAttribute(bOverWrite,"alt_init_string",ModemNode,Peripherals.Modem.InitString[1]);*/

        xmlGuard <_di_IXMLNode> ModemNode (GetNode("modem",xmlSecNode));
        ConnInfo NewConnInfo;
        if (bOverWrite)
          {
          NewConnInfo = Connection(0);
          }
          else
          {
          RASConnections.clear();
          //ClearConnections();
          }

        Peripherals.Modem.AutoDial = GetIntAttribute(bOverWrite,"auto_dial",ModemNode,Peripherals.Modem.AutoDial);
        NewConnInfo.Name = GetStringAttribute(bOverWrite,"connection_name",ModemNode,NewConnInfo.Name.c_str()).c_str();
        Peripherals.Modem.Interval = max(60,GetIntAttribute(bOverWrite,"interval",ModemNode,Peripherals.Modem.Interval));
        Peripherals.Modem.ConnectionCheckTimeOutDT = float(GetIntAttribute(bOverWrite,"connection_check_time_out",ModemNode,int(Peripherals.Modem.ConnectionCheckTimeOutDT.Val*24*60)))/24/60;
        Peripherals.Modem.Hosts[0] = GetStringAttribute(bOverWrite,"host0",ModemNode,Peripherals.Modem.Hosts[0].c_str()).c_str();
        Peripherals.Modem.Hosts[1] = GetStringAttribute(bOverWrite,"host1",ModemNode,Peripherals.Modem.Hosts[1].c_str()).c_str();
        NewConnInfo.DisconnectTime = GetIntAttribute(bOverWrite,"disconnect_time",ModemNode,NewConnInfo.DisconnectTime);
        NewConnInfo.InitCmd = GetStringAttribute(bOverWrite,"init_cmd",ModemNode,NewConnInfo.InitCmd.c_str()).c_str();
        NewConnInfo.Login = GetStringAttribute(bOverWrite,"login",ModemNode,NewConnInfo.Login.c_str()).c_str();
        NewConnInfo.Password = GetStringAttribute(bOverWrite,"password",ModemNode,NewConnInfo.Password.c_str()).c_str();
        NewConnInfo.InitString[0] = GetStringAttribute(bOverWrite,"init_string",ModemNode,NewConnInfo.InitString[0].c_str()).c_str();
        NewConnInfo.InitString[1] = GetStringAttribute(bOverWrite,"alt_init_string",ModemNode,NewConnInfo.InitString[1].c_str()).c_str();

        xmlGuard <_di_IXMLNode> ConnectionsNode (GetNode("connections",RootNode));
        xmlGuard <_di_IXMLNode> ProxyNode (GetNode("proxy",ConnectionsNode));

        NewConnInfo.HTTPProxy.CfgType = GetStringAttribute(bOverWrite,"type",ProxyNode,NewConnInfo.HTTPProxy.CfgType.c_str()).c_str();

        NewConnInfo.HTTPProxy.Host = GetStringAttribute(bOverWrite,"host",ProxyNode,NewConnInfo.HTTPProxy.Host.c_str()).c_str();
        NewConnInfo.Proxy->Host = NewConnInfo.HTTPProxy.Host.c_str();

        NewConnInfo.HTTPProxy.Port = GetIntAttribute(bOverWrite,"port",ProxyNode,NewConnInfo.HTTPProxy.Port);
        NewConnInfo.Proxy->Port = NewConnInfo.HTTPProxy.Port;

        NewConnInfo.HTTPProxy.UserName = GetStringAttribute(bOverWrite,"username",ProxyNode,NewConnInfo.HTTPProxy.UserName.c_str()).c_str();
        NewConnInfo.Proxy->UserID = NewConnInfo.HTTPProxy.UserName.c_str();

        NewConnInfo.HTTPProxy.Password = GetStringAttribute(bOverWrite,"password",ProxyNode,NewConnInfo.HTTPProxy.Password.c_str()).c_str();
        NewConnInfo.Proxy->Password = NewConnInfo.HTTPProxy.Password.c_str();

        if (AnsiString(NewConnInfo.HTTPProxy.CfgType.c_str()).LowerCase() == "socks4")
          {
          NewConnInfo.HTTPProxy.Type = "socks";
          NewConnInfo.Proxy->Version = svSocks4;
          }
          else
          if (AnsiString(NewConnInfo.HTTPProxy.CfgType.c_str()).LowerCase() == "socks4a")
            {
            NewConnInfo.HTTPProxy.Type = "socks";
            NewConnInfo.Proxy->Version = svSocks4A;
            }
            else
            if (AnsiString(NewConnInfo.HTTPProxy.CfgType.c_str()).LowerCase() == "socks5")
              {
              NewConnInfo.HTTPProxy.Type = "socks";
              NewConnInfo.Proxy->Version = svSocks5;
              }
              else
              {
              NewConnInfo.Proxy->Version = svNoSocks;
              if (AnsiString(NewConnInfo.HTTPProxy.CfgType.c_str()).LowerCase() == "http")
                NewConnInfo.HTTPProxy.Type = "http";
                else
                NewConnInfo.HTTPProxy.Type = "none";
              }

        if (NewConnInfo.HTTPProxy.UserName == "")
          NewConnInfo.Proxy->Authentication = saNoAuthentication;
          else
          NewConnInfo.Proxy->Authentication = saUsernamePassword;

        if (!bOverWrite)
          RASConnections.push_back(NewConnInfo);

        ActiveConnection = 0;

        Peripherals.Modem.FailuresBeforeModemReset = GetIntAttribute(bOverWrite,"failures_before_modem_reset",ModemNode,Peripherals.Modem.FailuresBeforeModemReset);
        Peripherals.Modem.FailuresBeforeTerminalReboot = GetIntAttribute(bOverWrite,"failures_before_reboot",ModemNode,Peripherals.Modem.FailuresBeforeTerminalReboot);
        Peripherals.Modem.Port = GetIntAttribute(bOverWrite,"port",ModemNode,Peripherals.Modem.Port);
        if (!IsAttributeExists("service_info_port",ModemNode))
          Peripherals.Modem.ServiceInfoPort = Peripherals.Modem.Port;
        Peripherals.Modem.ServiceInfoPort = GetIntAttribute(bOverWrite,"service_info_port",ModemNode,Peripherals.Modem.ServiceInfoPort);
        //Peripherals.Modem.PortBaudRate = GetIntAttribute(bOverWrite,"baudrate",ModemNode,Peripherals.Modem.PortBaudRate);
        Peripherals.Modem.ServiceNumber = GetStringAttribute(bOverWrite,"service_number",ModemNode,Peripherals.Modem.ServiceNumber.c_str()).c_str();
        Peripherals.Modem.GetServiceInfoInterval = GetIntAttribute(bOverWrite,"get_service_info_interval",ModemNode,Peripherals.Modem.GetServiceInfoInterval);
//!!!!!        Peripherals.Modem.ServiceAnswerMask = GetStringAttribute(bOverWrite,"service_answer_mask",ModemNode,Peripherals.Modem.ServiceAnswerMask);

        xmlGuard <_di_IXMLNode> WDNode (GetNode("watchdog",xmlSecNode));
        Peripherals.WatchDog.Type = GetStringAttribute(bOverWrite,"type",WDNode,Peripherals.WatchDog.Type.c_str());
        boost::to_lower(Peripherals.WatchDog.Type);
        Peripherals.WatchDog.Port = GetIntAttribute(bOverWrite,"port",WDNode,Peripherals.WatchDog.Port);
        //Peripherals.WatchDog.PortBaudRate = GetIntAttribute(bOverWrite,"baudrate",WDNode,Peripherals.WatchDog.PortBaudRate);
        //bWDRebootAllowed = GetIntAttribute(bOverWrite,"reboot_allowed",WDNode,false,true);
        //DeleteNode("reboot_allowed",WDNode);
        if (!(Peripherals.WatchDog.Port))
          Peripherals.WatchDog.Type = "";

        xmlGuard <_di_IXMLNode> KBDNode (GetNode("keyboard",xmlSecNode));
        Peripherals.Keyboard.Type = GetStringAttribute(bOverWrite,"type",KBDNode,Peripherals.Keyboard.Type.c_str());
        boost::to_lower(Peripherals.Keyboard.Type);
        Peripherals.Keyboard.Port = GetIntAttribute(bOverWrite,"port",KBDNode,Peripherals.Keyboard.Port);
        //Peripherals.Keyboard.PortBaudRate = GetIntAttribute(bOverWrite,"baudrate",KBDNode,Peripherals.Keyboard.PortBaudRate);
        if (!(Peripherals.Keyboard.Port))
          Peripherals.Keyboard.Type = "";

        xmlGuard <_di_IXMLNode> CRDNode (GetNode("cardreader",xmlSecNode));
        Peripherals.CardReader.Type = GetStringAttribute(bOverWrite,"type",CRDNode,Peripherals.CardReader.Type.c_str());
        boost::to_lower(Peripherals.CardReader.Type);
        Peripherals.CardReader.Port = GetIntAttribute(bOverWrite,"port",CRDNode,Peripherals.CardReader.Port);
        //Peripherals.CardReader.PortBaudRate = GetIntAttribute(bOverWrite,"baudrate",CRDNode,Peripherals.CardReader.PortBaudRate);
        Peripherals.CardReader.SystemCode = GetLongAttribute(bOverWrite,"system_code",CRDNode,Peripherals.CardReader.SystemCode);
        Peripherals.CardReader.DeviceCode = GetLongAttribute(bOverWrite,"device_code",CRDNode,Peripherals.CardReader.DeviceCode);
        //Peripherals.CardReader.FreeseOnError = GetIntAttribute(bOverWrite,"terminal_freese_on_error",CRDNode,Peripherals.CardReader.FreeseOnError);
        }

      xmlGuard <_di_IXMLNode> ServiceNode (GetNode("service",RootNode));
      ServiceInfo.OutOfOrderPassword = GetChildNodeValue(bOverWrite,"out_of_order_password",ServiceNode,ServiceInfo.OutOfOrderPassword.c_str()).c_str();
      ServiceInfo.Number = GetChildNodeValue(bOverWrite,"service_num",ServiceNode,ServiceInfo.Number.c_str()).c_str();
      ServiceInfo.OperatorId = GetInt(bOverWrite,"service_op",ServiceNode,ServiceInfo.OperatorId);
      JSMaker->AddStringAttribute("OutOfOrderPassword",ServiceInfo.OutOfOrderPassword.c_str());

      ServiceInfo.ServiceMenuPasswordMaskSrc = GetChildNodeValue(bOverWrite,"service_menu_password_mask",ServiceNode,ServiceInfo.ServiceMenuPasswordMaskSrc.c_str()).c_str();
      ServiceInfo.ServiceMenuPasswordMask = ChangeChars(ServiceInfo.ServiceMenuPasswordMaskSrc.c_str(), "%SD%", Keys[GetKeysNum()].SD.c_str()).c_str();
      ServiceInfo.ServiceMenuPasswordMask = ChangeChars(ServiceInfo.ServiceMenuPasswordMask.c_str(), "%AP%", Keys[GetKeysNum()].AP.c_str()).c_str();
      ServiceInfo.ServiceMenuPasswordMask = ChangeChars(ServiceInfo.ServiceMenuPasswordMask.c_str(), "%OP%", Keys[GetKeysNum()].OP.c_str()).c_str();

      //_di_IXMLNode ServiceMenuNode = GetNode("service_menu_password_mask", ServiceNode);
      //ServiceInfo.ServiceMenuItems = GetStringAttribute(bOverWrite,"items", ServiceMenuNode, ServiceInfo.ServiceMenuItems);

      ServiceInfo.ServiceShortMenuPasswordMaskSrc = GetChildNodeValue(bOverWrite,"short_service_menu_password_mask",ServiceNode,ServiceInfo.ServiceShortMenuPasswordMaskSrc.c_str()).c_str();
      ServiceInfo.ServiceShortMenuPasswordMask = ChangeChars(ServiceInfo.ServiceShortMenuPasswordMaskSrc.c_str(), "%SD%", Keys[GetKeysNum()].SD.c_str()).c_str();
      ServiceInfo.ServiceShortMenuPasswordMask = ChangeChars(ServiceInfo.ServiceShortMenuPasswordMask.c_str(), "%AP%", Keys[GetKeysNum()].AP.c_str()).c_str();
      ServiceInfo.ServiceShortMenuPasswordMask = ChangeChars(ServiceInfo.ServiceShortMenuPasswordMask.c_str(), "%OP%", Keys[GetKeysNum()].OP.c_str()).c_str();

      ServiceInfo.IncassationNumberMaskSrc = GetChildNodeValue(bOverWrite,"incassation_number_mask",ServiceNode,ServiceInfo.IncassationNumberMaskSrc.c_str()).c_str();
      ServiceInfo.IncassationNumberMask = ChangeChars(ServiceInfo.IncassationNumberMaskSrc.c_str(), "%SD%", Keys[GetKeysNum()].SD.c_str()).c_str();
      ServiceInfo.IncassationNumberMask = ChangeChars(ServiceInfo.IncassationNumberMask.c_str(), "%AP%", Keys[GetKeysNum()].AP.c_str()).c_str();
      ServiceInfo.IncassationNumberMask = ChangeChars(ServiceInfo.IncassationNumberMask.c_str(), "%OP%", Keys[GetKeysNum()].OP.c_str()).c_str();

      //_di_IXMLNode ServiceShortMenuNode = GetNode("short_service_menu_password_mask", ServiceNode);
      //ServiceInfo.ServiceShortMenuItems = GetStringAttribute(bOverWrite,"items", ServiceShortMenuNode, ServiceInfo.ServiceShortMenuItems);

      //		Log->Write ("IncassationNumberMask: "+AnsiString(ServiceInfo.IncassationNumberMask));

      xmlGuard <_di_IXMLNode> IncassNode (GetNode("incassation",ServiceNode));
      ServiceInfo.IncassReportCount = GetIntAttribute(bOverWrite,"reports_count",IncassNode,ServiceInfo.IncassReportCount);
      ServiceInfo.IncassGetAmountURL = GetStringAttribute(bOverWrite,"get_amount_url",IncassNode,ServiceInfo.IncassGetAmountURL.c_str()).c_str();

      if (bOverWrite)
        {
        AnsiString ATemp;
        xmlGuard <_di_IXMLNode> ChequeHeadNode (GetNode("cheque_head",RootNode));
        //_di_IXMLNode ChequeHeadNode = GetNode("cheque_head",RootNode);
        if (ChequeHeadNode.Assigned())
          {
          if (ChequeHeadNode->HasChildNodes)
            {
            xmlGuard <_di_IXMLNodeList> xmlCHeadNdl (ChequeHeadNode->GetChildNodes());
            xmlCHeadNdl->Clear();
            }
//          for (int i = 0;i<CheckCaption->Count;i++)
//            CreateNode("string",ChequeHeadNode,CheckCaption->Strings[i]);
          for (unsigned i = 0;i<ChequeCaption.size();i++)
            CreateNode("string",ChequeHeadNode,ChequeCaption[i].c_str());
          }
        }
        else
        {
        AnsiString ATemp;
        xmlGuard <_di_IXMLNode> ChequeHeadNode (GetNode("cheque_head",RootNode));
        //_di_IXMLNode ChequeHeadNode = GetNode("cheque_head",RootNode);
        if (ChequeHeadNode.Assigned())
          {
          if (ChequeHeadNode->HasChildNodes)
            {
            xmlGuard <_di_IXMLNodeList> xmlCHeadNdl (ChequeHeadNode->GetChildNodes());
            //CheckCaption->Clear();
            ChequeCaption.clear();
            for (int i = 0; ((i<xmlCHeadNdl->Count)&&(i<40)); i++)
              {
              xmlGuard <_di_IXMLNode> xmlCHNode (xmlCHeadNdl->Get(i));
              if (xmlCHNode->NodeName == WideString("string"))
                {
                if (xmlCHNode->IsTextElement)
                  ATemp = xmlCHNode->NodeValue;
                  else
                  ATemp = "";
                ChequeCaption.push_back(ATemp.c_str());
                //CheckCaption->Add(ATemp);
                }
              }
            }
            else
            {
            for (unsigned i = 0;i<ChequeCaption.size();i++)
              CreateNode("string",ChequeHeadNode,ChequeCaption[i].c_str());
            //for (int i = 0;i<CheckCaption->Count;i++)
            //  CreateNode("string",ChequeHeadNode,CheckCaption->Strings[i]);
            }
          }
        }

      xmlGuard <_di_IXMLNode> xmlTerminalNode (GetNode("terminal",RootNode));
      if (xmlTerminalNode.Assigned())
        {
        xmlGuard <_di_IXMLNode> InterfaceNode (GetNode("interface",xmlTerminalNode));
        Terminal.ShowCursor = GetIntAttribute(bOverWrite,"show_cursor",InterfaceNode,Terminal.ShowCursor);
        Terminal.InterfaceSoundVolume = GetIntAttribute(bOverWrite,"sound_volume",InterfaceNode,Terminal.InterfaceSoundVolume);
        JSMaker->AddStringAttribute("InterfaceSoundVolume",AnsiString(Terminal.InterfaceSoundVolume).c_str());
        //GetStringAttribute(bOverWrite,"check_in_local_base",InterfaceNode,"0");
        Terminal.NoChangeMessage = GetIntAttribute(bOverWrite,"no_change_message",InterfaceNode,Terminal.NoChangeMessage);
        JSMaker->AddStringAttribute("NoChangeMessage",AnsiString((int)Terminal.NoChangeMessage).c_str());

        xmlGuard <_di_IXMLNode> NumberNode (GetNode("number",xmlTerminalNode));
        Terminal.SrcNumber = GetChildNodeValue(bOverWrite,"number",xmlTerminalNode,Terminal.SrcNumber.c_str()).c_str();
        Terminal.WriteInCheque = GetIntAttribute(bOverWrite,"write_in_cheque",NumberNode,Terminal.WriteInCheque);
        if (Terminal.WriteInCheque)
          Terminal.NumberForCheque = (Terminal.SrcNumber == "" ? Terminal.Number.c_str() : Terminal.SrcNumber.c_str());
          else
          Terminal.NumberForCheque = Terminal.Number;
        JSMaker->AddStringAttribute("TerminalName",(Terminal.SrcNumber == "" ? Terminal.Number.c_str() : Terminal.SrcNumber.c_str()));

        Terminal.NoteMinAmount = GetDouble(bOverWrite,"note_min_amount",xmlTerminalNode,Terminal.NoteMinAmount);

        Terminal.SupportString = GetChildNodeValue(bOverWrite,"support_string",xmlTerminalNode,Terminal.SupportString.c_str()).c_str();
        JSMaker->AddStringAttribute("SupportString",Terminal.SupportString.c_str());

        Terminal.MainMenuMarqueeString = GetChildNodeValue(bOverWrite,"main_menu_marquee_string",xmlTerminalNode,Terminal.MainMenuMarqueeString.c_str()).c_str();
        JSMaker->AddStringAttribute("MainMenuMarqueeString",Terminal.MainMenuMarqueeString.c_str());

        Terminal.RebootAllowed = GetInt(bOverWrite,"reboot_allowed",xmlTerminalNode,Terminal.RebootAllowed);
        Terminal.DetectWriteErrors = GetInt(bOverWrite,"detect_write_errors",xmlTerminalNode,Terminal.DetectWriteErrors);
        Terminal.SetWebclientHighPriority = GetInt(bOverWrite,"set_high_priority",xmlTerminalNode,Terminal.SetWebclientHighPriority);
        //Terminal->StayOnTop = GetInt(bOverWrite,"stay_on_top",xmlTerminalNode,Terminal->StayOnTop);
      xmlGuard <_di_IXMLNode> xmlDirsNode (GetNode("dirs",RootNode));
      if (xmlDirsNode.Assigned())
        {
        Dirs.PaymentsOutbound = Dirs.WorkDir+"outbound\\payments";
        if (bCreateDirs) CheckDir(Dirs.PaymentsOutbound.c_str());
        Dirs.PaymentsOutboundTemp = Dirs.WorkDir+"outbound\\payments\\temp";
        if (bCreateDirs) CheckDir(Dirs.PaymentsOutboundTemp.c_str());
        Dirs.PaymentsUnprocessed = Dirs.WorkDir+"outbound\\payments\\unprocessed";
        if (bCreateDirs) CheckDir(Dirs.PaymentsUnprocessed.c_str());
        Dirs.PaymentsBad = Dirs.WorkDir+"outbound\\payments\\bad";
        if (bCreateDirs) CheckDir(Dirs.PaymentsBad.c_str());

        Dirs.StatOutbound = Dirs.WorkDir+"outbound\\stat";
        if (bCreateDirs) CheckDir(Dirs.StatOutbound.c_str());
        Dirs.StatOutboundTemp = Dirs.WorkDir+"outbound\\stat\\temp";
        if (bCreateDirs) CheckDir(Dirs.StatOutboundTemp.c_str());
        Dirs.StatOutboundBad = Dirs.WorkDir+"outbound\\stat\\bad";
        if (bCreateDirs) CheckDir(Dirs.StatOutboundBad.c_str());

        Dirs.EMailOutbound = Dirs.WorkDir+"outbound\\email";
        if (bCreateDirs) CheckDir(Dirs.EMailOutbound.c_str());
        Dirs.EMailOutboundTemp = Dirs.WorkDir+"outbound\\email\\temp";
        if (bCreateDirs) CheckDir(Dirs.EMailOutboundTemp.c_str());

        if ((EMailInfo.SMailHost == "external_sender")||(EMailInfo.Ext))
          {
          Dirs.EMailOutboundExt = Dirs.WorkDir+"outbound\\email\\ext";
          if (bCreateDirs) CheckDir(Dirs.EMailOutboundExt.c_str());
          Dirs.EMailOutboundExtTemp = Dirs.WorkDir+"outbound\\email\\ext\\temp";
          if (bCreateDirs) CheckDir(Dirs.EMailOutboundExtTemp.c_str());
          }

        Dirs.SMSOutbound = Dirs.WorkDir+"outbound\\sms";
        if (bCreateDirs) CheckDir(Dirs.SMSOutbound.c_str());
        Dirs.SMSOutboundTemp = Dirs.WorkDir+"outbound\\sms\\temp";
        if (bCreateDirs) CheckDir(Dirs.SMSOutboundTemp.c_str());

        Dirs.StatusFileName = Dirs.WorkDir+"config\\Details.xml";

        Dirs.CommandsInbound = Dirs.WorkDir+"inbound";
        if (bCreateDirs) CheckDir(Dirs.CommandsInbound.c_str());

        xmlGuard <_di_IXMLNode> InterfaceDirNode (GetNode("interface",xmlDirsNode));
        AnsiString strtmp=GetStringAttribute(bOverWrite,"path",InterfaceDirNode,".\\interface").c_str();
        if(strtmp.Pos("./"))
            strtmp=strtmp.SubString(strtmp.Pos("./")+2,strtmp.Length()-strtmp.Pos("./"));
        if(strtmp.Pos(".\\"))
            strtmp=strtmp.SubString(strtmp.Pos(".\\")+2,strtmp.Length()-strtmp.Pos(".\\"));
        if(strtmp=="")
            strtmp="interface";
        Dirs.InterfaceDir = Dirs.WorkDir+strtmp.c_str();
        //Dirs.InterfaceDir = Dirs.WorkDir+"interface";

        Dirs.InterfaceSkinName = GetStringAttribute(bOverWrite,"skin_name",InterfaceDirNode,Dirs.InterfaceSkinName.c_str()).c_str();
        JSMaker->AddStringAttribute("SkinName",Dirs.InterfaceSkinName.c_str());
        JSMaker->AddStringAttribute("ShowPB", static_cast<int>(Terminal.ShowPB));

        Dirs.DBNumCapacityUpdateAddress = GetStringAttribute(bOverWrite,"db_numcapacity_update_address",InterfaceDirNode,Dirs.DBNumCapacityUpdateAddress.c_str()).c_str();

        boost::algorithm::replace_all(Dirs.InterfaceDir, "\\", "/");
        }

      xmlGuard <_di_IXMLNode> xmlDealerInfo (GetNode("dealer_info",RootNode));
      if (xmlDealerInfo.Assigned())
      {
          //DealerInfo.DealerName = GetString(bOverWrite,"dealer_name",xmlDealerInfo.get(),DealerInfo.DealerName);
          DealerInfo.DealerName = GetChildNodeValue(bOverWrite, "dealer_name", xmlDealerInfo, DealerInfo.DealerName.c_str()).c_str();
          DealerInfo.DealerAddress = GetChildNodeValue(bOverWrite, "dealer_address", xmlDealerInfo, DealerInfo.DealerAddress.c_str()).c_str();
          DealerInfo.BusinessDealerAddress = GetChildNodeValue(bOverWrite, "business_dealer_address", xmlDealerInfo, DealerInfo.BusinessDealerAddress.c_str()).c_str();
          JSMaker->AddStringAttribute("BusinessDealerAddress",DealerInfo.BusinessDealerAddress.c_str());
          DealerInfo.DealerINN = GetChildNodeValue(bOverWrite, "dealer_inn", xmlDealerInfo, DealerInfo.DealerINN.c_str()).c_str();
          DealerInfo.DealerPhone = GetChildNodeValue(bOverWrite, "dealer_phone", xmlDealerInfo, DealerInfo.DealerPhone.c_str()).c_str();
          DealerInfo.PointAddress = GetChildNodeValue(bOverWrite, "point_address", xmlDealerInfo, DealerInfo.PointAddress.c_str()).c_str();
          DealerInfo.ContractNumber = GetChildNodeValue(bOverWrite, "contract_number", xmlDealerInfo, DealerInfo.ContractNumber.c_str()).c_str();
          DealerInfo.BankName = GetChildNodeValue(bOverWrite, "bank_name", xmlDealerInfo, DealerInfo.BankName.c_str()).c_str();
          DealerInfo.BankBIK = GetChildNodeValue(bOverWrite, "bank_bik", xmlDealerInfo, DealerInfo.BankBIK.c_str()).c_str();
          DealerInfo.BankPhone = GetChildNodeValue(bOverWrite, "bank_phone", xmlDealerInfo, DealerInfo.BankPhone.c_str()).c_str();
          DealerInfo.BankAddress = GetChildNodeValue(bOverWrite, "bank_address", xmlDealerInfo, DealerInfo.BankAddress.c_str()).c_str();
          DealerInfo.BankINN = GetChildNodeValue(bOverWrite, "bank_inn", xmlDealerInfo, DealerInfo.BankINN.c_str()).c_str();
      }

      xmlGuard <_di_IXMLNode> ConnectionsNode (GetNode("connections",RootNode));
      xmlGuard <_di_IXMLNode> ConnNode;
      xmlGuard <_di_IXMLNodeList> ConnectionsNDL;
      if (ConnectionsNode.Assigned())
        {
        if (ConnectionsNode->HasChildNodes)
          {
          ConnectionsNDL = ConnectionsNode->GetChildNodes();

          if (bOverWrite)
            {
            for (int i = ConnectionsNDL->Count-1; i>=0; i--)
              {
              ConnNode = ConnectionsNDL->Get(i);
              if (ConnNode->NodeName == WideString("alt_connection"))
                ConnectionsNDL->Delete(i);
              }

            if (RASConnections.size()>1)
              {
              for (unsigned j = 1; j<RASConnections.size();j++)                              // Сохраняем информацию о соединениях...
                {
                for (int i = 1; i<ConnectionsNDL->Count; i++)                           // Ищем соединение...
                  {
                  ConnNode = ConnectionsNDL->Get(i);
                  if (ConnNode->NodeName == WideString("alt_connection"))
                    {
                    if (IsAttributeExists("name",ConnNode))
                      {
                      AnsiString ConnectionName = GetStringAttribute(bOverWrite,"name",ConnNode,"").c_str();
                      if (ConnectionName!= "")
                        {
                        if (ConnectionName == RASConnections[j].Name.c_str())
                          break;
                          else
                          ConnNode = NULL;
                        }
                        else
                        ConnNode = NULL;
                      }
                      else
                      ConnNode = NULL;
                    }
                    else
                    ConnNode = NULL;
                  }
                if (!ConnNode.Assigned())
                  ConnNode = GetNode("alt_connection",ConnectionsNode);
                RASConnections[j].Name = GetStringAttribute(bOverWrite,"name",ConnNode,RASConnections[j].Name.c_str()).c_str();
                RASConnections[j].DisconnectTime = GetIntAttribute(bOverWrite,"disconnect_time",ConnNode,RASConnections[j].DisconnectTime);
                RASConnections[j].InitCmd = GetStringAttribute(bOverWrite,"init_cmd",ConnNode,RASConnections[j].InitCmd.c_str()).c_str();
                RASConnections[j].Login = GetStringAttribute(bOverWrite,"login",ConnNode,RASConnections[j].Login.c_str()).c_str();
                RASConnections[j].Password = GetStringAttribute(bOverWrite,"password",ConnNode,RASConnections[j].Password.c_str()).c_str();
                RASConnections[j].InitString[0] = GetStringAttribute(bOverWrite,"init_string",ConnNode,RASConnections[j].InitString[0].c_str()).c_str();
                RASConnections[j].InitString[1] = GetStringAttribute(bOverWrite,"alt_init_string",ConnNode,RASConnections[j].InitString[1].c_str()).c_str();

                xmlGuard <_di_IXMLNode> ProxyNode (GetNode("proxy",ConnNode));

                //RASConnections[j]->HTTPProxy = new _proxy_info;
                RASConnections[j].Proxy = new TSocksInfo();

                RASConnections[j].HTTPProxy.CfgType = GetStringAttribute(bOverWrite,"type",ProxyNode,RASConnections[j].HTTPProxy.CfgType.c_str()).c_str();

                RASConnections[j].HTTPProxy.Host = GetStringAttribute(bOverWrite,"host",ProxyNode,RASConnections[j].HTTPProxy.Host.c_str()).c_str();
                RASConnections[j].Proxy->Host = RASConnections[j].HTTPProxy.Host.c_str();

                RASConnections[j].HTTPProxy.Port = GetIntAttribute(bOverWrite,"port",ProxyNode,RASConnections[j].HTTPProxy.Port);
                RASConnections[j].Proxy->Port = RASConnections[j].HTTPProxy.Port;

                RASConnections[j].HTTPProxy.UserName = GetStringAttribute(bOverWrite,"username",ProxyNode,RASConnections[j].HTTPProxy.UserName.c_str()).c_str();
                RASConnections[j].Proxy->UserID = RASConnections[j].HTTPProxy.UserName.c_str();

                RASConnections[j].HTTPProxy.Password = GetStringAttribute(bOverWrite,"password",ProxyNode,RASConnections[j].HTTPProxy.Password.c_str()).c_str();
                RASConnections[j].Proxy->Password = RASConnections[j].HTTPProxy.Password.c_str();

                if (AnsiString(RASConnections[j].HTTPProxy.CfgType.c_str()).LowerCase() == "socks4")
                  {
                  RASConnections[j].HTTPProxy.Type = "socks";
                  RASConnections[j].Proxy->Version = svSocks4;
                  }
                  else
                  if (AnsiString(RASConnections[j].HTTPProxy.CfgType.c_str()).LowerCase() == "socks4a")
                    {
                    RASConnections[j].HTTPProxy.Type = "socks";
                    RASConnections[j].Proxy->Version = svSocks4A;
                    }
                    else
                    if (AnsiString(RASConnections[j].HTTPProxy.CfgType.c_str()).LowerCase() == "socks5")
                      {
                      RASConnections[j].HTTPProxy.Type = "socks";
                      RASConnections[j].Proxy->Version = svSocks5;
                      }
                      else
                      {
                      RASConnections[j].Proxy->Version = svNoSocks;
                      if (AnsiString(RASConnections[j].HTTPProxy.CfgType.c_str()).LowerCase() == "http")
                        RASConnections[j].HTTPProxy.Type = "http";
                        else
                        RASConnections[j].HTTPProxy.Type = "none";
                      }

                if (RASConnections[j].HTTPProxy.UserName == "")
                  RASConnections[j].Proxy->Authentication = saNoAuthentication;
                  else
                  RASConnections[j].Proxy->Authentication = saUsernamePassword;
                }
              }
            }
            else
            {
            //int ConnectionIndex;
            for (int i = 0; i<ConnectionsNDL->Count; i++)
              {
              ConnNode = ConnectionsNDL->Get(i);
              if (ConnNode->NodeName == WideString("alt_connection"))
                {
                if (IsAttributeExists("name",ConnNode))
                  {
                  AnsiString ConnectionName = GetStringAttribute(bOverWrite,"name",ConnNode,"").c_str();
                  if (ConnectionName!= "")
                    {
                    ConnInfo NewConnInfo;
                    //NewConnInfo = new TConnInfo;
                    //NewConnInfo->HTTPProxy = new _proxy_info;
                    //NewConnInfo->Proxy = new TSocksInfo();
                    NewConnInfo.Name = ConnectionName.c_str();
                    NewConnInfo.Name = GetStringAttribute(bOverWrite,"name",ConnNode,NewConnInfo.Name.c_str()).c_str();
                    NewConnInfo.DisconnectTime = GetIntAttribute(bOverWrite,"disconnect_time",ConnNode,NewConnInfo.DisconnectTime);
                    NewConnInfo.InitCmd = GetStringAttribute(bOverWrite,"init_cmd",ConnNode,NewConnInfo.InitCmd.c_str()).c_str();
                    NewConnInfo.Login = GetStringAttribute(bOverWrite,"login",ConnNode,NewConnInfo.Login.c_str()).c_str();
                    NewConnInfo.Password = GetStringAttribute(bOverWrite,"password",ConnNode,NewConnInfo.Password.c_str()).c_str();
                    NewConnInfo.InitString[0] = GetStringAttribute(bOverWrite,"init_string",ConnNode,NewConnInfo.InitString[0].c_str()).c_str();
                    NewConnInfo.InitString[1] = GetStringAttribute(bOverWrite,"alt_init_string",ConnNode,NewConnInfo.InitString[1].c_str()).c_str();

                    xmlGuard <_di_IXMLNode> ProxyNode (GetNode("proxy",ConnNode));

                    //NewConnInfo->HTTPProxy = new _proxy_info;
                    //NewConnInfo->Proxy = new TSocksInfo();

                    NewConnInfo.HTTPProxy.CfgType = GetStringAttribute(bOverWrite,"type",ProxyNode,NewConnInfo.HTTPProxy.CfgType.c_str()).c_str();

                    NewConnInfo.HTTPProxy.Host = GetStringAttribute(bOverWrite,"host",ProxyNode,NewConnInfo.HTTPProxy.Host.c_str()).c_str();
                    NewConnInfo.Proxy->Host = NewConnInfo.HTTPProxy.Host.c_str();

                    NewConnInfo.HTTPProxy.Port = GetIntAttribute(bOverWrite,"port",ProxyNode,NewConnInfo.HTTPProxy.Port);
                    NewConnInfo.Proxy->Port = NewConnInfo.HTTPProxy.Port;

                    NewConnInfo.HTTPProxy.UserName = GetStringAttribute(bOverWrite,"username",ProxyNode,NewConnInfo.HTTPProxy.UserName.c_str()).c_str();
                    NewConnInfo.Proxy->UserID = NewConnInfo.HTTPProxy.UserName.c_str();

                    NewConnInfo.HTTPProxy.Password = GetStringAttribute(bOverWrite,"password",ProxyNode,NewConnInfo.HTTPProxy.Password.c_str()).c_str();
                    NewConnInfo.Proxy->Password = NewConnInfo.HTTPProxy.Password.c_str();

                    if (AnsiString(NewConnInfo.HTTPProxy.CfgType.c_str()).LowerCase() == "socks4")
                      {
                      NewConnInfo.HTTPProxy.Type = "socks";
                      NewConnInfo.Proxy->Version = svSocks4;
                      }
                      else
                      if (AnsiString(NewConnInfo.HTTPProxy.CfgType.c_str()).LowerCase() == "socks4a")
                        {
                        NewConnInfo.HTTPProxy.Type = "socks";
                        NewConnInfo.Proxy->Version = svSocks4A;
                        }
                        else
                        if (AnsiString(NewConnInfo.HTTPProxy.CfgType.c_str()).LowerCase() == "socks5")
                          {
                          NewConnInfo.HTTPProxy.Type = "socks";
                          NewConnInfo.Proxy->Version = svSocks5;
                          }
                          else
                          {
                          NewConnInfo.Proxy->Version = svNoSocks;
                          if (AnsiString(NewConnInfo.HTTPProxy.CfgType.c_str()).LowerCase() == "http")
                            NewConnInfo.HTTPProxy.Type = "http";
                            else
                            NewConnInfo.HTTPProxy.Type = "none";
                          }

                    if (NewConnInfo.HTTPProxy.UserName == "")
                      NewConnInfo.Proxy->Authentication = saNoAuthentication;
                      else
                      NewConnInfo.Proxy->Authentication = saUsernamePassword;
                    if (Peripherals.Modem.Port)
                      RASConnections.push_back(NewConnInfo);                   // добавляем новое соединение.
                    }
                  }
                  else
                  continue;
                }
              }
            }
          }
        }
      }

/*      for (int i=0;i<RASConnections.size();i++)
      {
        Log->Write("#" + AnsiString(i) + " : " + RASConnections[i].GetString() );
      }*/

      if (bSaveFile)
        {
        Log->Write("Trying to save config file...");
        try
          {
          AnsiString Temp;
          XmlDoc->SaveToXML(Temp);
          if (!SaveFile(CfgFile,Temp))
            {
            Log->Write("Error writing config file to disk!");
            //_FileSystemError = true;
            bResult = false;
            }
            else
            {
            Log->Append("OK.");
            bResult = true;
            }
          }
          catch(...)
          {
              ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
              Log->Write("Exception in SaveFile");
          }
        }
      }
    Prepared = true;
		}
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		}
	}
__finally
	{
  JSMaker->CloseChild();

	if (CfgFile)
		{
		Log->Write("Config file closed.");
		delete CfgFile;
		CfgFile = NULL;
		}
                CS->Release();

	}
return bResult;
}

//---------------------------------------------------------------------------

bool TWConfig::isOperatorExists(int _OpId)
{
    std::vector<TOpInfo>::iterator it;
    for(it=OperatorsInfo.begin();it!=OperatorsInfo.end();++it)
    {
        if((*it).Id==_OpId)
            return true;
    }
    return false;
}

//---------------------------------------------------------------------------

TOpInfo TWConfig::Operator(int _OpId)
{
    std::vector<TOpInfo>::iterator it;
    for(it=OperatorsInfo.begin();it!=OperatorsInfo.end();++it)
    {
        if((*it).Id==_OpId)
            return (*it);
    }
    return BlankOper;
}


//---------------------------------------------------------------------------

_groups_info TWConfig::Group(int _GrId)
{
    std::vector<_groups_info>::iterator it;
    for(it=GroupsInfo.begin();it!=GroupsInfo.end();++it)
    {
        if((*it).Id==_GrId)
            return (*it);
    }
    return BlankGroup;
}

//---------------------------------------------------------------------------

TOpInfo TWConfig::OperatorByNum(int _OpNum)
{
    if (_OpNum<(int)OperatorsInfo.size() && _OpNum >= 0)
        return OperatorsInfo[_OpNum];
    return BlankOper;
}

//---------------------------------------------------------------------------

void TWConfig::SetCardsInfo(int _OpId, AnsiString _Source, TDateTime _ActualDT)
{
    try
    {
        std::vector<TOpInfo>::iterator it;
        for(it=OperatorsInfo.begin();it!=OperatorsInfo.end();++it)
        {
            if((*it).Id==_OpId)
            {
                (*it).ACardsInfo = _Source.c_str();
                (*it).DTCardsInfo = AnsiString(_ActualDT).c_str();
            }
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

AnsiString TWConfig::DeleteCardInfo(int _OpId, AnsiString _CardIdToDelete)
{
    AnsiString Res;
    try
    {
        TOpInfo OperatorInfo = Operator(_OpId);
        AnsiString Temp = OperatorInfo.ACardsInfo.c_str();
        Temp+= ":";
        while (Temp.Pos(":"))
        {
            AnsiString Temp2;
            Temp2 = Temp.SubString(0,Temp.Pos(":"));

            AnsiString Temp3 = Temp2;
            Temp3 = Temp3.SubString(Temp2.Pos("=")+1,Temp2.Length());
        //      Temp3 = Temp3.SubString(Temp3.Pos("=")+1,Temp3.Length());
            Temp3 = Temp3.SubString(0,Temp3.Pos("=")-1);
            if (Temp3!= _CardIdToDelete)
            {
                Res+= Temp2;
            }
            Temp = Temp.SubString(Temp.Pos(":")+1,Temp.Length());
        }
        OperatorInfo.DTCardsInfo = "0";
        if (Res.Length()>0)
            Res = Res.SubString(0,Res.Length()-1);
        return Res;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return Res;
    }
}

//---------------------------------------------------------------------------

bool TWConfig::isXMLParserOK()
{
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
  }
  catch(...)
  {
    ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    return false;
  }
return true;
}


/*bool TWConfig::SaveOperatorInfo(TOpInfo OpInfo)
{
bool bResult = true;
_di_IXMLNode xmlNode, xmlRoot, xmlSecNode, xmlParamNode, OperatorNode;
_di_IXMLNodeList xmlRootNodeList, xmlNodeList;
_di_IXMLDocument XmlDoc;
try
	{
	try
		{
		if (FileExists(OperatorsFileName))
			{
			AnsiString FileData;
			bool bRes = OpenFile(OperFile, OperatorsFileName, FileData);
			if (bRes)
				{
				try
					{
					XmlDoc = LoadXMLData(FileData);
					}
				catch (Exception &exception)                                              // не смогли распарсить содержимое файла
					{
					FileData = FileData.SubString(0,FileData.Pos("</root>")+6);
					XmlDoc = LoadXMLData(FileData);
					}				//XmlDoc = LoadXMLData(FileData);
				XmlDoc->Options = XmlDoc->Options << doNodeAutoIndent;

				_di_IXMLNode RootNode = XmlDoc->GetDocumentElement();
				if (RootNode->HasChildNodes) {
					xmlRootNodeList = RootNode->GetChildNodes();
					xmlSecNode = xmlRootNodeList->FindNode("operators");
					if ((xmlSecNode!= NULL)&&(xmlSecNode->HasChildNodes)) {
							xmlNodeList = xmlSecNode->GetChildNodes();
							for (int i = 0; (i<xmlNodeList->Count); i++) {
									OperatorNode = xmlNodeList->Get(i);
									if ((OperatorNode!= NULL)&&(OperatorNode->NodeName == WideString("operator"))&&(OperatorNode->HasChildNodes)) {
											int OperatorId = RequestTime = GetIntAttribute(false,"id",OperatorNode,0);

											if (OperatorId!= OpInfo.Id)
												continue;

											SetNodeValue(GetNode("name",OperatorNode), OpInfo.Name);

											_di_IXMLNode LimitNode = GetNode("limit",OperatorNode);
											SetAttribute("min",LimitNode, OpInfo.LimMin);
											SetAttribute("max",LimitNode, OpInfo.LimMax);

											DeleteNode("comission",OperatorNode);
											_di_IXMLNode ComissionNode = GetNode("comission",OperatorNode);
											for (int i = 0;i<OpInfo.CommPartsCount;i++)
												{
												_di_IXMLNode ComPartNode = CreateNode("part",ComissionNode,"");
												if (OpInfo.Comission[i].Min>0)
													SetAttribute("min",ComPartNode, OpInfo.Comission[i].Min);
												if (OpInfo.Comission[i].Relative)
													SetAttribute("value",ComPartNode, AnsiString(OpInfo.Comission[i].Value)+"%");
													else
													SetAttribute("value",ComPartNode, OpInfo.Comission[i].Value);
												}

											_di_IXMLNode ProcessorNode = GetNode("processor",OperatorNode);
											//SetAttribute("offline",ProcessorNode, OpInfo.Offline);

											_di_IXMLNode PrCheckNode = GetNode("check",ProcessorNode);
											SetAttribute("amount-value",PrCheckNode, OpInfo.CheckAmount);

											//XmlDoc->SaveToFile(OperatorsFileName);
											//SaveFile(OperFile,);
											Log->Write("Trying to save config file...");
											try
												{
//												AnsiString Temp = (XmlDoc->GetXML())->DelimitedText;
												AnsiString Temp;
												XmlDoc->SaveToXML(Temp);
												if (!SaveFile(OperFile,Temp))
													{
													Log->Write("Error writing operators.xml to disk!");
													//_FileSystemError = true;
													}
													else
													Log->Append("OK.");
												}
											catch (Exception &exception)
												{
												Log->Append("Error! "+exception.Message);
												//FileMap->WriteErrorFound = true;
												}
											break;
											}
									}
							}
					}
				}
				else
				{
				Log->Write("Error getting data from "+OperatorsFileName+"!");
				bResult = false;
				}
			}
			else
			{
			Log->Write("File "+OperatorsFileName+" does not exists.");
			bResult = false;
			}
		}
	catch (Exception &exception)
		{
		Log->Write("Operators config file Error: Exception Occured: "+exception.Message);
		}
	}
__finally
	{
	if (XmlDoc!= NULL)
		XmlDoc.Release();

	if (OperFile)
		{
		Log->Write("Operators file closed.");
		delete OperFile;
		OperFile = NULL;
		}
			return bResult;
	}
}*/

//---------------------------------------------------------------------------
void TWConfig::GetMenuTree(TTreeView* TV)
{
try
  {
	try
		{
		if (FileExists(OperatorsFileName))
			{
      xmlGuard <_di_IXMLDocument> XmlDoc;
			AnsiString FileData;
			bool bRes = OpenFile(OperFile, OperatorsFileName, FileData);
			if (bRes)
				{
				try
					{
					XmlDoc = LoadXMLData(FileData);
					}
                                catch(...)
                                {
                                    ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
					FileData = FileData.SubString(0,FileData.Pos("</root>")+6);
					XmlDoc = LoadXMLData(FileData);
					}
				XmlDoc->Options = XmlDoc->Options << doNodeAutoIndent;

        Menu = (TV ? TV->Items : NULL);


				xmlGuard <_di_IXMLNode> RootNode (XmlDoc->GetDocumentElement());
				if (RootNode->HasChildNodes)
          {
					xmlGuard <_di_IXMLNodeList> RootNDL (RootNode->GetChildNodes());
					xmlGuard <_di_IXMLNode> MenuNode (RootNDL->FindNode("menu"));
					//TTreeNode* RootNode = Menu->AddChildObject(NULL, "", NULL);
					//ProcessMenuGroup(MenuNode, RootNode);
					ProcessMenuGroup(RootNode);
					}
				}
				else
				{
				Log->Write((boost::format("Error getting data from %1%!") % OperatorsFileName.c_str()).str().c_str());
				}
			}
			else
			{
			Log->Write((boost::format("File %1% does not exists.") % OperatorsFileName.c_str()).str().c_str());
			}
		}
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		}
	}
__finally
	{
	if (OperFile)
		{
		Log->Write("Operators file closed.");
		delete OperFile;
		OperFile = NULL;
		}
	}
}

//---------------------------------------------------------------------------
void TWConfig::ProcessMenuGroup(xmlGuard <_di_IXMLNode>& ProcessedNode, TTreeNode* ParentNode, AnsiString Margin)
{
try
	{
	if (ProcessedNode->HasChildNodes) {
		xmlGuard <_di_IXMLNodeList> ProcessedNDL (ProcessedNode->GetChildNodes());
		for (int i = 0; (i<ProcessedNDL->Count); i++) {
			xmlGuard <_di_IXMLNode> ItemNode (ProcessedNDL->Get(i));
			int NodeID = GetIntAttribute(false,"id",ItemNode,-1,true);
			if (ItemNode->NodeName == AnsiString("menu")) {
        if (Menu)
          {
          TMenuItemData NewMenuItemData;
          NewMenuItemData.Type = cnMIRoot;
          NewMenuItemData.Id = 0;
          NewMenuItemData.Name = GetStringAttribute(false,"title",ItemNode,"",true).c_str();
          NewMenuItemData.Columns = GetIntAttribute(false,"columns",ItemNode,-1,true);
          NewMenuItemData.Rows = GetIntAttribute(false,"rows",ItemNode,-1,true);
  				Log->Write((boost::format("Group->Name: %1%") % NewMenuItemData.Name.c_str()).str().c_str());
	  			TTreeNode* NewNode = Menu->AddChildObject(ParentNode, NewMenuItemData.Name.c_str(), &NewMenuItemData);
  				ProcessMenuGroup(ItemNode, NewNode, Margin+"  ");
          }
          else
          {
          JSMaker->AddChild("0");
          JSMaker->AddStringAttribute("name",GetStringAttribute(false,"title",ItemNode,"",true).c_str());
          JSMaker->AddStringAttribute("columns",AnsiString(GetIntAttribute(false,"columns",ItemNode,-1,true)).c_str());
          JSMaker->AddStringAttribute("rows",AnsiString(GetIntAttribute(false,"rows",ItemNode,-1,true)).c_str());
          ProcessMenuGroup(ItemNode, NULL, Margin+"  ");
          JSMaker->CloseChild();
          }
				}
				else
				if (ItemNode->NodeName == AnsiString("group")) {
          if (Menu)
            {
            TMenuItemData NewMenuItemData;
            NewMenuItemData.Type = cnMIGroup;
            NewMenuItemData.Id = NodeID;
            NewMenuItemData.Name = GetStringAttribute(false,"title",ItemNode,"",true).c_str();
            NewMenuItemData.Image = GetStringAttribute(false,"image",ItemNode,"",true).c_str();
            NewMenuItemData.TitleImage = GetStringAttribute(false,"titleimage",ItemNode,"",true).c_str();
            NewMenuItemData.Columns = GetIntAttribute(false,"columns",ItemNode,-1,true);
            NewMenuItemData.Rows = GetIntAttribute(false,"rows",ItemNode,-1,true);
            Log->Write((boost::format("Group->Name: %1%") % NewMenuItemData.Name.c_str()).str().c_str());
            TTreeNode* NewNode = Menu->AddChildObject(ParentNode, NewMenuItemData.Name.c_str(), &NewMenuItemData);
  					ProcessMenuGroup(ItemNode, NewNode, Margin+"  ");
            }
            else
            {
            JSMaker->AddChild(AnsiString(NodeID));
            JSMaker->AddStringAttribute("name",GetStringAttribute(false,"title",ItemNode,"",true).c_str());
            xmlGuard <_di_IXMLNodeList> AttribsNDL (ItemNode->AttributeNodes);
            for (int i=0;i<AttribsNDL->Count;i++)
              {
              xmlGuard <_di_IXMLNode> CurrAttr (AttribsNDL->Get(i));
              if (CurrAttr->NodeName!=WideString("title"))
                JSMaker->AddStringAttribute(CurrAttr->NodeName,CurrAttr->Text);
              }
            JSMaker->AddStringAttribute("image",GetStringAttribute(false,"image",ItemNode,"",true).c_str());
            JSMaker->AddStringAttribute("titleimage",GetStringAttribute(false,"titleimage",ItemNode,"",true).c_str());
            JSMaker->AddStringAttribute("columns",AnsiString(GetIntAttribute(false,"columns",ItemNode,-1,true)));
            JSMaker->AddStringAttribute("rows",AnsiString(GetIntAttribute(false,"rows",ItemNode,-1,true)));
     				ProcessMenuGroup(ItemNode, NULL, Margin+"  ");
            JSMaker->CloseChild();
            }
					}
					else
					if (ItemNode->NodeName == AnsiString("operator_id"))
						{
            if (Menu)
              {
              TMenuItemData NewMenuItemData;
              NewMenuItemData.Type = cnMIOperator;
              NewMenuItemData.Id = NodeID;
              NewMenuItemData.Name = Operator(NodeID).Name.c_str();
              NewMenuItemData.Image = "";
              NewMenuItemData.TitleImage = "";
              NewMenuItemData.Columns = 0;
              NewMenuItemData.Rows = 0;
              Log->Write((boost::format("Id->Name: %1%") % NewMenuItemData.Name.c_str()).str().c_str());
              Menu->AddChildObject(ParentNode, NewMenuItemData.Name.c_str(), &NewMenuItemData);
              }
              else
              {
                xmlGuard <_di_IXMLNodeList> AttribsNDL (ItemNode->AttributeNodes);
                if((image999.empty()) || (NodeID != 999))
                {
                  JSMaker->AddChild("op"+AnsiString(NodeID));
                  for (int i=0;i<AttribsNDL->Count;i++)
                    {
                    xmlGuard <_di_IXMLNode> CurrAttr (AttribsNDL->Get(i));
                    JSMaker->AddStringAttribute(CurrAttr->NodeName,CurrAttr->Text);
                    }
    /*              JSMaker->AddStringAttribute("name",Operator(NodeID)->Name);
                  JSMaker->AddStringAttribute("image",Operator(NodeID)->Image);
                  JSMaker->AddStringAttribute("rootmenuimage",Operator(NodeID)->RootMenuImage);*/
                  //JSMaker->AddStringAttribute("name",GetStringAttribute(false,"title",ItemNode,"",true));
                  //JSMaker->AddStringAttribute("image",GetStringAttribute(false,"image",ItemNode,"",true));
                  //JSMaker->AddStringAttribute("titleimage",GetStringAttribute(false,"titleimage",ItemNode,"",true));
                  JSMaker->CloseChild();
                }
                else
                  if(!append999)
                    {
                      append999 = true;
                      JSMaker->AddChild("op_999");
                      for (int i=0;i<AttribsNDL->Count;i++)
                      {
                        xmlGuard <_di_IXMLNode> CurrAttr (AttribsNDL->Get(i));
                        if (AnsiString(CurrAttr->NodeName) == "image")
                        {
                          JSMaker->AddChild("image");
                          int tempCounter = 0;
                          for(std::vector<std::string>::iterator it = image999.begin(); it != image999.end(); it++)
                            JSMaker->AddStringAttribute(AnsiString(tempCounter++).c_str(),(*it).c_str());
                          JSMaker->CloseChild();
                        }
                        else
                          JSMaker->AddStringAttribute(CurrAttr->NodeName,CurrAttr->Text);
                      }
                      JSMaker->CloseChild();
                    }
              }
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

/*bool TWConfig::SaveMenuTree()
{
bool bResult = true;
_di_IXMLDocument XmlDoc;
try
	{
	try
		{
		if (FileExists(OperatorsFileName))
			{
			AnsiString FileData;
			bool bRes = OpenFile(OperFile, OperatorsFileName, FileData);
			if (bRes)
				{
				try
					{
					XmlDoc = LoadXMLData(FileData);
					}
				catch (Exception &exception)                                              // не смогли распарсить содержимое файла
					{
					FileData = FileData.SubString(0,FileData.Pos("</root>")+6);
					XmlDoc = LoadXMLData(FileData);
					}				//XmlDoc = LoadXMLData(FileData);
				XmlDoc->Options = XmlDoc->Options << doNodeAutoIndent;

				_di_IXMLNode RootNode = XmlDoc->GetDocumentElement();
				if (RootNode->HasChildNodes) {
					_di_IXMLNodeList RootNDL = RootNode->GetChildNodes();
					DeleteNode("menu",RootNode);
					_di_IXMLNode MenuNode = GetNode("menu",RootNode);
					PMenuItemData NewMenuItemData = (PMenuItemData)Menu->Item[0]->Data;
					if (NewMenuItemData!= NULL)
						{
						SetAttribute("title",MenuNode, NewMenuItemData->Name);
						SetAttribute("columns",MenuNode, NewMenuItemData->Columns);
						if (NewMenuItemData->Rows!= -1)
							SetAttribute("rows",MenuNode, NewMenuItemData->Rows);
						}
					SaveMenuGroup(MenuNode, Menu->Item[0]);
					Log->Write("Trying to save operators.xml...");
					try
						{
//						AnsiString Temp = (XmlDoc->GetXML())->DelimitedText;
						AnsiString Temp;
						XmlDoc->SaveToXML(Temp);
						Log->Write("XML: \n"+Temp);
						if (!SaveFile(OperFile, Temp))
							{
							Log->Write("Error writing file to disk!");
							bResult = false;
							}
							else
							Log->Append("OK.");
						}
					catch (Exception &exception)
						{
						Log->Append("Error! "+exception.Message);
						bResult = false;
						}
					}
				}
				else
				{
				Log->Write("Error getting data from "+OperatorsFileName+"!");
				bResult = false;
				}
			}
			else
			{
			Log->Write("File "+OperatorsFileName+" does not exists.");
			bResult = false;
			}
		}
	catch (Exception &exception)
		{
		Log->Write("Operators config file Error: Exception Occured: "+exception.Message);
		}
	}
__finally
	{
	if (XmlDoc!= NULL)
		XmlDoc.Release();

	if (OperFile)
		{
		Log->Write("Operators file closed.");
		delete OperFile;
		OperFile = NULL;
		}
	return bResult;
	}
}*/

//---------------------------------------------------------------------------

/*void TWConfig::SaveMenuGroup(_di_IXMLNode ProcessedNode, TTreeNode* ParentNode, AnsiString Margin)
{
try
	{
	if (ParentNode->HasChildren) {
		for (int i = 0; (i<ParentNode->Count); i++) {
			PMenuItemData NewMenuItemData = (PMenuItemData)ParentNode->Item[i]->Data;
			if (NewMenuItemData!= NULL)
					if (NewMenuItemData->Type == cnMIGroup)
						{
						_di_IXMLNode NewGroupNode = CreateNode("group",ProcessedNode,"");
						if (NewMenuItemData->Id!= -1)
							SetAttribute("id",NewGroupNode, NewMenuItemData->Id);
							SetAttribute("title",NewGroupNode, NewMenuItemData->Name);
							SetAttribute("image",NewGroupNode, NewMenuItemData->Image);
							SetAttribute("titleimage",NewGroupNode, NewMenuItemData->TitleImage);
							SetAttribute("columns",NewGroupNode, NewMenuItemData->Columns);
						if (NewMenuItemData->Rows!= -1)
							SetAttribute("rows",NewGroupNode, NewMenuItemData->Rows);
						SaveMenuGroup(NewGroupNode, ParentNode->Item[i], Margin+"  ");
						}
						else
						if (NewMenuItemData->Type == cnMIOperator)
							{
							_di_IXMLNode NewOperatorNode = CreateNode("operator_id",ProcessedNode,"");
							SetAttribute("id",NewOperatorNode, NewMenuItemData->Id);
							}
			}
		}
	}
catch (Exception &exception)
	{
	Log->Write("TWConfig::SaveMenuGroup: Exception Occured: "+exception.Message);
	}
}*/

//---------------------------------------------------------------------------

int TWConfig::GetKeysNum(int _Id)
{
    for (unsigned i = 0;i<Keys.size();i++)
    {
        if (Keys[i].Id == _Id)
            return i;
    }

    for (unsigned i = 0;i<Keys.size();i++)
    {
        if (Keys[i].Id == 0)
            return i;
    }
    return 0;
}

//---------------------------------------------------------------------------

bool TWConfig::IfKeysExist(int _Id)
{
    for (unsigned i = 0;i<Keys.size();i++)
    {
        if (Keys[i].Id == _Id)
            return true;
    }
    return false;
}

//---------------------------------------------------------------------------

void TWConfig::SortKeys()
{
try
	{
	for (unsigned i = 1;i <= Keys.size();i++)
		{
		for (unsigned j = 1;j <= Keys.size()-i;j++)
			{
			if (Keys[j].Id<Keys[j-1].Id)
				{
				_keys_info Temp;
				Temp = Keys[j];
				Keys[j] = Keys[j-1];
				Keys[j-1] = Temp;
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

bool TWConfig::SaveFile(TFileStream* &File, AnsiString SourceData)
{
    bool Res = false;
    try
    {
        if (File != NULL)
        {
            File->Seek(0, soFromBeginning);
            File->Write(SourceData.c_str(), SourceData.Length());
            File->Size = SourceData.Length();
            Res = true;
        }
        else
        {
            Log->Write("File not opened!");
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

AnsiString TWConfig::GetStatServerHost()
{
try
	{
	return StatInfo.Host[StatInfo.CurrentStatServersAdressNum].c_str();
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
	return "";
	}
}

//---------------------------------------------------------------------------

int TWConfig::GetStatServerPort()
{
try
	{
	return StatInfo.Port[StatInfo.CurrentStatServersAdressNum];
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
	    return 1024;
	}
}

//---------------------------------------------------------------------------

void TWConfig::ChangeStatServer()
{
try
	{
	if (StatInfo.StatServersAdressesCount <= 1)
		return;
	StatInfo.CurrentStatServersAdressNum++;
	if (StatInfo.CurrentStatServersAdressNum >= StatInfo.StatServersAdressesCount)
		StatInfo.CurrentStatServersAdressNum = 0;
	Log->Write((boost::format("Stat server configuration changed to %1%:%2%.") % GetStatServerHost().c_str() % GetStatServerPort()).str().c_str());
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
	}
}

//---------------------------------------------------------------------------

bool TWConfig::OpenFile(TFileStream* &File, AnsiString FileName, AnsiString &FileData)
{
bool Res = false;
	try
			{
			if (File!= NULL) {
				delete File;
				File = NULL;
				}
			//FileName = GetNewFileName(Extension);
			if (FileExists(FileName))
				{
				File = new TFileStream(FileName, fmOpenReadWrite | fmShareDenyNone);
				if (File != NULL)
          {
					//Log->Write("File "+FileName+" successfully opened.");

          std::vector<char> Buffer(static_cast<std::size_t>(File->Size)+1, 0 );
          File->Seek(0, soFromBeginning);
          File->Read(&*Buffer.begin(), static_cast<int>(File->Size));
          FileData = AnsiString(&*Buffer.begin(), static_cast<int>(File->Size));

/*					File->Read(Buffer, File->Size);
          FileData = AnsiString(Buffer, File->Size);*/
					Res = true;
					}
					else
					{
					Log->Write((boost::format("Open file %1% error!") % FileName.c_str()).str().c_str());
					}
				}
				else
				{
					Log->Write((boost::format("Open file %1% error! File not found.") % FileName.c_str()).str().c_str());
				}
			}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
			Log->Write((boost::format("Open file %1% error!") % FileName.c_str()).str().c_str());
			Res = false;
			}
	return Res;
}

//---------------------------------------------------------------------------

AnsiString _email_info::GetRecipientAddress(EMailTypes MsgType)
{
  switch (MsgType)
    {
    case cnETHWErr:
      return ToHWErrAddr.c_str();
    case cnETHWOK:
      return ToHWOKAddr.c_str();
    case cnETValFull:
      return ToValFullAddr.c_str();
    case cnETIncass:
      return ToIncassAddr.c_str();
    case cnETProgMsg:
      return ToProgMsgAddr.c_str();
    case cnETMdmMsg:
      return ToMdmMsgAddr.c_str();
    case cnETFile:
      return ToFileAddr.c_str();
    }
  return "";
}

//---------------------------------------------------------------------------

ConnInfo TWConfig::Connection(int i) {
    if (i!= -1) {
        if (i<(int)RASConnections.size())
            return RASConnections[i];
    }
    else
    {
        if ((ActiveConnection >= 0)&&(ActiveConnection < (int)RASConnections.size()))
            return RASConnections[ActiveConnection];
    }

    return DefaultConnInfo;
}

//---------------------------------------------------------------------------

/*void TWConfig::SetActiveConnection(int i)
{
try
  {
//  ActiveConnection = ((ActiveConnection<RASConnections->Count-1) ? ActiveConnection+1 : -1);
  ActiveConnection = ((ActiveConnection<RASConnections.size()-1) ? i : 0);
  }
catch (Exception &ex)
  {
  Log->Write("Exception in TWConfig::SetActiveConnection: "+ex.Message);
  }
}*/

//---------------------------------------------------------------------------

/*bool TWConfig::isConnectionExists(AnsiString Name)
{
try
  {
  for (int i = 0;i<RASConnections->Count;i++)
    if (Connection(i)->Name == Name)
      return true;
  }
catch (Exception &ex)
  {
  Log->Write("Exception in TWConfig::isConnectionExists: "+ex.Message);
  }
return false;
}

//---------------------------------------------------------------------------

int TWConfig::getConnectionIndex(AnsiString Name)
{
try
  {
  for (int i = 0;i<RASConnections->Count;i++)
    if (Connection(i)->Name == Name)
      return i;
  }
catch (Exception &ex)
  {
  Log->Write("Exception in TWConfig::getConnectionIndex: "+ex.Message);
  }
return false;
}*/

//---------------------------------------------------------------------------
bool TWConfig::RestoreFile(AnsiString FileName)
{
try
  {
  if (FileExists(FileName+".lastgood"))
    {                                                                           // есть резервная копия
    if (FileExists(FileName+".error"))
      if (!DeleteFile(FileName+".error"))
        {
        Log->Write((boost::format("Can not delete file %1%.error!") % FileName.c_str()).str().c_str());                // удаляем FileName.error
        return false;
        }

    if (!RenameFile(FileName+".xml",FileName+".error"))                         // сохраняем FileName.xml как FileName.error
      {
      Log->Write((boost::format("Can not rename file %1%.xml to %1%.error!") % FileName.c_str()).str().c_str());
      if (!DeleteFile(FileName+".xml"))                                         // не удалось сохранить FileName.xml как FileName.error
        {
        Log->Write((boost::format("Can not delete file %1%.xml!") % FileName.c_str()).str().c_str());                 // удаляем FileName.xml
        return false;
        }
      }
      else
      {
      if (!RenameFile(FileName+".lastgood",FileName+".xml"))
        {                                                                       // восстанавливаем FileName.xml из FileName.lastgood
        Log->Write((boost::format("Can not rename file %1%.lastgood to %1%.xml!") % FileName.c_str()).str().c_str());
        return false;
        }
      }
    }
    else
    {
    Log->Write((boost::format("Can not find file %1%.lastgood!") % FileName.c_str()).str().c_str());
    return false;
    }
  Log->Write((boost::format("File %1%.xml restored from %1%.lastgood!") % FileName.c_str()).str().c_str());
  return true;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  return false;
  }
}

//---------------------------------------------------------------------------

void TWConfig::GetConfigTree(/*TVirtualStringTree *TV,*/ TStringList* RuleSL, bool FillRules) {
    try {
        xmlGuard <_di_IXMLDocument> XmlDoc;
        std::auto_ptr <TFileStream> File;

        AnsiString FileName;

        if (FillRules)
            FileName = ".\\config\\config_profile.xml";
        else
            FileName = ConfigFileName;


        if (FileExists(FileName)) {
            AnsiString FileData;
            bool bRes = OpenFile(File.get(), FileName, FileData);


            if (FillRules) {
                FileData = ChangeChars(FileData,"[ns]","");
                FileData = ChangeChars(FileData,"[/ns]","");
            }

            if (bRes) {
                try {
                    XmlDoc = LoadXMLData(FileData);
                }
                catch(...)
                {
                    ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                    FileData = FileData.SubString(0,FileData.Pos("</root>")+6);
                    XmlDoc = LoadXMLData(FileData);
                }

                XmlDoc->Options = XmlDoc->Options << doNodeAutoIndent;

        /*ConfigTree = TV->Items;
        if(ConfigTree)
          {
          if (FillRules)
            ClearConfigTree(ConfigTree);*/



                 /*if (FillRules)//Igor
                     DeleteAllNodes(TV);

                 TV->NodeDataSize = sizeof(VTVNODEDATA);*/

                 _di_IXMLNode RootNode = XmlDoc->GetDocumentElement();

                 if (RootNode->HasChildNodes) {
                     //TVirtualNode *NewNode = TV->AddChild(NULL);
                     //TVirtualNode *NewNode = NULL;//Igor
                     //TTreeNode* NewNode = NULL;
                     //if (!FillRules)
                     //  NewNode = ConfigTree->AddChildObject(NULL, "root", NULL);
                     /*TV->BeginUpdate();//Igor
                     ProcessCfgNode(TV, RootNode, NewNode, false, "root", RuleSL, FillRules);//Igor
                     TV->EndUpdate();*/
                 } else {
                     Log->Write((boost::format("Error getting data from %1%!") % FileName.c_str()).str().c_str());
                 }

            } else {
                Log->Write((boost::format("File %1% does not exists.") % FileName.c_str()).str().c_str());
            }
        } //if
    }
    catch (...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TWConfig::ProcessCfgNode(/*TVirtualStringTree *TV, */_di_IXMLNode ProcessedNode, /*TVirtualNode *ParentNode, */bool ProcessAttributes, AnsiString NodeId, TStringList* RuleSL, bool FillRules)
{
try
	{
        /*//Igor
  _di_IXMLNodeList ProcessedNDL;
  if (ProcessAttributes)
    ProcessedNDL = ProcessedNode->AttributeNodes;
    else
    ProcessedNDL = ProcessedNode->GetChildNodes();
  //Log->Write(ProcessedNode->NodeName+" : "+ (ProcessAttributes ? "A:" : "")+AnsiString(ProcessedNDL->Count));
  for (int i = 0; (i<ProcessedNDL->Count); i++)
    {
    TVirtualNode *NewNode = NULL;
    _di_IXMLNode ItemNode = ProcessedNDL->Get(i);
    AnsiString NewNodeId = NodeId + "|" + (ProcessAttributes ? "." : "")+ItemNode->NodeName;
    //Log->Write(ItemNode->NodeName+" : " + NewNodeId);
    if ((ItemNode->NodeName!=WideString("#comment"))&&(ItemNode->NodeName!=WideString("#text")))
      {
      if ((FillRules)&&(RuleSL))
        {
        AnsiString Rule;
        if (ItemNode->IsTextElement)
          {
          Rule = ItemNode->Text;
          }
          else
          if (ItemNode->HasAttribute("nodeparams"))
            Rule = ItemNode->GetAttribute("nodeparams");

        if (Rule!="")
          {
          if (Rule.Pos("{"))
            {
            Rule = Rule.SubString(Rule.Pos("{")+1,Rule.Length());
            Rule.SetLength(Rule.Length()-1);
            }
          //Log->Write("Saved rule: \""+NewNodeId+"\" : \"" + Rule + "\"");
          RuleSL->Add(NewNodeId+"="+Rule);
          }
        }
        else
        {
        if (!((ProcessAttributes)&&(ItemNode->NodeName==AnsiString("nodeparams"))))
          {

          TCfgFieldData* CfgFieldData = NULL;

          CfgFieldData = new TCfgFieldData(this, Log, ItemNode->NodeName, (ItemNode->IsTextElement ? AnsiString(ItemNode->Text) : AnsiString("")),
             ProcessAttributes, ((!ItemNode->HasChildNodes)|(ItemNode->IsTextElement)), RuleSL->Values[NewNodeId]);
          if (CfgFieldData->Type==ftSArray)
            {
            std::auto_ptr <TStringList> SL ( new TStringList() );
            _di_IXMLNodeList StringsNDL = ItemNode->GetChildNodes();
            for (int i = 0; (i<StringsNDL->Count); i++)
              {
              _di_IXMLNode StringNode = StringsNDL->Get(i);
              if (StringNode->NodeName==CfgFieldData->SArrayName)
                {
                if (StringNode->IsTextElement)
                  SL->Add(StringNode->Text);
                  else
                  SL->Add("");
                }
              }
            CfgFieldData->Store(SL->DelimitedText);
            }
          //Log->Write("Node "+ItemNode->NodeName+" added.");
          NewNode = TV->AddChild(ParentNode);
          VTVNODEDATA *VTVNewNodeData = (VTVNODEDATA *)TV->GetNodeData(NewNode);
          VTVNewNodeData->NodeData = CfgFieldData;
          //NewNode = ConfigTree->AddChildObject(ParentNode, CfgFieldData->Display(), CfgFieldData);
          if (CfgFieldData->Type==ftSArray)
            continue;
          }
        }
      if (!ProcessAttributes)
        {
        ProcessCfgNode(TV, ItemNode,NewNode, true, NewNodeId, RuleSL, FillRules);
        ProcessCfgNode(TV, ItemNode,NewNode, false, NewNodeId, RuleSL, FillRules);
        }
      }
  	}
        */
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

bool TWConfig::SaveCfgFile(/*TVirtualStringTree *TV*/)
{
bool bResult = true;
_di_IXMLDocument XmlDoc;
try
	{
	try
		{
		if (FileExists(ConfigFileName))
			{
			AnsiString FileData;
			bool bRes = OpenFile(CfgFile, ConfigFileName, FileData);
			if (bRes)
				{
				try
					{
					XmlDoc = LoadXMLData(FileData);
					}
                                catch (...)
                                {
                                    ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
					FileData = FileData.SubString(0,FileData.Pos("</root>")+6);
					XmlDoc = LoadXMLData(FileData);
					}				//XmlDoc = LoadXMLData(FileData);
				XmlDoc->Options = XmlDoc->Options << doNodeAutoIndent;

				_di_IXMLNode RootNode = XmlDoc->GetDocumentElement();
        _di_IXMLNodeList RootNDL = RootNode->GetChildNodes();
 					//CleanCfgNode(RootNode);
          while (RootNode->HasChildNodes)
            {
            RootNDL->Delete(0);
            }
             /*
          PVirtualNode TVRootNode = TV->RootNode;//Igor
          PVirtualNode TVNode = TVRootNode->FirstChild;

          for (unsigned i = 0; i<TVRootNode->ChildCount; i++)
            {
            SaveCfgNode(TV, RootNode, TVNode);
            TVNode = TVNode->NextSibling;
            }*/
					//SaveCfgNode(RootNode, NULL);
					//SaveCfgNode(RootNode, ConfigTree->Item[0]);
					Log->Write("Trying to save config.xml...");
					try
						{
						AnsiString Temp;
						XmlDoc->SaveToXML(Temp);
						Log->Write((boost::format("XML: \n%1%") % Temp.c_str()).str().c_str());
						if (!SaveFile(CfgFile, Temp))
							{
							Log->Write("Error writing file to disk!");
							bResult = false;
							}
							else
							Log->Append("OK.");
						}
                                                catch(...)
                                                {
                                                    ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
						Log->Append("Error!");
						bResult = false;
						}
					//}
				}
				else
				{
				Log->Write((boost::format("Error getting data from %1%!") % ConfigFileName.c_str()).str().c_str());
				bResult = false;
				}
			}
			else
			{
			Log->Write((boost::format("File %1% does not exists.") % ConfigFileName.c_str()).str().c_str());
			bResult = false;
			}
		}
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		}
	}
__finally
	{
	if (XmlDoc!= NULL)
		XmlDoc.Release();

	if (OperFile)
		{
		Log->Write("Config file closed.");
		delete OperFile;
		OperFile = NULL;
		}
	return bResult;
	}
}

//---------------------------------------------------------------------------

void TWConfig::SaveCfgNode(/*TVirtualStringTree *TV, */_di_IXMLNode ProcessedNode, /*TVirtualNode* ParentNode, */AnsiString Margin)
{
try
	{
        /*//Igor
  _di_IXMLNode NewNode = NULL;
  VTVNODEDATA *VTVNodeData = (VTVNODEDATA *)TV->GetNodeData(ParentNode);
  if (VTVNodeData->NodeData)
    {
    if (VTVNodeData->NodeData->isAttribute)
      {
      ProcessedNode->SetAttribute(VTVNodeData->NodeData->Name,VTVNodeData->NodeData->Text);
      }
      else
      {
      _di_IXMLNodeList ProcessedNDL = ProcessedNode->GetChildNodes();
      //NewNode = ProcessedNDL->FindNode(CfgParamData->Name);
      if (NewNode == NULL)
        NewNode = ProcessedNode->AddChild(VTVNodeData->NodeData->Name);
      if (VTVNodeData->NodeData->isTextNode)
        NewNode->Text = VTVNodeData->NodeData->Text;
      if (VTVNodeData->NodeData->Type==ftSArray)
        {
        std::auto_ptr <TStringList> SL ( new TStringList() );
        SL->DelimitedText = VTVNodeData->NodeData->SArrayData;
        for (int i = 0; (i<SL->Count); i++)
          {
          _di_IXMLNode NewStringNode = NewNode->AddChild(VTVNodeData->NodeData->SArrayName);
          NewStringNode->Text = SL->Strings[i];
          }
        }
      }
    }
	if ((ParentNode->ChildCount)&&(VTVNodeData->NodeData->Type!=ftSArray))
    {
    PVirtualNode ChildNode = ParentNode->FirstChild;
		for (unsigned int i = 0; (i<ParentNode->ChildCount); i++)
      {
      SaveCfgNode(TV, NewNode, ChildNode);
      ChildNode = ChildNode->NextSibling;
			}
		}*/
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
	}
}

void TWConfig::ClearConfigTree(/*TVirtualStringTree *TV*/)
{
try
  {
  //DeleteAllNodes(TV);//Igor
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
	}
}


bool TWConfig::DeleteAllNodes(/*TVirtualStringTree *TV*/)
{
  bool bResult = true;
  xmlGuard <_di_IXMLDocument> XmlDoc(NULL);
	try
  {    /*//Igor
    PVirtualNode TVRootNode = TV->RootNode;
    if (TVRootNode->ChildCount>0)
    {
      PVirtualNode TVNode = TVRootNode->FirstChild;

      unsigned ChildCount = TVRootNode->ChildCount;
      for (unsigned i=0;i<ChildCount;i++)
      {
        PVirtualNode NodeToDelete = TVNode;
        TVNode = TVNode->NextSibling;
        DeleteCfgNode(TV, NodeToDelete);
      }
    }*/
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
	}
	return bResult;
}

//---------------------------------------------------------------------------

void TWConfig::DeleteCfgNode(/*TVirtualStringTree *TV, TVirtualNode* ParentNode, */AnsiString Margin)
{
try
	{
  //Log->Write("DeleteCfgNode started.");
  /*//Igor
  VTVNODEDATA *VTVNodeData = (VTVNODEDATA *)TV->GetNodeData(ParentNode);
  if (VTVNodeData->NodeData)
    {
    TCfgFieldData* CfgFieldData = VTVNodeData->NodeData;
    //Log->Write("Info deleted. "+Margin+CfgFieldData->Name);
    delete CfgFieldData;
    VTVNodeData->NodeData = NULL;
    }

	if (ParentNode->ChildCount)
    {
    PVirtualNode ChildNode = ParentNode->FirstChild;
    unsigned ChildCount = ParentNode->ChildCount;
		for (unsigned i = 0; i<ChildCount; i++)
      {
      PVirtualNode NodeToDelete = ChildNode;
      ChildNode = ChildNode->NextSibling;
      DeleteCfgNode(TV, NodeToDelete,Margin+" ");
			}
		}
  TV->DeleteNode(ParentNode);
  */
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
	}
}

bool TWConfig::FillCountryCodes(AnsiString FileName)
{
  bool bRes;
  std::auto_ptr <JSONDocument> JSONDoc( new JSONDocument(Log) );
  JSONItem Countries;
  Countries.Name="$ga_countries";
  AnsiString FileContent;
  if ((FileExists(".\\config\\country_codes.txt"))&&(GetFileData(FileName, FileContent, Log)))
  {
    long CurrStrBegin=0;
    long CurrStrEnd=0;
    while ((CurrStrEnd<FileContent.Length()))
    {
      CurrStrEnd = CurrStrBegin;
      while ((CurrStrEnd < FileContent.Length()-1)&&(FileContent.c_str()[CurrStrEnd]!='\r')&&(FileContent.c_str()[CurrStrEnd+1]!='\n'))
        CurrStrEnd++;
      AnsiString Temp = AnsiString(FileContent.c_str()+CurrStrBegin,CurrStrEnd-CurrStrBegin);
      CurrStrBegin = CurrStrEnd+2;

      //Log->Write("Processing "+Temp+"...");
      if (Temp.Length() > 4)
        {
        CountryCodes[Temp.SubString(0,3).c_str()] = Temp.SubString(4,Temp.Length()).c_str();
        Countries.ChildItems.push_back(JSONItem(Temp.SubString(0,3),Temp.SubString(4,Temp.Length()).Trim()));
        }
    }
    bRes = true;
  }
  else
  {
    Log->Write("Error getting ISO3166 data.");
    JSONItem NewCountry;
    NewCountry.Name = "643";
    NewCountry.Value = "Россия";
    Countries.ChildItems.push_back(NewCountry);
    bRes = false;
  }
  JSONDoc->RootItem.ChildItems.push_back(Countries);
  JSONDoc->RootItem.ChildItems.push_back(JSONItem(" - - - - - всегда в true, для динамического include файла","",true));
  JSONDoc->RootItem.ChildItems.push_back(JSONItem("$ga_countries_js","true"));
  AnsiString Res = JSONDoc->GetJSONString();
  //Log->Write(Res);
  std::string strtmp=Dirs.InterfaceDir;
  boost::algorithm::replace_all(strtmp,"/","\\");
  StoreStringToFile((strtmp+"\\ga_mt_iso3166.js").c_str(), Res, Log);
  return bRes;
}

AnsiString TWConfig::GetCountryName(AnsiString CountryCode)
{
  if (CountryCodes.find(CountryCode.c_str()) != CountryCodes.end())
    return (CountryCodes[CountryCode.c_str()]).c_str();
  else
    return ("#"+CountryCode).c_str();
}

bool TWConfig::operatorExist(int opID)
{
    return (m_allOperators.find(opID) != m_allOperators.end()) && (m_allOperatorsInMenu.find(opID) != m_allOperatorsInMenu.end());
}

void comm_info::swap(const comm_info& rhs)
{
    this->Relative=rhs.Relative;
    this->Value=rhs.Value;
    this->Min=rhs.Min;
    this->MinTime=rhs.MinTime;
    this->MinDay=rhs.MinDay;
}

comm_info::comm_info()
{
    Relative = false;
    Value = 0;
    Min = 0;
    MinTime = 0;
    MinDay = 0;
}

comm_info::comm_info(bool _Relative, double _Value, int _Min, int _MinTime, int _MinDay)
{
    Relative = _Relative;
    Value = _Value;
    Min = _Min;
    MinTime = _MinTime;
    MinDay = _MinDay;
}

comm_info::comm_info(const comm_info& ob)
{
    swap(ob);
}

comm_info& comm_info::operator = (const comm_info& ob)
{
    comm_info tmp(ob);
    swap( tmp );
    return *this;
}

AnsiString comm_info::GetString()
{
    return AnsiString(Value)+(Relative ? "%" : "")+" : "+AnsiString(Min)+" "+AnsiString(MinTime)+" "+AnsiString(MinDay);
}

void field::swap(const field& rhs)
{
    this->Id = rhs.Id;
    this->Name = rhs.Name;
    this->Mask = rhs.Mask;
    this->SendUnmasked = rhs.SendUnmasked;
    this->Type = rhs.Type;
    this->Enum = rhs.Enum;
    this->Regexp = rhs.Regexp;
    this->Format = rhs.Format;
    this->URL1 = rhs.URL1;
}

field::field()
{
    SendUnmasked = false;
}

field::field(const char* _Id, const char* _Name, const char* _Mask, bool _SendUnmasked, const char* _Type, const std::map <std::string, std::string>& _Enum,const char* regexp,const char* format, const char* tURL1)
{
    Id = _Id;
    Name = _Name;
    Mask = _Mask;
    SendUnmasked = _SendUnmasked;
    Type = _Type;
    Enum = _Enum;
    Regexp = regexp;
    Format = format;
    URL1 = tURL1;
}

field::field(const field& ob)
{
    swap(ob);
}

field& field::operator = (const field& ob)
{
    field tmp(ob);
    swap( tmp );
    return *this;
}

AnsiString field::GetString()
{
    return (AnsiString)("Id: " + Id + ", Name: " + Name + ", Mask: " + Mask + ", SendUnmasked: ").c_str() + AnsiString(int(SendUnmasked)) + ", Type: " + (Type + + ", Enum: ").c_str() + AnsiString(Enum.size()) + ".";
}

AnsiString field::GetEnumText(const char* Value)
{
    if (HasEnumValue(Value))
        return (Enum[Value]).c_str();
    else
        return "";
}

bool field::HasEnumValue(std::string Value)
{
    return Enum.find(Value) != Enum.end();
}

property::property()
{
}

property::property(const char* _Name, const char* _FieldId)
{
    Name = _Name;
    FieldId = _FieldId;
}

property::property(const property& ob)
{
    Name=ob.Name;
    FieldId=ob.FieldId;
}

property& property::operator = (const property& ob)
{
    Name=ob.Name;
    FieldId=ob.FieldId;
    return *this;
}

bool property::operator == (const property& ob)
{
    return (Name == ob.Name) && (FieldId == ob.FieldId);
}

bool property::operator != (const property& ob)
{
    return !operator==(ob);
}

AnsiString property::GetString()
{
    return ("Name: " + Name + ", FieldId: " + FieldId + ".").c_str();
}

_money_transfers_info::_money_transfers_info()
{
    randomize();
    MTInfoUpdateTime.Val = 0.25 + double((rand() % 200)-100)/2400;
}
_money_transfers_info::_money_transfers_info(const _money_transfers_info& ob)
{
    MTInfoUpdateTime=ob.MTInfoUpdateTime;
}

_money_transfers_info& _money_transfers_info::operator = (const _money_transfers_info& ob)
{
    MTInfoUpdateTime=ob.MTInfoUpdateTime;
    return *this;
}

AnsiString _money_transfers_info::GetString()
{
    return (AnsiString)"MTInfoUpdateTime: " + AnsiString(MTInfoUpdateTime) + ".";
}

receive_property::receive_property()
{
    Encrypted = false;
}
receive_property::receive_property(const char* _Name, const char* _Description, bool _Encrypted)
{
    Name = _Name;
    Description = _Description;
    Encrypted = _Encrypted;
}

receive_property::receive_property(const receive_property& ob)
{
    Name=ob.Name;
    Description=ob.Description;
    Encrypted=ob.Encrypted;
}

receive_property& receive_property::operator = (const receive_property& ob)
{
    Name=ob.Name;
    Description=ob.Description;
    Encrypted=ob.Encrypted;
    return *this;
}

void OpInfo::swap(const OpInfo& rhs)
{
    Id                  = rhs.Id;
    GroupId             = rhs.GroupId;
    SalonField          = rhs.SalonField;
    Name                = rhs.Name;
    Image               = rhs.Image;
    RootMenuImage       = rhs.RootMenuImage;
    CheckURL            = rhs.CheckURL;
    CheckAmount         = rhs.CheckAmount;
    CheckAmountFieldId  = rhs.CheckAmountFieldId;
    PaymentURL          = rhs.PaymentURL;
    StatusURL           = rhs.StatusURL;
    GetCardsURL         = rhs.GetCardsURL;
    LoginURL            = rhs.LoginURL;
    Offline             = rhs.Offline;
    LimMin              = rhs.LimMin;
    LimMax              = rhs.LimMax;
    fix                 = rhs.fix;
    MNVO_ID             = rhs.MNVO_ID;
    LimSetCount         = rhs.LimSetCount;
    ProcessorType       = rhs.ProcessorType;
    ACardsInfo          = rhs.ACardsInfo;
    DTCardsInfo         = rhs.DTCardsInfo;
    KeysId              = rhs.KeysId;
    RoundAmount         = rhs.RoundAmount;
    PrinterOkOnly       = rhs.PrinterOkOnly;
    GetCardsAllowed     = rhs.GetCardsAllowed;
    MaxSavedCardsCount  = rhs.MaxSavedCardsCount;
    SignatureType       = rhs.SignatureType;
    NameForCheque       = rhs.NameForCheque;
    INNForCheque        = rhs.INNForCheque;
    CurrencyId          = rhs.CurrencyId;
    MarketingOperatorId = rhs.MarketingOperatorId;
    CommissionInfo      = rhs.CommissionInfo;
    Fields              = rhs.Fields;
    Properties          = rhs.Properties;
    ReceiveProperties   = rhs.ReceiveProperties;
    ShowAddInfo         = rhs.ShowAddInfo;
    ServiceGuid         = rhs.ServiceGuid;
    showOnLineComment   = rhs.showOnLineComment;
    ChequeFileName      = rhs.ChequeFileName;
    ErrorChequeFileName = rhs.ErrorChequeFileName;
    ErrorMoneyChequeFileName = rhs.ErrorMoneyChequeFileName;
    PaymentBeforeChequePrinting = rhs.PaymentBeforeChequePrinting;
}

OpInfo::OpInfo(const OpInfo& rhs)
{
    swap( rhs );
}

OpInfo& OpInfo::operator=(const OpInfo& rhs)
{
    OpInfo tmp(rhs);
    swap( tmp );
    return *this;
}

bool OpInfo::HasReceiveProperty(const char* PropName)
{
    return ReceiveProperties.find(PropName) != ReceiveProperties.end();
}

field OpInfo::getFieldById(std::string id)
{
  for(int i = 0; i < Fields.size(); i++)
    if (Fields[i].Id == id)
      return Fields[i];
  return field();
}

void MenuItemData::swap(const MenuItemData& rhs)
{
    this->Type=rhs.Type;
    this->Id=rhs.Id;
    this->Name=rhs.Name;
    this->Image=rhs.Image;
    this->TitleImage=rhs.TitleImage;
    this->Columns=rhs.Columns;
    this->Rows=rhs.Rows;
}

MenuItemData::MenuItemData(){}
MenuItemData::MenuItemData(const MenuItemData& rhs)
{
    swap( rhs );
}

MenuItemData& MenuItemData::operator=(const MenuItemData& rhs)
{
    MenuItemData tmp(rhs);
    swap( tmp );
    return *this;
}

void _dealer_info::swap(const _dealer_info& rhs)
{
    this->DealerName=rhs.DealerName;
    this->DealerAddress=rhs.DealerAddress;
    this->DealerINN=rhs.DealerINN;
    this->DealerPhone=rhs.DealerPhone;
    this->PointAddress=rhs.PointAddress;
    this->ContractNumber=rhs.ContractNumber;
    this->BankName=rhs.BankName;
    this->BankBIK=rhs.BankBIK;
    this->BankPhone=rhs.BankPhone;
    this->BankAddress=rhs.BankAddress;
    this->BankINN=rhs.BankINN;
    this->BusinessDealerAddress=rhs.BusinessDealerAddress;
}

_dealer_info::_dealer_info(const _dealer_info& rhs)
{
    swap( rhs );
}

_dealer_info& _dealer_info::operator=(const _dealer_info& rhs)
{
    _dealer_info tmp(rhs);
    swap( tmp );
    return *this;
}

_dealer_info::_dealer_info()
{
    DealerName = "-";
    DealerAddress = "-";
    DealerINN = "-";
    DealerPhone = "-";
    PointAddress = "-";
    ContractNumber = "-";
    BankName = "-";
    BankBIK = "-";
    BankPhone = "-";
    BankAddress = "-";
    BankINN = "-";
}

_proxy_info::_proxy_info()
{
    Port = 0;
}

_proxy_info::_proxy_info(const _proxy_info& ob)
{
    CfgType=ob.CfgType;
    Type=ob.Type;
    Host=ob.Host;
    Port=ob.Port;
    UserName=ob.UserName;
    Password=ob.Password;
}

_proxy_info& _proxy_info::operator = (const _proxy_info& ob)
{
    CfgType=ob.CfgType;
    Type=ob.Type;
    Host=ob.Host;
    Port=ob.Port;
    UserName=ob.UserName;
    Password=ob.Password;
    return *this;
}

void ConnInfo::swap(const ConnInfo& ob)
{
    this->Name=ob.Name;
    this->DisconnectTime=ob.DisconnectTime;
    this->InitCmd=ob.InitCmd;
    this->InitString[0]=ob.InitString[0];
    this->InitString[1]=ob.InitString[1];
    this->Login=ob.Login;
    this->Name=ob.Name;
    this->Password=ob.Password;
    this->HTTPProxy=ob.HTTPProxy;
    this->Proxy->Host = ob.Proxy->Host.c_str();
    this->Proxy->Port = ob.Proxy->Port;
    this->Proxy->UserID = ob.Proxy->UserID.c_str();
    this->Proxy->Password = ob.Proxy->Password.c_str();
    this->Proxy->Version = ob.Proxy->Version;
}

ConnInfo::ConnInfo()
{
    DisconnectTime = 0;
    Proxy = NULL;
    Proxy = new TSocksInfo();
    Proxy->Port = 0;
    Proxy->Version = svNoSocks;
    DisconnectTime = 0;
    HTTPProxy.CfgType = "none";
    HTTPProxy.Type = "none";
    HTTPProxy.Port = 0;
}

ConnInfo::ConnInfo(const ConnInfo& ob)
{
    this->Proxy=new TSocksInfo;
    swap(ob);
}

ConnInfo& ConnInfo::operator = (const ConnInfo& ob)
{
    ConnInfo tmp(ob);
    swap( tmp );
    return *this;
}

ConnInfo::~ConnInfo()
{
}

void _dirs_info::swap(const _dirs_info& rhs)
{
    this->WorkDir=rhs.WorkDir;

    this->PaymentsOutbound=rhs.PaymentsOutbound;
    this->PaymentsOutboundTemp=rhs.PaymentsOutboundTemp;
    this->PaymentsUnprocessed=rhs.PaymentsUnprocessed;
    this->PaymentsBad=rhs.PaymentsBad;

    this->StatOutbound=rhs.StatOutbound;
    this->StatOutboundTemp=rhs.StatOutboundTemp;
    this->StatOutboundBad=rhs.StatOutboundBad;

    this->EMailOutbound=rhs.EMailOutbound;
    this->EMailOutboundExt=rhs.EMailOutboundExt;
    this->EMailOutboundTemp=rhs.EMailOutboundTemp;
    this->EMailOutboundExtTemp=rhs.EMailOutboundExtTemp;

    this->SMSOutbound=rhs.SMSOutbound;
    this->SMSOutboundTemp=rhs.SMSOutboundTemp;
    
    this->CommandsInbound=rhs.CommandsInbound;
    this->StatusFileName=rhs.StatusFileName;
    this->InterfaceDir=rhs.InterfaceDir;
    this->InterfaceSkinName=rhs.InterfaceSkinName;
    this->DBNumCapacityUpdateAddress=rhs.DBNumCapacityUpdateAddress;
    this->MTRoot=rhs.MTRoot;
}

_dirs_info::_dirs_info()
{
}

_dirs_info::_dirs_info(const _dirs_info& rhs)
{
    swap( rhs );
}

_dirs_info& _dirs_info::operator=(const _dirs_info& rhs)
{
    _dirs_info tmp(rhs);
    swap( tmp );
    return *this;
}

void _email_info::swap(const _email_info& rhs)
{
    this->SMailHost=rhs.SMailHost;
    this->FromAddress=rhs.FromAddress;
    this->UserId=rhs.UserId;
    this->Password=rhs.Password;
    this->Ext=rhs.Ext;
    this->ToHWErrAddr=rhs.ToHWErrAddr;
    this->ToHWOKAddr=rhs.ToHWOKAddr;
    this->ToValFullAddr=rhs.ToValFullAddr;
    this->ToIncassAddr=rhs.ToIncassAddr;
    this->ToProgMsgAddr=rhs.ToProgMsgAddr;
    this->ToMdmMsgAddr=rhs.ToMdmMsgAddr;
    this->ToFileAddr=rhs.ToFileAddr;
    this->IgnoredErrorsList=rhs.IgnoredErrorsList;
    this->SendInterval=rhs.SendInterval;
}

_email_info::_email_info()
{
    SMailHost = "none";
    SendInterval = 5;
    Ext = false;
    IgnoredErrorsList = "450";
}

_email_info::_email_info(const _email_info& rhs)
{
    swap( rhs );
}

_email_info& _email_info::operator=(const _email_info& rhs)
{
    _email_info tmp(rhs);
    swap( tmp );
    return *this;
}

void _sms_info::swap(const _sms_info& rhs)
{
    this->Interval=rhs.Interval;
    this->PhoneNumber=rhs.PhoneNumber;
    this->Comment=rhs.Comment;
    this->SendStartUpSMS=rhs.SendStartUpSMS;
}

_sms_info::_sms_info()
{
    Interval = 17;
    SendStartUpSMS = true;
}

_sms_info::_sms_info(const _sms_info& rhs)
{
    swap( rhs );
}

_sms_info& _sms_info::operator=(const _sms_info& rhs)
{
    _sms_info tmp(rhs);
    swap( tmp );
    return *this;
}

_terminal_info::_terminal_info()
{
    NightModePeriodBegin = 0;
    NightModePeriodEnd = 0;
    ShowCursor = true;
    WriteInCheque = false;
    SupportString = "Tелефон техподдержки: [b]***-****[/b][br]e-mail: [b]support@****.ru[/b]";
    MainMenuMarqueeString = "За проведение платежа взимается комиссия.";
    RebootAllowed = true;
    DetectWriteErrors = true;
    SetWebclientHighPriority = true;
    ShowPB = false;
    //StayOnTop = true;
    ChequeCounter = 0;
    InterfaceSoundVolume = -1;
    NoChangeMessage = false;
    NoteMinAmount = 10;
}

_cdebug::_cdebug()
{
    PeripheralsState = 0;
    explorerProcess = false;
    connProcess = true;
    sound = true;
    Logs = _debug_log();
}

_debug_log::_debug_log()
{
    url = true;
    full = false;
    sound = true;
}

void _debug_log::operator=(const bool value)
{
    this->url = value;
    this->sound = value;
}

_keys_info::_keys_info(int _Id, int _Hasp, const char* _SD, const char* _AP, const char* _OP, const char* _PubKeyPath, int _PubKeySerial, const char* _SecKeyPath, const char* _SecKeyPassword)
{
    Id = _Id;
    Hasp = _Hasp;
    SD = _SD;
    AP = _AP;
    OP = _OP;
    PubKeyPath = _PubKeyPath;
    PubKeySerial = _PubKeySerial;
    SecKeyPath = _SecKeyPath;
    SecKeyPassword = _SecKeyPassword;
}

_keys_info::_keys_info()
{
    Id = 0;
    Hasp = 0;
    PubKeySerial = 0;
}

_keys_info::_keys_info(int _Id)
{
    Id = _Id;
    Hasp = 0;
    PubKeySerial = 0;
}

_logs_delete_info::_logs_delete_info()
{
    Payments = 0;
    Main = 0;
    Other = 0;
    UnprocessedPayments = 30;
}

_printer_info::_printer_info()
{
    Type = "Windows";
    Port = 0;
    PortBaudRate = 0;
    AutoOpenShift = false;
    SaveUnprintedCheques = false;
    PrintUnprintedCheques = false;
    FreeseOnError = false;
    ShowMessageOnError = true;
    IncassBarCode = false;
    MinLinesCount = 0;
    Font = "";
    PresenterCommand = 0;
    ZReportWithIncassation = 1;
    TapeLength=35;
}

void _payments_info::SetRestState(restState tempRestState)
{
  if (tempRestState <= eYes)
    if (tempRestState >= eNo)
      restOperator = tempRestState;
    else
      restOperator = eNo;
  else
    restOperator = eYes;
}

std::string TWConfig::strToIntVector(std::string source, TIntVector& resultVector, std::string parameter, bool isRepeatAllowed)
{
    try
    {
        boost::replace_all(source, "- ", "-");
        boost::replace_all(source, " ", ",");

        typedef std::vector<std::string> split_vector_type;
        split_vector_type sourceVector;
        boost::split(sourceVector, source, boost::is_any_of(","));
        std::string result = "";
        resultVector.clear();
        if (sourceVector.size())
        {
          for(int i = 0; i < sourceVector.size(); i++)
          {
            try
            {
              if (!sourceVector[i].empty())
              {
                std::string q = sourceVector[i];
                int currentParameter = boost::lexical_cast<int>(sourceVector[i]);
                int vSize = resultVector.size();
                bool isParameterExist = false;
                if (resultVector.size())
                {
                    for(int j = 0; j < resultVector.size() && !isParameterExist; j++)
                    {
                        if (resultVector[j] == currentParameter)
                        {
                            isParameterExist = true;
                        }
                    }
                }
                if (!isParameterExist || isRepeatAllowed)
                    resultVector.push_back(currentParameter);
              }
            }
            catch(...)
            {
              Log->Write((boost::format("Bad parameter in %1%: {%2%}")
                % (parameter.empty() ? "unknown parameter" : parameter.c_str())
                % sourceVector[i]).str().c_str());
              ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            }
          }
          sort(resultVector.begin(), resultVector.end()); 
          for(int i = 0; i < resultVector.size(); i++)
            result += boost::lexical_cast<std::string>(resultVector[i]) + ", ";
          boost::trim_right_if(result, boost::is_any_of(", "));
        }
        else
          Log->Write((boost::format("%1% string is EMPTY!!")
            % (parameter.empty() ? "unknown parameter" : parameter.c_str())).str().c_str());
        return result;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//------------------------------------------------------------------------------

_bar_code_template::_bar_code_template()
{
    terminal = 6;
    validator = 6;
    cheque = 6;
    total = 6;
    date = "YYYYMMDD";
    time = "HHMMSS";
    nominal = 3;
    count = 4;
    filler = "0";
}

//------------------------------------------------------------------------------

bool TWConfig::processBarCodeTemplateFile(const char* FileName)
{
    try
    {
        std::string result = "";
        if (!FileExists(FileName))
        {
            Log->Write((boost::format("Can not find file: %1%") % FileName).str().c_str());
            return false;
        }

        xmlGuard <_di_IXMLDocument> XmlDoc(NULL);
        AnsiString FileData;
        TFileStream* tstream = NULL;
        bool bRes = OpenFile(tstream, FileName, FileData);

        if(!bRes)
        {
            Log->Write((boost::format("Can not open file: %1%") % FileName).str().c_str());
            return false;
        }

        try
        {
            XmlDoc = LoadXMLData(FileData);
        }
        catch (EDOMParseError &e)
        {
            Log->Write((boost::format("File: %1% EDOMParseError exception encountered: %2%") % FileName % e.Message.c_str()).str().c_str());
            return false;
        }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            return false;
        }

        xmlGuard <_di_IXMLNode> RootNode (XmlDoc->GetDocumentElement());
        std::string _filler = GetChildNodeValue(false, "filler", RootNode, "");
        if (_filler.length() > 1)
            Log->Write((boost::format("Bad filler for barCode data: {%1%}") % _filler).str().c_str());
        else
        {
            Log->Write((boost::format("Filler for barCode data: {%1%}") % _filler).str().c_str());
            BarCodeTemplate.filler = _filler;
        }

        if (RootNode.Assigned())
        {
            xmlGuard <_di_IXMLNodeList> xmlNodeList (RootNode->GetChildNodes());
            for(int i = 0; i < xmlNodeList->Count; i++)
            {
                xmlGuard <_di_IXMLNode> xmlNode (xmlNodeList->Get(i));
                std::string node_name = WCharToString(xmlNode->NodeName);

                if(node_name == "terminal")
                {
                    BarCodeTemplate.terminal = GetNodeValue(xmlNode, 0);
                    BarCodeTemplate.templateStr += node_name;
                }
                if(node_name == "validator")
                {
                    BarCodeTemplate.validator = GetNodeValue(xmlNode, 0);
                    BarCodeTemplate.templateStr += node_name;
                }
                if(node_name == "cheque")
                {
                    BarCodeTemplate.cheque = GetNodeValue(xmlNode, 0);
                    BarCodeTemplate.templateStr += node_name;
                }
                if(node_name == "total")
                {
                    BarCodeTemplate.total = GetNodeValue(xmlNode, 0);
                    BarCodeTemplate.templateStr += node_name;
                }
                if(node_name == "period")
                {
                    xmlGuard <_di_IXMLNodeList> xmlPeriodNodeList (xmlNode->GetAttributeNodes());
                    for(int j = 0; j < xmlPeriodNodeList->Count; j++)
                    {
                        xmlGuard <_di_IXMLNode> AttrNode (xmlPeriodNodeList->Get(j));
                        
                        if(AttrNode->NodeName == WideString("date"))
                            BarCodeTemplate.date = GetStringAttribute(false, "date", xmlNode, "", true);
                        if(AttrNode->NodeName == WideString("time"))
                            BarCodeTemplate.time = GetStringAttribute(false, "time", xmlNode, "", time);

                        BarCodeTemplate.templateStr += WCharToString(AttrNode->NodeName);
                    }
                }
                if(xmlNode->NodeName == WideString("notes"))
                {
                    xmlGuard <_di_IXMLNodeList> xmlPeriodNodeList (xmlNode->GetAttributeNodes());
                    for(int j = 0; j < xmlPeriodNodeList->Count; j++)
                    {
                        xmlGuard <_di_IXMLNode> AttrNode (xmlPeriodNodeList->Get(j));

                        if(AttrNode->NodeName == WideString("nominal"))
                            BarCodeTemplate.nominal = GetIntAttribute(false, "nominal", xmlNode, 0, true);
                        if(AttrNode->NodeName == WideString("count"))
                            BarCodeTemplate.count = GetIntAttribute(false, "count", xmlNode, 0, time);

                        BarCodeTemplate.templateStr += WCharToString(AttrNode->NodeName);
                    }
                }
            }
            if(BarCodeTemplate.templateStr.empty())
                return false;
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        return false;
    }
}
