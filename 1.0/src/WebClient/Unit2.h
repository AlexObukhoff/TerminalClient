//---------------------------------------------------------------------------

#ifndef Unit2H
#define Unit2H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
#include "LogClass.h"
//---------------------------------------------------------------------------
const int cnMaxPasswords = 2;

namespace form2
{
    typedef enum
    {
        Show2Choice = -3,
        HideAll = -2,
        ShowAll = -1

    } Elements;
}

class TForm2 : public TForm
{
__published:
    TButton *YesBtn;
    TButton *NoBtn;
    TLabel *Message;
    TProgressBar *PB;
    TTimer *CancelTimer;
    TBevel *Bevel3;
    TButton *NumBtn1;
    TButton *NumBtn2;
    TButton *NumBtn3;
    TButton *NumBtn4;
    TButton *NumBtn5;
    TButton *NumBtn6;
    TButton *NumBtn7;
    TButton *NumBtn8;
    TButton *NumBtn9;
    TButton *BSBtn;
    TButton *NumBtn0;
    TEdit *NumPadEdit;
    TButton *Incass_coins_Btn;
    TButton *Incass_banknotes_Btn;
    TButton *BackBtn;
    void __fastcall CancelTimerTimer(TObject *Sender);
    void __fastcall YesBtnClick(TObject *Sender);
    void __fastcall NoBtnClick(TObject *Sender);
    void __fastcall NumBtnClickEditPassword(TObject *Sender);
    void __fastcall BSBtnClick(TObject *Sender);
    void __fastcall FormCreate(TObject *Sender);
    void __fastcall FormKeyPress(TObject *Sender, char &Key);
    void __fastcall FormPaint(TObject *Sender);
    void __fastcall BackBtnClick(TObject *Sender);
    void __fastcall Choice2BtnClick(TObject *Sender);
private:
    bool PrevState[15];
    TLogClass* Log;
    bool Finished;
    bool IsEmptyPassword;
    int m_Timeout;
    TCriticalSection *m_FinishedCS;
    std::string PasswordMask[cnMaxPasswords];
private:
    __fastcall TForm2(TComponent* Owner);
    __fastcall ~TForm2();
    void __fastcall CheckForCompleteString();
    bool __fastcall IsGood(std::string &ASrc, std::string &Pswd);
public:
    int Result;
public:
    void EnableOK(bool Enable);
    void Init(TLogClass* _Log, int Timeout);
    void EnableNumPad(bool Enable);
    bool IsFinished();
    void ShowMessage(const char *AMessage, int FontSize = 20);
    void ShowForm(const char *AMessage, std::string Password = "", bool bShowText = true, form2::Elements elements = form2::ShowAll, int FontSize = 40);
    void PushState();
    void DisableAll();
    void PopState();
    void SetMask(std::string Passwords, bool bShowText);
    void ClearFinished();
};
//---------------------------------------------------------------------------
extern PACKAGE TForm2 *Form2;
//---------------------------------------------------------------------------
#endif
