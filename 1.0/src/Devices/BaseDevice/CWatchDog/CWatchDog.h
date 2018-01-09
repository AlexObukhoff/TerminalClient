//---------------------------------------------------------------------------

#ifndef CWatchDogH
#define CWatchDogH

#include "DeviceClass.h"
#include "LogClass.h"

class CWatchDog : public TDeviceClass
{
protected:
public:
  bool OnlyPnP;
  int SIMnumber;
  int SIMmin;
  int SIMmax;
  bool SIM_ChangingEnable;

  CWatchDog(int ComPort,TLogClass* _Log = NULL, AnsiString Prefix = "");
  virtual ~CWatchDog();

  virtual void StartTimer();
  virtual void StopTimer();
  virtual void ClearGSM();
  virtual void ResetPC();
  virtual void StartGSM();
  virtual void StopGSM();
  virtual void Lock();
  virtual void UnLock();
  virtual int  GetState();
  virtual void GetSensors();
  virtual int  ChangeGSM(int SIMnumber);

  virtual bool IsItYou(); 
};

//---------------------------------------------------------------------------
#endif

