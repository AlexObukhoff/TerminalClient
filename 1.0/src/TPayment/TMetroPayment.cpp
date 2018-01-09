//---------------------------------------------------------------------------


#pragma hdrstop

#include "TMetroPayment.h"
#include "globals.h"
#include "boost/format.hpp"

//---------------------------------------------------------------------------

#pragma package(smart_init)

//---------------------------------------------------------------------------

TMetroPayment::TMetroPayment(TWConfig *_Cfg, TLogClass *_Log, TFileMap* _FileMap, TXMLInfo* _InfoFile, CCardReader* _CardReader)
: TPinPayment("", _Cfg, _Log, _FileMap, _InfoFile)
{
  CardReader = NULL;
  CardReader = _CardReader;
  MetroMenuFile = NULL;
  Sum=0;
  MenuItem=-1;
/*  if (CardReader)
    {
    int res = CardReader->FindCard();
    Log->Write("Card Num: "+AnsiString(CardReader->FindCardInfo->psCardNum));
    Log->Write("Status: "+AnsiString(CardReader->FindCardInfo->Status));
    }*/
}

//---------------------------------------------------------------------------

TMetroPayment::~TMetroPayment()
{
try
	{
  if (MetroMenuFile)
    {
      delete MetroMenuFile;
      MetroMenuFile = NULL;
    }
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
	}
}

//---------------------------------------------------------------------------

