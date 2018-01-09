#pragma hdrstop

#include "DF_ControllerClass.h"

#pragma package(smart_init)

__fastcall DF_ControllerClass::DF_ControllerClass() : TThread(true)
{
    DevicesInfo = new DF_DescrClass();
    Searchers = new TList();
    COMPorts = new TList();
    FoundDevices = new TList();

    MaxComPortNumber = 20;
    SearchComplete = true;
    //ExecuteComplete = true;
    ExecuteComplete = false;
    SearchingTimeOut = 1000*60*10;//10 ����� ��������� �� �����

    //Log = new TLogClass("DeviceController","",(ErrorCallbackFunc)NULL,"finder_log");
    Log = new TLogClass("DeviceController");
    Log->Write("Create Device Searching object.");
}

__fastcall DF_ControllerClass::~DF_ControllerClass()
{
    if (ExecuteComplete == false)
    {
        StopSearching();
        if (WaitForTerminated() == false)
            TerminateThread((HANDLE)this->Handle,0);
    }

    //�������� ������� ���������� � ��������� ������������
    if (DevicesInfo)
        delete DevicesInfo;

    //�������� ���� �������� ������ ������������ � ������ ��������
    ClearObjects();
    delete Searchers;

    //�������� ������ ��������� ������
    delete COMPorts;

    //������� ������ ��������� ���������
    if (FoundDevices)
    {
        for(int i=0; i<FoundDevices->Count; i++)
        {
            DF_device_info* device = (DF_device_info*)FoundDevices->Items[i];
            delete device;
        }
        delete FoundDevices;
    }

    //�������� ������� ����
    Log->Write("Destroy Device Searching object.");
    if (Log)  delete Log;
    Log = NULL;
}

void DF_ControllerClass::SearchCOMPorts()
{
    COMPorts->Clear();
    if (Log) Log->Write("Have been found COM ports: ");

    ComPortsString = "";
    AnsiString str;
    //create list of availible numbers of COM ports
    for(int i=1; i<=MaxComPortNumber; i++)
    {
        if (Terminated) return;
        AnsiString PortName = "\\\\.\\COM"+AnsiString(i);
        //��������� ����
        HANDLE Port = CreateFile(PortName.c_str(),
                          GENERIC_READ | GENERIC_WRITE,
                          0,
                          NULL,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);
        // ���� ������ - �������
        if (Port == INVALID_HANDLE_VALUE)
            continue;

        if ((Port != NULL)&&(Port != INVALID_HANDLE_VALUE))
        CloseHandle(Port);
        COMPorts->Add((void*)i);
        str += "COM"+AnsiString(i)+" ";
        if (Log) Log->Append(str);
    }

    ComPortsString = str;
    if (ComPortsString.IsEmpty())
        ComPortsString = "no ports";
}

void DF_ControllerClass::StartSearching()
{
    SearchComplete = false;
    ExecuteComplete = false;
    Resume();
}

void DF_ControllerClass::StopSearching()
{
    //������������� ��� ���������� ������
    StopAllChildThreads();
    //������������� ����������� �����
    Terminate();
}

void DF_ControllerClass::StopAllChildThreads()
{
    //������������� ��� ���������� ������
    for(int i=0; i<Searchers->Count; i++)
    {
        if (Searchers->Items[i] != NULL)
        {
            DF_FinderClass* device = (DF_FinderClass*)Searchers->Items[i];
            device->StopSearching();
        }
    }
}

void DF_ControllerClass::StartAllChildThreads()
{
    //������������� ��� ���������� ������
    for(int i=0; i<Searchers->Count; i++)
    {
        if (Searchers->Items[i] != NULL)
        {
            DF_FinderClass* device = (DF_FinderClass*)Searchers->Items[i];
            device->StartSearching();
        }
    }
}

