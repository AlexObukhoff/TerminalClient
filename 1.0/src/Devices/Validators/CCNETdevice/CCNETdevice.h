#ifndef CCNETdeviceH
#define CCNETdeviceH

#include <math.h>

#include "CCNETDeviceClass.h"
//#include "CValidator.h"

//class TCCNETdevice : public TDeviceClass
class TCCNETdevice : public TCCNETDeviceClass
{
private:
protected:
public:
  TCCNETdevice(int id, int ComPort,TLogClass* _Log = NULL);
  virtual ~TCCNETdevice();
};


#endif
