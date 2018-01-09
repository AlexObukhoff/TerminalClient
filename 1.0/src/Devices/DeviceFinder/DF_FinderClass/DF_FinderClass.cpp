#pragma hdrstop

#include "DF_FinderClass.h"

#pragma package(smart_init)

__fastcall DF_FinderClass::DF_FinderClass(int ComPort) : TThread(true)
{
    PortNumber = ComPort;
    SubFolderName = "finder_log";
    //Log = new TLogClass("DF_COM"+AnsiString(ComPort),"",(ErrorCallbackFunc)NULL,SubFolderName);
    Log = new TLogClass("DF_COM"+AnsiString(ComPort));
    if (Log) Log->Write("Creating Device Searching Thread object on COM"+AnsiString(ComPort));

    TerminateWaitTimeOut = 10*1000;// 10 секунд на ожидание завершение потока при завершении программы 
    SearchingTimeOut = 1000*60*10;//10 minutes life timeout
    SearchComplete = false;
    ExecuteComplete = true;
    CurrentDeviceName = "";

    //записываем в инфо номер порта
    DeviceInfo = new DF_DescrClass();
    for(int i=0; i<=2; i++)
    {
        DeviceInfo->Validator[i]->PortNumber = PortNumber;
        DeviceInfo->CoinAcceptor[i]->PortNumber = PortNumber;
    }
    DeviceInfo->Printer->PortNumber = PortNumber;
    DeviceInfo->WatchDog->PortNumber = PortNumber;
    DeviceInfo->CardReader->PortNumber = PortNumber;
    DeviceInfo->Modem->PortNumber = PortNumber;
    DeviceInfo->Keyboard->PortNumber = PortNumber;
}

__fastcall DF_FinderClass::~DF_FinderClass()
{
    //если поток запущен то останавливаем его
    if (Terminated == false)
        StopSearching();

    //ожидаем его корректного завершения
    if (WaitForTerminated() == false)
        TerminateThread((HANDLE)this->Handle,0);

    //удаляем объект DeviceInfo
    if (DeviceInfo)
        delete DeviceInfo;

    //удаление объекта лога
    if (Log)  Log->Write("Destroing Device Searching Thread object on COM"+AnsiString(PortNumber));
    if (Log)  delete Log;
    Log = NULL;
}

bool DF_FinderClass::GetCompleteStatus()
{
    if(unsigned(clock()-BeginSearching) > SearchingTimeOut)
    {
        Log->Write("Закончился лимит времени поиска оборудования.");
        Terminate();
        return true;
    }
    return ExecuteComplete & SearchComplete;
}

void DF_FinderClass::StartSearching()
{
    Resume();
}

void DF_FinderClass::StopSearching()
{
    Terminate();
}

bool DF_FinderClass::WaitForTerminated()
{
    bool result = false;
    try
    {
        if (ExecuteComplete)
            return result = true;

        clock_t StartSearching = clock();
        while(ExecuteComplete == false)
        {
            Sleep(1);
            if(unsigned(clock()-StartSearching) > TerminateWaitTimeOut)
            {
                if (Log) Log->Write("Закончился лимит времени ожидания завершения потока.");
                return result = false;
            }
        }

        return result = true;
    }
    __finally
    {
        if (Log && result) Log->Write("Поток успешно завершён.");
    }
}

bool DF_FinderClass::WaitForCompleteStatus()
{
    if (SearchComplete)
        return true;

    while(SearchComplete == false)
    {
        if (Terminated) return true;
        Sleep(1);
        if(unsigned(clock()-BeginSearching) > SearchingTimeOut)
        {
            if (Log) Log->Write("Закончился лимит времени (10 минут) ожидания завершения поиска.");
            Terminate();
            return true;
        }
    }

    if (Log) Log->Write("Поиск успешно завершён.");
    return true;
}

AnsiString DF_FinderClass::GetSearchDeviceName()
{
    return DeviceInfo->SearchDeviceName;
}

