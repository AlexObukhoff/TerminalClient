#ifndef SIM2OSMPdeviceH
#define SIM2OSMPdeviceH

#include <math.h>

#include "SIM2OSMPClass.h"

class TSIM2OSMPdevice : public TSIM2OSMPDeviceClass
{
private:
protected:
  virtual void SetCommand(BYTE command);
  virtual void StartDevice();
public:
  TSIM2OSMPdevice(int ComPort = 1,TLogClass* _Log = NULL);
  virtual ~TSIM2OSMPdevice();

  virtual void StartTimer();
  virtual void StopTimer();
  virtual void ClearGSM();
  virtual void ResetPC();
  virtual int ChangeGSM(int SIMnumber);
  virtual int GetState();
  virtual bool IsItYou();
  clock_t GetCreationTime(); //метка времени создания потока
};


#endif
