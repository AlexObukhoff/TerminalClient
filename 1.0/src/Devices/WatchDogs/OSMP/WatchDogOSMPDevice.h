#ifndef WatchDogOSMPdeviceH
#define WatchDogOSMPdeviceH

#include <math.h>

#include "WatchDogOSMPClass.h"

class TWatchDogOSMPdevice : public TWatchDogOSMPDeviceClass
{
private:
protected:
  virtual void SetCommand(BYTE command);
  virtual void StartDevice();
public:
  TWatchDogOSMPdevice(int ComPort = 1,TLogClass* _Log = NULL);
  virtual ~TWatchDogOSMPdevice();

  virtual void StartTimer();
  virtual void StopTimer();
  virtual void ClearGSM();
  virtual void ResetPC();
  virtual bool IsItYou();
  clock_t GetCreationTime(); //метка времени создания потока
};


#endif
