//---------------------------------------------------------------------------

#include <vcl.h>
#include <algorith>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("ConnMainForm.cpp", Form1);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    SetErrorMode(SEM_NOGPFAULTERRORBOX);
    try
    {
         Application->Initialize();
         Application->ShowMainForm = false;
         Application->HideHint();
         Application->CreateForm(__classid(TForm1), &Form1);
         Application->Run();
    }
    catch(...){
    //catch(Exception &exception) {
         //Application->ShowException(&exception);
    }
    return 0;
}
//---------------------------------------------------------------------------
