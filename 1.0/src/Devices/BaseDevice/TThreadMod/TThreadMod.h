//---------------------------------------------------------------------------
#ifndef TThreadModH
#define TThreadModH
#include <Classes.hpp>

//---------------------------------------------------------------------------
class TThreadMod
{
protected:

public:
    HANDLE Handle;
    bool FreeOnTerminate;
    bool Terminated;
    //static int ID;

    virtual void Execute();
    virtual void Start();
    virtual void Resume();
    virtual void Terminate();

    TThreadMod();
    virtual ~TThreadMod();
};

#endif
