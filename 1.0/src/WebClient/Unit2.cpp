//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "Unit2.h"
#include "globals.h"
#include "localize.h"
#include "boost/format.hpp"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm2 *Form2;
//---------------------------------------------------------------------------
__fastcall TForm2::TForm2(TComponent* Owner)
    : TForm(Owner)
{
    m_FinishedCS = new TCriticalSection;
    Finished = false;
    Log=NULL;
}
//---------------------------------------------------------------------------
__fastcall TForm2::~TForm2()
{
    CancelTimer->Enabled = false;
}
//---------------------------------------------------------------------------
void __fastcall TForm2::CancelTimerTimer(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);
    PB->StepIt();
    if(PB->Position >= PB->Max) {
        CancelTimer->Enabled = false;
        m_FinishedCS->Acquire();
        Finished = true;
        m_FinishedCS->Release();
        Result = 0;
        if(Log)
            Log->Write("Form2 TimeOut.");
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm2::YesBtnClick(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);
    if (Log)
        Log->Write("Form2 - YES btn pressed.");
    ActiveControl = NULL;
    m_FinishedCS->Acquire();
    Finished = true;
    m_FinishedCS->Release();
    if(IsEmptyPassword)
        Result = 1;
    else {
        for(int j = 0; j < cnMaxPasswords; j++) {
            if(((IsGood(NumPadEdit->Text.c_str(), PasswordMask[j].c_str())) && (NumPadEdit->Text.Length() == (int)PasswordMask[j].length())) || (PasswordMask[j] == "A") || ((PasswordMask[j] == "A!E") && (NumPadEdit->Text != ""))) {
                Result = j + 1;
                return;
            }
        }
    }
}
//---------------------------------------------------------------------------
bool TForm2::IsFinished()
{
    m_FinishedCS->Acquire();
    bool f = Finished;
    m_FinishedCS->Release();
    return f;
}
//---------------------------------------------------------------------------
void __fastcall TForm2::NoBtnClick(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);
    CancelTimer->Enabled = false;
    ActiveControl = NULL;
    m_FinishedCS->Acquire();
    Finished = true;
    m_FinishedCS->Release();
    Result = 0;
    if(Log)
        Log->Write("Form2 - NO btn pressed.");
}
//---------------------------------------------------------------------------
void __fastcall TForm2::NumBtnClickEditPassword(TObject *Sender)
{
    ActiveControl = NULL;
    NumPadEdit->Text = NumPadEdit->Text + ((TButton *)Sender)->Caption;
    BSBtn->Enabled = true;
    CheckForCompleteString();
}
//---------------------------------------------------------------------------
void __fastcall TForm2::BSBtnClick(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);
    ActiveControl = NULL;
    NumPadEdit->Text = NumPadEdit->Text.SubString(0, NumPadEdit->Text.Length() - 1);
    if(NumPadEdit->Text == "")
        BSBtn->Enabled = false;
    CheckForCompleteString();
}
//---------------------------------------------------------------------------
void TForm2::EnableNumPad(bool Enable)
{
    NumBtn0->Enabled = Enable;
    NumBtn1->Enabled = Enable;
    NumBtn2->Enabled = Enable;
    NumBtn3->Enabled = Enable;
    NumBtn4->Enabled = Enable;
    NumBtn5->Enabled = Enable;
    NumBtn6->Enabled = Enable;
    NumBtn7->Enabled = Enable;
    NumBtn8->Enabled = Enable;
    NumBtn9->Enabled = Enable;
    BSBtn->Enabled = Enable;
    NumPadEdit->Enabled = Enable;
}
//---------------------------------------------------------------------------
void TForm2::Init(TLogClass* _Log, int Timeout)
{
    m_Timeout = Timeout;
    PB->Max = Timeout / 100;
    YesBtn->Enabled = true;
    NoBtn->Enabled = true;
    YesBtn->Default = true;
    if(!Log)
        Log = _Log;
    Finished = false;
    Result = mrNone;
    NumPadEdit->Text = "";
    PB->Position = 0;
    CancelTimer->Enabled = true;
}
//---------------------------------------------------------------------------
bool __fastcall TForm2::IsGood(std::string &ASrc, std::string &Pswd)
{
    for(int i = 0; i <= ASrc.length(); i++) {
        if(!((ASrc.substr(i, 1) == Pswd.substr(i, 1)) || (Pswd.substr(i, 1) == "*")))
            return false;
    }
    return true;
}
//---------------------------------------------------------------------------
void __fastcall TForm2::CheckForCompleteString()
{
    if(!IsEmptyPassword) {
        YesBtn->Enabled = false;
        if(NumPadEdit->Text == "")
            return;
            
        for(int j = 0; j < cnMaxPasswords; j++)
        {
            if((IsGood(NumPadEdit->Text.c_str(), PasswordMask[j])) || (PasswordMask[j] == "A") || (PasswordMask[j] == "A!E")) {
                YesBtn->Enabled = true;
                return;
            }
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm2::FormCreate(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);
    ScaleBy(Screen->Width, 1024);
}
//---------------------------------------------------------------------------
void __fastcall TForm2::FormKeyPress(TObject *Sender, char &Key)
{
    if ((Key >= '0')&&(Key <= '9')) {
        AnsiString Temp =  NumPadEdit->Text+AnsiString(Key);
        NumPadEdit->Text = Temp;
        CheckForCompleteString();
    }
    if ((Key == '\r') && (YesBtn->Enabled))
        YesBtnClick(Sender);
    if ((Key == '\b') && (BSBtn->Enabled))
        BSBtnClick(Sender);
}
//---------------------------------------------------------------------------
void TForm2::ShowMessage(const char *AMessage, int FontSize)
{
    Message->Caption = AMessage;
    Message->Font->Size = FontSize;
    NumPadEdit->PasswordChar = 0;
    YesBtn->Caption = Localization["OK"];
    NoBtn->Enabled=false;
    for(int j = 0; j < cnMaxPasswords; j++)
        PasswordMask[j]="";
    Show();
}
//---------------------------------------------------------------------------
void TForm2::ShowForm(const char *AMessage, std::string Passwords, bool bShowText, form2::Elements elements, int FontSize)
{
    try {
        SetWindowPos(Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        Message->Caption=AMessage;
        Message->Font->Size=FontSize;
        EnableNumPad(true);
        YesBtn->Caption = Localization["yes"];
        NoBtn->Caption = Localization["no"];
        Incass_banknotes_Btn->Caption = Localization["banknotes_incass"];
        Incass_coins_Btn->Caption = Localization["coins_incass"];
        BackBtn->Caption = Localization["back_to_menu"];

        SetMask(Passwords.c_str(), bShowText);
        switch (elements)
        {
            case form2::HideAll:
                NumBtn0->Visible = false;
                NumBtn1->Visible = false;
                NumBtn2->Visible = false;
                NumBtn3->Visible = false;
                NumBtn4->Visible = false;
                NumBtn5->Visible = false;
                NumBtn6->Visible = false;
                NumBtn7->Visible = false;
                NumBtn8->Visible = false;
                NumBtn9->Visible = false;

                YesBtn->Visible = false;
                NoBtn->Visible = false;
                BSBtn->Visible = false;
                NumPadEdit->Visible = false;
                PB->Visible = false;
                Incass_banknotes_Btn->Visible = false;
                Incass_coins_Btn->Visible = false;
                BackBtn->Visible = false;
            break;
            case form2::ShowAll:
                NumBtn0->Visible = true;
                NumBtn1->Visible = true;
                NumBtn2->Visible = true;
                NumBtn3->Visible = true;
                NumBtn4->Visible = true;
                NumBtn5->Visible = true;
                NumBtn6->Visible = true;
                NumBtn7->Visible = true;
                NumBtn8->Visible = true;
                NumBtn9->Visible = true;

                YesBtn->Visible = true;
                NoBtn->Visible = true;
                BSBtn->Visible = true;
                NumPadEdit->Visible = true;
                PB->Visible = true;
                Incass_banknotes_Btn->Visible = false;
                Incass_coins_Btn->Visible = false;
                BackBtn->Visible = false;
            break;
            case form2::Show2Choice:
                NumBtn0->Visible = false;
                NumBtn1->Visible = false;
                NumBtn2->Visible = false;
                NumBtn3->Visible = false;
                NumBtn4->Visible = false;
                NumBtn5->Visible = false;
                NumBtn6->Visible = false;
                NumBtn7->Visible = false;
                NumBtn8->Visible = false;
                NumBtn9->Visible = false;

                YesBtn->Visible = false;
                NoBtn->Visible = false;
                BSBtn->Visible = false;
                NumPadEdit->Visible = false;
                PB->Visible = false;
                Incass_banknotes_Btn->Visible = true;
                Incass_coins_Btn->Visible = true;
                BackBtn->Visible = true;
            break;
        }
        Show();
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}
//---------------------------------------------------------------------------
void TForm2::PushState()
{
    PrevState[0] = NumBtn0->Enabled;
    PrevState[1] = NumBtn1->Enabled;
    PrevState[2] = NumBtn2->Enabled;
    PrevState[3] = NumBtn3->Enabled;
    PrevState[4] = NumBtn4->Enabled;
    PrevState[5] = NumBtn5->Enabled;
    PrevState[6] = NumBtn6->Enabled;
    PrevState[7] = NumBtn7->Enabled;
    PrevState[8] = NumBtn8->Enabled;
    PrevState[9] = NumBtn9->Enabled;
    PrevState[10] = YesBtn->Enabled;
    PrevState[11] = NoBtn->Enabled;
    PrevState[12] = BSBtn->Enabled;
    PrevState[13] = NumPadEdit->Enabled;
}
//---------------------------------------------------------------------------
void TForm2::DisableAll()
{
    EnableNumPad(false);
    YesBtn->Enabled = false;
    NoBtn->Enabled = false;
    BSBtn->Enabled = false;
}
//---------------------------------------------------------------------------
void TForm2::PopState()
{
    NumBtn0->Enabled = PrevState[0];
    NumBtn1->Enabled = PrevState[1];
    NumBtn2->Enabled = PrevState[2];
    NumBtn3->Enabled = PrevState[3];
    NumBtn4->Enabled = PrevState[4];
    NumBtn5->Enabled = PrevState[5];
    NumBtn6->Enabled = PrevState[6];
    NumBtn7->Enabled = PrevState[7];
    NumBtn8->Enabled = PrevState[8];
    NumBtn9->Enabled = PrevState[9];
    YesBtn->Enabled = PrevState[10];
    NoBtn->Enabled = PrevState[11];
    BSBtn->Enabled = PrevState[12];
    NumPadEdit->Enabled = PrevState[13];
}
//---------------------------------------------------------------------------
void __fastcall TForm2::FormPaint(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);
    if(ActiveControl == NoBtn)
        ActiveControl =  NULL;
}
//---------------------------------------------------------------------------
void TForm2::EnableOK(bool Enable)
{
    YesBtn->Enabled = Enable;
}
//---------------------------------------------------------------------------
void TForm2::ClearFinished()
{
    Finished = false;
}
//---------------------------------------------------------------------------
void TForm2::SetMask(std::string Passwords, bool bShowText)
{
    CancelTimer->Enabled = true;
    NumPadEdit->Text = "";
    NumPadEdit->PasswordChar = (bShowText ? ((char)0) : '*');
    IsEmptyPassword = Passwords == "";
    if(!IsEmptyPassword) {
        Passwords += "|";
        std::string Password;
        for(int i = 0; i < cnMaxPasswords; i++)
            PasswordMask[i] = "";
        int i = 0;
        while((Passwords.find("|")) && (i < cnMaxPasswords)) {
            Password = Passwords.substr(0, Passwords.find("|"));
            PasswordMask[i] = Password;
            Passwords = Passwords.substr(Passwords.find("|") + 1, Passwords.length());
            i++;
        }
        CheckForCompleteString();
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm2::Choice2BtnClick(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);
    if (Log)
        Log->Write("Form2 - BACK btn pressed.");
    ActiveControl = NULL;
    m_FinishedCS->Acquire();
    Finished = true;
    m_FinishedCS->Release();

    ActiveControl = NULL;
    if (((TButton *)Sender)->Name == "Incass_banknotes_Btn")
        NumPadEdit->Text = "0";
    else if (((TButton *)Sender)->Name == "Incass_coins_Btn")
        NumPadEdit->Text = "1";

    BSBtn->Enabled = true;
    CheckForCompleteString();
}
//---------------------------------------------------------------------------
void __fastcall TForm2::BackBtnClick(TObject *Sender)
{
    UNREFERENCED_PARAMETER(Sender);
    CancelTimer->Enabled = false;
    ActiveControl = NULL;
    m_FinishedCS->Acquire();
    Finished = true;
    m_FinishedCS->Release();
    Result = 0;
    if(Log)
        Log->Write("Form2 - Back btn pressed.");
}
//---------------------------------------------------------------------------

