//---------------------------------------------------------------------------


#pragma hdrstop

#include "DeviceChecker.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

CDeviceChecker::CDeviceChecker(CDeviceDescriptor* descriptor, TLogClass* log)
{
    MinPortNumber = 1;
    MaxPortNumber = 40;
    Descriptor = descriptor;
    NewLog = false;
    Log = log;
    if (Log == NULL)
    {
      NewLog = true;
      Log = new TLogClass("DeviceChecker");
    }
    Ports = new TList();
    PortsList = new TStringList();
    CheckingLoop();
}

void CDeviceChecker::GetPortsList()
{
    if (Ports)
        Ports->Clear();
    if (PortsList)
        PortsList->Clear();
    //create list of availible numbers of COM ports
    for(int i=MinPortNumber; i<=MaxPortNumber; i++)
    {
        AnsiString PortName = "\\\\.\\COM"+AnsiString(i);
        //открываем порт
        HANDLE Port = CreateFile(PortName.c_str(),
                          GENERIC_READ | GENERIC_WRITE,
                          0,
                          NULL,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);
        // если ошибка - выходим
        if (Port == INVALID_HANDLE_VALUE)
            continue;

        if ((Port != NULL)&&(Port != INVALID_HANDLE_VALUE))
            CloseHandle(Port);
        Log->Write("Найден порт СОМ"+AnsiString(i));
        Ports->Add((void*)i);
        PortsList->Add(PortName);
    }
}

