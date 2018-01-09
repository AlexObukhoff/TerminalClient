#ifndef DF_ControllerClassH
#define DF_ControllerClassH

#include <Classes.hpp>
#include <SyncObjs.hpp>
#include <time.h>

#include "LogClass.h"
#include "DF_DescrClass.h"
#include "DF_FinderClass.h"

class DF_ControllerClass : public TThread
{
private:
//members
    TLogClass* Log;//��� �����
    TList* COMPorts;//������ ������� ��������� ��� ������
    TList* Searchers;//������ �������� ������ ������������
    TList* FoundDevices;//������ ��������� ���������

    clock_t BeginSearching;

    bool SearchComplete;//���� ��������� �������� ������ �� ���� �������
    bool ExecuteComplete;//���� ���������� ������� Execute
    int MaxComPortNumber;//������������ ����� ��� �����
    DWORD SearchingTimeOut;//���������� ����� ������ ������������ � ������������

//properties

//functions
    void SearchCOMPorts();//���������� ��������� � ������� ��� �����
    void ClearObjects();//�������� ������ �������� ��� ������ ������ ������
    bool GetCompleteStatus();//������� ������ ���� �������� � ���� ��� ��������� ������ ���������� SearchComplete � true
    void StopAllChildThreads();//������������� ��� �������� ������ �������������� ����� ������������
    void StartAllChildThreads();//��������� ��� �������� ������ �� ����� ������������
    void GetInformation(int index);//�� ������ �������� ������ �� ������� �������� ������ � ��������� ���������� � ��������� ������������
    bool WaitForTerminated();//� ����������� ������� ����� ����� ���������� ������
    void WaitForCompleteStatus();//���������� ��� ������ �� ������� ���������� � ��� ��������� �������� ������ � ���� ��������� ������
    void AddFoundDevice(AnsiString name, int port, int type);//��������� ��������� ���������� � ������

protected:
//members

//properties

//functions
    void __fastcall Execute();//������� ��������� ������, � ������� ����� ������������ ����� � ����������� ������������ ������

public:
//members
    AnsiString ComPortsString;
    DF_DescrClass* DevicesInfo;//����� ��� ���������� � ��������� �����������

//properties
    __property bool CompleteStatus = {read = GetCompleteStatus};//���������� �������� �� ������� ������

//constructor & destructor
    __fastcall DF_ControllerClass();
    virtual __fastcall ~DF_ControllerClass();

//functions
    void StartSearching();//������ �������� ������
    void StopSearching();//��������� �������� ������
    bool WaitForSearchComplete();//��� ��������� �������� ������ ��� �������� ����������
    DF_device_info* GetFoundDevice();//���������� ��������� ��������� �� ����� �������� ���������� ���������� �� ������. ���������� ������� delete ����� ������������� ��� ������� �������� �� ������� ������ 
    bool GetSearchEnable(AnsiString DeviceName);//����������, �������� �� ����� ��� ������� ����������
};

#endif
