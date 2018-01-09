//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "SetupUnit.h"
#include "common.h"
#include "globals.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//#pragma link "RxGIF"
#pragma link "VirtualTrees"
#pragma resource "*.dfm"
TSetupForm *SetupForm;

const KeyNum = 63;

AnsiString Caps[4][KeyNum] = {
           {"'","1","2","3","4","5","6","7","8","9","0","-","=","<",
           "Tab","q","w","e","r","t","y","u","i","o","p","[","]","\\",
           "Caps","a","s","d","f","g","h","j","k","l",";","'","Enter",
           "Shift","z","x","c","v","b","n","m",",",".","/","",
           "Rus"," ","","","",
           "Сохранить","Закрыть","OK","Проверить","Определить"
           },
           {"~","!","@","#","$","%","^","&","*","(",")","_","+","<",
           "Tab","q","w","e","r","t","y","u","i","o","p","{","}","|",
           "Caps","a","s","d","f","g","h","j","k","l",":","\"","Enter",
           "Shift","z","x","c","v","b","n","m","<",">","?","",
           "Rus"," ","","","",
           "Сохранить","Закрыть","OK","Проверить","Определить"
           },
           {"ё","1","2","3","4","5","6","7","8","9","0","-","=","<",
           "Tab","й","ц","у","к","е","н","г","ш","щ","з","х","ъ","\\",
           "Caps","ф","ы","в","а","п","р","о","л","д","ж","э","Enter",
           "Shift","я","ч","с","м","и","т","ь","б","ю",".","",
           "Lat"," ","","","",
           "Сохранить","Закрыть","OK","Проверить","Определить"
           },
           {"ё","!","\"","№",";","%",":","?","*","(",")","_","+","<",
           "Tab","й","ц","у","к","е","н","г","ш","щ","з","х","ъ","/",
           "Caps","ф","ы","в","а","п","р","о","л","д","ж","э","Enter",
           "Shift","я","ч","с","м","и","т","ь","б","ю",".","",
           "Lat"," ","","","",
           "Сохранить","Закрыть","OK","Проверить","Определить"
           }
           };

int Keys[KeyNum] =
           {192,49,50,51,52,53,54,55,56,57,48,189,187,VK_BACK,
           VK_TAB,81,87,69,82,84,89,85,73,79,80,219,221,220,
           VK_CAPITAL,65,83,68,70,71,72,74,75,76,186,222,13,
           VK_SHIFT,90,88,67,86,66,78,77,188,190,191,VK_UP,
           0,VK_SPACE,VK_LEFT,VK_DOWN,VK_RIGHT,
           0,0,0,0,0
           };

int ColorsId[KeyNum] =
           {0,0,0,0,0,0,0,0,0,0,0,0,0,1,
           1,0,0,0,0,0,0,0,0,0,0,0,0,0,
           1,0,0,0,0,0,0,0,0,0,0,0,1,
           1,0,0,0,0,0,0,0,0,0,0,1,
           1,0,1,1,1,
           1,1,1,1,1
           };

//---------------------------------------------------------------------------
__fastcall TSetupForm::TSetupForm(TComponent* Owner)
  : TForm(Owner)
{
    UNREFERENCED_PARAMETER(Owner);
    //COMPortsScanned = 0;
    //ComPortsList = "Отсутствует,"+GetCOMPortsList(2);
    Top = 0;
    Left = 0;
    Log = NULL;
    Log = new TLogClass("SetupForm");
    Cfg = NULL;
    LastKeyPressed = NULL;
    ShiftPressed = false;
    CapsLock = false;
    Finished = false;
    SetKeysColors();
    SetCaps();

    char Layout[KL_NAMELENGTH];
    Language=lgEng;
    StrCopy(Layout,"00000409");
    LoadKeyboardLayout(Layout,KLF_ACTIVATE);
    ShowActiveControls(NULL);
}
//---------------------------------------------------------------------------



