//---------------------------------------------------------------------------

#ifndef CStarTSP600ThreadH
#define CStarTSP600ThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"
//#include "Global.h"

class CStarTSP600Thread : public TDeviceThread
{
protected:
    bool checkAnswer(/*BYTE *buffer, unsigned int count*/);
    int SendCommand(BYTE* Command, int CommandSize, BYTE*& Answer, int& AnswerSize, TSendType type, int mode = 0);
    void setCOMTimeOuts();
    DWORD writeCmd();
    BYTE readByteFromCOM();
    DWORD readAnswer();
public:
    __fastcall CStarTSP600Thread();
};
#endif
