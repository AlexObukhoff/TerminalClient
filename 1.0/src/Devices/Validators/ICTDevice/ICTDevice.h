#ifndef ICTDeviceH
#define ICTDeviceH

#include <math.h>

#include "ICTDeviceClass.h"

class TICTDevice : public TICTDeviceClass
{
private:
protected:
public:
  TICTDevice(int id, int ComPort,TLogClass* _Log = NULL);
  virtual ~TICTDevice();
};


#endif
