#ifndef ExpressPaydeviceH
#define ExpressPaydeviceH

#include <math.h>

#include "ExpressPayClass.h"

class TExpressPaydevice : public TExpressPayDeviceClass
{
private:
protected:
  virtual void SetCommand(BYTE command);
  virtual void StartDevice();
public:
  TExpressPaydevice(int ComPort = 1,TLogClass* _Log = NULL);
  virtual ~TExpressPaydevice();

  virtual void StartTimer();
  virtual void StopTimer();
  virtual void ClearGSM();
  virtual void ResetPC();
  virtual bool IsItYou();
  clock_t GetCreationTime(); //метка времени создания потока
};


#endif
