#pragma once

#include <list>
//------------------------------------------------------------------------------

const AnsiString c_strMemAllocInfoLogName = "Memleaks";
//------------------------------------------------------------------------------

typedef struct
{
    unsigned long Address;
    unsigned long Size;
    char File[128];
    unsigned long Line;
} TMemAllocInfo, *PTMemAllocInfo;

typedef std::list<TMemAllocInfo*> TMemAllocInfoList, *PTMemAllocInfoList;
//------------------------------------------------------------------------------