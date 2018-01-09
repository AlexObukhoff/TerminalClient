#ifndef WatchDogAlarmDeviceH
#define WatchDogAlarmDeviceH

#include <math.h>

#include "WatchDogAlarmClass.h"

class TWatchDogAlarmDevice : public TWatchDogAlarmDeviceClass
{
private:
protected:
  //virtual void SetCommand(BYTE command);
  virtual void StartDevice();
public:
  TWatchDogAlarmDevice(int ComPort = 1,TLogClass* _Log = NULL);
  virtual ~TWatchDogAlarmDevice();

  virtual void StartTimer();
  virtual void ClearGSM();
  virtual void Lock();
  virtual void UnLock();
  virtual bool IsItYou();
  clock_t GetCreationTime(); //метка времени создания потока
};


#endif
