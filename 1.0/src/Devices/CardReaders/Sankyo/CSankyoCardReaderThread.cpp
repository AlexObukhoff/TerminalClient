//---------------------------------------------------------------------------


#pragma hdrstop

#include "CSankyoCardReaderThread.h"
#include <boost/format.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)

const BYTE STX = 0xF2;
const BYTE ACK = 0x06;
const BYTE NAK = 0x15;
const BYTE DLE = 0x10;
const BYTE EOT = 0x04;

const int sendCmdRetires = 3;
const int waitACKRetires = 5;

const unsigned int timeoutNextCmd = 50;
const unsigned int timeoutACK = 300;
const unsigned int timeoutSTX = 2000;
const unsigned int timeoutData = 250;


__fastcall CSankyoCardReaderThread::CSankyoCardReaderThread()
    : TDeviceThread(true)
{
}

__fastcall CSankyoCardReaderThread::~CSankyoCardReaderThread()
{
    Log->Write("CSankyoCardReaderThread::~ctor()");
}

void __fastcall CSankyoCardReaderThread::ProcessLoopCommand()
{
    Log->Write("ProcessLoopCommand");
    while(true)
    {

        DeviceState->oldScannerDataValue = DeviceState->scannerDataValue;
        DeviceState->oldScannerDataType = DeviceState->scannerDataType;

        cmdGetStatus();
        if(m_pReply->type == 'P')
        {
            if(m_pReply->an.Positive.st0 != 0x30)
            {
                if(m_pReply->an.Positive.st0 & 1)
                {
                    // track3 exist
                    cmdGetData();
                    parseEAN();
                    DeviceState->OutStateCode = DSE_OK;
                    //Log->Write((boost::format("data: %1%. oldData: %2%") % DeviceState->scannerDataValue % DeviceState->oldScannerDataValue).str().c_str());
                    ChangeDeviceState();
                }
            }
            else
            {
                // данных нет, есть ли карта
                if((m_pReply->an.Positive.st1 & 1) && (m_pReply->an.Positive.st1 & 2))
                {
                    // card inserted, but no data
                    DeviceState->scannerDataValue = "";
                    DeviceState->oldScannerDataValue = "";
                    DeviceState->OutStateCode = DSE_NODATA;
                    //Log->Write("no data");
                    ChangeDeviceState();
                }
            }
        }
    }
}

int CSankyoCardReaderThread::SendCommand(BYTE* command, int commandSize, BYTE*& answer, int& answerSize, TSendType type, int mode)
{
    // все данные команды передаются через члена класса
    command;
    commandSize;
    answer;
    answerSize;

    AnswerSize = -1;
    unsigned char result = 2; // no link

    if(Port->Port != 0 && Port->Port != INVALID_HANDLE_VALUE)
    {
        // подготовить команду
        prepareCmd();

        // отправить команду
        result = sendCmd();
        if(result == 0)
            result = recvAnswer(type);

        m_pReply = (Reply*)(Answer + 2);
    }

    DeviceState->ExecCode = result;
    return AnswerSize;
}

// отправить команду утсройству
unsigned char CSankyoCardReaderThread::sendCmd()
{
    unsigned char result = 2;
    Sleep(timeoutNextCmd);  // между командами надо выдерживать интервал

    ::PurgeComm(Port->Port, PURGE_RXCLEAR | PURGE_TXCLEAR);
    for(int i = 0; i < sendCmdRetires; i++)
    {
        bool isACK = false;
        DWORD bytesWritten = 0;
        ::WriteFile(Port->Port, Command, CommandSize, &bytesWritten, 0);

        // ждем реакцию на команду
        for(int j = 0; j < waitACKRetires; j++)
        {
            BYTE ackByte = 0x00;
            bool readRes = readData<BYTE>(&ackByte, timeoutACK);

            if(readRes && ackByte != ACK && ackByte != NAK)
                continue;   // !!!! //

            if(ackByte == ACK)
                isACK = true;
                
            break;
        }

        if(isACK)
        {
            result = 0;
            break;
        }
    }

    return result;
}

// получить ответ
unsigned char CSankyoCardReaderThread::recvAnswer(TSendType type)
{
    unsigned char result = 2;
    AnswerSize = 0;
    memset(Answer, 0, DTBufferSize);

    do
    {
        BYTE stxByte = 0x00;
        // читаем STX
        if(readData<BYTE>(&stxByte, timeoutSTX))
        {
            if(stxByte == STX)
            {
                Answer[AnswerSize++] = STX;
                bool isSendingNAK = true;
                WORD len = 0;
                // читаем блину данных
                if(readData<WORD>(&len, timeoutData))
                {
                    memcpy(Answer + AnswerSize, &len, sizeof(len));
                    AnswerSize += sizeof(len);
                    // сами данные
                    if(readData(Answer + AnswerSize, Answer[2], timeoutData))
                    {
                        AnswerSize += Answer[2];
                        WORD crc = 0xFF;
                        // CRC
                        if(readData<WORD>(&crc, timeoutData))
                            isSendingNAK = testCRC(crc);
                    }
                }

                // если вылетаем по тайм-ауту или CRC не совпал, отсылаем NAK
                if(isSendingNAK)
                {
                    ::PurgeComm(Port->Port, PURGE_RXCLEAR | PURGE_TXCLEAR);
                    DWORD bytesWritten = 0;
                    ::WriteFile(Port->Port, &NAK, sizeof(NAK), &bytesWritten, 0);
                }
                else
                {
                    result = true;
                    break;
                }
            }
        }
    }while(type == RecieveAnswer);

    return result;
}