void CDeviceChecker::CheckingLoop()
{
    if (Descriptor == NULL)
        return;

    GetPortsList();

    bool FindDevice = false;
    for (int i=0; i<Ports->Count; i++)
    {
        if(FindDevice) break;
        if (Ports->Items[i] == NULL) continue;

        CValidator* validator;
        CPrinter* printer;
        CWatchDog* watchdog;
        CModem* modem;
        CKeyboard* keyboard;

        AnsiString DeviceName = "";
        int PortNumber = (int)Ports->Items[i];
        //validators
        for(int j=0; (j<3); j++)
        {
              if (Descriptor->Validator[j]->Determinated)
                  continue;
              if ((Descriptor->Validator[j]->Port > 0)&&(Descriptor->Validator[j]->Port != PortNumber))
                  continue;
              DeviceName = Descriptor->Validator[j]->DeviceName.LowerCase();
              // IF VALIDATOR DOESN`T KNOW CHECKING EVERYTHING
              if (DeviceName != "")
              {
                 if (DeviceName == "validator")
                 {
                    // CHECK CCNET
                    validator = new TCCNETdevice(Descriptor->Validator[j]->ID, PortNumber);
                    if (validator)
                    {
                        validator->StartDevice();
                        DWORD ticks = 500;
                        while((!validator->IsInitialized())&&(ticks>0))
                        {
                            Application->ProcessMessages();
                            ticks--;
                            Sleep(10);
                        }
                    }
                    if ((validator)&&(validator->DeviceState)&&(validator->DeviceState->OldStateCode != DSE_NOTMOUNT)&&(validator->IsItYou()))
                    {
                        Log->Write("Найдено устройство "+validator->DeviceName+" на порту СОМ"+AnsiString(PortNumber));
                        Descriptor->Validator[j]->Port = PortNumber;
                        Descriptor->Validator[j]->Determinated = true;
                        Descriptor->Validator[j]->DeviceName = validator->DeviceName;
                        Descriptor->Validator[j]->Device = (void*)validator;
                        FindDevice = true;
                        if ((validator)&&(Descriptor->Validator[j]->AutoDestroy))
                        {
                            delete validator;
                            validator = NULL;
                            Descriptor->Validator[j]->Device = NULL;
                        }
                    }
                    else
                    {
                        Ports->Items[i] = 0;
                        if (validator)
                        {
                            delete validator;
                            validator = NULL;
                        }
                    }


                    // CHECK WBA003
                    if (!Descriptor->Validator[j]->Determinated)
                    {
                        validator = new TID003_1device(Descriptor->Validator[j]->ID, PortNumber);
                        if (validator)
                        {
                            validator->StartDevice();
                            DWORD ticks = 500;
                            while((!validator->IsInitialized())&&(ticks>0))
                            {
                                Application->ProcessMessages();
                                ticks--;
                                Sleep(10);
                            }
                        }
                        if ((validator)&&(validator->DeviceState)&&(validator->DeviceState->OldStateCode != DSE_NOTMOUNT)&&(validator->IsItYou()))
                        {
                            Log->Write("Найдено устройство "+validator->DeviceName+" на порту СОМ"+AnsiString(PortNumber));
                            Descriptor->Validator[j]->Port = PortNumber;
                            Descriptor->Validator[j]->Determinated = true;
                            Descriptor->Validator[j]->DeviceName = validator->DeviceName;
                            Descriptor->Validator[j]->Device = (void*)validator;
                            FindDevice = true;
                            if ((validator)&&(Descriptor->Validator[j]->AutoDestroy))
                            {
                                delete validator;
                                validator = NULL;
                                Descriptor->Validator[j]->Device = NULL;
                            }
                        }
                        else
                        {
                            Ports->Items[i] = 0;
                            if (validator)
                            {
                                delete validator;
                                validator = NULL;
                            }
                        }

                    }
                    // CHECK ICT
                    if (!Descriptor->Validator[j]->Determinated)
                    {
                        validator = new TICTDevice(Descriptor->Validator[j]->ID, PortNumber);
                        if (validator)
                        {
                            validator->StartDevice();
                            DWORD ticks = 500;
                            while((!validator->IsInitialized())&&(ticks>0))
                            {
                                Application->ProcessMessages();
                                ticks--;
                                Sleep(10);
                            }
                        }
                        if ((validator)&&(validator->DeviceState)&&(validator->DeviceState->OldStateCode != DSE_NOTMOUNT)&&(validator->IsItYou()))
                        {
                            Log->Write("Найдено устройство "+validator->DeviceName+" на порту СОМ"+AnsiString(PortNumber));
                            Descriptor->Validator[j]->Port = PortNumber;
                            Descriptor->Validator[j]->Determinated = true;
                            Descriptor->Validator[j]->DeviceName = validator->DeviceName;
                            Descriptor->Validator[j]->Device = (void*)validator;
                            FindDevice = true;
                            if ((validator)&&(Descriptor->Validator[j]->AutoDestroy))
                            {
                                delete validator;
                                validator = NULL;
                                Descriptor->Validator[j]->Device = NULL;
                            }
                        }
                        else
                        {
                            Ports->Items[i] = 0;
                            if (validator)
                            {
                                delete validator;
                                validator = NULL;
                            }
                        }
                    }
                    // CHECK JCM
                    if (!Descriptor->Validator[j]->Determinated)
                    {
                        validator = new TJCMdevice(Descriptor->Validator[j]->ID, PortNumber);
                        if (validator)
                        {
                            validator->StartDevice();
                            DWORD ticks = 500;
                            while((!validator->IsInitialized())&&(ticks>0))
                            {
                                Application->ProcessMessages();
                                ticks--;
                                Sleep(10);
                            }
                        }
                        if ((validator)&&(validator->DeviceState)&&(validator->DeviceState->OldStateCode != DSE_NOTMOUNT)&&(validator->IsItYou()))
                        {
                            Log->Write("Найдено устройство "+validator->DeviceName+" на порту СОМ"+AnsiString(PortNumber));
                            Descriptor->Validator[j]->Port = PortNumber;
                            Descriptor->Validator[j]->Determinated = true;
                            Descriptor->Validator[j]->DeviceName = validator->DeviceName;
                            Descriptor->Validator[j]->Device = (void*)validator;
                            FindDevice = true;
                            if ((validator)&&(Descriptor->Validator[j]->AutoDestroy))
                            {
                                delete validator;
                                validator = NULL;
                                Descriptor->Validator[j]->Device = NULL;
                            }
                        }
                        else
                        {
                            Ports->Items[i] = 0;
                            if (validator)
                            {
                                  delete validator;
                                  validator = NULL;
                            }
                        }

                    }
                 }
                 //IF VALIDATOR KNOWN
                 else
                 {
                    if (DeviceName == "ccnet")
                        validator = new TCCNETdevice(Descriptor->Validator[j]->ID, PortNumber);
                    if (DeviceName == "WBA003_1Validator")
                        validator = new TID003_1device(Descriptor->Validator[j]->ID, PortNumber);
                    if (DeviceName == "ict")
                        validator = new TICTDevice(Descriptor->Validator[j]->ID, PortNumber);
                    if (DeviceName == "JCMValidator")
                        validator = new TJCMdevice(Descriptor->Validator[j]->ID, PortNumber);

                    if (validator)
                    {
                        validator->StartDevice();
                        DWORD ticks = 500;
                        while((!validator->IsInitialized())&&(ticks>0))
                        {
                            Application->ProcessMessages();
                            ticks--;
                            Sleep(10);
                        }
                    }
                    if ((validator)&&(validator->DeviceState)&&(validator->DeviceState->OldStateCode != DSE_NOTMOUNT)&&(validator->IsItYou()))
                    {
                        Log->Write("Найдено устройство "+validator->DeviceName+" на порту СОМ"+AnsiString(PortNumber));
                        Descriptor->Validator[j]->Port = PortNumber;
                        Descriptor->Validator[j]->Determinated = true;
                        Descriptor->Validator[j]->Device = (void*)validator;
                        FindDevice = true;
                        if ((validator)&&(Descriptor->Validator[j]->AutoDestroy))
                        {
                            delete validator;
                            validator = NULL;
                            Descriptor->Validator[j]->Device = NULL;
                        }
                    }
                    else
                    {
                        Ports->Items[i] = 0;
                        if (validator)
                        {
                            delete validator;
                            validator = NULL;
                        }
                    }
                 }
              }
        }

        //Modem
        DeviceName = Descriptor->Modem->DeviceName.LowerCase();
        if ((DeviceName != "")&&
            (Descriptor->Modem->Determinated == false)&&
            ((Descriptor->Modem->Port == 0)||(Descriptor->Modem->Port == PortNumber)))
        {
            if (DeviceName == "mc35i")
                modem = new CModemSiemensMC35i(PortNumber);

            Sleep(1500);
            if ( (modem) && (modem->IsItYou()) )
            {
                Log->Write("Найдено устройство " + modem->DeviceName + " на порту СОМ" + AnsiString(PortNumber));
                modem->DeviceName = modem->GetOperatorName();
                Descriptor->Modem->Port = PortNumber;
                Descriptor->Modem->Determinated = true;
                Descriptor->Modem->Device = (void*)modem;
                FindDevice = true;
                if ((modem)&&(Descriptor->Modem->AutoDestroy))
                {
                    delete modem;
                    modem = NULL;
                    Descriptor->Modem->Device = NULL;
                }
            }
            else
            {
                Ports->Items[i] = 0;
                if (modem)
                {
                    delete modem;
                    modem = NULL;
                }
            }
        }

        //Watchdogs
        DeviceName = Descriptor->WatchDog->DeviceName.LowerCase();
        if ((DeviceName != "")&&
            (Descriptor->WatchDog->Determinated == false)&&
            ((Descriptor->WatchDog->Port == 0)||(Descriptor->WatchDog->Port == PortNumber)))
        {
            if (DeviceName == "st1.2")
                watchdog = new TWatchDogdevice(PortNumber);
            if (DeviceName == "osmp")
                watchdog = new TWatchDogOSMPdevice(PortNumber);
            if (DeviceName == "alarm")
                watchdog = new TWatchDogAlarmDevice(PortNumber);
            if (DeviceName == "osmp2")
                watchdog = new TWatchDogOSMP2device(PortNumber);
            if (DeviceName == "platix")
                watchdog = new TWatchDogPlatixDevice(PortNumber);

            if (watchdog)
            {
                watchdog->StartDevice();
                Sleep(500);
            }
            if ( (watchdog) && (watchdog->IsItYou()) )
            {
                Log->Write("Найдено устройство "+watchdog->DeviceName+" на порту СОМ"+AnsiString(PortNumber));
                Descriptor->WatchDog->Port = PortNumber;
                Descriptor->WatchDog->Determinated = true;
                Descriptor->WatchDog->Device = (void*)watchdog;
                FindDevice = true;
                if ((watchdog)&&(Descriptor->WatchDog->AutoDestroy))
                {
                    delete watchdog;
                    watchdog = NULL;
                    Descriptor->WatchDog->Device = NULL;
                }
            }
            else
            {
                Ports->Items[i] = 0;
                if (watchdog)
                {
                    delete watchdog;
                    watchdog = NULL;
                }
            }
        }

        //printers
        DeviceName = Descriptor->Printer->DeviceName.LowerCase();
        if ((DeviceName != "")&&
            (Descriptor->Printer->Determinated == false)&&
            ((Descriptor->Printer->Port == 0)||(Descriptor->Printer->Port == PortNumber)))
        {
            if (DeviceName == "citizencbm1000t2")
                printer = new CBM1000Type2(NULL,PortNumber);
            if (DeviceName == "av-268")
                printer = new CCitizen268(PortNumber);
            if (DeviceName == "citizen_ppu_700")
                printer = new CCitizenPPU700(PortNumber);
            if (DeviceName == "custom_vkp_80")
                printer = new CCustomPrn(PortNumber);
            if (DeviceName == "epson_442")
                printer = new CEpson442(PortNumber);
            if (DeviceName == "startup900")
                printer = new CStarTUP900(PortNumber);
            if (DeviceName == "swecoin-ttp2010")
                printer = new CSwecoinTTP2010(PortNumber);
            if ((DeviceName == "shtrih-kiosk")||(DeviceName == "shtrih-frk"))
                printer = new CShtrihPrinter();
            if (DeviceName == "windows")
                printer = new CWinPrinter();
            if (DeviceName == "citizen_ppu_231")
                printer = new CCitizenPPU231(PortNumber);
            if (DeviceName == "citizen_ppu_232")
                printer = new CCitizenPPU232(PortNumber);
            if (DeviceName == "star_tsp_700")
                printer = new CStarTSP700(PortNumber);
            if (DeviceName == "wp_t833")
                printer = new wp_t833(NULL,PortNumber);
            if ((DeviceName == "prim21k-frk")||(DeviceName == "prim21k-kiosk"))
                printer = new CPrim21kClass(PortNumber);
            if ((DeviceName == "primo8tk-frk")||(DeviceName == "primo8tk-kiosk"))
                printer = new CPrim08TKClass(PortNumber);

            if ( (printer) && (printer->IsItYou()) )
            {
                Log->Write("Найдено устройство "+printer->DeviceName+" на порту СОМ"+AnsiString(PortNumber));
                Descriptor->Printer->Port = PortNumber;
                Descriptor->Printer->Determinated = true;
                Descriptor->Printer->Device = (void*)printer;
                FindDevice = true;
                if ((printer)&&(Descriptor->Printer->AutoDestroy))
                {
                    delete printer;
                    printer = NULL;
                    Descriptor->Printer->Device = NULL;
                }
            }
            else
            {
                Ports->Items[i] = 0;
                if (printer)
                {
                    delete printer;
                    printer = NULL;
                }
            }
        }

        //Keyboards
        DeviceName = Descriptor->Keyboard->DeviceName.LowerCase();
        if ((DeviceName != "")&&
            (Descriptor->Keyboard->Determinated == false)&&
            ((Descriptor->Keyboard->Port == 0)||(Descriptor->Keyboard->Port == PortNumber)))
        {
            if (DeviceName == "iskrakeyboard")
                keyboard = new TIskraKeybDevice(0,PortNumber);

            if (keyboard)
            {
                keyboard->StartDevice();
                Sleep(500);
            }
            if ( (keyboard) && (keyboard->IsItYou()) )
            {
                Log->Write("Найдено устройство "+keyboard->DeviceName+" на порту СОМ"+AnsiString(PortNumber));
                Descriptor->Keyboard->Port = PortNumber;
                Descriptor->Keyboard->Determinated = true;
                Descriptor->Keyboard->Device = (void*)keyboard;
                FindDevice = true;
                if ((keyboard)&&(Descriptor->Keyboard->AutoDestroy))
                {
                    delete keyboard;
                    keyboard = NULL;
                    Descriptor->Keyboard->Device = NULL;
                }
            }
            else
            {
                Ports->Items[i] = 0;
                if (keyboard)
                {
                    delete keyboard;
                    keyboard = NULL;
                }
            }
        }

    }
}

CDeviceChecker::~CDeviceChecker()
{
    if (NewLog)
        delete Log;
    if (PortsList)
        delete PortsList;
    if (Ports)
        delete Ports;
}