void __fastcall TSetupForm::Shape1MouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    UNREFERENCED_PARAMETER(Sender);
    UNREFERENCED_PARAMETER(Button);
    UNREFERENCED_PARAMETER(Shift);
    UNREFERENCED_PARAMETER(X);
    UNREFERENCED_PARAMETER(Y);
    LastKeyPressed = Sender;
    KeyDepressed->Enabled=true;

    if (ShiftPressed)
    {
        TKeyboardState KState;
        GetKeyboardState(KState);
        KState[VK_SHIFT] = KState[VK_SHIFT]|0x80;
        SetKeyboardState(KState);
    }

    ::PostMessage(this->ActiveControl->Handle, WM_KEYDOWN, Keys[((TLabel*)Sender)->Tag-1], 0x00000001);
    ::PostMessage(this->ActiveControl->Handle, WM_KEYUP, Keys[((TLabel*)Sender)->Tag-1], 0xC0000001);

    Application->ProcessMessages();

    if (ShiftPressed)
    {
        TKeyboardState KState;
        GetKeyboardState(KState);
        KState[VK_SHIFT] = KState[VK_SHIFT]^0x80;
        SetKeyboardState(KState);

        SetKeyColor(LShift);
        //RShift->Color=clBtnFace;
        ShiftPressed = 1-ShiftPressed;
        SetCaps();
    }
}

//---------------------------------------------------------------------------

void __fastcall TSetupForm::Shape1MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    UNREFERENCED_PARAMETER(Button);
    UNREFERENCED_PARAMETER(Shift);
    UNREFERENCED_PARAMETER(X);
    UNREFERENCED_PARAMETER(Y);
    UnpressLastKey();
    ((TLabel*)Sender)->Color=clSkyBlue;
}

//---------------------------------------------------------------------------

void TSetupForm::SetKeyColor(TLabel* Label)
{
if (Label)
  {
  switch (ColorsId[Label->Tag-1])
    {
    case 1:
      Label->Color = OtherKeysColor;Label20->Color;
      break;
    default:
      Label->Color = AlphaKeysColor;Label22->Color;
      break;
    }
  }
}
//---------------------------------------------------------------------------

void TSetupForm::SetKeysColors()
{
OtherKeysColor = Label20->Color;
AlphaKeysColor = Label22->Color;

for (int i=0;i<ComponentCount;i++)
  {
  if (Components[i]->Tag>0)
    {
    SetKeyColor((TLabel*)Components[i]);
    }
  }
}

//---------------------------------------------------------------------------

void TSetupForm::SetCaps()
{
for (int i=0;i<ComponentCount;i++)
  {
  if (Components[i]->Tag>0)
    {
    ((TLabel*)Components[i])->Caption = Caps[ShiftPressed+(Language==lgRus ? 2 : 0)][Components[i]->Tag-1];
    if (((TLabel*)Components[i])->Caption.Length() == 1)
      {
      if (((ShiftPressed)&&(CapsLock))||((!ShiftPressed)&&(!CapsLock)))
        ((TLabel*)Components[i])->Caption = ((TLabel*)Components[i])->Caption.LowerCase();
        else
        ((TLabel*)Components[i])->Caption = ((TLabel*)Components[i])->Caption.UpperCase();
      }
    }
  }
}


void __fastcall TSetupForm::LShiftMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    UNREFERENCED_PARAMETER(Sender);
    UNREFERENCED_PARAMETER(Button);
    UNREFERENCED_PARAMETER(Shift);
    UNREFERENCED_PARAMETER(X);
    UNREFERENCED_PARAMETER(Y);
    if (ShiftPressed)
    {
        SetKeyColor(LShift);
        //RShift->Color=clBtnFace;
    }
    ShiftPressed = 1-ShiftPressed;
    SetCaps();
}
//---------------------------------------------------------------------------

void __fastcall TSetupForm::LShiftMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    UNREFERENCED_PARAMETER(Sender);
    UNREFERENCED_PARAMETER(Button);
    UNREFERENCED_PARAMETER(Shift);
    UNREFERENCED_PARAMETER(X);
    UNREFERENCED_PARAMETER(Y);
    LShift->Color=clSkyBlue;
    //RShift->Color=clSkyBlue;
}
//---------------------------------------------------------------------------

