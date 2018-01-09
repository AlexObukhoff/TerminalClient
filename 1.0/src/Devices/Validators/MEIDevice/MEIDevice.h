#ifndef MEIdeviceH
#define MEIdeviceH

#include <math.h>

#include "MEIDeviceClass.h"

class TMEIdevice : public TMEIDeviceClass
{
private:
protected:
public:
  TMEIdevice(int id, int ComPort,TLogClass* _Log = NULL);
  virtual ~TMEIdevice();
};


#endif