void __fastcall DF_FinderClass::Execute()
{
    try
    {
        try
        {
            ExecuteComplete = false;
            SearchComplete = false;
            if (Log) Log->Write("Begin Device Searching.");

            BeginSearching = clock();

            //определяем, искать ли конкретное устройство по имени
            CurrentDeviceName = GetSearchDeviceName();
            if (Terminated) return;

            //если имя указано, то ищем конкретное устройство на порту
            if (CurrentDeviceName.IsEmpty() == false)
            {
                if (Log) Log->Write("Searching device by name ["+CurrentDeviceName+"].");
                FindDeviceByName(CurrentDeviceName);
            }
            else
            {
                if ((Terminated)||(SearchComplete)) return;
                //если имя пусто и тип устройства >= 0 то ищем любое устройство конкретного типа на порту
                if ((CurrentDeviceName.IsEmpty())&&(DeviceInfo->SearchType >= 0))
                {
                    if (Log) Log->Write("Searching device by type ["+AnsiString(DeviceInfo->SearchType)+"].");
                    FindDevicesByType(DeviceInfo->SearchType);
                }
                else
                {
                    if ((Terminated)||(SearchComplete)) return;
                    //если имя пусто и тип устройства -1 то ищем любое устройство на порту
                    if ((CurrentDeviceName.IsEmpty())&&(DeviceInfo->SearchType == ht_Unknown))
                    {
                        if (Log) Log->Write("Searching anyone device.");
                        FindDevicesByAllNames();
                    }
                }
            }
        }
        __finally
        {
            if (SearchComplete)
            {
                if (Log) Log->Write("Device has been found.");
            }
            else
            {
                if (Log) Log->Write("Device not found.");
            }
            ExecuteComplete = true;
            SearchComplete = true;
            if (Log) Log->Write("Finish Device Searching.");
        }
    }
    catch(Exception& error)
    {
      if (Log) Log->Write("DF_FinderClass::Execute() Exception: "+ AnsiString(error.Message));
    }
}

bool DF_FinderClass::FindDevicesByAllNames()
{
    for(int i=0; i<DeviceInfo->HardwareNames->Count; i++)
    {
        if (Terminated) return false;
        if (DeviceInfo->HardwareNames->Items[i] != NULL)
        {
            DF_HardwareName* name = (DF_HardwareName*)DeviceInfo->HardwareNames->Items[i];
            if (FindDeviceByName(name->Name))
                return true;
        }
    }
    return false;
}

bool DF_FinderClass::FindDevicesByType(int Type)
{
    if (Type == ht_Unknown) return false;
    for(int i=0; i<DeviceInfo->HardwareNames->Count; i++)
    {
        if (Terminated) return false;
        if (DeviceInfo->HardwareNames->Items[i] != NULL)
        {
            DF_HardwareName* name = (DF_HardwareName*)DeviceInfo->HardwareNames->Items[i];
            if (name->Type == Type)
            {
                if (FindDeviceByName(name->Name))
                    return true;
            }
        }
    }
    return false;
}

