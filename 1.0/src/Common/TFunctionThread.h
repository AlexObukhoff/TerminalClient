//---------------------------------------------------------------------------

#ifndef TFunctionThreadH
#define TFunctionThreadH

#include <Classes.hpp>
#include <boost/function.hpp>

template<typename T>
class TFunctionThread : public TThread
{
    private:
        T m_arg;

    protected:
        virtual void __fastcall Execute();

    public:
        boost::function1<void, T> execFunction;
        TFunctionThread(T);
        __fastcall virtual ~TFunctionThread();
};

//---------------------------------------------------------------------------
#endif

