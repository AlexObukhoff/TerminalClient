//---------------------------------------------------------------------------

#ifndef ScanerThreadH
#define ScanerThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"
/*
const BYTE STX = 0x02;
const BYTE ETX = 0x03;
const BYTE ACK = 0x06;
*/
class CScannerThread : public TDeviceThread
{
private:
        void logError(std::string desc);
protected:
        BYTE codeType;
        std::string codeValue;
        BYTE unknownByte;

        virtual void __fastcall ProcessLoopCommand();
        /*
        virtual void PoolingLoop();

        bool acceptData(unsigned int dataLength);
        void sendACK();
        */

public:
        __fastcall CScannerThread();
        virtual __fastcall ~CScannerThread();
};
#endif
