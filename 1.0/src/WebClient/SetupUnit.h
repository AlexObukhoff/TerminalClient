//---------------------------------------------------------------------------

#ifndef SetupUnitH
#define SetupUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <msxmldom.hpp>
#include <XMLDoc.hpp>
#include <xmldom.hpp>
#include <XMLIntf.hpp>
#include <CheckLst.hpp>
#include <Grids.hpp>
#include <ValEdit.hpp>
#include "TWConfig.h"
#include <Graphics.hpp>
#include <ImgList.hpp>
//#include "VirtualTrees.hpp"
#include <jpeg.hpp>
#include "TCfgFieldData.h"
//---------------------------------------------------------------------------

enum InputLangs {lgEng, lgRus};

class TSetupForm : public TForm
{
__published:	// IDE-managed Components
  TPanel *KeyboardPanel;
  TBevel *Bevel64;
  TBevel *Bevel65;
  TBevel *Bvl;
  TBevel *Bevel1;
  TLabel *Label22;
  TBevel *Bevel2;
  TBevel *Bevel3;
  TLabel *Label8;
  TBevel *Bevel4;
  TLabel *Label9;
  TBevel *Bevel5;
  TLabel *Label10;
  TBevel *Bevel6;
  TLabel *Label11;
  TBevel *Bevel7;
  TLabel *Label12;
  TBevel *Bevel8;
  TLabel *Label13;
  TBevel *Bevel9;
  TLabel *Label14;
  TBevel *Bevel10;
  TLabel *Label15;
  TBevel *Bevel11;
  TLabel *Label16;
  TBevel *Bevel12;
  TLabel *Label17;
  TBevel *Bevel13;
  TLabel *Label18;
  TBevel *Bevel14;
  TLabel *Label19;
  TLabel *Label23;
  TBevel *Bevel15;
  TLabel *Label20;
  TBevel *Bevel16;
  TBevel *Bevel17;
  TLabel *Label21;
  TBevel *Bevel18;
  TLabel *Label24;
  TBevel *Bevel19;
  TLabel *Label25;
  TBevel *Bevel20;
  TLabel *Label26;
  TBevel *Bevel21;
  TLabel *Label27;
  TBevel *Bevel22;
  TLabel *Label28;
  TBevel *Bevel23;
  TLabel *Label29;
  TBevel *Bevel24;
  TLabel *Label30;
  TBevel *Bevel25;
  TLabel *Label31;
  TBevel *Bevel26;
  TLabel *Label32;
  TBevel *Bevel27;
  TLabel *Label33;
  TLabel *Label34;
  TBevel *Bevel28;
  TLabel *Label35;
  TBevel *Bevel29;
  TLabel *CLock;
  TBevel *Bevel30;
  TBevel *Bevel31;
  TLabel *Label37;
  TBevel *Bevel32;
  TLabel *Label38;
  TBevel *Bevel33;
  TLabel *Label39;
  TBevel *Bevel34;
  TLabel *Label40;
  TBevel *Bevel35;
  TLabel *Label41;
  TBevel *Bevel36;
  TLabel *Label42;
  TBevel *Bevel37;
  TLabel *Label43;
  TBevel *Bevel38;
  TLabel *Label44;
  TBevel *Bevel39;
  TLabel *Label45;
  TBevel *Bevel40;
  TLabel *Label46;
  TBevel *Bevel41;
  TLabel *Label47;
  TLabel *Label48;
  TBevel *Bevel42;
  TLabel *LShift;
  TBevel *Bevel43;
  TBevel *Bevel44;
  TLabel *Label50;
  TBevel *Bevel45;
  TLabel *Label51;
  TBevel *Bevel46;
  TLabel *Label52;
  TBevel *Bevel47;
  TLabel *Label53;
  TBevel *Bevel48;
  TLabel *Label54;
  TBevel *Bevel49;
  TLabel *Label55;
  TBevel *Bevel50;
  TLabel *Label56;
  TBevel *Bevel51;
  TLabel *Label57;
  TBevel *Bevel52;
  TLabel *Label58;
  TLabel *Label61;
  TBevel *Bevel53;
  TLabel *RusLat;
  TBevel *Bevel55;
  TLabel *Label62;
  TBevel *Bevel61;
  TLabel *RightArrowBKLbl;
  TImage *RightArrowImg;
  TLabel *RightArrowLbl;
  TLabel *DownArrowBKLbl;
  TLabel *UpArrowBKLbl;
  TLabel *LeftArrowBKLbl;
  TImage *DownArrowImg;
  TImage *LefttArrowImg;
  TLabel *LeftArrowLbl;
  TImage *UpArrowImg;
  TLabel *UpArrowLbl;
  TLabel *DownArrowLbl;
  TTimer *KeyDepressed;
  TLabel *SaveL;
  TBevel *Bevel54;
  TLabel *CloseL;
  TBevel *Bevel56;
  TLabel *CheckL;
  TBevel *CheckB;
  TLabel *FindL;
  TBevel *FindB;
  TLabel *OKL;
  TBevel *OKB;
  TEdit *DataE;
  TShape *EditPanelS;
  TLabel *CommentLbl;
  TShape *DelimiterS;
  TRadioGroup *BoolRG;
  TMemo *StringsEditMemo;
  TListBox *EnumLB;
  void __fastcall Shape1MouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall Shape1MouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall LShiftMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall LShiftMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall CLockMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall RusLatMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall KeyDepressedTimer(TObject *Sender);
  void __fastcall CheckForDouble(TObject *Sender);
  void __fastcall CheckForInt(TObject *Sender);
  void __fastcall Button3Click(TObject *Sender);
  void __fastcall RightArrowLblMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
  void __fastcall RightArrowLblMouseUp(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
  void __fastcall UpArrowLblMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall UpArrowLblMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall DownArrowLblMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
  void __fastcall DownArrowLblMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall LeftArrowLblMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
  void __fastcall LeftArrowLblMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall FormResize(TObject *Sender);
  /*
  void __fastcall ConfigTVGetText(TBaseVirtualTree *Sender,
          PVirtualNode Node, TColumnIndex Column, TVSTTextType TextType,
          WideString &CellText);
  void __fastcall ConfigTVChange(TBaseVirtualTree *Sender,
          PVirtualNode Node);
  void __fastcall ConfigTVPaintText(TBaseVirtualTree *Sender,
          const TCanvas *TargetCanvas, PVirtualNode Node,
          TColumnIndex Column, TVSTTextType TextType);
  void __fastcall ConfigTVBeforeCellPaint(TBaseVirtualTree *Sender,
          TCanvas *TargetCanvas, PVirtualNode Node, TColumnIndex Column,
          TRect &CellRect);
  void __fastcall ConfigTVExpanded(TBaseVirtualTree *Sender,
          PVirtualNode Node);
  void __fastcall ConfigTVCollapsed(TBaseVirtualTree *Sender,
          PVirtualNode Node);
  void __fastcall ConfigTVFreeNode(TBaseVirtualTree *Sender,
          PVirtualNode Node);
  void __fastcall ConfigTVHeaderDraw(TVTHeader *Sender,
          TCanvas *HeaderCanvas, TVirtualTreeColumn *Column, TRect &R,
          bool Hover, bool Pressed, TVTDropMarkMode DropMark);
          */
  void __fastcall SaveLMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall CloseLMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
//  void __fastcall FormActivate(TObject *Sender);
  void __fastcall OKLMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall CheckLMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall FindLMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall FormKeyPress(TObject *Sender, char &Key);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations
  void SetCaps();
  int KeybState;
  bool ShiftPressed;
  bool CapsLock;
  InputLangs Language;
  TObject* LastKeyPressed;
  TColor AlphaKeysColor;
  TColor OtherKeysColor;
  void UnpressLastKey();
  AnsiString ActiveFieldType;
  AnsiString ActiveFieldComment;
  AnsiString ActiveFieldData;
  int ActiveIntFieldLimMin;
  int ActiveIntFieldLimMax;
  int ActiveIntFieldDefault;
  //TVirtualNode* ActiveNode;//Igor
//  void TSetupForm::ParseRule(AnsiString Rule);
  void CheckConnectionToMonitoringServer(AnsiString URL);
  void SetKeyColor(TLabel* Label);
  void SetKeysColors();
  void ShowActiveControls(TCfgFieldData *NodeData);
public:		// User declarations
  __fastcall TSetupForm(TComponent* Owner);
  void __fastcall SetConfigTree();
  TLogClass *Log;
  TWConfig* Cfg;
  bool Finished;
};
//---------------------------------------------------------------------------
extern PACKAGE TSetupForm *SetupForm;
//---------------------------------------------------------------------------
#endif
