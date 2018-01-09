//---------------------------------------------------------------------------

#ifndef CSankyoCardReaderH
#define CSankyoCardReaderH

#include "BaseCardReader.h"
#include "LogClass.h"

//---------------------------------------------------------------------------
class CSankyoCardReader : public CBaseCardReader
{
protected:
    void Start();

public:
    CSankyoCardReader(unsigned int comPortNum, TLogClass* pLog = 0);
    ~CSankyoCardReader();

    int Initialize();
};
#endif
