#pragma once

#include <stdio.h>
#include <time.h>
#include <SyncObjs.hpp>
//------------------------------------------------------------------------------
#include <boost\format.hpp>
#include <boost\algorithm\string.hpp>
//------------------------------------------------------------------------------
#include "MemoryLeaksTypes.h"
//------------------------------------------------------------------------------

#ifdef INSPECT_MEMLEAKS

// Проинициализируем здесь всё, что не нужно отслеживать, а потом заинклудим "MemoryLeaks.h"

// Критическая секция для работы со списком выделенных блоков памяти
TCriticalSection* MemAllocCriticalSection = new TCriticalSection();
// Список выделенных блоков памяти
static PTMemAllocInfoList MemAllocInfoList = new TMemAllocInfoList();
// Хэндл файла с логом о мем ликах, стандартный использовать не можем, потому что его тоже нужно инспектировать
static FILE* MemAllocLogFileHandle = NULL;
//------------------------------------------------------------------------------

void writeToMemAllocInfoLog(const char* _Message)
{
    SYSTEMTIME CurTime;
    GetSystemTime(&CurTime);

    ForceDirectories(".\\logs");

    if (!MemAllocLogFileHandle)
    {
        std::string LogName = (boost::format("logs\\%04i-%02i-%02i - %d.log")
            % CurTime.wYear
            % CurTime.wMonth
            % CurTime.wDay
            % c_strMemAllocInfoLogName.c_str()).str();
        MemAllocLogFileHandle = fopen(LogName.c_str(), "a+");

        if (!MemAllocLogFileHandle)
            return;

        std::string LogMessage = (boost::format("%04i.%02i.%02i %02i:%02i:%02i.%03i Starting new log session\n")
            % CurTime.wYear
            % CurTime.wMonth
            % CurTime.wDay
            % CurTime.wHour
            % CurTime.wMinute
            % CurTime.wSecond
            % CurTime.wMilliseconds).str();
        fwrite(LogMessage.c_str(), sizeof(char), LogMessage.size(), MemAllocLogFileHandle);
    }

    std::string LogMessage = (boost::format("%04i.%02i.%02i %02i:%02i:%02i.%03i %d\n")
        % CurTime.wYear
        % CurTime.wMonth
        % CurTime.wDay
        % CurTime.wHour
        % CurTime.wMinute
        % CurTime.wSecond
        % CurTime.wMilliseconds
        % _Message).str();
    fwrite(LogMessage.c_str(), sizeof(char), LogMessage.size(), MemAllocLogFileHandle);

    fflush(MemAllocLogFileHandle);
}
//------------------------------------------------------------------------------

#include "MemoryLeaks.h"
//------------------------------------------------------------------------------

void addMemAllocInfo(void* _Address, unsigned long _Size, char* _File, unsigned long _Line)
{
    MemAllocCriticalSection->Enter();

    try
    {
        PTMemAllocInfo pNewMemAllocInfo = (PTMemAllocInfo)malloc(sizeof(TMemAllocInfo));
        if (pNewMemAllocInfo)
        {
            pNewMemAllocInfo->Address = reinterpret_cast<unsigned long>(_Address);
            pNewMemAllocInfo->Size = _Size;
            strcpy(pNewMemAllocInfo->File, _File);
            pNewMemAllocInfo->Line = _Line;

            MemAllocInfoList->push_front(pNewMemAllocInfo);
        }
    }
    __finally
    {
        MemAllocCriticalSection->Leave();
    }
}
//------------------------------------------------------------------------------

void removeMemAllocInfo(void* _AddrToRemove)
{
    MemAllocCriticalSection->Enter();

    try
    {
        TMemAllocInfoList::iterator it;
        for (it = MemAllocInfoList->begin(); it != MemAllocInfoList->end(); it++)
        {
            if ((*it)->Address == reinterpret_cast<unsigned long>(_AddrToRemove))
            {
                MemAllocInfoList->erase(it);
                break;
            }
        }
    }
    __finally
    {
        MemAllocCriticalSection->Leave();
    }
}
//------------------------------------------------------------------------------

void dumpMemoryLeaks()
{
    MemAllocCriticalSection->Enter();

    try
    {
        writeToMemAllocInfoLog("Dumping memory allocation info...");

        TMemAllocInfoList::iterator it;
        for (it = MemAllocInfoList->begin(); it != MemAllocInfoList->end(); it++)
        {
            writeToMemAllocInfoLog(" ");

            writeToMemAllocInfoLog((boost::format("Address: 0x%X") % (*it)->Address).str().c_str());
            writeToMemAllocInfoLog((boost::format("Size: %1%") % (*it)->Size).str().c_str());
            writeToMemAllocInfoLog((boost::format("File: %1%") % (*it)->File).str().c_str());
            writeToMemAllocInfoLog((boost::format("Line: %1%") % (*it)->Line).str().c_str());
        }
    }
    __finally
    {
        MemAllocCriticalSection->Leave();
    }
}
//------------------------------------------------------------------------------

#endif INSPECT_MEMLEAKS
