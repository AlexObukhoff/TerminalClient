#ifndef WatchDogdeviceH
#define WatchDogdeviceH

#include <math.h>

#include "WatchDogClass.h"

class TWatchDogdevice : public TWatchDogDeviceClass
{
private:
protected:
public:
  TWatchDogdevice(int ComPort = 1,TLogClass* _Log = NULL);
  virtual ~TWatchDogdevice();

  virtual void StartDevice();
  virtual void ClearGSM();
  virtual void StartGSM();
  virtual void StopGSM();
  virtual void StopTimer();

  virtual bool IsItYou();
};


#endif
