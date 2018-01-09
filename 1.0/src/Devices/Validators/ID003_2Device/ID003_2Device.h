#ifndef ID003_2deviceH
#define ID003_2deviceH

#include <math.h>

#include "ID003_2DeviceClass.h"

class TID003_2device : public TID003_2DeviceClass
{
private:
protected:
public:
  TID003_2device(int id, int ComPort,TLogClass* _Log = NULL);
  virtual ~TID003_2device();
};


#endif