void __fastcall TSetupForm::CLockMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    UNREFERENCED_PARAMETER(Sender);
    UNREFERENCED_PARAMETER(Button);
    UNREFERENCED_PARAMETER(Shift);
    UNREFERENCED_PARAMETER(X);
    UNREFERENCED_PARAMETER(Y);
    TKeyboardState KState;
    GetKeyboardState(KState);
    if (CapsLock)
    {
        KState[VK_CAPITAL] = KState[VK_CAPITAL]^1;
        SetKeyColor(CLock);
    }
    else
    {
        KState[VK_CAPITAL] = KState[VK_CAPITAL]|1;
    }
    SetKeyboardState(KState);
    CapsLock = 1 - CapsLock;
    SetCaps();
}

//---------------------------------------------------------------------------

void __fastcall TSetupForm::RusLatMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    UNREFERENCED_PARAMETER(Sender);
    UNREFERENCED_PARAMETER(Button);
    UNREFERENCED_PARAMETER(Shift);
    UNREFERENCED_PARAMETER(X);
    UNREFERENCED_PARAMETER(Y);
    char Layout[KL_NAMELENGTH];
    //RusLat->Color=clBtnFace;
    if (Language==lgRus)
    {
        Language=lgEng;
        StrCopy(Layout,"00000409");
    }
    else
    {
        Language=lgRus;
        StrCopy(Layout,"00000419");
    }
    LoadKeyboardLayout(Layout,KLF_ACTIVATE);
    SetCaps();
    LastKeyPressed = Sender;
    KeyDepressed->Enabled=true;
}

//---------------------------------------------------------------------------

void __fastcall TSetupForm::KeyDepressedTimer(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);
    UnpressLastKey();
}

//---------------------------------------------------------------------------

void TSetupForm::UnpressLastKey()
{
if (LastKeyPressed != NULL)
  SetKeyColor((TLabel*)LastKeyPressed);
KeyDepressed->Enabled=false;
LastKeyPressed = NULL;
}

//---------------------------------------------------------------------------

void __fastcall TSetupForm::CheckForDouble(TObject *Sender)
{
    ((TEdit*)Sender)->Font->Color = clWindowText;
    ((TEdit*)Sender)->Color = clWindow;
    try
    {
        ((TEdit*)Sender)->Text.ToDouble();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        ((TEdit*)Sender)->Font->Color = clYellow;
        ((TEdit*)Sender)->Color = clRed;
    }
}

//---------------------------------------------------------------------------

void __fastcall TSetupForm::CheckForInt(TObject *Sender)
{
    ((TEdit*)Sender)->Font->Color = clWindowText;
    ((TEdit*)Sender)->Color = clWindow;
    try
    {
        ((TEdit*)Sender)->Text.ToInt();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
        ((TEdit*)Sender)->Font->Color = clYellow;
        ((TEdit*)Sender)->Color = clRed;
    }
}

//---------------------------------------------------------------------------


void __fastcall TSetupForm::Button3Click(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);
    //Cfg->SaveCfgFile(ConfigTV);//Igor
}

//---------------------------------------------------------------------------

void __fastcall TSetupForm::RightArrowLblMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
RightArrowBKLbl->Color=clSkyBlue;
Shape1MouseDown(Sender, Button, Shift, X, Y);
}

//---------------------------------------------------------------------------

void __fastcall TSetupForm::RightArrowLblMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
Shape1MouseUp(Sender, Button, Shift, X, Y);
LastKeyPressed = RightArrowBKLbl;
}
//---------------------------------------------------------------------------

void __fastcall TSetupForm::UpArrowLblMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
UpArrowBKLbl->Color=clSkyBlue;
Shape1MouseDown(Sender, Button, Shift, X, Y);
}
//---------------------------------------------------------------------------

void __fastcall TSetupForm::UpArrowLblMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
Shape1MouseUp(Sender, Button, Shift, X, Y);
LastKeyPressed = UpArrowBKLbl;
}
//---------------------------------------------------------------------------


void __fastcall TSetupForm::DownArrowLblMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
DownArrowBKLbl->Color=clSkyBlue;
Shape1MouseDown(Sender, Button, Shift, X, Y);
}
//---------------------------------------------------------------------------

void __fastcall TSetupForm::DownArrowLblMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
Shape1MouseUp(Sender, Button, Shift, X, Y);
LastKeyPressed = DownArrowBKLbl;
}
//---------------------------------------------------------------------------

