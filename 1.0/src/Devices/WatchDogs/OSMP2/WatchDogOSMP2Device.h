#ifndef WatchDogOSMP2deviceH
#define WatchDogOSMP2deviceH

#include <math.h>

#include "WatchDogOSMP2Class.h"

class TWatchDogOSMP2device : public TWatchDogOSMP2DeviceClass
{
private:
protected:
  virtual void SetCommand(BYTE command);
  virtual void StartDevice();
public:
  TWatchDogOSMP2device(int ComPort = 1,TLogClass* _Log = NULL);
  virtual ~TWatchDogOSMP2device();

  virtual void StartTimer();
  virtual void StopTimer();
  virtual void ClearGSM();
  virtual void ResetPC();
  virtual bool IsItYou();
  clock_t GetCreationTime(); //метка времени создания потока
};


#endif
