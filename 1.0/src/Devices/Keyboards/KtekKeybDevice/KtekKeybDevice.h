#ifndef KtekKeybDeviceH
#define KtekKeybDeviceH

#include <math.h>

#include "KtekKeybDeviceClass.h"

class TKtekKeybDevice : public TKtekKeybDeviceClass
{
private:
protected:
public:
  TKtekKeybDevice(int id, int ComPort,TLogClass* _Log = NULL);
  virtual ~TKtekKeybDevice();
};


#endif