bool DF_FinderClass::FindDeviceByName(AnsiString DeviceName)
{
    if (Terminated) return false;

    bool SearchResult = SearchComplete = false;

    if (DeviceName.IsEmpty() == false)
    {//если имя устройства не пустое
        //определяем тип устройства по имени
        int HardwareType = DeviceInfo->GetHardwareType(DeviceName);
        if (HardwareType == ht_Unknown) return false;

        //выставив флаг предупреждения начинаем поиск устройства
        switch(HardwareType)
        {
            case ht_Validator:
                DeviceInfo->Validator[0]->DeviceName = DeviceName;
                DeviceInfo->Validator[0]->Alert = true;
                SearchResult = FindValidator(DeviceName);
                break;

            case ht_CoinAcceptor:
                DeviceInfo->CoinAcceptor[0]->DeviceName = DeviceName;
                DeviceInfo->CoinAcceptor[0]->Alert = true;
                SearchResult = FindCoinAcceptor(DeviceName);
                break;

            case ht_Printer:
                DeviceInfo->Printer->DeviceName = DeviceName;
                DeviceInfo->Printer->Alert = true;
                SearchResult = FindPrinter(DeviceName);
                break;

            case ht_WatchDog:
                DeviceInfo->WatchDog->DeviceName = DeviceName;
                DeviceInfo->WatchDog->Alert = true;
                SearchResult = FindWatchDog(DeviceName);
                break;

            case ht_CardReader:
                DeviceInfo->CardReader->DeviceName = DeviceName;
                DeviceInfo->CardReader->Alert = true;
                SearchResult = FindCardReader(DeviceName);
                break;

            case ht_Modem:
                DeviceInfo->Modem->DeviceName = DeviceName;
                DeviceInfo->Modem->Alert = true;
                SearchResult = FindModem(DeviceName);
                break;

            case ht_Keyboard:
                DeviceInfo->Keyboard->DeviceName = DeviceName;
                DeviceInfo->Keyboard->Alert = true;
                SearchResult = FindKeyboard(DeviceName);
                break;
        }

        //перезагрузки в процессе поиска не произошло, сбрасываем флаг
        switch(HardwareType)
        {
            case ht_Validator:
                DeviceInfo->Validator[0]->Alert = false;
                break;
            case ht_CoinAcceptor:
                DeviceInfo->CoinAcceptor[0]->Alert = false;
                break;
            case ht_Printer:
                DeviceInfo->Printer->Alert = false;
                break;
            case ht_WatchDog:
                DeviceInfo->WatchDog->Alert = false;
                break;
            case ht_CardReader:
                DeviceInfo->CardReader->Alert = false;
                break;
            case ht_Modem:
                DeviceInfo->Modem->Alert = false;
                break;
            case ht_Keyboard:
                DeviceInfo->Keyboard->Alert = false;
                break;
        }

        if (Terminated) return false;
        //устройство найдено - заполняем инфо
        if (SearchResult)//если нашли устройство, заполняем инфо о нём
        {
            switch(HardwareType)
            {
                case ht_Validator:
                    if (Log)  Log->Write("Найден купюроприёмник ["+DeviceName+"]");
                    DeviceInfo->Validator[0]->DeviceName = DeviceName;
                    DeviceInfo->Validator[0]->Read = false;
                    DeviceInfo->Validator[0]->Determinated = true;
                    break;

                case ht_CoinAcceptor:
                    if (Log)  Log->Write("Найден монетоприёмник ["+DeviceName+"]");
                    DeviceInfo->CoinAcceptor[0]->DeviceName = DeviceName;
                    DeviceInfo->CoinAcceptor[0]->Read = false;
                    DeviceInfo->CoinAcceptor[0]->Determinated = true;
                    break;

                case ht_Printer:
                    if (Log)  Log->Write("Найден принтер ["+DeviceName+"]");
                    DeviceInfo->Printer->DeviceName = DeviceName;
                    DeviceInfo->Printer->Read = false;
                    DeviceInfo->Printer->Determinated = true;
                    break;

                case ht_WatchDog:
                    if (Log)  Log->Write("Найден сторожевой таймер ["+DeviceName+"]");
                    DeviceInfo->WatchDog->DeviceName = DeviceName;
                    DeviceInfo->WatchDog->Read = false;
                    DeviceInfo->WatchDog->Determinated = true;
                    break;

                case ht_CardReader:
                    if (Log)  Log->Write("Найден кард ридер ["+DeviceName+"]");
                    DeviceInfo->CardReader->DeviceName = DeviceName;
                    DeviceInfo->CardReader->Read = false;
                    DeviceInfo->CardReader->Determinated = true;
                    break;

                case ht_Modem:
                    if (Log)  Log->Write("Найден модем ["+DeviceName+"]");
                    DeviceInfo->Modem->DeviceName = DeviceName;
                    DeviceInfo->Modem->Read = false;
                    DeviceInfo->Modem->Determinated = true;
                    break;

                case ht_Keyboard:
                    if (Log)  Log->Write("Найдена клавиатура ["+DeviceName+"]");
                    DeviceInfo->Keyboard->DeviceName = DeviceName;
                    DeviceInfo->Keyboard->Read = false;
                    DeviceInfo->Keyboard->Determinated = true;
                    break;
            }
        }
    }

    return SearchComplete = SearchResult;
}

//============== При нахождении устройства прекращаем остальные поиски =========

