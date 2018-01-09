//---------------------------------------------------------------------------


#pragma hdrstop

#include "AtolPrinterThread.h"
#include <boost/format.hpp>

#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall CAtolPrinterThread::CAtolPrinterThread() : TDeviceThread(true, false)
{
}

int CAtolPrinterThread::SendCommand(BYTE* Command, int CommandSize, BYTE*& Answer, int& AnswerSize, TSendType type, int mode)
{
    Command;
    CommandSize;
    Answer;
    AnswerSize;
    type;

    AnswerSize = -1;
    CommandResult result = CMDRES_NO_LINK;
    if(Port->Port != NULL && Port->Port != INVALID_HANDLE_VALUE)
    {
        result = requestConnection();

        if(result == CMDRES_OK)
        {
            result = send(20000); // тайм-аут 20 секунд
            if(result == CMDRES_OK)
            result = recieve();

            // избавляемся от символов маскирования
            maskOff();
        }
    }

    DeviceState->ExecCode = result;
    return AnswerSize;
}

////////////////////////////////////////////////////////////////////////////////
// protected methods
////////////////////////////////////////////////////////////////////////////////

//запрос соединения
CommandResult CAtolPrinterThread::requestConnection()
{
    CommandResult result = CMDRES_NO_LINK;

    bool isACK = false;

    for(UINT frc = 0; frc < CICLE_COUNTER2; ++frc)
    {
        UINT rc = 0;
        for(rc = 0; rc < 5; ++rc)
        {
            BYTE answer = 0x00;
            PurgeComm(Port->Port, PURGE_RXCLEAR | PURGE_TXCLEAR);

            if(transmitData(&ENQ, sizeof(ENQ), &answer, TO1))
            {
                if(answer == NAK)
                {
                    Sleep(TO1);
                }
                else if(answer == ACK)
                {
                    PurgeComm(Port->Port, PURGE_RXCLEAR | PURGE_TXCLEAR);

                    isACK = true;
                    result = CMDRES_OK;
                    break;
                }
                else if(answer == ENQ)
                {
                    Sleep(TO7);
                    break;
                }
                else
                {
                    break;
                }
            }
        } // for(rc)
        if(rc == 5 || isACK)
            break;
    } // for(frc)

    if(!isACK)
        transmitData(&EOT, sizeof(EOT));

    return result;
}

// послать команду на выполнение
CommandResult CAtolPrinterThread::send(clock_t cmdTimeout)
{
    CommandResult result = CMDRES_NO_ANSWER;

    bool exitInner = false;
    bool isACK = false;

    for(UINT rc = 0; rc < CICLE_COUNTER1; ++rc)
    {
        BYTE answer = 0x00;
        if(transmitData(Command, CommandSize, &answer, TO3))
        {
            if(answer == ENQ && rc > 1)
            {
                exitInner = true;
                break;
            }
            else if(answer == ACK)
            {
                PurgeComm(Port->Port, PURGE_RXCLEAR | PURGE_TXCLEAR);
                exitInner = true;
                isACK = true;
                break;
            }
        }
    }

    if(exitInner)
    {
        result = CMDRES_OK;

        if(isACK)
        {
            transmitData(&EOT, sizeof(EOT));
            for(UINT rc = 0; rc < CICLE_COUNTER2; ++rc)
            {
                BYTE answer = 0x00;
                if(readByte(&answer, cmdTimeout))
                {
                    if(answer == ENQ)
                    {
                        PurgeComm(Port->Port, PURGE_RXCLEAR | PURGE_TXCLEAR);
                        break;
                    }
                }
                else
                {
                    result = CMDRES_NO_ANSWER;
                    break;
                }
            } // for(rc)
        }
    }
    else
    {
        transmitData(&EOT, sizeof(EOT));
    }

    return result;
}

