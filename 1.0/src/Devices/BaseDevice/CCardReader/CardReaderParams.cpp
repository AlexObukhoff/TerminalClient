#include <vcl.h>
#include <stdio.h>
//---------------------------------------------------------------------------
#pragma hdrstop

#include "CardReaderParams.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

//==============================================================================
_init_info::_init_info() :_info()
{
    ClearMembers();
    Status = cs_Wait;
}

void _init_info::ClearMembers()
{
    nSystemCode = 0;
    nDeviceCode = 0;
    nCom = 0;
}

//==============================================================================
_findcard_info::_findcard_info() :_info()
{
    ClearMembers();
    Status = cs_Wait;
}

void _findcard_info::ClearMembers()
{
    CardFound = false;
    memset(psCardNum,0,2048);
}

//==============================================================================
_getmenu_info::_getmenu_info() :_info()
{
    ClearMembers();
    Status = cs_Wait;
}

_getmenu_info::~_getmenu_info()
{
}

void _getmenu_info::ClearMembers()
{
    pnOrderId = 0;
    memset(psCardNum,0,2048);

    for(int i=0; i<512; i++)
    {
        pAMenu[i].nID = 0;
        pAMenu[i].nParentID = 0;
        memset(pAMenu[i].sText,0,100);
        pAMenu[i].nPriceInRub = 0;
        pAMenu[i].nHasChildren = 0;
        memset(pAMenu[i].nReserved,0,16);
    }

    pnItemsNum = 0;
    memset(psCardStatus,0,2048);
}

//==============================================================================
_writecard_info::_writecard_info() :_info()
{
    ClearMembers();
    Status = cs_Wait;
}

void _writecard_info::ClearMembers()
{
    pnOrderId = 0;
    memset(psCardNum,0,2048);
    nMenuItemID = 0;
    nAcceptedInRub = 0;
    nAuthoriseCode = 0;
    memset(psCardStatus,0,2048);
}