bool DF_FinderClass::FindValidator(AnsiString Name)
{
    if (Terminated) return false;
    TLogClass* log = NULL;
    bool result = false;

    if (DeviceInfo)
    {
        if (DeviceInfo->GetSearchEnable(Name) == false)
            return false;
    }
    else
        return false;

    try
    {
        try
        {
            CValidator* validator = NULL;

            //====================CCNET=========================================
            if (Name.LowerCase() == "ccnetvalidator")
            {
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_CCNET","",(ErrorCallbackFunc)NULL,SubFolderName);
                validator = new TCCNETdevice(0,PortNumber, log);
                if (validator)
                {
                    validator->StartDevice();
                    Sleep(500);
                    if ((validator->DeviceState->OutStateCode != DSE_NOTMOUNT)&&(validator->DeviceState->OutStateCode != DSE_UNKNOWN))
                        result = true;
                    delete validator;
                    if (result) return result;
                }
            }
            //==================================================================

            //====================MEI===========================================
            if (Name.LowerCase() == "meivalidator")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_MEI","",(ErrorCallbackFunc)NULL,SubFolderName);
                validator = new TMEIdevice(0,PortNumber, log);
                if (validator)
                {
                    validator->StartDevice();
                    Sleep(200);
                    if ((validator->DeviceState->OutStateCode != DSE_NOTMOUNT)&&(validator->DeviceState->OutStateCode != DSE_UNKNOWN))
                        result = true;
                    delete validator;
                    if (result) return result;
                }
            }
            //==================================================================

            //====================ID003=========================================
            if ((Name.LowerCase() == "wba003_1validator")||(Name.LowerCase() == "wba003_2validator"))
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_ID003","",(ErrorCallbackFunc)NULL,SubFolderName);
                validator = new TID003_1device(0,PortNumber, log);
                if (validator)
                {
                    validator->StartDevice();
                    Sleep(200);
                    if ((validator->DeviceState->OutStateCode != DSE_NOTMOUNT)&&(validator->DeviceState->OutStateCode != DSE_UNKNOWN))
                        result = true;
                    delete validator;
                    if (result) return result;
                }
            }
            //==================================================================

            //====================ICT===========================================
            if (Name.LowerCase() == "ictvalidator")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_ICT","",(ErrorCallbackFunc)NULL,SubFolderName);
                validator = new TICTDevice(0,PortNumber, log);
                if (validator)
                {
                    validator->StartDevice();
                    Sleep(200);
                    if ((validator->DeviceState->OutStateCode != DSE_NOTMOUNT)&&(validator->DeviceState->OutStateCode != DSE_UNKNOWN))
                        result = true;
                    delete validator;
                    if (result) return result;
                }
            }
            //==================================================================

            //====================JCM===========================================
            if (Name.LowerCase() == "jcmvalidator")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_JCM","",(ErrorCallbackFunc)NULL,SubFolderName);
                validator = new TJCMdevice(0,PortNumber, log);
                if (validator)
                {
                    validator->StartDevice();
                    Sleep(200);
                    if ((validator->DeviceState->OutStateCode != DSE_NOTMOUNT)&&(validator->DeviceState->OutStateCode != DSE_UNKNOWN))
                        result = true;
                    delete validator;
                    if (result) return result;
                }
            }
            //==================================================================

            //====================NV9_CCNET=====================================
            if (Name.LowerCase() == "nv9_ccnetvalidator")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_NV9CCNET","",(ErrorCallbackFunc)NULL,SubFolderName);
                validator = new TNV9_CCNETdevice(0,PortNumber, log);
                if (validator)
                {
                    validator->StartDevice();
                    Sleep(200);
                    if ((validator->DeviceState->OutStateCode != DSE_NOTMOUNT)&&(validator->DeviceState->OutStateCode != DSE_UNKNOWN))
                        result = true;
                    delete validator;
                    if (result) return result;
                }
            }
            //==================================================================

            //====================V2E===========================================
            if (Name.LowerCase() == "v2evalidator")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_V2E","",(ErrorCallbackFunc)NULL,SubFolderName);
                validator = new TV2Edevice(0,PortNumber, log);
                if (validator)
                {
                    validator->StartDevice();
                    Sleep(200);
                    if ((validator->DeviceState->OutStateCode != DSE_NOTMOUNT)&&(validator->DeviceState->OutStateCode != DSE_UNKNOWN))
                        result = true;
                    delete validator;
                    if (result) return result;
                }
            }
            //==================================================================
        }
        catch(Exception& error)
        {
          if (Log) Log->Write("DF_FinderClass::FindValidator() Exception: "+ AnsiString(error.Message));
        }
    }
    __finally
    {
        if (log) delete log;
        log = NULL;
        return result;
    }
}

