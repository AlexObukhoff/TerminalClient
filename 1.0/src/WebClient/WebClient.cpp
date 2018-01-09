//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"
#include "boost/format.hpp"
#undef FullDebugMode
//---------------------------------------------------------------------------
USEFORM("Unit1.cpp", Form1);
USEFORM("Unit2.cpp", Form2);
USEFORM("SetupUnit.cpp", SetupForm);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
        Form1 = NULL;
        Form2 = NULL;
        Application->Initialize();
        Application->CreateForm(__classid(TForm1), &Form1);
         Application->CreateForm(__classid(TForm2), &Form2);
         Application->CreateForm(__classid(TSetupForm), &SetupForm);
         Application->Run();
        //delete Form2;
    }
    catch (Exception &exception)
    {
        if((Form1)&&(Form1->Log != NULL))
            Form1->Log->Write((boost::format("WinMain() Exception: %1%") % exception.Message.c_str()).str().c_str());
        Application->ShowException(&exception);
    }
    catch (std::exception& ex1)
    {
        Exception *ex = new Exception(ex1.what());
        Application->ShowException(ex);
        delete ex;
    }
    catch (...)
    {
        Exception *ex = new Exception("¬нутренн€€ ошибка");
        Application->ShowException(ex);
        delete ex;
    }
    return 0;
}
//---------------------------------------------------------------------------