void TMetroPayment::ParseLocation(AnsiString LocationString)
{
	try
	{

    std::auto_ptr <TLocationParser> Location ( new TLocationParser(LocationString.c_str()));

    if (Location->HasParameter("recepient"))
  		Recepient=GetInt(Location->GetParameter("recepient").c_str());
    else
    {
 			Log->Write("Error: \"recepient\" parameter missing in location string!");
			return;
    }

  	for (std::size_t i=0; i<Cfg->Operator(Recepient).Properties.size(); i++)
    {
      AnsiString Mask=Cfg->Operator(Recepient).Properties[i].FieldId.c_str(); //Cfg->GetPrFieldID(Recepient,i);
      for (std::size_t j=0; j<Cfg->Operator(Recepient).Fields.size();j++)
      {
			  AnsiString FieldId=Cfg->Operator(Recepient).Fields[j].Id.c_str();
        Mask = ChangeChars(Mask,"[#"+FieldId+"]",Location->GetParameter(("field"+FieldId).c_str()).c_str());
      }
      XMLP->AddParam(Cfg->Operator(Recepient).Properties[i].Name.c_str() ,Mask);
    }

    MenuItem = -1;
    MenuItemName = "";
    ForcedSum = 0;
    CardValue = 0;
    AnsiString AMenuItem = XMLP->GetParamValue("MENUITEM");
    if (AMenuItem!="")
      {
      MenuItem=GetLong(AMenuItem);
      for (int i=0;i<CardReader->GetMenuInfo->pnItemsNum;i++)
        {
        if (CardReader->GetMenuInfo->pAMenu[i].nID==MenuItem)
          {
          MenuItemName = AnsiString(CardReader->GetMenuInfo->pAMenu[i].sText);
          ForcedSum = double(CardReader->GetMenuInfo->pAMenu[i].nPriceInRub);
          CardValue = MinSum = ForcedSum;
          break;
          }
        }
      }
      else
      {
      Log->Write("Parameter MENUITEM not found!");
      }
    Log->Write((boost::format("MenuItemName: %1%") % MenuItemName.c_str()).str().c_str());
    Log->Write((boost::format("Price: %1%") % ForcedSum).str().c_str());
    AFieldsForInterface=Location->GetParameters().c_str();
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
}

//---------------------------------------------------------------------------

AnsiString TMetroPayment::GetMenuContent()
{
//bool bRes=false;
AnsiString ToFile;
try
  {
  if (CardReader)
    {
    CardReader->GetMenu();
    switch (CardReader->GetMenuInfo->Result)
      {
      case AGM_Menu_Completed:
        OrderId = CardReader->GetMenuInfo->pnOrderId;
/*        Log->Write("GetMenu OK.");
        Log->Write("pnOrderId:    " + AnsiString(CardReader->GetMenuInfo->pnOrderId));
        Log->Write("psCardNum:    " + AnsiString(CardReader->GetMenuInfo->psCardNum));
        Log->Write("psCardStatus: " + AnsiString(CardReader->GetMenuInfo->psCardStatus));
        Log->Write("pnItemsNum:   " + AnsiString(CardReader->GetMenuInfo->pnItemsNum));
        for (int i=0;i<CardReader->GetMenuInfo->pnItemsNum;i++)
          {
          Log->Write("Menu["+AnsiString(i)+"]: " + AnsiString(CardReader->GetMenuInfo->pAMenu[i].nID)+" | " + AnsiString(CardReader->GetMenuInfo->pAMenu[i].nParentID)+" | " + AnsiString(CardReader->GetMenuInfo->pAMenu[i].sText)+" | " + AnsiString(CardReader->GetMenuInfo->pAMenu[i].nPriceInRub)+" | " + AnsiString(CardReader->GetMenuInfo->pAMenu[i].nHasChildren)+".");
          }*/
        ToFile = "var $ga_metro_menu = {\n";
        ToFile+="     CardNum: "+AnsiString(CardReader->GetMenuInfo->psCardNum)+",\n";
        ToFile+="     Message: '"+AnsiString(CardReader->GetMenuInfo->psCardStatus)+"',\n";
        ToFile+="     ItemsNum: "+AnsiString(CardReader->GetMenuInfo->pnItemsNum)+",\n";
        ToFile+="     Menu: [";
        for (int i=0;i<CardReader->GetMenuInfo->pnItemsNum;i++)
          {
          ToFile+="["+AnsiString(CardReader->GetMenuInfo->pAMenu[i].nID)+", " + AnsiString(CardReader->GetMenuInfo->pAMenu[i].nParentID)+", '" + AnsiString(CardReader->GetMenuInfo->pAMenu[i].sText)+"', " + AnsiString(CardReader->GetMenuInfo->pAMenu[i].nPriceInRub)+", " + AnsiString(CardReader->GetMenuInfo->pAMenu[i].nHasChildren)+"]";
          if (i<CardReader->GetMenuInfo->pnItemsNum-1)
          ToFile+=",";
          }
        ToFile+="]\n";
        ToFile+="}\n\n\n//  - - - - - всегда в true, для динамического include файла\n$inc_metro_menu_js = true;";
        break;
      case AGM_No_BSK_Connect:
        Log->Write("GetMenu Error! Can't connect to BSK!");
        ToFile = "var $ga_metro_menu = {\n";
        ToFile+="     CardNum: 0,\n";
        ToFile+="     Message: 'Ошибка - нет связи с УЧЗ БСК.',\n";
        ToFile+="     ItemsNum: 0,\n";
        ToFile+="     Menu: []\n";
        ToFile+="}\n\n\n//  - - - - - всегда в true, для динамического include файла\n$inc_metro_menu_js = true;";
        break;
      case AGM_No_Server_Connect:
        Log->Write("GetMenu Error! Can't connect to server!");
        ToFile = "var $ga_metro_menu = {\n";
        ToFile+="     CardNum: 0,\n";
        ToFile+="     Message: 'Ошибка - нет связи с сервером.',\n";
        ToFile+="     ItemsNum: 0,\n";
        ToFile+="     Menu: []\n";
        ToFile+="}\n\n\n//  - - - - - всегда в true, для динамического include файла\n$inc_metro_menu_js = true;";
        break;
      case AGM_Card_DontRead:
        Log->Write("GetMenu Error! Error reading card info!");
        ToFile = "var $ga_metro_menu = {\n";
        ToFile+="     CardNum: 0,\n";
        ToFile+="     Message: 'Ошибка - невозможно прочитать информацию с карты!',\n";
        ToFile+="     ItemsNum: 0,\n";
        ToFile+="     Menu: []\n";
        ToFile+="}\n\n\n//  - - - - - всегда в true, для динамического include файла\n$inc_metro_menu_js = true;";
        break;
      }
    }
    else
    {
    Log->Write("Error! CardReader not initialized!");
    ToFile = "var $ga_metro_menu = {\n";
    ToFile+="     CardNum: 0,\n";
    ToFile+="     Message: 'Ошибка - устройство недоступно!',\n";
    ToFile+="     ItemsNum: 0,\n";
    ToFile+="     Menu: []\n";
    ToFile+="}\n\n\n//  - - - - - всегда в true, для динамического include файла\n$inc_metro_menu_js = true;";
    }
/*  bRes = StoreFile(".\\interface\\metro_menu.js",ToFile);
  if (!bRes)
    Log->Write("Error getting menu!");*/
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
return ToFile;
}

//---------------------------------------------------------------------------
/*bool TMetroPayment::StoreFile(AnsiString FileName, AnsiString Content)
{
return StoreStringToFile(FileName, Content, Log);
bool bRes = false;
try
  {
  try
    {
    //Log->Write(Content);
    if (MetroMenuFile)
      {
      delete MetroMenuFile;
      MetroMenuFile = NULL;
      }
    DeleteFile(FileName);
    //MetroMenuFile = new TFileStream(".\\interface\\details.js");
    MetroMenuFile = new TFileStream(FileName, fmCreate);
    if (MetroMenuFile)
      {
      delete MetroMenuFile;
      MetroMenuFile = NULL;
      }
    MetroMenuFile = new TFileStream(FileName, fmOpenReadWrite | fmShareDenyNone);
    MetroMenuFile->Size = 0;
    MetroMenuFile->Seek(0, soFromBeginning);
    MetroMenuFile->Write(Content.c_str(), Content.Length());
    }
  __finally
    {
    if (MetroMenuFile)
      {
      delete MetroMenuFile;
      MetroMenuFile = NULL;
      }
    bRes=true;
    }
  }
catch (Exception &ex)
  {
  Log->Write("Exception occured while saving "+FileName+" file: "+ex.Message);
  }
return bRes;
}*/

//---------------------------------------------------------------------------

bool TMetroPayment::PreInit(int OperatorId, AnsiString Login, AnsiString Password)
{
bool bRes = false;
//int iGetMenuRes;
AnsiString ToFile;
try
	{
	try
		{
		Log->Write("New metro payment pre-init started.");
		MinSum = 0;
	  Sum=Cfg->Payments.Rest;
    Cfg->Payments.Rest = 0;
		XMLP->OperatorId = OperatorId;
		XMLP->ProcessorType = Cfg->Operator(OperatorId).ProcessorType.c_str();
    if (CardReader)
      {
      /*if (CardReader->DeviceState->StateCode != DSE_OK)
        {
        Log->Write("Card reader is in error state!");
        return false;
        }*/
      CardNumber = "";
      CardReader->FindCard();
      int TimeOut = 600;

      while ((TimeOut)&&(!CardReader->FindCardInfo->CardFound)&&(!CancelPaymentReq))
        {
        Application->ProcessMessages();
        TimeOut--;
        Sleep(100);
        }

      if (CancelPaymentReq)
        {
        Log->Write("Pre-Init cancel requested!");
        }

      if (!CardReader->FindCardInfo->CardFound)
        {
        CardReader->StopOperation();
        ToFile = "var $ga_metro_menu = {\n";
        ToFile+="     CardNum: 0,\n";
        ToFile+="     Message: 'Ошибка - карта не найдена!',\n";
        ToFile+="     ItemsNum: 0,\n";
        ToFile+="     Menu: []\n";
        ToFile+="}\n\n\n//  - - - - - всегда в true, для динамического include файла\n$inc_metro_menu_js = true;";
        }
        else
        {
        CardNumber = AnsiString(CardReader->FindCardInfo->psCardNum);
        switch (CardReader->FindCardInfo->Result)
          {
          case AFC_Card_Activated:
            Log->Write("GetCard OK. Trying to get menu...");
            ToFile = GetMenuContent();
            break;
          case AFC_No_BSK_Connect:
            Log->Write("GetMenu Error! Can't connect to BSK!");
            ToFile = "var $ga_metro_menu = {\n";
            ToFile+="     CardNum: 0,\n";
            ToFile+="     Message: 'Ошибка поиска карты - нет связи с УЧЗ БСК.',\n";
            ToFile+="     ItemsNum: 0,\n";
            ToFile+="     Menu: []\n";
            ToFile+="}\n\n\n//  - - - - - всегда в true, для динамического include файла\n$inc_metro_menu_js = true;";
            break;
          case AFC_Card_DontRead:
            Log->Write("GetMenu Error! Error reading card info!");
            ToFile = "var $ga_metro_menu = {\n";
            ToFile+="     CardNum: 0,\n";
            ToFile+="     Message: 'Ошибка поиска карты - невозможно прочитать информацию с карты!',\n";
            ToFile+="     ItemsNum: 0,\n";
            ToFile+="     Menu: []\n";
            ToFile+="}\n\n\n//  - - - - - всегда в true, для динамического include файла\n$inc_metro_menu_js = true;";
            break;
          }
        }
      }
      else
      {
      Log->Write("Cardreader not initialized. Pre-init cancelled.");
      ToFile = "var $ga_metro_menu = {\n";
      ToFile+="     CardNum: 0,\n";
      ToFile+="     Message: 'Ошибка поиска карты - устройство недоступно!',\n";
      ToFile+="     ItemsNum: 0,\n";
      ToFile+="     Menu: []\n";
      ToFile+="}\n\n\n//  - - - - - всегда в true, для динамического include файла\n$inc_metro_menu_js = true;";
      }
    bRes = StoreStringToFile(ChangeChars(Cfg->Dirs.InterfaceDir.c_str(),"/","\\")+"\\metro_menu.js",ToFile, Log);
    if (!bRes)
      Log->Write("Error getting menu!");

    if (CancelPaymentReq)
      {
      bRes = false;
      }
    }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
		}
	}
__finally
	{
	return bRes;
	}
}

//---------------------------------------------------------------------------
bool TMetroPayment::Check(bool bFirstCheck, AnsiString AForcedOffline)
{
    UNREFERENCED_PARAMETER(bFirstCheck);
try
  {
  if ((CardValue<=0)||(ForcedSum<=0))
    {
    Log->Write("Wrong amount!");
    return false;
    }
  if (MenuItem == -1)
    {
    Log->Write("Wrong menu item!");
    return false;
    }
  Session=GetSessionNumber().c_str();
  XMLP->InitialSessionNum=Session;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
return true;
}

//---------------------------------------------------------------------------

bool TMetroPayment::Payment(bool bProcessMessages, bool bSendStatusChange)
{
    UNREFERENCED_PARAMETER(bProcessMessages);
    UNREFERENCED_PARAMETER(bSendStatusChange);
bool bResult=false;
bool bRes = false;
AnsiString ToFile;
AnsiString Result;
  try
    {
    RetryAllowed = false;
    if ((!XMLP)||(!PS))
      return false;

    Log->Write("  Payment. ");

    if (CardReader)
      {
      CardReader->WriteCardInfo->pnOrderId=this->OrderId;
      strcpy(CardReader->WriteCardInfo->psCardNum,(CardNumber.SubString(0,2048)).c_str());
      CardReader->WriteCardInfo->nMenuItemID=this->MenuItem;
      CardReader->WriteCardInfo->nAcceptedInRub=short(Sum);
      CardReader->WriteCardInfo->nAuthoriseCode = 0; // Временно
      Result = "MENUITEMNAME="+AnsiString(MenuItemName)+"\r\nCARDNUMBER="+AnsiString(CardNumber)+"\r\n";
      int iWriteCardRes = CardReader->WriteCard();
      switch (iWriteCardRes)
        {
        case AWC_Card_Writed:
          bResult = true;
          PostPaymentInfo = "stored";
          ToFile = "var $ga_metro_menu = {\n";
          ToFile+="     CardNum: 0,\n";
          ToFile+="     Message: '"+AnsiString(CardReader->WriteCardInfo->psCardStatus)+"',\n";
          ToFile+="     ItemsNum: 0,\n";
          ToFile+="     Menu: []\n";
          ToFile+="}\n";
          bRes = StoreStringToFile(ChangeChars(Cfg->Dirs.InterfaceDir.c_str(),"/","\\")+"\\metro_menu.js",ToFile,Log);
          if (!bRes)
            Log->Write("Error storing file!");
          break;
        case AWC_No_BSK_Connect:
          Log->Write("WriteCard Error! Can't connect to BSK!");
          PostPaymentInfo = "Нет связи с УЧЗ БСК!";
          break;
        case AWC_No_Server_Connect:
          Log->Write("WriteCard Error! Can't connect to server!");
          PostPaymentInfo = "Нет связи с сервером приложений метрополитена!";
          break;
        case AWC_Card_DontRead:
          Log->Write("WriteCard Error! Error reading card info!");
          PostPaymentInfo = "stored";
          RetryAllowed = true;
          ToFile = "var $ga_metro_menu = {\n";
          ToFile+="     CardNum: 0,\n";
          ToFile+="     Message: '"+AnsiString(CardReader->WriteCardInfo->psCardStatus)+"',\n";
          ToFile+="     ItemsNum: 0,\n";
          ToFile+="     Menu: []\n";
          ToFile+="}\n";
          bRes = StoreStringToFile(ChangeChars(Cfg->Dirs.InterfaceDir.c_str(),"/","\\")+"\\metro_menu.js",ToFile,Log);
          if (!bRes)
            Log->Write("Error storing file!");
          break;
        case AWC_Invalid_Summ:
          Log->Write("WriteCard Error! Invalid sum!");
          PostPaymentInfo = "Полученная сумма не соответствует выбранному пункту меню!";
          break;
        case AWC_Invalid_Card:
          Log->Write("WriteCard Error! Invalid card!");
          PostPaymentInfo = "Карта в зоне антенны УЧЗ БСК не соотвествует той, с которой начата операция!";
          break;
        case AWC_Invalid_Menu:
          Log->Write("WriteCard Error! Invalid menu item!");
          PostPaymentInfo = "Выбран недопустимый пункт меню!";
          break;
        }

      std::auto_ptr <TStringList> slResult ( new TStringList() );
      if (!slResult.get())
        {
        Log->Write("  slResult creating Error. ");
        bResult = false;
        }
        else
        {
        PrepareAnswer(Result,slResult.get());

        ParseAnswer(slResult.get());

        PaymentProcessedDT=TDateTime::CurrentDateTime();
        }

      PaymentErrorCode = (iWriteCardRes==0 ? 0 : iWriteCardRes+1000);
      }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
  return bResult;
}

//---------------------------------------------------------------------------