bool DF_FinderClass::FindCoinAcceptor(AnsiString Name)
{
    if (Terminated) return false;
    TLogClass* log = NULL;
    bool result = false;

    if (DeviceInfo)
    {
        if (DeviceInfo->GetSearchEnable(Name) == false)
            return false;
    }
    else
        return false;

    try
    {
        try
        {
            CCoinAcceptor* CoinAcceptor = NULL;

            //====================NRI===========================================
            if (Name.LowerCase() == "nricoinacceptor")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_NRI","",(ErrorCallbackFunc)NULL,SubFolderName);
                CoinAcceptor = new TNRIdevice(0,PortNumber, log);
                if (CoinAcceptor)
                {
                    CoinAcceptor->StartDevice();
                    Sleep(200);
                    if ((CoinAcceptor->DeviceState->OutStateCode != DSE_NOTMOUNT)&&(CoinAcceptor->DeviceState->OutStateCode != DSE_UNKNOWN))
                        result = true;
                    delete CoinAcceptor;
                    if (result) return result;
                }
            }
            //==================================================================

        }
        catch(Exception& error)
        {
          if (Log) Log->Write("DF_FinderClass::FindCoinAcceptor() Exception: "+ AnsiString(error.Message));
        }
    }
    __finally
    {
        if (log) delete log;
        log = NULL;
        return result;
    }
}

bool DF_FinderClass::FindPrinter(AnsiString Name)
{
    if (Terminated) return false;
    TLogClass* log = NULL;
    bool result = false;

    if (DeviceInfo)
    {
        if (DeviceInfo->GetSearchEnable(Name) == false)
            return false;
    }
    else
        return false;

    try
    {
        try
        {
            CPrinter* Printer = NULL;

            //===============================================================
            if (Name.LowerCase() == "citizencbm100t2")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_CBM1000T2","",(ErrorCallbackFunc)NULL,SubFolderName);
                Printer = new CBM1000Type2(PortNumber, 0, log);
                if (Printer)
                {
                    if (Printer->IsItYou())
                        result = true;
                    delete Printer;
                    if (result) return result;
                }
            }
            //==================================================================

            //===============================================================
            if (Name.LowerCase() == "citizen268")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_Citizen268","",(ErrorCallbackFunc)NULL,SubFolderName);
                Printer = new CCitizen268(PortNumber, 0, log);
                if (Printer)
                {
                    if (Printer->IsItYou())
                        result = true;
                    delete Printer;
                    if (result) return result;
                }
            }
            //==================================================================

            //===============================================================
            if (Name.LowerCase() == "citizenppu700")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_CitizenPPU700","",(ErrorCallbackFunc)NULL,SubFolderName);
                Printer = new CCitizenPPU700(PortNumber, 0, log);
                if (Printer)
                {
                    if (Printer->IsItYou())
                        result = true;
                    delete Printer;
                    if (result) return result;
                }
            }
            //==================================================================

            //===============================================================
            if (Name.LowerCase() == "custom")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_CustomPrn","",(ErrorCallbackFunc)NULL,SubFolderName);
                Printer = new CCustomPrn(PortNumber, 0, log);
                if (Printer)
                {
                    if (Printer->IsItYou())
                        result = true;
                    delete Printer;
                    if (result) return result;
                }
            }
            //==================================================================

            //===============================================================
            if (Name.LowerCase() == "epson442")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_Epson442","",(ErrorCallbackFunc)NULL,SubFolderName);
                Printer = new CEpson442(PortNumber, 0, log);
                if (Printer)
                {
                    if (Printer->IsItYou())
                        result = true;
                    delete Printer;
                    if (result) return result;
                }
            }
            //==================================================================

            //===============================================================
            if (Name.LowerCase() == "startup900")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_StarTUP900","",(ErrorCallbackFunc)NULL,SubFolderName);
                Printer = new CStarTUP900(PortNumber, 0, log);
                if (Printer)
                {
                    if (Printer->IsItYou())
                        result = true;
                    delete Printer;
                    if (result) return result;
                }
            }
            //==================================================================

            //===============================================================
            if (Name.LowerCase() == "swecointtp2010")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_SwecoinTTP2010","",(ErrorCallbackFunc)NULL,SubFolderName);
                Printer = new CSwecoinTTP2010(PortNumber, 0, log);
                if (Printer)
                {
                    if (Printer->IsItYou())
                        result = true;
                    delete Printer;
                    if (result) return result;
                }
            }
            //==================================================================

            //===============================================================
            if (Name.LowerCase() == "shtrihfr")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_ShtrihPrinter","",(ErrorCallbackFunc)NULL,SubFolderName);
                Printer = new CShtrihPrinter(log);
                if (Printer)
                {
                    if (Printer->IsItYou())
                        result = true;
                    delete Printer;
                    if (result) return result;
                }
            }
            //==================================================================

            //===============================================================
            if (Name.LowerCase() == "winprinter")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_WinPrinter","",(ErrorCallbackFunc)NULL,SubFolderName);
                Printer = new CWinPrinter(log);
                if (Printer)
                {
                    if (Printer->IsItYou())
                        result = true;
                    delete Printer;
                    if (result) return result;
                }
            }
            //==================================================================

            //===============================================================
            if (Name.LowerCase() == "citizenppu231")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_CitizenPPU231","",(ErrorCallbackFunc)NULL,SubFolderName);
                Printer = new CCitizenPPU231(PortNumber, 0, log);
                if (Printer)
                {
                    if (Printer->IsItYou())
                        result = true;
                    delete Printer;
                    if (result) return result;
                }
            }
            //==================================================================

            //===============================================================
            if (Name.LowerCase() == "citizenppu232")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_CitizenPPU232","",(ErrorCallbackFunc)NULL,SubFolderName);
                Printer = new CCitizenPPU232(PortNumber, 0, log);
                if (Printer)
                {
                    if (Printer->IsItYou())
                        result = true;
                    delete Printer;
                    if (result) return result;
                }
            }
            //==================================================================

            //===============================================================
            if (Name.LowerCase() == "star_tsp_700")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_StarTSP700","",(ErrorCallbackFunc)NULL,SubFolderName);
                Printer = new CStarTSP700(PortNumber, 0, log);
                if (Printer)
                {
                    if (Printer->IsItYou())
                        result = true;
                    delete Printer;
                    if (result) return result;
                }
            }
            //==================================================================

            //===============================================================
            if (Name.LowerCase() == "wp_t833")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_wp-t833","",(ErrorCallbackFunc)NULL,SubFolderName);
                Printer = new wp_t833(PortNumber, 0, log);
                if (Printer)
                {
                    if (Printer->IsItYou())
                        result = true;
                    delete Printer;
                    if (result) return result;
                }
            }
            //==================================================================

            //===============================================================
            if (Name.LowerCase() == "prim21k")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_Prim21kClass","",(ErrorCallbackFunc)NULL,SubFolderName);
                Printer = new CPrim21kClass(PortNumber, 0, log);
                if (Printer)
                {
                    if (Printer->IsItYou())
                        result = true;
                    delete Printer;
                    if (result) return result;
                }
            }
            //==================================================================

            //===============================================================
            if (Name.LowerCase() == "prim08tk")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_Prim08TKClass","",(ErrorCallbackFunc)NULL,SubFolderName);
                Printer = new CPrim08TKClass(PortNumber, 0, log);
                if (Printer)
                {
                    if (Printer->IsItYou())
                        result = true;
                    delete Printer;
                    if (result) return result;
                }
            }
            //==================================================================

            //===============================================================
            if (Name.LowerCase() == "citizencpp8001")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_CitizenCPP8001","",(ErrorCallbackFunc)NULL,SubFolderName);
                Printer = new CCitizenCPP8001(PortNumber, 0, log);
                if (Printer)
                {
                    if (Printer->IsItYou())
                        result = true;
                    delete Printer;
                    if (result) return result;
                }
            }
            //==================================================================
        }
        catch(Exception& error)
        {
          if (Log) Log->Write("DF_FinderClass::FindPrinter() Exception: "+ AnsiString(error.Message));
        }
    }
    __finally
    {
        if (log) delete log;
        log = NULL;
        return result;
    }
}