void __fastcall TSetupForm::LeftArrowLblMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
LeftArrowBKLbl->Color=clSkyBlue;
Shape1MouseDown(Sender, Button, Shift, X, Y);
}
//---------------------------------------------------------------------------

void __fastcall TSetupForm::LeftArrowLblMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
Shape1MouseUp(Sender, Button, Shift, X, Y);
LastKeyPressed = LeftArrowBKLbl;
}

//---------------------------------------------------------------------------

void __fastcall TSetupForm::FormResize(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);
    if (ClientWidth>1150)
        KeyboardPanel->Left = (ClientWidth - KeyboardPanel->Width)/2;
}

//---------------------------------------------------------------------------
/*
void __fastcall TSetupForm::ConfigTVGetText(TBaseVirtualTree *Sender,
      PVirtualNode Node, TColumnIndex Column, TVSTTextType TextType,
      WideString &CellText)
{
//CellText = (TCfgFieldData*)((VTVNODEDATA*)(Node->Data)->NodeData)->Name;;
    VTVNODEDATA *VTVNodeData = (VTVNODEDATA *)Sender->GetNodeData(Node);

    if (VTVNodeData->NodeData)
    {
      CellText = VTVNodeData->NodeData->Display(Column);
    }
    else
    {
      CellText = "Нет текста";
    }

}
//---------------------------------------------------------------------------

void __fastcall TSetupForm::ConfigTVChange(TBaseVirtualTree *Sender,
      PVirtualNode Node)
{
  if (!Node)
    return;
  if (ActiveNode)
    {
    VTVNODEDATA *VTActiveVNodeData = (VTVNODEDATA *)Sender->GetNodeData(ActiveNode);
    if (VTActiveVNodeData->NodeData)
      {
      VTActiveVNodeData->NodeData->isActiveNode = false;
      switch (VTActiveVNodeData->NodeData->Type)
        {
        case ftBool:
            VTActiveVNodeData->NodeData->Store(AnsiString(1-BoolRG->ItemIndex));
          break;
        case ftList:
        case ftEnum:
        case ftComPortsEnum:
          if(EnumLB->ItemIndex!=-1)
            {
            VTActiveVNodeData->NodeData->Store(EnumLB->Items->Strings[EnumLB->ItemIndex]);
            }
          break;
        }
      }
    }

  VTVNODEDATA *VTVNodeData = (VTVNODEDATA *)Sender->GetNodeData(Node);

  if (VTVNodeData->NodeData)
    {
    CommentLbl->Caption = VTVNodeData->NodeData->Comment;
    switch (VTVNodeData->NodeData->Type)
      {
      case ftNone:
        break;
      case ftInt:
        DataE->Text = VTVNodeData->NodeData->Text;
        CommentLbl->Caption = "От: \t"+AnsiString(VTVNodeData->NodeData->IntMin)+",\nдо: \t"+AnsiString(VTVNodeData->NodeData->IntMax)+".\nПо умолчанию: "+AnsiString(VTVNodeData->NodeData->IntDef) + ".\n\n" + VTVNodeData->NodeData->Comment;
        break;
      case ftBool:
        BoolRG->ItemIndex = 1-GetInt(VTVNodeData->NodeData->Text);
        BoolRG->Height=DelimiterS->Top - 10 - BoolRG->Top;
        break;
      case ftString:
        DataE->Text = VTVNodeData->NodeData->Text;
        CommentLbl->Caption = VTVNodeData->NodeData->Comment + VTVNodeData->NodeData->CheckResult;
        break;
      case ftList:
        EnumLB->Items->DelimitedText=VTVNodeData->NodeData->ListData;
        EnumLB->ItemIndex = EnumLB->Items->IndexOf(VTVNodeData->NodeData->Text);
        break;
      case ftEnum:
        EnumLB->Items->DelimitedText=VTVNodeData->NodeData->ListData;
        EnumLB->ItemIndex = VTVNodeData->NodeData->ListIndex;
        break;
      case ftComPortsEnum:
        EnumLB->Items->DelimitedText=VTVNodeData->NodeData->ListData;
        EnumLB->ItemIndex = VTVNodeData->NodeData->ListIndex;
        break;
      case ftSArray:
        StringsEditMemo->Lines->DelimitedText = VTVNodeData->NodeData->SArrayData;
        break;
      }
    ShowActiveControls(VTVNodeData->NodeData);
    VTVNodeData->NodeData->isActiveNode = true;
    }

  ActiveNode = Node;
}
*/

