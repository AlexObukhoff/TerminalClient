//---------------------------------------------------------------------------


#pragma hdrstop

#include "DeviceDescriptor.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

_device_info::_device_info()
{
    DeviceName = "";
    Port = 0;
    ID = -1;
    Determinated = false;
    Device = NULL;
    AutoDestroy = true;
}

_device_info::~_device_info()
{
    /*if (Device)
        delete Device;*/
    Device = NULL;
}

CDeviceDescriptor::CDeviceDescriptor()
{
    Validator[0] = new _device_info();
    Validator[0]->DeviceName = "";
    Validator[0]->Port = 0;
    Validator[0]->ID = -1;
    Validator[0]->Determinated = false;
    Validator[0]->Device = NULL;

    Validator[1] = new _device_info();
    Validator[1]->DeviceName = "";
    Validator[1]->Port = 0;
    Validator[1]->ID = -1;
    Validator[1]->Determinated = false;
    Validator[1]->Device = NULL;

    Validator[2] = new _device_info();
    Validator[2]->DeviceName = "";
    Validator[2]->Port = 0;
    Validator[2]->ID = -1;
    Validator[2]->Determinated = false;
    Validator[2]->Device = NULL;

    Printer = new _device_info();
    Printer->DeviceName = "";
    Printer->Port = 0;
    Printer->ID = -1;
    Printer->Determinated = false;
    Printer->Device = NULL;

    WatchDog = new _device_info();
    WatchDog->DeviceName = "";
    WatchDog->Port = 0;
    WatchDog->ID = -1;
    WatchDog->Determinated = false;
    WatchDog->Device = NULL;

    CardReader = new _device_info();
    CardReader->DeviceName = "";
    CardReader->Port = 0;
    CardReader->ID = -1;
    CardReader->Determinated = false;
    CardReader->Device = NULL;

    Modem = new _device_info();
    Modem->DeviceName = "";
    Modem->Port = 0;
    Modem->ID = -1;
    Modem->Determinated = false;
    Modem->Device = NULL;

    Keyboard = new _device_info();
    Keyboard->DeviceName = "";
    Keyboard->Port = 0;
    Keyboard->ID = -1;
    Keyboard->Determinated = false;
    Keyboard->Device = NULL;
}

CDeviceDescriptor::~CDeviceDescriptor()
{
    if (Validator[0])
      delete Validator[0];

    if (Validator[1])
      delete Validator[1];

    if (Validator[2])
      delete Validator[2];

    if (Printer)
      delete Printer;

    if (WatchDog)
      delete WatchDog;

    if (CardReader)
      delete CardReader;

    if (Modem)
      delete Modem;

    if (Keyboard)
      delete Keyboard;
}