// обрамляем команду необходимыми аттрибутами
void CSankyoCardReaderThread::prepareCmd()
{
    BYTE tmp[0xFF];
    memset(tmp, 0, sizeof(tmp));
    memcpy(tmp, Command, CommandSize);

    size_t tmpSize = CommandSize;
    CommandSize = 0;
    Command[CommandSize++] = STX;
    Command[CommandSize++] = BYTE((tmpSize >> 8) & 0xFF);
    Command[CommandSize++] = BYTE(tmpSize & 0xFF);

    memcpy(Command + CommandSize, tmp, tmpSize);
    CommandSize += tmpSize;

    unsigned short crc = getcrc(Command, CommandSize);
    Command[CommandSize++] = BYTE((crc >> 8) & 0xFF);
    Command[CommandSize++] = BYTE(crc & 0xFF);
}

// проверить CRC. если совпали вернуть false!!!
bool CSankyoCardReaderThread::testCRC(unsigned short originalCRC)
{
    unsigned short crc = getcrc(Answer, AnswerSize);

    BYTE hi = BYTE((crc >> 8) & 0xFF);
    BYTE lo = BYTE(crc & 0xFF);

    return originalCRC ^ ((lo << 8) + hi);
}

// читать данные из порта
bool CSankyoCardReaderThread::readData(BYTE* buffer, unsigned int bufferSize, unsigned int readTimeout)
{
    bool result = false;
    clock_t endWait = clock() + readTimeout;

    do
    {
        DWORD lastError = 0;
        COMSTAT cs;
        ::ClearCommError(Port->Port, &lastError, &cs);
        if(cs.cbInQue >= bufferSize)
        {
            DWORD bytesRead = 0;
            if(::ReadFile(Port->Port, buffer, bufferSize, &bytesRead, 0))
            {
                result = true;
                break;
            }
        }
    }while(clock() < endWait || readTimeout == 0);
    return result;
}

// читать данные из порта
template<class T>
bool CSankyoCardReaderThread::readData(T* dest, unsigned int readTimeout)
{
    bool result = false;
    clock_t endWait = clock() + readTimeout;

    do
    {
        DWORD lastError = 0;
        COMSTAT cs;
        ::ClearCommError(Port->Port, &lastError, &cs);
        if(cs.cbInQue >= sizeof(T))
        {
            DWORD bytesRead = 0;
            if(::ReadFile(Port->Port, dest, sizeof(T), &bytesRead, 0))
            {
                result = true;
                break;
            }
        }
    }while(clock() < endWait || readTimeout == 0);
    return result;
}

// рассчитать CRC
unsigned short CSankyoCardReaderThread::calccrc(unsigned short crc, unsigned short ch)
{
    unsigned short i;
    ch <<= 8;
    for(i = 8; i > 0; i--)
    {
        if((ch ^ crc) & 0x8000)
        {
            crc = (crc << 1) ^ 0x1021;
        }
        else
        {
            crc <<= 1;
        }
        ch <<= 1;
    }
    return crc;
}

unsigned short CSankyoCardReaderThread::getcrc(unsigned char *p, unsigned short n)
{
    unsigned char ch;
    unsigned short i;
    unsigned short crc(0x0000);
    for(i = 0; i < n; i++)
    {
        ch = *p++;
        crc = calccrc(crc, (unsigned short)ch);
    }
    return crc;
}

void CSankyoCardReaderThread::cmdGetStatus()
{
    BYTE cmdStatus_InquireStatus[] = {'C', 0x31, 0x30};
    memcpy(Command, cmdStatus_InquireStatus, sizeof(cmdStatus_InquireStatus));
    CommandSize = sizeof(cmdStatus_InquireStatus);

    // 2 strings below keep compiler happy
    int answSize = 0;
    BYTE* answ = 0;
    SendCommand(0, 0, answ, answSize, RecieveAnswer);
}

void CSankyoCardReaderThread::cmdGetData()
{
    BYTE cmdRead_ISO3[] = {'C', 0x32, 0x33};
    memcpy(Command, cmdRead_ISO3, sizeof(cmdRead_ISO3));
    CommandSize = sizeof(cmdRead_ISO3);

    // 2 strings below keep compiler happy
    int answSize = 0;
    BYTE* answ = 0;
    SendCommand(0, 0, answ, answSize, RecieveAnswer);
}

// получить EAN код
void CSankyoCardReaderThread::parseEAN()
{
    DeviceState->scannerDataType = 0;
    DeviceState->scannerDataValue = reinterpret_cast<char*>(m_pReply->data);

    if(m_pReply->dataSize == 0x29)
    {
        // проверям на формат xxx=xxx
        if(m_pReply->data[18] == '=')
        {
            DeviceState->scannerDataType = 1;
            BYTE EAN[14];
            memset(EAN, 0, sizeof(EAN));

            EAN[0] = '2';
            EAN[1] = '9';
            EAN[2] = '8';

            // отбрасываем от ПАН (первая часть буфера, до знака равно)
            // служебные два байта (обычно всегда 99), и следующие 6 цифп
            // ccReply->data + 2 + 6
            // и последнюю цифру, длина ПАН 18 цифр (8 уже отбросили)
            // 18 - (2 + 6) - 1 = 10 - 1
            memcpy(EAN + 3, m_pReply->data + 2 + 6, 10 - 1);	// отбрасывание от ПАН первых 6 и последнего числа

            int oddSum = 0;
            int evenSum = 0;
            for(int i = 0; i < 13 - 1; i++)
            {
                if(i % 2 == 0)
                    evenSum += EAN[i] - 0x30;
                else
                    oddSum += EAN[i] - 0x30;
            }

            int ost = (evenSum + oddSum * 3) % 10;

            if(ost != 0)
                EAN[12] = BYTE(ost + 0x30);
            else
                EAN[12] = 0x30;

            DeviceState->scannerDataValue = reinterpret_cast<char*>(EAN);
        }
    }
}

