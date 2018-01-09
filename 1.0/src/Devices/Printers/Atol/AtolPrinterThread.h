//---------------------------------------------------------------------------

#ifndef AtolPrinterThreadH
#define AtolPrinterThreadH
//---------------------------------------------------------------------------
#include "DeviceThread.h"

typedef unsigned char CommandResult;

const BYTE ENQ = 0x05;
const BYTE ACK = 0x06;
const BYTE STX = 0x02;
const BYTE ETX = 0x03;
const BYTE EOT = 0x04;
const BYTE NAK = 0x15;
const BYTE DLE = 0x10;

const clock_t TO1 = 500;
const clock_t TO2 = 2000;
const clock_t TO3 = 500;
const clock_t TO4 = 500;
const clock_t TO5 = 10000;
const clock_t TO6 = 500;
const clock_t TO7 = 500;
const clock_t TO8 = 1000;

const CommandResult CMDRES_OK = 0x00;
const CommandResult CMDRES_NO_ANSWER = 0x01;
const CommandResult CMDRES_NO_LINK = 0x02;

const UINT CICLE_COUNTER1 = 10;
const UINT CICLE_COUNTER2 = 100;

//
// описание сего алгоритма можно найти в документации к принтерам :)
//

class CAtolPrinterThread : public TDeviceThread
{
protected:
 bool readByte(BYTE* dest, clock_t readTimeout);
 bool transmitData(const BYTE* data, size_t dataSize, BYTE* dest = NULL, clock_t readTimeout = 0);
 bool recieveData(/*BYTE* buffer, size_t& bufferSize*/);
 bool checkCRC(/*const BYTE* buffer, size_t bufferSize, */BYTE crc);
 void maskOff();

 CommandResult requestConnection();
 CommandResult send(clock_t cmdTimeout);
 CommandResult recieve(/*BYTE* answer, size_t& answerSize*/);

 // derived from TDeviceThread
 int SendCommand(BYTE* Command, int CommandSize, BYTE*& Answer, int& AnswerSize, TSendType type, int mode = 0);
public:
 __fastcall CAtolPrinterThread();
};

#endif