void DF_ControllerClass::ClearObjects()
{
    //�������� ���� �������� ������ ������������ � ������ ��������
    for(int i=0; i<Searchers->Count; i++)
    {
        if (Searchers->Items[i] != NULL)
        {
            DF_FinderClass* device = (DF_FinderClass*)Searchers->Items[i];
            delete device;
        }
    }
    Searchers->Clear();
}

void __fastcall DF_ControllerClass::Execute()
{
    try
    {
        try
        {
            SearchComplete = false;
            ExecuteComplete = false;
            Log->Write("Begin Device Searching.");

            BeginSearching = clock();

            if (Terminated) return;

            //������� ������ �������� ��� ������ ������
            ClearObjects();

            //���������� ��� ����� ������������� � �������
            SearchCOMPorts();
            if (Terminated) return;

            //������ ������� �� ������ ������������ ��� ������� ���������� �����
            for(int i=0; i<COMPorts->Count; i++)
            {
                if (Terminated) return;
                int ComPortNumber = (int)COMPorts->Items[i];
                DF_FinderClass* device = new DF_FinderClass(ComPortNumber);

                //���� ����� ����� ������ ����������� ����������, ��������� ����� ����������
                if (DevicesInfo->SearchDeviceName.IsEmpty() == false)
                {
                    if (Log) Log->Write("Search device by name ["+DevicesInfo->SearchDeviceName+"]");
                }
                else
                if (DevicesInfo->SearchType != ht_Unknown)
                {
                    if (Log) Log->Write("Search device by type ["+AnsiString(DevicesInfo->SearchType)+"]");
                }
                device->DeviceInfo->SearchDeviceName = DevicesInfo->SearchDeviceName;
                device->DeviceInfo->SearchType = DevicesInfo->SearchType;

                device->StartSearching();
                Searchers->Add((void*)device);
            }
            if (Terminated) return;

            //��� ��������� ������
            WaitForCompleteStatus();
        }
        __finally
        {
            ExecuteComplete = true;
            if (Log) Log->Write("Finish Device Searching.");
        }
    }
    catch(Exception& error)
    {
      if (Log) Log->Write("DF_ControllerClass::Execute() Exception: "+ AnsiString(error.Message));
    }
}

bool DF_ControllerClass::GetCompleteStatus()
{
    try
    {
        if (ExecuteComplete)
            return true;

        if (Searchers->Count > 0)
            SearchComplete = true;
        else
            SearchComplete = false;
        for(int i=0; i<Searchers->Count; i++)
        {
            if (Terminated) return SearchComplete = false;
            if (Searchers->Items[i] != NULL)
            {
                DF_FinderClass* device = (DF_FinderClass*)Searchers->Items[i];
                if (device->GetCompleteStatus() == false)
                    SearchComplete = false;
                else
                    GetInformation(i);
            }
        }
        return SearchComplete;
    }
    catch(Exception& error)
    {
        if (Log) Log->Write("DF_ControllerClass::GetCompleteStatus() Exception: "+ AnsiString(error.Message));
        return SearchComplete = false;
    }
}