bool DF_FinderClass::FindWatchDog(AnsiString Name)
{
    if (Terminated) return false;
    TLogClass* log = NULL;
    bool result = false;

    if (DeviceInfo)
    {
        if (DeviceInfo->GetSearchEnable(Name) == false)
            return false;
    }
    else
        return false;
    try
    {
        try
        {
            CWatchDog* WatchDog = NULL;

            //===============================================================
            if (Name.LowerCase() == "st1.2")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_st1-2","",(ErrorCallbackFunc)NULL,SubFolderName);
                WatchDog = new TWatchDogdevice(PortNumber, log);
                if (WatchDog)
                {
                    WatchDog->OnlyPnP = true;
                    WatchDog->StartDevice();
                    Sleep(200);
                    if (WatchDog->IsItYou())
                        result = true;
                    WatchDog->StopTimer();
                    delete WatchDog;
                    if (result) return result;
                }
            }
            //==================================================================

            //===============================================================
            if (Name.LowerCase() == "osmp1")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_WatchDogOSMP","",(ErrorCallbackFunc)NULL,SubFolderName);
                WatchDog = new TWatchDogOSMPdevice(PortNumber, log);
                if (WatchDog)
                {
                    WatchDog->OnlyPnP = true;
                    WatchDog->StartDevice();
                    Sleep(200);
                    if (WatchDog->IsItYou())
                        result = true;
                    WatchDog->StopTimer();
                    delete WatchDog;
                    if (result) return result;
                }
            }
            //==================================================================

            //===============================================================
            if (Name.LowerCase() == "osmp2")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_WatchDogOSMP2","",(ErrorCallbackFunc)NULL,SubFolderName);
                WatchDog = new TWatchDogOSMP2device(PortNumber, log);
                if (WatchDog)
                {
                    WatchDog->OnlyPnP = true;
                    WatchDog->StartDevice();
                    Sleep(200);
                    if (WatchDog->IsItYou())
                        result = true;
                    WatchDog->StopTimer();
                    delete WatchDog;
                    if (result) return result;
                }
            }
            //==================================================================

            //===============================================================
            if (Name.LowerCase() == "alarm")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_WatchDogAlarm","",(ErrorCallbackFunc)NULL,SubFolderName);
                WatchDog = new TWatchDogAlarmDevice(PortNumber, log);
                if (WatchDog)
                {
                    WatchDog->OnlyPnP = true;
                    WatchDog->StartDevice();
                    Sleep(200);
                    if (WatchDog->IsItYou())
                        result = true;
                    WatchDog->StopTimer();
                    delete WatchDog;
                    if (result) return result;
                }
            }
            //==================================================================

            //===============================================================
            if (Name.LowerCase() == "platix")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_WatchDogPlatix","",(ErrorCallbackFunc)NULL,SubFolderName);
                WatchDog = new TWatchDogPlatixDevice(PortNumber, log);
                if (WatchDog)
                {
                    WatchDog->OnlyPnP = true;
                    WatchDog->StartDevice();
                    Sleep(200);
                    if (WatchDog->IsItYou())
                        result = true;
                    WatchDog->StopTimer();
                    delete WatchDog;
                    if (result) return result;
                }
            }
            //==================================================================

            //===============================================================
            if (Name.LowerCase() == "fairpay")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_FairPayWD","",(ErrorCallbackFunc)NULL,SubFolderName);
                WatchDog = new TFairPayWDDevice(log);
                if (WatchDog)
                {
                    WatchDog->OnlyPnP = true;
                    WatchDog->StartDevice();
                    Sleep(200);
                    if (WatchDog->IsItYou())
                        result = true;
                    WatchDog->StopTimer();
                    delete WatchDog;
                    if (result) return result;
                }
            }
            //==================================================================

            //===============================================================
            if (Name.LowerCase() == "ldog")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_LDOG","",(ErrorCallbackFunc)NULL,SubFolderName);
                WatchDog = new TLDOGdevice(PortNumber, log);
                if (WatchDog)
                {
                    WatchDog->OnlyPnP = true;
                    WatchDog->StartDevice();
                    Sleep(200);
                    if (WatchDog->IsItYou())
                        result = true;
                    WatchDog->StopTimer();
                    delete WatchDog;
                    if (result) return result;
                }
            }
            //==================================================================

            //===============================================================
            if (Name.LowerCase() == "sbk2")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_SBK2","",(ErrorCallbackFunc)NULL,SubFolderName);
                WatchDog = new TSBK2Device(log);
                if (WatchDog)
                {
                    WatchDog->OnlyPnP = true;
                    WatchDog->StartDevice();
                    Sleep(200);
                    if (WatchDog->IsItYou())
                        result = true;
                    WatchDog->StopTimer();
                    delete WatchDog;
                    if (result) return result;
                }
            }
            //==================================================================

            //===============================================================
            if (Name.LowerCase() == "sim2osmp")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_SIM2OSMP","",(ErrorCallbackFunc)NULL,SubFolderName);
                WatchDog = new TSIM2OSMPdevice(PortNumber, log);
                if (WatchDog)
                {
                    WatchDog->OnlyPnP = true;
                    WatchDog->StartDevice();
                    Sleep(200);
                    if (WatchDog->IsItYou())
                        result = true;
                    WatchDog->StopTimer();
                    delete WatchDog;
                    if (result) return result;
                }
            }
            //==================================================================
        }
        catch(Exception& error)
        {
          if (Log) Log->Write("DF_FinderClass::FindWatchDog() Exception: "+ AnsiString(error.Message));
        }
    }
    __finally
    {
        if (log) delete log;
        log = NULL;
        return result;
    }
}