// получить ответ принтера
CommandResult CAtolPrinterThread::recieve()
{
    CommandResult result = CMDRES_NO_ANSWER;

    bool isSendingACK = true;
    for(UINT frc = 0; frc < CICLE_COUNTER1; ++frc)
    {
        if(isSendingACK)
        {
            isSendingACK = false;
            transmitData(&ACK, sizeof(ACK));
        }

        bool isSTX = false;
        bool exitInner = false;

        for(UINT rc = 0; rc < CICLE_COUNTER2; ++rc)
        {
            BYTE readValue = 0x00;
            if(readByte(&readValue, TO2))
            {
                if(readValue == ENQ)
                {
                    isSendingACK = true;
                    exitInner = true;
                    break;
                }
                else if(readValue == STX)
                {
                    isSTX = true;
                    exitInner = true;
                    break;
                }
            }
            else
            {
                result = CMDRES_NO_ANSWER;
                break;
            }
        }

        if(!exitInner)
            break;

        // получили начало ответа, получаем остальную часть
        if(isSTX)
        {
            if(recieveData())
            {
                BYTE crc = 0x00;
                if(readByte(&crc, TO6))
                {
                    if(checkCRC(/*Answer, AnswerSize, */crc))
                    {
                        BYTE readValue = 0x00;
                        transmitData(&ACK, sizeof(ACK), &readValue, TO4);
                        if(readValue == EOT || readValue == 0x00)
                        {
                            // readValue == 0x00 - нет ответа, и это нормально
                            result = CMDRES_OK;
                            break;
                        }
                        else if(readValue == STX)
                        {
                            // ничего не делаем, повторяем цикл приема
                        }
                        else
                        {
                            if(!readByte(&readValue, TO6))
                            {
                                result = CMDRES_OK;
                                break;
                            }
                        }
                    }
                    else
                    {
                        transmitData(&NAK, sizeof(NAK));
                    }
                }
            }
        } // if(isSTX)
    } // for(frc)

    return result;
}

// проверить crc
bool CAtolPrinterThread::checkCRC(/*const BYTE* buffer, size_t bufferSize, */BYTE crc)
{
    BYTE calculatedCRC = 0x03; // инициализируем ETX так как мы не читаем его

    for(size_t i = 0; i < (size_t)AnswerSize; i++)
        calculatedCRC ^= Answer[i];

    // используем факт что  - A XOR A = 0
    return bool(calculatedCRC ^ crc) ^ true;
}

// получить данные
bool CAtolPrinterThread::recieveData(/*BYTE* buffer, size_t& bufferSize*/)
{
    bool result = false;
    bool isETX = false;
    bool isDLE = false;
    AnswerSize = 0;

    memset(Answer, 0, /*sizeof(Answer)*/DTBufferSize);

    while(AnswerSize < DTBufferSize)
    {
        BYTE readValue = 0xFF;
        if(readByte(&readValue, TO6))
        {
            if(isDLE)
            {
                isDLE = false;
            }
            else
            {
                if(readValue == DLE)
                {
                    isDLE = true;
                }
                else if(readValue == ETX)
                {
                    isETX = true;
                    break;
                }
            }

            Answer[AnswerSize] = readValue;
            AnswerSize++;

        }
        else
        {
            break;
        }
    }

    if(isETX)
        result = true;

    // на выходе в буфере будет лежать только переданные данные, без STX, ETX, CRC, но с замаскированными байтами
    return result;
}

// передать данные
bool CAtolPrinterThread::transmitData(const BYTE *data, size_t dataSize, BYTE *dest, clock_t readTimeout)
{
    DWORD bytesWritten = 0;
    WriteFile(Port->Port, data, (DWORD)dataSize, &bytesWritten, NULL);

    return readByte(dest, readTimeout);
}

// считать байт
bool CAtolPrinterThread::readByte(BYTE *dest, clock_t readTimeout)
{
    bool result = false;
    clock_t startTimeStamp = clock();

    while(clock() - startTimeStamp < readTimeout)
    {
        DWORD lastError = 0;
        COMSTAT cs;
        ClearCommError(Port->Port, &lastError, &cs);
        if(cs.cbInQue > 0)
        {
            DWORD bytesRead = 0;
            if(ReadFile(Port->Port, dest, 1, &bytesRead, NULL))
            {
                if(bytesRead == 1)
                {
                    result = true;
                    break;
                }
            }
        }
    }

    return result;
}

void CAtolPrinterThread::maskOff()
{
    int newPos = 0;
    bool isDLE = false;
    for(int i = 0; i < AnswerSize; i++)
    {
        if(isDLE)
        {
            isDLE = false;
        }
        else
        {
            if(Answer[i] == DLE)
            {
                isDLE = true;
                continue;
            }
        }

        Answer[newPos] = Answer[i];
        newPos++;
    }

    AnswerSize = newPos;
}