void TSetupForm::ShowActiveControls(TCfgFieldData *NodeData)
{
DataE->Visible=false;
StringsEditMemo->Visible=false;
BoolRG->Visible=false;
OKL->Visible=false;
OKB->Visible=false;
CheckL->Visible=false;
CheckB->Visible=false;
FindL->Visible=false;
FindB->Visible=false;
EnumLB->Visible=false;



if (!NodeData)
  return;

switch (NodeData->Type)
  {
  case ftNone:
    break;
  case ftInt:
    DataE->Visible=true;
    OKL->Visible=true;
    OKB->Visible=true;
    break;
  case ftString:
    DataE->Visible=true;
    OKL->Visible=true;
    OKB->Visible=true;
    if (NodeData->ASubTypes.Pos(".checkstaturl."))
      {
      CheckL->Visible=true;
      CheckB->Visible=true;
      if (NodeData->CheckInProgress)
        CheckL->Enabled=false;
        else
        CheckL->Enabled=true;
      }
    break;
  case ftBool:
    BoolRG->Visible=true;
    BoolRG->Height = 170;
    break;
  case ftList:
    EnumLB->Visible=true;
    EnumLB->Height = min(EditPanelS->Height/1.5,EnumLB->ItemHeight*(EnumLB->Count+0.5));
    break;
  case ftEnum:
    EnumLB->Visible=true;
    //EnumLB->Height = FindL->Top - EnumLB->Top + FindL->Height;
    EnumLB->Height = min(EditPanelS->Height/1.5,EnumLB->ItemHeight*(EnumLB->Count+0.5));
    //FindL->Top = EnumLB->Top +  EnumLB->Height + 10;
    //FindB->Top = EnumLB->Top +  EnumLB->Height + 10;
    if ((NodeData->ASubTypes.Pos(".find."))||(NodeData->ASubTypes.Pos(".findval.")))
      {
      FindL->Visible=true;
      FindB->Visible=true;
      FindL->Enabled=true;
      FindL->Top = EnumLB->Top +  EnumLB->Height + 10;
      FindB->Top = FindL->Top-1;
      }
    break;
  case ftComPortsEnum:
    EnumLB->Visible=true;
    FindB->Visible = true;
    FindL->Visible = true;
    FindL->Enabled = true;
    //EnumLB->Height = FindL->Top - EnumLB->Top - 10;
    EnumLB->Height = min(EditPanelS->Height/1.5,EnumLB->ItemHeight*(EnumLB->Count+0.5));
    FindL->Top = EnumLB->Top +  EnumLB->Height + 10;
    FindB->Top = FindL->Top-1;
    break;
  case ftSArray:
    StringsEditMemo->Visible=true;
    StringsEditMemo->Height = EditPanelS->Height/1.5;
    break;
  }

CommentLbl->Top = EditPanelS->Top+10;

if (DataE->Visible) CommentLbl->Top = max(CommentLbl->Top, DataE->Top+DataE->Height+20);
if (StringsEditMemo->Visible) CommentLbl->Top = max(CommentLbl->Top, StringsEditMemo->Top+StringsEditMemo->Height+20);
if (BoolRG->Visible) CommentLbl->Top = max(CommentLbl->Top, BoolRG->Top+BoolRG->Height+20);
if (OKL->Visible) CommentLbl->Top = max(CommentLbl->Top, OKL->Top+OKL->Height+20);
if (CheckL->Visible) CommentLbl->Top = max(CommentLbl->Top, CheckL->Top+CheckL->Height+20);
if (FindL->Visible) CommentLbl->Top = max(CommentLbl->Top, FindL->Top+FindL->Height+20);
//if (CheckL->Visible) CommentLbl->Top = max(CommentLbl->Top, CheckL->Top+CheckL->Height+20);
if (EnumLB->Visible) CommentLbl->Top = max(CommentLbl->Top, EnumLB->Top+EnumLB->Height+20);

DelimiterS->Top = CommentLbl->Top - 10;

}
/*
//---------------------------------------------------------------------------

void __fastcall TSetupForm::ConfigTVPaintText(TBaseVirtualTree *Sender,
      const TCanvas *TargetCanvas, PVirtualNode Node, TColumnIndex Column,
      TVSTTextType TextType)
{

if (Column==1)
  {

  TargetCanvas->Font->Style = TFontStyles()<< fsBold;

  VTVNODEDATA *VTVNodeData = (VTVNODEDATA *)Sender->GetNodeData(Node);

  if (VTVNodeData->NodeData)
    {
    switch (VTVNodeData->NodeData->Type)
      {
      case ftNone:
        break;
      case ftInt:
        TargetCanvas->Font->Color = clBlack;
        break;
      case ftBool:
        if (VTVNodeData->NodeData->Text=="0")
          TargetCanvas->Font->Color = clRed;
          else
          TargetCanvas->Font->Color = clGreen;
        break;
      case ftString:
        TargetCanvas->Font->Color = clBlue;
        break;
      case ftList:
        break;
      case ftEnum:
        break;
      case ftComPortsEnum:
        break;
      case ftSArray:
        break;
      }
    }
  }
}
//---------------------------------------------------------------------------

void __fastcall TSetupForm::ConfigTVBeforeCellPaint(
      TBaseVirtualTree *Sender, TCanvas *TargetCanvas, PVirtualNode Node,
      TColumnIndex Column, TRect &CellRect)
{
if (Column==1)
  {
  TargetCanvas->Brush->Color = clWhite;
  TargetCanvas->FillRect(CellRect);
  }
}
//---------------------------------------------------------------------------

void __fastcall TSetupForm::ConfigTVExpanded(TBaseVirtualTree *Sender,
      PVirtualNode Node)
{
ConfigTV->Header->Columns->Items[1]->Width = ConfigTV->ClientWidth - ConfigTV->Header->Columns->Items[0]->Width - GetSystemMetrics(SM_CXHSCROLL);
}
//---------------------------------------------------------------------------

void __fastcall TSetupForm::ConfigTVCollapsed(TBaseVirtualTree *Sender,
      PVirtualNode Node)
{
ConfigTV->Header->Columns->Items[1]->Width = ConfigTV->ClientWidth - ConfigTV->Header->Columns->Items[0]->Width - GetSystemMetrics(SM_CXHSCROLL);
}
//---------------------------------------------------------------------------

void __fastcall TSetupForm::ConfigTVFreeNode(TBaseVirtualTree *Sender,
      PVirtualNode Node)
{
if (Log) Log->Write("Free node #"+AnsiString(Node->Index));
VTVNODEDATA *VTVNodeData = (VTVNODEDATA *)Sender->GetNodeData(Node);
if (VTVNodeData->NodeData)
  {
  if (Log) Log->Append(", delete node #"+AnsiString(Node->Index)+" : "+VTVNodeData->NodeData->Name+".");
  delete VTVNodeData->NodeData;
  VTVNodeData->NodeData = NULL;
  }
}
//---------------------------------------------------------------------------



void __fastcall TSetupForm::ConfigTVHeaderDraw(TVTHeader *Sender,
      TCanvas *HeaderCanvas, TVirtualTreeColumn *Column, TRect &R,
      bool Hover, bool Pressed, TVTDropMarkMode DropMark)
{
//  HeaderCanvas->Brush->Color = clWhite;
//  HeaderCanvas->FillRect(R);
}
//---------------------------------------------------------------------------
*/

