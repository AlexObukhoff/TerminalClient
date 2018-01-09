//---------------------------------------------------------------------------

#ifndef CSankyoCardReaderThreadH
#define CSankyoCardReaderThreadH

#include "DeviceThread.h"
//---------------------------------------------------------------------------
// структура ответа от кардридера
typedef struct tagReply
{
    BYTE dataSize;
    BYTE type;
    BYTE cm;
    BYTE pm;

    union tagAN
    {
        struct tagPositive
        {
            BYTE st1;
            BYTE st0;
        }Positive;
        struct tagNegative
        {
            BYTE err1;
            BYTE err0;
        }Negative;
    }an;

    BYTE data[];
}Reply;

class CSankyoCardReaderThread : public TDeviceThread
{
private:

        void cmdGetStatus();
        void cmdGetData();
        Reply* m_pReply;
protected:
        unsigned char sendCmd();
        unsigned char recvAnswer(TSendType type);
        void prepareCmd();
        bool testCRC(unsigned short originalCRC);
        unsigned short getcrc(unsigned char *p, unsigned short n);
        unsigned short calccrc(unsigned short crc, unsigned short ch);
        void parseEAN();

        template<class T>
        bool readData(T* dest, unsigned int readTimeout = 0);
        bool readData(BYTE* buffer, unsigned int bufferSize, unsigned int readTimeout = 0);

        // from TDeviceClass
        virtual int SendCommand(BYTE* command, int commandSize, BYTE*& answer, int& answerSize, TSendType type, int mode = 0);
        virtual void __fastcall ProcessLoopCommand();

public:
      __fastcall CSankyoCardReaderThread();
      virtual __fastcall ~CSankyoCardReaderThread();
};
#endif
