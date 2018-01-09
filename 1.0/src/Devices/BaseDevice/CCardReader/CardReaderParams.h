//---------------------------------------------------------------------------

#ifndef CardReaderParamsH
#define CardReaderParamsH

#include "DeviceThread.h"

typedef struct
{
    long nID;                //��� ������ ����
    long nParentID;        //��� ������ ����-��������
    char sText[100];        //����� ������ ����
    short nPriceInRub;        //��������� ����������� �� ����� ������ �                                         ������
    short nHasChildren;        //���� ���� �� � ����� ������ �������� ��������
    char  nReserved[16];        //��������������� ��� ��������
} ASKOPM_Menu;

class _init_info : public _info
{
public:
    virtual void ClearMembers();
    _init_info();
    virtual ~_init_info(){};

    long nSystemCode;
    long nDeviceCode;
    short nCom;
};

class _findcard_info : public _info
{
public:
    virtual void ClearMembers();
    _findcard_info();
    virtual ~_findcard_info(){};

    char psCardNum[2048];
    bool CardFound;
};

class _getmenu_info : public _info
{
public:
    virtual void ClearMembers();
    _getmenu_info();
    virtual ~_getmenu_info();

    long pnOrderId;
    char psCardNum[2048];
    ASKOPM_Menu pAMenu[512];
    short pnItemsNum;
    char psCardStatus[2048];
};

class _writecard_info : public _info
{
public:
    virtual void ClearMembers();
    _writecard_info();
    virtual ~_writecard_info(){};

    long pnOrderId;
    char psCardNum[2048];
    long nMenuItemID;
    short nAcceptedInRub;
    long nAuthoriseCode;
    char psCardStatus[2048];
};

#endif