void DF_ControllerClass::GetInformation(int index)
{
    DF_FinderClass* device = (DF_FinderClass*)Searchers->Items[index];

    if (device->GetCompleteStatus())
    {
        for(int i=0; i<=2; i++)
        {
            //���������� ����� ���������� �������
            if((device->DeviceInfo->Validator[i]->Determinated)&&(device->DeviceInfo->Validator[i]->Read == false))
            {
                //���������� ��������� ������ � ������ ����������
                for(int j=0; j<=2; i++)
                {
                    if(DevicesInfo->Validator[j]->Determinated == false)
                    {
                        //����� � ��������� ������
                        device->DeviceInfo->Validator[i]->Read = true;
                        DevicesInfo->Validator[j]->DeviceName = device->DeviceInfo->Validator[i]->DeviceName;
                        DevicesInfo->Validator[j]->Determinated = true;
                        DevicesInfo->Validator[j]->PortNumber = device->PortNumber;
                        Log->Write("������ ������������� ["+device->DeviceInfo->Validator[i]->DeviceName+"] �� ����� ���"+device->PortNumber+", ������� � ������["+AnsiString(j)+"]");
                        AddFoundDevice(device->DeviceInfo->Validator[i]->DeviceName,device->PortNumber,device->DeviceInfo->Validator[i]->Type);
                        break;
                    }
                }
                break;
            }
        }

        for(int i=0; i<=2; i++)
        {
            //���������� ����� ���������� �������
            if((device->DeviceInfo->CoinAcceptor[i]->Determinated)&&(device->DeviceInfo->CoinAcceptor[i]->Read == false))
            {
                //���������� ��������� ������ � ������ ����������
                for(int j=0; j<=2; i++)
                {
                    if(DevicesInfo->CoinAcceptor[j]->Determinated == false)
                    {
                        //����� � ��������� ������
                        device->DeviceInfo->CoinAcceptor[i]->Read = true;
                        DevicesInfo->CoinAcceptor[j]->DeviceName = device->DeviceInfo->CoinAcceptor[i]->DeviceName;
                        DevicesInfo->CoinAcceptor[j]->Determinated = true;
                        DevicesInfo->CoinAcceptor[j]->PortNumber = device->PortNumber;
                        Log->Write("������ ������������� ["+device->DeviceInfo->CoinAcceptor[i]->DeviceName+"] �� ����� ���"+device->PortNumber+", ������� � ������["+AnsiString(j)+"]");
                        AddFoundDevice(device->DeviceInfo->CoinAcceptor[i]->DeviceName,device->PortNumber,device->DeviceInfo->CoinAcceptor[i]->Type);
                        break;
                    }
                }
                break;
            }
        }

        if((device->DeviceInfo->Printer->Determinated)&&(device->DeviceInfo->Printer->Read == false))
        {
            device->DeviceInfo->Printer->Read = true;
            DevicesInfo->Printer->DeviceName = device->DeviceInfo->Printer->DeviceName;
            DevicesInfo->Printer->Determinated = true;
            DevicesInfo->Printer->PortNumber = device->PortNumber;
            AddFoundDevice(device->DeviceInfo->Printer->DeviceName,device->PortNumber,device->DeviceInfo->Printer->Type);
            Log->Write("������ ������� ["+device->DeviceInfo->Printer->DeviceName+"] �� ����� ���"+device->PortNumber);
        }

        if((device->DeviceInfo->WatchDog->Determinated)&&(device->DeviceInfo->WatchDog->Read == false))
        {
            device->DeviceInfo->WatchDog->Read = true;
            DevicesInfo->WatchDog->DeviceName = device->DeviceInfo->WatchDog->DeviceName;
            DevicesInfo->WatchDog->Determinated = true;
            DevicesInfo->WatchDog->PortNumber = device->PortNumber;
            AddFoundDevice(device->DeviceInfo->WatchDog->DeviceName,device->PortNumber,device->DeviceInfo->WatchDog->Type);
            Log->Write("������ ���������� ������ ["+device->DeviceInfo->WatchDog->DeviceName+"] �� ����� ���"+device->PortNumber);
        }

        if((device->DeviceInfo->CardReader->Determinated)&&(device->DeviceInfo->CardReader->Read == false))
        {
            device->DeviceInfo->CardReader->Read = true;
            DevicesInfo->CardReader->DeviceName = device->DeviceInfo->CardReader->DeviceName;
            DevicesInfo->CardReader->Determinated = true;
            DevicesInfo->CardReader->PortNumber = device->PortNumber;
            AddFoundDevice(device->DeviceInfo->CardReader->DeviceName,device->PortNumber,device->DeviceInfo->CardReader->Type);
            Log->Write("������ ��������� ["+device->DeviceInfo->CardReader->DeviceName+"] �� ����� ���"+device->PortNumber);
        }

        if((device->DeviceInfo->Modem->Determinated)&&(device->DeviceInfo->Modem->Read == false))
        {
            device->DeviceInfo->Modem->Read = true;
            DevicesInfo->Modem->DeviceName = device->DeviceInfo->Modem->DeviceName;
            DevicesInfo->Modem->Determinated = true;
            DevicesInfo->Modem->PortNumber = device->PortNumber;
            AddFoundDevice(device->DeviceInfo->Modem->DeviceName,device->PortNumber,device->DeviceInfo->Modem->Type);
            Log->Write("������ ����� ["+device->DeviceInfo->Modem->DeviceName+"] �� ����� ���"+device->PortNumber);
        }

        if((device->DeviceInfo->Keyboard->Determinated)&&(device->DeviceInfo->Keyboard->Read == false))
        {
            device->DeviceInfo->Keyboard->Read = true;
            DevicesInfo->Keyboard->DeviceName = device->DeviceInfo->Keyboard->DeviceName;
            DevicesInfo->Keyboard->Determinated = true;
            DevicesInfo->Keyboard->PortNumber = device->PortNumber;
            AddFoundDevice(device->DeviceInfo->Keyboard->DeviceName,device->PortNumber,device->DeviceInfo->Keyboard->Type);
            Log->Write("������� ���������� ["+device->DeviceInfo->Keyboard->DeviceName+"] �� ����� ���"+device->PortNumber);
        }
    }
}