void __fastcall TSetupForm::SaveLMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    UNREFERENCED_PARAMETER(Sender);
    UNREFERENCED_PARAMETER(Button);
    UNREFERENCED_PARAMETER(Shift);
    UNREFERENCED_PARAMETER(X);
    UNREFERENCED_PARAMETER(Y);
    LastKeyPressed = Sender;
    KeyDepressed->Enabled=true;
    Application->ProcessMessages();
    //Cfg->SaveCfgFile(ConfigTV);//Igor
}
//---------------------------------------------------------------------------

void __fastcall TSetupForm::CloseLMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    UNREFERENCED_PARAMETER(Sender);
    UNREFERENCED_PARAMETER(Button);
    UNREFERENCED_PARAMETER(Shift);
    UNREFERENCED_PARAMETER(X);
    UNREFERENCED_PARAMETER(Y);
    LastKeyPressed = Sender;
    KeyDepressed->Enabled=true;
    Application->ProcessMessages();
    Close();
    Finished = true;
}
//---------------------------------------------------------------------------

void __fastcall TSetupForm::SetConfigTree()
{
  Top = 0;
  Left = 0;
  Finished = false;
  std::auto_ptr <TStringList> RuleSL ( new TStringList() );
  //ConfigTV->BeginUpdate();//Igor
  Cfg->ClearConfigTree(/*ConfigTV*/);//Igor
  //ConfigTV->EndUpdate();//Igor
  Cfg->GetConfigTree(/*ConfigTV, */RuleSL.get(), true);//Igor
  Cfg->GetConfigTree(/*ConfigTV, */RuleSL.get(), false);//Igor
}
//---------------------------------------------------------------------------

