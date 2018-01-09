#ifndef JCMdeviceH
#define JCMdeviceH

#include <math.h>

#include "JCMDeviceClass.h"

class TJCMdevice : public TJCMDeviceClass
{
private:
protected:
public:
  TJCMdevice(int id, int ComPort,TLogClass* _Log = NULL);
  virtual ~TJCMdevice();
};


#endif