void DF_ControllerClass::WaitForCompleteStatus()
{
    bool TimeOutBreak = false;
    while( (GetCompleteStatus() == false) && (TimeOutBreak == false) )
    {
        if (Terminated) return;
        Sleep(1);
        if(unsigned(clock()-BeginSearching) > SearchingTimeOut)
        {
            TimeOutBreak = true;
            Log->Write("���������� ����� ������� ������ ������������.");
            Terminate();
            return;
        }
    }

    if (Terminated) return;
    if(TimeOutBreak)
        GetCompleteStatus();
    else
        Log->Write("����� ������� ��������.");
}

bool DF_ControllerClass::WaitForTerminated()
{
    if (ExecuteComplete)
    {
        Log->Write("����� ������� ��������.");
        return true;
    }

    DWORD TimeOut = 10*1000;//10 ������ ��� ���������� ������
    clock_t StartSearching = clock();
    while(ExecuteComplete == false)
    {
        Sleep(1);
        if(unsigned(clock()-StartSearching) > TimeOut)
        {
            Log->Write("���������� ����� ������� �������� ���������� ������.");
            return false;
        }
    }

    Log->Write("����� ������� ��������.");
    return true;
}

bool DF_ControllerClass::WaitForSearchComplete()
{
    if (ExecuteComplete)
        return true;

    while(ExecuteComplete == false)
    {
        Sleep(100);
        Application->ProcessMessages();
        if(unsigned(clock()-BeginSearching) > SearchingTimeOut)
        {
            Log->Write("���������� ����� ������� �������� ���������� ������.");
            Terminate();
            return false;
        }
    }

    Log->Write("����� ������� ��������.");
    return true;
}

DF_device_info* DF_ControllerClass::GetFoundDevice()
{
    DF_device_info* result = NULL;

    if ((FoundDevices)&&(FoundDevices->Count > 0))
    {
        result = (DF_device_info*)FoundDevices->Items[FoundDevices->Count-1];
        FoundDevices->Delete(FoundDevices->Count-1);
    }

    return result;
}

void DF_ControllerClass::AddFoundDevice(AnsiString name, int port, int type)
{
    DF_device_info* device = new DF_device_info;

    device->DeviceName = DevicesInfo->GetExternalDeviceName(name);;
    device->PortNumber = port;
    device->Type = type;
    FoundDevices->Add((void*)device);
}

bool DF_ControllerClass::GetSearchEnable(AnsiString DeviceName)
{
    if (DevicesInfo)
        return DevicesInfo->GetSearchEnable(DeviceName);
    else
        return false;
}