void __fastcall TSetupForm::OKLMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    UNREFERENCED_PARAMETER(Sender);
    UNREFERENCED_PARAMETER(Button);
    UNREFERENCED_PARAMETER(Shift);
    UNREFERENCED_PARAMETER(X);
    UNREFERENCED_PARAMETER(Y);
    LastKeyPressed = Sender;
    KeyDepressed->Enabled=true;
    Application->ProcessMessages();
    try
    {
    /*//Igor
      if (ActiveNode)
      {
      VTVNODEDATA *VTActiveVNodeData = (VTVNODEDATA *)ConfigTV->GetNodeData(ActiveNode);
      if (VTActiveVNodeData->NodeData)
        {
        switch (VTActiveVNodeData->NodeData->Type)
          {
          case ftInt:
              VTActiveVNodeData->NodeData->Store(DataE->Text);
              ConfigTV->RepaintNode(ActiveNode);
            break;
          case ftString:
              VTActiveVNodeData->NodeData->Store(DataE->Text);
              ConfigTV->RepaintNode(ActiveNode);
            break;
          case ftSArray:
              VTActiveVNodeData->NodeData->Store(StringsEditMemo->Lines->DelimitedText);
              ConfigTV->RepaintNode(ActiveNode);
            break;
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
void __fastcall TSetupForm::CheckLMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    UNREFERENCED_PARAMETER(Button);
    UNREFERENCED_PARAMETER(Shift);
    UNREFERENCED_PARAMETER(X);
    UNREFERENCED_PARAMETER(Y);
    LastKeyPressed = Sender;
    KeyDepressed->Enabled=true;
    Application->ProcessMessages();
    CheckL->Enabled = false;
    /*VTVNODEDATA *VTVNodeData = (VTVNODEDATA *)ConfigTV->GetNodeData(ActiveNode);//Igor
    if (VTVNodeData->NodeData)
    {
        VTVNodeData->NodeData->Check(DataE->Text, CommentLbl, CheckL);
    }
    */
}
//---------------------------------------------------------------------------
void __fastcall TSetupForm::FindLMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    UNREFERENCED_PARAMETER(Button);
    UNREFERENCED_PARAMETER(Shift);
    UNREFERENCED_PARAMETER(X);
    UNREFERENCED_PARAMETER(Y);
    LastKeyPressed = Sender;
    KeyDepressed->Enabled=true;
    Application->ProcessMessages();
}
//---------------------------------------------------------------------------
void __fastcall TSetupForm::FormKeyPress(TObject *Sender, char &Key)
{
    UNREFERENCED_PARAMETER(Sender);
    if(Key == '\b')
    {
        if(Log)
            Log->Write("Esc key pressed.");
        Close();
    }
}
//---------------------------------------------------------------------------
void __fastcall TSetupForm::FormClose(TObject *Sender, TCloseAction &Action)
{
    UNREFERENCED_PARAMETER(Sender);
    UNREFERENCED_PARAMETER(Action);
    /*ConfigTV->BeginUpdate();//Igor
    if(Cfg)
        Cfg->ClearConfigTree(ConfigTV);
    ConfigTV->EndUpdate();*/
    if(Log)
    {
        delete Log;
        Log = NULL;
    }
}
//---------------------------------------------------------------------------