bool DF_FinderClass::FindCardReader(AnsiString Name)
{
    if (Terminated) return false;
    TLogClass* log = NULL;
    bool result = false;

    if (DeviceInfo)
    {
        if (DeviceInfo->GetSearchEnable(Name) == false)
            return false;
    }
    else
        return false;

    try
    {
        try
        {
            CCardReader* CardReader;

            //==================================================================
            if (Name.LowerCase() == "metrocardreader")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_MetroCardReader","",(ErrorCallbackFunc)NULL,SubFolderName);
                CardReader = new TMetroCardDevice(PortNumber, log);
                if (CardReader)
                {
                    CardReader->StartDevice();
                    Sleep(500);
                    if (CardReader->IsItYou())
                        result = true;
                    delete CardReader;
                    if (result) return result;
                }
            }
            //==================================================================
        }
        catch(Exception& error)
        {
          if (Log) Log->Write("DF_FinderClass::FindCardReader() Exception: "+ AnsiString(error.Message));
        }
    }
    __finally
    {
        if (log) delete log;
        log = NULL;
        return result;
    }
}

bool DF_FinderClass::FindModem(AnsiString Name)
{
    if (Terminated) return false;
    TLogClass* log = NULL;
    bool result = false;

    if (DeviceInfo)
    {
        if (DeviceInfo->GetSearchEnable(Name) == false)
            return false;
    }
    else
        return false;

    try
    {
        try
        {
            CModem* Modem = NULL;

           //==================================================================
           if (Name.LowerCase() == "modemsiemensmc35i")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_mc35i","",(ErrorCallbackFunc)NULL,SubFolderName);
                Modem = new CModemSiemensMC35i(PortNumber, log);
                if (Modem)
                {
                    if (Modem->IsItYou())
                        result = true;
                    delete Modem;
                    if (result) return result;
                }
            }
            //==================================================================

           //==================================================================
           if (Name.LowerCase() == "siemensmc39multisim")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_mc39","",(ErrorCallbackFunc)NULL,SubFolderName);
                Modem = new CModemSiemensMC39MultiSIM(PortNumber, log);
                if (Modem)
                {
                    if (Modem->IsItYou())
                        result = true;
                    delete Modem;
                    if (result) return result;
                }
            }
            //==================================================================

        }
        catch(Exception& error)
        {
          if (Log) Log->Write("DF_FinderClass::FindModem() Exception: "+ AnsiString(error.Message));
        }
    }
    __finally
    {
        if (log) delete log;
        log = NULL;
        return result;
    }
}

