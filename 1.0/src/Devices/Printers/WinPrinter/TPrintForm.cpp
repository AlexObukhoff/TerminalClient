//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "TPrintForm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPrintForm *PrintForm;
//---------------------------------------------------------------------------
__fastcall TPrintForm::TPrintForm(TComponent* Owner)
  : TForm(Owner)
{
    Visible = false;
}
//---------------------------------------------------------------------------

void TPrintForm::Clear()
{
    PrintBox->Clear();
}

void TPrintForm::Add(AnsiString line)
{
    PrintBox->Lines->Add(line);
}

void TPrintForm::Print()
{
    PrintBox->Print("");
}
