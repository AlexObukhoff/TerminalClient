//---------------------------------------------------------------------------

#ifndef DeviceDescriptorH
#define DeviceDescriptorH

#include <Classes.hpp>
//---------------------------------------------------------------------------

class _device_info
{
public:
    AnsiString DeviceName;
    int Port;
    int ID;
    bool Determinated;//note either object was found or not
    bool AutoDestroy;//note either destroy object after finding or not for next using
    void* Device;

    _device_info();
    virtual ~_device_info();
};


class CDeviceDescriptor
{
private:
public:
    _device_info* Validator[3];
    _device_info* Printer;
    _device_info* WatchDog;
    _device_info* CardReader;
    _device_info* Modem;
    _device_info* Keyboard;

    CDeviceDescriptor();
    virtual ~CDeviceDescriptor();
};


#endif