bool DF_FinderClass::FindKeyboard(AnsiString Name)
{
    if (Terminated) return false;
    TLogClass* log = NULL;
    bool result = false;

    if (DeviceInfo)
    {
        if (DeviceInfo->GetSearchEnable(Name) == false)
            return false;
    }
    else
        return false;

    try
    {
        try
        {
            CKeyboard* Keyboard;

            //==================================================================
            if (Name.LowerCase() == "iskrakeyboard")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_IskraKeyboard","",(ErrorCallbackFunc)NULL,SubFolderName);
                Keyboard = new TIskraKeybDevice(0,PortNumber, log);
                if (Keyboard)
                {
                    Keyboard->StartDevice();
                    Sleep(500);
                    if (Keyboard->IsItYou())
                        result = true;
                    delete Keyboard;
                    if (result) return result;
                }
            }
            //==================================================================

            //==================================================================
            if (Name.LowerCase() == "ktekkeyboard")
            {
                Sleep(200);
                if (log) delete log;
                log = new TLogClass("COM"+AnsiString(PortNumber)+"_KtekKeyboard","",(ErrorCallbackFunc)NULL,SubFolderName);
                Keyboard = new TKtekKeybDevice(0,PortNumber, log);
                if (Keyboard)
                {
                    Keyboard->StartDevice();
                    Sleep(500);
                    if (Keyboard->IsItYou())
                        result = true;
                    delete Keyboard;
                    if (result) return result;
                }
            }
            //==================================================================
        }
        catch(Exception& error)
        {
          if (Log) Log->Write("DF_FinderClass::FindKeyboard() Exception: "+ AnsiString(error.Message));
        }
    }
    __finally
    {
        if (log) delete log;
        log = NULL;
        return result;
    }
}



