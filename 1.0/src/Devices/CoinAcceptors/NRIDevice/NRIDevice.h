#ifndef NRIdeviceH
#define NRIdeviceH

#include <math.h>

#include "NRIDeviceClass.h"

class TNRIdevice : public TNRIDeviceClass
{
private:
protected:
public:
  TNRIdevice(int id, int ComPort,TLogClass* _Log = NULL);
  virtual ~TNRIdevice();
};


#endif
