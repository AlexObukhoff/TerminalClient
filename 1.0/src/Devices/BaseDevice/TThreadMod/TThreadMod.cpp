//---------------------------------------------------------------------------
#include "process.h"

#pragma hdrstop

#include "TThreadMod.h"
#include "globals.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

void ThreadFunction(void* param);


TThreadMod::TThreadMod()
{
    Handle = NULL;
    Terminated = false;
}

TThreadMod::~TThreadMod()
{
    Terminate();
}

void ThreadFunction(void* param)
{
    try
    {
        if (param)
        {
            ((TThreadMod*)param)->Execute();
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, NULL);
    }
}

void TThreadMod::Execute()
{
}

void TThreadMod::Start()
{
    //ID++;
}

void TThreadMod::Resume()
{
    try
    {
        Handle = (HANDLE)_beginthread(ThreadFunction, 4096, this);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, NULL);
    }
}

void TThreadMod::Terminate()
{
    Terminated = true;
}
