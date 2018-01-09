#ifndef NV9_CCNETdeviceH
#define NV9_CCNETdeviceH

#include <math.h>

#include "NV9_CCNETDeviceClass.h"
//#include "CValidator.h"

//class TNV9_CCNETdevice : public TDeviceClass
class TNV9_CCNETdevice : public TNV9_CCNETDeviceClass
{
private:
protected:
public:
  TNV9_CCNETdevice(int id, int ComPort,TLogClass* _Log = NULL);
  virtual ~TNV9_CCNETdevice();
};


#endif
