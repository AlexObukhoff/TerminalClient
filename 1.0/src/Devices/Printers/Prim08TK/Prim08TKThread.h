//---------------------------------------------------------------------------

#ifndef CPrim08TKThreadH
#define CPrim08TKThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"
#include "Global.h"

class CPrim08TKThread : public TDeviceThread
{
protected:
    union codes StatusErrorCode;

    bool NAKInAnswer(BYTE *Buffer);
    bool ACKInAnswer(BYTE *Buffer);
    bool CANInAnswer(BYTE *Buffer);
    bool TruncateAnswer(BYTE *Buffer, DWORD &Count);
    bool CompareBCC(BYTE *Buffer, unsigned int Count);
    BYTE PrepareAnswer(BYTE *Buffer, unsigned int& Count);
    bool CheckStatus(BYTE *Buffer, unsigned int BufferCount);
    int FindByteInBuffer(BYTE *Buffer, int BufferCount, BYTE Value, int Position);
    bool IsBufferEmpty(BYTE *Buffer, int size);
    BYTE GetBYTE(BYTE *Buffer, int index);
    WORD GetWORD(BYTE *Buffer, int index);
    BYTE CharToBYTE(BYTE ch);
    BYTE GetConstantStatusIndex();
    BYTE GetCurrentStatusIndex();
    BYTE GetResultCommandIndex();
    BYTE GetPrinterStatusIndex();

    virtual BYTE SendCommand(BYTE *Command, unsigned int CommandCount, BYTE *Answer, unsigned int& AnswerCount);
    virtual int SendCommand(BYTE* Command, int CommandSize, BYTE*& Answer, int& AnswerSize, TSendType type, int mode = 0);
public:

    __fastcall CPrim08TKThread();
    __fastcall ~CPrim08TKThread();
};
#endif
