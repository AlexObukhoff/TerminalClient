#ifndef IskraKeybDeviceH
#define IskraKeybDeviceH

#include <math.h>

#include "IskraKeybDeviceClass.h"

class TIskraKeybDevice : public TIskraKeybDeviceClass
{
private:
protected:
public:
  TIskraKeybDevice(int id, int ComPort,TLogClass* _Log = NULL);
  virtual ~TIskraKeybDevice();
};


#endif
