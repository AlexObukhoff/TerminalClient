#ifndef LDOGdeviceH
#define LDOGdeviceH

#include <math.h>

#include "LDOGClass.h"

class TLDOGdevice : public TLDOGDeviceClass
{
private:
protected:
  virtual void StartDevice();
public:
  TLDOGdevice(int ComPort = 1,TLogClass* _Log = NULL);
  virtual ~TLDOGdevice();

  void StartTimer();
  void ClearGSM();
  void ResetPC();
  void GetSensors();
  bool IsItYou();
  clock_t GetCreationTime(); //метка времени создания потока
};


#endif
