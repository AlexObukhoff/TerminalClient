#ifndef V2EdeviceH
#define V2EdeviceH

#include <math.h>

#include "V2EDeviceClass.h"

class TV2Edevice : public TV2EDeviceClass
{
private:
  void CommandDisable();
protected:
public:
  TV2Edevice(int id, int ComPort,TLogClass* _Log = NULL);
  virtual ~TV2Edevice();
};


#endif
