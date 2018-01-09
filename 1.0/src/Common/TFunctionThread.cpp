//---------------------------------------------------------------------------
#pragma hdrstop
#include "TFunctionThread.h"

template<typename T>
TFunctionThread<T>::TFunctionThread(T arg): TThread(true)
{
    m_arg = arg;
}

//---------------------------------------------------------------------------

template<typename T>
__fastcall TFunctionThread<T>::~TFunctionThread()
{

}

//---------------------------------------------------------------------------

template<typename T>
void __fastcall TFunctionThread<T>::Execute()
{
    execFunction(m_arg);
}
//---------------------------------------------------------------------------

//#pragma package(smart_init)

