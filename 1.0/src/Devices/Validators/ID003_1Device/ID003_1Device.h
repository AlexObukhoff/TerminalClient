#ifndef ID003_1deviceH
#define ID003_1deviceH

#include <math.h>

#include "ID003_1DeviceClass.h"

class TID003_1device : public TID003_1DeviceClass
{
private:
protected:
public:
  TID003_1device(int id, int ComPort,TLogClass* _Log = NULL);
  virtual ~TID003_1device();
};


#endif
