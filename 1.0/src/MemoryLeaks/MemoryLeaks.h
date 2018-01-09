#pragma once

#ifdef INSPECT_MEMLEAKS
#include <time.h>

//------------------------------------------------------------------------------

void addMemAllocInfo(void* _Address, unsigned long _Size, char* _File, unsigned long _Line);
void removeMemAllocInfo(void* _AddrToRemove);
void dumpMemoryLeaks(void);
void writeToMemAllocInfoLog(const char* _Message);
//------------------------------------------------------------------------------

void* operator new(size_t _Size, char* _File, int _Line)
{
    size_t Size = _Size ? _Size : 1;

    void* Pointer = (void *)malloc(Size);
    if (Pointer)
    {
        addMemAllocInfo(Pointer, Size, _File, _Line);
    }

    return(Pointer);
};

void * __cdecl operator new[](size_t _Size, char* _File, int _Line)
{
    size_t Size = _Size ? _Size : 1;

    void* Pointer = (void *)malloc(Size);
    if (Pointer)
    {
        addMemAllocInfo(Pointer, Size, _File, _Line);
    }

    return(Pointer);
}

void operator delete(void* _Pointer)
{
  if (_Pointer)
  {
    removeMemAllocInfo(_Pointer);
    free(_Pointer);
  }
};

void operator delete[](void* _Pointer)
{
  if (_Pointer)
  {
    removeMemAllocInfo(_Pointer);
    free(_Pointer);
  }
};

#endif // INSPECT_MEMLEAKS


#ifdef INSPECT_MEMLEAKS
#define DEBUG_NEW new(__FILE__, __LINE__)
#define DUMP_MEMORY_LEAKS dumpMemoryLeaks();
#else
#define DEBUG_NEW new
#define DUMP_MEMORY_LEAKS
#endif

// Перегружаем глобальный оператор new
#define new DEBUG_NEW
