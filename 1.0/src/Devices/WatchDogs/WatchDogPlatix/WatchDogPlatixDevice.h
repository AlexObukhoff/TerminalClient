#ifndef WatchDogPlatixDeviceH
#define WatchDogPlatixDeviceH

#include <math.h>

#include "WatchDogPlatixClass.h"

class TWatchDogPlatixDevice : public TWatchDogPlatixDeviceClass
{
private:
protected:
  //virtual void SetCommand(BYTE command);
  virtual void StartDevice();
public:
  TWatchDogPlatixDevice(int ComPort = 1,TLogClass* _Log = NULL);
  virtual ~TWatchDogPlatixDevice();

  virtual void StartTimer();
  virtual void StopTimer();
  virtual void ClearGSM();
  virtual void ResetPC();
  virtual void GetVersion();
  virtual bool IsItYou();
  clock_t GetCreationTime(); //метка времени создания потока
};


#endif
